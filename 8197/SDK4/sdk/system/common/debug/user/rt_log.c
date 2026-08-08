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
 * $Revision: 39249 $
 * $Date: 2013-05-08 13:55:42 +0800 (Wed, 08 May 2013) $
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
#include <rtcore/rtcore.h>
#include <osal/memory.h>
#include <common/debug/rt_log.h>

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */
static uint8 *rtLogModuleName[] = 
{
    (uint8 *)STR_MOD_GENERAL, (uint8 *)STR_MOD_DOT1X, (uint8 *)STR_MOD_FILTER, (uint8 *)STR_MOD_FLOWCTRL,
    (uint8 *)STR_MOD_INIT, (uint8 *)STR_MOD_L2, (uint8 *)STR_MOD_MIRROR, (uint8 *)STR_MOD_NIC, (uint8 *)STR_MOD_PORT,
    (uint8 *)STR_MOD_QOS, (uint8 *)STR_MOD_RATE, (uint8 *)STR_MOD_STAT, (uint8 *)STR_MOD_STP, (uint8 *)STR_MOD_SVLAN,
    (uint8 *)STR_MOD_SWITCH, (uint8 *)STR_MOD_TRAP, (uint8 *)STR_MOD_TRUNK, (uint8 *)STR_MOD_VLAN, (uint8 *)STR_MOD_HAL,
    (uint8 *)STR_MOD_DAL, (uint8 *)STR_MOD_RTDRV, (uint8 *)STR_MOD_RTUSR, (uint8 *)STR_MOD_DIAGSHELL, (uint8 *)STR_MOD_END
};

static rtcore_dev_data_t *pLogCfg;

/*
 * Macro Declaration
 */
#define RT_LOG_USR_PARAM_CHK(level, module)         \
do {                                                \
    if (LOG_TYPE_LEVEL == pLogCfg->log_type)        \
    {                                               \
        if (LOG_MSG_OFF == pLogCfg->log_level)      \
            return RT_ERR_NOT_ALLOWED;              \
        if ((LOG_MSG_OFF <= level) ||               \
                (pLogCfg->log_level < level))       \
            return RT_ERR_NOT_ALLOWED;              \
    }                                               \
    else if (LOG_TYPE_MASK == pLogCfg->log_type)    \
    {                                               \
        if (!(pLogCfg->log_mask & (1 << level)))   \
            return RT_ERR_NOT_ALLOWED;              \
    }                                               \
    else                                            \
        return RT_ERR_FAILED;                       \
    if (!(pLogCfg->log_module_mask & module))       \
        return RT_ERR_NOT_ALLOWED;                  \
} while(0)

/*
 * Function Declaration
 */
 
 /* Function Name:
 *      rt_log_init
 * Description:
 *      Initialize common log module
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 * Note:
 *      Will be called from the init process of RTK module
 */
int32 rt_log_init(void)
{
    int32 page_size = sysconf(_SC_PAGESIZE); 
    pLogCfg = (rtcore_dev_data_t *)osal_mmap(RTCORE_DEV_NAME, 0, page_size);

    return ((void *)RT_ERR_FAILED != pLogCfg) ? RT_ERR_OK : RT_ERR_FAILED;   
}

/* Function Name:
 *      rt_log_enable_get
 * Description:
 *      Get the enable status of the log module
 * Input:
 *      None
 * Output:
 *      pEnable - pointer buffer of the enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 rt_log_enable_get(uint32 *pEnable)
{
    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    /* check which log type is used */    
    if (LOG_TYPE_LEVEL == pLogCfg->log_type)
    {
        if (LOG_MSG_OFF == pLogCfg->log_level)
        {
            *pEnable = DISABLED;
        }
        else
        {
            *pEnable = ENABLED;
        }
    }    
    else if (LOG_TYPE_MASK == pLogCfg->log_type)
    {
        if (LOG_MSG_OFF == pLogCfg->log_mask)
        {
            *pEnable = DISABLED;
        }
        else
        {
            *pEnable = ENABLED; 
        }
    }
    else
    { 
        /* log type not support */
        return RT_ERR_FAILED;           
    }
    
    return RT_ERR_OK;        
}

