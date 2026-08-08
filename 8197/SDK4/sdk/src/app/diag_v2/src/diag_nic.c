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
 * $Revision: 27701 $
 * $Date: 2012-04-02 11:10:51 +0800 (Mon, 02 Apr 2012) $
 *
 * Purpose : Definition those NIC command and APIs in the SDK diagnostic shell.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) NIC
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
#include <drv/nic/nic.h>
#include <drv/nic/diag.h>
#include <diag_util.h>
#include <diag_om.h>
#include <parser/cparser_priv.h>

#define TRACE_RX    (1)
#define TRACE_TX    (2)

#define START_OF_TX_RING   (0)
#define END_OF_TX_RING     (1)

#define START_OF_RX_RING   (0)
#define END_OF_RX_RING     (7)

#ifdef CMD_NIC_RESET_DUMP_COUNTER
/*
 * nic ( reset | dump ) counter
 */
cparser_result_t cparser_cmd_nic_reset_dump_counter(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('r' == TOKEN_CHAR(1,0))
    {
        DIAG_UTIL_ERR_CHK(drv_nic_cntr_clear(unit), ret);
    }
    else if ('d' == TOKEN_CHAR(1,0))
    {
        DIAG_UTIL_ERR_CHK(drv_nic_cntr_dump(unit), ret);
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
} /* end of cparser_cmd_nic_reset_dump_counter */
#endif

#ifdef CMD_NIC_DUMP_BUFFER_USAGE
/*
 * nic dump buffer-usage
 */
cparser_result_t cparser_cmd_nic_dump_buffer_usage(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(drv_nic_ringbuf_dump(unit), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_nic_dump_buffer_usage */
#endif

#ifdef CMD_NIC_DUMP_PKTHDR_MBUF_RAW_DATA
/*
 * nic dump pkthdr-mbuf { raw-data }
 */
cparser_result_t cparser_cmd_nic_dump_pkthdr_mbuf_raw_data(cparser_context_t *context)
{
    uint32      unit = 0;
    uint32      flag_rawdata = FALSE;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (3 == TOKEN_NUM)
    {
        flag_rawdata = FALSE;
    }
    else if ('r' == context->parser->tokens[3].buf[0])
    {
        flag_rawdata = TRUE;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(drv_nic_pkthdr_mbuf_dump(unit, NIC_PKTHDR_MBUF_MODE_RX, START_OF_RX_RING, END_OF_RX_RING, flag_rawdata), ret);
    DIAG_UTIL_ERR_CHK(drv_nic_pkthdr_mbuf_dump(unit, NIC_PKTHDR_MBUF_MODE_TX, START_OF_TX_RING, END_OF_TX_RING, flag_rawdata), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_nic_dump_pkthdr_mbuf_raw_data */
#endif

#ifdef CMD_NIC_DUMP_PKTHDR_MBUF_TX_RING_IDX_RAW_DATA
/*
 * nic dump pkthdr-mbuf tx { <UINT:ring_idx> } { raw-data }
 */
cparser_result_t cparser_cmd_nic_dump_pkthdr_mbuf_tx_ring_idx_raw_data(cparser_context_t *context,
    uint32_t *ring_idx_ptr)
{
    uint32      unit = 0;
    uint32      flag_rawdata = FALSE;
    uint32      start = 0;
    uint32      end = 0;
    int32       ret = RT_ERR_FAILED;

    /* Don't check the (NULL == ring_idx_ptr) due to it is optional token */
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (6 == TOKEN_NUM)
    {
        DIAG_UTIL_ERR_CHK(diag_util_str2ul(&start, TOKEN_STR(4)), ret);
        end = start;
        flag_rawdata = TRUE;
    }
    else if (5 == TOKEN_NUM)
    {
        if ('r' == context->parser->tokens[4].buf[0])
        {
            start = START_OF_TX_RING;
            end = END_OF_TX_RING;
            flag_rawdata = TRUE;
        }
        else
        {
            DIAG_UTIL_ERR_CHK(diag_util_str2ul(&start, TOKEN_STR(4)), ret);
            end = start;
            flag_rawdata = FALSE;
        }
    }
    else
    {
        start = START_OF_TX_RING;
        end = END_OF_TX_RING;
        flag_rawdata = FALSE;
    }

    if (end > END_OF_TX_RING)
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(drv_nic_pkthdr_mbuf_dump(unit, NIC_PKTHDR_MBUF_MODE_TX, start, end, flag_rawdata), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_nic_dump_pkthdr_mbuf_tx_ring_idx_raw_data */
#endif

#ifdef CMD_NIC_DUMP_PKTHDR_MBUF_RX_RING_IDX_RAW_DATA
/*
 * nic dump pkthdr-mbuf rx { <UINT:ring_idx> } { raw-data }
 */
cparser_result_t cparser_cmd_nic_dump_pkthdr_mbuf_rx_ring_idx_raw_data(cparser_context_t *context,
    uint32_t *ring_idx_ptr)
{
    uint32      unit = 0;
    uint32      flag_rawdata = FALSE;
    uint32      start = 0;
    uint32      end = 0;
    int32       ret = RT_ERR_FAILED;

    /* Don't check the (NULL == ring_idx_ptr) due to it is optional token */
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (6 == TOKEN_NUM)
    {
        DIAG_UTIL_ERR_CHK(diag_util_str2ul(&start, TOKEN_STR(4)), ret);
        end = start;
        flag_rawdata = TRUE;
    }
    else if (5 == TOKEN_NUM)
    {
        if ('r' == context->parser->tokens[4].buf[0])
        {
            start = START_OF_RX_RING;
            end = END_OF_RX_RING;
            flag_rawdata = TRUE;
        }
        else
        {
            DIAG_UTIL_ERR_CHK(diag_util_str2ul(&start, TOKEN_STR(4)), ret);
            end = start;
            flag_rawdata = FALSE;
        }
    }
    else
    {
        start = START_OF_RX_RING;
        end = END_OF_RX_RING;
        flag_rawdata = FALSE;
    }

    if (end > END_OF_RX_RING)
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(drv_nic_pkthdr_mbuf_dump(unit, NIC_PKTHDR_MBUF_MODE_RX, start, end, flag_rawdata), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_nic_dump_pkthdr_mbuf_rx_ring_idx_raw_data */
#endif

#ifdef CMD_NIC_SET_RX_STATE_DISABLE_ENABLE
/*
 * nic set rx state ( disable | enable )
 */
cparser_result_t cparser_cmd_nic_set_rx_state_disable_enable(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('e' == context->parser->tokens[4].buf[0])
    {
        DIAG_UTIL_ERR_CHK(drv_nic_rx_start(unit), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(drv_nic_rx_stop(unit), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_nic_set_rx_state_disable_enable */
#endif

#ifdef CMD_NIC_SET_RX_TX_TRACE_START_RAW_DATA_CPU_TAG
/*
 * nic set ( rx | tx ) trace start { raw-data } { cpu-tag }
 */
cparser_result_t cparser_cmd_nic_set_rx_tx_trace_start_raw_data_cpu_tag(cparser_context_t *context)
{
    uint32      unit = 0;
    uint32      flags = 0;
    uint32      trace = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('r' == TOKEN_CHAR(2,0))
    {
        trace = TRACE_RX;
    }
    else if ('t' == TOKEN_CHAR(2,0))
    {
        trace = TRACE_TX;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(drv_nic_dbg_get(unit, &flags), ret);
    if (TRACE_RX == trace)
    {
        if (7 == TOKEN_NUM)
        {
            flags = flags | DEBUG_RX_RAW_LEN_BIT | DEBUG_RX_CPU_TAG_BIT;
        }
        else if (6 == TOKEN_NUM)
        {
            if ('r' == TOKEN_CHAR(5,0))
                flags = flags | DEBUG_RX_RAW_LEN_BIT;
            if ('c' == TOKEN_CHAR(5,0))
                flags = flags | DEBUG_RX_CPU_TAG_BIT;
        }
    }
    else if (TRACE_TX == trace)
    {
        if (7 == TOKEN_NUM)
        {
            flags = flags | DEBUG_TX_RAW_LEN_BIT | DEBUG_TX_CPU_TAG_BIT;
        }
        else if (6 == TOKEN_NUM)
        {
            if ('r' == TOKEN_CHAR(5,0))
                flags = flags | DEBUG_TX_RAW_LEN_BIT;
            if ('c' == TOKEN_CHAR(5,0))
                flags = flags | DEBUG_TX_CPU_TAG_BIT;
        }
    }

    DIAG_UTIL_ERR_CHK(drv_nic_dbg_set(unit, flags), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_nic_set_rx_tx_trace_start_raw_data_cpu_tag */
#endif

#ifdef CMD_NIC_SET_RX_TX_TRACE_STOP
/*
 * nic set ( rx | tx ) trace stop
 */
cparser_result_t cparser_cmd_nic_set_rx_tx_trace_stop(cparser_context_t *context)
{
    uint32      unit = 0;
    uint32      flags = 0;
    uint32      trace = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('r' == TOKEN_CHAR(2,0))
    {
        trace = TRACE_RX;
    }
    else if ('t' == TOKEN_CHAR(2,0))
    {
        trace = TRACE_TX;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(drv_nic_dbg_get(unit, &flags), ret);
    if (TRACE_RX == trace)
    {
        flags = flags & (~ DEBUG_RX_RAW_LEN_BIT);
        flags = flags & (~ DEBUG_RX_CPU_TAG_BIT);
    }
    else if (TRACE_TX == trace)
    {
        flags = flags & (~ DEBUG_TX_RAW_LEN_BIT);
        flags = flags & (~ DEBUG_TX_CPU_TAG_BIT);
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(drv_nic_dbg_set(unit, flags), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_nic_set_rx_tx_trace_stop */
#endif

#ifdef CMD_NIC_GET
/*
 * nic get
 */
cparser_result_t cparser_cmd_nic_get(cparser_context_t *context)
{
    uint32      unit = 0;
    uint32      flags = 0, rx_status = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(drv_nic_dbg_get(unit, &flags), ret);
    DIAG_UTIL_ERR_CHK(drv_nic_rx_status_get(unit, &rx_status), ret);

    if (rx_status)
    {
        diag_util_mprintf("Rx status : Enabled\n");
    }
    else
    {
        diag_util_mprintf("Rx status : Disabled\n");
    }

    diag_util_mprintf("Rx debug flags:\n");
    diag_util_printf("    +raw-data : ");
    if (flags & DEBUG_RX_RAW_LEN_BIT)
    {
        diag_util_mprintf("Enabled\n");
    }
    else
    {
        diag_util_mprintf("Disabled\n");
    }
    diag_util_printf("    +cpu-tag : ");
    if (flags & DEBUG_RX_CPU_TAG_BIT)
    {
        diag_util_mprintf("Enabled\n");
    }
    else
    {
        diag_util_mprintf("Disabled\n");
    }

    diag_util_mprintf("Tx debug flags:\n");
    diag_util_printf("    +raw-data : ");
    if (flags & DEBUG_TX_RAW_LEN_BIT)
    {
        diag_util_mprintf("Enabled\n");
    }
    else
    {
        diag_util_mprintf("Disabled\n");
    }
    diag_util_printf("    +cpu-tag : ");
    if (flags & DEBUG_TX_CPU_TAG_BIT)
    {
        diag_util_mprintf("Enabled\n");
    }
    else
    {
        diag_util_mprintf("Disabled\n");
    }

    return CPARSER_OK;
} /* end of cparser_cmd_nic_get */
#endif

#ifdef CMD_NIC_GET_RX_TX
/*
 * nic get ( rx | tx )
 */
cparser_result_t cparser_cmd_nic_get_rx_tx(cparser_context_t *context)
{
    uint32      unit = 0;
    uint32      flags = 0, rx_status = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(drv_nic_dbg_get(unit, &flags), ret);

    if ('r' == TOKEN_CHAR(2,0))
    {
        DIAG_UTIL_ERR_CHK(drv_nic_rx_status_get(unit, &rx_status), ret);
        if (rx_status)
        {
            diag_util_mprintf("Rx status : Enabled\n");
        }
        else
        {
            diag_util_mprintf("Rx status : Disabled\n");
        }

        diag_util_mprintf("Rx debug flags:\n");
        diag_util_printf("    +raw-data : ");
        if (flags & DEBUG_RX_RAW_LEN_BIT)
        {
            diag_util_mprintf("Enabled\n");
        }
        else
        {
            diag_util_mprintf("Disabled\n");
        }
        diag_util_printf("    +cpu-tag : ");
        if (flags & DEBUG_RX_CPU_TAG_BIT)
        {
            diag_util_mprintf("Enabled\n");
        }
        else
        {
            diag_util_mprintf("Disabled\n");
        }
    }

    if ('t' == TOKEN_CHAR(2,0))
    {
        diag_util_mprintf("Tx debug flags:\n");
        diag_util_printf("    +raw-data : ");
        if (flags & DEBUG_TX_RAW_LEN_BIT)
        {
            diag_util_mprintf("Enabled\n");
        }
        else
        {
            diag_util_mprintf("Disabled\n");
        }
        diag_util_printf("    +cpu-tag : ");
        if (flags & DEBUG_TX_CPU_TAG_BIT)
        {
            diag_util_mprintf("Enabled\n");
        }
        else
        {
            diag_util_mprintf("Disabled\n");
        }
    }

    return CPARSER_OK;
} /* end of cparser_cmd_nic_get_rx_tx */
#endif

#if defined(CONFIG_SDK_SOFTWARE_RX_CPU_TAG)
#ifdef CMD_NIC_SET_SOFTWARE_DATABASE_ENTRY_INDEX_DPM_VID_ETHERTYPE_PROTOCOL_ID_L4_DST_PORT_DATA_MASK
/*
 * nic set software-database <UINT:entry_index> ( dpm | vid | ethertype | protocol-id | l4-dst-port ) <UINT:data> <UINT:mask>
 */
cparser_result_t cparser_cmd_nic_set_software_database_entry_index_dpm_vid_ethertype_protocol_id_l4_dst_port_data_mask(cparser_context_t *context,
    uint32_t *entry_index_ptr,
    uint32_t *data_ptr,
    uint32_t *mask_ptr)
{
    uint32  unit;
    uint32  index = *entry_index_ptr;
    int32   ret = RT_ERR_FAILED;
    drv_nic_CpuEntry_t entry;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    osal_memset(&entry, 0, sizeof(drv_nic_CpuEntry_t));
    DIAG_UTIL_ERR_CHK(drv_nic_pieCpuEntry_get(unit, index, &entry), ret);

    switch (TOKEN_CHAR(4,0))
    {
        case 'd': /* dpm */
            entry.dpm = *data_ptr;
            entry.care_dpm = *mask_ptr;
            break;
        case 'v': /* vid */
            entry.vid = *data_ptr;
            entry.care_vid = *mask_ptr;
            break;
        case 'e': /* ethertype */
            entry.ethertype = *data_ptr;
            entry.care_ethertype = *mask_ptr;
            break;
        case 'p': /* protocol_id */
            entry.protocol_id = *data_ptr;
            entry.care_protocol_id = *mask_ptr;
            break;
        case 'l': /* l4_dst_port */
            entry.l4_dst_port = *data_ptr;
            entry.care_l4_dst_port = *mask_ptr;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(drv_nic_pieCpuEntry_set(unit, index, &entry), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_NIC_SET_SOFTWARE_DATABASE_ENTRY_INDEX_DMAC_MAC_CARE_MAC
/*
 * nic set software-database <UINT:entry_index> dmac <MACADDR:mac> <MACADDR:care_mac>
 */
cparser_result_t cparser_cmd_nic_set_software_database_entry_index_dmac_mac_care_mac(cparser_context_t *context,
    uint32_t *entry_index_ptr,
    cparser_macaddr_t *mac_ptr,
    cparser_macaddr_t *care_mac_ptr)
{
    uint32  unit;
    uint32  index = *entry_index_ptr;
    int32   ret = RT_ERR_FAILED;
    drv_nic_CpuEntry_t entry;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    osal_memset(&entry, 0, sizeof(drv_nic_CpuEntry_t));
    DIAG_UTIL_ERR_CHK(drv_nic_pieCpuEntry_get(unit, index, &entry), ret);
    memcpy(&entry.dmac[0], &mac_ptr->octet[0], sizeof(cparser_macaddr_t));
    memcpy(&entry.care_dmac[0], &care_mac_ptr->octet[0], sizeof(cparser_macaddr_t));
    DIAG_UTIL_ERR_CHK(drv_nic_pieCpuEntry_set(unit, index, &entry), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_NIC_GET_SOFTWARE_DATABASE_ENTRY_INDEX
/*
 * nic get software-database <UINT:entry_index>
 */
cparser_result_t cparser_cmd_nic_get_software_database_entry_index(cparser_context_t *context,
    uint32_t *entry_index_ptr)
{
    uint32  unit;
    uint32  index = *entry_index_ptr;
    int32   ret = RT_ERR_FAILED;
    drv_nic_CpuEntry_t entry;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    osal_memset(&entry, 0, sizeof(drv_nic_CpuEntry_t));
    DIAG_UTIL_ERR_CHK(drv_nic_pieCpuEntry_get(unit, index, &entry), ret);

    diag_util_mprintf("ACL entry index: 0x%x (%d)\n", index, index);
    diag_util_mprintf("  DPM: 0x%x, carebits: 0x%x\n", entry.dpm, entry.care_dpm);
    diag_util_mprintf("  VID: 0x%x, carebits: 0x%x\n", entry.vid, entry.care_vid);
    diag_util_mprintf("  ETHERTYPE: 0x%x, carebits: 0x%x\n", entry.ethertype, entry.care_ethertype);
    diag_util_mprintf("  PROTOCOL_ID: 0x%x, carebits: 0x%x\n", entry.protocol_id, entry.care_protocol_id);
    diag_util_mprintf("  DMAC: %02x:%02x:%02x:%02x:%02x:%02x, carebits: %02x:%02x:%02x:%02x:%02x:%02x\n", entry.dmac[0], entry.dmac[1], entry.dmac[2], entry.dmac[3], entry.dmac[4], entry.dmac[5], 
                      entry.care_dmac[0], entry.care_dmac[1], entry.care_dmac[2], entry.care_dmac[3], entry.care_dmac[4], entry.care_dmac[5]);

    return CPARSER_OK;
}
#endif

#ifdef CMD_NIC_DEL_SOFTWARE_DATABASE_ENTRY_INDEX
/*
 * nic del software-database <UINT:entry_index>
 */
cparser_result_t cparser_cmd_nic_del_software_database_entry_index(cparser_context_t *context,
    uint32_t *entry_index_ptr)
{
    uint32  unit;
    uint32  index = *entry_index_ptr;
    int32   ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(drv_nic_pieCpuEntry_del(unit, index), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_NIC_ADD_SOFTWARE_DATABASE_ENTRY_INDEX
/*
 * nic add software-database <UINT:entry_index>
 */
cparser_result_t cparser_cmd_nic_add_software_database_entry_index(cparser_context_t *context,
    uint32_t *entry_index_ptr)
{
    uint32  unit;
    uint32  index = *entry_index_ptr;
    int32   ret = RT_ERR_FAILED;
    drv_nic_CpuEntry_t entry;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    osal_memset(&entry, 0, sizeof(drv_nic_CpuEntry_t));
    DIAG_UTIL_ERR_CHK(drv_nic_pieCpuEntry_add(unit, index, &entry), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_NIC_DUMP_SOFTWARE_DATABASE
/*
 * nic dump software-database
 */
cparser_result_t cparser_cmd_nic_dump_software_database(cparser_context_t *context)
{
    uint32  unit;
    uint32  i;
    int32   ret = RT_ERR_FAILED;
    drv_nic_CpuEntry_t entry;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();


    for (i = 0; i < 2048; i++)
    {
        osal_memset(&entry, 0, sizeof(drv_nic_CpuEntry_t));
        if ((ret = drv_nic_pieCpuEntry_get(unit, i, &entry)) != RT_ERR_OK)
            continue;

        diag_util_mprintf("ACL entry index: 0x%x (%d)\n", i, i);
        diag_util_mprintf("  DPM: 0x%x, carebits: 0x%x\n", entry.dpm, entry.care_dpm);
        diag_util_mprintf("  VID: 0x%x, carebits: 0x%x\n", entry.vid, entry.care_vid);
        diag_util_mprintf("  ETHERTYPE: 0x%x, carebits: 0x%x\n", entry.ethertype, entry.care_ethertype);
        diag_util_mprintf("  PROTOCOL_ID: 0x%x, carebits: 0x%x\n", entry.protocol_id, entry.care_protocol_id);
        diag_util_mprintf("  L4_DST_PORT: 0x%x, carebits: 0x%x\n", entry.l4_dst_port, entry.care_l4_dst_port);
        diag_util_mprintf("  DMAC: %02x:%02x:%02x:%02x:%02x:%02x, carebits: %02x:%02x:%02x:%02x:%02x:%02x\n", entry.dmac[0], entry.dmac[1], entry.dmac[2], entry.dmac[3], entry.dmac[4], entry.dmac[5], 
                          entry.care_dmac[0], entry.care_dmac[1], entry.care_dmac[2], entry.care_dmac[3], entry.care_dmac[4], entry.care_dmac[5]);
    }

    return CPARSER_OK;
}
#endif
#endif
