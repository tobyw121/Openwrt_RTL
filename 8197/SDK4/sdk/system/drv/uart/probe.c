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
 * Purpose : Definition those public uart APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) uart probe
 *
 */

/*
 * Include Files
 */
#include <common/rt_autoconf.h>
#include <drv/swcore/chip.h>
#include <drv/uart/probe.h>
#if defined(CONFIG_SDK_RTL8390)
#include <drv/uart/r8390.h>
#endif
#if defined(CONFIG_SDK_RTL8380)
#include <drv/uart/r8380.h>
#endif

/*
 * Symbol Definition
 */
#define UART_DB_SIZE (sizeof(uart_db)/sizeof(cid_group_t))

/*
 * Data Declaration
 */
const static cid_group_t uart_db[] =
{
#if defined(CONFIG_SDK_RTL8390)
    /* RTL8390 series chips */
    {RTL8352M_CHIP_ID, UART_R8390},
    {RTL8353M_CHIP_ID, UART_R8390},
    {RTL8391M_CHIP_ID, UART_R8390},
    {RTL8392M_CHIP_ID, UART_R8390},
    {RTL8393M_CHIP_ID, UART_R8390},
    {RTL8396M_CHIP_ID, UART_R8390},
    {RTL8352MES_CHIP_ID, UART_R8390},
    {RTL8353MES_CHIP_ID, UART_R8390},
    {RTL8392MES_CHIP_ID, UART_R8390},
    {RTL8393MES_CHIP_ID, UART_R8390},
    {RTL8396MES_CHIP_ID, UART_R8390},
#endif
#if defined(CONFIG_SDK_RTL8380)
    /* RTL8380 series chips */
    {RTL8380M_CHIP_ID, UART_R8380},
    {RTL8330M_CHIP_ID, UART_R8380},
    {RTL8382M_CHIP_ID, UART_R8380},
    {RTL8332M_CHIP_ID, UART_R8380},
    {RTL8380MES_CHIP_ID, UART_R8380},
    {RTL8330MES_CHIP_ID, UART_R8380},
    {RTL8382MES_CHIP_ID, UART_R8380},
    {RTL8332MES_CHIP_ID, UART_R8380},
#endif
};

uart_mapper_operation_t uart_ops[UART_CTRL_END] =
{
#if defined(CONFIG_SDK_RTL8390)
    {   /* UART_R8390 */
        .init = r8390_uart_init,
        .tstc = r8390_uart_tstc,
        .poll_getc = r8390_uart_getc,
        .poll_putc = r8390_uart_putc,
        .baudrate_get = r8390_uart_baudrate_get,
        .baudrate_set = r8390_uart_baudrate_set,
        .starttx = r8390_serial_starttx,
        .clearfifo = r8390_serial_clearfifo,
    },
#endif
#if defined(CONFIG_SDK_RTL8380)
    {   /* UART_R8380 */
        .init = r8380_uart_init,
        .tstc = r8380_uart_tstc,
        .poll_getc = r8380_uart_getc,
        .poll_putc = r8380_uart_putc,
        .baudrate_get = r8380_uart_baudrate_get,
        .baudrate_set = r8380_uart_baudrate_set,
        .starttx = r8380_serial_starttx,
        .clearfifo = r8380_serial_clearfifo,
    },
#endif
};

uint32 uart_if[RTK_MAX_NUM_OF_UNIT];
uint32 uart_chipId[RTK_MAX_NUM_OF_UNIT];

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
int32
uart_probe(uint32 unit)
{
    uint32 i;

    for (i = 0; i < UART_DB_SIZE; i++)
    {
        if(!drv_swcore_cid_cmp(unit, uart_db[i].cid))
        {
            uart_if[unit] = uart_db[i].gid;
            uart_chipId[unit] = uart_db[i].cid;
			return RT_ERR_OK;
        }
    }

    return RT_ERR_FAILED;
} /* end of uart_probe */
