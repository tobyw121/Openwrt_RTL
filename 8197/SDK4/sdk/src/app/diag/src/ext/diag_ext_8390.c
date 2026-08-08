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
 * $Revision: 14511 $
 * $Date: 2010-11-29 17:18:44 +0800 (?üÊ?‰∏Ä, 29 ?Å‰???2010) $
 *
 * Purpose : Definition those extension command and APIs in the SDK diagnostic shell.
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
#include <common/rt_error.h>
#include <common/rt_type.h>
#include <rtusr_util.h>
#include <diag_util.h>
#include <diag_om.h>
#include <diag_str.h>
#include <parser/cparser_priv.h>
#include <rtdrv/ext/rtdrv_netfilter_ext_8390.h>
#include <hal/common/halctrl.h>
#include <hal/chipdef/cypress/rtk_cypress_reg_struct.h>
#include <hal/mac/reg.h>
#include <ioal/mem32.h>
#ifdef CONFIG_SDK_MODEL_MODE
#include <model.h>
#include <virtualmac/vmac_target.h>
#endif
/*
 * SET
 */


/** MIB **/
cparser_result_t cparser_cmd_mib_set_rst_mib_val_0_1(cparser_context_t *context)
{
    uint32  unit = 0;
    rtdrv_ext_mibCfg_t cntr_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    cntr_cfg.unit = unit;
    switch(TOKEN_CHAR(3,0))
    {
        case '0':
            cntr_cfg.rst_val = 0;
            break;

        case '1':
            cntr_cfg.rst_val = 1;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }
    SETSOCKOPT(RTDRV_EXT_MIB_RST_VAL_SET, &cntr_cfg, rtdrv_ext_mibCfg_t, 1);

    return CPARSER_OK;
}

/*
 *  sqa reset
 */
cparser_result_t cparser_cmd_sqa_reset(cparser_context_t *context)
{
    uint32  unit;
    int32   ret = RT_ERR_FAILED;
    rtk_mac_t mac;
    uint32  addr, val, i, group_idx_max;
    rtk_qos_priSelWeight_t weight;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_qos_1pPriRemap_set(unit, 0, 0), ret);
    DIAG_UTIL_ERR_CHK(rtk_qos_1pPriRemap_set(unit, 1, 1), ret);
    DIAG_UTIL_ERR_CHK(rtk_qos_1pPriRemap_set(unit, 2, 2), ret);
    DIAG_UTIL_ERR_CHK(rtk_qos_1pPriRemap_set(unit, 3, 3), ret);
    DIAG_UTIL_ERR_CHK(rtk_qos_1pPriRemap_set(unit, 4, 4), ret);
    DIAG_UTIL_ERR_CHK(rtk_qos_1pPriRemap_set(unit, 5, 5), ret);
    DIAG_UTIL_ERR_CHK(rtk_qos_1pPriRemap_set(unit, 6, 6), ret);
    DIAG_UTIL_ERR_CHK(rtk_qos_1pPriRemap_set(unit, 7, 7), ret);

    DIAG_UTIL_ERR_CHK(rtk_switch_pppoePassthrough_set(unit, ENABLED), ret);

    /* reset switch MAC address */
    mac.octet[0] = 0x0;
    mac.octet[1] = 0x11;
    mac.octet[2] = 0x83;
    mac.octet[3] = 0x89;
    mac.octet[4] = 0x23;
    mac.octet[5] = 0x79;
    DIAG_UTIL_ERR_CHK(rtk_switch_mgmtMacAddr_set(unit, &mac), ret);

    /* enable L2 CAM */
    ioal_mem32_read(unit, RTL8390_L2_CTRL_0_ADDR, &val);
    ioal_mem32_write(unit, RTL8390_L2_CTRL_0_ADDR, (val | RTL8390_L2_CTRL_0_LUTCAM_EN_MASK));

    /* set backpressure method to be JAM mode */
    ioal_mem32_read(unit, RTL8390_MAC_GLB_CTRL_ADDR, &val);
    val &= ~RTL8390_MAC_GLB_CTRL_BKPRES_MTHD_SEL_MASK;
    ioal_mem32_write(unit, RTL8390_MAC_GLB_CTRL_ADDR, val);

    /* adjust priority selection weight */
    memset(&weight,0x0,sizeof(rtk_qos_priSelWeight_t));
    weight.weight_of_innerTag = 1;
    DIAG_OM_GET_CHIP_CAPACITY(unit, group_idx_max, pri_sel_group_index_max);
    for (i=0; i<= group_idx_max; i++)
        rtk_qos_priSelGroup_set(unit, 0, &weight);
    
#if defined(CONFIG_SDK_FPGA_PLATFORM)
    DIAG_UTIL_ERR_CHK(rtk_qos_queueNum_set(unit, 4), ret);

    /* reset FC threshold */
    ioal_mem32_write(unit, RTL8390_FC_DROP_THR_ADDR, 0x3ff);
    ioal_mem32_write(unit, RTL8390_FC_GLB_HI_THR_ADDR, 0x02320226);
    ioal_mem32_write(unit, RTL8390_FC_GLB_LO_THR_ADDR, 0x0168015e);
    ioal_mem32_write(unit, RTL8390_FC_GLB_FCOFF_HI_THR_ADDR, 0x02320226);
    ioal_mem32_write(unit, RTL8390_FC_GLB_FCOFF_LO_THR_ADDR, 0x0168015e);
    addr = RTL8390_FC_P_HI_THR_ADDR(0);
    ioal_mem32_write(unit, addr, 0x0064005e);
    addr = RTL8390_FC_P_LO_THR_ADDR(0);
    ioal_mem32_write(unit, addr, 0x00140014);
    addr = RTL8390_FC_P_FCOFF_HI_THR_ADDR(0);
    ioal_mem32_write(unit, addr, 0x0064005e);

    /* configure mode to trigger changing (T,B) and burst size */
    rtk_rate_stormControlRateMode_set(unit, BASED_ON_PKT);

    /* configure low threshold of input bandwidth leaky bucket */
    ioal_mem32_write(unit, RTL8390_IGR_BWCTRL_CTRL_ADDR, 0x17ff0);
#else
    DIAG_UTIL_ERR_CHK(rtk_qos_queueNum_set(unit, 8), ret);

    ioal_mem32_write(unit, RTL8390_FC_DROP_THR_ADDR, 0xfff);
    ioal_mem32_write(unit, RTL8390_FC_GLB_HI_THR_ADDR, 0x07780746);
    ioal_mem32_write(unit, RTL8390_FC_GLB_LO_THR_ADDR, 0x03520320);
    ioal_mem32_write(unit, RTL8390_FC_GLB_FCOFF_HI_THR_ADDR, 0x07780746);
    ioal_mem32_write(unit, RTL8390_FC_GLB_FCOFF_LO_THR_ADDR, 0x03520320);
    addr = RTL8390_FC_P_HI_THR_ADDR(0);
    ioal_mem32_write(unit, addr, 0x012c00fa);
    addr = RTL8390_FC_P_LO_THR_ADDR(0);
    ioal_mem32_write(unit, addr, 0x00140014);
    addr = RTL8390_FC_P_FCOFF_HI_THR_ADDR(0);
    ioal_mem32_write(unit, addr, 0x012c00fa);
#endif

    return CPARSER_OK;
}

/** SWITCH **/
cparser_result_t cparser_cmd_switch_set_48_pass_1_state_disable_enable(cparser_context_t *context)
{
    uint32 unit;
    rtk_enable_t enable = DISABLED;
    rtdrv_ext_switchCfg_t switch_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(4,0))
    {
        case 'd':
            enable = DISABLED;
            break;

        case 'e':
            enable = ENABLED;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    switch_cfg.unit = unit;
    switch_cfg.half_48pass1 = enable;
    SETSOCKOPT(RTDRV_EXT_SWITCH_48PASS1_SET, &switch_cfg, rtdrv_ext_switchCfg_t, 1);

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_switch_set_limit_pause_state_disable_enable(cparser_context_t *context)
{
    uint32 unit;
    rtk_enable_t enable = DISABLED;
    rtdrv_ext_switchCfg_t switch_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(4,0))
    {
        case 'd':
            enable = DISABLED;
            break;

        case 'e':
            enable = ENABLED;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    switch_cfg.unit = unit;
    switch_cfg.limit_pause = enable;
    SETSOCKOPT(RTDRV_EXT_SWITCH_LIMITPAUSE_SET, &switch_cfg, rtdrv_ext_switchCfg_t, 1);

    return CPARSER_OK;
}

/*
 *  switch set ipg-compensation ( ge | 10g ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_switch_set_ipg_compensation_ge_10g_state_disable_enable(cparser_context_t *context)
{
    uint32 unit;
    rtk_enable_t enable = DISABLED;
    rtdrv_ext_switchCfg_t switch_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(5,0))
    {
        case 'd':
            enable = DISABLED;
            break;

        case 'e':
            enable = ENABLED;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    switch(TOKEN_CHAR(3,0))
    {
        case 'g': /* ge */
            switch_cfg.unit = unit;
            switch_cfg.ipg_cmpstn = enable;
            SETSOCKOPT(RTDRV_EXT_SWITCH_IPGCOMSTN_SET, &switch_cfg, rtdrv_ext_switchCfg_t, 1);
            break;

        case '1': /* 10g */
            switch_cfg.unit = unit;
            switch_cfg.ipg_cmpstn = enable;
            SETSOCKOPT(RTDRV_EXT_SWITCH_IPGCOMSTN_10G_SET, &switch_cfg, rtdrv_ext_switchCfg_t, 1);
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}

/*
 *  switch set ipg-compensation ( ge | 10g ) ( 65ppm | 90ppm )
 */
cparser_result_t cparser_cmd_switch_set_ipg_compensation_ge_10g_65ppm_90ppm(cparser_context_t *context)
{
    uint32 unit;
    uint32 ipg_cmpstn_sel = 1;
    rtdrv_ext_switchCfg_t switch_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(4,0))
    {
        case '6':
            ipg_cmpstn_sel = 0;
            break;

        case '9':
            ipg_cmpstn_sel = 1;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    switch(TOKEN_CHAR(3,0))
    {
        case 'g': /* ge */
            switch_cfg.unit = unit;
            switch_cfg.ipg_cmpstn_sel = ipg_cmpstn_sel;
            SETSOCKOPT(RTDRV_EXT_SWITCH_IPGCOMSTN_SEL_SET, &switch_cfg, rtdrv_ext_switchCfg_t, 1);
            break;

        case '1': /* 10g */
            switch_cfg.unit = unit;
            switch_cfg.ipg_cmpstn_sel = ipg_cmpstn_sel;
            SETSOCKOPT(RTDRV_EXT_SWITCH_IPGCOMSTN_10G_SEL_SET, &switch_cfg, rtdrv_ext_switchCfg_t, 1);
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}

/*
 *  switch set ipg-min-length ( <PORT_LIST:ports> | all ) <UINT:length>
 */
cparser_result_t cparser_cmd_switch_set_ipg_min_length_ports_all_length(cparser_context_t *context,
    uint32_t *length_ptr)
{
    uint32 unit, port;
    int32  ret = RT_ERR_FAILED;
    rtdrv_ext_switchCfg_t switch_cfg;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        switch_cfg.unit = unit;
        switch_cfg.port = port;
        switch_cfg.data = *length_ptr;
        SETSOCKOPT(RTDRV_EXT_SWITCH_IPGMINLEN_SET, &switch_cfg, rtdrv_ext_switchCfg_t, 1);
    }

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_switch_set_back_pressure_jam_defer(cparser_context_t *context)
{
    uint32 unit;
    uint32 bkpres = 1;
    rtdrv_ext_switchCfg_t switch_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(3,0))
    {
        case 'j':
            bkpres = 0;
            break;

        case 'd':
            bkpres = 1;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    switch_cfg.unit = unit;
    switch_cfg.bkpres = bkpres;
    SETSOCKOPT(RTDRV_EXT_SWITCH_BKPRES_SET, &switch_cfg, rtdrv_ext_switchCfg_t, 1);

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_switch_set_pass_all_mode_ports_all_state_disable_enable(cparser_context_t *context)
{
    uint32 unit, port;
    int32  ret = RT_ERR_FAILED;
    rtk_enable_t enable = DISABLED;
    diag_portlist_t portlist;
    rtdrv_ext_switchCfg_t switch_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(5,0))
    {
        case 'd':
            enable = DISABLED;
            break;

        case 'e':
            enable = ENABLED;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    switch_cfg.unit = unit;
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        switch_cfg.port = port;
        switch_cfg.pass_all_mode = enable;
        SETSOCKOPT(RTDRV_EXT_SWITCH_PASSALLMODE_SET, &switch_cfg, rtdrv_ext_switchCfg_t, 1);
    }

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_switch_set_rx_check_crc_ports_all_state_disable_enable(cparser_context_t *context)
{
    uint32 unit, port;
    int32  ret = RT_ERR_FAILED;
    rtk_enable_t enable = DISABLED;
    diag_portlist_t portlist;
    rtdrv_ext_switchCfg_t switch_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(5,0))
    {
        case 'd':
            enable = DISABLED;
            break;

        case 'e':
            enable = ENABLED;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    switch_cfg.unit = unit;
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        switch_cfg.port = port;
        switch_cfg.rx_check_crc = enable;
        SETSOCKOPT(RTDRV_EXT_SWITCH_RXCHECKCRC_SET, &switch_cfg, rtdrv_ext_switchCfg_t, 1);
    }

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_switch_set_bypass_tx_crc_ports_all_state_disable_enable(cparser_context_t *context)
{
    uint32 unit, port;
    int32  ret = RT_ERR_FAILED;
    rtk_enable_t enable = DISABLED;
    diag_portlist_t portlist;
    rtdrv_ext_switchCfg_t switch_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(5,0))
    {
        case 'd':
            enable = DISABLED;
            break;

        case 'e':
            enable = ENABLED;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    switch_cfg.unit = unit;
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        switch_cfg.port = port;
        switch_cfg.bypass_tx_crc = enable;
        SETSOCKOPT(RTDRV_EXT_SWITCH_BYPASSTXCRC_SET, &switch_cfg, rtdrv_ext_switchCfg_t, 1);
    }

    return CPARSER_OK;
}

/*
 * switch set padding-und-size ( <PORT_LIST:ports> | all ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_switch_set_padding_und_size_ports_all_state_disable_enable(cparser_context_t *context)
{
    uint32 unit, port;
    int32  ret = RT_ERR_FAILED;
    rtk_enable_t enable = DISABLED;
    diag_portlist_t portlist;
    rtdrv_ext_switchCfg_t switch_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    switch(TOKEN_CHAR(5,0))
    {
        case 'd':
            enable = DISABLED;
            break;

        case 'e':
            enable = ENABLED;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    switch_cfg.unit = unit;
    switch_cfg.enable = enable;
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        switch_cfg.port = port;
        SETSOCKOPT(RTDRV_EXT_SWITCH_PADDINGUNDSIZE_SET, &switch_cfg, rtdrv_ext_switchCfg_t, 1);
    }

    return CPARSER_OK;
}

/** QoS **/
/*
 * qos set queue number port ( <PORT_LIST:ports> | all ) <UINT:queue_num>
 */
cparser_result_t cparser_cmd_qos_set_queue_number_port_ports_all_queue_num(cparser_context_t *context,
    uint32_t *queue_num_ptr)
{
    uint32 unit, port;
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtdrv_ext_qosCfg_t qos_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    qos_cfg.unit = unit;
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        qos_cfg.port = port;
        qos_cfg.qnum = *queue_num_ptr - 1;
        SETSOCKOPT(RTDRV_EXT_QOS_PORT_QUEUE_NUM_SET, &qos_cfg, rtdrv_ext_qosCfg_t, 1);
    }

    return CPARSER_OK;
}

