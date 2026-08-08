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
 * $Revision: 48879 $
 * $Date: 2014-06-25 14:45:17 +0800 (Wed, 25 Jun 2014) $
 *
 * Purpose : Definition those debug command and APIs in the SDK diagnostic shell.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) debug
 *
 */

/*
 * Include Files
 */
#include <common/rt_autoconf.h>
#include <common/rt_error.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <common/debug/rt_log.h>
#include <common/debug/mem.h>
#include <diag_util.h>
#include <diag_om.h>
#include <parser/cparser_priv.h>
#include <hal/mac/mac_debug.h>
#include <hal/mac/mem.h>
#include <rtk/switch.h>
#ifdef CMD_DEBUG_FLASHTEST_MTD
#include <mtd/mtd-user.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <osal/memory.h>
#include <sys/ioctl.h>
#endif

#ifdef CMD_DEBUG_GET_LOG
/*
 * debug get log
 */
cparser_result_t cparser_cmd_debug_get_log(cparser_context_t *context)
{
#if defined(CONFIG_SDK_DEBUG)
    uint32  unit = 0;
    uint32  data = 0;
    uint64  data64 = 0;
    uint32  i = 0;
    uint32  log_type = LOG_TYPE_DEFAULT;
    int32   ret = RT_ERR_FAILED;

    char *pLevelName[] = {
        "fatal", "major", "minor", "warning", "event", "info",
        "func", "debug", "trace", ""
    };

    char *pModName[] = {
        STR_MOD_GENERAL, STR_MOD_DOT1X, STR_MOD_FILTER, STR_MOD_FLOWCTRL,
        STR_MOD_INIT, STR_MOD_L2, STR_MOD_MIRROR, STR_MOD_NIC, STR_MOD_PORT,
        STR_MOD_QOS, STR_MOD_RATE, STR_MOD_STAT, STR_MOD_STP, STR_MOD_SVLAN,
        STR_MOD_SWITCH, STR_MOD_TRAP, STR_MOD_TRUNK, STR_MOD_VLAN, STR_MOD_PIE,
        STR_MOD_HAL, STR_MOD_DAL, STR_MOD_RTDRV, STR_MOD_RTUSR, STR_MOD_DIAGSHELL,
        STR_MOD_UNITTEST, STR_MOD_OAM, STR_MOD_L3, STR_MOD_RTCORE, STR_MOD_EEE,
        STR_MOD_SEC, STR_MOD_LED, STR_MOD_RSVD_001, STR_MOD_RSVD_002, STR_MOD_RSVD_003, 
        STR_MOD_END
    };

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(rt_log_enable_get(&data), ret);
    if (data < RTK_ENABLE_END)
        diag_util_printf("    status      : %s \n", data ? "ENABLE" : "DISABLE");
    else
        diag_util_printf("    status      : ERROR \n");

    DIAG_UTIL_ERR_CHK(rt_log_type_get(&log_type), ret);
    if (log_type < LOG_TYPE_END)
        diag_util_printf("    type        : %s \n", log_type ? "LEVEL-MASK" : "LEVEL");
    else
        diag_util_printf("    type        : ERROR \n");

    data = 0;
    DIAG_UTIL_ERR_CHK(rt_log_level_get(&data), ret);
    if (data < LOG_LV_END)
    {
        if (LOG_MSG_OFF == data)
        {
            diag_util_printf("    level       : Message off ");
        }
        else
        {
            diag_util_printf("    level       : %d ", data);
        }
        if (LOG_TYPE_LEVEL == log_type)
            diag_util_printf("(*)");
        diag_util_printf("\n");
    }
    else
        diag_util_printf("    level       : ERROR \n");

    data = 0;
    DIAG_UTIL_ERR_CHK(rt_log_mask_get(&data), ret);
    if (data <= LOG_MASK_ALL)
    {
        diag_util_printf("    level-mask  : ");
        if (data)
        {
            for (i = 0; i < LOG_MSG_OFF; i++)
            {
                if ((data >> i) & 0x1)
                    diag_util_printf("%s ", *(pLevelName + i));
            }
        }
        else
            diag_util_printf("ALL_MSG_OFF");

        if (LOG_TYPE_MASK == log_type)
            diag_util_printf("(*)");
        diag_util_printf("\n");
    }
    else
        diag_util_printf("    level-mask  : ERROR \n");

    data = 0;
    DIAG_UTIL_ERR_CHK(rt_log_format_get(&data), ret);
    if (data < LOG_FORMAT_END)
        diag_util_printf("    format      : %s \n", data ? "DETAILED" : "NORMAL");
    else
        diag_util_printf("    format      : ERROR \n");

    data64 = 0;
    DIAG_UTIL_ERR_CHK(rt_log_moduleMask_get(&data64), ret);
    if (data64 <= MOD_ALL)
    {
        diag_util_printf("    module-mask : ");
        if (data64)
        {
            for (i = 0; i < SDK_MOD_END; i++)
            {
                if ((data64 >> i) & 0x1)
                    diag_util_printf("%s ", *(pModName + i));
            }
        }
        else
            diag_util_printf("ALL_MODULE_OFF");
        diag_util_printf("\n\n");
    }
    else
        diag_util_printf("    module-mask : ERROR \n");
#endif
    return CPARSER_OK;
} /* end of cparser_cmd_debug_get_log */
#endif

#ifdef CMD_DEBUG_GET_MEMORY_ADDRESS_WORDS
/*
 * debug get memory <UINT:address> { <UINT:words> }
 */
cparser_result_t cparser_cmd_debug_get_memory_address_words(cparser_context_t *context,
    uint32_t *address_ptr, uint32_t *words_ptr)
{
#if defined(CONFIG_SDK_DEBUG)
    uint32  unit = 0;
    uint32  mem = 0;
    uint32  value = 0;
    uint32  mem_words = 0;
    uint32  index = 0;
    int32   ret = RT_ERR_FAILED;

    /* Don't check the (NULL == words_ptr) due to it is optional token */
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    mem = *address_ptr;
    if (0 != (mem % 4))
    {
        diag_util_printf("\n\rWarning! The address must be a multiple of 4.\n\r\n\r");
        return CPARSER_NOT_OK;
    }

    if ('\0' == TOKEN_CHAR(4,0))
    {
        DIAG_UTIL_ERR_CHK(debug_mem_read(unit, mem, &value), ret);
        diag_util_mprintf("Memory 0x%x : 0x%08x\n", mem, value);
    }
    else
    {
        mem_words = *words_ptr;
        for (index = 0; index < mem_words; index++)
        {
            DIAG_UTIL_ERR_CHK(debug_mem_read(unit, mem, &value), ret);
            if (0 == (index % 4))
            {
                diag_util_mprintf("\n");
                diag_util_printf("0x%08x ", mem);
            }
            diag_util_printf("0x%08x ", value);
            mem = mem + 4;
        }
        diag_util_mprintf("\n");
    }

#endif
    return CPARSER_OK;
} /* end of cparser_cmd_debug_get_memory_address_words */
#endif

