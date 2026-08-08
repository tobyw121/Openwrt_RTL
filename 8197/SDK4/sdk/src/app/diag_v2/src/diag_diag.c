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
 * $Revision: 51430 $
 * $Date: 2014-09-18 16:02:46 +0800 (Thu, 18 Sep 2014) $
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
#include <ioal/ioal_init.h>
#include <ioal/mem32.h>
#include <rtk/diag.h>
#include <rtk/port.h>
#include <diag_util.h>
#include <diag_om.h>
#include <parser/cparser_priv.h>

#define PORT_NUM_IN_8218B   8

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

#ifdef CMD_DIAG_GET_CABLE_DOCTOR_PORT_PORTS_ALL
/*
 * diag get cable-doctor port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_diag_get_cable_doctor_port_ports_all(cparser_context_t *context,
    char **ports_ptr)
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
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        memset(&rtctResult, 0, sizeof(rtk_rtctResult_t));
        if ((ret = rtk_diag_portRtctResult_get(unit, port, &rtctResult)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

        if (RTK_PORTMASK_IS_PORT_SET(devInfo.ge.portmask, port))
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

#ifdef CMD_DIAG_SET_CABLE_DOCTOR_PORT_PORTS_ALL_START
/*
 * diag set cable-doctor port ( <PORT_LIST:ports> | all ) start
 */
cparser_result_t cparser_cmd_diag_set_cable_doctor_port_ports_all_start(cparser_context_t *context,
    char **ports_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    diag_portlist_t               portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_ERR_CHK(rtk_diag_rtctEnable_set(unit, &portlist.portmask), ret);

    return CPARSER_OK;
}
#endif

/*
 * diag dump command
 */
#ifdef CMD_DIAG_DUMP_TABLE_INDEX
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
#endif

