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
 * $Revision: 57050 $
 * $Date: 2015-03-23 14:36:24 +0800 (Mon, 23 Mar 2015) $
 *
 * Purpose : Define diag shell commands for switch
 *
 * Feature : The file includes the following module and sub-modules
 *           1) switch commands.
 *
 */

#include <common/debug/rt_log.h>
#include <common/rt_error.h>
#include <common/rt_type.h>
#include <common/rt_version.h>
#include <ioal/mem32.h>
#include <rtk/switch.h>
#include <rtk/port.h>
#include <diag_util.h>
#include <diag_om.h>
#include <diag_str.h>
#include <drv/watchdog/watchdog.h>
#include <parser/cparser_priv.h>

#ifdef CMD_SWITCH_GET_CHKSUM_ERR_TYPE_L2_L3_L4_PORT_PORTS_ALL_ACTION
/*
 * switch get chksum-err-type ( l2 | l3 | l4 ) port ( <PORT_LIST:ports> | all ) action
 */
cparser_result_t cparser_cmd_switch_get_chksum_err_type_l2_l3_l4_port_ports_all_action(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    uint8  *actStr[2] = {(uint8 *)"FORWARD", (uint8 *)"DROP"};
    rtk_switch_chksum_fail_t type = LAYER2_CHKSUM_FAIL;
    rtk_action_t action = ACTION_DROP;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    switch(TOKEN_CHAR(3,1))
    {
        case '2':
            type = LAYER2_CHKSUM_FAIL;
            break;

        case '3':
            type = LAYER3_CHKSUM_FAIL;
            break;

        case '4':
            type = LAYER4_CHKSUM_FAIL;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    diag_util_mprintf("Layer %c Checksum Error Action\n", TOKEN_CHAR(3,1));

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_switch_chksumFailAction_get(unit, port, type, &action), ret);
        diag_util_mprintf("\tPort %d : %s\n", port, actStr[action]);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_GET_CRC_RECAL_PORT_PORTS_ALL_STATE
/*
 * switch get crc-recal port ( <PORT_LIST:ports> | all ) state
 */
cparser_result_t cparser_cmd_switch_get_crc_recal_port_ports_all_state(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("CRC Re-Calculation Enable Status\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_switch_recalcCRCEnable_get(unit, port, &enable), ret);
        diag_util_mprintf("\tPort %d : %s\n", port, enable ? "ENABLE" : "DISABLE");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_GET_IPV4_ADDRESS
/*
 * switch get ipv4-address
 */
cparser_result_t cparser_cmd_switch_get_ipv4_address(cparser_context_t *context)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    uint8  ipv4Str[16];
    ipaddr_t ip;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(rtk_switch_IPv4Addr_get(unit, &ip), ret);
    DIAG_UTIL_ERR_CHK(diag_util_ip2str(ipv4Str, ip), ret);
    diag_util_printf("IPv4 address : %s\n", ipv4Str);

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_GET_IPV6_ADDRESS
/*
 * switch get ipv6-address
 */
cparser_result_t cparser_cmd_switch_get_ipv6_address(cparser_context_t *context)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    uint8  ipv6Str[32];
    rtk_ipv6_addr_t ip;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(rtk_switch_IPv6Addr_get(unit, &ip), ret);
    DIAG_UTIL_ERR_CHK(diag_util_ipv62str(ipv6Str, (uint8 *)&ip.ipv6_addr[0]), ret);
    diag_util_printf("IPv6 address : %s\n", ipv6Str);

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_GET_MAC_ADDRESS
/*
 * switch get mac-address
 */
cparser_result_t cparser_cmd_switch_get_mac_address(cparser_context_t *context)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    uint8 macStr[16];
    rtk_mac_t mac;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(rtk_switch_mgmtMacAddr_get(unit, &mac), ret);
    DIAG_UTIL_ERR_CHK(diag_util_mac2str(macStr, mac.octet), ret);
    diag_util_printf("MAC address : %s\n", macStr);

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_GET_CPU_MAX_PKT_LEN_RX_DIR_TX_DIR_LENGTH
/*
 * switch get cpu-max-pkt-len ( rx-dir | tx-dir ) length
 */
cparser_result_t cparser_cmd_switch_get_cpu_max_pkt_len_rx_dir_tx_dir_length(cparser_context_t *context)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    uint32 len;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if ('r' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(rtk_switch_cpuMaxPktLen_get(unit, PKTDIR_RX, &len), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_switch_cpuMaxPktLen_get(unit, PKTDIR_TX, &len), ret);
    }

    diag_util_printf("CPU Port Max Packet Length (%s) : %u\n", ('r' == TOKEN_CHAR(3,0))? "Rx" : "Tx", len);

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_GET_MAX_PKT_LEN_LENGTH
/*
 * switch get max-pkt-len length
 */
cparser_result_t cparser_cmd_switch_get_max_pkt_len_length(cparser_context_t *context)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    uint8 *lenStr[4] = { "1522", "1536", "1552", "9216" };
    rtk_switch_maxPktLen_t len;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(rtk_switch_maxPktLen_get(unit, &len), ret);
    diag_util_printf("System Max Packet Length : %s\n", lenStr[len]);

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_GET_MAX_PKT_LEN_FE_GE_LENGTH
/*
 * switch get max-pkt-len ( fe | ge ) length
 */
cparser_result_t cparser_cmd_switch_get_max_pkt_len_fe_ge_length(cparser_context_t *context)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    uint32 len;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    switch(TOKEN_CHAR(3,0))
    {
        case 'f':
            DIAG_UTIL_ERR_CHK(rtk_switch_maxPktLenLinkSpeed_get(unit, MAXPKTLEN_LINK_SPEED_FE, &len), ret);
            diag_util_printf("System Max Packet Length in 10M/100M Speed : %d\n", len);
            break;

        case 'g':
            DIAG_UTIL_ERR_CHK(rtk_switch_maxPktLenLinkSpeed_get(unit, MAXPKTLEN_LINK_SPEED_GE, &len), ret);
            diag_util_printf("System Max Packet Length in Giga Speed : %d\n", len);
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_GET_MAX_PKT_LEN_PORT_PORTS_ALL_LENGTH
/*
 * switch get max-pkt-len port ( <PORT_LIST:ports> | all ) length
 */
cparser_result_t cparser_cmd_switch_get_max_pkt_len_port_ports_all_length(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port, len;
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Per Port Max Packet Length\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_switch_portMaxPktLen_get(unit, port, &len), ret);
        diag_util_mprintf("\tPort %d : %d\n", port, len);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_GET_MAX_PKT_LEN_TAG_LENGTH_STATE
/*
 * switch get max-pkt-len tag-length-state
 */
cparser_result_t cparser_cmd_switch_get_max_pkt_len_tag_length_state(cparser_context_t *context)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_switch_maxPktLenTagLenCntIncEnable_get(unit, &enable), ret);
    diag_util_printf("Max Packet Length Tag Length Configuration : %s\n", (enable == ENABLED) ? "Include" : "Exclude");

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_GET_MGMT_VLAN_ID_INNER_OUTER
/*
 * switch get mgmt-vlan-id ( inner | outer )
 */
cparser_result_t cparser_cmd_switch_get_mgmt_vlan_id_inner_outer(cparser_context_t *context)
{
    uint32 unit, vid;
    int32  ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    switch(TOKEN_CHAR(3,0))
    {
        case 'i':
            DIAG_UTIL_ERR_CHK(rtk_switch_mgmtVlanId_get(unit, &vid), ret);
            break;

        case 'o':
            DIAG_UTIL_ERR_CHK(rtk_switch_outerMgmtVlanId_get(unit, &vid), ret);
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    diag_util_printf("MGMT VID(%s) : %d\n", TOKEN_STR(3), vid);

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_GET_PROBE_INFORMATION
/*
 * switch get probe-information
 */
cparser_result_t cparser_cmd_switch_get_probe_information(cparser_context_t *context)
{
    uint32                  unit = 0, port = 0, base_port;
    uint32                  chipStrIndex = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_switch_devInfo_t    devInfo;
    uint8  *chipNameStr[28] = {(uint8 *)"RTL8389M", (uint8 *)"RTL8389LM", (uint8 *)"RTL8329M", (uint8 *)"RTL8329",
                               (uint8 *)"RTL8377M", (uint8 *)"RTL8328M",  (uint8 *)"RTL8328S", (uint8 *)"RTL8328L",
                               (uint8 *)"RTL8352M", (uint8 *)"RTL8353M", (uint8 *)"RTL8392M",  (uint8 *)"RTL8393M",
                               (uint8 *)"RTL8396M", (uint8 *)"RTL8330M", (uint8 *)"RTL8380M",  (uint8 *)"RTL8332M",
                               (uint8 *)"RTL8382M", (uint8 *)"RTL8330MES", (uint8 *)"RTL8380MES",  (uint8 *)"RTL8332MES",
                               (uint8 *)"RTL8382MES", (uint8 *)"RTL8352MES", (uint8 *)"RTL8353MES", (uint8 *)"RTL8392MES",
                               (uint8 *)"RTL8393MES", (uint8 *)"RTL8396MES", (uint8 *)"RTL8391M", (uint8 *)"RTL83xx"};
    rtk_portmask_t          portmask;
    uint32                  value, data;
    uint32                  poll_portmask=0;
    uint32                  chip_mode = 0, cfg_chip_mode = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    memset(&devInfo, 0, sizeof(rtk_switch_devInfo_t));
    DIAG_UTIL_ERR_CHK(rtk_switch_deviceInfo_get(unit, &devInfo), ret);
    switch (devInfo.chipId)
    {
        case RTL8389M_CHIP_ID:
            chipStrIndex = 0;
            break;
        case RTL8389L_CHIP_ID:
            chipStrIndex = 1;
            break;
        case RTL8329M_CHIP_ID:
            chipStrIndex = 2;
            break;
        case RTL8329_CHIP_ID:
            chipStrIndex = 3;
            break;
        case RTL8377M_CHIP_ID:
            chipStrIndex = 4;
            break;
        case RTL8328M_CHIP_ID:
            chipStrIndex = 5;
            break;
        case RTL8328S_CHIP_ID:
            chipStrIndex = 6;
            break;
        case RTL8328L_CHIP_ID:
            chipStrIndex = 7;
            break;
        case RTL8352M_CHIP_ID:
            chipStrIndex = 8;
            break;
        case RTL8353M_CHIP_ID:
            chipStrIndex = 9;
            break;
        case RTL8392M_CHIP_ID:
            chipStrIndex = 10;
            break;
        case RTL8393M_CHIP_ID:
            chipStrIndex = 11;
            break;
        case RTL8396M_CHIP_ID:
            chipStrIndex = 12;
            break;
        case RTL8330M_CHIP_ID:
            chipStrIndex = 13;
            break;
        case RTL8380M_CHIP_ID:
            chipStrIndex = 14;
            break;
        case RTL8332M_CHIP_ID:
            chipStrIndex = 15;
            break;
        case RTL8382M_CHIP_ID:
            chipStrIndex = 16;
            break;
        case RTL8330MES_CHIP_ID:
            chipStrIndex = 17;
            break;
        case RTL8380MES_CHIP_ID:
            chipStrIndex = 18;
            break;
        case RTL8332MES_CHIP_ID:
            chipStrIndex = 19;
            break;
        case RTL8382MES_CHIP_ID:
            chipStrIndex = 20;
            break;
        case RTL8352MES_CHIP_ID:
            chipStrIndex = 21;
            break;
        case RTL8353MES_CHIP_ID:
            chipStrIndex = 22;
            break;
        case RTL8392MES_CHIP_ID:
            chipStrIndex = 23;
            break;
        case RTL8393MES_CHIP_ID:
            chipStrIndex = 24;
            break;
        case RTL8396MES_CHIP_ID:
            chipStrIndex = 25;
            break;
        case RTL8391M_CHIP_ID:
            chipStrIndex = 26;
            break;
        default:
            chipStrIndex = 27;
            break;
    }

    diag_util_mprintf("Chip ID: %x (%s)\n", devInfo.chipId, chipNameStr[chipStrIndex]);
    diag_util_mprintf("Family ID: %x\n", devInfo.familyId);
    diag_util_mprintf("Port Number: %2d\n", devInfo.port_number);
    if (devInfo.fe.portNum)
        diag_util_mprintf("FE Port Number: %2d, Minimum: %2d, Maximum: %2d\n", devInfo.fe.portNum, devInfo.fe.min, devInfo.fe.max);
    if (devInfo.ge.portNum)
        diag_util_mprintf("GE Port Number: %2d, Minimum: %2d, Maximum: %2d\n", devInfo.ge.portNum, devInfo.ge.min, devInfo.ge.max);
    if (devInfo.ge_combo.portNum)
        diag_util_mprintf("GE Combo Port Number: %2d, Minimum: %2d, Maximum: %2d\n", devInfo.ge_combo.portNum, devInfo.ge_combo.min, devInfo.ge_combo.max);
    if (devInfo.serdes.portNum)
        diag_util_mprintf("Serdes Port Number: %2d, Minimum: %2d, Maximum: %2d\n", devInfo.serdes.portNum, devInfo.serdes.min, devInfo.serdes.max);
    diag_util_mprintf("CPU Port : %d\n", devInfo.cpuPort);

    /* Display PHY chip information */
    diag_util_mprintf("\n");
    diag_util_mprintf("Port     PHY chip\n");
    diag_util_mprintf("======================\n");
    osal_memcpy(&portmask, &devInfo.ether.portmask, sizeof(rtk_portmask_t));
    for (port = (uint32)devInfo.ether.min; port <= (uint32)devInfo.ether.max; port++)
    {
        if (RTK_PORTMASK_IS_PORT_SET(devInfo.serdes.portmask, port))
        {
            if (RTK_PORTMASK_IS_PORT_SET(devInfo.ge_10ge.portmask, port))
                diag_util_mprintf(" %2d      Int 10GE Serdes\n", port);
            else if (RTK_PORTMASK_IS_PORT_SET(devInfo.ge.portmask, port))
                diag_util_mprintf(" %2d      Int GE Serdes\n", port);
            else
                diag_util_mprintf(" %2d      Int FE Serdes\n", port);
        }
        else if (RTK_PORTMASK_IS_PORT_SET(devInfo.ether.portmask, port))
        {
            ret = rtk_port_phyReg_get(unit, port, 0, 3, &value);
            if (ret != RT_ERR_OK)
                diag_util_mprintf(" %2d      Unknown\n", port);
            else
            {
                switch (value)
                {
                    case 0xCA00:
                        diag_util_mprintf(" %2d      Int 8208D\n", port);
                        break;
                    case 0xCA40:
                        diag_util_mprintf(" %2d      Int PHY\n", port);
                        break;
                    case 0xC881:
                        diag_util_mprintf(" %2d      RTL8208G\n", port);
                        break;
                    case 0xC882:
                        diag_util_mprintf(" %2d      RTL8208D/RTL8208L\n", port);
                        break;
                    case 0xC940:
                        base_port = port - (port % 4);
                        rtk_port_phyReg_get(unit, base_port, 8, 17, &data);
                        if (0xF != ((data >> 10) & 0xF))
                            diag_util_mprintf(" %2d      RTL8214F\n", port);
                        else
                            diag_util_mprintf(" %2d      RTL8214\n", port);
                        break;
                    case 0xC941:
                        rtk_port_phyReg_get(unit, port, 10, 18, &data);
                        if (0xC == (data & 0xF))
                            diag_util_mprintf(" %2d      RTL8214FB\n", port);
                        else
                        {
                            base_port = port - (port % 4);
                            if ((ret = rtk_port_phyReg_set(unit, base_port+3, 8, 18, 0x93f0)) != RT_ERR_OK)
                            {
                                diag_util_mprintf(" %2d      Unknown\n", port);
                                return CPARSER_OK;
                            }
                            if ((ret = rtk_port_phyReg_get(unit, base_port+3, 8, 19, &data)) != RT_ERR_OK)
                            {
                                diag_util_mprintf(" %2d      Unknown\n", port);
                                return CPARSER_OK;
                            }
                            if (((data & 0xF) == 0xF) || ((data & 0xF) == 0xC) || ((data & 0xF) == 0x0))
                                diag_util_mprintf(" %2d      RTL8214B\n", port);
                            else if (((data & 0xF) == 0xE) || ((data & 0xF) == 0x8))
                                diag_util_mprintf(" %2d      RTL8212B\n", port);
                            else
                                diag_util_mprintf(" %2d      Unknown\n", port);
                        }
                        break;
                    case 0x8201:
                        diag_util_mprintf(" %2d      RTL8201\n", port);
                        break;
                    case 0xC980:
                        if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID))
                            diag_util_mprintf(" %2d      RTL8218B\n", port);
                        else
                            diag_util_mprintf(" %2d      RTL8218\n", port);
                        break;
                    case 0xC981:
                        base_port = port - (port % 8);
                        if (DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
                        {
                            DIAG_UTIL_ERR_CHK(ioal_mem32_read(unit, 0xA17C, &poll_portmask), ret);
                            DIAG_UTIL_ERR_CHK(ioal_mem32_write(unit, 0xA17C, 0x0), ret);
                        }
                        DIAG_UTIL_ERR_CHK(rtk_port_phyReg_set(unit, base_port, 0x0a42, 29, 0x0008), ret);
                        DIAG_UTIL_ERR_CHK(rtk_port_phyReg_set(unit, base_port, 0x0278, 18, 0x0455), ret);
                        DIAG_UTIL_ERR_CHK(rtk_port_phyReg_get(unit, base_port, 0x0260, 18, &data), ret);
                        chip_mode = (data & 0xF);
                        cfg_chip_mode = ((data>>4) & 0xF);
                        DIAG_UTIL_ERR_CHK(rtk_port_phyReg_set(unit, base_port, 0x0a42, 29, 0x0000), ret);
#if 0
                        DIAG_UTIL_ERR_CHK(rtk_port_phyReg_set(unit, base_port, 0x0a42, 31, 0x0a42), ret);
                        DIAG_UTIL_ERR_CHK(rtk_port_phyReg_set(unit, base_port, 0x0a42, 27, 0x0004), ret);
                        DIAG_UTIL_ERR_CHK(rtk_port_phyReg_get(unit, base_port, 0x0a42, 28, &data), ret);
                        rlrid = (data & 0xF);
                        DIAG_UTIL_ERR_CHK(rtk_port_phyReg_set(unit, base_port, 0x0a42, 27, 0x0002), ret);
                        DIAG_UTIL_ERR_CHK(rtk_port_phyReg_get(unit, base_port, 0x0a42, 28, &data), ret);
                        rlcid = (data & 0xFFFF);
#endif
                        if (DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
                        {
                            DIAG_UTIL_ERR_CHK(ioal_mem32_write(unit, 0xA17C, poll_portmask), ret);
                        }
                        switch (chip_mode)
                        {
                            case 0xD:
                            case 0xF:
                                diag_util_mprintf(" %2d      RTL8218B\n", port);
                                break;
                            case 0xC:
                            case 0xE:
                                if (cfg_chip_mode == 0x4 || cfg_chip_mode == 0x6)
                                    diag_util_mprintf(" %2d      RTL8214FC\n", port);
                                else
                                    diag_util_mprintf(" %2d      RTL8218FB\n", port);
                                break;
                            case 0x4:
                            case 0x6:
                                diag_util_mprintf(" %2d      RTL8214FC\n", port);
                                break;
                            default:
                                diag_util_mprintf(" %2d      Unknown\n", port);
                                break;
                        }
                        break;
                    case 0xC982:
                        diag_util_mprintf(" %2d      RTL8218C\n", port);
                        break;
                    case 0xC930:
                    case 0xC931:
                    case 0xC932:
                        diag_util_mprintf(" %2d      RTL8212F\n", port);
                        break;
                    default:
                        diag_util_mprintf(" %2d      Unknown\n", port);
                        break;
                }
            }
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_GET_SDK_VERSION
/*
 * switch get sdk-version
 */
cparser_result_t cparser_cmd_switch_get_sdk_version(cparser_context_t *context)
{
    uint32  unit = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("SDK version: %s\n", RT_VERSION_SDK);
    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_GET_SNAP_MODE_PORT_PORTS_ALL
/*
 * switch get snap-mode port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_switch_get_snap_mode_port_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;
    int32  ret = RT_ERR_FAILED;
    uint8 *snapStr[2] = { (uint8 *)"snap03000000", (uint8 *)"snap03xxxxxx" };
    diag_portlist_t portlist;
    rtk_snapMode_t mode;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Per Port Snap Mode\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_switch_portSnapMode_get(unit, port, &mode), ret);
        diag_util_mprintf("\tPort %d : %s\n", port, snapStr[mode]);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_GET_WATCHDOG_MODE
/*
 * switch get watchdog mode
 */
cparser_result_t cparser_cmd_switch_get_watchdog_mode(cparser_context_t *context)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    drv_watchdog_mode_t mode;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(drv_watchdog_mode_get(unit, &mode), ret);
    diag_util_mprintf("\tWatchdog Mode: %s\n", (mode == WATCHDOG_MODE_NORMAL) ? "Normal" : "Interrupt");

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_GET_WATCHDOG_SCALE_BITS
/*
 * switch get watchdog mode
 */
cparser_result_t cparser_cmd_switch_get_watchdog_scale_bits(cparser_context_t *context)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    drv_watchdog_scale_t scale;
    uint8 *scaleStr[4] = { (uint8 *)"25-bits", (uint8 *)"26-bits", (uint8 *)"27-bits", (uint8 *)"28-bits"};

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(drv_watchdog_scale_get(unit, &scale), ret);
    diag_util_mprintf("\tScale length: %s\n", scaleStr[scale]);

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_GET_WATCHDOG_STATE
/*
 * switch get watchdog state
 */
cparser_result_t cparser_cmd_switch_get_watchdog_state(cparser_context_t *context)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    rtk_enable_t enable = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(drv_watchdog_enable_get(unit, &enable), ret);
    diag_util_mprintf("\tWatchdog State: %s\n", (enable == ENABLED) ? DIAG_STR_ENABLE : DIAG_STR_DISABLE);

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_GET_PPPOE_PASSTHROUGH_STATE
/*
 * switch get pppoe-passthrough state
 */
cparser_result_t cparser_cmd_switch_get_pppoe_passthrough_state(cparser_context_t *context)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    rtk_enable_t state = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_switch_pppoePassthrough_get(unit, &state), ret);
    diag_util_mprintf("\tPPPoE Passthrough State: %s\n", (state == ENABLED) ? DIAG_STR_ENABLE : DIAG_STR_DISABLE);

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_SET_CHKSUM_ERR_TYPE_L2_L3_L4_PORT_PORTS_ALL_ACTION_DROP_FORWARD
/*
 * switch set chksum-err-type ( l2 | l3 | l4 ) port ( <PORT_LIST:ports> | all ) action ( drop | forward )
 */
cparser_result_t cparser_cmd_switch_set_chksum_err_type_l2_l3_l4_port_ports_all_action_drop_forward(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_switch_chksum_fail_t type = LAYER2_CHKSUM_FAIL;
    rtk_action_t action = ACTION_DROP;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(3,1))
    {
        case '2':
            type = LAYER2_CHKSUM_FAIL;
            break;

        case '3':
            type = LAYER3_CHKSUM_FAIL;
            break;

        case '4':
            type = LAYER4_CHKSUM_FAIL;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    switch(TOKEN_CHAR(7,0))
    {
        case 'd':
            action = ACTION_DROP;
            break;

        case 'f':
            action = ACTION_FORWARD;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_switch_chksumFailAction_set(unit, port, type, action), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_SET_CRC_RECAL_PORT_PORTS_ALL_STATE_DISABLE_ENABLE
/*
 * switch set crc-recal port ( <PORT_LIST:ports> | all ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_switch_set_crc_recal_port_ports_all_state_disable_enable(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_enable_t enable = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(6,0))
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

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_switch_recalcCRCEnable_set(unit, port, enable), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_SET_IPV4_ADDRESS_IP
/*
 * switch set ipv4-address <IPV4ADDR:ip>
 */
cparser_result_t cparser_cmd_switch_set_ipv4_address_ip(cparser_context_t *context,
    uint32_t *ip_ptr)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    ipaddr_t ip;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(diag_util_str2ip(&ip, (uint8 *)TOKEN_STR(3)), ret);
    DIAG_UTIL_ERR_CHK(rtk_switch_IPv4Addr_set(unit, ip), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_SET_IPV6_ADDRESS_IP
/*
 * switch set ipv6-address <IPV6ADDR:ip>
 */
cparser_result_t cparser_cmd_switch_set_ipv6_address_ip(cparser_context_t *context,
    char **ip_ptr)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    rtk_ipv6_addr_t ip;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(diag_util_str2ipv6((uint8 *)&ip.ipv6_addr[0], (uint8 *)TOKEN_STR(3)), ret);
    DIAG_UTIL_ERR_CHK(rtk_switch_IPv6Addr_set(unit, ip), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_SET_MAC_ADDRESS_MAC
/*
 * switch set mac-address <MACADDR:mac>
 */
cparser_result_t cparser_cmd_switch_set_mac_address_mac(cparser_context_t *context,
    cparser_macaddr_t *mac_ptr)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    rtk_mac_t mac;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(diag_util_str2mac(mac.octet, (uint8 *)TOKEN_STR(3)), ret);
    DIAG_UTIL_ERR_CHK(rtk_switch_mgmtMacAddr_set(unit, &mac), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_SET_CPU_MAX_PKT_LEN_BOTH_DIR_RX_DIR_TX_DIR_LENGTH_LEN
/*
 * switch set cpu-max-pkt-len ( both-dir | rx-dir | tx-dir ) length <UINT:len>
 */
cparser_result_t cparser_cmd_switch_set_cpu_max_pkt_len_both_dir_rx_dir_tx_dir_length_len(cparser_context_t *context,
    uint32_t *len_ptr)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    uint32 len = *len_ptr;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if ('b' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(rtk_switch_cpuMaxPktLen_set(unit, PKTDIR_BOTH, len), ret);
    }
    else if ('r' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(rtk_switch_cpuMaxPktLen_set(unit, PKTDIR_RX, len), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_switch_cpuMaxPktLen_set(unit, PKTDIR_TX, len), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_SET_MAX_PKT_LEN_LENGTH_LEN
/*
 * switch set max-pkt-len length <UINT:len>
 */
cparser_result_t cparser_cmd_switch_set_max_pkt_len_length_len(cparser_context_t *context, uint32_t *len_ptr)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    rtk_switch_maxPktLen_t len;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(*len_ptr)
    {
        case 1522:
            len = MAXPKTLEN_1522B;
            break;

        case 1536:
            len = MAXPKTLEN_1536B;
            break;

        case 1552:
            len = MAXPKTLEN_1552B;
            break;

        case 9216:
            len = MAXPKTLEN_9216B;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_switch_maxPktLen_set(unit, len), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_SET_MAX_PKT_LEN_FE_GE_LENGTH_LEN
/*
 * switch set max-pkt-len ( fe | ge ) length <UINT:len>
 */
cparser_result_t cparser_cmd_switch_set_max_pkt_len_fe_ge_length_len(cparser_context_t *context,
    uint32_t *len_ptr)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    uint32 len;
    rtk_switch_maxPktLen_linkSpeed_t speed;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(3,0))
    {
        case 'f':
            speed = MAXPKTLEN_LINK_SPEED_FE;
            break;

        case 'g':
            speed = MAXPKTLEN_LINK_SPEED_GE;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }
    len = *len_ptr;

    DIAG_UTIL_ERR_CHK(rtk_switch_maxPktLenLinkSpeed_set(unit, speed, len), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_SET_MAX_PKT_LEN_PORT_PORTS_ALL_LENGTH_LEN
/*
 * switch set max-pkt-len port ( <PORT_LIST:ports> | all ) length <UINT:len>
 */
cparser_result_t cparser_cmd_switch_set_max_pkt_len_port_ports_all_length_len(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *len_ptr)
{
    uint32 unit, port;
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_switch_portMaxPktLen_set(unit, port, *len_ptr), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_SET_MAX_PKT_LEN_TAG_LENGTH_STATE_EXCLUDE_INCLUDE
/*
 * switch set max-pkt-len tag-length-state ( exclude | include )
 */
cparser_result_t cparser_cmd_switch_set_max_pkt_len_tag_length_state_exclude_include(cparser_context_t *context)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(4,0))
    {
        case 'e':
            enable = DISABLED;
            break;

        case 'i':
            enable = ENABLED;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_switch_maxPktLenTagLenCntIncEnable_set(unit, enable), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_SET_MGMT_VLAN_ID_INNER_OUTER_VID
/*
 * switch set mgmt-vlan-id ( inner | outer ) <UINT:vid>
 */
cparser_result_t cparser_cmd_switch_set_mgmt_vlan_id_inner_outer_vid(cparser_context_t *context,
    uint32_t *vid_ptr)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if((*vid_ptr < 1) || (*vid_ptr > 4094))
    {
        diag_util_printf("User config: Error VID! (%d)\n", *vid_ptr);
        return CPARSER_NOT_OK;
    }

    switch(TOKEN_CHAR(3,0))
    {
        case 'i':
            DIAG_UTIL_ERR_CHK(rtk_switch_mgmtVlanId_set(unit, *vid_ptr), ret);
            break;

        case 'o':
            DIAG_UTIL_ERR_CHK(rtk_switch_outerMgmtVlanId_set(unit, *vid_ptr), ret);
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_SET_SNAP_MODE_PORT_PORTS_ALL_SNAP03000000_SNAP03XXXXXX
/*
 * switch set snap-mode port ( <PORT_LIST:ports> | all ) ( snap03000000 | snap03xxxxxx )
 */
cparser_result_t cparser_cmd_switch_set_snap_mode_port_ports_all_snap03000000_snap03xxxxxx(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_snapMode_t mode;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(4,6))
    {
        case '0':
            mode = SNAP_MODE_AAAA03000000;
            break;

        case 'x':
            mode = SNAP_MODE_AAAA03;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_switch_portSnapMode_set(unit, port, mode), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_DUMP_DELAY
/*
 * switch dump delay
 */
cparser_result_t cparser_cmd_switch_dump_delay(cparser_context_t *context)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_switch_hwInterfaceDelayEnable_get(unit, DELAY_TYPE_INTRA_LINK0_RX, &enable), ret);
    diag_util_mprintf("\tIntraLink 0 rx: %s\n", (enable == ENABLED) ? DIAG_STR_ENABLE : DIAG_STR_DISABLE);
    DIAG_UTIL_ERR_CHK(rtk_switch_hwInterfaceDelayEnable_get(unit, DELAY_TYPE_INTRA_LINK0_TX, &enable), ret);
    diag_util_mprintf("\tIntraLink 0 tx: %s\n", (enable == ENABLED) ? DIAG_STR_ENABLE : DIAG_STR_DISABLE);
    DIAG_UTIL_ERR_CHK(rtk_switch_hwInterfaceDelayEnable_get(unit, DELAY_TYPE_INTRA_LINK1_RX, &enable), ret);
    diag_util_mprintf("\tIntraLink 1 rx: %s\n", (enable == ENABLED) ? DIAG_STR_ENABLE : DIAG_STR_DISABLE);
    DIAG_UTIL_ERR_CHK(rtk_switch_hwInterfaceDelayEnable_get(unit, DELAY_TYPE_INTRA_LINK1_TX, &enable), ret);
    diag_util_mprintf("\tIntraLink 1 tx: %s\n", (enable == ENABLED) ? DIAG_STR_ENABLE : DIAG_STR_DISABLE);

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_GET_DELAY_INTRALINK0_RX_INTRALINK0_TX_INTRALINK1_RX_INTRALINK1_TX_STATE
/*
 * switch get delay ( intralink0_rx | intralink0_tx | intralink1_rx | intralink1_tx ) state
 */
cparser_result_t cparser_cmd_switch_get_delay_intralink0_rx_intralink0_tx_intralink1_rx_intralink1_tx_state(cparser_context_t *context)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    rtk_switch_delayType_t type;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (TOKEN_CHAR(3,9) == '0')
    {
        if (TOKEN_CHAR(3,11) == 'r')
            type = DELAY_TYPE_INTRA_LINK0_RX;
        else
            type = DELAY_TYPE_INTRA_LINK0_TX;
    }
    else
    {
        if (TOKEN_CHAR(3,11) == 'r')
            type = DELAY_TYPE_INTRA_LINK1_RX;
        else
            type = DELAY_TYPE_INTRA_LINK1_TX;
    }

    DIAG_UTIL_ERR_CHK(rtk_switch_hwInterfaceDelayEnable_get(unit, type, &enable), ret);
    diag_util_mprintf("\tIntraLink %c %cx: %s\n", TOKEN_CHAR(3,9), TOKEN_CHAR(3,11), (enable == ENABLED) ? DIAG_STR_ENABLE : DIAG_STR_DISABLE);

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_SET_DELAY_INTRALINK0_RX_INTRALINK0_TX_INTRALINK1_RX_INTRALINK1_TX_STATE_DISABLE_ENABLE
/*
 * switch set delay ( intralink0_rx | intralink0_tx | intralink1_rx | intralink1_tx ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_switch_set_delay_intralink0_rx_intralink0_tx_intralink1_rx_intralink1_tx_state_disable_enable(cparser_context_t *context)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    rtk_switch_delayType_t type;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (TOKEN_CHAR(3,9) == '0')
    {
        if (TOKEN_CHAR(3,11) == 'r')
            type = DELAY_TYPE_INTRA_LINK0_RX;
        else
            type = DELAY_TYPE_INTRA_LINK0_TX;
    }
    else
    {
        if (TOKEN_CHAR(3,11) == 'r')
            type = DELAY_TYPE_INTRA_LINK1_RX;
        else
            type = DELAY_TYPE_INTRA_LINK1_TX;
    }

    if (TOKEN_CHAR(5,0) == 'd')
        enable = DISABLED;
    else
        enable = ENABLED;

    DIAG_UTIL_ERR_CHK(rtk_switch_hwInterfaceDelayEnable_set(unit, type, enable), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_SET_WATCHDOG_MODE_ISR_NORMAL
/*
 * switch set watchdog mode ( isr | normal )
 */
cparser_result_t cparser_cmd_switch_set_watchdog_mode_isr_normal(cparser_context_t *context)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    drv_watchdog_mode_t mode = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(4,0))
    {
        case 'i':
            mode = WATCHDOG_MODE_INTERRUPT;
            break;

        case 'n':
            mode = WATCHDOG_MODE_NORMAL;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(drv_watchdog_mode_set(unit, mode), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_SET_WATCHDOG_SCALE_BITS_25_26_27_28
/*
 * switch set watchdog scale-bits ( 25 | 26 | 27 | 28 )
 */
cparser_result_t cparser_cmd_switch_set_watchdog_scale_bits_25_26_27_28(cparser_context_t *context)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    drv_watchdog_scale_t scale;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(4,1))
    {
        case '5':
            scale = WATCHDOG_SCALE_1;
            break;

        case '6':
            scale = WATCHDOG_SCALE_2;
            break;

        case '7':
            scale = WATCHDOG_SCALE_3;
            break;

        case '8':
            scale = WATCHDOG_SCALE_4;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(drv_watchdog_scale_set(unit, scale), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_SET_WATCHDOG_STATE_DISABLE_ENABLE
/*
 * switch set watchdog state ( disable | enable )
 */
cparser_result_t cparser_cmd_switch_set_watchdog_state_disable_enable(cparser_context_t *context)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    rtk_enable_t enable = DISABLED;

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

    DIAG_UTIL_ERR_CHK(drv_watchdog_enable_set(unit, enable), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_GET_PKT2CPU_FORMAT
/*
 * switch get pkt2cpu format
 */
cparser_result_t cparser_cmd_switch_get_pkt2cpu_format(cparser_context_t *context)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    rtk_pktFormat_t format = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_switch_pkt2CpuFormat_get(unit, &format), ret);
    diag_util_mprintf("\tPacket to CPU Format: %s\n", (format == MODIFIED_PACKET) ? "Modified" : "Original");

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_SET_PKT2CPU_FORMAT_MODIFIED_ORIGINAL
/*
 * switch set pkt2cpu format ( modified | original )
 */
cparser_result_t cparser_cmd_switch_set_pkt2cpu_format_modified_original(cparser_context_t *context)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    rtk_pktFormat_t format = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(4,0))
    {
        case 'm':
            format = MODIFIED_PACKET;
            break;

        case 'o':
            format = ORIGINAL_PACKET;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_switch_pkt2CpuFormat_set(unit, format), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_GET_PKT2CPU_FORWARD_TRAP_FORMAT
/*
 * switch get pkt2cpu ( forward | trap ) format
 */
cparser_result_t
cparser_cmd_switch_get_pkt2cpu_forward_trap_format(
    cparser_context_t *context)
{
    uint32                      unit;
    int32                       ret;
    rtk_pktFormat_t             format;
    rtk_switch_pkt2CpuType_t    type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    switch(TOKEN_CHAR(3,0))
    {
        case 'f':
            type = PKT2CPU_TYPE_FORWARD;
            break;

        case 't':
            type = PKT2CPU_TYPE_TRAP;
            break;

        default:
            diag_util_printf("User config type: Error!\n");
            return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_switch_pkt2CpuTypeFormat_get(unit, type, &format), ret);

    diag_util_mprintf("\t%s packet to CPU: ", (type == PKT2CPU_TYPE_FORWARD)? "Forward": "Trap");
    diag_util_mprintf("%s\n", (format == MODIFIED_PACKET) ? "Modified" : "Original");

    return CPARSER_OK;
}   /* end of cparser_cmd_switch_get_pkt2cpu_forward_trap_format */
#endif

#ifdef CMD_SWITCH_SET_PKT2CPU_FORWARD_TRAP_FORMAT_MODIFIED_ORIGINAL
/*
 * switch set pkt2cpu ( forward | trap ) format ( modified | original )
 */
cparser_result_t
cparser_cmd_switch_set_pkt2cpu_forward_trap_format_modified_original(
    cparser_context_t *context)
{
    uint32                      unit;
    int32                       ret;
    rtk_pktFormat_t             format;
    rtk_switch_pkt2CpuType_t    type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(3,0))
    {
        case 'f':
            type = PKT2CPU_TYPE_FORWARD;
            break;

        case 't':
            type = PKT2CPU_TYPE_TRAP;
            break;

        default:
            diag_util_printf("User config type: Error!\n");
            return CPARSER_NOT_OK;
    }

    switch(TOKEN_CHAR(5,0))
    {
        case 'm':
            format = MODIFIED_PACKET;
            break;

        case 'o':
            format = ORIGINAL_PACKET;
            break;

        default:
            diag_util_printf("User config format: Error!\n");
            return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_switch_pkt2CpuTypeFormat_set(unit, type, format), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_switch_set_pkt2cpu_forward_trap_format_modified_original */
#endif

#ifdef CMD_SWITCH_SET_PPPOE_PASSTHROUGH_STATE_DISABLE_ENABLE
/*
 * switch set pppoe-passthrough state ( disable | enable )
 */
cparser_result_t cparser_cmd_switch_set_pppoe_passthrough_state_disable_enable(cparser_context_t *context)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    rtk_enable_t state = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(4,0))
    {
        case 'd':
            state = DISABLED;
            break;

        case 'e':
            state = ENABLED;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_switch_pppoePassthrough_set(unit, state), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_GET_CPU_PACKET_TRUNCATION_STATE
/*
 * switch get cpu-packet-truncate state
 */
cparser_result_t cparser_cmd_switch_get_cpu_packet_truncate_state(cparser_context_t *context)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    rtk_enable_t state = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_switch_cpuPktTruncateEnable_get(unit, &state), ret);
    diag_util_mprintf("\tCPU Packet Truncation State: %s\n", (state == ENABLED) ? DIAG_STR_ENABLE : DIAG_STR_DISABLE);

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_SET_CPU_PACKET_TRUNCATION_STATE_DISABLE_ENABLE
/*
 * switch set cpu-packet-truncate state ( disable | enable )
 */
cparser_result_t cparser_cmd_switch_set_cpu_packet_truncate_state_disable_enable(cparser_context_t *context)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    rtk_enable_t state = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(4,0))
    {
        case 'd':
            state = DISABLED;
            break;

        case 'e':
            state = ENABLED;
            break;

        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_switch_cpuPktTruncateEnable_set(unit, state), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_GET_CPU_PACKET_TRUNCATION_LEN
/*
 * switch get cpu-packet-truncate len
 */
cparser_result_t cparser_cmd_switch_get_cpu_packet_truncate_len(cparser_context_t *context)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    uint32 len = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_switch_cpuPktTruncateLen_get(unit, &len), ret);
    diag_util_mprintf("\tCPU Packet Truncation Length: %d\n", len);

    return CPARSER_OK;
}
#endif

#ifdef CMD_SWITCH_SET_CPU_PACKET_TRUNCATION_LEN_LEN
/*
 * switch set cpu-packet-truncate len <UINT:len>
 */
cparser_result_t cparser_cmd_switch_set_cpu_packet_truncate_len_len(cparser_context_t *context,
    uint32_t *len_ptr)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_switch_cpuPktTruncateLen_set(unit, *len_ptr), ret);

    return CPARSER_OK;
}
#endif


