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
 * $Revision: 27298 $
 * $Date: 2012-03-16 13:33:08 +0800 (Fri, 16 Mar 2012) $
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
    DIAG_UTIL_ERR_CHK(hal_dumpHsmIdx(unit, *index_ptr), ret);
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
        DIAG_UTIL_ERR_CHK(hal_dumpPpi(unit, *index_ptr), ret);
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
    
    for (loop = 0; loop < 20; loop++)
    {
        diag_util_printf("%x-", value[loop]);
    }
    
    diag_util_mprintf("\n");
    
    return CPARSER_OK;
}
#endif
