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
 * $Revision: 42821 $
 * $Date: 2013-09-13 16:33:18 +0800 (Fri, 13 Sep 2013) $
 *
 * Purpose : Definition those public uart APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *            1) uart init
 *            2) uart set & get
 */
#ifndef __UART_R8380_H__ 
#define __UART_R8380_H__

/*
 * Include Files
 */
#include <drv/uart/uart.h>

/*
 * Symbol Definition
 */

/*
 * Data Type Definition
 */

/*
 * Data Declaration
 */

/*
 * Macro Definition
 */

/*
 * Function Declaration
 */

/* Function Name:
 *      r8380_uart_init
 * Description:
 *      Init the uart module of the specified device.
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
r8380_uart_init(uint32 unit,drv_uart_rxcallback_t rxcallback,drv_uart_txcallback_t txcallback);

/* Function Name:
 *      r8380_uart_tstc
 * Description:
 *      test if serial data in
 * Input:
 *      unit  - unit id 
 * Output:
 *      
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 */
extern int32
r8380_uart_tstc(uint32 unit);

/* Function Name:
 *      r8380_uart_getc
 * Description:
 *      Get the character from uart interface 
 * Input:
 *      unit    - unit id
 * Output:
 *      pData   - pointer buffer of character from uart interface
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
extern int32
r8380_uart_getc(uint32 unit, uint8 *pData);

/* Function Name:
 *      r8380_uart_putc
 * Description:
 *      Output the character to uart interface in the specified device
 * Input:
 *      unit - unit id
 * Output:
 *      data - output the character to uart interface
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
extern int32
r8380_uart_putc(uint32 unit, uint8 data);

/* Function Name:
 *      r8380_uart_baudrate_get
 * Description:
 *      Get the baudrate of the uart interface in the specified device
 * Input:
 *      unit      - unit id
 * Output:
 *      pBaudrate - pointer buffer of baudrate value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
extern int32
r8380_uart_baudrate_get(uint32 unit, drv_uart_baudrate_t *pBaudrate);

/* Function Name:
 *      r8380_uart_baudrate_set
 * Description:
 *      Configure the baudrate of the uart interface in the specified device
 * Input:
 *      unit     - unit id
 *      baudrate - baudrate value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
extern int32
r8380_uart_baudrate_set(uint32 unit, drv_uart_baudrate_t baudrate);

/* Function Name:
 *      r8380_serial_starttx
 * Description:
 *      trigger hw to start tx.
 * Input:
 *      unit  - unit id 
 * Output:
 *      
 * Return:
 * Note:
 */
extern void   
r8380_serial_starttx(uint32 unit);

/* Function Name:
 *      r8380_serial_clearfifo
 * Description:
 *      empty hw rx/tx fifo.
 * Input:
 *      unit  - unit id 
 * Output:
 *      
 * Return:
 * Note:
 *      for sync io, poe should call this function before exchange message
 */
extern void 
r8380_serial_clearfifo(uint32 unit);
#endif /* __UART_R8380_H__ */