/* Function Name:
 *      rt_log_enable_set
 * Description:
 *      Set the enable status of the log module
 * Input:
 *      enable - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32 rt_log_enable_set(uint32 enable)
{
    if (ENABLED == enable)
    {
        /* restore the log level or mask */
        if (LOG_TYPE_LEVEL == pLogCfg->log_type)
        {
            pLogCfg->log_level = pLogCfg->log_level_bak;    
        }    
        else if (LOG_TYPE_MASK == pLogCfg->log_type)
        {
            pLogCfg->log_mask = pLogCfg->log_mask_bak;  
        }
        else
        {
            /* log type not support */ 
            return RT_ERR_FAILED;          
        }
    }
    else
    {
        /* keep the current log level or mask before turning off */
        if (LOG_TYPE_LEVEL == pLogCfg->log_type)
        {
            pLogCfg->log_level_bak = pLogCfg->log_level;    
            pLogCfg->log_level = LOG_MSG_OFF;   
        }    
        else if (LOG_TYPE_MASK == pLogCfg->log_type)
        {
            pLogCfg->log_mask_bak = pLogCfg->log_mask;
            pLogCfg->log_mask = LOG_MASK_OFF;    
        }
        else
        {
            /* log type not support */
            return RT_ERR_FAILED;           
        }
    }    
    
    return RT_ERR_OK;      
}

/* Function Name:
 *      rt_log_reset
 * Description:
 *      Reset the log module
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      Used to reset all configuration levels to the default values
 */
void rt_log_reset(void)
{
    rt_log_level_reset();
    rt_log_mask_reset();
    rt_log_type_reset();
    rt_log_moduleMask_reset();
    rt_log_format_reset();  
}

/* Function Name:
 *      rt_log_level_get
 * Description:
 *      Get the log level of the module
 * Input:
 *      None
 * Output:
 *      pLv - pointer buffer of the log level
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 rt_log_level_get(uint32 *pLv)
{
    /* parameter check */
    RT_PARAM_CHK((NULL == pLv), RT_ERR_NULL_POINTER);
            
    *pLv = pLogCfg->log_level;  
      
    return RT_ERR_OK;    
}

/* Function Name:
 *      rt_log_level_set
 * Description:
 *      Set the log level of the module
 * Input:
 *      lv - log level
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
int32 rt_log_level_set(uint32 lv)
{
    /* parameter check */
    RT_PARAM_CHK(!LOG_LEVEL_CHK(lv), RT_ERR_OUT_OF_RANGE);
    
    pLogCfg->log_level = lv;    
    
    return RT_ERR_OK;    
}

/* Function Name:
 *      rt_log_level_reset
 * Description:
 *      Reset the log level to default
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rt_log_level_reset(void)
{
    pLogCfg->log_level = LOG_LEVEL_DEFAULT;    
}

/* Function Name:
 *      rt_log_mask_get
 * Description:
 *      Get the log level mask of the module
 * Input:
 *      None
 * Output:
 *      pMask - pointer buffer of the log level mask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 rt_log_mask_get(uint32 *pMask)
{
    /* parameter check */
    RT_PARAM_CHK((NULL == pMask), RT_ERR_NULL_POINTER);
                
    *pMask = pLogCfg->log_mask;    
    
    return RT_ERR_OK;    
}

/* Function Name:
 *      rt_log_mask_set
 * Description:
 *      Set the log level mask of the module
 * Input:
 *      mask - log level mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
int32 rt_log_mask_set(uint32 mask)
{
    /* parameter check */
    RT_PARAM_CHK(!LOG_MASK_CHK(mask), RT_ERR_OUT_OF_RANGE);

    pLogCfg->log_mask = mask;    
    
    return RT_ERR_OK;    
}

/* Function Name:
 *      rt_log_mask_reset
 * Description:
 *      Reset the log level mask to default
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rt_log_mask_reset(void)
{
    pLogCfg->log_mask = LOG_MASK_DEFAULT;    
}

/* Function Name:
 *      rt_log_type_get
 * Description:
 *      Get the log type of the module
 * Input:
 *      None
 * Output:
 *      pType - pointer buffer of the log type
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 rt_log_type_get(uint32 *pType)
{
    /* parameter check */
    RT_PARAM_CHK((NULL == pType), RT_ERR_NULL_POINTER);  
        
    *pType = pLogCfg->log_type;
    
    return RT_ERR_OK;    
}

/* Function Name:
 *      rt_log_type_set
 * Description:
 *      Set the log type of the module
 * Input:
 *      type - log type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
int32 rt_log_type_set(uint32 type)
{    
    /* parameter check */
    RT_PARAM_CHK(!LOG_TYPE_CHK(type), RT_ERR_OUT_OF_RANGE);
        
    if(pLogCfg->log_type != type)
    {
#ifdef __TYPE_CHANGE_RESET_DEFAULT__

        /* reset to DEFAULT when changing the log type */
        if (LOG_TYPE_LEVEL == type)
        {
            rt_log_mask_reset();
        }
        else if (LOG_TYPE_MASK == type)
        {
            rt_log_level_reset();
        }
        else
        {
            /* log type not support */
            return RT_ERR_FAILED;     
        }
#endif        
        pLogCfg->log_type = type;
    }
    
    return RT_ERR_OK;    
}

