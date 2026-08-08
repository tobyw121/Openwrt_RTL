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
 * $Revision: 42820 $
 * $Date: 2013-09-13 16:30:51 +0800 (Fri, 13 Sep 2013) $
 *
 * Purpose : Definition those public uart APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) uart probe
 * 
 */ 
#ifndef __DRV_UART_PROBE_H__
#define __DRV_UART_PROBE_H__

/*  
 * Include Files 
 */
#include <common/rt_autoconf.h>
#include <drv/uart/uart.h>

/* 
 * Symbol Definition 
 */
typedef enum uart_controller_e
{
#if defined(CONFIG_SDK_RTL8390)
    UART_R8390,
#endif
#if defined(CONFIG_SDK_RTL8380)
    UART_R8380,
#endif
    UART_CTRL_END            
} uart_controller_t;

typedef struct uart_mapper_operation_s
{
    /* UART */
    int32   (*init)(uint32,drv_uart_rxcallback_t,drv_uart_txcallback_t);
    int32   (*tstc)(uint32);
    int32   (*poll_getc)(uint32, uint8 *);
    int32   (*poll_putc)(uint32, uint8);
    int32   (*baudrate_get)(uint32, drv_uart_baudrate_t *);
    int32   (*baudrate_set)(uint32, drv_uart_baudrate_t);
    void   (*starttx)(uint32);
    void   (*clearfifo)(uint32);
} uart_mapper_operation_t;

#define UART_CTRL(unit)      uart_ops[uart_if[unit]]

/* 
 * Data Declaration 
 */
extern uart_mapper_operation_t uart_ops[UART_CTRL_END];
extern uint32 uart_if[RTK_MAX_NUM_OF_UNIT];

/*
 * Macro Definition
 */

/*
 * Function Declaration
 */

/* Function Name:
 *      uart_probe
 * Description:
 *      Probe uart module of the specified device.
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
uart_probe(uint32 unit);

#endif /* __DRV_UART_PROBE_H__ */
