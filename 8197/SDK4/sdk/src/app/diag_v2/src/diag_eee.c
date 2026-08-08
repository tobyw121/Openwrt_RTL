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
 * $Revision: 39788 $
 * $Date: 2013-05-28 17:24:58 +0800 (Tue, 28 May 2013) $
 *
 * Purpose : Definition those EEE command and APIs in the SDK diagnostic shell.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) EEE enable/disable
 *           2) EEE wake up and sleep mode select
 *           3) Parameter for wake up and sleep mode
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
#include <rtk/switch.h>
#include <rtk/port.h>
#include <rtk/eee.h>
#include <diag_util.h>
#include <diag_om.h>
#include <parser/cparser_priv.h>


#ifdef CMD_EEE_GET_PORT_PORTS_ALL_STATE
/*
 * eee get port ( <PORT_LIST:ports> | all ) state
 */
cparser_result_t cparser_cmd_eee_get_port_ports_all_state(cparser_context_t *context,
    char **ports_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_port_t          port = 0;
    rtk_enable_t        enabled = DISABLED;
    diag_portlist_t     portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    /* show port info */
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("Port %d:\n", port);
        diag_util_printf("    EEE : ");
        DIAG_UTIL_ERR_CHK(rtk_eee_portEnable_get(unit, port, &enabled), ret);
        if (ENABLED == enabled)
        {
            diag_util_mprintf("enable\n");
        }
        else
        {
            diag_util_mprintf("disable\n");
        }
    }

    return CPARSER_OK;
} /* end of cparser_cmd_eee_get_port_ports_all_state */
#endif /* CMD_EEE_GET_PORT_PORTS_ALL_STATE */

#ifdef CMD_EEE_SET_PORT_PORTS_ALL_STATE_DISABLE_ENABLE
/*
 * eee set port ( <PORT_LIST:ports> | all ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_eee_set_port_ports_all_state_disable_enable(cparser_context_t *context,
    char **ports_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    rtk_enable_t    enable = DISABLED;
    diag_portlist_t portlist;
    rtk_switch_devInfo_t devInfo;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('e' == TOKEN_CHAR(5,0))
    {
        enable = ENABLED;
    }
    else if ('d' == TOKEN_CHAR(5,0))
    {
        enable = DISABLED;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_switch_deviceInfo_get(unit, &devInfo), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_eee_portEnable_set(unit, port, enable), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_eee_set_port_ports_all_state_disable_enable */
#endif /* CMD_EEE_SET_PORT_PORTS_ALL_STATE_DISABLE_ENABLE */

#ifdef CMD_EEEP_GET_PORT_PORTS_ALL_STATE
/*
 * eeep get port ( <PORT_LIST:ports> | all ) state
 */
cparser_result_t cparser_cmd_eeep_get_port_ports_all_state(cparser_context_t *context,
    char **ports_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_port_t          port = 0;
    rtk_enable_t        enabled = DISABLED;
    diag_portlist_t     portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    /* show port info */
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("Port %d:\n", port);
        diag_util_printf("    EEEP : ");
        DIAG_UTIL_ERR_CHK(rtk_eeep_portEnable_get(unit, port, &enabled), ret);
        if (ENABLED == enabled)
        {
            diag_util_mprintf("enable\n");
        }
        else
        {
            diag_util_mprintf("disable\n");
        }
    }

    return CPARSER_OK;
} /* end of cparser_cmd_eeep_get_port_ports_all_state */
#endif /* CMD_EEEP_GET_PORT_PORTS_ALL_STATE */

#ifdef CMD_EEEP_SET_PORT_PORTS_ALL_STATE_DISABLE_ENABLE
/*
 * eeep set port ( <PORT_LIST:ports> | all ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_eeep_set_port_ports_all_state_disable_enable(cparser_context_t *context,
    char **ports_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    rtk_enable_t    enable = DISABLED;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('e' == TOKEN_CHAR(5,0))
    {
        enable = ENABLED;
    }
    else if ('d' == TOKEN_CHAR(5,0))
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
        DIAG_UTIL_ERR_CHK(rtk_eeep_portEnable_set(unit, port, enable), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_eeep_set_port_ports_all_state_disable_enable */
#endif /* CMD_EEEP_SET_PORT_PORTS_ALL_STATE_DISABLE_ENABLE */

