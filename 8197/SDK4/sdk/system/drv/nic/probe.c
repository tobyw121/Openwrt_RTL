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
 * Purpose : Definition those public NIC(Network Interface Controller) APIs and
 *           its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) CPU tag
 *           2) NIC tx
 *           3) NIC rx
 *
 */

/*
 * Include Files
 */
#include <drv/swcore/chip.h>
#include <drv/nic/probe.h>

#if (defined(CONFIG_SDK_DRIVER_NIC_R8328) || defined(CONFIG_SDK_DRIVER_NIC_R8328_MODULE))
    #include <drv/nic/r8328.h>
#endif

#if (defined(CONFIG_SDK_DRIVER_NIC_R8389) || defined(CONFIG_SDK_DRIVER_NIC_R8389_MODULE))
    #include <drv/nic/r8389.h>
#endif

#if (defined(CONFIG_SDK_DRIVER_NIC_R8390) || defined(CONFIG_SDK_DRIVER_NIC_R8390_MODULE))
    #include <drv/nic/r8390.h>
#endif

#if (defined(CONFIG_SDK_DRIVER_NIC_R8380) || defined(CONFIG_SDK_DRIVER_NIC_R8380_MODULE))
    #include <drv/nic/r8380.h>
#endif


/*
 * Symbol Definition
 */
#define NIC_DB_SIZE     (sizeof(nic_db)/sizeof(cid_group_t))

/*
 * Data Declaration
 */
const static cid_group_t nic_db[] =
{
#if (defined(CONFIG_SDK_DRIVER_NIC_R8389) || defined(CONFIG_SDK_DRIVER_NIC_R8389_MODULE))
    {RTL8389M_CHIP_ID, NIC_R8389},
    {RTL8329M_CHIP_ID, NIC_R8389},
    {RTL8377M_CHIP_ID, NIC_R8389},
    {RTL8389L_CHIP_ID, NIC_R8389},
#endif
#if (defined(CONFIG_SDK_DRIVER_NIC_R8328) || defined(CONFIG_SDK_DRIVER_NIC_R8328_MODULE))
    {RTL8328M_CHIP_ID, NIC_R8328},
    {RTL8328S_CHIP_ID, NIC_R8328},
    {RTL8328L_CHIP_ID, NIC_R8328},
#endif
#if (defined(CONFIG_SDK_DRIVER_NIC_R8390) || defined(CONFIG_SDK_DRIVER_NIC_R8390_MODULE))
    {RTL8352M_CHIP_ID, NIC_R8390},
    {RTL8353M_CHIP_ID, NIC_R8390},
    {RTL8391M_CHIP_ID, NIC_R8390},
    {RTL8392M_CHIP_ID, NIC_R8390},
    {RTL8393M_CHIP_ID, NIC_R8390},
    {RTL8396M_CHIP_ID, NIC_R8390},
    {RTL8352MES_CHIP_ID, NIC_R8390},
    {RTL8353MES_CHIP_ID, NIC_R8390},
    {RTL8392MES_CHIP_ID, NIC_R8390},
    {RTL8393MES_CHIP_ID, NIC_R8390},
    {RTL8396MES_CHIP_ID, NIC_R8390},
#endif
#if (defined(CONFIG_SDK_DRIVER_NIC_R8380) || defined(CONFIG_SDK_DRIVER_NIC_R8380_MODULE))
    {RTL8380M_CHIP_ID, NIC_R8380},
    {RTL8330M_CHIP_ID, NIC_R8380},
    {RTL8382M_CHIP_ID, NIC_R8380},
    {RTL8332M_CHIP_ID, NIC_R8380},
    {RTL8380MES_CHIP_ID, NIC_R8380},
    {RTL8330MES_CHIP_ID, NIC_R8380},
    {RTL8382MES_CHIP_ID, NIC_R8380},
    {RTL8332MES_CHIP_ID, NIC_R8380},
#endif
};