/** Port **/
/*
 * port set ( <PORT_LIST:port> | all ) mac-state ( both-disable | tx-only | rx-only | both-enable )
 */
cparser_result_t cparser_cmd_port_set_port_all_mac_state_both_disable_tx_only_rx_only_both_enable(cparser_context_t *context)
{
    uint32 unit, port;
    int32  ret = RT_ERR_FAILED;
    uint32 state = 0;
    diag_portlist_t portlist;
    rtdrv_ext_portCfg_t port_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(4,0))
    {
        case 't':
            state = 0x1;
            break;

        case 'r':
            state = 0x2;
            break;

        case 'b':
            if('d' == TOKEN_CHAR(4,5))
                state = 0x0;
            else if('e' == TOKEN_CHAR(4,5))
                state = 0x3;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    port_cfg.unit = unit;
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 2), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        port_cfg.port = port;
        port_cfg.state = state;
        SETSOCKOPT(RTDRV_EXT_PORT_MAC_STATE_SET, &port_cfg, rtdrv_ext_portCfg_t, 1);
    }

    return CPARSER_OK;
}

/*
 * port set special-congest drain-out-thresh <UINT:threshold> duplex ( full | half )
 */
cparser_result_t cparser_cmd_port_set_special_congest_drain_out_thresh_threshold_duplex_full_half(
    cparser_context_t *context, 
    uint32_t *threshold_ptr)
{
    uint32 unit;
    rtdrv_ext_portCfg_t port_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    port_cfg.unit = unit;

    if (TOKEN_CHAR(6,0) == 'f')
    {
        port_cfg.full_th = *threshold_ptr;
        SETSOCKOPT(RTDRV_EXT_PORT_SPECIAL_CONGEST_DRAIN_OUT_THRESH_SET, &port_cfg, rtdrv_ext_portCfg_t, 1);
    }
    else if (TOKEN_CHAR(6,0) == 'h')
    {
        port_cfg.half_th = *threshold_ptr;
        SETSOCKOPT(RTDRV_EXT_PORT_SPECIAL_CONGEST_DRAIN_OUT_THRESH_HALF_SET, &port_cfg, rtdrv_ext_portCfg_t, 1);
    }
    else        
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}

/*
 * port set special-congest port ( <PORT_LIST:ports> | all ) sustain-timer <UINT:second> duplex ( full | half )
 */
cparser_result_t cparser_cmd_port_set_special_congest_port_ports_all_sustain_timer_second_duplex_full_half(
    cparser_context_t *context,
    uint32_t *second_ptr)
{
    uint32 unit, port;
    int32  ret = RT_ERR_FAILED;
    uint32 optid;
    diag_portlist_t portlist;
    rtdrv_ext_portCfg_t port_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    port_cfg.unit = unit;
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if (TOKEN_CHAR(8,0) == 'f')
    {
        optid = RTDRV_EXT_PORT_SPECIAL_CONGEST_PORT_SUSTAIN_TIMER_SET;
        port_cfg.full_sec = *second_ptr;
    }
    else if (TOKEN_CHAR(8,0) == 'h')
    {
        optid = RTDRV_EXT_PORT_SPECIAL_CONGEST_PORT_SUSTAIN_TIMER_HALF_SET;
        port_cfg.half_sec = *second_ptr;
    }
    else        
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        port_cfg.port = port;
        SETSOCKOPT(optid, &port_cfg, rtdrv_ext_portCfg_t, 1);
    }

    return CPARSER_OK;
}




/*
 * GET
 */


/** MIB **/
/*
 * mib dump counter debug <UINT:index>
 */