#ifdef CMD_DEBUG_SET_LOG_STATE_DISABLE_ENABLE
/*
 * debug set log state ( disable | enable )
 */
cparser_result_t cparser_cmd_debug_set_log_state_disable_enable(cparser_context_t *context)
{
#if defined(CONFIG_SDK_DEBUG)
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('e' == TOKEN_CHAR(4,0))
    {
        DIAG_UTIL_ERR_CHK(rt_log_enable_set(ENABLED), ret);
    }
    else if ('d' == TOKEN_CHAR(4,0))
    {
        DIAG_UTIL_ERR_CHK(rt_log_enable_set(DISABLED), ret);
    }
    else {}
#endif
    return CPARSER_OK;
} /* end of cparser_cmd_debug_set_log_state_disable_enable */
#endif

#ifdef CMD_DEBUG_SET_LOG_LEVEL_VALUE
/*
 * debug set log level <UINT:value>
 */
cparser_result_t cparser_cmd_debug_set_log_level_value(cparser_context_t *context,
    uint32_t *value_ptr)
{
#if defined(CONFIG_SDK_DEBUG)
    uint32  unit = 0;
    uint32  log_level = 0;
    int32   ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    log_level = *value_ptr;
    DIAG_UTIL_ERR_CHK(rt_log_level_set(log_level), ret);
#endif
    return CPARSER_OK;
} /* end of cparser_cmd_debug_set_log_level_value */
#endif

#ifdef CMD_DEBUG_SET_LOG_LEVEL_MASK_BITMASK
/*
 * debug set log level-mask <UINT:bitmask>
 */
cparser_result_t cparser_cmd_debug_set_log_level_mask_bitmask(cparser_context_t *context,
    uint32_t *bitmask_ptr)
{
#if defined(CONFIG_SDK_DEBUG)
    uint32  unit = 0;
    uint32  log_level_mask = 0;
    int32   ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    log_level_mask = *bitmask_ptr;
    DIAG_UTIL_ERR_CHK(rt_log_mask_set(log_level_mask), ret);
#endif
    return CPARSER_OK;
} /* end of cparser_cmd_debug_set_log_level_mask_bitmask */
#endif

#ifdef CMD_DEBUG_SET_LOG_LEVEL_TYPE_LEVEL_LEVEL_MASK
/*
 * debug set log level-type ( level | level-mask )
 */
cparser_result_t cparser_cmd_debug_set_log_level_type_level_level_mask(cparser_context_t *context)
{
#if defined(CONFIG_SDK_DEBUG)
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (strlen(TOKEN_STR(4)) == strlen("level"))
    {
        DIAG_UTIL_ERR_CHK(rt_log_type_set(LOG_TYPE_LEVEL), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rt_log_type_set(LOG_TYPE_MASK), ret);
    }
#endif
    return CPARSER_OK;
} /* end of cparser_cmd_debug_set_log_level_type_level_level_mask */
#endif

#ifdef CMD_DEBUG_SET_LOG_FORMAT_NORMAL_DETAIL
/*
 * debug set log format ( normal | detail )
 */
cparser_result_t cparser_cmd_debug_set_log_format_normal_detail(cparser_context_t *context)
{
#if defined(CONFIG_SDK_DEBUG)
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('d' == TOKEN_CHAR(4,0))
    {
        DIAG_UTIL_ERR_CHK(rt_log_format_set(LOG_FORMAT_DETAILED), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rt_log_format_set(LOG_FORMAT_NORMAL), ret);
    }
#endif
    return CPARSER_OK;
} /* end of cparser_cmd_debug_set_log_format_normal_detail */
#endif

#ifdef CMD_DEBUG_SET_LOG_MODULE_BITMASK
/*
 * debug set log module <UINT64:bitmask>
 */
cparser_result_t cparser_cmd_debug_set_log_module_bitmask(cparser_context_t *context,
    uint64_t *bitmask_ptr)
{
#if defined(CONFIG_SDK_DEBUG)
    uint32  unit = 0;
    uint64  log_module_mask = 0;
    int32   ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    log_module_mask = *bitmask_ptr;
    DIAG_UTIL_ERR_CHK(rt_log_moduleMask_set(log_module_mask), ret);
#endif
    return CPARSER_OK;
} /* end of cparser_cmd_debug_set_log_module_bitmask */
#endif

#ifdef CMD_DEBUG_SET_MEMORY_ADDRESS_VALUE
/*
 * debug set memory <UINT:address> <UINT:value>
 */