#ifdef CMD_DIAG_DUMP_REGISTER
cparser_result_t cparser_cmd_diag_dump_register(cparser_context_t *context)
{
    int32 return_value;
    uint32 unit;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('r' == TOKEN_CHAR(2,0))
    {
	DIAG_UTIL_ERR_CHK(rtk_diag_peripheral_register_dump(unit), return_value);
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;

}
#endif

#ifdef CMD_DIAG_WHOLEDUMP_MAC_REG_PHY_REG_SOC_REG_TABLE_ALL
cparser_result_t cparser_cmd_diag_wholedump_mac_reg_phy_reg_soc_reg_table_all(cparser_context_t *context)
{
	int32 return_value;
	uint32 unit;

	DIAG_UTIL_PARAM_CHK();
	DIAG_OM_GET_CHIP_ID(unit);

	if ('s' == TOKEN_CHAR(2,0))		/*Dump SoC registers*/
	{
			DIAG_UTIL_ERR_CHK(rtk_diag_peripheral_register_dump(unit), return_value);
	}
	else if('m' == TOKEN_CHAR(2,0))	/*Dump MAC registers*/
	{
		  DIAG_UTIL_ERR_CHK(rtk_diag_reg_whole_read(unit), return_value);
	}
	else if('t'== TOKEN_CHAR(2,0))	/*Dump MAC tables*/
	{
		  DIAG_UTIL_ERR_CHK(rtk_diag_table_whole_read(unit, 0xff), return_value);
	}
	else if('p'== TOKEN_CHAR(2,0)) /*Dump PHY registers*/
	{
		  DIAG_UTIL_ERR_CHK(rtk_diag_phy_reg_whole_read(unit), return_value);
	}
	else if('a'== TOKEN_CHAR(2,0)) /*Dump All*/
	{
    	  DIAG_UTIL_ERR_CHK(rtk_diag_peripheral_register_dump(unit), return_value);
		  DIAG_UTIL_ERR_CHK(rtk_diag_reg_whole_read(unit), return_value);
		  DIAG_UTIL_ERR_CHK(rtk_diag_phy_reg_whole_read(unit), return_value);
		  DIAG_UTIL_ERR_CHK(rtk_diag_table_whole_read(unit, 0xff), return_value);
	}
	else
	{
		diag_util_printf("User config: Error!\n");
		return CPARSER_NOT_OK;
	}

	return CPARSER_OK;
}
#endif

#ifdef CMD_DIAG_GET_SERDES_SDSID_LINK_STATUS
/*
 * diag get serdes <UINT:sdsId> link-status
 */
cparser_result_t
cparser_cmd_diag_get_serdes_sdsId_link_status(
    cparser_context_t *context,
    uint32_t *sdsId_ptr)
{
    uint32  unit, val;
    int32   ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        uint32  addr;

        if ((*sdsId_ptr) < 0 || (*sdsId_ptr) > 13)
            return CPARSER_NOT_OK;

        addr = 0xa07c + (((*sdsId_ptr) / 2) * 0x400);
        if ((*sdsId_ptr) % 2 != 0)
            addr += 0x100;

        DIAG_UTIL_ERR_CHK(ioal_mem32_read(unit, addr, &val), ret);
        diag_util_mprintf("sds %d status: 0x%x\n", *sdsId_ptr, val);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_diag_get_serdes_sdsId_link_status */
#endif

#ifdef CMD_DIAG_RESET_SERDES_SDSID
/*
 * diag reset serdes <UINT:sdsId>
 */
cparser_result_t
cparser_cmd_diag_reset_serdes_sdsId(
    cparser_context_t *context,
    uint32_t *sdsId_ptr)
{
    uint32  unit, val;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        uint32  addr, ofst;

        ofst = ((*sdsId_ptr) / 2) * 0x400;

        switch ((*sdsId_ptr))
        {
            case 0 ... 7:
            case 10 ... 11:
                addr = 0xa3c0 + ofst;
                ioal_mem32_read(unit, addr, &val);
                val |= ((1 << 20) | (1 << 22));
                val &= ~((1 << 21) | (1 << 23));
                ioal_mem32_write(unit, addr, val);
                val |= (0xf << 20);
                ioal_mem32_write(unit, addr, val);
                val &= ~(0xf << 20);
                ioal_mem32_write(unit, addr, val);

                addr = 0xa340 + ofst;
                ioal_mem32_read(unit, addr, &val);
                val |= ((1 << 24) | (1 << 26));
                val &= ~((1 << 25) | (1 << 27));
                ioal_mem32_write(unit, addr, val);
                val |= (0xf << 24);
                ioal_mem32_write(unit, addr, val);
                val &= ~(0xf << 24);
                ioal_mem32_write(unit, addr, val);
                break;
            case 8 ... 9:
                addr = 0xb3f8;
                ioal_mem32_read(unit, addr, &val);
                val |= ((1 << 16) | (1 << 18));
                val &= ~((1 << 17) | (1 << 19));
                ioal_mem32_write(unit, addr, val);
                val |= (0xf << 16);
                ioal_mem32_write(unit, addr, val);
                val &= ~(0xf << 16);
                ioal_mem32_write(unit, addr, val);

                val |= ((1 << 24) | (1 << 26));
                val &= ~((1 << 25) | (1 << 27));
                ioal_mem32_write(unit, addr, val);
                val |= (0xf << 24);
                ioal_mem32_write(unit, addr, val);
                val &= ~(0xf << 24);
                ioal_mem32_write(unit, addr, val);
                break;
            case 12 ... 13:
                addr = 0xbbf8;
                ioal_mem32_read(unit, addr, &val);
                val |= ((1 << 16) | (1 << 18));
                val &= ~((1 << 17) | (1 << 19));
                ioal_mem32_write(unit, addr, val);
                val |= (0xf << 16);
                ioal_mem32_write(unit, addr, val);
                val &= ~(0xf << 16);
                ioal_mem32_write(unit, addr, val);

                val |= ((1 << 24) | (1 << 26));
                val &= ~((1 << 25) | (1 << 27));
                ioal_mem32_write(unit, addr, val);
                val |= (0xf << 24);
                ioal_mem32_write(unit, addr, val);
                val &= ~(0xf << 24);
                ioal_mem32_write(unit, addr, val);
                break;
            default:
                return CPARSER_NOT_OK;
        }

        addr = 0xa004 + ofst;
        ioal_mem32_read(unit, addr, &val);
        val &= ~(0xFFFF << 16);
        val |= (0x7146 << 16);
        ioal_mem32_write(unit, addr, val);
        val &= ~(0xFFFF << 16);
        val |= (0x7106 << 16);
        ioal_mem32_write(unit, addr, val);

        addr = 0xa004 + ofst + 0x100;
        ioal_mem32_read(unit, addr, &val);
        val &= ~(0xFFFF << 16);
        val |= (0x7146 << 16);
        ioal_mem32_write(unit, addr, val);
        val &= ~(0xFFFF << 16);
        val |= (0x7106 << 16);
        ioal_mem32_write(unit, addr, val);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_diag_reset_serdes_sdsId */
#endif

#ifdef CMD_DIAG_GET_SERDES_SDSID_RX_SYM_ERR
/*
 * diag get serdes <UINT:sdsId> rx-sym-err
 */
cparser_result_t
cparser_cmd_diag_get_serdes_sdsId_rx_sym_err(
    cparser_context_t *context,
    uint32_t *sdsId_ptr)
{
    uint32  unit, val;
    int32   ret;
    int8    i;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        uint32  addr;

        if ((*sdsId_ptr) < 0 || (*sdsId_ptr) > 13)
            return CPARSER_NOT_OK;

        diag_util_mprintf("sds %d\n", *sdsId_ptr);
        addr = 0xa070 + (((*sdsId_ptr) / 2) * 0x400);
        if ((*sdsId_ptr) % 2 != 0)
            addr += 0x100;

        for (i = 0x10; i <= 0x13; ++i)
        {
            DIAG_UTIL_ERR_CHK(ioal_mem32_write(unit, addr, i), ret);
            DIAG_UTIL_ERR_CHK(ioal_mem32_read(unit, addr, &val), ret);
            diag_util_mprintf(" CH%d: 0x%x\n", i - 0x10, val >> 16);
        }
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_diag_get_serdes_sdsId_rx_sym_err */
#endif

#ifdef CMD_DIAG_GET_PHY_PHYID_SERDES_LINK_STATUS
/*
 * diag get phy <UINT:phyId> serdes link-status
 */
cparser_result_t
cparser_cmd_diag_get_phy_phyId_serdes_link_status(
    cparser_context_t *context,
    uint32_t *phyId_ptr)
{
    rtk_switch_devInfo_t    devInfo;
    rtk_port_t              port;
    uint32                  unit;
    uint32                  maxPage = 0x1fff, regVal, oriReg;
    uint32                  sdsPage[] = {0x40f, 0x42f};
    int32                   ret;
    uint8                   i;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    memset(&devInfo, 0, sizeof(rtk_switch_devInfo_t));
    DIAG_UTIL_ERR_CHK(rtk_switch_deviceInfo_get(unit, &devInfo), ret);

    port = (*phyId_ptr) * PORT_NUM_IN_8218B;

    if (RTK_PORTMASK_IS_PORT_SET(devInfo.serdes.portmask, port))
        return CPARSER_OK;

    if (!RTK_PORTMASK_IS_PORT_SET(devInfo.ether.portmask, port))
        return CPARSER_OK;

    DIAG_UTIL_ERR_CHK(rtk_port_phyReg_get(unit, port, 0, 3, &regVal), ret);
    if (regVal == 0xC981)
    {
        diag_util_mprintf("PHY Port ID: %d\n", port);
        DIAG_UTIL_ERR_CHK(rtk_port_phyReg_get(unit, port, maxPage, 30, &oriReg), ret);
        DIAG_UTIL_ERR_CHK(rtk_port_phyReg_set(unit, port, maxPage, 30, 8), ret);

        for (i = 0; i < sizeof(sdsPage)/sizeof(uint32); ++i)
        {
            if ((ret = rtk_port_phyReg_get(unit, port, sdsPage[i], 0x16, &regVal)) != RT_ERR_OK)
            {
                rtk_port_phyReg_set(unit, port, maxPage, 30, oriReg);
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }
            diag_util_mprintf(" status %d: 0x%04x\n", i, regVal);
        }

        DIAG_UTIL_ERR_CHK(rtk_port_phyReg_set(unit, port, maxPage, 30, oriReg), ret);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_diag_get_phy_phyId_serdes_link_status */
#endif

#ifdef CMD_DIAG_GET_PHY_PHYID_SERDES_RX_SYM_ERR
/*
 * diag get phy <UINT:phyId> serdes rx-sym-err
 */
cparser_result_t
cparser_cmd_diag_get_phy_phyId_serdes_rx_sym_err(
    cparser_context_t *context,
    uint32_t *phyId_ptr)
{
    rtk_switch_devInfo_t    devInfo;
    rtk_port_t              port;
    uint32                  unit;
    uint32                  maxPage = 0x1fff, regVal, oriReg;
    uint32                  sdsPage[] = {0x40f, 0x42f};
    int32                   ret;
    uint8                   i, j;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    memset(&devInfo, 0, sizeof(rtk_switch_devInfo_t));
    DIAG_UTIL_ERR_CHK(rtk_switch_deviceInfo_get(unit, &devInfo), ret);

    port = (*phyId_ptr) * PORT_NUM_IN_8218B;

    if (RTK_PORTMASK_IS_PORT_SET(devInfo.serdes.portmask, port))
        return CPARSER_OK;

    if (!RTK_PORTMASK_IS_PORT_SET(devInfo.ether.portmask, port))
        return CPARSER_OK;

    DIAG_UTIL_ERR_CHK(rtk_port_phyReg_get(unit, port, 0, 3, &regVal), ret);
    if (regVal == 0xC981)
    {
        diag_util_mprintf("PHY Port ID: %d\n", port);
        DIAG_UTIL_ERR_CHK(rtk_port_phyReg_get(unit, port, maxPage, 30, &oriReg), ret);
        DIAG_UTIL_ERR_CHK(rtk_port_phyReg_set(unit, port, maxPage, 30, 8), ret);

        for (i = 0; i < sizeof(sdsPage)/sizeof(uint32); ++i)
        {
            diag_util_mprintf(" Sds ID: %d\n", i);

            for (j = 0x10; j <= 0x13; ++j)
            {
                if ((ret = rtk_port_phyReg_set(unit, port, sdsPage[i], 0x10, j)) != RT_ERR_OK)
                {
                    rtk_port_phyReg_set(unit, port, maxPage, 30, oriReg);
                    DIAG_ERR_PRINT(ret);
                    return CPARSER_NOT_OK;
                }

                if ((ret = rtk_port_phyReg_get(unit, port, sdsPage[i], 0x11, &regVal)) != RT_ERR_OK)
                {
                    rtk_port_phyReg_set(unit, port, maxPage, 30, oriReg);
                    DIAG_ERR_PRINT(ret);
                    return CPARSER_NOT_OK;
                }
                diag_util_mprintf("  CH%d: 0x%04x\n", (j - 0x10), regVal);
            }
        }

        DIAG_UTIL_ERR_CHK(rtk_port_phyReg_set(unit, port, maxPage, 30, oriReg), ret);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_diag_get_phy_phyId_serdes_rx_sym_err */
#endif

#ifdef CMD_DIAG_CLEAR_PHY_PHYID_SERDES_RX_SYM_ERR
/*
 * diag clear phy <UINT:phyId> serdes rx-sym-err
 */
cparser_result_t
cparser_cmd_diag_clear_phy_phyId_serdes_rx_sym_err(
    cparser_context_t *context,
    uint32_t *phyId_ptr)
{
    rtk_switch_devInfo_t    devInfo;
    rtk_port_t              port;
    uint32                  unit;
    uint32                  maxPage = 0x1fff, regVal, oriReg;
    uint32                  sdsPage[] = {0x40f, 0x42f};
    int32                   ret;
    uint8                   i, j;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&devInfo, 0, sizeof(rtk_switch_devInfo_t));
    DIAG_UTIL_ERR_CHK(rtk_switch_deviceInfo_get(unit, &devInfo), ret);

    port = (*phyId_ptr) * PORT_NUM_IN_8218B;

    if (RTK_PORTMASK_IS_PORT_SET(devInfo.serdes.portmask, port))
        return CPARSER_OK;

    if (!RTK_PORTMASK_IS_PORT_SET(devInfo.ether.portmask, port))
        return CPARSER_OK;

    DIAG_UTIL_ERR_CHK(rtk_port_phyReg_get(unit, port, 0, 3, &regVal), ret);
    if (regVal == 0xC981)
    {
        DIAG_UTIL_ERR_CHK(rtk_port_phyReg_get(unit, port, maxPage, 30, &oriReg), ret);
        DIAG_UTIL_ERR_CHK(rtk_port_phyReg_set(unit, port, maxPage, 30, 8), ret);

        for (i = 0; i < sizeof(sdsPage)/sizeof(uint32); ++i)
        {
            for (j = 0x10; j <= 0x13; ++j)
            {
                if ((ret = rtk_port_phyReg_set(unit, port, sdsPage[i], 0x10, j)) != RT_ERR_OK)
                {
                    rtk_port_phyReg_set(unit, port, maxPage, 30, oriReg);
                    DIAG_ERR_PRINT(ret);
                    return CPARSER_NOT_OK;
                }

                if ((ret = rtk_port_phyReg_get(unit, port, sdsPage[i], 0x11, &regVal)) != RT_ERR_OK)
                {
                    rtk_port_phyReg_set(unit, port, maxPage, 30, oriReg);
                    DIAG_ERR_PRINT(ret);
                    return CPARSER_NOT_OK;
                }
            }
        }

        DIAG_UTIL_ERR_CHK(rtk_port_phyReg_set(unit, port, maxPage, 30, oriReg), ret);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_diag_clear_phy_phyId_serdes_rx_sym_err */
#endif

#ifdef CMD_DIAG_GET_PHY_PORT_PORTS_ALL_RX_CNT
/*
 * diag get phy port ( <PORT_LIST:ports> | all ) rx-cnt
 */
cparser_result_t
cparser_cmd_diag_get_phy_port_ports_all_rx_cnt(
    cparser_context_t *context,
    char **ports_ptr)
{
    rtk_switch_devInfo_t    devInfo;
    diag_portlist_t         portlist;
    rtk_port_t              port;
    uint32                  unit;
    uint32                  maxPage = 0x1fff, regVal, oriReg;
    int32                   ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    memset(&devInfo, 0, sizeof(rtk_switch_devInfo_t));
    DIAG_UTIL_ERR_CHK(rtk_switch_deviceInfo_get(unit, &devInfo), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if (RTK_PORTMASK_IS_PORT_SET(devInfo.serdes.portmask, port))
            continue;

        if (!RTK_PORTMASK_IS_PORT_SET(devInfo.ether.portmask, port))
            continue;

        DIAG_UTIL_ERR_CHK(rtk_port_phyReg_get(unit, port, 0, 3, &regVal), ret);
        if (regVal == 0xC981)
        {
            diag_util_mprintf("Port ID: %d\n", port);

            DIAG_UTIL_ERR_CHK(rtk_port_phyReg_get(unit, port, maxPage, 30, &oriReg), ret);
            DIAG_UTIL_ERR_CHK(rtk_port_phyReg_set(unit, port, maxPage, 30, 0), ret);

            if ((ret = rtk_port_phyReg_get(unit, port, 0xc81, 0x10, &regVal)) != RT_ERR_OK)
            {
                rtk_port_phyReg_set(unit, port, maxPage, 30, oriReg);
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }
            diag_util_mprintf(" good1: 0x%x\n", regVal);

            if ((ret = rtk_port_phyReg_get(unit, port, 0xc81, 0x11, &regVal)) != RT_ERR_OK)
            {
                rtk_port_phyReg_set(unit, port, maxPage, 30, oriReg);
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }
            diag_util_mprintf(" good2: 0x%x\n", regVal);

            if ((ret = rtk_port_phyReg_get(unit, port, 0xc81, 0x12, &regVal)) != RT_ERR_OK)
            {
                rtk_port_phyReg_set(unit, port, maxPage, 30, oriReg);
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }
            diag_util_mprintf("    err: 0x%x\n", regVal);

            DIAG_UTIL_ERR_CHK(rtk_port_phyReg_set(unit, port, maxPage, 30, oriReg), ret);
        }
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_diag_get_phy_port_ports_all_rx_cnt */
#endif

#ifdef CMD_DIAG_CLEAR_PHY_PORT_PORTS_ALL_RX_CNT
/*
 * diag clear phy port ( <PORT_LIST:ports> | all ) rx-cnt
 */
cparser_result_t
cparser_cmd_diag_clear_phy_port_ports_all_rx_cnt(
    cparser_context_t *context,
    char **ports_ptr)
{
    rtk_switch_devInfo_t    devInfo;
    diag_portlist_t         portlist;
    rtk_port_t              port;
    uint32                  unit;
    uint32                  maxPage = 0x1fff, regVal, oriReg;
    int32                   ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    memset(&devInfo, 0, sizeof(rtk_switch_devInfo_t));
    DIAG_UTIL_ERR_CHK(rtk_switch_deviceInfo_get(unit, &devInfo), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if (RTK_PORTMASK_IS_PORT_SET(devInfo.serdes.portmask, port))
            continue;

        if (!RTK_PORTMASK_IS_PORT_SET(devInfo.ether.portmask, port))
            continue;

        DIAG_UTIL_ERR_CHK(rtk_port_phyReg_get(unit, port, 0, 3, &regVal), ret);
        if (regVal == 0xC981)
        {
            DIAG_UTIL_ERR_CHK(rtk_port_phyReg_get(unit, port, maxPage, 30, &oriReg), ret);
            DIAG_UTIL_ERR_CHK(rtk_port_phyReg_set(unit, port, maxPage, 30, 0), ret);

            if ((ret = rtk_port_phyReg_set(unit, port, 0xc80, 0x11, 0x73)) != RT_ERR_OK)
            {
                rtk_port_phyReg_set(unit, port, maxPage, 30, oriReg);
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }

            DIAG_UTIL_ERR_CHK(rtk_port_phyReg_set(unit, port, maxPage, 30, oriReg), ret);
        }
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_diag_clear_phy_port_ports_all_rx_cnt */
#endif

#ifdef CMD_DIAG_SET_PHY_PORT_PORTS_ALL_RX_CNT_MAC_TX_PHY_RX
/*
 * diag set phy port ( <PORT_LIST:ports> | all ) rx-cnt ( mac-tx | phy-rx )
 */
cparser_result_t
cparser_cmd_diag_set_phy_port_ports_all_rx_cnt_mac_tx_phy_rx(
    cparser_context_t *context,
    char **ports_ptr)
{
    rtk_switch_devInfo_t    devInfo;
    diag_portlist_t         portlist;
    rtk_port_t              port;
    uint32                  unit, val;
    uint32                  maxPage = 0x1fff, regVal, oriReg;
    int32                   ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    memset(&devInfo, 0, sizeof(rtk_switch_devInfo_t));
    DIAG_UTIL_ERR_CHK(rtk_switch_deviceInfo_get(unit, &devInfo), ret);

    if ('m' == TOKEN_CHAR(6, 0))
        val = 0x6;
    else
        val = 0x2;

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if (RTK_PORTMASK_IS_PORT_SET(devInfo.serdes.portmask, port))
            continue;

        if (!RTK_PORTMASK_IS_PORT_SET(devInfo.ether.portmask, port))
            continue;

        DIAG_UTIL_ERR_CHK(rtk_port_phyReg_get(unit, port, 0, 3, &regVal), ret);
        if (regVal == 0xC981)
        {
            DIAG_UTIL_ERR_CHK(rtk_port_phyReg_get(unit, port, maxPage, 30, &oriReg), ret);
            DIAG_UTIL_ERR_CHK(rtk_port_phyReg_set(unit, port, maxPage, 30, 0), ret);

            if ((ret = rtk_port_phyReg_set(unit, port, 0xc80, 0x10, val)) != RT_ERR_OK)
            {
                rtk_port_phyReg_set(unit, port, maxPage, 30, oriReg);
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }

            DIAG_UTIL_ERR_CHK(rtk_port_phyReg_set(unit, port, maxPage, 30, oriReg), ret);
        }
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_diag_set_phy_port_ports_all_rx_cnt_mac_tx_phy_rx */
#endif