/* Function Name:
 *      rt_log_type_reset
 * Description:
 *      Reset the log type to default
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rt_log_type_reset(void)
{
    pLogCfg->log_type = LOG_TYPE_DEFAULT;   
}

/* Function Name:
 *      rt_log_moduleMask_get
 * Description:
 *      Get the log module mask of the module
 * Input:
 *      None
 * Output:
 *      pMask - pointer buffer of the log module mask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 rt_log_moduleMask_get(uint64 *pMask)
{
    /* parameter check */
    RT_PARAM_CHK((NULL == pMask), RT_ERR_NULL_POINTER);  
                
    *pMask = pLogCfg->log_module_mask;    
    
    return RT_ERR_OK;    
}

/* Function Name:
 *      rt_log_moduleMask_set
 * Description:
 *      Set the log module mask of the module
 * Input:
 *      mask - log module mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
int32 rt_log_moduleMask_set(uint64 mask)
{
    /* parameter check */
    RT_PARAM_CHK(!LOG_MOD_CHK(mask), RT_ERR_OUT_OF_RANGE);

    pLogCfg->log_module_mask = mask;    
    
    return RT_ERR_OK;    
}

/* Function Name:
 *      rt_log_moduleMask_reset
 * Description:
 *      Reset the log module mask to default
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rt_log_moduleMask_reset(void)
{
    pLogCfg->log_module_mask = MOD_MASK_DEFAULT;
}

/* Function Name:
 *      rt_log_format_get
 * Description:
 *      Get the log format of the module
 * Input:
 *      None
 * Output:
 *      pFormat - pointer buffer of the log format
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 rt_log_format_get(uint32 *pFormat)
{
    /* parameter check */
    RT_PARAM_CHK((NULL == pFormat), RT_ERR_NULL_POINTER);  
        
    *pFormat = pLogCfg->log_format;    
    
    return RT_ERR_OK;    
}

/* Function Name:
 *      rt_log_format_set
 * Description:
 *      Set the log format of the module
 * Input:
 *      format - log format
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
int32 rt_log_format_set(uint32 format)
{
    /* parameter check */
    RT_PARAM_CHK(!LOG_FORMAT_CHK(format), RT_ERR_OUT_OF_RANGE);

    pLogCfg->log_format = format;   
    
    return RT_ERR_OK;    
}

/* Function Name:
 *      rt_log_format_reset
 * Description:
 *      Reset the log format to default
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rt_log_format_reset(void)
{
    pLogCfg->log_format = LOG_FORMAT_DEFAULT;   
}

/* Function Name:
 *      rt_log_config_get
 * Description:
 *      Get the log config settings of the module
 * Input:
 *      None
 * Output:
 *      pCfg - pointer buffer of the log config settings
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 rt_log_config_get(uint32 *pCfg)
{   
    /* parameter check */
    RT_PARAM_CHK((NULL == pCfg), RT_ERR_NULL_POINTER);    
        
    *(pCfg + 0) = pLogCfg->log_level; 
    *(pCfg + 1) = pLogCfg->log_mask;
    *(pCfg + 2) = pLogCfg->log_type;
    *(pCfg + 3) = pLogCfg->log_module_mask; 
    *(pCfg + 4) = pLogCfg->log_format;    

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
    int32  result = RT_ERR_FAILED;
    static uint8  buf[LOG_BUFSIZE_DEFAULT]; /* init value will be given by RT_LOG_FORMATTED_OUTPUT */    
   
    /* check log level and module */
    RT_LOG_USR_PARAM_CHK(level, module);

    /* formatted output conversion */     
    RT_LOG_FORMATTED_OUTPUT((char *)buf, format, result);

    if (result < 0)
        return RT_ERR_FAILED;  
    
    /* get the current time */
    time_t t = time(0);
    struct tm *pLt = localtime(&t);	    
    
    /* if that worked, print to console */
    if (LOG_FORMAT_DETAILED == pLogCfg->log_format)
    {
        rt_log_printf("* ------------------------------------------------------------\n");
        rt_log_printf("* Level : %d\n", level);
        rt_log_printf("* Module: %s\n", *rt_log_moduleName_get(pLogCfg->log_module_mask & module));			    
        rt_log_printf("* Date  : %04d-%02d-%02d\n", pLt->tm_year + 1900, pLt->tm_mon + 1, pLt->tm_mday);
        rt_log_printf("* Time  : %02d:%02d:%02d\n", pLt->tm_hour, pLt->tm_min, pLt->tm_sec);                
        rt_log_printf("* Log   : %s\n", buf);			    
    }
    else
    {
        rt_log_printf("[%d][%s] %02d:%02d:%02d %s", level, *rt_log_moduleName_get(pLogCfg->log_module_mask & module),
                      pLt->tm_hour, pLt->tm_min, pLt->tm_sec, buf);            
    }    	 

    return RT_ERR_OK;
}