cparser_result_t cparser_cmd_debug_set_memory_address_value(cparser_context_t *context,
    uint32_t *address_ptr, uint32_t *value_ptr)
{
#if defined(CONFIG_SDK_DEBUG)
    uint32  unit = 0;
    uint32  mem = 0;
    uint32  value  = 0;
    int32   ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    mem = *address_ptr;
    value = *value_ptr;

    if (0 != (mem % 4))
    {
        diag_util_printf("\n\rWarning! The address must be a multiple of 4.\n\r\n\r");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(debug_mem_write(unit, mem, value), ret);
#endif
    return CPARSER_OK;
} /* end of cparser_cmd_debug_set_memory_address_value */
#endif

#ifdef CMD_DEBUG_DUMP_HSA
/*
 * debug dump hsa
 */
cparser_result_t cparser_cmd_debug_dump_hsa(cparser_context_t *context)
{
#if defined(CONFIG_SDK_DEBUG)
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(hal_dumpHsa(unit), ret);
#endif
    return CPARSER_OK;
} /* end of cparser_cmd_debug_dump_hsa */
#endif

#ifdef CMD_DEBUG_DUMP_HSB
/*
 * debug dump hsb
 */
cparser_result_t cparser_cmd_debug_dump_hsb(cparser_context_t *context)
{
#if defined(CONFIG_SDK_DEBUG)
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(hal_dumpHsb(unit), ret);
#endif
    return CPARSER_OK;
} /* end of cparser_cmd_debug_dump_hsb */
#endif

#ifdef CMD_DEBUG_DUMP_HSM
/*
 * debug dump hsm
 */
cparser_result_t cparser_cmd_debug_dump_hsm(cparser_context_t *context)
{
#if defined(CONFIG_SDK_DEBUG)
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(hal_dumpHsm(unit), ret);
#endif
    return CPARSER_OK;
} /* end of cparser_cmd_debug_dump_hsm */
#endif

#ifdef CMD_DEBUG_DUMP_PMI
/*
 * debug dump pmi
 */
cparser_result_t cparser_cmd_debug_dump_pmi(cparser_context_t *context)
{
#if defined(CONFIG_SDK_DEBUG)
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(hal_dumpPmi(unit), ret);
#endif
    return CPARSER_OK;
} /* end of cparser_cmd_debug_dump_pmi */
#endif


#ifdef CMD_DEBUG_DUMP_HSM_INDEX
/*
 * debug dump hsm <UINT:index>
 */
cparser_result_t cparser_cmd_debug_dump_hsm_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
#if defined(CONFIG_SDK_DEBUG)
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_switch_devInfo_t devInfo;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if((ret = diag_om_get_deviceInfo(unit, &devInfo)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if(*index_ptr > 2)
    {
        return CPARSER_NOT_OK;
    }

#if defined(CONFIG_SDK_RTL8380)
    if (DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        DIAG_UTIL_ERR_CHK(hal_dumpHsmIdx(unit, *index_ptr), ret);
    }
#endif

#endif
    return CPARSER_OK;
} /* end of cparser_cmd_debug_dump_ppi_index */
#endif


#ifdef CMD_DEBUG_DUMP_PPI_INDEX
/*
 * debug dump ppi <UINT:index>
 */
cparser_result_t cparser_cmd_debug_dump_ppi_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
#if defined(CONFIG_SDK_DEBUG)
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_switch_devInfo_t devInfo;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if((ret = diag_om_get_deviceInfo(unit, &devInfo)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (devInfo.chipId == RTL8389M_CHIP_ID || devInfo.chipId == RTL8389L_CHIP_ID ||
        devInfo.chipId == RTL8329M_CHIP_ID || devInfo.chipId == RTL8377M_CHIP_ID)
    {
        diag_util_printf("\n\rWarning! The chip is not supported PPI.\n\r");
        return CPARSER_NOT_OK;
    }
    else
    {
        if(*index_ptr > 6)
        {
            return CPARSER_NOT_OK;
        }
    
#if defined(CONFIG_SDK_RTL8328)
        if (DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
        {
            DIAG_UTIL_ERR_CHK(hal_dumpPpi(unit, *index_ptr), ret);
        }
#endif
    }
#endif
    return CPARSER_OK;
} /* end of cparser_cmd_debug_dump_ppi_index */
#endif

#ifdef CMD_DEBUG_GET_TABLE_TABLE_IDX_ADDRESS
/*
 * debug get table <UINT:table_idx> <UINT:address>
 */
cparser_result_t cparser_cmd_debug_get_table_table_idx_address(cparser_context_t *context,
    uint32_t *table_idx_ptr,
    uint32_t *address_ptr)

{
    uint32      unit = 0;
    uint32      loop;
    int32       ret = RT_ERR_FAILED;
    uint32      value[20];

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
        
    if ((ret = table_read(unit, *table_idx_ptr, *address_ptr, value)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    } 

    diag_util_mprintf("Table %u, address %u\n", *table_idx_ptr, *address_ptr);

    if (DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        uint32  table_size;

        switch(*table_idx_ptr)
        {
            case 0:
              table_size = 18;  /*ACL*/
              break;
            case 11:
            case 13:
            case 14:
            case 16:    
              table_size = 2;
              break;
            case 12:
            case 15:    
              table_size = 1;
              break;
            case 17:    
              table_size = 6;
              break;
            default:
              table_size = 3;  /*L2*/
              break;
        }
        
        for (loop = 0; loop < table_size; loop++)
        {
            diag_util_printf("%x", value[loop]);
            if(loop < table_size-1)
                diag_util_printf("-");
        }
        
        diag_util_mprintf("\n\n");
    
        if(0 == *table_idx_ptr)
        {
            for (loop = 0; loop < 7; loop++)
            {
                if(6 != loop)
                    diag_util_printf("Field %d-%d:\t", 11-loop*2, 10-loop*2);
                else
                    diag_util_printf("Fixed Field:\t");
                
                diag_util_printf("data 0x%08x  ", value[loop]);
                diag_util_printf("mask 0x%08x", value[loop+7]);            
                diag_util_mprintf("\n");
    
            }
        
            diag_util_mprintf("\n");
            diag_util_printf("valid:%d\t\tnot:%d\t\tand1:%d\t\tand2:%d\n", 
                            (value[14]&0x80000000) >> 31,
                            (value[14]&0x40000000) >> 30,
                            (value[14]&0x20000000) >> 29,
                            (value[14]&0x10000000) >> 28);
            diag_util_printf("shap:%d\t\titpid:%d\t\totpid:%d\t\tcpu_pri:%d\tnor_pri:%d\n", 
                            (value[17]&0x1) >> 0,
                            (value[17]&0x2) >> 1,
                            (value[17]&0x4) >> 2,
                            (value[17]&0x8) >> 3,
                            (value[17]&0x10) >> 4);
            diag_util_printf("mir:%d\t\ttagst:%d\t\tmeter:%d\t\trmk:%d\t\tlog:%d\n", 
                            (value[17]&0x20) >> 5,
                            (value[17]&0x40) >> 6,
                            (value[17]&0x80) >> 7,
                            (value[17]&0x100) >> 8,
                            (value[17]&0x200) >> 9); 
            diag_util_printf("flt:%d\t\tivid:%d\t\tovid:%d\t\tfwd:%d\t\tdrop:%d\n", 
                            (value[17]&0x400) >> 10,
                            (value[17]&0x800) >> 11,
                            (value[17]&0x1000) >> 12,
                            (value[17]&0x2000) >> 13,
                            (value[17]&0xc000) >> 14);
            diag_util_printf("AIF0:0x%4x\tAIF1:0x%4x\tAIF2:0x%4x\tAIF3:0x%4x\tAIF4:0x%4x\n", 
                            (value[16]&0xffff) >> 0,
                            (value[16]&0x3fff0000) >> 16,
                            (value[15]&0x3fff) >> 0,
                            (value[15]&0x3ff0000) >> 16,
                            (value[14]&0x1ff) >> 0);
        }
    
        diag_util_mprintf("\n");    
    }
    else if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        for (loop = 0; loop < 20; loop++)
        {
            diag_util_printf("%x-", value[loop]);
        }
        
        diag_util_mprintf("\n");
    }
    return CPARSER_OK;
}
#endif

#ifdef CMD_DEBUG_GET_WATCHDOG_COUNTER
cparser_result_t cparser_cmd_debug_get_sw_wd(cparser_context_t *context)
{
	uint32      value;
	hal_getWatchdogCnt(0, 0, &value);
	diag_util_printf("\nPktBuf WatchDog CNT = %d", value);
	hal_getWatchdogCnt(0, 1, &value);
	diag_util_printf("\nSerDes WatchDog CNT = %d", value);
	hal_getWatchdogCnt(0, 2, &value);
	diag_util_printf("\nPHY       WatchDog CNT = %d", value);
	hal_getWatchdogCnt(0, 3, &value);
	diag_util_printf("\nFiber Rx  WatchDog CNT = %d\n", value);

    return CPARSER_OK;
}
#endif
#ifdef CMD_DEBUG_GET_CHIP
cparser_result_t cparser_cmd_debug_get_chip(cparser_context_t *context)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_switch_devInfo_t    devInfo;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    memset(&devInfo, 0, sizeof(rtk_switch_devInfo_t));
    DIAG_UTIL_ERR_CHK(rtk_switch_deviceInfo_get(unit, &devInfo), ret);
   

    diag_util_mprintf("Chip ID   : %x\n", devInfo.chipId);
	diag_util_mprintf("Reversion : %x\n", devInfo.revision);
    diag_util_mprintf("Family ID : %x\n", devInfo.familyId);
	
	return CPARSER_OK;
}
#endif

#ifdef CMD_DEBUG_DUMP_MIB_COUNTER_DEBUG_INDEX
/*
 * debug dump mib counter debug <UINT:index>
 */
cparser_result_t cparser_cmd_debug_dump_mib_counter_debug_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32  unit = 0, cntr = 0;
    int32   ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(hal_getDbgCntr(unit, *index_ptr, &cntr), ret);

    switch (*index_ptr)
    {
        case RTK_DBG_MIB_ALE_TX_GOOD_PKTS:
            diag_util_mprintf("Debug Counter %d - The number of ALE TX good packets : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_ERROR_PKTS:
            diag_util_mprintf("Debug Counter %d - Error packet : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_EGR_ACL_DROP:
            diag_util_mprintf("Debug Counter %d - Egress ACL drop : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_EGR_METER_DROP:
            diag_util_mprintf("Debug Counter %d - Egress meter drop : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_OAM:
            diag_util_mprintf("Debug Counter %d - OAM : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_CFM:
            diag_util_mprintf("Debug Counter %d - CFM : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_VLAN_IGR_FLTR:
            diag_util_mprintf("Debug Counter %d - VLAN ingress filter : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_VLAN_ERR:
            diag_util_mprintf("Debug Counter %d - VLAN Error(VID=4095 or MBR=0) : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_INNER_OUTER_CFI_EQUAL_1:
            diag_util_mprintf("Debug Counter %d - inner/outer CFI=1 : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_VLAN_TAG_FORMAT:
            diag_util_mprintf("Debug Counter %d - VLAN tag format : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_SRC_PORT_SPENDING_TREE:
            diag_util_mprintf("Debug Counter %d - Source port spending tree : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_INBW:
            diag_util_mprintf("Debug Counter %d - Input bandwidth : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_RMA:
            diag_util_mprintf("Debug Counter %d - RMA : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_HW_ATTACK_PREVENTION:
            diag_util_mprintf("Debug Counter %d - hardware attack prevention : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_PROTO_STORM:
            diag_util_mprintf("Debug Counter %d - protocol storm : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_MCAST_SA:
            diag_util_mprintf("Debug Counter %d - multicast SA : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_IGR_ACL_DROP:
            diag_util_mprintf("Debug Counter %d - Ingress ACL drop : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_IGR_METER_DROP:
            diag_util_mprintf("Debug Counter %d - Ingress Meter drop : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_DFLT_ACTION_FOR_MISS_ACL_AND_C2SC:
            diag_util_mprintf("Debug Counter %d - Default action for miss ACL &C2SC : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_NEW_SA:
            diag_util_mprintf("Debug Counter %d - New SA : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_PORT_MOVE:
            diag_util_mprintf("Debug Counter %d - Port move : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_SA_BLOCKING:
            diag_util_mprintf("Debug Counter %d - SA blocking : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_ROUTING_EXCEPTION:
            diag_util_mprintf("Debug Counter %d - routing exception : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_SRC_PORT_SPENDING_TREE_NON_FWDING:
            diag_util_mprintf("Debug Counter %d - Source port spending tree(non forwarding) : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_MAC_LIMIT:
            diag_util_mprintf("Debug Counter %d - MAC limit : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_UNKNOW_STORM:
            diag_util_mprintf("Debug Counter %d - Unknow storm : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_MISS_DROP:
            diag_util_mprintf("Debug Counter %d - Miss Drop : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_CPU_MAC_DROP:
            diag_util_mprintf("Debug Counter %d - CPU MAC drop : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_DA_BLOCKING:
            diag_util_mprintf("Debug Counter %d - DA blocking : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_SRC_PORT_FILTER_BEFORE_EGR_ACL:
            diag_util_mprintf("Debug Counter %d (Egress) - Source port filter(before Egress ACL) : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_VLAN_EGR_FILTER:
            diag_util_mprintf("Debug Counter %d (Egress) - VLAN egress filter : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_SPANNING_TRE:
            diag_util_mprintf("Debug Counter %d (Egress) - Spanning tree : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_PORT_ISOLATION:
            diag_util_mprintf("Debug Counter %d (Egress) - Port isolation : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_OAM_EGRESS_DROP:
            diag_util_mprintf("Debug Counter %d (Egress) - OAM : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_MIRROR_ISOLATION:
            diag_util_mprintf("Debug Counter %d (Egress) - Mirror isolation : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_MAX_LEN_BEFORE_EGR_ACL:
            diag_util_mprintf("Debug Counter %d (Egress) - Max length(before Egress ACL) : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_SRC_PORT_FILTER_BEFORE_MIRROR:
            diag_util_mprintf("Debug Counter %d (Egress) - Source port filter(before Mirror) : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_MAX_LEN_BEFORE_MIRROR:
            diag_util_mprintf("Debug Counter %d (Egress) - Max length(before Mirror) : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_SPECIAL_CONGEST_BEFORE_MIRROR:
            diag_util_mprintf("Debug Counter %d (Egress) - Special congest(before Mirror) : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_LINK_STATUS_BEFORE_MIRROR:
            diag_util_mprintf("Debug Counter %d (Egress) - Link status(before Mirror) : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_WRED_BEFORE_MIRROR:
            diag_util_mprintf("Debug Counter %d (Egress) - WRED(before Mirror) : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_MAX_LEN_AFTER_MIRROR:
            diag_util_mprintf("Debug Counter %d (Egress) - Max length(after Mirror) : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_SPECIAL_CONGEST_AFTER_MIRROR:
            diag_util_mprintf("Debug Counter %d (Egress) - Special congest(after Mirror) : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_LINK_STATUS_AFTER_MIRROR:
            diag_util_mprintf("Debug Counter %d (Egress) - Link status(after Mirror) : %u \n", *index_ptr, cntr);
            break;
        case RTK_DBG_MIB_WRED_AFTER_MIRROR:
            diag_util_mprintf("Debug Counter %d (Egress) - WRED(after Mirror) : %u \n", *index_ptr, cntr);
            break;
        default:
            diag_util_printf("User config: Error!\n");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_DEBUG_DUMP_MIB_COUNTER_DEBUG_ALL
/*
 * debug dump mib counter debug all
 */
cparser_result_t cparser_cmd_debug_dump_mib_counter_debug_all(cparser_context_t *context)
{
    uint32  unit = 0, idx=0, cntr = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  idx2 = 0;
	
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
    	    for (idx2 = 0; idx2 < RTK_DBG_MIB_DBG_TYPE_END; idx2++)
	    {
	        ret = hal_getDbgCntr(unit, idx2, &cntr);
		 if(ret != RT_ERR_OK)
		 	continue;

	        switch (idx2)
	        {
	            case RTK_DBG_MIB_ALE_TX_GOOD_PKTS:
	                diag_util_mprintf("Debug Counter %d - The number of ALE TX good packets : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_ERROR_PKTS:
	                diag_util_mprintf("Debug Counter %d - Error packet : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_EGR_ACL_DROP:
	                diag_util_mprintf("Debug Counter %d - Egress ACL drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_EGR_METER_DROP:
	                diag_util_mprintf("Debug Counter %d - Egress meter drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_OAM:
	                diag_util_mprintf("Debug Counter %d - OAM : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_CFM:
	                diag_util_mprintf("Debug Counter %d - CFM : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_VLAN_IGR_FLTR:
	                diag_util_mprintf("Debug Counter %d - VLAN ingress filter : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_VLAN_ERR:
	                diag_util_mprintf("Debug Counter %d - VLAN Error(VID=4095 or MBR=0) : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_INNER_OUTER_CFI_EQUAL_1:
	                diag_util_mprintf("Debug Counter %d - inner/outer CFI=1 : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_VLAN_TAG_FORMAT:
	                diag_util_mprintf("Debug Counter %d - VLAN tag format : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_SRC_PORT_SPENDING_TREE:
	                diag_util_mprintf("Debug Counter %d - Source port spending tree : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_INBW:
	                diag_util_mprintf("Debug Counter %d - Input bandwidth : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_RMA:
	                diag_util_mprintf("Debug Counter %d - RMA : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_HW_ATTACK_PREVENTION:
	                diag_util_mprintf("Debug Counter %d - hardware attack prevention : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_PROTO_STORM:
	                diag_util_mprintf("Debug Counter %d - protocol storm : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_MCAST_SA:
	                diag_util_mprintf("Debug Counter %d - multicast SA : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_IGR_ACL_DROP:
	                diag_util_mprintf("Debug Counter %d - Ingress ACL drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_IGR_METER_DROP:
	                diag_util_mprintf("Debug Counter %d - Ingress Meter drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_DFLT_ACTION_FOR_MISS_ACL_AND_C2SC:
	                diag_util_mprintf("Debug Counter %d - Default action for miss ACL &C2SC : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_NEW_SA:
	                diag_util_mprintf("Debug Counter %d - New SA : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_PORT_MOVE:
	                diag_util_mprintf("Debug Counter %d - Port move : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_SA_BLOCKING:
	                diag_util_mprintf("Debug Counter %d - SA blocking : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_ROUTING_EXCEPTION:
	                diag_util_mprintf("Debug Counter %d - routing exception : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_SRC_PORT_SPENDING_TREE_NON_FWDING:
	                diag_util_mprintf("Debug Counter %d - Source port spending tree(non forwarding) : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_MAC_LIMIT:
	                diag_util_mprintf("Debug Counter %d - MAC limit : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_UNKNOW_STORM:
	                diag_util_mprintf("Debug Counter %d - Unknow storm : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_MISS_DROP:
	                diag_util_mprintf("Debug Counter %d - Miss Drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_CPU_MAC_DROP:
	                diag_util_mprintf("Debug Counter %d - CPU MAC drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_DA_BLOCKING:
	                diag_util_mprintf("Debug Counter %d - DA blocking : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_SRC_PORT_FILTER_BEFORE_EGR_ACL:
	                diag_util_mprintf("Debug Counter %d (Egress)- Source port filter(before Egress ACL) : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_VLAN_EGR_FILTER:
	                diag_util_mprintf("Debug Counter %d (Egress) - VLAN egress filter : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_SPANNING_TRE:
	                diag_util_mprintf("Debug Counter %d (Egress) - Spanning tree : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_PORT_ISOLATION:
	                diag_util_mprintf("Debug Counter %d (Egress) - Port isolation : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_OAM_EGRESS_DROP:
	                diag_util_mprintf("Debug Counter %d (Egress) - OAM : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_MIRROR_ISOLATION:
	                diag_util_mprintf("Debug Counter %d (Egress) - Mirror isolation : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_MAX_LEN_BEFORE_EGR_ACL:
	                diag_util_mprintf("Debug Counter %d (Egress) - Max length(before Egress ACL) : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_SRC_PORT_FILTER_BEFORE_MIRROR:
	                diag_util_mprintf("Debug Counter %d (Egress) - Source port filter(before Mirror) : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_MAX_LEN_BEFORE_MIRROR:
	                diag_util_mprintf("Debug Counter %d (Egress) - Max length(before Mirror) : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_SPECIAL_CONGEST_BEFORE_MIRROR:
	                diag_util_mprintf("Debug Counter %d (Egress) - Special congest(before Mirror) : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_LINK_STATUS_BEFORE_MIRROR:
	                diag_util_mprintf("Debug Counter %d (Egress) - Link status(before Mirror) : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_WRED_BEFORE_MIRROR:
	                diag_util_mprintf("Debug Counter %d (Egress) - WRED(before Mirror) : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_MAX_LEN_AFTER_MIRROR:
	                diag_util_mprintf("Debug Counter %d (Egress) - Max length(after Mirror) : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_SPECIAL_CONGEST_AFTER_MIRROR:
	                diag_util_mprintf("Debug Counter %d (Egress) - Special congest(after Mirror) : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_LINK_STATUS_AFTER_MIRROR:
	                diag_util_mprintf("Debug Counter %d (Egress) - Link status(after Mirror) : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_WRED_AFTER_MIRROR:
	                diag_util_mprintf("Debug Counter %d (Egress) - WRED(after Mirror) : %u \n", idx, cntr);
	                break;
	            default:
	                diag_util_mprintf("Debug Counter %d Usr Config Error !\n", idx);
	                break;
	        }
		 idx++;
			
	    }  
    }
#endif

#if defined(CONFIG_SDK_RTL8380)
    if (DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
    	    for (idx2 = 0; idx2 < RTK_DBG_MIB_DBG_TYPE_END; idx2++)
	    {
	        ret = hal_getDbgCntr(unit, idx2, &cntr);
			
		 if(ret != RT_ERR_OK)
		 	continue;

	        switch (idx2)
	        {
	            case RTK_DBG_MIB_ALE_TX_GOOD_PKTS_RTL8380:
	                diag_util_mprintf("Debug Counter %d - The number of ALE TX good packets : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_MAC_RX_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - MAC RX Drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_ACL_FWD_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - ACL Forward drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_HW_ATTACK_PREVENTION_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - Attack prevention drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_RMA_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - RMA Drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_VLAN_IGR_FLTR_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - VLAN IGR Filter Drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_INNER_OUTER_CFI_EQUAL_1_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - CFI=1 Drop: %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_PORT_MOVE_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - Port Move Drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_NEW_SA_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - NEW SA Drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_MAC_LIMIT_SYS_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - MAC Limit SYS Drop: %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_MAC_LIMIT_VLAN_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - MAC Limit VLAN Drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_MAC_LIMIT_PORT_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - MAC Limit Port Drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_SWITCH_MAC_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - Switch MAC Drop: %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_ROUTING_EXCEPTION_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - Routing Exception Drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_DA_LKMISS_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - DA Lookup Missed Drop: %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_RSPAN_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - RSPAN Drop: %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_ACL_LKMISS_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - Ingress ACL Lookup missed drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_ACL_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - Ingress ACL  drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_INBW_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - Input Bandwidth Control Drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_IGR_METER_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - Ingress Meter Drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_ACCEPT_FRAME_TYPE_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - Accept Frame Type Drop : %u \n", idx, cntr);
	                break;		
	            case RTK_DBG_MIB_STP_IGR_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - STP Ingress Drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_INVALID_SA_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - Invalid SA Drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_SA_BLOCKING_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - SA Blocking Drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_DA_BLOCKING_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - DA Blocking Drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_L2_INVALID_DPM_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - L2 invalid DPM Drop  : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_MCST_INVALID_DPM_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - MAC Constraint Invalid DPM Drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_ROUTE_INVALID_NHP_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - Routing Invalid NHP Drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_STORM_SPPRS_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - Storm Susspresion Drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_LALS_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - Link Aggregation Load sharing Drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_VLAN_EGR_FILTER_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - VLAN egress filter Drop: %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_STP_EGR_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - Egress Spanning tree Drop: %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_SRC_PORT_FILTER_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - Source Port Filter  Drop: %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_PORT_ISOLATION_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - Port Isolation Drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_ACL_FLTR_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - ACL Filter Drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_MIRROR_FLTR_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - Mirror Filter Drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_TX_MAX_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - TX MAX Length Drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_LINK_DOWN_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - Link down Drop : %u \n", idx, cntr);
	                break;
	            case RTK_DBG_MIB_FLOW_CONTROL_DROP_RTL8380:
	                diag_util_mprintf("Debug Counter %d - Flow Control Drop: %u \n", idx, cntr);
	                break;
	            default:
	                diag_util_mprintf("Debug Counter %d Usr Config Error !\n", idx);
	                break;
	        }

		idx++;
			
	    }  
    	}
#endif


    return CPARSER_OK;
}
#endif

#ifdef CMD_DEBUG_GET_FLOWCTRL_USED_PAGE_CNT_EGRESS_INGRESS_PORT_PORTS_ALL
/*
 * debug get flowctrl used-page-cnt ( ingress | egress ) port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_debug_get_flowctrl_used_page_cnt_egress_ingress_port_ports_all(cparser_context_t *context,
    char **ports_ptr)    
{
    uint32      unit=0, port, cntr, maxCntr;
    int32       ret = RT_ERR_FAILED;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 6), ret);
    if ('i' == TOKEN_CHAR(4,0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(hal_getFlowCtrlIgrPortUsedPageCnt(unit, port, &cntr, &maxCntr), ret);
            diag_util_mprintf("Port %2d\n", port);
            diag_util_mprintf("\tIngress Used Page Count : %d\n", cntr);
            diag_util_mprintf("\tIngress Max Used Page Count : %d\n", maxCntr);
        }
    }
    else if ('e' == TOKEN_CHAR(4,0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(hal_getFlowCtrlEgrPortUsedPageCnt(unit, port, &cntr, &maxCntr), ret);
            diag_util_mprintf("Port %2d\n", port);
            diag_util_mprintf("\tEgress Used Page Count : %d\n", cntr);
            diag_util_mprintf("\tEgress Max Used Page Count : %d\n", maxCntr);
        }
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_DEBUG_GET_FLOWCTRL_USED_PAGE_CNT_QUEUE_BASED_SYSTEM
/*
 * debug get flowctrl used-page-cnt system
 */
cparser_result_t cparser_cmd_debug_get_flowctrl_used_page_cnt_system(cparser_context_t *context)
{
    uint32      unit = 0;
    uint32      cntr, maxCntr;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(hal_getFlowCtrlSystemUsedPageCnt(unit, &cntr, &maxCntr), ret);

    diag_util_mprintf("System Used Page Count : %d\n", cntr);
    diag_util_mprintf("System Max Used Page Count : %d\n", maxCntr);

    return CPARSER_OK;
}
#endif

#ifdef CMD_DEBUG_GET_FLOWCTRL_USED_PAGE_CNT_QUEUE_BASED_PORT_PORTS_ALL
/*
 * debug get flowctrl used-page-cnt queue-based port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_debug_get_flowctrl_used_page_cnt_queue_based_port_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit=0, port = 0;
    rtk_qid_t   queue, qid_max;
    int32       ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_dbg_queue_usedPageCnt_t qCntr, qMaxCntr;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    osal_memset(&qCntr, 0, sizeof(qCntr));
    osal_memset(&qMaxCntr, 0, sizeof(qMaxCntr));

    DIAG_OM_GET_CHIP_CAPACITY(unit, qid_max, max_num_of_queue);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 6), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(hal_getFlowCtrlPortQueueUsedPageCnt(unit, port, &qCntr, &qMaxCntr), ret);
        diag_util_mprintf("Port %2d\n", port);
        for (queue = 0; queue < qid_max; queue++)
        {
            diag_util_mprintf("\tQueue %d Used Page Count : %d\n", queue, qCntr.cntr[queue]);
            diag_util_mprintf("\tQueue %d Max Used Page Count : %d\n", queue, qMaxCntr.cntr[queue]);
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_DEBUG_RESET_FLOWCTRL_USED_PAGE_CNT_EGRESS_INGRESS_PORT_PORTS_ALL
/*
 * debug reset flowctrl used-page-cnt ( egress | ingress ) port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_debug_reset_flowctrl_used_page_cnt_egress_ingress_port_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit=0, port;
    int32       ret = RT_ERR_FAILED;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 6), ret);
    if ('i' == TOKEN_CHAR(4,0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(hal_resetFlowCtrlIgrPortUsedPageCnt(unit, port), ret);
        }
    }
    else if ('e' == TOKEN_CHAR(4,0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(hal_resetFlowCtrlEgrPortUsedPageCnt(unit, port), ret);
        }
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_DEBUG_RESET_FLOWCTRL_USED_PAGE_CNT_QUEUE_BASED_SYSTEM
/*
 * debug reset flowctrl used-page-cnt system
 */
cparser_result_t cparser_cmd_debug_reset_flowctrl_used_page_cnt_system(cparser_context_t *context)
{
    uint32      unit;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(hal_resetFlowCtrlSystemUsedPageCnt(unit), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_DEBUG_FLASHTEST_MTD
#define TEST_PATTERN_SIZE 16
char flash_test_pattern[TEST_PATTERN_SIZE+1] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x8, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x0};
char flash_erase_verify_pattern[TEST_PATTERN_SIZE+1] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0};

cparser_result_t cparser_cmd_debug_flashtest_mtd_mtd_idx(cparser_context_t *context, uint32_t *mtd_idx_ptr)
{
    uint32      unit;
	char		test_mtd[32];

    char *buf = NULL;
    char str[16];
    int32 fd = -1, i = 0;
    mtd_info_t mtd_info;
    erase_info_t erase_info;

    osal_memset(str, 0, sizeof(str));

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

	osal_memset(test_mtd, 0, 32);

	sprintf(test_mtd,"/dev/mtdchar%d",(*mtd_idx_ptr)*2);
    if ((fd = open(test_mtd, O_RDONLY)) < 0)
    {
    	diag_util_mprintf("Cannot open MTD device : %s\n", test_mtd);
        return RT_ERR_FAILED;
    }	

    /* Get MTD partition size information */
    if (ioctl(fd, MEMGETINFO, &mtd_info) < 0)
    {
        close(fd);

        return RT_ERR_FAILED;
    }	
	diag_util_mprintf("Target MTD is %s(mtdchar ID should %d * 2), size is 0x%x\n\n", test_mtd, *mtd_idx_ptr, mtd_info.size);

	buf = osal_alloc(TEST_PATTERN_SIZE+1);
	osal_memset(buf, 0, TEST_PATTERN_SIZE+1);

	close(fd);
    if ((fd = open(test_mtd, O_RDWR)) < 0)
    {
    	close(fd);
    	diag_util_mprintf("Cannot open MTD device : %s\n", test_mtd);
        return RT_ERR_FAILED;
    }	

	/* Read flash*/
    if (read(fd, buf, TEST_PATTERN_SIZE) < 0)
    {
    	diag_util_mprintf("Cannot read MTD device : %s\n", test_mtd);
        close(fd);
        osal_free(buf); 
        return RT_ERR_FAILED;
    }

	close(fd);

	diag_util_mprintf_init();
	diag_util_mprintf("[0] Read back pattern : ");

	for(i = 0; i < TEST_PATTERN_SIZE; i++)
	{
		if((i%8) == 0)
			diag_util_mprintf("\n");
		diag_util_mprintf(" 0x%02x", (uint8)*(buf+i));
	}	
	diag_util_mprintf("\n\n");
	diag_util_mprintf_init();


    if ((fd = open(test_mtd, O_RDWR)) < 0)
    {
    	diag_util_mprintf("Cannot open MTD device : %s\n", test_mtd);
        return RT_ERR_FAILED;
    }	

	/* Erase flash */
	 erase_info.start = 0x0;
	 erase_info.length = mtd_info.size;
	 if (ioctl(fd, MEMERASE, &erase_info) < 0)
	 {
    	 diag_util_mprintf("Cannot erase MTD device : %s\n", test_mtd);
		 osal_free(buf);
		 close(fd);
	
		 return RT_ERR_FAILED;
	 }
	/* Read flash*/
    if (read(fd, buf, TEST_PATTERN_SIZE) < 0)
    {
    	diag_util_mprintf("Cannot read MTD device : %s\n", test_mtd);
        close(fd);
        osal_free(buf); 
        return RT_ERR_FAILED;
    }

	close(fd);
	if(osal_memcmp(buf, flash_erase_verify_pattern, TEST_PATTERN_SIZE) != 0)
		diag_util_mprintf("[1] Read back pattern : (ERASE Failed!!!)");
	else
		diag_util_mprintf("[1] Read back pattern : (ERASE Success!!)");

	for(i = 0; i < TEST_PATTERN_SIZE; i++)
	{
		if((i%8) == 0)
			diag_util_mprintf("\n");
		diag_util_mprintf(" 0x%02x", (uint8)*(buf+i));
	}
	diag_util_mprintf("\n\n");
	diag_util_mprintf_init();
	
	/* Re-open*/
    if ((fd = open(test_mtd, O_RDWR)) < 0)
    {
    	diag_util_mprintf("Cannot open MTD device : %s\n", test_mtd);
        return RT_ERR_FAILED;
    }
	
    /* Write test pattern into MTD partition, make sure write function */
    if (write(fd, flash_test_pattern, TEST_PATTERN_SIZE) < 0)
    {
		diag_util_mprintf("Cannot write MTD device : %s\n", test_mtd);    
        osal_free(buf);
        close(fd);
        return RT_ERR_FAILED;
    }
	
	close(fd);
    if ((fd = open(test_mtd, O_RDWR)) < 0)
    {
    	diag_util_mprintf("Cannot open MTD device : %s\n", test_mtd);
        return RT_ERR_FAILED;
    }
	/* Read Back Test pattern from flash*/
    if (read(fd, buf, TEST_PATTERN_SIZE) < 0)
    {
    	diag_util_mprintf("Cannot read MTD device : %s\n", test_mtd);
        close(fd);
        osal_free(buf);
        return RT_ERR_FAILED;
    }
	 	
	if(osal_memcmp(buf, flash_test_pattern, TEST_PATTERN_SIZE) != 0)
		diag_util_mprintf("[2] Read back pattern : (WRITE Failed!!!)");
	else
		diag_util_mprintf("[2] Read back pattern : (WRITE Success!!)");

	for(i = 0; i < TEST_PATTERN_SIZE; i++)
	{
		if((i%8) == 0)
			diag_util_mprintf("\n");
		diag_util_mprintf(" 0x%02x", (uint8)*(buf+i));
	}
	diag_util_mprintf("\n\n");
	diag_util_mprintf_init();
	
	/* Erase Flash, and make sure Erase function*/
	if (ioctl(fd, MEMERASE, &erase_info) < 0)
	{
    	 diag_util_mprintf("Cannot erase MTD device : %s\n", test_mtd);
		 osal_free(buf);
		 close(fd);
	
		 return RT_ERR_FAILED;
	}
	
	close(fd);
    if ((fd = open(test_mtd, O_RDWR)) < 0)
    {
    	diag_util_mprintf("Cannot open MTD device : %s\n", test_mtd);
        return RT_ERR_FAILED;
    }
	
    if (read(fd, buf, TEST_PATTERN_SIZE) < 0)
    {
    	diag_util_mprintf("Cannot read MTD device : %s\n", test_mtd);
        close(fd);
        osal_free(buf);
        return RT_ERR_FAILED;
    }
	 	
	if(osal_memcmp(buf, flash_erase_verify_pattern, TEST_PATTERN_SIZE) != 0)
		diag_util_mprintf("[3] Read back pattern : (ERASE Failed!!!)");
	else
		diag_util_mprintf("[3] Read back pattern : (ERASE Success!!)");

	for(i = 0; i < TEST_PATTERN_SIZE; i++)
	{
		if((i%8) == 0)
			diag_util_mprintf("\n");	
		diag_util_mprintf(" 0x%02x", (uint8)*(buf+i));
	}
	diag_util_mprintf("\n\n");
	diag_util_mprintf_init();

    close(fd);
    osal_free(buf);

	return CPARSER_OK;

}

#endif


int32 _diag_flowCtrl_dump(void)
{
#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
    uint32      unit = 0, cntr, maxCntr;
    rtk_qid_t   queue, qid_max;
    rtk_dbg_queue_usedPageCnt_t qCntr, qMaxCntr;
    rtk_port_t  port;
    int32       ret = RT_ERR_FAILED;
    rtk_switch_devInfo_t devInfo;

    memset(&devInfo, 0, sizeof(rtk_switch_devInfo_t));
    if (rtk_switch_deviceInfo_get(unit, &devInfo) != RT_ERR_OK)
    {
        return RT_ERR_FAILED;
    }

    DIAG_UTIL_ERR_CHK(hal_getFlowCtrlSystemUsedPageCnt(unit, &cntr, &maxCntr), ret);
    diag_util_mprintf("\n===================== Flow Control Used Page Info =============================\n");
    diag_util_mprintf("\n");
    diag_util_mprintf("System Used Page Count : %d\n", cntr);
    diag_util_mprintf("System Max Used Page Count : %d\n\n", maxCntr);

    osal_memset(&qCntr, 0, sizeof(qCntr));
    osal_memset(&qMaxCntr, 0, sizeof(qMaxCntr));
    qid_max = devInfo.capacityInfo.max_num_of_queue;

    for (port = devInfo.all.min; port <= devInfo.all.max; port++)
    {
        if (RTK_PORTMASK_IS_PORT_SET((devInfo.all.portmask), port))
        { 
            hal_getFlowCtrlPortQueueUsedPageCnt(unit, port, &qCntr, &qMaxCntr);
            diag_util_mprintf("Port %2d\n", port);
            for (queue = 0; queue < qid_max; queue++)
            {
                diag_util_mprintf("\tQueue %d Used Page Count : %d\n", queue, qCntr.cntr[queue]);
                diag_util_mprintf("\tQueue %d Max Used Page Count : %d\n", queue, qMaxCntr.cntr[queue]);
            }
        }
    }
    diag_util_mprintf("===============================================================================\n");
#endif
    return RT_ERR_OK;
}

/* print all chip debug informations */
int32 diag_debug_dump(void)
{
    uint32      unit = 0;
#if defined(CONFIG_SDK_RTL8328)
    int32       i = 0;
#endif
    char        cmd[64];

    /* Clear kernel message command */
    osal_memset(cmd, 0, sizeof(cmd));
    osal_sprintf(cmd, "dmesg -c > /dev/null");

    /* To prevent always print message on console and backup printk config */
    system("cp /proc/sys/kernel/printk /tmp");
    system("echo \"4       4       1       7\" > /proc/sys/kernel/printk");

#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
    system(cmd);
    hal_dumpHsa(unit);
    system("dmesg");
#endif

    system(cmd);
    hal_dumpHsb(unit);
    system("dmesg");
    
#if defined(CONFIG_SDK_RTL8390)
    system(cmd);
    hal_dumpHsm(unit);
    system("dmesg");
#endif

#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
    _diag_flowCtrl_dump();
#endif

#if defined(CONFIG_SDK_RTL8328)
    system(cmd);
    hal_dumpPmi(unit);
    system("dmesg");
    
    for (i = 0; i < 7; i++)
    {
        system(cmd);
        hal_dumpPpi(unit, i);
        system("dmesg");
    }
#endif

    /* Recovery printk config to old one */
    system("cp /tmp/printk /proc/sys/kernel/printk;rm /tmp/printk");

    return RT_ERR_OK;
}
