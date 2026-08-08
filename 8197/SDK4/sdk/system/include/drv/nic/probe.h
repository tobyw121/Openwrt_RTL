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
 * $Revision: 38707 $
 * $Date: 2013-04-18 19:53:07 +0800 (Thu, 18 Apr 2013) $
 *
 * Purpose : Mapper Layer is used to seperate different kind of software or hardware platform
 * 
 * Feature : Just dispatch information to Multiplex layer
 * 
 */ 
#ifndef __DRV_NIC_PROBE_H__
#define __DRV_NIC_PROBE_H__

/*  
 * Include Files 
 */
#include <common/rt_autoconf.h>
#include <drv/nic/common.h>

/* 
 * Symbol Definition 
 */
typedef enum nic_controller_e
{
#if (defined(CONFIG_SDK_DRIVER_NIC_R8389) || defined(CONFIG_SDK_DRIVER_NIC_R8389_MODULE))
    NIC_R8389,
#endif
#if (defined(CONFIG_SDK_DRIVER_NIC_R8328) || defined(CONFIG_SDK_DRIVER_NIC_R8328_MODULE))
    NIC_R8328,    
#endif    
#if (defined(CONFIG_SDK_DRIVER_NIC_R8390) || defined(CONFIG_SDK_DRIVER_NIC_R8390_MODULE))
    NIC_R8390,
#endif
#if (defined(CONFIG_SDK_DRIVER_NIC_R8380) || defined(CONFIG_SDK_DRIVER_NIC_R8380_MODULE))
    NIC_R8380,
#endif
    NIC_CTRL_END            
} nic_controller_t;

typedef struct nic_mapper_operation_s
{
    /* NIC */
    int32   (*init)(uint32, drv_nic_initCfg_t *);
    int32   (*pkt_tx)(uint32, drv_nic_pkt_t *, drv_nic_tx_cb_f, void *);
    int32   (*rx_start)(uint32);
    int32   (*rx_stop)(uint32);
    int32   (*rx_register)(uint32 , uint8, drv_nic_rx_cb_f, void *, uint32);
    int32   (*rx_unregister)(uint32 , uint8, drv_nic_rx_cb_f);
    int32   (*pkt_alloc)(uint32 , int32 , uint32 , drv_nic_pkt_t **);
    int32   (*pkt_free)(uint32 , drv_nic_pkt_t *);
#if defined(CONFIG_SDK_SOFTWARE_RX_CPU_TAG)
    int32   (*pieCpuEntry_add)(uint32 , uint32 , drv_nic_CpuEntry_t *);
    int32   (*pieCpuEntry_del)(uint32 , uint32 );
    int32   (*pieCpuEntry_get)(uint32 , uint32 , drv_nic_CpuEntry_t *);
    int32   (*pieCpuEntry_set)(uint32 , uint32 , drv_nic_CpuEntry_t *);
#endif

    /* DIAG */
    int32   (*dbg_set)(uint32, uint32);
    int32   (*dbg_get)(uint32, uint32 *);
    int32   (*cntr_dump)(uint32);
    int32   (*cntr_clear)(uint32);
    int32   (*ringbuf_dump)(uint32);
    int32   (*pkthdr_mbuf_dump)(uint32, uint32, uint32, uint32, uint32);
    int32   (*rx_status_get)(uint32, uint32 *);
    int32   (*nic_reset)(uint32);

} nic_mapper_operation_t;

#define NIC_CTRL(unit)      nic_ops[nic_if[unit]]

/* 
 * Data Declaration 
 */
extern nic_mapper_operation_t nic_ops[NIC_CTRL_END];
extern uint32 nic_if[RTK_MAX_NUM_OF_UNIT];

/*
 * Macro Definition
 */

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
extern int32
nic_probe(uint32 unit);

#endif /* __DRV_NIC_PROBE_H __ */

