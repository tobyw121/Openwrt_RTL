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
 * $Revision: 57123 $
 * $Date: 2015-03-24 15:24:11 +0800 (Tue, 24 Mar 2015) $
 *
 * Purpose : Definition those OAM command and APIs in the SDK diagnostic shell.
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
#include <rtk/oam.h>
#include <diag_util.h>
#include <diag_om.h>
#include <parser/cparser_priv.h>

#define UTIL_STRING_BUFFER_LENGTH   (30)

#define MIN_PKTLEN         64
#define MAX_PKTLEN      (9*1024) /*9k*/

#ifdef CMD_OAM_SET_DYING_GASP_PORT_PORTS_ALL_LOCAL_TLV_REMOTE_TLV_TLV_VALUE
static int32
_getnext (uint8 *src, int32 separator, uint8 *dest)
{
    int32   len = 0;
    uint8   *c = NULL;

    if ((NULL == src) || (NULL == dest))
    {
        return -1;
    }

    c = (uint8 *)strchr((char *)src, separator);
    if (NULL == c)
    {
        strcpy((char *)dest, (char *)src);
        return -1;
    }
    len = c - src;
    strncpy((char *)dest, (char *)src, len);
    dest[len] = '\0';

    return len + 1;
}

static int32 _diag_util_str2Tlv(uint8 *array, uint8 *str, uint32 length)
{
    int32    len = 0;
    uint32   i = 0;
    uint8    *ptr = str;
    uint8    buf[UTIL_STRING_BUFFER_LENGTH];

    if ((NULL == array) || (NULL == str))
    {
        return RT_ERR_FAILED;
    }

    memset(buf, 0, UTIL_STRING_BUFFER_LENGTH);

    for (i = 0; i < length - 1; ++i)
    {
        if ((len = _getnext(ptr, ':', buf)) == -1 &&
            (len = _getnext(ptr, '-', buf)) == -1)
        {
            return RT_ERR_FAILED; /* parse error */
        }
        array[i] = strtol((char *)buf, NULL, 16);
        ptr += len;
    }
    array[length - 1] = strtol((char *)ptr, NULL, 16);

    return RT_ERR_OK;
}
#endif

#ifdef CMD_OAM_ADD_DEL_REMOTE_MEPID_VALUE
/*
 * oam ( add | del ) remote-mepid <HEX:value>
 */
cparser_result_t cparser_cmd_oam_add_del_remote_mepid_value(cparser_context_t *context,
    uint32_t *value_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;

    DIAG_OM_GET_CHIP_ID(unit);


    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((value_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    if('a' == TOKEN_CHAR(1, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_oam_cfmRemoteMep_add(unit, *value_ptr), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_oam_cfmRemoteMep_del(unit, *value_ptr), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_DUMP_CFM_DYING_GASP_OAM
/*
 * oam dump ( cfm | dying-gasp | oam )
 */
cparser_result_t cparser_cmd_oam_dump_cfm_dying_gasp_oam(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                   ret = RT_ERR_FAILED;
    uint32                   waitTime;
    uint64                  txCounter;
    rtk_oam_testFrameTxStatus_t txStatus;
    int32                   tmpIndex = 0;
    uint32                 value = 0;
    rtk_oam_cfm_t     config;
    rtk_oam_ccmFrame_t  ccmFrameType;
    rtk_snapOui_t         snapoui;
    rtk_oam_ccmInterval_t interval;
    rtk_oam_cfmLoopbackCtrl_t ctrl;
    uint8                            macStr[UTIL_STRING_BUFFER_LENGTH];

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    if('c' == TOKEN_CHAR(2, 0))
    {
        diag_util_mprintf("CFM Configuration:\n");
        ret = rtk_oam_cfmEntry_get(unit, 0 ,&config);
        if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("\tMd Level0 : %u\n", config.md_level);
            diag_util_mprintf("\tMaid0     : 0x");
            for(tmpIndex = 0; tmpIndex < RTK_MAX_LEN_OF_CFM_MAID; tmpIndex++)
            {
                diag_util_mprintf("%x ", config.maid[tmpIndex]);
            }

            diag_util_mprintf("\n");
        }

        ret = rtk_oam_cfmEntry_get(unit, 1 ,&config);
        if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("\tMd Level1 : %u\n", config.md_level);
            diag_util_mprintf("\tMaid1     : 0x");
            for(tmpIndex = 0; tmpIndex < RTK_MAX_LEN_OF_CFM_MAID; tmpIndex++)
            {
                diag_util_mprintf("%x ", config.maid[tmpIndex]);
            }
            diag_util_mprintf("\n");
        }

        ret = rtk_oam_cfmCCMFrame_get(unit, 0 ,&ccmFrameType);
        if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("CCM Packet Index0 Config :\n");
            diag_util_mprintf("\tPacket Type    : ");
            if( ETHERNET_II_PACKET == ccmFrameType.pktType)
                diag_util_mprintf("Ethernet-II\n");
            else
                diag_util_mprintf("SNAP\n");
            if(ccmFrameType.enable_innerTag)
            {
                diag_util_mprintf("\tInner Vlan Tag : ENABLE\n");
                diag_util_mprintf("\t\tInner Vlan Tag Tpid     : 0x%x\n", ccmFrameType.inner_tpid);
                diag_util_mprintf("\t\tInner Vlan Tag Vid      : %u\n", ccmFrameType.inner_vid);
                diag_util_mprintf("\t\tInner Vlan Tag Priority : %u\n", ccmFrameType.inner_pri);
                diag_util_mprintf("\t\tInner Vlan Tag CFI      : %u\n", ccmFrameType.inner_cfi);
            }
            else
                diag_util_mprintf("\tInner Vlan Tag : DISABLE\n");

             if(ccmFrameType.enable_outerTag)
            {
                diag_util_mprintf("\tOuter Vlan Tag : ENABLE\n");
                diag_util_mprintf("\t\tOuter Vlan Tag Tpid     : 0x%x\n", ccmFrameType.outer_tpid);
                diag_util_mprintf("\t\tOuter Vlan Tag Vid      : %u\n", ccmFrameType.outer_vid);
                diag_util_mprintf("\t\tOuter Vlan Tag Priority : %u\n", ccmFrameType.outer_pri);
                diag_util_mprintf("\t\tOuter Vlan Tag DEI      : %u\n", ccmFrameType.outer_dei);
            }
            else
                diag_util_mprintf("\tOuter Vlan Tag : DISABLE\n");

            DIAG_UTIL_ERR_CHK(diag_util_mac2str(macStr, ccmFrameType.dest_mac.octet), ret);
            diag_util_mprintf("\tDst Mac        : %s\n", macStr);
            DIAG_UTIL_ERR_CHK(diag_util_mac2str(macStr, ccmFrameType.source_mac.octet), ret);
            diag_util_mprintf("\tSrc Mac        : %s\n", macStr);
        }

        ret = rtk_oam_cfmCCMFlag_get(unit, 0, &value);
        if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("\tCCM Flag0      : 0x%x\n", value);
        }


        ret = rtk_oam_cfmCCMFrame_get(unit, 1 ,&ccmFrameType);
        if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("CCM Packet Index1 Config :\n");
            diag_util_mprintf("\tPacket Type    : ");
            if( ETHERNET_II_PACKET == ccmFrameType.pktType)
                diag_util_mprintf("Ethernet-II\n");
            else
                diag_util_mprintf("SNAP\n");
            if(ccmFrameType.enable_innerTag)
            {
                diag_util_mprintf("\tInner Vlan Tag : ENABLE\n");
                diag_util_mprintf("\t\tInner Vlan Tag Tpid     : 0x%x\n", ccmFrameType.inner_tpid);
                diag_util_mprintf("\t\tInner Vlan Tag Vid      : %u\n", ccmFrameType.inner_vid);
                diag_util_mprintf("\t\tInner Vlan Tag Priority : %u\n", ccmFrameType.inner_pri);
                diag_util_mprintf("\t\tInner Vlan Tag CFI      : %u\n", ccmFrameType.inner_cfi);
            }
            else
                diag_util_mprintf("\tInner Vlan Tag : DISABLE\n");

             if(ccmFrameType.enable_outerTag)
            {
                diag_util_mprintf("\tOuter Vlan Tag : ENABLE\n");
                diag_util_mprintf("\t\tOuter Vlan Tag Tpid     : 0x%x\n", ccmFrameType.outer_tpid);
                diag_util_mprintf("\t\tOuter Vlan Tag Vid      : %u\n", ccmFrameType.outer_vid);
                diag_util_mprintf("\t\tOuter Vlan Tag Priority : %u\n", ccmFrameType.outer_pri);
                diag_util_mprintf("\t\tOuter Vlan Tag DEI      : %u\n", ccmFrameType.outer_dei);
            }
            else
                diag_util_mprintf("\tOuter Vlan Tag : DISABLE\n");

            DIAG_UTIL_ERR_CHK(diag_util_mac2str(macStr, ccmFrameType.dest_mac.octet), ret);
            diag_util_mprintf("\tDst Mac        : %s\n", macStr);
            DIAG_UTIL_ERR_CHK(diag_util_mac2str(macStr, ccmFrameType.source_mac.octet), ret);
            diag_util_mprintf("\tSrc Mac        : %s\n", macStr);
        }

        ret = rtk_oam_cfmCCMFlag_get(unit, 1, &value);
        if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("\tCCM Flag1      : 0x%x\n", value);
        }

        ret = rtk_oam_cfmCCMSnapOui_get(unit, &snapoui);
        if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("Snap Oui                             : 0x%x:%x:%x\n",
                           snapoui.snapOui[0], snapoui.snapOui[1], snapoui.snapOui[2]);
        }

        ret = rtk_oam_cfmCCMEtype_get(unit, &value);
        if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("EtherType                            : 0x%x\n", value);
        }

        ret = rtk_oam_cfmCCMOpcode_get(unit, &value);
        if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("CCM OpCode                           : 0x%x\n", value);
        }


        ret = rtk_oam_cfmCCMInterval_get(unit, &interval);
        if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("CCM Interval                         : ");
            if (INTERVAL_END == interval)
                diag_util_mprintf("invalid\n");
            else if(INTERVAL_3_3_MS == interval)
                diag_util_mprintf("3.3ms\n");
            else if(INTERVAL_10_MS == interval)
                diag_util_mprintf("10ms\n");
            else if(INTERVAL_100_MS == interval)
                diag_util_mprintf("100ms\n");
            else if(INTERVAL_1_S == interval)
                diag_util_mprintf("1s\n");
            else if(INTERVAL_10_S == interval)
                diag_util_mprintf("10s\n");
            else if(INTERVAL_1_MIN == interval)
                diag_util_mprintf("1m\n");
            else if(INTERVAL_10_MIN == interval)
                diag_util_mprintf("10m\n");
            else
                diag_util_mprintf("the time the asic not support\n");
        }

        ret = rtk_oam_cfmLoopbackReplyCtrl_get(unit, &ctrl);
        if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            if(ENABLED == ctrl.keep_innerTag)
                diag_util_mprintf("Lbm Packet Auto Reply Keep Inner Vid : ENABLE\n");
            else
                diag_util_mprintf("Lbm Packet Auto Reply Keep Inner Vid : DISABLE\n");

            if(ENABLED == ctrl.keep_outerTag)
                diag_util_mprintf("Lbm Packet Auto Reply Keep Outer Vid : ENABLE\n");
            else
                diag_util_mprintf("Lbm Packet Auto Reply Keep Outer Vid : DISABLE\n");
        }
    }
    else if('d' == TOKEN_CHAR(2, 0))
    {
        diag_util_mprintf("Dying Gasp Configuration : \n");
        ret = rtk_oam_dyingGaspWaitTime_get(unit, &waitTime);
        if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
            diag_util_mprintf("\tDying Gasp Waiting Time : %u\n", waitTime);
    }
    else if('o' == TOKEN_CHAR(2, 0))
    {
        diag_util_mprintf("Smart Packet Generator Configuration :\n");

        ret = rtk_oam_txStatusOfTestFrame_get(unit, &txStatus ,&txCounter);
        if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            if(TX_FINISH == txStatus)
                diag_util_mprintf("\tSPG Tx Finished.\n");
            else
                diag_util_mprintf("\tSPG Tx NOT Finished.\n");

            diag_util_mprintf("\tSPG Transimit Packets   : %llu\n", txCounter);
        }

        ret = rtk_oam_dyingGaspWaitTime_get(unit, &value);
        if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("\tDying Gasp Waiting Time : %u\n", value);
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_DUMP_CFM_DYING_GASP_OAM_PORT_PORT_ALL
/*
 * oam dump ( cfm | dying-gasp | oam ) port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_oam_dump_cfm_dying_gasp_oam_port_ports_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    rtk_oam_dyingGaspTLV_t  tlv;
    int32                    index;
    uint32                  counter = 0;
    rtk_enable_t            enable = DISABLED;
    int32                   tmpIndex = 0;
    uint32                  value = 0;
    rtk_oam_cfmPort_t       portConfig;
    diag_portlist_t         portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    if('c' == TOKEN_CHAR(2, 0))
    {
        DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

        diag_util_mprintf("CFM Configuration:\n");

        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            diag_util_mprintf("\tPort %u:\n", port);

            memset(&portConfig, 0, sizeof(rtk_oam_cfmPort_t));
            ret = rtk_oam_cfmPortEntry_get(unit, port ,&portConfig);
            if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }
            else
            {
                diag_util_mprintf("\t\tCfm Index             : %u\n", portConfig.cfm_idx);
                diag_util_mprintf("\t\tMepid                 : 0x%x\n", portConfig.mepid);
            }

            ret = rtk_oam_cfmMepEnable_get(unit, port ,&enable);
            if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }
            else
            {
                if(ENABLED == enable)
                    diag_util_mprintf("\t\tMepid Comapre         : ENABLE\n");
                else
                    diag_util_mprintf("\t\tMepid Comapre         : DISABLE\n");
            }

            ret = rtk_oam_cfmIntfStatus_get(unit, port ,&value);
            if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }
            else
            {
                if((value > 7) || (value == 0))
                    diag_util_mprintf("\t\tInterface Status      : Invalid Value(%u)\n", value);
                else
                    diag_util_mprintf("\t\tInterface Status      : %u\n", value);
            }

            ret = rtk_oam_cfmPortStatus_get(unit, port ,&value);
            if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }
            else
            {
                if(1 == value)
                    diag_util_mprintf("\t\tPort Status           : Blocked(%u)\n", value);
                else if(2 == value)
                    diag_util_mprintf("\t\tPort Status           : Uped(%u)\n", value);
                else
                    diag_util_mprintf("\t\tPort Status           : Invalid Value(%u)\n", value);
            }

            ret = rtk_oam_cfmLoopbackReplyEnable_get(unit, port ,&enable);
            if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }
            else
            {
                if(ENABLED == enable)
                    diag_util_mprintf("\t\tLbm Packet Auto Reply : ENABLE\n");
                else
                    diag_util_mprintf("\t\tLbm Packet Auto Reply : DISABLE\n");
            }

            diag_util_mprintf("\n");
        }
    }
    else if('d' == TOKEN_CHAR(2, 0))
    {
        diag_util_mprintf("Dying Gasp Configuration : \n");
        DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            diag_util_mprintf("\tPort %u:\n", port);

            ret = rtk_oam_autoDyingGaspEnable_get(unit, port, &enable);
            if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }
            else
            {
                if(ENABLED == enable)
                    diag_util_mprintf("\t\tAsic Auto Dying Gasp  : ENABLE\n");
                else
                    diag_util_mprintf("\t\tAsic Auto Dying Gasp  : DISABLE\n");
            }

            ret = rtk_oam_dyingGaspTLV_get(unit, port, &tlv);
            if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }
            else
            {
                diag_util_mprintf("\t\tDying Gasp Remote Tlv : 0x");
                for(index = 0; index < RTK_OAM_REMOTE_TLV_LEN;index++)
                {
                    if(index == RTK_OAM_REMOTE_TLV_LEN - 1)
                        diag_util_mprintf("%02x\n", tlv.remoteTLV[index]);
                    else
                        diag_util_mprintf("%02x:", tlv.remoteTLV[index]);
                }

                diag_util_mprintf("\t\tDying Gasp Local Tlv  : 0x");
                for(index = 0; index < RTK_OAM_LOCAL_TLV_LEN;index++)
                {
                    if(index == RTK_OAM_REMOTE_TLV_LEN - 1)
                        diag_util_mprintf("%02x\n", tlv.localTLV[index]);
                    else
                        diag_util_mprintf("%02x:", tlv.localTLV[index]);
                }

            }

            diag_util_mprintf("\n");

        }
    }
    else if('o' == TOKEN_CHAR(2, 0))
    {
        diag_util_mprintf("OAM Configuration :\n");

        DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            diag_util_mprintf("\tPort %u : \n", port);

            ret = rtk_oam_oamCounter_get(unit, port ,&counter);
            if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }
            else
            {
                diag_util_mprintf("\t\tOampdu Received Counter  : %u\n", counter);
            }

            ret = rtk_oam_autoDyingGaspEnable_get(unit, port ,&enable);
            if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }
            else
            {
                if(ENABLED == enable)
                    diag_util_mprintf("\t\tAuto Dying Gasp Generate : ENABLE\n");
                else
                    diag_util_mprintf("\t\tAuto Dying Gasp Generate : DISABLE\n");
            }

            ret = rtk_oam_dyingGaspTLV_get(unit, port ,&tlv);
            if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }
            else
            {
                diag_util_mprintf("\t\tRemote Tlv Value         : 0x");
                for(tmpIndex = 0; tmpIndex < RTK_OAM_REMOTE_TLV_LEN; tmpIndex++)
                {
                    diag_util_mprintf("%x ", tlv.remoteTLV[tmpIndex]);
                }
                diag_util_mprintf("\n");

                diag_util_mprintf("\t\tLocal Tlv Value          : 0x");
                for(tmpIndex = 0; tmpIndex < RTK_OAM_LOCAL_TLV_LEN; tmpIndex++)
                {
                    diag_util_mprintf("%x ", tlv.localTLV[tmpIndex]);
                }
                diag_util_mprintf("\n");
            }
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CCM_MEPID_STATUS_PORT_PORTS_ALL_MEPID_MEPID
/*
 * oam get ccm mepid-status port ( <PORT_LIST:port> | all ) mepid <HEX:mepid>
 */