nic_mapper_operation_t nic_ops[NIC_CTRL_END] =
{
#if (defined(CONFIG_SDK_DRIVER_NIC_R8389) || defined(CONFIG_SDK_DRIVER_NIC_R8389_MODULE))
    {   /* NIC_R8389 */
        .init = r8389_init,
        .pkt_tx = r8389_pkt_tx,
        .rx_start = r8389_rx_start,
        .rx_stop = r8389_rx_stop,
        .rx_register = r8389_rx_register,
        .rx_unregister = r8389_rx_unregister,
        .pkt_alloc = r8389_pkt_alloc,
        .pkt_free = r8389_pkt_free,
#if defined(CONFIG_SDK_DRIVER_NIC_KERNEL_MODE)
        .dbg_set = r8389_debug_set,
        .dbg_get = r8389_debug_get,
        .cntr_dump = r8389_counter_dump,
        .cntr_clear = r8389_counter_clear,
        .ringbuf_dump = r8389_bufStatus_dump,
        .pkthdr_mbuf_dump = r8389_pkthdrMbuf_dump,
        .rx_status_get = r8389_rxStatus_get
#else
        .dbg_set = r8389_debug_set,
        .dbg_get = r8389_debug_get,
        .cntr_dump = r8389_counter_dump,
        .cntr_clear = r8389_counter_clear,
        .ringbuf_dump = r8389_bufStatus_dump,
        .pkthdr_mbuf_dump = r8389_pkthdrMbuf_dump,
        .rx_status_get = r8389_rxStatus_get,
        .nic_reset = NULL
#endif
    },
#endif
#if (defined(CONFIG_SDK_DRIVER_NIC_R8328) || defined(CONFIG_SDK_DRIVER_NIC_R8328_MODULE))
    {   /* NIC_R8328 */
        .init = r8328_init,
        .pkt_tx = r8328_pkt_tx,
        .rx_start = r8328_rx_start,
        .rx_stop = r8328_rx_stop,
        .rx_register = r8328_rx_register,
        .rx_unregister = r8328_rx_unregister,
        .pkt_alloc = r8328_pkt_alloc,
        .pkt_free = r8328_pkt_free,
#if defined(CONFIG_SDK_SOFTWARE_RX_CPU_TAG)
        .pieCpuEntry_add = r8328_pieCpuEntry_add,
        .pieCpuEntry_del = r8328_pieCpuEntry_del,
        .pieCpuEntry_get = r8328_pieCpuEntry_get,
        .pieCpuEntry_set = r8328_pieCpuEntry_set,
#endif
        .dbg_set = r8328_debug_set,
        .dbg_get = r8328_debug_get,
        .cntr_dump = r8328_counter_dump,
        .cntr_clear = r8328_counter_clear,
        .ringbuf_dump = r8328_bufStatus_dump,
        .pkthdr_mbuf_dump = r8328_pkthdrMbuf_dump,
        .rx_status_get = r8328_rxStatus_get,
        .nic_reset = NULL
    },
#endif
#if (defined(CONFIG_SDK_DRIVER_NIC_R8390) || defined(CONFIG_SDK_DRIVER_NIC_R8390_MODULE))
    {   /* NIC_R8390 */
        .init = r8390_init,
        .pkt_tx = r8390_pkt_tx,
        .rx_start = r8390_rx_start,
        .rx_stop = r8390_rx_stop,
        .rx_register = r8390_rx_register,
        .rx_unregister = r8390_rx_unregister,
        .pkt_alloc = r8390_pkt_alloc,
        .pkt_free = r8390_pkt_free,
        .dbg_set = r8390_debug_set,
        .dbg_get = r8390_debug_get,
        .cntr_dump = r8390_counter_dump,
        .cntr_clear = r8390_counter_clear,
        .ringbuf_dump = r8390_bufStatus_dump,
        .pkthdr_mbuf_dump = r8390_pkthdrMbuf_dump,
        .rx_status_get = r8390_rxStatus_get,
        .nic_reset = NULL
    },
#endif
#if (defined(CONFIG_SDK_DRIVER_NIC_R8380) || defined(CONFIG_SDK_DRIVER_NIC_R8380_MODULE))
    {   /* NIC_R8380 */
        .init = r8380_init,
        .pkt_tx = r8380_pkt_tx,
        .rx_start = r8380_rx_start,
        .rx_stop = r8380_rx_stop,
        .rx_register = r8380_rx_register,
        .rx_unregister = r8380_rx_unregister,
        .pkt_alloc = r8380_pkt_alloc,
        .pkt_free = r8380_pkt_free,
        .dbg_set = r8380_debug_set,
        .dbg_get = r8380_debug_get,
        .cntr_dump = r8380_counter_dump,
        .cntr_clear = r8380_counter_clear,
        .ringbuf_dump = r8380_bufStatus_dump,
        .pkthdr_mbuf_dump = r8380_pkthdrMbuf_dump,
        .rx_status_get = r8380_rxStatus_get,
        .nic_reset = r8380_nic_reset
    },
#endif
};

uint32 nic_if[RTK_MAX_NUM_OF_UNIT];
uint32 nic_chipId[RTK_MAX_NUM_OF_UNIT];

/*
 * Function Declaration
 */

/* Function Name:
 *      nic_probe
 * Description:
 *      Probe nic module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
nic_probe(uint32 unit)
{
    uint32 i;

    for (i=0; i<NIC_DB_SIZE; i++)
    {
        if(!drv_swcore_cid_cmp(unit, nic_db[i].cid))
        {
            nic_if[unit] = nic_db[i].gid;
            nic_chipId[unit] = nic_db[i].cid;
            return RT_ERR_OK;
        }
    }

    return RT_ERR_FAILED;

} /* end of nic_probe */