cparser_result_t cparser_cmd_mib_dump_counter_debug_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32  unit = 0;
    rtdrv_ext_mibCfg_t cntr_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    cntr_cfg.unit = unit;
    cntr_cfg.type = *index_ptr;
    GETSOCKOPT(RTDRV_EXT_MIB_DEBUG_GET, &cntr_cfg, rtdrv_ext_mibCfg_t, 1);

    switch (cntr_cfg.type)
    {
        case ALE_TX_GOOD_PKTS:
            diag_util_mprintf("Debug Counter %d - The number of ALE TX good packets : %u \n", cntr_cfg.type, cntr_cfg.cntr);
            break;
        case ERROR_PKTS:
            diag_util_mprintf("Debug Counter %d - Error packet : %u \n", cntr_cfg.type, cntr_cfg.cntr);
            break;
        case EGR_ACL_DROP:
            diag_util_mprintf("Debug Counter %d - Egress ACL drop : %u \n", cntr_cfg.type, cntr_cfg.cntr);
            break;
        case EGR_METER_DROP:
            diag_util_mprintf("Debug Counter %d - Egress meter drop : %u \n", cntr_cfg.type, cntr_cfg.cntr);
            break;
        case OAM:
            diag_util_mprintf("Debug Counter %d - OAM : %u \n", cntr_cfg.type, cntr_cfg.cntr);
            break;
        case CFM:
            diag_util_mprintf("Debug Counter %d - CFM : %u \n", cntr_cfg.type, cntr_cfg.cntr);
            break;
        case VLAN_IGR_FLTR:
            diag_util_mprintf("Debug Counter %d - VLAN ingress filter : %u \n", cntr_cfg.type, cntr_cfg.cntr);
            break;
        case VLAN_ERR:
            diag_util_mprintf("Debug Counter %d - VLAN Error(VID=4095 or MBR=0) : %u \n", cntr_cfg.type, cntr_cfg.cntr);
            break;
        case INNER_OUTER_CFI_EQUAL_1:
            diag_util_mprintf("Debug Counter %d - inner/outer CFI=1 : %u \n", cntr_cfg.type, cntr_cfg.cntr);
            break;
        case VLAN_TAG_FORMAT:
            diag_util_mprintf("Debug Counter %d - VLAN tag format : %u \n", cntr_cfg.type, cntr_cfg.cntr);
            break;
        case SRC_PORT_SPENDING_TREE:
            diag_util_mprintf("Debug Counter %d - Source port spending tree : %u \n", cntr_cfg.type, cntr_cfg.cntr);
            break;
        case INBW:
            diag_util_mprintf("Debug Counter %d - Input bandwidth : %u \n", cntr_cfg.type, cntr_cfg.cntr);
            break;
        case RMA:
            diag_util_mprintf("Debug Counter %d - RMA : %u \n", cntr_cfg.type, cntr_cfg.cntr);
            break;
        case HW_ATTACK_PREVENTION:
            diag_util_mprintf("Debug Counter %d - hardware attack prevention : %u \n", cntr_cfg.type, cntr_cfg.cntr);
            break;
        case PROTO_STORM:
            diag_util_mprintf("Debug Counter %d - protocol storm : %u \n", cntr_cfg.type, cntr_cfg.cntr);
            break;
        case MCAST_SA:
            diag_util_mprintf("Debug Counter %d - multicast SA : %u \n", cntr_cfg.type, cntr_cfg.cntr);
            break;
        case IGR_ACL_DROP:
            diag_util_mprintf("Debug Counter %d - Ingress ACL drop : %u \n", cntr_cfg.type, cntr_cfg.cntr);
            break;
        case IGR_METER_DROP:
            diag_util_mprintf("Debug Counter %d - Ingress Meter drop : %u \n", cntr_cfg.type, cntr_cfg.cntr);
            break;
        case DFLT_ACTION_FOR_MISS_ACL_AND_C2SC:
            diag_util_mprintf("Debug Counter %d - Default action for miss ACL &C2SC : %u \n", cntr_cfg.type, cntr_cfg.cntr);
            break;
        case NEW_SA:
            diag_util_mprintf("Debug Counter %d - New SA : %u \n", cntr_cfg.type, cntr_cfg.cntr);
            break;
        case PORT_MOVE:
            diag_util_mprintf("Debug Counter %d - Port move : %u \n", cntr_cfg.type, cntr_cfg.cntr);
            break;
        case SA_BLOCKING:
            diag_util_mprintf("Debug Counter %d - SA blocking : %u \n", cntr_cfg.type, cntr_cfg.cntr);
            break;
        case ROUTING_EXCEPTION:
            diag_util_mprintf("Debug Counter %d - routing exception : %u \n", cntr_cfg.type, cntr_cfg.cntr);
            break;
        case SRC_PORT_SPENDING_TREE_NON_FWDING:
            diag_util_mprintf("Debug Counter %d - Source port spending tree(non forwarding) : %u \n", cntr_cfg.type, cntr_cfg.cntr);
            break;
        case MAC_LIMIT:
            diag_util_mprintf("Debug Counter %d - MAC limit : %u \n", cntr_cfg.type, cntr_cfg.cntr);
            break;
        case UNKNOW_STORM:
            diag_util_mprintf("Debug Counter %d - Unknow storm : %u \n", cntr_cfg.type, cntr_cfg.cntr);
            break;
        case MISS_DROP:
            diag_util_mprintf("Debug Counter %d - Miss Drop : %u \n", cntr_cfg.type, cntr_cfg.cntr);
            break;
        case CPU_MAC_DROP:
            diag_util_mprintf("Debug Counter %d - CPU MAC drop : %u \n", cntr_cfg.type, cntr_cfg.cntr);
            break;
        case DA_BLOCKING:
            diag_util_mprintf("Debug Counter %d - DA blocking : %u \n", cntr_cfg.type, cntr_cfg.cntr);
            break;
        case SRC_PORT_FILTER_BEFORE_EGR_ACL:
            diag_util_mprintf("Egress Debug Counter %d - Source port filter(before Egress ACL) : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
            break;
        case VLAN_EGR_FILTER:
            diag_util_mprintf("Egress Debug Counter %d - VLAN egress filter : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
            break;
        case SPANNING_TRE:
            diag_util_mprintf("Egress Debug Counter %d - Spanning tree : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
            break;
        case PORT_ISOLATION:
            diag_util_mprintf("Egress Debug Counter %d - Port isolation : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
            break;
        case OAM_EGRESS_DROP:
            diag_util_mprintf("Egress Debug Counter %d - OAM : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
            break;
        case MIRROR_ISOLATION:
            diag_util_mprintf("Egress Debug Counter %d - Mirror isolation : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
            break;
        case MAX_LEN_BEFORE_EGR_ACL:
            diag_util_mprintf("Egress Debug Counter %d - Max length(before Egress ACL) : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
            break;
        case SRC_PORT_FILTER_BEFORE_MIRROR:
            diag_util_mprintf("Egress Debug Counter %d - Source port filter(before Mirror) : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
            break;
        case MAX_LEN_BEFORE_MIRROR:
            diag_util_mprintf("Egress Debug Counter %d - Max length(before Mirror) : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
            break;
        case SPECIAL_CONGEST_BEFORE_MIRROR:
            diag_util_mprintf("Egress Debug Counter %d - Special congest(before Mirror) : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
            break;
        case LINK_STATUS_BEFORE_MIRROR:
            diag_util_mprintf("Egress Debug Counter %d - Link status(before Mirror) : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
            break;
        case WRED_BEFORE_MIRROR:
            diag_util_mprintf("Egress Debug Counter %d - WRED(before Mirror) : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
            break;
        case MAX_LEN_AFTER_MIRROR:
            diag_util_mprintf("Egress Debug Counter %d - Max length(after Mirror) : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
            break;
        case SPECIAL_CONGEST_AFTER_MIRROR:
            diag_util_mprintf("Egress Debug Counter %d - Special congest(after Mirror) : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
            break;
        case LINK_STATUS_AFTER_MIRROR:
            diag_util_mprintf("Egress Debug Counter %d - Link status(after Mirror) : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
            break;
        case WRED_AFTER_MIRROR:
            diag_util_mprintf("Egress Debug Counter %d - WRED(after Mirror) : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
            break;
        default:
            diag_util_printf("User config: Error!\n");
    }

    return CPARSER_OK;
}

/*
 * mib dump counter debug all
 */
cparser_result_t cparser_cmd_mib_dump_counter_debug_all(cparser_context_t *context)
{
    uint32  unit = 0, i;
    rtdrv_ext_mibCfg_t cntr_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    cntr_cfg.unit = unit;
    for (i = 0; i < MIB_DBG_TYPE_END; i++)
    {
        cntr_cfg.type = i;
        GETSOCKOPT(RTDRV_EXT_MIB_DEBUG_GET, &cntr_cfg, rtdrv_ext_mibCfg_t, 1);
        switch (cntr_cfg.type)
        {
            case ALE_TX_GOOD_PKTS:
                diag_util_mprintf("Debug Counter %d - The number of ALE TX good packets : %u \n", cntr_cfg.type, cntr_cfg.cntr);
                break;
            case ERROR_PKTS:
                diag_util_mprintf("Debug Counter %d - Error packet : %u \n", cntr_cfg.type, cntr_cfg.cntr);
                break;
            case EGR_ACL_DROP:
                diag_util_mprintf("Debug Counter %d - Egress ACL drop : %u \n", cntr_cfg.type, cntr_cfg.cntr);
                break;
            case EGR_METER_DROP:
                diag_util_mprintf("Debug Counter %d - Egress meter drop : %u \n", cntr_cfg.type, cntr_cfg.cntr);
                break;
            case OAM:
                diag_util_mprintf("Debug Counter %d - OAM : %u \n", cntr_cfg.type, cntr_cfg.cntr);
                break;
            case CFM:
                diag_util_mprintf("Debug Counter %d - CFM : %u \n", cntr_cfg.type, cntr_cfg.cntr);
                break;
            case VLAN_IGR_FLTR:
                diag_util_mprintf("Debug Counter %d - VLAN ingress filter : %u \n", cntr_cfg.type, cntr_cfg.cntr);
                break;
            case VLAN_ERR:
                diag_util_mprintf("Debug Counter %d - VLAN Error(VID=4095 or MBR=0) : %u \n", cntr_cfg.type, cntr_cfg.cntr);
                break;
            case INNER_OUTER_CFI_EQUAL_1:
                diag_util_mprintf("Debug Counter %d - inner/outer CFI=1 : %u \n", cntr_cfg.type, cntr_cfg.cntr);
                break;
            case VLAN_TAG_FORMAT:
                diag_util_mprintf("Debug Counter %d - VLAN tag format : %u \n", cntr_cfg.type, cntr_cfg.cntr);
                break;
            case SRC_PORT_SPENDING_TREE:
                diag_util_mprintf("Debug Counter %d - Source port spending tree : %u \n", cntr_cfg.type, cntr_cfg.cntr);
                break;
            case INBW:
                diag_util_mprintf("Debug Counter %d - Input bandwidth : %u \n", cntr_cfg.type, cntr_cfg.cntr);
                break;
            case RMA:
                diag_util_mprintf("Debug Counter %d - RMA : %u \n", cntr_cfg.type, cntr_cfg.cntr);
                break;
            case HW_ATTACK_PREVENTION:
                diag_util_mprintf("Debug Counter %d - hardware attack prevention : %u \n", cntr_cfg.type, cntr_cfg.cntr);
                break;
            case PROTO_STORM:
                diag_util_mprintf("Debug Counter %d - protocol storm : %u \n", cntr_cfg.type, cntr_cfg.cntr);
                break;
            case MCAST_SA:
                diag_util_mprintf("Debug Counter %d - multicast SA : %u \n", cntr_cfg.type, cntr_cfg.cntr);
                break;
            case IGR_ACL_DROP:
                diag_util_mprintf("Debug Counter %d - Ingress ACL drop : %u \n", cntr_cfg.type, cntr_cfg.cntr);
                break;
            case IGR_METER_DROP:
                diag_util_mprintf("Debug Counter %d - Ingress Meter drop : %u \n", cntr_cfg.type, cntr_cfg.cntr);
                break;
            case DFLT_ACTION_FOR_MISS_ACL_AND_C2SC:
                diag_util_mprintf("Debug Counter %d - Default action for miss ACL &C2SC : %u \n", cntr_cfg.type, cntr_cfg.cntr);
                break;
            case NEW_SA:
                diag_util_mprintf("Debug Counter %d - New SA : %u \n", cntr_cfg.type, cntr_cfg.cntr);
                break;
            case PORT_MOVE:
                diag_util_mprintf("Debug Counter %d - Port move : %u \n", cntr_cfg.type, cntr_cfg.cntr);
                break;
            case SA_BLOCKING:
                diag_util_mprintf("Debug Counter %d - SA blocking : %u \n", cntr_cfg.type, cntr_cfg.cntr);
                break;
            case ROUTING_EXCEPTION:
                diag_util_mprintf("Debug Counter %d - routing exception : %u \n", cntr_cfg.type, cntr_cfg.cntr);
                break;
            case SRC_PORT_SPENDING_TREE_NON_FWDING:
                diag_util_mprintf("Debug Counter %d - Source port spending tree(non forwarding) : %u \n", cntr_cfg.type, cntr_cfg.cntr);
                break;
            case MAC_LIMIT:
                diag_util_mprintf("Debug Counter %d - MAC limit : %u \n", cntr_cfg.type, cntr_cfg.cntr);
                break;
            case UNKNOW_STORM:
                diag_util_mprintf("Debug Counter %d - Unknow storm : %u \n", cntr_cfg.type, cntr_cfg.cntr);
                break;
            case MISS_DROP:
                diag_util_mprintf("Debug Counter %d - Miss Drop : %u \n", cntr_cfg.type, cntr_cfg.cntr);
                break;
            case CPU_MAC_DROP:
                diag_util_mprintf("Debug Counter %d - CPU MAC drop : %u \n", cntr_cfg.type, cntr_cfg.cntr);
                break;
            case DA_BLOCKING:
                diag_util_mprintf("Debug Counter %d - DA blocking : %u \n", cntr_cfg.type, cntr_cfg.cntr);
                break;
            case SRC_PORT_FILTER_BEFORE_EGR_ACL:
                diag_util_mprintf("Egress Debug Counter %d - Source port filter(before Egress ACL) : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
                break;
            case VLAN_EGR_FILTER:
                diag_util_mprintf("Egress Debug Counter %d - VLAN egress filter : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
                break;
            case SPANNING_TRE:
                diag_util_mprintf("Egress Debug Counter %d - Spanning tree : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
                break;
            case PORT_ISOLATION:
                diag_util_mprintf("Egress Debug Counter %d - Port isolation : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
                break;
            case OAM_EGRESS_DROP:
                diag_util_mprintf("Egress Debug Counter %d - OAM : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
                break;
            case MIRROR_ISOLATION:
                diag_util_mprintf("Egress Debug Counter %d - Mirror isolation : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
                break;
            case MAX_LEN_BEFORE_EGR_ACL:
                diag_util_mprintf("Egress Debug Counter %d - Max length(before Egress ACL) : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
                break;
            case SRC_PORT_FILTER_BEFORE_MIRROR:
                diag_util_mprintf("Egress Debug Counter %d - Source port filter(before Mirror) : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
                break;
            case MAX_LEN_BEFORE_MIRROR:
                diag_util_mprintf("Egress Debug Counter %d - Max length(before Mirror) : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
                break;
            case SPECIAL_CONGEST_BEFORE_MIRROR:
                diag_util_mprintf("Egress Debug Counter %d - Special congest(before Mirror) : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
                break;
            case LINK_STATUS_BEFORE_MIRROR:
                diag_util_mprintf("Egress Debug Counter %d - Link status(before Mirror) : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
                break;
            case WRED_BEFORE_MIRROR:
                diag_util_mprintf("Egress Debug Counter %d - WRED(before Mirror) : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
                break;
            case MAX_LEN_AFTER_MIRROR:
                diag_util_mprintf("Egress Debug Counter %d - Max length(after Mirror) : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
                break;
            case SPECIAL_CONGEST_AFTER_MIRROR:
                diag_util_mprintf("Egress Debug Counter %d - Special congest(after Mirror) : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
                break;
            case LINK_STATUS_AFTER_MIRROR:
                diag_util_mprintf("Egress Debug Counter %d - Link status(after Mirror) : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
                break;
            case WRED_AFTER_MIRROR:
                diag_util_mprintf("Egress Debug Counter %d - WRED(after Mirror) : %u \n", cntr_cfg.type - 29, cntr_cfg.cntr);
                break;
            default:
                diag_util_printf("User config: Error!\n");
        }
    }

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_mib_get_rst_mib_val(cparser_context_t *context)
{
    uint32  unit = 0;
    rtdrv_ext_mibCfg_t cntr_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    cntr_cfg.unit = unit;
    GETSOCKOPT(RTDRV_EXT_MIB_RST_VAL_GET, &cntr_cfg, rtdrv_ext_mibCfg_t, 1);

    diag_util_mprintf("\tReset MIB Value : %d \n", cntr_cfg.rst_val);

    return CPARSER_OK;
}


/** SWITCH **/
cparser_result_t cparser_cmd_switch_get_48_pass_1_state(cparser_context_t *context)
{
    uint32 unit;
    rtk_enable_t enable = DISABLED;
    rtdrv_ext_switchCfg_t switch_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    switch_cfg.unit = unit;
    GETSOCKOPT(RTDRV_EXT_SWITCH_48PASS1_GET, &switch_cfg, rtdrv_ext_switchCfg_t, 1);
    enable = switch_cfg.half_48pass1;

    diag_util_mprintf("\t48pass1 State: %s\n", (enable == ENABLED) ? DIAG_STR_ENABLE : DIAG_STR_DISABLE);

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_switch_get_limit_pause_state(cparser_context_t *context)
{
    uint32 unit;
    rtk_enable_t enable = DISABLED;
    rtdrv_ext_switchCfg_t switch_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    switch_cfg.unit = unit;
    GETSOCKOPT(RTDRV_EXT_SWITCH_LIMITPAUSE_GET, &switch_cfg, rtdrv_ext_switchCfg_t, 1);
    enable = switch_cfg.limit_pause;

    diag_util_mprintf("\tLimit Pause State : %s\n", (enable == ENABLED) ? DIAG_STR_ENABLE : DIAG_STR_DISABLE);

    return CPARSER_OK;
}

/*
 *  switch get ipg-compensation ( ge | 10g ) state
 */
cparser_result_t cparser_cmd_switch_get_ipg_compensation_ge_10g_state(cparser_context_t *context)
{
    uint32 unit;
    rtk_enable_t enable = DISABLED;
    rtdrv_ext_switchCfg_t switch_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    switch(TOKEN_CHAR(3,0))
    {
        case 'g': /* ge */
            switch_cfg.unit = unit;
            GETSOCKOPT(RTDRV_EXT_SWITCH_IPGCOMSTN_GET, &switch_cfg, rtdrv_ext_switchCfg_t, 1);
            enable = switch_cfg.ipg_cmpstn;
            diag_util_mprintf("\tGE IPG Compensation State : %s\n", (enable == ENABLED) ? DIAG_STR_ENABLE : DIAG_STR_DISABLE);
            break;

        case '1': /* 10g */
            switch_cfg.unit = unit;
            GETSOCKOPT(RTDRV_EXT_SWITCH_IPGCOMSTN_10G_GET, &switch_cfg, rtdrv_ext_switchCfg_t, 1);
            enable = switch_cfg.ipg_cmpstn;
            diag_util_mprintf("\t10G IPG Compensation State : %s\n", (enable == ENABLED) ? DIAG_STR_ENABLE : DIAG_STR_DISABLE);
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}

/*
 *  switch get ipg-compensation ( ge | 10g )
 */
cparser_result_t cparser_cmd_switch_get_ipg_compensation_ge_10g(cparser_context_t *context)
{
    uint32 unit;
    rtdrv_ext_switchCfg_t switch_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    switch(TOKEN_CHAR(3,0))
    {
        case 'g': /* ge */
            switch_cfg.unit = unit;
            GETSOCKOPT(RTDRV_EXT_SWITCH_IPGCOMSTN_SEL_GET, &switch_cfg, rtdrv_ext_switchCfg_t, 1);
            diag_util_mprintf("\tGE IPG Compensation : %s\n", (switch_cfg.ipg_cmpstn_sel == 1) ? "90ppm" : "65ppm");
            break;

        case '1': /* 10g */
            switch_cfg.unit = unit;
            GETSOCKOPT(RTDRV_EXT_SWITCH_IPGCOMSTN_10G_SEL_GET, &switch_cfg, rtdrv_ext_switchCfg_t, 1);
            diag_util_mprintf("\t10G IPG Compensation : %s\n", (switch_cfg.ipg_cmpstn_sel == 1) ? "90ppm" : "65ppm");
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}

/*
 * switch get ipg-min-length ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_switch_get_ipg_min_length_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;
    int32  ret = RT_ERR_FAILED;
    rtdrv_ext_switchCfg_t switch_cfg;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("IPG Minimum Receive Length Configuration\n");
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        switch_cfg.unit = unit;
        switch_cfg.port = port;
        GETSOCKOPT(RTDRV_EXT_SWITCH_IPGMINLEN_GET, &switch_cfg, rtdrv_ext_switchCfg_t, 1);
        diag_util_mprintf("\tPort %2d : %d-Byte\n", port, switch_cfg.data);
    }

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_switch_get_back_pressure(cparser_context_t *context)
{
    uint32 unit;
    rtdrv_ext_switchCfg_t switch_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    switch_cfg.unit = unit;
    GETSOCKOPT(RTDRV_EXT_SWITCH_BKPRES_GET, &switch_cfg, rtdrv_ext_switchCfg_t, 1);

    diag_util_mprintf("\tBackpressure : %s\n", (switch_cfg.bkpres == 1) ? "Defer mode" : "Jam mode");

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_switch_get_pass_all_mode_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtdrv_ext_switchCfg_t switch_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    switch_cfg.unit = unit;


    diag_util_mprintf("Pass All Packets Mode Status\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        switch_cfg.port = port;
        GETSOCKOPT(RTDRV_EXT_SWITCH_PASSALLMODE_GET, &switch_cfg, rtdrv_ext_switchCfg_t, 1);
        diag_util_mprintf("\tPort %2d : %s\n", port, (switch_cfg.pass_all_mode == ENABLED) ? "ENABLE" : "DISABLE");
    }

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_switch_get_rx_check_crc_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtdrv_ext_switchCfg_t switch_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    switch_cfg.unit = unit;


    diag_util_mprintf("RX Check CRC Function Status\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        switch_cfg.port = port;
        GETSOCKOPT(RTDRV_EXT_SWITCH_RXCHECKCRC_GET, &switch_cfg, rtdrv_ext_switchCfg_t, 1);
        diag_util_mprintf("\tPort %2d : %s\n", port, (switch_cfg.rx_check_crc == ENABLED) ? "ENABLE" : "DISABLE");
    }

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_switch_get_bypass_tx_crc_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtdrv_ext_switchCfg_t switch_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    switch_cfg.unit = unit;


    diag_util_mprintf("Bypress TX Recalculating CRC Status\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        switch_cfg.port = port;
        GETSOCKOPT(RTDRV_EXT_SWITCH_BYPASSTXCRC_GET, &switch_cfg, rtdrv_ext_switchCfg_t, 1);
        diag_util_mprintf("\tPort %2d : %s\n", port, (switch_cfg.bypass_tx_crc == ENABLED) ? "ENABLE" : "DISABLE");
    }

    return CPARSER_OK;
}

/*
 * switch get padding-und-size ( <PORT_LIST:ports> | all ) state
 */
cparser_result_t cparser_cmd_switch_get_padding_und_size_ports_all_state(cparser_context_t *context)
{
    uint32 unit, port;
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtdrv_ext_switchCfg_t switch_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    switch_cfg.unit = unit;
    diag_util_mprintf("Padding Under Size State\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        switch_cfg.port = port;
        GETSOCKOPT(RTDRV_EXT_SWITCH_PADDINGUNDSIZE_GET, &switch_cfg, rtdrv_ext_switchCfg_t, 1);
        diag_util_mprintf("\tPort %2d : %s\n", port, (switch_cfg.enable == ENABLED) ? "ENABLE" : "DISABLE");
    }

    return CPARSER_OK;
}

/*
 * switch get info
 */
cparser_result_t cparser_cmd_switch_get_info(cparser_context_t *context)
{
    uint32 unit;
    rtdrv_ext_switchCfg_t switch_cfg;
    uint32 fpga_date = 0;
    uint32 rtl_version = 0;
    uint32 svn_revision = 0;
    FILE *fp;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    switch_cfg.unit = unit;
    GETSOCKOPT(RTDRV_EXT_SWITCH_INFO_GET, &switch_cfg, rtdrv_ext_switchCfg_t, 1);

    fpga_date = switch_cfg.chip_info;
    rtl_version  = (switch_cfg.model_char_1st & 0x1F) << 10;
    rtl_version |= (switch_cfg.model_char_2nd & 0x1F) << 5;
    rtl_version |= (switch_cfg.model_char_3rd & 0x1F) << 0;

    diag_util_mprintf(" FPGA Date : %04X\n", fpga_date);
    diag_util_mprintf("  RTL Ver. : %04X\n", rtl_version);

    /* SVN revision */
    fp = fopen("/etc/revision", "r");
    if (NULL != fp)
    {
        fscanf(fp, "%d", &svn_revision);
        fclose(fp);
        diag_util_mprintf("  F/W Ver. : %d\n", svn_revision);
    }

    return CPARSER_OK;
}

/** QoS **/
/*
 * qos get queue number port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_qos_get_queue_number_port_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtdrv_ext_qosCfg_t qos_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    qos_cfg.unit = unit;

    diag_util_mprintf("Queue Number\n");
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        qos_cfg.port = port;
        GETSOCKOPT(RTDRV_EXT_QOS_PORT_QUEUE_NUM_GET, &qos_cfg, rtdrv_ext_qosCfg_t, 1);
        diag_util_mprintf("\tPort %2d : %d\n", port, qos_cfg.qnum + 1);
    }

    return CPARSER_OK;
}

/** Flow Control **/

/*
 * flowctrl get used-page-cnt ( ingress | egress ) ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_flowctrl_get_used_page_cnt_ingress_egress_ports_all(cparser_context_t *context)
{
    uint32      unit, port;
    int32       ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtdrv_ext_flowctrlCfg_t flowctrl_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    flowctrl_cfg.unit = unit;

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    if ('i' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            flowctrl_cfg.port = port;
            GETSOCKOPT(RTDRV_EXT_FLOWCTRL_PORT_USED_PAGE_CNT_INGRESS_GET, &flowctrl_cfg, rtdrv_ext_flowctrlCfg_t, 1);
            diag_util_mprintf("Port %2d\n", port);
            diag_util_mprintf("\tIngress Used Page Count : %d\n", flowctrl_cfg.used_page_cnt);
            diag_util_mprintf("\tIngress Max Used Page Count : %d\n", flowctrl_cfg.used_page_cnt_max);
        }
    }
    else if ('e' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            flowctrl_cfg.port = port;
            GETSOCKOPT(RTDRV_EXT_FLOWCTRL_PORT_USED_PAGE_CNT_EGRESS_GET, &flowctrl_cfg, rtdrv_ext_flowctrlCfg_t, 1);
            diag_util_mprintf("Port %2d\n", port);
            diag_util_mprintf("\tEgress Used Page Count : %d\n", flowctrl_cfg.used_page_cnt);
            diag_util_mprintf("\tEgress Max Used Page Count : %d\n", flowctrl_cfg.used_page_cnt_max);
        }
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}

/*
 * flowctrl get used-page-cnt system
 */
cparser_result_t cparser_cmd_flowctrl_get_used_page_cnt_system(cparser_context_t *context)
{
    uint32      unit;
    rtdrv_ext_flowctrlCfg_t flowctrl_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    flowctrl_cfg.unit = unit;

    GETSOCKOPT(RTDRV_EXT_FLOWCTRL_SYSTEM_USED_PAGE_CNT_GET, &flowctrl_cfg, rtdrv_ext_flowctrlCfg_t, 1);
    diag_util_mprintf("System Used Page Count : %d\n", flowctrl_cfg.used_page_cnt);
    diag_util_mprintf("System Max Used Page Count : %d\n", flowctrl_cfg.used_page_cnt_max);

    return CPARSER_OK;
}

/*
 * flowctrl get used-page-cnt queue-based ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_flowctrl_get_used_page_cnt_queue_based_ports_all(cparser_context_t *context)
{
    uint32      unit, port = 0;
    rtk_qid_t   queue, qid_max;
    int32       ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtdrv_ext_flowctrlCfg_t flowctrl_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_OM_GET_CHIP_CAPACITY(unit, qid_max, max_num_of_queue);
    flowctrl_cfg.unit = unit;

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("Port %2d\n", port);
        flowctrl_cfg.port = port;
        for (queue = 0; queue < qid_max; queue++)
        {
            flowctrl_cfg.queue = queue;
            GETSOCKOPT(RTDRV_EXT_FLOWCTRL_PORT_QUEUE_USED_PAGE_CNT_GET, &flowctrl_cfg, rtdrv_ext_flowctrlCfg_t, 1);
            diag_util_mprintf("\tQueue %d Used Page Count : %d\n", queue, flowctrl_cfg.used_page_cnt);
            diag_util_mprintf("\tQueue %d Max Used Page Count : %d\n", queue, flowctrl_cfg.used_page_cnt_max);
        }
    }

    return CPARSER_OK;
}

/*
 * flowctrl reset used-page-cnt ( ingress | egress ) ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_flowctrl_reset_used_page_cnt_ingress_egress_ports_all(cparser_context_t *context)
{
    uint32      unit, port;
    int32       ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtdrv_ext_flowctrlCfg_t flowctrl_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    flowctrl_cfg.unit = unit;

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    if ('i' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            flowctrl_cfg.port = port;
            SETSOCKOPT(RTDRV_EXT_FLOWCTRL_PORT_USED_PAGE_CNT_INGRESS_RESET, &flowctrl_cfg, rtdrv_ext_flowctrlCfg_t, 1);
        }
    }
    else if ('e' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            flowctrl_cfg.port = port;
            SETSOCKOPT(RTDRV_EXT_FLOWCTRL_PORT_USED_PAGE_CNT_EGRESS_RESET, &flowctrl_cfg, rtdrv_ext_flowctrlCfg_t, 1);
        }
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}

/*
 * flowctrl reset used-page-cnt system
 */
cparser_result_t cparser_cmd_flowctrl_reset_used_page_cnt_system(cparser_context_t *context)
{
    uint32      unit;
    rtdrv_ext_flowctrlCfg_t flowctrl_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    flowctrl_cfg.unit = unit;

    SETSOCKOPT(RTDRV_EXT_FLOWCTRL_SYSTEM_USED_PAGE_CNT_RESET, &flowctrl_cfg, rtdrv_ext_flowctrlCfg_t, 1);

    return CPARSER_OK;
}


/** Port **/
/*
 * port get ( <PORT_LIST:port> | all ) mac-state
 */
cparser_result_t cparser_cmd_port_get_port_all_mac_state(cparser_context_t *context)
{
    uint32      unit, port;
    int32       ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtdrv_ext_portCfg_t port_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    port_cfg.unit = unit;

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 2), ret);

    diag_util_mprintf("Port MAC State\n");

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        port_cfg.port = port;
        GETSOCKOPT(RTDRV_EXT_PORT_MAC_STATE_GET, &port_cfg, rtdrv_ext_portCfg_t, 1);
        if (0x0 == port_cfg.state)
            diag_util_mprintf("\tPort %2d : TX and RX disabled\n", port);
        else if (0x1 == port_cfg.state)
            diag_util_mprintf("\tPort %2d : TX-only\n", port);
        else if (0x2 == port_cfg.state)
            diag_util_mprintf("\tPort %2d : RX-only\n", port);
        else if (0x3 == port_cfg.state)
            diag_util_mprintf("\tPort %2d : TX and RX enabled\n", port);
    }

    return CPARSER_OK;
}

/*
 * port get special-congest drain-out-thresh
 */
cparser_result_t cparser_cmd_port_get_special_congest_drain_out_thresh(cparser_context_t *context)
{
    uint32 unit;
    rtdrv_ext_portCfg_t port_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    memset(&port_cfg, 0x00, sizeof(rtdrv_ext_portCfg_t));
    port_cfg.unit = unit;
    GETSOCKOPT(RTDRV_EXT_PORT_SPECIAL_CONGEST_DRAIN_OUT_THRESH_GET, &port_cfg, rtdrv_ext_portCfg_t, 1);
    diag_util_mprintf("\tFull Duplex Drain-Out Threshold : %d\n", port_cfg.full_th);
    diag_util_mprintf("\tHalf Duplex Drain-Out Threshold : %d\n", port_cfg.half_th);

    return CPARSER_OK;
}

/*
 * port get special-congest port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_port_get_special_congest_port_ports_all(cparser_context_t *context, char **ports_ptr)
{
    uint32      unit, port;
    int32       ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtdrv_ext_portCfg_t port_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    memset(&port_cfg, 0x00, sizeof(rtdrv_ext_portCfg_t));
    port_cfg.unit = unit;
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        port_cfg.port = port;
        GETSOCKOPT(RTDRV_EXT_PORT_SPECIAL_CONGEST_PORT_GET, &port_cfg, rtdrv_ext_portCfg_t, 1);
        diag_util_mprintf("Port %2d : \n", port);
        diag_util_mprintf("\tFull Duplex Sustain Timer : %d\n", port_cfg.full_sec);
        diag_util_mprintf("\tHalf Duplex Sustain Timer : %d\n", port_cfg.half_sec);
    }

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_eee_get_tx_wake_mode(cparser_context_t *context)
{
    uint32  unit = 0;
    rtdrv_ext_eeeCfg_t  eee_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    eee_cfg.unit = unit;
    GETSOCKOPT(RTDRV_EXT_EEE_TX_WAKE_MODE_GET, &eee_cfg, rtdrv_ext_eeeCfg_t, 1);

    diag_util_printf("\tEEE TX Wake Mode : ");
    switch(eee_cfg.wakeMode)
    {
        case WAKE_MODE_ANY_PKT_TX:
            diag_util_mprintf("packet-tx\n");
            break;
        case WAKE_MODE_QOS_BASED:
            diag_util_mprintf("qos-based\n");
            break;
        default:
            break;
    }

    return CPARSER_OK;

}

cparser_result_t cparser_cmd_eee_get_status_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32  unit = 0;
    int32  ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t portlist;
    rtdrv_ext_eeeCfg_t  eee_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    eee_cfg.unit = unit;
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    diag_util_mprintf("EEE or EEEP Status:\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        eee_cfg.port = port;
        GETSOCKOPT(RTDRV_EXT_EEE_PORT_STATUS_GET, &eee_cfg, rtdrv_ext_eeeCfg_t, 1);
        diag_util_mprintf("\tPort %2d :\n", port);
        if (EEE_EEEP_WAKE_STATE == eee_cfg.rxState)
            diag_util_mprintf("\t\tRX : Wake\n");
        else if (EEE_EEEP_SLEEP_STATE == eee_cfg.rxState)
            diag_util_mprintf("\t\tRX : Sleep\n");

        if (EEE_EEEP_WAKE_STATE == eee_cfg.txState)
            diag_util_mprintf("\t\tTX : Wake\n");
        else if (EEE_EEEP_SLEEP_STATE == eee_cfg.txState)
            diag_util_mprintf("\t\tTX : Sleep\n");
    }

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_eee_get_link_up_delay(cparser_context_t *context)
{
    uint32 unit;
    rtdrv_ext_eeeCfg_t  eee_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    eee_cfg.unit = unit;
    GETSOCKOPT(RTDRV_EXT_EEE_LINK_UP_DELAY_GET, &eee_cfg, rtdrv_ext_eeeCfg_t, 1);

    diag_util_printf("\tEEE Link Up Delay ");
    switch(eee_cfg.linkUpDelay)
    {
        case 0:
            diag_util_mprintf(": 1024 ms\n");
            break;
        case 1:
            diag_util_mprintf(": 512 ms\n");
            break;
        case 2:
            diag_util_mprintf(": 256 ms\n");
            break;
        case 3:
            diag_util_mprintf(": 1 ms\n");
            break;
        default:
            break;
    }

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_eee_get_multi_wake_state(cparser_context_t *context)
{
    uint32 unit;
    rtdrv_ext_eeeCfg_t  eee_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    eee_cfg.unit = unit;
    GETSOCKOPT(RTDRV_EXT_EEE_MULTI_WAKE_STATE_GET, &eee_cfg, rtdrv_ext_eeeCfg_t, 1);

    diag_util_printf("\tEEE Multi-wake State : ");
    switch(eee_cfg.enable)
    {
        case DISABLED:
            diag_util_mprintf("Disabled\n");
            break;
        case ENABLED:
            diag_util_mprintf("Enabled\n");
            break;
        default:
            break;
    }

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_eee_get_multi_wake_interval(cparser_context_t *context)
{
    uint32 unit;
    rtdrv_ext_eeeCfg_t  eee_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    eee_cfg.unit = unit;
    GETSOCKOPT(RTDRV_EXT_EEE_MULTI_WAKE_INTERVAL_GET, &eee_cfg, rtdrv_ext_eeeCfg_t, 1);

    diag_util_printf("\tEEE Multi-wake Interval : ");
    switch(eee_cfg.interval)
    {
        case 0:
            diag_util_mprintf("1 us\n");
            break;
        case 1:
            diag_util_mprintf("2 us\n");
            break;
        case 2:
            diag_util_mprintf("3 us\n");
            break;
        case 3:
            diag_util_mprintf("4 us\n");
            break;
        default:
            break;
    }

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_eee_get_multi_wake_port_num(cparser_context_t *context)
{
    uint32 unit;
    rtdrv_ext_eeeCfg_t  eee_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    eee_cfg.unit = unit;
    GETSOCKOPT(RTDRV_EXT_EEE_MULTI_WAKE_PORT_NUM_GET, &eee_cfg, rtdrv_ext_eeeCfg_t, 1);

    diag_util_printf("\tEEE Multi-wake Port Number of a Group : ");
    switch(eee_cfg.portNum)
    {
        case 0:
            diag_util_mprintf("2 ports\n");
            break;
        case 1:
            diag_util_mprintf("4 ports\n");
            break;
        case 2:
            diag_util_mprintf("6 ports\n");
            break;
        case 3:
            diag_util_mprintf("8 ports\n");
            break;
        case 4:
            diag_util_mprintf("10 ports\n");
            break;
        case 5:
            diag_util_mprintf("12 ports\n");
            break;
        case 6:
            diag_util_mprintf("16 ports\n");
            break;
        case 7:
            diag_util_mprintf("20 ports\n");
            break;
        default:
            break;
    }

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_eee_set_tx_wake_mode_packet_tx_qos_based(cparser_context_t *context)
{
    uint32 unit;
    rtdrv_ext_eeeCfg_t  eee_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(4,0))
    {
        case 'p':
            eee_cfg.wakeMode = WAKE_MODE_ANY_PKT_TX;
            break;

        case 'q':
            eee_cfg.wakeMode = WAKE_MODE_QOS_BASED;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    eee_cfg.unit = unit;
    SETSOCKOPT(RTDRV_EXT_EEE_TX_WAKE_MODE_SET, &eee_cfg, rtdrv_ext_eeeCfg_t, 1);

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_eee_set_link_up_delay_1ms_256ms_512ms_1024ms(cparser_context_t *context)
{
    uint32 unit;
    rtdrv_ext_eeeCfg_t  eee_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(3,1))
    {
        case 'm': /*1ms*/
            eee_cfg.linkUpDelay = 3;
            break;
        case '5': /*256ms*/
            eee_cfg.linkUpDelay = 2;
            break;
        case '1': /*512ms*/
            eee_cfg.linkUpDelay = 1;
            break;
        case '0': /*1024ms*/
            eee_cfg.linkUpDelay = 0;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    eee_cfg.unit = unit;
    SETSOCKOPT(RTDRV_EXT_EEE_LINK_UP_DELAY_SET, &eee_cfg, rtdrv_ext_eeeCfg_t, 1);

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_eee_set_multi_wake_state_disable_enable(cparser_context_t *context)
{
    uint32 unit;
    rtdrv_ext_eeeCfg_t  eee_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(4,0))
    {
        case 'd': /*disable*/
            eee_cfg.enable = DISABLED;
            break;
        case 'e': /*enable*/
            eee_cfg.enable = ENABLED;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    eee_cfg.unit = unit;
    SETSOCKOPT(RTDRV_EXT_EEE_MULTI_WAKE_STATE_SET, &eee_cfg, rtdrv_ext_eeeCfg_t, 1);

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_eee_set_multi_wake_interval_1us_2us_3us_4us(cparser_context_t *context)
{
    uint32 unit;
    rtdrv_ext_eeeCfg_t  eee_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(4,0))
    {
        case '1': /*1us*/
            eee_cfg.interval = 0;
            break;
        case '2': /*2us*/
            eee_cfg.interval = 1;
            break;
        case '3': /*3us*/
            eee_cfg.interval = 2;
            break;
        case '4': /*4us*/
            eee_cfg.interval = 3;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    eee_cfg.unit = unit;
    SETSOCKOPT(RTDRV_EXT_EEE_MULTI_WAKE_INTERVAL_SET, &eee_cfg, rtdrv_ext_eeeCfg_t, 1);

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_eee_set_multi_wake_port_num_2port_4port_6port_8port_10port_12port_16port_20port(cparser_context_t *context)
{
    uint32 unit;
    rtdrv_ext_eeeCfg_t  eee_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(4,0))
    {
        case '2':
            if('p' == TOKEN_CHAR(4,1)) /*2port*/
                eee_cfg.portNum = 0;
            else /*20port*/
                eee_cfg.portNum = 7;
            break;
        case '4': /*4port*/
            eee_cfg.portNum = 1;
            break;
        case '6': /*6port*/
            eee_cfg.portNum = 2;
            break;
        case '8': /*8port*/
            eee_cfg.portNum = 3;
            break;
        case '1':
            if('0' == TOKEN_CHAR(4,1)) /*10port*/
                eee_cfg.portNum = 4;
            else if('2' == TOKEN_CHAR(4,1)) /*12port*/
                eee_cfg.portNum = 5;
            else
                eee_cfg.portNum = 6; /*16port*/
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    eee_cfg.unit = unit;
    SETSOCKOPT(RTDRV_EXT_EEE_MULTI_WAKE_PORT_NUM_SET, &eee_cfg, rtdrv_ext_eeeCfg_t, 1);

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_eeep_get_tx_sleep_rate(cparser_context_t *context)
{
    uint32 unit;
    rtdrv_ext_eeeCfg_t  eee_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    eee_cfg.unit = unit;
    GETSOCKOPT(RTDRV_EXT_EEEP_TX_SLEEP_RATE_GET, &eee_cfg, rtdrv_ext_eeeCfg_t, 1);

    diag_util_mprintf("\tEEEP TX Sleep Rate (unit:0.5Mbps): %d\n", eee_cfg.rate);

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_eeep_get_rx_sleep_rate(cparser_context_t *context)
{
    uint32 unit;
    rtdrv_ext_eeeCfg_t  eee_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    eee_cfg.unit = unit;
    GETSOCKOPT(RTDRV_EXT_EEEP_RX_SLEEP_RATE_GET, &eee_cfg, rtdrv_ext_eeeCfg_t, 1);

    diag_util_mprintf("\tEEEP RX Sleep Rate (unit:0.5Mbps): %d\n", eee_cfg.rate);

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_eeep_set_tx_sleep_rate_rate(cparser_context_t *context,
    uint32_t *rate_ptr)
{
    uint32 unit;
    rtdrv_ext_eeeCfg_t  eee_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    eee_cfg.rate = *rate_ptr;

    eee_cfg.unit = unit;
    SETSOCKOPT(RTDRV_EXT_EEEP_TX_SLEEP_RATE_SET, &eee_cfg, rtdrv_ext_eeeCfg_t, 1);

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_eeep_set_rx_sleep_rate_rate(cparser_context_t *context,
    uint32_t *rate_ptr)
{
    uint32 unit;
    rtdrv_ext_eeeCfg_t  eee_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    eee_cfg.rate = *rate_ptr;
    eee_cfg.unit = unit;
    SETSOCKOPT(RTDRV_EXT_EEEP_RX_SLEEP_RATE_SET, &eee_cfg, rtdrv_ext_eeeCfg_t, 1);

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_iol_dump(cparser_context_t *context)
{
    uint32 unit;
    rtdrv_ext_iolCfg_t  iol_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    iol_cfg.unit = unit;
    GETSOCKOPT(RTDRV_EXT_IOL_COLLISION_MAX_ATTEMPT_GET, &iol_cfg, rtdrv_ext_iolCfg_t, 1);
    diag_util_printf("Collision-max-attempt: ");
    switch(iol_cfg.action)
    {
        case 0:
            diag_util_mprintf("retry\n");
            break;
        case 1:
            diag_util_mprintf("drop\n");
            break;
        default:
            break;
    }

    GETSOCKOPT(RTDRV_EXT_IOL_ERROR_LENGTH_GET, &iol_cfg, rtdrv_ext_iolCfg_t, 1);
    diag_util_printf("Error-length: ");
    switch(iol_cfg.action)
    {
        case 0:
            diag_util_mprintf("forward\n");
            break;
        case 1:
            diag_util_mprintf("drop\n");
            break;
        default:
            break;
    }

    GETSOCKOPT(RTDRV_EXT_IOL_INVALID_PAUSE_GET, &iol_cfg, rtdrv_ext_iolCfg_t, 1);
    diag_util_printf("Invalid-pause: ");
    switch(iol_cfg.action)
    {
        case 0:
            diag_util_mprintf("forward\n");
            break;
        case 1:
            diag_util_mprintf("drop\n");
            break;
        default:
            break;
    }

    GETSOCKOPT(RTDRV_EXT_IOL_LATE_COLLISION_GET, &iol_cfg, rtdrv_ext_iolCfg_t, 1);
    diag_util_printf("Late-collision: ");
    switch(iol_cfg.action)
    {
        case 0:
            diag_util_mprintf("re-transmit\n");
            break;
        case 1:
            diag_util_mprintf("drop\n");
            break;
        default:
            break;
    }

    GETSOCKOPT(RTDRV_EXT_IOL_MAX_LENGTH_GET, &iol_cfg, rtdrv_ext_iolCfg_t, 1);
    diag_util_printf("Max-length: ");
    switch(iol_cfg.enable)
    {
        case DISABLED:
            diag_util_mprintf("disable\n");
            break;
        case ENABLED:
            diag_util_mprintf("enable\n");
            break;
        default:
            break;
    }

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_iol_get_collision_max_attempt(cparser_context_t *context)
{
    uint32 unit;
    rtdrv_ext_iolCfg_t  iol_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    iol_cfg.unit = unit;
    GETSOCKOPT(RTDRV_EXT_IOL_COLLISION_MAX_ATTEMPT_GET, &iol_cfg, rtdrv_ext_iolCfg_t, 1);

    diag_util_printf("Collision-max-attempt: ");
    switch(iol_cfg.action)
    {
        case 0:
            diag_util_mprintf("retry\n");
            break;
        case 1:
            diag_util_mprintf("drop\n");
            break;
        default:
            break;
    }

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_iol_get_error_length(cparser_context_t *context)
{
    uint32 unit;
    rtdrv_ext_iolCfg_t  iol_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    iol_cfg.unit = unit;
    GETSOCKOPT(RTDRV_EXT_IOL_ERROR_LENGTH_GET, &iol_cfg, rtdrv_ext_iolCfg_t, 1);

    diag_util_printf("Error-length: ");
    switch(iol_cfg.action)
    {
        case 0:
            diag_util_mprintf("forward\n");
            break;
        case 1:
            diag_util_mprintf("drop\n");
            break;
        default:
            break;
    }

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_iol_get_invalid_pause(cparser_context_t *context)
{
    uint32 unit;
    rtdrv_ext_iolCfg_t  iol_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    iol_cfg.unit = unit;
    GETSOCKOPT(RTDRV_EXT_IOL_INVALID_PAUSE_GET, &iol_cfg, rtdrv_ext_iolCfg_t, 1);

    diag_util_printf("Invalid-pause: ");
    switch(iol_cfg.action)
    {
        case 0:
            diag_util_mprintf("forward\n");
            break;
        case 1:
            diag_util_mprintf("drop\n");
            break;
        default:
            break;
    }

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_iol_get_late_collision(cparser_context_t *context)
{
    uint32 unit;
    rtdrv_ext_iolCfg_t  iol_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    iol_cfg.unit = unit;
    GETSOCKOPT(RTDRV_EXT_IOL_LATE_COLLISION_GET, &iol_cfg, rtdrv_ext_iolCfg_t, 1);

    diag_util_printf("Late-collision: ");
    switch(iol_cfg.action)
    {
        case 0:
            diag_util_mprintf("re-transmit\n");
            break;
        case 1:
            diag_util_mprintf("drop\n");
            break;
        default:
            break;
    }

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_iol_get_max_length(cparser_context_t *context)
{
    uint32 unit;
    rtdrv_ext_iolCfg_t  iol_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    iol_cfg.unit = unit;
    GETSOCKOPT(RTDRV_EXT_IOL_MAX_LENGTH_GET, &iol_cfg, rtdrv_ext_iolCfg_t, 1);

    diag_util_printf("Max-length: ");
    switch(iol_cfg.enable)
    {
        case DISABLED:
            diag_util_mprintf("disable\n");
            break;
        case ENABLED:
            diag_util_mprintf("enable\n");
            break;
        default:
            break;
    }

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_iol_set_collision_max_attempt_retry_drop(cparser_context_t *context)
{
    uint32 unit;
    rtdrv_ext_iolCfg_t  iol_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(3,0))
    {
        case 'r': /*retry*/
            iol_cfg.action = 0;
            break;
        case 'd': /*drop*/
            iol_cfg.action = 1;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    iol_cfg.unit = unit;
    SETSOCKOPT(RTDRV_EXT_IOL_COLLISION_MAX_ATTEMPT_SET, &iol_cfg, rtdrv_ext_iolCfg_t, 1);

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_iol_set_error_length_forward_drop(cparser_context_t *context)
{
    uint32 unit;
    rtdrv_ext_iolCfg_t  iol_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(3,0))
    {
        case 'f': /*forward*/
            iol_cfg.action = 0;
            break;
        case 'd': /*drop*/
            iol_cfg.action = 1;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    iol_cfg.unit = unit;
    SETSOCKOPT(RTDRV_EXT_IOL_ERROR_LENGTH_SET, &iol_cfg, rtdrv_ext_iolCfg_t, 1);

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_iol_set_invalid_pause_forward_drop(cparser_context_t *context)
{
    uint32 unit;
    rtdrv_ext_iolCfg_t  iol_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(3,0))
    {
        case 'f': /*retry*/
            iol_cfg.action = 0;
            break;
        case 'd': /*drop*/
            iol_cfg.action = 1;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    iol_cfg.unit = unit;
    SETSOCKOPT(RTDRV_EXT_IOL_INVALID_PAUSE_SET, &iol_cfg, rtdrv_ext_iolCfg_t, 1);

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_iol_set_late_collision_re_transmit_drop(cparser_context_t *context)
{
    uint32 unit;
    rtdrv_ext_iolCfg_t  iol_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(3,0))
    {
        case 'r': /*re-transmit*/
            iol_cfg.action = 0;
            break;
        case 'd': /*drop*/
            iol_cfg.action = 1;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    iol_cfg.unit = unit;
    SETSOCKOPT(RTDRV_EXT_IOL_LATE_COLLISION_SET, &iol_cfg, rtdrv_ext_iolCfg_t, 1);

    return CPARSER_OK;
}

cparser_result_t cparser_cmd_iol_set_max_length_disable_enable(cparser_context_t *context)
{
    uint32 unit;
    rtdrv_ext_iolCfg_t  iol_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(3,0))
    {
        case 'd': /*disable*/
            iol_cfg.enable = DISABLED;
            break;
        case 'e': /*enable*/
            iol_cfg.enable = ENABLED;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    iol_cfg.unit = unit;
    SETSOCKOPT(RTDRV_EXT_IOL_MAX_LENGTH_SET, &iol_cfg, rtdrv_ext_iolCfg_t, 1);

    return CPARSER_OK;
}


cparser_result_t cparser_cmd_model_set_debug_mask_maskVal(cparser_context_t *context,
    uint32_t *maskVal_ptr)
{
#ifdef CONFIG_SDK_MODEL_MODE
    rtdrv_ext_modelCfg_t  modelTest_cfg;

    modelTest_cfg.debugFlagMask = *maskVal_ptr;

    SETSOCKOPT(RTDRV_EXT_MODEL_DEBUG_MASK_SET, &modelTest_cfg, rtdrv_ext_modelCfg_t, 1);
#endif
    return CPARSER_OK;
}

cparser_result_t cparser_cmd_model_get_debug_mask(cparser_context_t *context)
{
#ifdef CONFIG_SDK_MODEL_MODE
    rtdrv_ext_modelCfg_t  modelTest_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    GETSOCKOPT(RTDRV_EXT_MODEL_DEBUG_MASK_GET, &modelTest_cfg, rtdrv_ext_modelCfg_t, 1);

    diag_util_mprintf("GRP_NONE: %s\n", (modelTest_cfg.debugFlagMask & GRP_NONE)?"YES":"NO");
    diag_util_mprintf("GRP_ALL: %s\n", (modelTest_cfg.debugFlagMask & GRP_ALL)?"YES":"NO");
    diag_util_mprintf("GRP_TBL: %s\n", (modelTest_cfg.debugFlagMask & GRP_TBL)?"YES":"NO");
    diag_util_mprintf("GRP_ACL: %s\n", (modelTest_cfg.debugFlagMask & GRP_ACL)?"YES":"NO");
    diag_util_mprintf("GRP_L2: %s\n", (modelTest_cfg.debugFlagMask & GRP_L2)?"YES":"NO");
    diag_util_mprintf("GRP_MIRROR: %s\n", (modelTest_cfg.debugFlagMask & GRP_MIRROR)?"YES":"NO");
    diag_util_mprintf("GRP_VLAN: %s\n", (modelTest_cfg.debugFlagMask & GRP_VLAN)?"YES":"NO");
    diag_util_mprintf("GRP_RMA: %s\n", (modelTest_cfg.debugFlagMask & GRP_RMA)?"YES":"NO");
    diag_util_mprintf("GRP_MSTP: %s\n", (modelTest_cfg.debugFlagMask & GRP_MSTP)?"YES":"NO");
    diag_util_mprintf("GRP_QOS: %s\n", (modelTest_cfg.debugFlagMask & GRP_QOS)?"YES":"NO");
    diag_util_mprintf("GRP_LA: %s\n", (modelTest_cfg.debugFlagMask & GRP_LA)?"YES":"NO");
    diag_util_mprintf("GRP_RSPAN: %s\n", (modelTest_cfg.debugFlagMask & GRP_RSPAN)?"YES":"NO");
    diag_util_mprintf("GRP_CFM: %s\n", (modelTest_cfg.debugFlagMask & GRP_CFM)?"YES":"NO");
    diag_util_mprintf("GRP_PORTISOLATION: %s\n", (modelTest_cfg.debugFlagMask & GRP_PORTISOLATION)?"YES":"NO");
    diag_util_mprintf("GRP_SPECIALTRAP: %s\n", (modelTest_cfg.debugFlagMask & GRP_SPECIALTRAP)?"YES":"NO");
    diag_util_mprintf("GRP_ATKPRVNT: %s\n", (modelTest_cfg.debugFlagMask & GRP_ATKPRVNT)?"YES":"NO");
    diag_util_mprintf("GRP_AVB: %s\n", (modelTest_cfg.debugFlagMask & GRP_AVB)?"YES":"NO");
    diag_util_mprintf("GRP_OAM: %s\n", (modelTest_cfg.debugFlagMask & GRP_OAM)?"YES":"NO");
    diag_util_mprintf("GRP_DPM: %s\n", (modelTest_cfg.debugFlagMask & GRP_DPM)?"YES":"NO");

#endif
    return CPARSER_OK;
}

/*
 * model ( ic-only | model-only | both ) <UINT:start> <UINT:end> */
cparser_result_t
cparser_cmd_model_ic_only_model_only_both_start_end(
    cparser_context_t *context,
    uint32_t  *start_ptr,
    uint32_t  *end_ptr)
{
#ifdef CONFIG_SDK_MODEL_MODE
    rtdrv_ext_modelCfg_t  modelTest_cfg;

    modelTest_cfg.startID = *start_ptr;
    modelTest_cfg.endID = *end_ptr;

    if ('i' == TOKEN_CHAR(1, 0))
        modelTest_cfg.caredType = CARE_TYPE_REAL;
    else if ('m' == TOKEN_CHAR(1, 0))
        modelTest_cfg.caredType = CARE_TYPE_MODEL;
    else
        modelTest_cfg.caredType = CARE_TYPE_BOTH;

    SETSOCKOPT(RTDRV_EXT_MODEL_TEST_SET, &modelTest_cfg, rtdrv_ext_modelCfg_t, 1);
#endif
    return CPARSER_OK;
}

/*
 * pktgen get tx-cmd
 */
cparser_result_t
cparser_cmd_pktgen_get_tx_cmd(cparser_context_t *context)
{
    uint32                  unit;
    rtdrv_ext_pktGenCfg_t   cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    cfg.unit = unit;
    GETSOCKOPT(RTDRV_EXT_PKTGEN_TX_CMD_GET, &cfg, rtdrv_ext_pktGenCfg_t, 1);

    diag_util_mprintf("\tPacket generation TX command : ");

    switch (cfg.status)
    {
        case 0:
            diag_util_mprintf("None\n");
            break;
        case 1:
            diag_util_mprintf("start\n");
            break;
        case 2:
            diag_util_mprintf("stop and reset counter\n");
            break;
        case 3:
            diag_util_mprintf("stop and hold counter\n");
            break;
        default:
            break;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_get_tx_cmd */

/*
 * pktgen set tx-cmd ( start | stop-and-reset-counter | stop-and-hold-counter )
 */
cparser_result_t
cparser_cmd_pktgen_set_tx_cmd_start_stop_and_reset_counter_stop_and_hold_counter(
    cparser_context_t *context)
{
    uint32                  unit;
    rtdrv_ext_pktGenCfg_t   cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    cfg.unit = unit;

    if ('a' == TOKEN_CHAR(3, 2))
        cfg.status = 1;
    else if ('r' == TOKEN_CHAR(3, 9))
        cfg.status = 2;
    else
        cfg.status = 3;

    SETSOCKOPT(RTDRV_EXT_PKTGEN_TX_CMD_SET, &cfg, rtdrv_ext_pktGenCfg_t, 1);

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_set_tx_cmd_start_stop_and_reset_counter_stop_and_hold_counter */

/*
 * pktgen get state
 */
cparser_result_t
cparser_cmd_pktgen_get_state(
    cparser_context_t *context)
{
    uint32                  unit;
    rtdrv_ext_pktGenCfg_t   cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    cfg.unit = unit;
    GETSOCKOPT(RTDRV_EXT_PKTGEN_STATE_GET, &cfg, rtdrv_ext_pktGenCfg_t, 1);

    diag_util_mprintf("\tPacket generation status : ");
    if (0 == cfg.enable)
        diag_util_mprintf("Disabled\n");
    else if (1 == cfg.enable)
        diag_util_mprintf("Enabled\n");
    else
        diag_util_mprintf("\n");

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_get_state */

/*
 * pktgen set state ( enable | disable )
 */
cparser_result_t
cparser_cmd_pktgen_set_state_enable_disable(
    cparser_context_t *context)
{
    uint32                  unit;
    rtdrv_ext_pktGenCfg_t   cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    cfg.unit = unit;
    switch(TOKEN_CHAR(3,0))
    {
        case 'd': /*disable*/
            cfg.enable = DISABLED;
            break;
        case 'e': /*enable*/
            cfg.enable = ENABLED;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }
    SETSOCKOPT(RTDRV_EXT_PKTGEN_STATE_SET, &cfg, rtdrv_ext_pktGenCfg_t, 1);

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_set_state_enable_disable */

/*
 * pktgen get port ( <PORT_LIST:ports> | all ) state
 */
cparser_result_t
cparser_cmd_pktgen_get_port_ports_all_state(
    cparser_context_t *context,
    char **ports_ptr)
{
    uint32                  unit, ret;
    rtdrv_ext_pktGenCfg_t   cfg;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    cfg.unit = unit;

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        GETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_STATE_GET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
        diag_util_mprintf("Port %2d : \n", port);
        diag_util_mprintf("\tTX state : ");
        if (0 == cfg.enable)
            diag_util_mprintf("Disabled\n");
        else if (1 == cfg.enable)
            diag_util_mprintf("Enabled\n");
        else
            diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_get_port_ports_all_state */

/*
 * pktgen set port ( <PORT_LIST:ports> | all ) state ( enable | disable )
 */
cparser_result_t
cparser_cmd_pktgen_set_port_ports_all_state_enable_disable(
    cparser_context_t *context,
    char **ports_ptr)
{
    uint32                  unit;
    rtdrv_ext_pktGenCfg_t   cfg;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    cfg.unit = unit;
    switch(TOKEN_CHAR(5,0))
    {
        case 'd': /*disable*/
            cfg.enable = DISABLED;
            break;
        case 'e': /*enable*/
            cfg.enable = ENABLED;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        SETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_STATE_SET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_set_port_ports_all_state_enable_disable */

/*
 * pktgen get port ( <PORT_LIST:ports> | all ) tx-done state
 */
cparser_result_t
cparser_cmd_pktgen_get_port_ports_all_tx_done_state(
    cparser_context_t *context,
    char **ports_ptr)
{
    uint32                  unit;
    int32                   ret;
    rtdrv_ext_pktGenCfg_t   cfg;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    cfg.unit = unit;
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        GETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_TX_DONE_STATE_GET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
        diag_util_mprintf("Port %2d : \n", port);
        diag_util_mprintf("\tTX done state : ");
        if (0 == cfg.enable)
            diag_util_mprintf("Normal\n");
        else if (1 == cfg.enable)
            diag_util_mprintf("Finished\n");
        else
            diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_get_port_ports_all_tx_done_state */

/*
 * pktgen get port ( <PORT_LIST:ports> | all ) frag-pkt action
 */
cparser_result_t
cparser_cmd_pktgen_get_port_ports_all_frag_pkt_action(
    cparser_context_t *context,
    char **ports_ptr)
{
    uint32                  unit;
    int32                   ret;
    rtdrv_ext_pktGenCfg_t   cfg;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    cfg.unit = unit;
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        GETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_FRAG_PKT_ACTION_GET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
        diag_util_mprintf("Port %2d : \n", port);
        diag_util_mprintf("\tFragment packet action : ");
        if (0 == cfg.action)
            diag_util_mprintf("Drop\n");
        else if (1 == cfg.action)
            diag_util_mprintf("Trap\n");
        else
            diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_get_port_ports_all_frag_pkt_action */

/*
 * pktgen set port ( <PORT_LIST:ports> | all ) frag-pkt action ( drop | trap )
 */
cparser_result_t
cparser_cmd_pktgen_set_port_ports_all_frag_pkt_action_drop_trap(
    cparser_context_t *context,
    char **ports_ptr)
{
    uint32                  unit;
    rtdrv_ext_pktGenCfg_t   cfg;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    cfg.unit = unit;
    switch(TOKEN_CHAR(6,0))
    {
        case 'd':
            cfg.action = 0;
            break;
        case 't':
            cfg.action = 1;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        SETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_FRAG_PKT_ACTION_SET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_set_port_ports_all_frag_pkt_action_drop_trap */

/*
 * pktgen get port ( <PORT_LIST:ports> | all ) oversize-pkt action
 */
cparser_result_t
cparser_cmd_pktgen_get_port_ports_all_oversize_pkt_action(
    cparser_context_t *context,
    char **ports_ptr)
{
    uint32                  unit;
    int32                   ret;
    rtdrv_ext_pktGenCfg_t   cfg;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    cfg.unit = unit;
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        GETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_OVERSIZE_PKT_ACTION_GET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
        diag_util_mprintf("Port %2d : \n", port);
        diag_util_mprintf("\tOver size packet action : ");
        if (0 == cfg.action)
            diag_util_mprintf("Drop\n");
        else if (1 == cfg.action)
            diag_util_mprintf("Trap\n");
        else
            diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_get_port_ports_all_oversize_pkt_action */

/*
 * pktgen set port ( <PORT_LIST:ports> | all ) oversize-pkt action ( drop | trap )
 */
cparser_result_t
cparser_cmd_pktgen_set_port_ports_all_oversize_pkt_action_drop_trap(
    cparser_context_t *context,
    char **ports_ptr)
{
    uint32                  unit;
    rtdrv_ext_pktGenCfg_t   cfg;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    cfg.unit = unit;
    switch(TOKEN_CHAR(6,0))
    {
        case 'd':
            cfg.action = 0;
            break;
        case 't':
            cfg.action = 1;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        SETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_OVERSIZE_PKT_ACTION_SET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_set_port_ports_all_oversize_pkt_action_drop_trap */

/*
 * pktgen get port ( <PORT_LIST:ports> | all ) undersize-pkt action
 */
cparser_result_t
cparser_cmd_pktgen_get_port_ports_all_undersize_pkt_action(
    cparser_context_t *context,
    char **ports_ptr)
{
    uint32                  unit;
    int32                   ret;
    rtdrv_ext_pktGenCfg_t   cfg;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    cfg.unit = unit;
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        GETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_UNDERSIZE_PKT_ACTION_GET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
        diag_util_mprintf("Port %2d : \n", port);
        diag_util_mprintf("\tUnder size packet action : ");
        if (0 == cfg.action)
            diag_util_mprintf("Drop\n");
        else if (1 == cfg.action)
            diag_util_mprintf("Trap\n");
        else
            diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_get_port_ports_all_undersize_pkt_action */

/*
 * pktgen set port ( <PORT_LIST:ports> | all ) undersize-pkt action ( drop | trap )
 */
cparser_result_t
cparser_cmd_pktgen_set_port_ports_all_undersize_pkt_action_drop_trap(
    cparser_context_t *context,
    char **ports_ptr)
{
    uint32                  unit;
    rtdrv_ext_pktGenCfg_t   cfg;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    cfg.unit = unit;
    switch(TOKEN_CHAR(6,0))
    {
        case 'd':
            cfg.action = 0;
            break;
        case 't':
            cfg.action = 1;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        SETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_UNDERSIZE_PKT_ACTION_SET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_set_port_ports_all_undersize_pkt_action_drop_trap */

/*
 * pktgen get port ( <PORT_LIST:ports> | all ) bad-crc-pkt action
 */
cparser_result_t
cparser_cmd_pktgen_get_port_ports_all_bad_crc_pkt_action(
    cparser_context_t *context,
    char **ports_ptr)
{
    uint32                  unit;
    int32                   ret;
    rtdrv_ext_pktGenCfg_t   cfg;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    cfg.unit = unit;
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        GETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_BADCRC_PKT_ACTION_GET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
        diag_util_mprintf("Port %2d : \n", port);
        diag_util_mprintf("\tBad CRC packet action : ");
        if (0 == cfg.action)
            diag_util_mprintf("Drop\n");
        else if (1 == cfg.action)
            diag_util_mprintf("Trap\n");
        else
            diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_get_port_ports_all_bad_crc_pkt_action */

/*
 * pktgen set port ( <PORT_LIST:ports> | all ) bad-crc-pkt action ( drop | trap )
 */
cparser_result_t
cparser_cmd_pktgen_set_port_ports_all_bad_crc_pkt_action_drop_trap(
    cparser_context_t *context,
    char **ports_ptr)
{
    uint32                  unit;
    rtdrv_ext_pktGenCfg_t   cfg;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    cfg.unit = unit;
    switch(TOKEN_CHAR(6,0))
    {
        case 'd':
            cfg.action = 0;
            break;
        case 't':
            cfg.action = 1;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        SETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_BADCRC_PKT_ACTION_SET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_set_port_ports_all_bad_crc_pkt_action_drop_trap */

/*
 * pktgen get port ( <PORT_LIST:ports> | all ) tx-pkt-cnt
 */
cparser_result_t
cparser_cmd_pktgen_get_port_ports_all_tx_pkt_cnt(
    cparser_context_t *context)
{
    uint32                  unit;
    int32                   ret;
    rtdrv_ext_pktGenCfg_t   cfg;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    cfg.unit = unit;
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        GETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_TX_PKT_CNT_GET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
        diag_util_mprintf("Port %2d TX packet counter: %d\n", port, cfg.status);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_get_port_ports_all_tx_pkt_cnt */

/*
 * pktgen set port ( <PORT_LIST:ports> | all ) tx-pkt-cnt <UINT:pktcnt>
 */
cparser_result_t
cparser_cmd_pktgen_set_port_ports_all_tx_pkt_cnt_pktcnt(
    cparser_context_t *context,
    uint32_t *pktcnt_ptr)
{
    uint32                  unit;
    rtdrv_ext_pktGenCfg_t   cfg;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    cfg.unit = unit;
    cfg.status = *pktcnt_ptr;

    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        SETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_TX_PKT_CNT_SET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_set_port_ports_all_tx_pkt_cnt_pktcnt */

#define DIAG_EXT_PKTGEN_STREAM_MAX      2

/*
 * pktgen get port ( <PORT_LIST:ports> | all ) stream <UINT:stream_idx> bad-crc state
 */
cparser_result_t
cparser_cmd_pktgen_get_port_ports_all_stream_stream_idx_bad_crc_state(
    cparser_context_t *context,
    uint32_t *stream_idx_ptr)
{
    uint32                  unit;
    int32                   ret;
    rtdrv_ext_pktGenCfg_t   cfg;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (*stream_idx_ptr >= DIAG_EXT_PKTGEN_STREAM_MAX)
    {
        diag_util_printf("User stream config: Error!\n");
        return CPARSER_NOT_OK;
    }

    cfg.unit = unit;
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        cfg.stream_idx = *stream_idx_ptr;
        GETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_BADCRC_STATE_GET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
        diag_util_mprintf("Port %2d stream %d TX bad CRC state: ",
                port, *stream_idx_ptr);

        if (0 == cfg.status)
            diag_util_mprintf("Disabled\n");
        else if (1 == cfg.status)
            diag_util_mprintf("Enabled\n");
        else
            diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_get_port_ports_all_bad_crc_state */

/*
 * pktgen set port ( <PORT_LIST:ports> | all ) stream <UINT:stream_idx> bad-crc state ( enable | disable )
 */
cparser_result_t
cparser_cmd_pktgen_set_port_ports_all_stream_stream_idx_bad_crc_state_enable_disable(
    cparser_context_t *context,
    uint32_t *stream_idx_ptr)
{
    uint32                  unit;
    rtdrv_ext_pktGenCfg_t   cfg;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (*stream_idx_ptr >= DIAG_EXT_PKTGEN_STREAM_MAX)
    {
        diag_util_printf("User stream config: Error!\n");
        return CPARSER_NOT_OK;
    }

    cfg.unit = unit;
    cfg.stream_idx = *stream_idx_ptr;
    switch(TOKEN_CHAR(8,0))
    {
        case 'd': /*disable*/
            cfg.status = DISABLED;
            break;
        case 'e': /*enable*/
            cfg.status = ENABLED;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        SETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_BADCRC_STATE_SET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_set_port_ports_all_stream_stream_idx_bad_crc_state_enable_disable */

/*
 * pktgen get port ( <PORT_LIST:ports> | all ) stream <UINT:stream_idx> len-type
 */
cparser_result_t
cparser_cmd_pktgen_get_port_ports_all_stream_stream_idx_len_type(
    cparser_context_t *context,
    uint32_t *stream_idx_ptr)
{
    uint32                  unit;
    int32                   ret;
    rtdrv_ext_pktGenCfg_t   cfg;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (*stream_idx_ptr >= DIAG_EXT_PKTGEN_STREAM_MAX)
    {
        diag_util_printf("User stream config: Error!\n");
        return CPARSER_NOT_OK;
    }

    cfg.unit = unit;
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        cfg.stream_idx = *stream_idx_ptr;
        GETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_STREAM_LENTYPE_GET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
        diag_util_mprintf("Port %2d stream %d packet length type: ",
                port, *stream_idx_ptr);

        if (0 == cfg.status)
            diag_util_mprintf("Fixed\n");
        else if (1 == cfg.status)
            diag_util_mprintf("Random\n");
        else if (2 == cfg.status)
            diag_util_mprintf("Increamental\n");
        else
            diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_get_port_ports_all_stream_stream_idx_len_type */

/*
 * pktgen set port ( <PORT_LIST:ports> | all ) stream <UINT:stream_idx> len-type ( fixed | random | increamental )
 */
cparser_result_t
cparser_cmd_pktgen_set_port_ports_all_stream_stream_idx_len_type_fixed_random_increamental(
    cparser_context_t *context,
    uint32_t *stream_idx_ptr)
{
    uint32                  unit;
    rtdrv_ext_pktGenCfg_t   cfg;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (*stream_idx_ptr >= DIAG_EXT_PKTGEN_STREAM_MAX)
    {
        diag_util_printf("User stream config: Error!\n");
        return CPARSER_NOT_OK;
    }

    cfg.unit = unit;
    cfg.stream_idx = *stream_idx_ptr;
    switch(TOKEN_CHAR(7,0))
    {
        case 'f':
            cfg.status = 0;
            break;
        case 'r':
            cfg.status = 1;
            break;
        case 'i':
            cfg.status = 2;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        SETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_STREAM_LENTYPE_SET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_set_port_ports_all_stream_stream_idx_len_type_fixed_random_increamental */

/*
 * pktgen get port ( <PORT_LIST:ports> | all ) stream <UINT:stream_idx> random-content state
 */
cparser_result_t
cparser_cmd_pktgen_get_port_ports_all_stream_stream_idx_random_content_state(
    cparser_context_t *context,
    uint32_t *stream_idx_ptr)
{
    uint32                  unit;
    int32                   ret;
    rtdrv_ext_pktGenCfg_t   cfg;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (*stream_idx_ptr >= DIAG_EXT_PKTGEN_STREAM_MAX)
    {
        diag_util_printf("User stream config: Error!\n");
        return CPARSER_NOT_OK;
    }

    cfg.unit = unit;
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        cfg.stream_idx = *stream_idx_ptr;
        GETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_STREAM_RANDOM_CONTENT_STATE_GET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
        diag_util_mprintf("Port %2d stream %d random content state: ",
                port, *stream_idx_ptr);

        if (0 == cfg.status)
            diag_util_mprintf("Disabled\n");
        else if (1 == cfg.status)
            diag_util_mprintf("Enabled\n");
        else
            diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_get_port_ports_all_stream_stream_idx_random_content_state */

/*
 * pktgen set port ( <PORT_LIST:ports> | all ) stream <UINT:stream_idx> random-content state ( enable | disable )
 */
cparser_result_t
cparser_cmd_pktgen_set_port_ports_all_stream_stream_idx_random_content_state_enable_disable(
    cparser_context_t *context,
    uint32_t *stream_idx_ptr)
{
    uint32                  unit;
    rtdrv_ext_pktGenCfg_t   cfg;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (*stream_idx_ptr >= DIAG_EXT_PKTGEN_STREAM_MAX)
    {
        diag_util_printf("User stream config: Error!\n");
        return CPARSER_NOT_OK;
    }

    cfg.unit = unit;
    cfg.stream_idx = *stream_idx_ptr;
    switch(TOKEN_CHAR(8,0))
    {
        case 'd': /*disable*/
            cfg.status = DISABLED;
            break;
        case 'e': /*enable*/
            cfg.status = ENABLED;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        SETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_STREAM_RANDOM_CONTENT_STATE_SET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_set_port_ports_all_stream_stream_idx_random_content_state_enable_disable */

/*
 * pktgen get port ( <PORT_LIST:ports> | all ) stream <UINT:stream_idx> random-offset
 */
cparser_result_t
cparser_cmd_pktgen_get_port_ports_all_stream_stream_idx_random_offset(
    cparser_context_t *context,
    uint32_t *stream_idx_ptr)
{
    uint32                  unit;
    int32                   ret;
    rtdrv_ext_pktGenCfg_t   cfg;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (*stream_idx_ptr >= DIAG_EXT_PKTGEN_STREAM_MAX)
    {
        diag_util_printf("User stream config: Error!\n");
        return CPARSER_NOT_OK;
    }

    cfg.unit = unit;
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        cfg.stream_idx = *stream_idx_ptr;
        GETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_STREAM_RANDOM_OFFSET_GET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
        diag_util_mprintf("Port %2d stream %d random content offset: %d\n",
                port, *stream_idx_ptr, cfg.status);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_get_port_ports_all_stream_stream_idx_random_offset */

/*
 * pktgen set port ( <PORT_LIST:ports> | all ) stream <UINT:stream_idx> random-offset <UINT:offset>
 */
cparser_result_t
cparser_cmd_pktgen_set_port_ports_all_stream_stream_idx_random_offset_offset(
    cparser_context_t *context,
    uint32_t *stream_idx_ptr,
    uint32_t *offset_ptr)
{
    uint32                  unit;
    rtdrv_ext_pktGenCfg_t   cfg;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (*stream_idx_ptr >= DIAG_EXT_PKTGEN_STREAM_MAX)
    {
        diag_util_printf("User stream config: Error!\n");
        return CPARSER_NOT_OK;
    }

    cfg.unit = unit;
    cfg.stream_idx = *stream_idx_ptr;
    cfg.status = *offset_ptr;
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        SETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_STREAM_RANDOM_OFFSET_SET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_set_port_ports_all_stream_stream_idx_random_offset_offset */

/*
 * pktgen get port ( <PORT_LIST:ports> | all ) stream <UINT:stream_idx> sa-inc state
 */
cparser_result_t
cparser_cmd_pktgen_get_port_ports_all_stream_stream_idx_sa_inc_state(
    cparser_context_t *context,
    uint32_t *stream_idx_ptr)
{
    uint32                  unit;
    int32                   ret;
    rtdrv_ext_pktGenCfg_t   cfg;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (*stream_idx_ptr >= DIAG_EXT_PKTGEN_STREAM_MAX)
    {
        diag_util_printf("User stream config: Error!\n");
        return CPARSER_NOT_OK;
    }

    cfg.unit = unit;
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        cfg.stream_idx = *stream_idx_ptr;
        GETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_STREAM_SA_INC_STATE_GET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
        diag_util_mprintf("Port %2d stream %d SA increament state: ",
                port, *stream_idx_ptr);

        if (0 == cfg.status)
            diag_util_mprintf("Disabled\n");
        else if (1 == cfg.status)
            diag_util_mprintf("Enabled\n");
        else
            diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_get_port_ports_all_stream_stream_idx_sa_inc_state */

/*
 * pktgen set port ( <PORT_LIST:ports> | all ) stream <UINT:stream_idx> sa-inc state ( enable | disable )
 */
cparser_result_t
cparser_cmd_pktgen_set_port_ports_all_stream_stream_idx_sa_inc_state_enable_disable(
    cparser_context_t *context,
    uint32_t *stream_idx_ptr)
{
    uint32                  unit;
    rtdrv_ext_pktGenCfg_t   cfg;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (*stream_idx_ptr >= DIAG_EXT_PKTGEN_STREAM_MAX)
    {
        diag_util_printf("User stream config: Error!\n");
        return CPARSER_NOT_OK;
    }

    cfg.unit = unit;
    cfg.stream_idx = *stream_idx_ptr;
    switch(TOKEN_CHAR(8,0))
    {
        case 'd': /*disable*/
            cfg.status = DISABLED;
            break;
        case 'e': /*enable*/
            cfg.status = ENABLED;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        SETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_STREAM_SA_INC_STATE_SET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_set_port_ports_all_stream_stream_idx_sa_inc_state_enable_disable */

/*
 * pktgen get port ( <PORT_LIST:ports> | all ) stream <UINT:stream_idx> da-inc state
 */
cparser_result_t
cparser_cmd_pktgen_get_port_ports_all_stream_stream_idx_da_inc_state(
    cparser_context_t *context,
    uint32_t *stream_idx_ptr)
{
    uint32                  unit;
    int32                   ret;
    rtdrv_ext_pktGenCfg_t   cfg;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (*stream_idx_ptr >= DIAG_EXT_PKTGEN_STREAM_MAX)
    {
        diag_util_printf("User stream config: Error!\n");
        return CPARSER_NOT_OK;
    }

    cfg.unit = unit;
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        cfg.stream_idx = *stream_idx_ptr;
        GETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_STREAM_DA_INC_STATE_GET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
        diag_util_mprintf("Port %2d stream %d DA increament state: ",
                port, *stream_idx_ptr);

        if (0 == cfg.status)
            diag_util_mprintf("Disabled\n");
        else if (1 == cfg.status)
            diag_util_mprintf("Enabled\n");
        else
            diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_get_port_ports_all_stream_stream_idx_da_inc_state */

/*
 * pktgen set port ( <PORT_LIST:ports> | all ) stream <UINT:stream_idx> da-inc state ( enable | disable )
 */
cparser_result_t
cparser_cmd_pktgen_set_port_ports_all_stream_stream_idx_da_inc_state_enable_disable(
    cparser_context_t *context,
    uint32_t *stream_idx_ptr)
{
    uint32                  unit;
    rtdrv_ext_pktGenCfg_t   cfg;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (*stream_idx_ptr >= DIAG_EXT_PKTGEN_STREAM_MAX)
    {
        diag_util_printf("User stream config: Error!\n");
        return CPARSER_NOT_OK;
    }

    cfg.unit = unit;
    cfg.stream_idx = *stream_idx_ptr;
    switch(TOKEN_CHAR(8,0))
    {
        case 'd': /*disable*/
            cfg.status = DISABLED;
            break;
        case 'e': /*enable*/
            cfg.status = ENABLED;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        SETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_STREAM_DA_INC_STATE_SET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_set_port_ports_all_stream_stream_idx_da_inc_state_enable_disable */

/*
 * pktgen get port ( <PORT_LIST:ports> | all ) stream <UINT:stream_idx> tx-pkt-cnt
 */
cparser_result_t
cparser_cmd_pktgen_get_port_ports_all_stream_stream_idx_tx_pkt_cnt(
    cparser_context_t *context,
    uint32_t *stream_idx_ptr)
{
    uint32                  unit;
    int32                   ret;
    rtdrv_ext_pktGenCfg_t   cfg;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (*stream_idx_ptr >= DIAG_EXT_PKTGEN_STREAM_MAX)
    {
        diag_util_printf("User stream config: Error!\n");
        return CPARSER_NOT_OK;
    }

    cfg.unit = unit;
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        cfg.stream_idx = *stream_idx_ptr;
        GETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_STREAM_TX_PKT_CNT_GET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
        diag_util_mprintf("Port %2d stream %d TX packet counter: %d\n",
                port, *stream_idx_ptr, cfg.status);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_get_port_ports_all_stream_stream_idx_tx_pkt_cnt */

/*
 * pktgen set port ( <PORT_LIST:ports> | all ) stream <UINT:stream_idx> tx-pkt-cnt <UINT:pktcnt>
 */
cparser_result_t
cparser_cmd_pktgen_set_port_ports_all_stream_stream_idx_tx_pkt_cnt_pktcnt(
    cparser_context_t *context,
    uint32_t *stream_idx_ptr,
    uint32_t *pktcnt_ptr)
{
    uint32                  unit;
    rtdrv_ext_pktGenCfg_t   cfg;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (*stream_idx_ptr >= DIAG_EXT_PKTGEN_STREAM_MAX)
    {
        diag_util_printf("User stream config: Error!\n");
        return CPARSER_NOT_OK;
    }

    cfg.unit = unit;
    cfg.stream_idx = *stream_idx_ptr;
    cfg.status = *pktcnt_ptr;

    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        SETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_STREAM_TX_PKT_CNT_SET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_set_port_ports_all_stream_stream_idx_tx_pkt_cnt_pktcnt */

/*
 * pktgen get port ( <PORT_LIST:ports> | all ) stream <UINT:stream_idx> pkt-len
 */
cparser_result_t
cparser_cmd_pktgen_get_port_ports_all_stream_stream_idx_pkt_len(
    cparser_context_t *context,
    uint32_t *stream_idx_ptr)
{
    uint32                  unit;
    rtdrv_ext_pktGenCfg_t   cfg;
    int32                   ret;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (*stream_idx_ptr >= DIAG_EXT_PKTGEN_STREAM_MAX)
    {
        diag_util_printf("User stream config: Error!\n");
        return CPARSER_NOT_OK;
    }

    cfg.unit = unit;
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        cfg.stream_idx = *stream_idx_ptr;
        GETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_STREAM_PKT_LEN_GET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
        diag_util_mprintf("Port %2d stream %d TX packet length start: %d end: %d\n",
                port, *stream_idx_ptr, cfg.pktlen_start, cfg.pktlen_end);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_get_port_ports_all_stream_stream_idx_pkt_len */

/*
 * pktgen set port ( <PORT_LIST:ports> | all ) stream <UINT:stream_idx> pkt-len start <UINT:start_val> end <UINT:end_val>
 */
cparser_result_t
cparser_cmd_pktgen_set_port_ports_all_stream_stream_idx_pkt_len_start_start_val_end_end_val(
    cparser_context_t *context,
    uint32_t *stream_idx_ptr,
    uint32_t *start_val_ptr,
    uint32_t *end_val_ptr)
{
    uint32                  unit;
    rtdrv_ext_pktGenCfg_t   cfg;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (*stream_idx_ptr >= DIAG_EXT_PKTGEN_STREAM_MAX)
    {
        diag_util_printf("User stream config: Error!\n");
        return CPARSER_NOT_OK;
    }

    cfg.unit = unit;
    cfg.stream_idx = *stream_idx_ptr;
    cfg.pktlen_start = *start_val_ptr;
    cfg.pktlen_end = *end_val_ptr;

    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        SETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_STREAM_PKT_LEN_SET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_set_port_ports_all_stream_stream_idx_pkt_len_start_start_val_end_end_val */

/*
 * pktgen get port ( <PORT_LIST:ports> | all ) stream <UINT:stream_idx> sa-repeat-cnt
 */
cparser_result_t
cparser_cmd_pktgen_get_port_ports_all_stream_stream_idx_sa_repeat_cnt(
    cparser_context_t *context,
    uint32_t *stream_idx_ptr)
{
    uint32                  unit;
    int32                   ret;
    rtdrv_ext_pktGenCfg_t   cfg;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (*stream_idx_ptr >= DIAG_EXT_PKTGEN_STREAM_MAX)
    {
        diag_util_printf("User stream config: Error!\n");
        return CPARSER_NOT_OK;
    }

    cfg.unit = unit;
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        cfg.stream_idx = *stream_idx_ptr;
        GETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_STREAM_SA_REPEAT_CNT_GET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
        diag_util_mprintf("Port %2d stream %d SA repeat counter: %d\n",
                port, *stream_idx_ptr, cfg.status);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_get_port_ports_all_stream_stream_idx_sa_repeat_cnt */

/*
 * pktgen set port ( <PORT_LIST:ports> | all ) stream <UINT:stream_idx> sa-repeat-cnt <UINT:repeat_cnt>
 */
cparser_result_t
cparser_cmd_pktgen_set_port_ports_all_stream_stream_idx_sa_repeat_cnt_repeat_cnt(
    cparser_context_t *context,
    uint32_t *stream_idx_ptr,
    uint32_t *repeat_cnt_ptr)
{
    uint32                  unit;
    rtdrv_ext_pktGenCfg_t   cfg;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (*stream_idx_ptr >= DIAG_EXT_PKTGEN_STREAM_MAX)
    {
        diag_util_printf("User stream config: Error!\n");
        return CPARSER_NOT_OK;
    }

    cfg.unit = unit;
    cfg.stream_idx = *stream_idx_ptr;
    cfg.status = *repeat_cnt_ptr;

    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        SETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_STREAM_SA_REPEAT_CNT_SET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_set_port_ports_all_stream_stream_idx_sa_repeat_cnt_repeat_cnt */

/*
 * pktgen get port ( <PORT_LIST:ports> | all ) stream <UINT:stream_idx> da-repeat-cnt
 */
cparser_result_t
cparser_cmd_pktgen_get_port_ports_all_stream_stream_idx_da_repeat_cnt(
    cparser_context_t *context,
    uint32_t *stream_idx_ptr)
{
    uint32                  unit;
    rtdrv_ext_pktGenCfg_t   cfg;
    int32                   ret;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (*stream_idx_ptr >= DIAG_EXT_PKTGEN_STREAM_MAX)
    {
        diag_util_printf("User stream config: Error!\n");
        return CPARSER_NOT_OK;
    }

    cfg.unit = unit;
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        cfg.stream_idx = *stream_idx_ptr;
        GETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_STREAM_DA_REPEAT_CNT_GET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
        diag_util_mprintf("Port %2d stream %d DA repeat counter: %d\n",
                port, *stream_idx_ptr, cfg.status);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_get_port_ports_all_stream_stream_idx_da_repeat_cnt */

/*
 * pktgen set port ( <PORT_LIST:ports> | all ) stream <UINT:stream_idx> da-repeat-cnt <UINT:repeat_cnt>
 */
cparser_result_t
cparser_cmd_pktgen_set_port_ports_all_stream_stream_idx_da_repeat_cnt_repeat_cnt(
    cparser_context_t *context,
    uint32_t *stream_idx_ptr,
    uint32_t *repeat_cnt_ptr)
{
    uint32                  unit;
    rtdrv_ext_pktGenCfg_t   cfg;
    rtk_port_t              port;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (*stream_idx_ptr >= DIAG_EXT_PKTGEN_STREAM_MAX)
    {
        diag_util_printf("User stream config: Error!\n");
        return CPARSER_NOT_OK;
    }

    cfg.unit = unit;
    cfg.stream_idx = *stream_idx_ptr;
    cfg.status = *repeat_cnt_ptr;

    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        cfg.port = port;
        SETSOCKOPT(RTDRV_EXT_PKTGEN_PORT_STREAM_DA_REPEAT_CNT_SET, &cfg,
                rtdrv_ext_pktGenCfg_t, 1);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_pktgen_set_port_ports_all_stream_stream_idx_da_repeat_cnt_repeat_cnt */

/* notification dump counter */
cparser_result_t cparser_cmd_notification_dump_counter(cparser_context_t *context)
{
    rtdrv_ext_l2Cfg_t   l2_cfg;

    DIAG_OM_GET_CHIP_ID(l2_cfg.unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    GETSOCKOPT(RTDRV_EXT_L2_NOTIFICATION_DUMP_COUNTER, &l2_cfg, rtdrv_ext_l2Cfg_t, 1);

    return CPARSER_OK;
}

/* notification dump ring */
cparser_result_t cparser_cmd_notification_dump_ring(cparser_context_t *context)
{
    rtdrv_ext_l2Cfg_t   l2_cfg;

    DIAG_OM_GET_CHIP_ID(l2_cfg.unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    GETSOCKOPT(RTDRV_EXT_L2_NOTIFICATION_DUMP_RING, &l2_cfg, rtdrv_ext_l2Cfg_t, 1);

    return CPARSER_OK;
}

/* notification set debug-flag <UINT:bitmask> */
cparser_result_t cparser_cmd_notification_set_debug_flag_bitmask(cparser_context_t *context,
    uint32_t *bitmask)
{
    rtdrv_ext_l2Cfg_t   l2_cfg;

    DIAG_OM_GET_CHIP_ID(l2_cfg.unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    l2_cfg.flag = *bitmask;
    SETSOCKOPT(RTDRV_EXT_L2_NOTIFICATION_DEBUG_SET, &l2_cfg, rtdrv_ext_l2Cfg_t, 1);

    return CPARSER_OK;
}

/* l2-table get aging-unit */
cparser_result_t cparser_cmd_l2_table_get_aging_unit(cparser_context_t *context)
{
    rtdrv_ext_l2Cfg_t   l2_cfg;

    DIAG_OM_GET_CHIP_ID(l2_cfg.unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("\tAging Time                                     : ");

    GETSOCKOPT(RTDRV_EXT_L2_AGING_UNIT_GET, &l2_cfg, rtdrv_ext_l2Cfg_t, 1);

    diag_util_mprintf("%d (0.1 seconds).\n", l2_cfg.aging_time);

    return CPARSER_OK;
}

/* l2-table set aging-unit <UINT:time> */
cparser_result_t cparser_cmd_l2_table_set_aging_unit_time(cparser_context_t *context,
    uint32_t *time_ptr)
{
    rtdrv_ext_l2Cfg_t   l2_cfg;

    DIAG_OM_GET_CHIP_ID(l2_cfg.unit);
    DIAG_UTIL_PARAM_CHK();
    l2_cfg.aging_time = *time_ptr;

    SETSOCKOPT(RTDRV_EXT_L2_AGING_UNIT_SET, &l2_cfg, rtdrv_ext_l2Cfg_t, 1);

    return CPARSER_OK;
}

/* l2-table clear table */
cparser_result_t cparser_cmd_l2_table_clear_table(cparser_context_t *context,
    uint32_t *time_ptr)
{
    rtdrv_ext_l2Cfg_t   l2_cfg;

    DIAG_OM_GET_CHIP_ID(l2_cfg.unit);
    DIAG_UTIL_PARAM_CHK();

    SETSOCKOPT(RTDRV_EXT_L2_TBL_CLEAR, &l2_cfg, rtdrv_ext_l2Cfg_t, 1);

    return CPARSER_OK;
}

/* acl get meter counter */
cparser_result_t cparser_cmd_acl_get_meter_counter(cparser_context_t *context)
{
    rtdrv_ext_aclCfg_t   acl_cfg;

    DIAG_OM_GET_CHIP_ID(acl_cfg.unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();



    GETSOCKOPT(RTDRV_EXT_ACL_METER_COUNTER_GET, &acl_cfg, rtdrv_ext_aclCfg_t, 1);
    diag_util_printf("Counter of Meter %d unit ", acl_cfg.index);
    switch(acl_cfg.counterUnit)
    {
        case 0:
            diag_util_mprintf("1Byte\n");
            break;
        case 1:
            diag_util_mprintf("16Byte\n");
            break;
        case 2:
            diag_util_mprintf("Packet\n");
            break;
    }

    diag_util_mprintf("GreenCounter %d\n", acl_cfg.greenCounter);
    diag_util_mprintf("YellowCounter %d\n", acl_cfg.yellowCounter);
    diag_util_mprintf("RedCounter %d\n", acl_cfg.redCounter);
    diag_util_mprintf("TotalCounter %d\n", acl_cfg.totalCounter);

    return CPARSER_OK;
}

/* acl set meter counter ( 1byte | 16byte | packet ) <UINT:index> */
cparser_result_t cparser_cmd_acl_set_meter_counter_1byte_16byte_packet_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    rtdrv_ext_aclCfg_t   acl_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(acl_cfg.unit);
    switch(TOKEN_CHAR(4,1))
    {
        case 'b':
            acl_cfg.counterUnit = 0;
            break;
        case '6':
            acl_cfg.counterUnit = 1;
            break;
        case 'a':
            acl_cfg.counterUnit = 2;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    acl_cfg.index = *index_ptr;
    acl_cfg.clear = 1;

    SETSOCKOPT(RTDRV_EXT_ACL_METER_COUNTER_SET, &acl_cfg, rtdrv_ext_aclCfg_t, 1);

    return CPARSER_OK;
}

/* acl reset meter counter */
cparser_result_t cparser_cmd_acl_reset_meter_counter(cparser_context_t *context)
{
    rtdrv_ext_aclCfg_t   acl_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(acl_cfg.unit);
    acl_cfg.clear = 1;
    SETSOCKOPT(RTDRV_EXT_ACL_METER_COUNTER_RESET, &acl_cfg, rtdrv_ext_aclCfg_t, 1);

    return CPARSER_OK;
}

/* time get ptp ( <PORT_LIST:ports> | all ) timestamp ( rx-sync | rx-dreq | rx-pdrq | rx-pdrp | tx-sync | tx-dreq | tx-pdrq | tx-pdrp ) */
cparser_result_t cparser_cmd_time_get_ptp_ports_all_timestamp_rx_sync_rx_dreq_rx_pdrq_rx_pdrp_tx_sync_tx_dreq_tx_pdrq_tx_pdrp(cparser_context_t *context)
{
    uint32_t            unit;
    rtk_port_t          port;
    diag_portlist_t     portlist;
    rtdrv_ext_timeCfg_t time_cfg;
    char                ts_str[8][8] = { "RX-SYNC", "RX-DREQ", "RX-PDRQ", "RX-PDRP", "TX-SYNC", "TX-DREQ", "TX-PDRQ", "TX-PDRP" };

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    time_cfg.unit = unit;
    /* Type of Timestamp */
    if ('r' == TOKEN_CHAR(5, 0))
    {   /* Rx */
        time_cfg.type = 0;
    }
    else
    {   /* Tx */
        time_cfg.type = 4;
    }

    if ('s' == TOKEN_CHAR(5, 3))
    {   /* Sync */
        time_cfg.type += 0;
    }
    else if ('d' == TOKEN_CHAR(5, 3))
    {   /* Delay Req */
        time_cfg.type += 1;
    }
    else if ('q' == TOKEN_CHAR(5, 6))
    {   /* Pdelay Req */
        time_cfg.type += 2;
    }
    else
    {   /* Pdelay Resp */
        time_cfg.type += 3;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        time_cfg.port = port;
        /* clear */
        time_cfg.sid = 0;
        time_cfg.sec = 0;
        time_cfg.nsec = 0;
        GETSOCKOPT(RTDRV_EXT_TIME_PORT_PTP_TIMESTAMP_GET, &time_cfg, rtdrv_ext_timeCfg_t, 1);
        diag_util_mprintf("\tPort: %2d\n", port);
        diag_util_mprintf("\t\t%s: SID = %5d, TS = %10u.%09u [0x%04X, 0x%08X, 0x%08X]\n",
            ts_str[time_cfg.type], time_cfg.sid, time_cfg.sec, time_cfg.nsec,
            time_cfg.sid, time_cfg.sec, time_cfg.nsec);
    }

    return CPARSER_OK;
}