cparser_result_t cparser_cmd_oam_get_ccm_mepid_status_port_port_all_mepid_mepid(cparser_context_t *context,
    char **port_ptr,
    uint32_t *mepid_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    uint32                  rdiValue = 0;
    rtk_oam_ccStatus_t      status = CC_STATUS_AGEOUT;
    diag_portlist_t         portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((mepid_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    diag_util_mprintf("CCM Mepid Status:\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("\tPort %u:\n", port);
        ret = rtk_oam_cfmCCStatus_get(unit, port, *mepid_ptr, &rdiValue, &status);
        if ((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("\t\tMepid         : 0x%x\n", *mepid_ptr);
            diag_util_mprintf("\t\tRDI Bit Value : %u\n", rdiValue);
            if(CC_STATUS_AGEOUT == status)
                diag_util_mprintf("\t\tMepid is already Aged\n");
            else if(CC_STATUS_NOT_AGE == status)
                diag_util_mprintf("\t\tMepid is not Aged\n");
        }
        diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_LOOPBACK_MODE_PORT_PORTS_ALL
/*
 * oam get loopback-mode port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_oam_get_loopback_mode_port_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    rtk_oam_loopbackMode_t  lbMode;
    diag_portlist_t         portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("\tPort %u:\n", port);
        diag_util_mprintf("\tLoopback Mode : ");

        ret = rtk_oam_loopbackMode_get(unit, port, &lbMode);
        if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            if(lbMode == OAM_LOOPBACK_ALL_FRAME)
                diag_util_mprintf("Loopback All Frames\n");
            else if(lbMode == OAM_LOOPBACK_TEST_FRAME)
                diag_util_mprintf("Loopback Only Oam Test Frames\n");
            else if(lbMode == OAM_LOOPBACK_DROP_TEST_FRAME)
                diag_util_mprintf("Drop Oam Test Frames\n");
            else
                diag_util_mprintf("Off\n");
        }
        diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_ASIC_AUTO_DYING_GASP_PORT_PORTS_ALL_STATE
/*
 * oam get asic-auto-dying-gasp port ( <PORT_LIST:ports> | all ) state
 */
cparser_result_t cparser_cmd_oam_get_asic_auto_dying_gasp_port_ports_all_state(cparser_context_t *context,
    char **port_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    rtk_enable_t            enable = DISABLED;
    diag_portlist_t         portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("\tPort %u:\n", port);

        ret = rtk_oam_autoDyingGaspEnable_get(unit, port, &enable);
        if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            if(ENABLED == enable)
                diag_util_mprintf("\t\tAsic Auto Dying Gasp  : ENABLE\n");
            else
                diag_util_mprintf("\t\tAsic Auto Dying Gasp  : DISABLE\n");
        }
        diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CFM_INDEX_INDEX_MD_LEVEL
/*
 * oam get cfm-index <UINT:index> md-level
 */
cparser_result_t cparser_cmd_oam_get_cfm_index_index_md_level(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    rtk_oam_cfm_t           config;
    diag_portlist_t         list;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(list, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(list, port)
    {
        ret = rtk_oam_cfmEntry_get(unit, port ,&config);
        if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
           DIAG_ERR_PRINT(ret);
           return CPARSER_NOT_OK;
        }
        else
        {
           diag_util_mprintf("\tMd Level : %u\n", config.md_level);
           diag_util_mprintf("\n");
        }

    }
    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CFM_INDEX_INDEX_MAID_INDEX_MAID_INDEX
/*
 * oam get cfm-index <UINT:index> maid-index <UINT:maid_index>
 */
cparser_result_t cparser_cmd_oam_get_cfm_index_index_maid_index_maid_index(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *maid_index_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    int32                   tmpIndex = 0;
    rtk_oam_cfm_t           config;
    diag_portlist_t         list;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(list, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(list, port)
    {
        ret = rtk_oam_cfmEntry_get(unit, port ,&config);
        if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
           DIAG_ERR_PRINT(ret);
           return CPARSER_NOT_OK;
        }
        else
        {
           diag_util_mprintf("\tMaid     : 0x");
           for(tmpIndex = 0; tmpIndex < RTK_MAX_LEN_OF_CFM_MAID; tmpIndex++)
           {
               diag_util_mprintf("%x ", config.maid[tmpIndex]);
           }
           diag_util_mprintf("\n");
        }
    }
    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CCM_PORT_PORTS_ALL_GROUP_ID_INDEX_MEPID
/*
 * oam get ccm port ( <PORT_LIST:ports> | all ) group-id <UINT:index> mepid
 */
cparser_result_t cparser_cmd_oam_get_ccm_port_ports_all_group_id_index_mepid(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *index_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    rtk_oam_cfmPort_t       portConfig;
    diag_portlist_t         portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {

       memset(&portConfig, 0, sizeof(rtk_oam_cfmPort_t));
       ret = rtk_oam_cfmPortEntry_get(unit, port ,&portConfig);
       if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
       {
           DIAG_ERR_PRINT(ret);
           return CPARSER_NOT_OK;
       }
       else
       {
           diag_util_mprintf("\t\tMepid                 : 0x%x\n", portConfig.mepid);
       }

    }
    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CCM_PORT_PORTS_ALL_INTERFACE_STATUS
/*
 * oam get ccm port ( <PORT_LIST:ports> | all ) interface-status
 */
cparser_result_t cparser_cmd_oam_get_ccm_port_ports_all_interface_status (cparser_context_t *context,
    char **port_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    uint32                  value = 0;
    rtk_oam_cfmPort_t       portConfig;
    diag_portlist_t         portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
       memset(&portConfig, 0, sizeof(rtk_oam_cfmPort_t));
       ret = rtk_oam_cfmIntfStatus_get(unit, port ,&value);
       if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
       {
           DIAG_ERR_PRINT(ret);
           return CPARSER_NOT_OK;
       }
       else
       {
           if((value > 7) || (value == 0))
               diag_util_mprintf("\t\tInterface Status      : Invalid Value(%u)\n", value);
           else
               diag_util_mprintf("\t\tInterface Status      : %u\n", value);
       }
       diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CCM_PORT_PORTS_ALL_PORT_STATUS
/*
 * oam get ccm port ( <PORT_LIST:ports> | all ) port-status
 */
cparser_result_t cparser_cmd_oam_get_ccm_port_ports_all_port_status(cparser_context_t *context,
    char **port_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    uint32                  value = 0;
    rtk_oam_cfmPort_t       portConfig;
    diag_portlist_t         portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {

       memset(&portConfig, 0, sizeof(rtk_oam_cfmPort_t));
       ret = rtk_oam_cfmPortStatus_get(unit, port ,&value);
       if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
       {
           DIAG_ERR_PRINT(ret);
           return CPARSER_NOT_OK;
       }
       else
       {
           if(1 == value)
               diag_util_mprintf("\t\tPort Status           : Blocked(%u)\n", value);
           else if(2 == value)
               diag_util_mprintf("\t\tPort Status           : Uped(%u)\n", value);
           else
               diag_util_mprintf("\t\tPort Status           : Invalid Value(%u)\n", value);
       }


       diag_util_mprintf("\n");

    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CCM_CFM_INDEX_INDEX_INNER_TAG_TPID
/*
 * oam get ccm cfm-index <UINT:index> inner-tag <HEX:tpid>
 */
cparser_result_t cparser_cmd_oam_get_ccm_cfm_index_index_inner_tag_tpid(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    diag_portlist_t         portlist;
    rtk_oam_ccmFrame_t      config;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    memset(&config, 0, sizeof(rtk_oam_ccmFrame_t));


    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMFrame_get(unit, port, &config), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port){

        if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("\t\tInner Vlan Tag Tpid     : 0x%x\n", config.inner_tpid);
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CCM_CFM_INDEX_INDEX_INNER_TAG_VID
/*
 * oam get ccm cfm-index <UINT:index> inner-tag vid
 */
cparser_result_t cparser_cmd_oam_get_ccm_cfm_index_index_inner_tag_vid(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    diag_portlist_t         portlist;
    rtk_oam_ccmFrame_t      config;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    memset(&config, 0, sizeof(rtk_oam_ccmFrame_t));

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMFrame_get(unit, port, &config), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port){

        if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("\t\tInner Vlan Tag Vid      : %u\n", config.inner_vid);

        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CCM_CFM_INDEX_INDEX_INNER_TAG_PRI
/*
 * oam get ccm cfm-index <UINT:index> inner-tag pri
 */
cparser_result_t cparser_cmd_oam_get_ccm_cfm_index_index_inner_tag_pri(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    diag_portlist_t         portlist;
    rtk_oam_ccmFrame_t      config;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    memset(&config, 0, sizeof(rtk_oam_ccmFrame_t));


    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMFrame_get(unit, port, &config), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port){

      if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
      {
          DIAG_ERR_PRINT(ret);
          return CPARSER_NOT_OK;
      }
      else
      {
          diag_util_mprintf("\t\tInner Vlan Tag Priority : %u\n", config.inner_pri);
       }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CCM_CFM_INDEX_INDEX_INNER_TAG_CFI
/*
 * oam get ccm cfm-index <UINT:index> inner-tag cfi
 */
cparser_result_t cparser_cmd_oam_get_ccm_cfm_index_index_inner_tag_cfi(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    diag_portlist_t         portlist;
    rtk_oam_ccmFrame_t      config;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    memset(&config, 0, sizeof(rtk_oam_ccmFrame_t));

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMFrame_get(unit, port, &config), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port){

      if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
      {
          DIAG_ERR_PRINT(ret);
          return CPARSER_NOT_OK;
      }
      else
      {
          diag_util_mprintf("\t\tInner Vlan Tag CFI      : %u\n", config.inner_cfi);

       }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CCM_CFM_INDEX_INDEX_OUTER_TAG_TPID
/*
 * oam get ccm cfm-index <UINT:index> outer-tag <HEX:tpid>
 */
cparser_result_t cparser_cmd_oam_get_ccm_cfm_index_index_outer_tag_tpid(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    diag_portlist_t         portlist;
    rtk_oam_ccmFrame_t      config;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    memset(&config, 0, sizeof(rtk_oam_ccmFrame_t));


    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMFrame_get(unit, port, &config), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port){

      if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
      {
          DIAG_ERR_PRINT(ret);
          return CPARSER_NOT_OK;
      }
      else
      {
          diag_util_mprintf("\t\tOuter Vlan Tag Tpid     : 0x%x\n", config.outer_tpid);
       }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CCM_CFM_INDEX_INDEX_OUTER_TAG_VID
/*
 * oam get ccm cfm-index <UINT:index> outer-tag vid
 */
cparser_result_t cparser_cmd_oam_get_ccm_cfm_index_index_outer_tag_vid(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    diag_portlist_t         portlist;
    rtk_oam_ccmFrame_t      config;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    memset(&config, 0, sizeof(rtk_oam_ccmFrame_t));


    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMFrame_get(unit, port, &config), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port){

      if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
      {
          DIAG_ERR_PRINT(ret);
          return CPARSER_NOT_OK;
      }
      else
      {
          diag_util_mprintf("\t\tOuter Vlan Tag Vid      : %u\n", config.outer_vid);

       }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CCM_CFM_INDEX_INDEX_OUTER_TAG_PRI
/*
 * oam get ccm cfm-index <UINT:index> outer-tag pri
 */
cparser_result_t cparser_cmd_oam_get_ccm_cfm_index_index_outer_tag_pri(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    diag_portlist_t         portlist;
    rtk_oam_ccmFrame_t      config;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    memset(&config, 0, sizeof(rtk_oam_ccmFrame_t));


    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMFrame_get(unit, port, &config), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port){

      if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
      {
          DIAG_ERR_PRINT(ret);
          return CPARSER_NOT_OK;
      }
      else
      {
          diag_util_mprintf("\t\tOuter Vlan Tag Priority : %u\n", config.outer_pri);
       }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CCM_CFM_INDEX_INDEX_OUTER_TAG_DEI
/*
 * oam get ccm cfm-index <UINT:index> outer-tag dei
 */
cparser_result_t cparser_cmd_oam_get_ccm_cfm_index_index_outer_tag_dei(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    diag_portlist_t         portlist;
    rtk_oam_ccmFrame_t      config;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    memset(&config, 0, sizeof(rtk_oam_ccmFrame_t));


    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMFrame_get(unit, port, &config), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port){

      if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
      {
          DIAG_ERR_PRINT(ret);
          return CPARSER_NOT_OK;
      }
      else
      {
          diag_util_mprintf("\t\tOuter Vlan Tag DEI      : %u\n", config.outer_dei);

       }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CCM_CFM_INDEX_INDEX_CCM_FLAG
/*
 * oam get ccm cfm-index <UINT:index> ccm-flag
 */
cparser_result_t cparser_cmd_oam_get_ccm_cfm_index_index_ccm_flag(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    uint32                  value = 0;
    diag_portlist_t         portlist;
    rtk_oam_ccmFrame_t      config;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    memset(&config, 0, sizeof(rtk_oam_ccmFrame_t));


    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMFrame_get(unit, port, &config), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port){

        ret = rtk_oam_cfmCCMFlag_get(unit, port, &value);
        if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("\tCCM Flag1      : 0x%x\n", value);
        }
     }


    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CCM_CFM_INDEX_INDEX_PKT_TYPE_MAC
/*
 * oam get ccm cfm-index <UINT:index> pkt-type mac
 */
cparser_result_t cparser_cmd_oam_get_ccm_cfm_index_index_pkt_type_mac(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    uint32                  value = 0;
    diag_portlist_t         portlist;
    rtk_oam_ccmFrame_t      config;
    uint8                   macStr[16];

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    memset(&config, 0, sizeof(rtk_oam_ccmFrame_t));


    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMFrame_get(unit, port, &config), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port){

        ret = rtk_oam_cfmCCMFlag_get(unit, port, &value);
        if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            memset(&macStr, 0, sizeof(macStr));
            DIAG_UTIL_ERR_CHK(diag_util_mac2str(macStr, config.dest_mac.octet), ret);
            diag_util_mprintf("Dst Mac    : %s", macStr);
            memset(&macStr, 0, sizeof(macStr));
            DIAG_UTIL_ERR_CHK(diag_util_mac2str(macStr, config.source_mac.octet), ret);
            diag_util_mprintf("\tSrc Mac    : %s\n", macStr);
        }
     }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CCM_CFM_INDEX_INDEX_PKT_TYPE
/*
 * oam get ccm cfm-index <UINT:index> pkt-type ( ethernet-ii | snap )
 */
cparser_result_t cparser_cmd_oam_get_ccm_cfm_index_index_pkt_type(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    uint32                  value = 0;
    diag_portlist_t         portlist;
    rtk_oam_ccmFrame_t      config;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    memset(&config, 0, sizeof(rtk_oam_ccmFrame_t));


    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMFrame_get(unit, port, &config), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port){

        ret = rtk_oam_cfmCCMFlag_get(unit, port, &value);
        if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
         {
             DIAG_ERR_PRINT(ret);
             return CPARSER_NOT_OK;
         }
         else
         {
             diag_util_mprintf("CCM Packet Index0 Config :\n");
             diag_util_mprintf("\tPacket Type    : ");
             if( ETHERNET_II_PACKET == config.pktType)
                 diag_util_mprintf("Ethernet-II\n");
             else
                 diag_util_mprintf("SNAP\n");
        }
     }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CCM_ETHER_TYPE_OPCODE_SNAP_OUI
/*
 * oam get ccm ( ether-type | opcode )
 */
cparser_result_t cparser_cmd_oam_get_ccm_ether_type_opcode_snap_oui(cparser_context_t *context)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    uint32                  value = 0;
    rtk_snapOui_t           snapoui;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();


    if ('e' == TOKEN_CHAR(3, 0))
    {

        ret = rtk_oam_cfmCCMEtype_get(unit, &value);
        if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("EtherType                            : 0x%x\n", value);
        }

    }
    else if ('o' == TOKEN_CHAR(3, 0))
    {
        ret = rtk_oam_cfmCCMOpcode_get(unit, &value);
        if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("CCM OpCode                           : 0x%x\n", value);
        }

    }else if('s' == TOKEN_CHAR(3, 0)) {


        ret = rtk_oam_cfmCCMSnapOui_get(unit, &snapoui);
        if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("Snap Oui                             : 0x%x:%x:%x\n", \
                           snapoui.snapOui[0], snapoui.snapOui[1], snapoui.snapOui[2]);
        }
    }
    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CCM_INTERVAL
/*
 * oam get ccm interval
 */
cparser_result_t cparser_cmd_oam_get_ccm_interval(cparser_context_t *context)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_oam_ccmInterval_t   interval;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();


    ret = rtk_oam_cfmCCMInterval_get(unit, &interval);
    if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
       DIAG_ERR_PRINT(ret);
       return CPARSER_NOT_OK;
    }
    else
    {
       diag_util_mprintf("CCM Interval                         : ");
       if(INTERVAL_3_3_MS == interval)
           diag_util_mprintf("3.3ms\n");
       else if(INTERVAL_10_MS == interval)
           diag_util_mprintf("10ms\n");
       else if(INTERVAL_100_MS == interval)
           diag_util_mprintf("100ms\n");
       else if(INTERVAL_1_S == interval)
           diag_util_mprintf("1s\n");
       else
           diag_util_mprintf("the time the asic not support\n");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CFM_MEPID_COMPARE_PORT_PORTS_ALL_STATE
/*
 * oam get cfm-mepid-compare port ( <PORT_LIST:ports> | all ) state
 */
cparser_result_t cparser_cmd_oam_get_cfm_mepid_compare_port_ports_all_state(cparser_context_t *context,
    char **port_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    rtk_enable_t            enable = DISABLED;
    diag_portlist_t         portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();


    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port){

        ret = rtk_oam_cfmMepEnable_get(unit, port ,&enable);
        if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            if(ENABLED == enable)
                diag_util_mprintf("\t\tMepid Comapre         : ENABLE\n");
            else
                diag_util_mprintf("\t\tMepid Comapre         : DISABLE\n");
        }
     }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CFM_LOOPBACK_REPLY_PORT_PORTS_ALL_STATE
/*
 * oam get cfm-loopback-reply port ( <PORT_LIST:ports> | all ) state
 */
cparser_result_t cparser_cmd_oam_get_cfm_loopback_reply_port_ports_all_state(cparser_context_t *context,
    char **port_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    rtk_enable_t            enable = DISABLED;
    diag_portlist_t         portlist;


    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port){

        ret = rtk_oam_cfmLoopbackReplyEnable_get(unit, port ,&enable);
        if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            if(ENABLED == enable)
                diag_util_mprintf("\t\tLbm Packet Auto Reply : ENABLE\n");
            else
                diag_util_mprintf("\t\tLbm Packet Auto Reply : DISABLE\n");
        }
     }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_DYING_GASP_PORT_PORTS_ALL_LOCAL_TLV_REMOTE_TLV
/*
 * oam get dying-gasp port ( <PORT_LIST:ports> | all ) ( local-tlv | remote-tlv )
 */
cparser_result_t cparser_cmd_oam_get_dying_gasp_port_ports_all_local_tlv_remote_tlv(cparser_context_t *context,
    char **port_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    rtk_oam_dyingGaspTLV_t  tlv;
    diag_portlist_t         portlist;
    int32                   index;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    memset(&tlv, 0, sizeof(rtk_oam_dyingGaspTLV_t));

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        ret = rtk_oam_dyingGaspTLV_get(unit, port, &tlv);
         if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
         {
             DIAG_ERR_PRINT(ret);
             return CPARSER_NOT_OK;
         }
         else
         {
            if('r' == TOKEN_CHAR(5, 0))
            {
                diag_util_mprintf("\t\tDying Gasp Remote Tlv : 0x");
                 for(index = 0; index < RTK_OAM_REMOTE_TLV_LEN;index++)
                 {
                     if(index == RTK_OAM_REMOTE_TLV_LEN - 1)
                         diag_util_mprintf("%02x\n", tlv.remoteTLV[index]);
                     else
                         diag_util_mprintf("%02x:", tlv.remoteTLV[index]);
                 }

            }
            else if('l' == TOKEN_CHAR(5, 0))
            {
                diag_util_mprintf("\t\tDying Gasp Local Tlv  : 0x");
                for(index = 0; index < RTK_OAM_LOCAL_TLV_LEN;index++)
                {
                    if(index == RTK_OAM_REMOTE_TLV_LEN - 1)
                        diag_util_mprintf("%02x\n", tlv.localTLV[index]);
                    else
                        diag_util_mprintf("%02x:", tlv.localTLV[index]);
                }

            }
            else
            {
                diag_util_printf("User config: Error!\n");
                return CPARSER_NOT_OK;
            }
         }

    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_DYING_GASP_WAITING_TIME
/*
 * oam get dying-gasp waiting-time
 */
cparser_result_t cparser_cmd_oam_get_dying_gasp_waiting_time(cparser_context_t *context)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    uint32                  value = 0;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();


    ret = rtk_oam_dyingGaspWaitTime_get(unit, &value);
    if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_mprintf("\tDying Gasp Waiting Time : %u\n", value);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_LOOPBACK_CTRL_PORT_PORTS_ALL_SA_ACTION_DA_ACTION
/*
 * oam get loopback-ctrl port ( <PORT_LIST:ports> | all ) sa-action da-action
 */
cparser_result_t cparser_cmd_oam_get_loopback_ctrl_port_ports_all_sa_action_da_action(
    cparser_context_t *context,
    char **port_ptr)
{
    uint32                      unit = 0;
    int32                       ret = RT_ERR_FAILED;
    rtk_port_t                  port = 0;
    rtk_oam_loopbackCtrl_t      loopbackCtrl;
    diag_portlist_t             portlist;
    uint8                       macStr[16];

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {


        ret = rtk_oam_loopbackCtrl_get(unit, port, &loopbackCtrl);
        if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("\tPort %u:\n", port);
            diag_util_mprintf("\tLoopback Control Configuration : \n");
            diag_util_mprintf("\t\tSa Action : ");
            if(SA_ACTION_KEEP_MAC == loopbackCtrl.sa_action)
                diag_util_mprintf("Keep-Smac\t");
            else if(SA_ACTION_USE_SWITCH_MAC == loopbackCtrl.sa_action)
                diag_util_mprintf("Use-Switch-Mac-Address\t");
            else if(SA_ACTION_USE_DA == loopbackCtrl.sa_action)
                diag_util_mprintf("Use-Packet-Dmac\t");
            else if (SA_ACTION_USE_USER_DEFINE_MAC == loopbackCtrl.sa_action)
            {
                DIAG_UTIL_ERR_CHK(diag_util_mac2str(macStr, loopbackCtrl.user_defined_src_mac.octet), ret);
                diag_util_mprintf("Use-User-Defined-Mac-Address(%s)\t", macStr);
            }

            diag_util_mprintf("\t\tDa Action : ");
            if(DA_ACTION_KEEP_MAC == loopbackCtrl.da_action)
                diag_util_mprintf("Keep-Dmac\n");
            else if(DA_ACTION_USE_SWITCH_MAC == loopbackCtrl.da_action)
                diag_util_mprintf("Use-Switch-Mac-Address\n");
            else if(DA_ACTION_USE_SA == loopbackCtrl.da_action)
                diag_util_mprintf("Use-Packet-Smac\n");
            else if (DA_ACTION_USE_USER_DEFINE_MAC == loopbackCtrl.da_action)
            {
                DIAG_UTIL_ERR_CHK(diag_util_mac2str(macStr, loopbackCtrl.user_defined_dst_mac.octet), ret);
                diag_util_mprintf("Use-User-Defined-Mac-Address(%s)\n", macStr);
            }
        }

        diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_KEEP_INNER_TAG_KEEP_OUTER_TAG_STATE
/*
 * oam get ( keep-inner-tag | keep-outer-tag ) state
 */
cparser_result_t cparser_cmd_oam_get_keep_inner_tag_keep_outer_tag_state (cparser_context_t *context)
{
    uint32                      unit = 0;
    int32                       ret = RT_ERR_FAILED;
    rtk_oam_cfmLoopbackCtrl_t   ctrl;


    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();


    ret = rtk_oam_cfmLoopbackReplyCtrl_get(unit, &ctrl);
    if ( (ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }



    if ('i' == TOKEN_CHAR(2, 5))
    {

    if(ENABLED == ctrl.keep_innerTag)
      diag_util_mprintf("Lbm Packet Auto Reply Keep Inner Vid : ENABLE\n");
    else
      diag_util_mprintf("Lbm Packet Auto Reply Keep Inner Vid : DISABLE\n");

    }
    else if ('o' == TOKEN_CHAR(2, 5))
    {

      if(ENABLED == ctrl.keep_outerTag)
      diag_util_mprintf("Lbm Packet Auto Reply Keep Outer Vid : ENABLE\n");
    else
      diag_util_mprintf("Lbm Packet Auto Reply Keep Outer Vid : DISABLE\n");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_ASIC_AUTO_DYING_GASP_PORT_PORTS_ALL_STATE_DISABLE_ENABLE
/*
 * oam set asic-auto-dying-gasp port ( <PORT_LIST:ports> | all ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_oam_set_asic_auto_dying_gasp_port_ports_all_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    rtk_enable_t            enable;
    diag_portlist_t         portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if('e' == TOKEN_CHAR(6, 0))
        enable = ENABLED;
    else if('d' == TOKEN_CHAR(6, 0))
        enable = DISABLED;
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_oam_autoDyingGaspEnable_set(unit, port, enable), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CCM_PORT_PORTS_ALL_START
/*
 * oam set ccm port ( <PORT_LIST:ports> | all ) start
 */
cparser_result_t cparser_cmd_oam_set_ccm_port_ports_all_start(cparser_context_t *context,
    char **port_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    diag_portlist_t       portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_ERR_CHK(rtk_oam_txCCMFrame_start(unit, &portlist.portmask), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CCM_STOP
/*
 * oam set ccm stop
 */
cparser_result_t cparser_cmd_oam_set_ccm_stop(cparser_context_t *context)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rtk_oam_txCCMFrame_stop(unit), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CCM_CFM_INDEX_INDEX_PKT_TYPE_ETHERNET_II_SNAP_DST_MAC_DST_MAC_SRC_MAC_SRC_MAC
/*
 * oam set ccm cfm-index <UINT:index> pkt-type ( ethernet-ii | snap ) dst-mac <MACADDR:dst_mac> src-mac <MACADDR:src_mac>
 */
cparser_result_t cparser_cmd_oam_set_ccm_cfm_index_index_pkt_type_ethernet_ii_snap_dst_mac_dst_mac_src_mac_src_mac(
    cparser_context_t *context,
    uint32_t *index_ptr,
    cparser_macaddr_t *dst_mac_ptr,
    cparser_macaddr_t *src_mac_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_oam_ccmFrame_t  config;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((index_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((dst_mac_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((src_mac_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    memset(&config, 0, sizeof(rtk_oam_ccmFrame_t));

    if('e' == TOKEN_CHAR(6, 0))
    {
        config.pktType = ETHERNET_II_PACKET;
    }
    else
    {
        config.pktType = SNAP_PACKET;
    }

    memcpy(&config.dest_mac.octet[0], dst_mac_ptr, sizeof(cparser_macaddr_t));
    memcpy(&config.source_mac.octet[0], src_mac_ptr, sizeof(cparser_macaddr_t));

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMFrame_set(unit, *index_ptr, &config), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CCM_CFM_INDEX_INDEX_INNER_TAG_TPID_TPID
/*
 * oam set ccm cfm-index <UINT:index> inner-tag tpid <HEX:tpid>
 */
cparser_result_t cparser_cmd_oam_set_ccm_cfm_index_index_inner_tag_tpid_tpid(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *tpid_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_oam_ccmFrame_t  config;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((index_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((tpid_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    memset(&config, 0, sizeof(rtk_oam_ccmFrame_t));

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMFrame_get(unit, *index_ptr, &config), ret);

    config.enable_innerTag = TRUE;
    config.inner_tpid = *tpid_ptr;
    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMFrame_set(unit, *index_ptr, &config), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CCM_CFM_INDEX_INDEX_INNER_TAG_VID_VID
/*
 * oam set ccm cfm-index <UINT:index> inner-tag vid <UINT:vid>
 */
cparser_result_t cparser_cmd_oam_set_ccm_cfm_index_index_inner_tag_vid_vid(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *vid_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_oam_ccmFrame_t  config;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((index_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((vid_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    memset(&config, 0, sizeof(rtk_oam_ccmFrame_t));

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMFrame_get(unit, *index_ptr, &config), ret);

    config.enable_innerTag = TRUE;
    config.inner_vid = *vid_ptr;
    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMFrame_set(unit, *index_ptr, &config), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CCM_CFM_INDEX_INDEX_INNER_TAG_PRIORITY_PRIORITY
/*
 * oam set ccm cfm-index <UINT:index> inner-tag priority <UINT:priority>
 */
cparser_result_t cparser_cmd_oam_set_ccm_cfm_index_index_inner_tag_priority_priority(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *pri_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_oam_ccmFrame_t  config;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((index_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((pri_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    memset(&config, 0, sizeof(rtk_oam_ccmFrame_t));

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMFrame_get(unit, *index_ptr, &config), ret);

    config.enable_innerTag = TRUE;
    config.inner_pri = *pri_ptr;
    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMFrame_set(unit, *index_ptr, &config), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CCM_CFM_INDEX_INDEX_INNER_TAG_CFI
/*
 * oam set ccm cfm-index <UINT:index> inner-tag cfi
 */
cparser_result_t cparser_cmd_oam_set_ccm_cfm_index_index_inner_tag_cfi(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_oam_ccmFrame_t  config;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((index_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    memset(&config, 0, sizeof(rtk_oam_ccmFrame_t));

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMFrame_get(unit, *index_ptr, &config), ret);

    config.enable_innerTag = TRUE;
    config.inner_cfi = 1;
    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMFrame_set(unit, *index_ptr, &config), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CCM_CFM_INDEX_INDEX_OUTER_TAG_TPID_TPID
/*
 * oam set ccm cfm-index <UINT:index> outer-tag tpid <HEX:tpid>
 */
cparser_result_t cparser_cmd_oam_set_ccm_cfm_index_index_outer_tag_tpid_tpid(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *tpid_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_oam_ccmFrame_t  config;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((index_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((tpid_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    memset(&config, 0, sizeof(rtk_oam_ccmFrame_t));

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMFrame_get(unit, *index_ptr, &config), ret);

    config.enable_outerTag= TRUE;
    config.outer_tpid = *tpid_ptr;
    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMFrame_set(unit, *index_ptr, &config), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CCM_CFM_INDEX_INDEX_OUTER_TAG_VID_VID
/*
 * oam set ccm cfm-index <UINT:index> outer-tag vid <UINT:vid>
 */
cparser_result_t cparser_cmd_oam_set_ccm_cfm_index_index_outer_tag_vid_vid(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *vid_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_oam_ccmFrame_t  config;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((index_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((vid_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    memset(&config, 0, sizeof(rtk_oam_ccmFrame_t));

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMFrame_get(unit, *index_ptr, &config), ret);

    config.enable_outerTag= TRUE;
    config.outer_vid = *vid_ptr;
    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMFrame_set(unit, *index_ptr, &config), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CCM_CFM_INDEX_INDEX_OUTER_TAG_PRIORITY_PRIORITY
/*
 * oam set ccm cfm-index <UINT:index> outer-tag priority <UINT:priority>
 */
cparser_result_t cparser_cmd_oam_set_ccm_cfm_index_index_outer_tag_priority_priority(
    cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *pri_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_oam_ccmFrame_t  config;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((index_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((pri_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    memset(&config, 0, sizeof(rtk_oam_ccmFrame_t));

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMFrame_get(unit, *index_ptr, &config), ret);

    config.enable_outerTag= TRUE;
    config.outer_pri = *pri_ptr;
    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMFrame_set(unit, *index_ptr, &config), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CCM_CFM_INDEX_INDEX_OUTER_TAG_DEI
/*
 * oam set ccm cfm-index <UINT:index> outer-tag dei
 */
cparser_result_t cparser_cmd_oam_set_ccm_cfm_index_index_outer_tag_dei(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_oam_ccmFrame_t  config;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((index_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    memset(&config, 0, sizeof(rtk_oam_ccmFrame_t));

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMFrame_get(unit, *index_ptr, &config), ret);

    config.enable_outerTag= TRUE;
    config.outer_dei = 1;
    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMFrame_set(unit, *index_ptr, &config), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CCM_CFM_INDEX_INDEX_CCM_FLAG_VALUE
/*
 * oam set ccm cfm-index <UINT:index> ccm-flag <HEX:value>
 */
cparser_result_t cparser_cmd_oam_set_ccm_cfm_index_index_ccm_flag_value(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *value_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((index_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((value_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMFlag_set(unit, *index_ptr, *value_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CCM_ETHER_TYPE_OPCODE_VALUE
/*
 *oam set ccm ( ether-type | opcode ) <HEX:value>
 */
cparser_result_t cparser_cmd_oam_set_ccm_ether_type_opcode_value(cparser_context_t *context,
    uint32_t *value_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((value_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    if('e' == TOKEN_CHAR(3, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMEtype_set(unit, *value_ptr), ret);
    }
    else if('o' == TOKEN_CHAR(3, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMOpcode_set(unit, *value_ptr), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CCM_INTERVAL_3DOT3MS_10MS_100MS_1S
/*
 *oam set ccm interval ( 3dot3ms | 10ms | 100ms | 1s )
 */
cparser_result_t cparser_cmd_oam_set_ccm_interval_3dot3ms_10ms_100ms_1s(cparser_context_t *context)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_oam_ccmInterval_t  interval;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    if('3' == TOKEN_CHAR(4, 0))
    {
        interval = INTERVAL_3_3_MS;
    }
    else if('1' == TOKEN_CHAR(4, 0))
    {
        if('s' == TOKEN_CHAR(4, 1))
            interval = INTERVAL_1_S;
        else if('m' == TOKEN_CHAR(4, 2))
            interval = INTERVAL_10_MS;
        else
            interval = INTERVAL_100_MS;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMInterval_set(unit, interval), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CCM_SNAP_OUI_VALUE
/*
 *oam set ccm snap-oui <HEX:value>
 */
cparser_result_t cparser_cmd_oam_set_ccm_snap_oui_value(cparser_context_t *context,
    uint32_t *value_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_snapOui_t      oui;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((value_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    oui.snapOui[0] = ((*value_ptr) >> 16) & 0xff;
    oui.snapOui[1] = ((*value_ptr) >> 8) & 0xff;
    oui.snapOui[2] = (*value_ptr) & 0xff;

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCCMSnapOui_set(unit, &oui), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CFM_INDEX_INDEX_MAID_INDEX_MAID_INDEX_MAID_MAID_VALUE
/*
 * oam set cfm-index <UINT:index> maid-index <UINT:maid_index> maid <HEX:maid_value>
 */
cparser_result_t cparser_cmd_oam_set_cfm_index_index_maid_index_maid_index_maid_maid_value(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *maid_index_ptr,
    uint32_t *maid_value_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_oam_cfm_t           entryContent;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((index_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((maid_index_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((maid_value_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*index_ptr > 1), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*maid_index_ptr > 47), CPARSER_ERR_INVALID_PARAMS);

    memset(&entryContent, 0, sizeof(rtk_oam_cfm_t));

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmEntry_get(unit, *index_ptr, &entryContent), ret);

    entryContent.maid[*maid_index_ptr] = *maid_value_ptr;

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmEntry_set(unit, *index_ptr, &entryContent), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CFM_INDEX_INDEX_MD_LEVEL_LEVEL
/*
 *oam set cfm-index <UINT:index> md-level <UINT:level>
 */
cparser_result_t cparser_cmd_oam_set_cfm_index_index_md_level_level(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *level_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_oam_cfm_t     entryContent;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((index_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((level_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*index_ptr > 1), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*level_ptr > 7), CPARSER_ERR_INVALID_PARAMS);

    memset(&entryContent, 0, sizeof(rtk_oam_cfm_t));

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmEntry_get(unit, *index_ptr, &entryContent), ret);

    entryContent.md_level = *level_ptr;

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmEntry_set(unit, *index_ptr, &entryContent), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CFM_MEPID_COMPARE_PORT_PORTS_ALL_STATE_DISABLE_ENABLE
/*
 *oam set cfm-mepid-compare port ( <PORT_LIST:ports> | all ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_oam_set_cfm_mepid_compare_port_ports_all_state_disable_enable(
    cparser_context_t *context,
    char **port_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t            port = 0;
    rtk_enable_t         enable = DISABLED;
    diag_portlist_t       portlist;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if('e' == TOKEN_CHAR(6, 0))
        enable = ENABLED;
    else if('d' == TOKEN_CHAR(6, 0))
        enable = DISABLED;

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_oam_cfmMepEnable_set(unit, port ,enable), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CFM_LOOPBACK_REPLY_PORT_PORTS_ALL_STATE_DISABLE_ENABLE
/*
 *oam set cfm-loopback-reply port ( <PORT_LIST:ports> | all ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_oam_set_cfm_loopback_reply_port_ports_all_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32                unit = 0;
    int32                 ret = RT_ERR_FAILED;
    rtk_port_t            port = 0;
    rtk_enable_t          enable = DISABLED;
    diag_portlist_t       portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if('e' == TOKEN_CHAR(6, 0))
        enable = ENABLED;
    else if('d' == TOKEN_CHAR(6, 0))
        enable = DISABLED;

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_oam_cfmLoopbackReplyEnable_set(unit, port ,enable), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_DYING_GASP_PORT_PORTS_ALL_LOCAL_TLV_REMOTE_TLV_TLV_VALUE
/*
 * oam set dying-gasp port ( <PORT_LIST:ports> | all ) ( local-tlv | remote-tlv ) <STRING:tlv_value>
 */
cparser_result_t cparser_cmd_oam_set_dying_gasp_port_ports_all_local_tlv_remote_tlv_tlv_value(
    cparser_context_t *context,
    char **ports_ptr,
    char **tlv_value_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    rtk_oam_dyingGaspTLV_t  tlv;
    diag_portlist_t         portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*tlv_value_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    memset(&tlv, 0, sizeof(rtk_oam_dyingGaspTLV_t));

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_oam_dyingGaspTLV_get(unit, port, &tlv), ret);

        if('r' == TOKEN_CHAR(5, 0))
        {
            _diag_util_str2Tlv(&tlv.remoteTLV[0], (uint8 *)*tlv_value_ptr, RTK_OAM_REMOTE_TLV_LEN);

            DIAG_UTIL_ERR_CHK(rtk_oam_dyingGaspTLV_set(unit, port, &tlv), ret);
        }
        else if('l' == TOKEN_CHAR(5, 0))
        {
            _diag_util_str2Tlv(&tlv.localTLV[0], (uint8 *)*tlv_value_ptr, RTK_OAM_LOCAL_TLV_LEN);

            DIAG_UTIL_ERR_CHK(rtk_oam_dyingGaspTLV_set(unit, port, &tlv), ret);
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

#ifdef CMD_OAM_SET_DYING_GASP_PORT_PORTS_ALL_START
/*
 * oam set dying-gasp port ( <PORT_LIST:ports> | all ) start
 */
cparser_result_t cparser_cmd_oam_set_dying_gasp_port_ports_all_start(cparser_context_t *context,
    char **port_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    diag_portlist_t         portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_ERR_CHK(rtk_oam_DyingGaspSend_start(unit, &portlist.portmask), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_DYING_GASP_SEND_PAYLOAD_PAYLOAD
/* convert Integer from string to number */
int32
diag_oam_str2IntArray(uint8 *int_array, uint8 *str, uint32 size)
{
    uint32 str_idx, array_idx, hi_bits;
    uint8  value = 0;

    if (!(int_array && str))
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DIAGSHELL), "");
        return  RT_ERR_FAILED;
    }

    if (!((strlen((char *)str) > 2) && ('0' == str[0]) && ('x' == str[1])))
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DIAGSHELL), "");
        return  RT_ERR_FAILED;
    }

    value = 0;
    array_idx = size - 1;
    hi_bits = 0;

    /* exclude 0x head */
    for (str_idx = (strlen((char *)str) - 1); str_idx >= 2; str_idx--)
    {
        if (('0' <= str[str_idx]) && ('9' >= str[str_idx]))
        {
            value = str[str_idx] - '0';
        }
        else if (('a' <= str[str_idx]) && ('f' >= str[str_idx]))
        {
            value = str[str_idx] - 'a' + 10;
        }
        else if (('A' <= str[str_idx]) && ('F' >= str[str_idx]))
        {
            value = str[str_idx] - 'A' + 10;
        }
        else
        {
            RT_ERR(RT_ERR_FAILED, (MOD_DIAGSHELL), "");
            return  RT_ERR_FAILED;
        }

        if(hi_bits == 1)
        {
            int_array[array_idx] = int_array[array_idx] + (value  << 4);
            hi_bits = 0;
            array_idx--;
        }
        else
        {
            int_array[array_idx] = value;
            hi_bits = 1;
        }
    }

    return RT_ERR_OK;
}   /* end of diag_oam_str2IntArray */

/*
 * oam set dying-gasp port ( <PORT_LIST:ports> | all ) payload <STRING:payload>
 */
cparser_result_t cparser_cmd_oam_set_dying_gasp_port_ports_all_payload_payload(cparser_context_t *context,
    char **port_ptr,
    char **payload_ptr)
{
    uint32                  unit = 0, size;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    diag_portlist_t         portlist;
    uint8                   data[RTK_OAM_DYINGGASPPAYLOAD_MAX];

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    size = strlen(*payload_ptr);
    if (size > RTK_OAM_DYINGGASPPAYLOAD_MAX)
    {
        diag_util_printf("The length of payload is too long!\n");
        return CPARSER_NOT_OK;
    }

    size = ((size - 2) + 1) / 2;
    if (diag_oam_str2IntArray(data, (uint8 *)*payload_ptr, size) != RT_ERR_OK)
    {
        diag_util_printf("field data error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_oam_portDyingGaspPayload_set(unit, port, data, size), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_DYING_GASP_SEND_ENABLE
/*
 * oam set dying-gasp send enable
 */
cparser_result_t cparser_cmd_oam_set_dying_gasp_send_enable(cparser_context_t *context)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rtk_oam_dyingGaspSend_set(unit, ENABLED), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_DYING_GASP_WAITING_TIME_TIME
/*
 * oam set dying-gasp waiting-time <UINT:time>
 */
cparser_result_t cparser_cmd_oam_set_dying_gasp_waiting_time_time(cparser_context_t *context,
    uint32_t *time_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((time_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_oam_dyingGaspWaitTime_set(unit, *time_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_LOOPBACK_CTRL_PORT_PORTS_ALL_SA_ACTION_KEEP_USE_SWITCH_MAC_USE_PKT_DMAC_DA_ACTION_KEEP_USE_SWITCH_MAC_USE_PKT_SMAC
/*
 * oam set loopback-ctrl port ( <PORT_LIST:ports> | all ) sa-action ( keep | use-switch-mac | use-pkt-dmac ) da-action ( keep | use-switch-mac | use-pkt-smac )
 */
cparser_result_t cparser_cmd_oam_set_loopback_ctrl_port_ports_all_sa_action_keep_use_switch_mac_use_pkt_dmac_da_action_keep_use_switch_mac_use_pkt_smac(cparser_context_t *context,
    char **port_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    rtk_oam_loopbackCtrl_t  lbCtrl;
    diag_portlist_t         portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    memset(&lbCtrl, 0, sizeof(rtk_oam_loopbackCtrl_t));

    if('k' == TOKEN_CHAR(6, 0))
        lbCtrl.sa_action = SA_ACTION_KEEP_MAC;
    else if('u' == TOKEN_CHAR(6, 0))
    {
        if('s' == TOKEN_CHAR(6, 4))
            lbCtrl.sa_action = SA_ACTION_USE_SWITCH_MAC;
        else
            lbCtrl.sa_action = SA_ACTION_USE_DA;
    }

    if('k' == TOKEN_CHAR(8, 0))
        lbCtrl.da_action = DA_ACTION_KEEP_MAC;
    else if('u' == TOKEN_CHAR(8, 0))
    {
        if('s' == TOKEN_CHAR(8, 4))
            lbCtrl.da_action = DA_ACTION_USE_SWITCH_MAC;
        else
            lbCtrl.da_action = DA_ACTION_USE_SA;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_oam_loopbackCtrl_set(unit, port,  &lbCtrl), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_LOOPBACK_CTRL_PORT_PORTS_ALL_SA_ACTION_KEEP_USE_SWITCH_MAC_USE_PKT_DMAC_DA_ACTION_USER_DEFINED_MAC_DMAC
/*
 * oam set loopback-ctrl port ( <PORT_LIST:ports> | all ) sa-action ( keep | use-switch-mac | use-pkt-dmac ) da-action user-defined-mac <MACADDR:dmac>
 */
cparser_result_t cparser_cmd_oam_set_loopback_ctrl_port_ports_all_sa_action_keep_use_switch_mac_use_pkt_dmac_da_action_user_defined_mac_dmac(cparser_context_t *context,
    char **port_ptr,
    cparser_macaddr_t *dmac_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    rtk_oam_loopbackCtrl_t  lbCtrl;
    diag_portlist_t         portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((dmac_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    memset(&lbCtrl, 0, sizeof(rtk_oam_loopbackCtrl_t));

    if('k' == TOKEN_CHAR(6, 0))
        lbCtrl.sa_action = SA_ACTION_KEEP_MAC;
    else if('u' == TOKEN_CHAR(6, 0))
    {
        if('s' == TOKEN_CHAR(6, 4))
            lbCtrl.sa_action = SA_ACTION_USE_SWITCH_MAC;
        else
            lbCtrl.sa_action = SA_ACTION_USE_DA;
    }

    lbCtrl.da_action = SA_ACTION_USE_USER_DEFINE_MAC;
    memcpy(&lbCtrl.user_defined_dst_mac.octet[0], dmac_ptr, sizeof(cparser_macaddr_t));

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_oam_loopbackCtrl_set(unit, port,  &lbCtrl), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_LOOPBACK_CTRL_PORT_PORTS_ALL_SA_ACTION_USER_DEFINED_MAC_SMAC_DA_ACTION_KEEP_USE_SWITCH_MAC_USE_PKT_SMAC
/*
  * oam set loopback-ctrl port ( <PORT_LIST:ports> | all ) sa-action user-defined-mac <MACADDR:smac> da-action ( keep | use-switch-mac | use-pkt-smac )
  */
cparser_result_t cparser_cmd_oam_set_loopback_ctrl_port_ports_all_sa_action_user_defined_mac_smac_da_action_keep_use_switch_mac_use_pkt_smac(
    cparser_context_t *context,
    char **ports_ptr,
    cparser_macaddr_t *smac_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    rtk_oam_loopbackCtrl_t  lbCtrl;
    diag_portlist_t         portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((smac_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    memset(&lbCtrl, 0, sizeof(rtk_oam_loopbackCtrl_t));

    lbCtrl.sa_action = SA_ACTION_USE_USER_DEFINE_MAC;
    memcpy(&lbCtrl.user_defined_src_mac.octet[0], smac_ptr, sizeof(cparser_macaddr_t));

    if('k' == TOKEN_CHAR(6, 0))
        lbCtrl.da_action = DA_ACTION_KEEP_MAC;
    else if('u' == TOKEN_CHAR(6, 0))
    {
        if('s' == TOKEN_CHAR(6, 4))
            lbCtrl.da_action = DA_ACTION_USE_SWITCH_MAC;
        else
            lbCtrl.da_action = DA_ACTION_USE_SA;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_oam_loopbackCtrl_set(unit, port,  &lbCtrl), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_LOOPBACK_CTRL_PORT_PORTS_ALL_SA_ACTION_USER_DEFINED_MAC_SMAC_DA_ACTION_USER_DEFINED_MAC_DMAC
/*
 * oam set loopback-ctrl port ( <PORT_LIST:ports> | all ) sa-action user-defined-mac <MACADDR:smac> da-action user-defined-mac <MACADDR:dmac>
 */
cparser_result_t cparser_cmd_oam_set_loopback_ctrl_port_ports_all_sa_action_user_defined_mac_smac_da_action_user_defined_mac_dmac(
    cparser_context_t *context,
    char **ports_ptr,
    cparser_macaddr_t *smac_ptr,
    cparser_macaddr_t *dmac_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    rtk_oam_loopbackCtrl_t  lbCtrl;
    diag_portlist_t         portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((smac_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((dmac_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    memset(&lbCtrl, 0, sizeof(rtk_oam_loopbackCtrl_t));

    lbCtrl.sa_action = SA_ACTION_USE_USER_DEFINE_MAC;
    memcpy(&lbCtrl.user_defined_src_mac.octet[0], smac_ptr, sizeof(cparser_macaddr_t));

    lbCtrl.da_action = DA_ACTION_USE_USER_DEFINE_MAC;
    memcpy(&lbCtrl.user_defined_dst_mac.octet[0], dmac_ptr, sizeof(cparser_macaddr_t));

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_oam_loopbackCtrl_set(unit, port,  &lbCtrl), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_LOOPBACK_MODE_PORT_PORTS_ALL_LB_ALL_PKT_LB_TEST_PKT_ONLY_DROP_TEST_PKT_LB_OFF
/*
 * oam set loopback-mode port ( <PORT_LIST:ports> | all ) ( lb-all-pkt | lb-test-pkt-only | drop-test-pkt | lb-off )
 */
cparser_result_t cparser_cmd_oam_set_loopback_mode_port_ports_all_lb_all_pkt_lb_test_pkt_only_drop_test_pkt_lb_off(cparser_context_t *context,
    char **ports_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    rtk_oam_loopbackMode_t lbMode;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    if('a' == TOKEN_CHAR(5, 3))
        lbMode = OAM_LOOPBACK_ALL_FRAME;
    else if('t' == TOKEN_CHAR(5, 3))
        lbMode = OAM_LOOPBACK_TEST_FRAME;
    else if('d' == TOKEN_CHAR(5, 0))
        lbMode = OAM_LOOPBACK_DROP_TEST_FRAME;
    else if('o' == TOKEN_CHAR(5, 3))
        lbMode = OAM_LOOPBACK_DISABLE;
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_oam_loopbackMode_set(unit, port, lbMode), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_KEEP_INNER_TAG_KEEP_OUTER_TAG_STATE_DISABLE_ENABLE
/*
 * oam set ( keep-inner-tag | keep-outer-tag ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_oam_set_keep_inner_tag_keep_outer_tag_state_disable_enable(cparser_context_t *context)
{
    uint32                      unit = 0;
    int32                       ret = RT_ERR_FAILED;
    rtk_enable_t                enable = DISABLED;
    rtk_oam_cfmLoopbackCtrl_t   loopbackCtrl;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    memset(&loopbackCtrl, 0, sizeof(rtk_oam_cfmLoopbackCtrl_t));

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmLoopbackReplyCtrl_get(unit, &loopbackCtrl), ret);

    if('e' == TOKEN_CHAR(4, 0))
        enable = ENABLED;
    else if('d' == TOKEN_CHAR(4, 0))
        enable = DISABLED;

    if('i' == TOKEN_CHAR(2, 5))
        loopbackCtrl.keep_innerTag = enable;
    else if('o' == TOKEN_CHAR(2, 5))
        loopbackCtrl.keep_outerTag = enable;

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmLoopbackReplyCtrl_set(unit, &loopbackCtrl), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CCM_PORT_PORTS_ALL_GROUP_ID_INDEX_MEPID_MEPID
/*
 * oam set ccm port ( <PORT_LIST:ports> | all ) group-id <UINT:index> mepid <HEX:mepid>
 */
cparser_result_t cparser_cmd_oam_set_ccm_port_ports_all_group_id_index_mepid_mepid(
    cparser_context_t *context,
    char **port_ptr,
    uint32_t *group_id_ptr,
    uint32_t *mepid_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    rtk_oam_cfmPort_t       entryContent;
    diag_portlist_t         portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((group_id_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*group_id_ptr > 1), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((mepid_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    memset(&entryContent, 0, sizeof(rtk_oam_cfmPort_t));
    entryContent.cfm_idx = *group_id_ptr;
    entryContent.mepid = *mepid_ptr;

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_oam_cfmPortEntry_set(unit, port, &entryContent), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CCM_PORT_PORTS_ALL_INTERFACE_STATUS_PORT_STATUS_VALUE
/*
 * oam set ccm port ( <PORT_LIST:ports> | all ) ( interface-status | port-status ) <UINT:value>
 */
cparser_result_t cparser_cmd_oam_set_ccm_port_ports_all_interface_status_port_status_value(cparser_context_t *context,
    char **port_ptr,
    uint32_t *value_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port = 0;
    diag_portlist_t         portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((value_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if('i' == TOKEN_CHAR(5, 0))
        {
            if((*value_ptr < 1) || (*value_ptr > 7))
            {
                diag_util_printf("interface status out of range");
                return CPARSER_NOT_OK;
            }

            DIAG_UTIL_ERR_CHK(rtk_oam_cfmIntfStatus_set(unit, port ,*value_ptr), ret);

        }
        else if('p' == TOKEN_CHAR(5, 0))
        {
            if((*value_ptr < 1) || (*value_ptr > 2))
            {
                diag_util_printf("port status out of range");
                return CPARSER_NOT_OK;
            }

            DIAG_UTIL_ERR_CHK(rtk_oam_cfmPortStatus_set(unit, port ,*value_ptr), ret);
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_SPG_TX_PORT_LIST_PORT_ALL_CONTINUS_MODE_PKT_LEN_INTERVAL_START
/*
 * oam set spg tx-port-list ( <PORT_LIST:port> | all ) continus-mode <UINT:pkt_len> <UINT:interval> start
 */
cparser_result_t cparser_cmd_oam_set_spg_tx_port_list_port_all_continus_mode_pkt_len_interval_start(cparser_context_t *context,
    char **port_ptr,
    uint32_t *pkt_len_ptr,
    uint32_t *interval_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_oam_testFrameCfg_t  config;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((pkt_len_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((interval_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    memset(&config, 0, sizeof(rtk_oam_testFrameCfg_t));

    /*get tx port mask*/
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    config.txPortmask = portlist.portmask;

    /*continus mode*/
    config.continuousTx = TRUE;

    config.length = *pkt_len_ptr;
    config.frameInterval = *interval_ptr;

    DIAG_UTIL_ERR_CHK(rtk_oam_txTestFrame_start(unit, &config), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_SPG_TX_PORT_LIST_PORT_ALL_FIX_COUNT_MODE_COUNT_PKT_LEN_INTERVAL_START
/*
 * oam set spg tx-port-list ( <PORT_LIST:port> | all ) fix-count-mode <UINT:count> <UINT:pkt_len> <UINT:interval> start
 */
cparser_result_t cparser_cmd_oam_set_spg_tx_port_list_port_all_fix_count_mode_count_pkt_len_interval_start(cparser_context_t *context,
    char **port_ptr,
    uint32_t *count_ptr,
    uint32_t *pkt_len_ptr,
    uint32_t *interval_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_oam_testFrameCfg_t  config;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((pkt_len_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK(!((*pkt_len_ptr >= MIN_PKTLEN) && (*pkt_len_ptr < MAX_PKTLEN)), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((count_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((interval_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    memset(&config, 0, sizeof(rtk_oam_testFrameCfg_t));

    /*get tx port mask*/
   DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    config.txPortmask = portlist.portmask;

    /*fix count mode*/
    config.continuousTx = FALSE;
    config.frameCount = *count_ptr;
    config.length = *pkt_len_ptr;
    config.frameInterval = *interval_ptr;

    DIAG_UTIL_ERR_CHK(rtk_oam_txTestFrame_start(unit, &config), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_SPG_STOP
/*
 * oam set spg stop
 */
cparser_result_t cparser_cmd_oam_set_spg_stop(cparser_context_t *context)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_oam_txTestFrame_stop(unit), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_LOOPBACK_CTRL_MAC_SWAP_STATE
/*
 * oam get loopback-ctrl mac-swap state
 */
cparser_result_t
cparser_cmd_oam_get_loopback_ctrl_mac_swap_state(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    enable;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_oam_loopbackMacSwapEnable_get(unit, &enable);
    if ((ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if( DISABLED == enable )
        diag_util_mprintf("OAM MAC swap state: Disable\n");
    else if( ENABLED == enable )
        diag_util_mprintf("OAM MAC swap state: Enable\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_LOOPBACK_CTRL_PORT_PORTS_ALL_MUX
/*
 * oam get loopback-ctrl port ( <PORT_LIST:ports> | all ) mux
 */
cparser_result_t
cparser_cmd_oam_get_loopback_ctrl_port_ports_all_mux(cparser_context_t *context,
    char **ports_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t      port = 0;
    rtk_action_t    action;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        ret = rtk_oam_portLoopbackMuxAction_get(unit, port, &action);
        if ((ret!= RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

        diag_util_mprintf("\tPort %u:\n", port);
        if( ACTION_DROP == action )
            diag_util_mprintf("\t\tOAM multiplexer action: Drop\n");
        else if( ACTION_FORWARD == action )
            diag_util_mprintf("\t\tOAM multiplexer action: Forward\n");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_LOOPBACK_CTRL_MAC_SWAP_STATE_DISABLE_ENABLE
/*
 * oam set loopback-ctrl mac-swap state ( disable | enable )
 */
cparser_result_t
cparser_cmd_oam_set_loopback_ctrl_mac_swap_state_disable_enable(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    enable;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    if('e' == TOKEN_CHAR(5, 0))
        enable = ENABLED;
    else if('d' == TOKEN_CHAR(5, 0))
        enable = DISABLED;
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_oam_loopbackMacSwapEnable_set(unit, enable), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_LOOPBACK_CTRL_PORT_PORTS_ALL_MUX_ACTION_DROP_FORWARD
/*
 * oam set loopback-ctrl port ( <PORT_LIST:ports> | all ) mux action ( drop | forward )
 */
cparser_result_t
cparser_cmd_oam_set_loopback_ctrl_port_ports_all_mux_action_drop_forward(cparser_context_t *context,
    char **ports_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_action_t    muxAction;
    diag_portlist_t portlist;
    rtk_port_t      port = 0;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if('d' == TOKEN_CHAR(7, 0))
        muxAction = ACTION_DROP;
    else if('f' == TOKEN_CHAR(7, 0))
        muxAction = ACTION_FORWARD;
    else
    {
        diag_util_printf("User config: muxAction Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_oam_portLoopbackMuxAction_set(unit, port, muxAction), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CFM_CCM_PCP
/* oam get cfm ccm pcp */
cparser_result_t
cparser_cmd_oam_get_cfm_ccm_pcp(cparser_context_t *context)
{
    uint32  unit = 0;
    uint32  pcp;
    int32   ret;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_oam_cfmCcmPcp_get(unit, &pcp);
    if (ret != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("\tPriority Code Point value for CCM tx frame: %d\n", pcp);

    return CPARSER_OK;
}   /* end of cparser_cmd_oam_get_cfm_ccm_pcp */
#endif

#ifdef CMD_OAM_SET_CFM_CCM_PCP_VALUE
/* oam set cfm ccm pcp <UINT:value> */
cparser_result_t
cparser_cmd_oam_set_cfm_ccm_pcp_value(cparser_context_t *context,
                                      uint32_t *value_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((value_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*value_ptr > RTK_DOT1P_PRIORITY_MAX), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCcmPcp_set(unit, *value_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CFM_CCM_CFI
/* oam get cfm ccm cfi */
cparser_result_t
cparser_cmd_oam_get_cfm_ccm_cfi(cparser_context_t *context)
{
    uint32  unit = 0;
    uint32  cfi;
    int32   ret;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_oam_cfmCcmCfi_get(unit, &cfi);
    if (ret != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("\tCanonical Format Identifier value for CCM tx frame: %d\n", cfi);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CFM_CCM_CFI_VALUE
/* oam set cfm ccm cfi <UINT:value> */
cparser_result_t
cparser_cmd_oam_set_cfm_ccm_cfi_value(cparser_context_t *context,
                                      uint32_t *value_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((value_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*value_ptr > RTK_DOT1P_DEI_MAX), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCcmCfi_set(unit, *value_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CFM_CCM_TPID
/* oam get cfm ccm tpid */
cparser_result_t
cparser_cmd_oam_get_cfm_ccm_tpid(cparser_context_t *context)
{
    uint32  unit = 0;
    uint32  tpid;
    int32   ret;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_oam_cfmCcmTpid_get(unit, &tpid);
    if (ret != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("\tTPID value for CCM tx frame: 0x%04x\n", tpid);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CFM_CCM_TPID_VALUE
/* oam set cfm ccm tpid <HEX:value> */
cparser_result_t
cparser_cmd_oam_set_cfm_ccm_tpid_value(cparser_context_t *context,
                                       uint32_t *value_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((value_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*value_ptr > RTK_TPID_MAX), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCcmTpid_set(unit, *value_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CFM_CCM_INSTANCE_LIFETIME
/* oam get cfm ccm <UINT:instance> lifetime */
cparser_result_t
cparser_cmd_oam_get_cfm_ccm_instance_lifetime(cparser_context_t *context,
                                                    uint32_t *instance_ptr)
{
    uint32  unit = 0;
    uint32  lifetime;
    int32   ret;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((instance_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*instance_ptr >= RTK_CFM_TX_INSTANCE_MAX), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_oam_cfmCcmInstLifetime_get(unit, *instance_ptr, &lifetime);
    if (ret != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("\tCFM CCM instance %d reset lifetime: %d\n",
                      *instance_ptr, lifetime);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CFM_CCM_INSTANCE_LIFETIME_VALUE
/* oam set cfm ccm <UINT:instance> lifetime <UINT:value> */
cparser_result_t
cparser_cmd_oam_set_cfm_ccm_instance_lifetime_value(cparser_context_t *context,
                                                          uint32_t *instance_ptr,
                                                          uint32_t *value_ptr)
{
    uint32  unit = 0;
    int32   ret;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((instance_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*instance_ptr >= RTK_CFM_TX_INSTANCE_MAX), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((value_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*value_ptr > RTK_CFM_RESET_LIFETIME_MAX), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCcmInstLifetime_set(unit, *instance_ptr, *value_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CFM_CCM_MEPID
/* oam get cfm ccm mepid */
cparser_result_t
cparser_cmd_oam_get_cfm_ccm_mepid(cparser_context_t *context)
{
    uint32  unit = 0;
    uint32  mepid;
    int32   ret;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_oam_cfmCcmMepid_get(unit, &mepid);
    if (ret != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("\tCFM CCM mepid: %d\n", mepid);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CFM_CCM_MEPID_MEPID
/* oam set cfm ccm mepid <UINT:mepid> */
cparser_result_t
cparser_cmd_oam_set_cfm_ccm_mepid_mepid(cparser_context_t *context,
                                        uint32_t *mepid_ptr)
{
    uint32  unit = 0;
    int32   ret;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((mepid_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*mepid_ptr > RTK_CFM_MEPID_MAX), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCcmMepid_set(unit, *mepid_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CFM_CCM_LIFETIME
/* oam get cfm ccm lifetime */
cparser_result_t
cparser_cmd_oam_get_cfm_ccm_lifetime(cparser_context_t *context)
{
    uint32  unit = 0;
    uint32  lifetime;
    int32   ret;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_oam_cfmCcmIntervalField_get(unit, &lifetime);
    if (ret != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("\tCFM CCM lifetime: %d\n", lifetime);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CFM_CCM_LIFETIME_LIFETIME
/* oam set cfm ccm lifetime <UINT:lifetime> */
cparser_result_t
cparser_cmd_oam_set_cfm_ccm_lifetime_lifetime(cparser_context_t *context,
                                              uint32_t *lifetime_ptr)
{
    uint32  unit = 0;
    int32   ret;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((lifetime_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*lifetime_ptr >= RTK_CFM_LIFETIME_MAX), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCcmIntervalField_set(unit, *lifetime_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CFM_CCM_MDL
/* oam get cfm ccm mdl */
cparser_result_t
cparser_cmd_oam_get_cfm_ccm_mdl(cparser_context_t *context)
{
    uint32  unit = 0;
    uint32  level;
    int32   ret;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_oam_cfmCcmMdl_get(unit, &level);
    if (ret != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("\tCFM CCM MD level %d\n", level);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CFM_CCM_MDL_MDL
/* oam set cfm ccm mdl <UINT:mdl> */
cparser_result_t
cparser_cmd_oam_set_cfm_ccm_mdl_mdl(cparser_context_t *context,
                                    uint32_t *mdl_ptr)
{
    uint32  unit = 0;
    int32   ret;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((mdl_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*mdl_ptr >= RTK_CFM_MDL_MAX), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCcmMdl_set(unit, *mdl_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CFM_CCM_INSTANCE_TAG_STATE
/* oam get cfm ccm <UINT:instance> tag-state */
cparser_result_t
cparser_cmd_oam_get_cfm_ccm_instance_tag_state(cparser_context_t *context,
                                                uint32_t *instance_ptr)
{
    uint32          unit = 0;
    int32           ret;
    rtk_enable_t    enable;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((instance_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*instance_ptr >= RTK_CFM_TX_INSTANCE_MAX), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_oam_cfmCcmInstTagStatus_get(unit, *instance_ptr, &enable);
    if (ret != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("CFM CCM instance %d tag status: ", *instance_ptr);
    if( DISABLED == enable )
        diag_util_mprintf("Disable\n");
    else if( ENABLED == enable )
        diag_util_mprintf("Enable\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CFM_CCM_INSTANCE_TAG_STATE_DISABLE_ENABLE
/* oam set cfm ccm <UINT:instance> tag-state ( disable | enable ) */
cparser_result_t
cparser_cmd_oam_set_cfm_ccm_instance_tag_state_disable_enable(cparser_context_t *context,
                                                               uint32_t *instance_ptr)
{
    uint32          unit = 0;
    int32           ret;
    rtk_enable_t    enable;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((instance_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*instance_ptr >= RTK_CFM_TX_INSTANCE_MAX), CPARSER_ERR_INVALID_PARAMS);

    if('e' == TOKEN_CHAR(6, 0))
        enable = ENABLED;
    else if('d' == TOKEN_CHAR(6, 0))
        enable = DISABLED;
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCcmInstTagStatus_set(unit, *instance_ptr, enable), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CFM_CCM_INSTANCE_VID
/* oam get cfm ccm <UINT:instance> vid */
cparser_result_t
cparser_cmd_oam_get_cfm_ccm_instance_vid(cparser_context_t *context,
                                         uint32_t *instance_ptr)
{
    uint32      unit = 0;
    int32       ret;
    rtk_vlan_t  vid;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((instance_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*instance_ptr >= RTK_CFM_TX_INSTANCE_MAX), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_oam_cfmCcmInstVid_get(unit, *instance_ptr, &vid);
    if (ret != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("\tCFM CCM instance %d vid: %d\n", *instance_ptr, vid);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CFM_CCM_INSTANCE_VID_VID
/* oam set cfm ccm <UINT:instance> vid <UINT:vid> */
cparser_result_t
cparser_cmd_oam_set_cfm_ccm_instance_vid_vid(cparser_context_t *context,
                                             uint32_t *instance_ptr,
                                             uint32_t *vid_ptr)
{
    uint32  unit = 0;
    int32   ret;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((instance_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*instance_ptr >= RTK_CFM_TX_INSTANCE_MAX), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((vid_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*vid_ptr > RTK_VLAN_ID_MAX), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCcmInstVid_set(unit, *instance_ptr, *vid_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CFM_CCM_INSTANCE_MAID
/* oam get cfm ccm <UINT:instance> maid */
cparser_result_t
cparser_cmd_oam_get_cfm_ccm_instance_maid(cparser_context_t *context,
                                          uint32_t *instance_ptr)
{
    uint32  unit = 0;
    uint32  maid;
    int32   ret;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((instance_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*instance_ptr >= RTK_CFM_TX_INSTANCE_MAX), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_oam_cfmCcmInstMaid_get(unit, *instance_ptr, &maid);
    if (ret != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("\tCFM CCM instance %d MAID: %d\n", *instance_ptr, maid);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CFM_CCM_INSTANCE_MAID_MAID
/* oam set cfm ccm <UINT:instance> maid <UINT:maid> */
cparser_result_t
cparser_cmd_oam_set_cfm_ccm_instance_maid_maid(cparser_context_t *context,
                                               uint32_t *instance_ptr,
                                               uint32_t *maid_ptr)
{
    uint32  unit = 0;
    int32   ret;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((instance_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*instance_ptr >= RTK_CFM_TX_INSTANCE_MAX), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((maid_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*maid_ptr >= RTK_CFM_MAID_MAX), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCcmInstMaid_set(unit, *instance_ptr, *maid_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CFM_CCM_INSTANCE_TX_STATE
/* oam get cfm ccm <UINT:instance> tx-state */
cparser_result_t
cparser_cmd_oam_get_cfm_ccm_instance_tx_state(cparser_context_t *context,
                                               uint32_t *instance_ptr)
{
    uint32          unit = 0;
    int32           ret;
    rtk_enable_t    enable;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((instance_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*instance_ptr >= RTK_CFM_TX_INSTANCE_MAX), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_oam_cfmCcmInstTxStatus_get(unit, *instance_ptr, &enable);
    if (ret != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("CFM CCM instance %d tx status: ", *instance_ptr);
    if( DISABLED == enable )
        diag_util_mprintf("Disable\n");
    else if( ENABLED == enable )
        diag_util_mprintf("Enable\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CFM_CCM_INSTANCE_TX_STATE_DISABLE_ENABLE
/* oam set cfm ccm <UINT:instance> tx-state ( disable | enable ) */
cparser_result_t
cparser_cmd_oam_set_cfm_ccm_instance_tx_state_disable_enable(cparser_context_t *context,
                                                              uint32_t *instance_ptr)
{
    uint32          unit = 0;
    int32           ret;
    rtk_enable_t    enable;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((instance_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*instance_ptr >= RTK_CFM_TX_INSTANCE_MAX), CPARSER_ERR_INVALID_PARAMS);

    if('e' == TOKEN_CHAR(6, 0))
        enable = ENABLED;
    else if('d' == TOKEN_CHAR(6, 0))
        enable = DISABLED;
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCcmInstTxStatus_set(unit, *instance_ptr, enable), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CFM_CCM_INSTANCE_INTERVAL
/* oam get cfm ccm <UINT:instance> interval */
cparser_result_t
cparser_cmd_oam_get_cfm_ccm_instance_interval(cparser_context_t *context,
                                              uint32_t *instance_ptr)
{
    uint32  unit = 0;
    uint32  interval;
    int32   ret;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((instance_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*instance_ptr >= RTK_CFM_TX_INSTANCE_MAX), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_oam_cfmCcmInstInterval_get(unit, *instance_ptr, &interval);
    if (ret != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("\tCFM CCM instance %d interval: %d\n", *instance_ptr, interval);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CFM_CCM_INSTANCE_INTERVAL_INTERVAL
/* oam set cfm ccm <UINT:instance> interval <UINT:interval> */
cparser_result_t
cparser_cmd_oam_set_cfm_ccm_instance_interval_interval(cparser_context_t *context,
                                                       uint32_t *instance_ptr,
                                                       uint32_t *interval_ptr)
{
    uint32  unit = 0;
    int32   ret;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((instance_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*instance_ptr >= RTK_CFM_TX_INSTANCE_MAX), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((interval_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*interval_ptr > RTK_CFM_TX_INTERVAL), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCcmInstInterval_set(unit, *instance_ptr, *interval_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CFM_CCM_INSTANCE_TX_PORT_INDEX_PORT
/* oam get cfm ccm <UINT:instance> tx <UINT:port_index> port */
cparser_result_t
cparser_cmd_oam_get_cfm_ccm_instance_tx_port_index_port(cparser_context_t *context,
                                                   uint32_t *instance_ptr,
                                                   uint32_t *port_index_ptr)
{
    uint32      unit = 0;
    int32       ret;
    rtk_port_t  port;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((instance_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*instance_ptr >= RTK_CFM_TX_INSTANCE_MAX), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((port_index_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*port_index_ptr >= RTK_CFM_CCM_PORT_MAX), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_oam_cfmCcmTxInstPort_get(unit, *instance_ptr, *port_index_ptr, &port);
    if (ret != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("\tCFM CCM TX instance %d index %d port: %d\n",
                      *instance_ptr, *port_index_ptr, port);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CFM_CCM_INSTANCE_TX_PORT_INDEX_PORT_PORT
/* oam set cfm ccm <UINT:instance> tx <UINT:port_index> port <UINT:port> */
cparser_result_t
cparser_cmd_oam_set_cfm_ccm_instance_tx_port_index_port_port(cparser_context_t *context,
                                                        uint32_t *instance_ptr,
                                                        uint32_t *port_index_ptr,
                                                        uint32_t *port_ptr)
{
    uint32  unit = 0;
    int32   ret;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((instance_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*instance_ptr >= RTK_CFM_TX_INSTANCE_MAX), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((port_index_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*port_index_ptr >= RTK_CFM_CCM_PORT_MAX), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((port_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*port_ptr >= RTK_MAX_NUM_OF_PORTS), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCcmTxInstPort_set(unit, *instance_ptr, *port_index_ptr,
                      *port_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CFM_CCM_INSTANCE_RX_VID
/* oam get cfm ccm <UINT:instance> rx vid */
cparser_result_t
cparser_cmd_oam_get_cfm_ccm_instance_rx_vid(cparser_context_t *context,
                                            uint32_t *instance_ptr)
{
    uint32      unit = 0;
    int32       ret;
    rtk_vlan_t  vid;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((instance_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*instance_ptr >= RTK_CFM_RX_INSTANCE_MAX), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_oam_cfmCcmRxInstVid_get(unit, *instance_ptr, &vid);
    if (ret != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("\tCFM CCM instance %d rx vid: %d\n", *instance_ptr, vid);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CFM_CCM_INSTANCE_RX_VID_VID
/* oam set cfm ccm <UINT:instance> rx vid <UINT:vid> */
cparser_result_t
cparser_cmd_oam_set_cfm_ccm_instance_rx_vid_vid(cparser_context_t *context,
                                                uint32_t *instance_ptr,
                                                uint32_t *vid_ptr)
{
    uint32  unit = 0;
    int32   ret;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((instance_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*instance_ptr >= RTK_CFM_RX_INSTANCE_MAX), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((vid_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*vid_ptr > RTK_VLAN_ID_MAX), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCcmRxInstVid_set(unit, *instance_ptr, *vid_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CFM_CCM_INSTANCE_RX_PORT_INDEX_PORT
/* oam get cfm ccm <UINT:instance> rx <UINT:port_index> port */
cparser_result_t
cparser_cmd_oam_get_cfm_ccm_instance_rx_port_index_port(cparser_context_t *context,
                                                   uint32_t *instance_ptr,
                                                   uint32_t *port_index_ptr)
{
    uint32      unit = 0;
    int32       ret;
    rtk_port_t  port;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((instance_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*instance_ptr >= RTK_CFM_RX_INSTANCE_MAX), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((port_index_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*port_index_ptr >= RTK_CFM_CCM_PORT_MAX), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_oam_cfmCcmRxInstPort_get(unit, *instance_ptr, *port_index_ptr, &port);
    if (ret != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("\tCFM CCM RX instance %d index %d port: %d\n",
                      *instance_ptr, *port_index_ptr, port);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CFM_CCM_INSTANCE_RX_PORT_INDEX_PORT_PORT
/* oam set cfm ccm <UINT:instance> rx <UINT:port_index> port <UINT:port> */
cparser_result_t
cparser_cmd_oam_set_cfm_ccm_instance_rx_port_index_port_port(cparser_context_t *context,
                                                        uint32_t *instance_ptr,
                                                        uint32_t *port_index_ptr,
                                                        uint32_t *port_ptr)
{
    uint32  unit = 0;
    int32   ret;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((instance_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*instance_ptr >= RTK_CFM_RX_INSTANCE_MAX), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((port_index_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*port_index_ptr >= RTK_CFM_CCM_PORT_MAX), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((port_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*port_ptr >= RTK_MAX_NUM_OF_PORTS), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmCcmRxInstPort_set(unit, *instance_ptr, *port_index_ptr,
                      *port_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CFM_CCM_INSTANCE_INDEX_KEEPALIVE_COUNTER
/* oam get cfm ccm <UINT:instance> <UINT:index> keepalive-counter */
cparser_result_t
cparser_cmd_oam_get_cfm_ccm_instance_index_keepalive_counter(cparser_context_t *context,
                                                     uint32_t *instance_ptr,
                                                     uint32_t *index_ptr)
{
    uint32  unit = 0;
    uint32  keepalive;
    int32   ret;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((instance_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*instance_ptr >= RTK_CFM_RX_INSTANCE_MAX), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((index_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*index_ptr >= RTK_CFM_CCM_PORT_MAX), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_oam_cfmCcmInstAliveTime_get(unit, *instance_ptr, *index_ptr, &keepalive);
    if (ret != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("\tCFM CCM RX instance %d index %d keep alive: %d\n",
                      *instance_ptr, *index_ptr, keepalive);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CFM_ETH_DM_PORT_ALL_STATE
/* oam get cfm eth-dm port ( <PORT_LIST:port> | all ) state */
cparser_result_t
cparser_cmd_oam_get_cfm_eth_dm_port_port_all_state(cparser_context_t *context,
    char **port_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    rtk_enable_t    enable;
    diag_portlist_t portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("\tPort %u:\n", port);
        diag_util_mprintf("\tCFM ETH-DM state: ");

        ret = rtk_oam_cfmPortEthDmEnable_get(unit, port, &enable);
        if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            if(ENABLED == enable)
                diag_util_mprintf("Enabled\n");
            else
                diag_util_mprintf("Disabled\n");
        }
        diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_CFM_ETH_DM_PORT_ALL_STATE_DISABLE_ENABLE
/* oam set cfm eth-dm port ( <PORT_LIST:port> | all ) state ( disable | enable ) */
cparser_result_t
cparser_cmd_oam_set_cfm_eth_dm_port_port_all_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    rtk_enable_t    enable;
    diag_portlist_t portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    if('e' == TOKEN_CHAR(7, 0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        ret = rtk_oam_cfmPortEthDmEnable_set(unit, port, enable);
        if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_GET_CFM_ETH_DM_RX_TIMESTAMP_INDEX_INDEX
/*
 * oam get cfm eth-dm rx-timestamp index <UINT:index>
 */
cparser_result_t
cparser_cmd_oam_get_cfm_eth_dm_rx_timestamp_index_index(
    cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_time_timeStamp_t timeStamp;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_oam_cfmEthDmRxTimestamp_get(unit, *index_ptr, &timeStamp), ret);
    diag_util_mprintf("\tETH-DM Rx Timestamp [%u]: %10u.%09u\n", *index_ptr, timeStamp.sec, timeStamp.nsec);

    return CPARSER_OK;
}   /* end of cparser_cmd_oam_get_cfm_eth_dm_rx_timestamp_index_index */
#endif

#ifdef CMD_OAM_GET_DYING_GASP_PACKET_COUNT
/*
 * oam get dying-gasp packet-count
 */
cparser_result_t
cparser_cmd_oam_get_dying_gasp_packet_count(
    cparser_context_t *context)
{
    uint32  unit;
    uint32  cnt;
    int32   ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_oam_dyingGaspPktCnt_get(unit, &cnt);
    if (ret != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("The dying gasp packet count: %d", cnt);

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_DYING_GASP_PACKET_COUNT_COUNT
/*
 * oam set dying-gasp packet-count <UINT:count>
 */
cparser_result_t
cparser_cmd_oam_set_dying_gasp_packet_count_count(
    cparser_context_t *context,
    uint32_t *count_ptr)
{
    uint32  unit;
    int32   ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    ret = rtk_oam_dyingGaspPktCnt_set(unit, *count_ptr);
    if (ret != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_OAM_SET_LINK_FAULT_MONITOR_STATE_DISABLE_ENABLE
/* oam set link-fault monitor state ( disable | enable ) */
cparser_result_t
cparser_cmd_oam_set_link_fault_monitor_state_disable_enable(cparser_context_t *context)
{
    uint32          unit;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    enable;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    if('e' == TOKEN_CHAR(5, 0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    ret = rtk_oam_linkFaultMonEnable_set(enable);
    if (ret != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif
