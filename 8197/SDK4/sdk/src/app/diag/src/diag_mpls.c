/*
 * Copyright (C) 2011 Realtek Semiconductor Corp.
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
 * Purpose : Definition those MPLS command and APIs in the SDK diagnostic shell.
 *
 * Feature : The file have include the following module and sub-modules
 *              1) MPLS
 */

/*
 * Include Files
 */
#include <common/debug/rt_log.h>
#include <common/rt_error.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_util.h>

#include <rtk/mpls.h>
#include <diag_util.h>
#include <diag_om.h>
#include <parser/cparser_priv.h>

/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */


/*
 * Macro Declaration
 */


/*
 * Function Declaration
 */


/* Module Name : MPLS */
#ifdef CMD_MPLS_GET_ENCAP_TTL_INHERIT
/*
 * mpls get encap ttl inherit
 */
cparser_result_t
cparser_cmd_mpls_get_encap_ttl_inherit(cparser_context_t *context)
{
    uint32                  unit = 0;
    int32                   ret;
    rtk_mpls_ttlInherit_t   inherit;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_mpls_ttlInherit_get(unit, &inherit);
    if (ret != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("\tMPLS TTL inherit: ");
    if (RTK_MPLS_TTL_INHERIT_UNIFORM == inherit)
        diag_util_mprintf("Uniform\n");
    else if(RTK_MPLS_TTL_INHERIT_PIPE == inherit)
        diag_util_mprintf("Pipe\n");

    return CPARSER_OK;
}   /* end of cparser_cmd_mpls_get_encap_ttl_inherit */
#endif  /* CPARSER_CMD_MPLS_GET_ENCAP_TTL_INHERIT */

#ifdef CMD_MPLS_SET_ENCAP_TTL_INHERIT_UNIFORM_PIPE
/*
 * mpls set encap ttl inherit ( uniform | pipe )
 */
cparser_result_t
cparser_cmd_mpls_set_encap_ttl_inherit_uniform_pipe(cparser_context_t *context)
{
    uint32                  unit = 0;
    int32                   ret;
    rtk_mpls_ttlInherit_t   inherit;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    if('u' == TOKEN_CHAR(5, 0))
        inherit = RTK_MPLS_TTL_INHERIT_UNIFORM;
    else if('p' == TOKEN_CHAR(5, 0))
        inherit = RTK_MPLS_TTL_INHERIT_PIPE;
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_mpls_ttlInherit_set(unit, inherit), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_mpls_set_encap_ttl_inherit_uniform_pipe */
#endif  /* CPARSER_CMD_MPLS_SET_ENCAP_TTL_INHERIT_UNIFORM_PIPE */

#ifdef CMD_MPLS_GET_ENCAP_LIB_INDEX
/*
 * mpls get encap lib <UINT:index>
 */
cparser_result_t
cparser_cmd_mpls_get_encap_lib_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32              unit = 0;
    int32               ret;
    rtk_mpls_encap_t    info;

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((NULL == index_ptr), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();

    info.oper = RTK_MPLS_LABEL_OPER_SINGLE;

    ret = rtk_mpls_encap_get(unit, *index_ptr, &info);
    if (ret != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("\tMPLS LIB %d ", *index_ptr);
    diag_util_mprintf("inner information: \n");
    diag_util_mprintf("\t\tLable:%d \n", info.label0);
    diag_util_mprintf("\t\tExp:%d \n", info.exp0);
    diag_util_mprintf("\t\tTTL:%d \n", info.ttl0);

    return CPARSER_OK;
}   /* end of cparser_cmd_mpls_get_encap_lib_index */
#endif  /* CPARSER_CMD_MPLS_GET_ENCAP_LIB_INDEX */

#ifdef CMD_MPLS_SET_ENCAP_LIB_INDEX_LABEL_EXP_TTL
/*
 * mpls set encap lib <UINT:index> <UINT:label> <UINT:exp> <UINT:ttl>
 */
cparser_result_t
cparser_cmd_mpls_set_encap_lib_index_label_exp_ttl(
    cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *label_ptr,
    uint32_t *exp_ptr,
    uint32_t *ttl_ptr)
{
    uint32              unit = 0;
    int32               ret;
    rtk_mpls_encap_t    info;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    info.oper = RTK_MPLS_LABEL_OPER_SINGLE;
    info.label0 = *label_ptr;
    info.exp0   = *exp_ptr;
    info.ttl0   = *ttl_ptr;

    DIAG_UTIL_ERR_CHK(rtk_mpls_encap_set(unit, *index_ptr, &info), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_mpls_set_encap_lib_index_label_exp_ttl */
#endif  /* CPARSER_CMD_MPLS_SET_ENCAP_LIB_INDEX_LABEL_EXP_TTL */

#ifdef CMD_MPLS_GET_STATE
/*
 * mpls get state
 */
cparser_result_t
cparser_cmd_mpls_get_state(
    cparser_context_t *context)
{
    uint32          unit;
    int32           ret;
    rtk_enable_t    enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_mpls_enable_get(unit, &enable);
    if (ret != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("MPLS decapsulation state: ");
    if (ENABLED == enable)
        diag_util_mprintf("Enable\n");
    else
        diag_util_mprintf("Disable\n");

    return CPARSER_OK;
}   /* end of cparser_cmd_mpls_get_state */
#endif

#ifdef CMD_MPLS_SET_STATE_ENABLE_DISABLE
/*
 * mpls set state ( enable | disable )
 */
cparser_result_t
cparser_cmd_mpls_set_state_enable_disable(
    cparser_context_t *context)
{
    uint32          unit;
    int32           ret;
    rtk_enable_t    enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('e' == TOKEN_CHAR(3, 0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    DIAG_UTIL_ERR_CHK(rtk_mpls_enable_set(unit, enable), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_mpls_set_state_enable_disable */
#endif
