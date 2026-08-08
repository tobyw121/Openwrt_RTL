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
 * $Revision: 42822 $
 * $Date: 2013-09-13 16:34:23 +0800 (Fri, 13 Sep 2013) $
 *
 * Purpose : DRV APIs definition.
 *
 * Feature : Uart relative API
 *
 */

#ifndef __DRV_UART_H__
#define __DRV_UART_H__


/*
 * Include Files
 */
#include <common/error.h>

/*
 * Symbol Definition
 */
 
/*
 * Data Declaration
 */

typedef int32 (*drv_uart_txcallback_t)(uint32 unit, uint8 *inch);
typedef int32 (*drv_uart_rxcallback_t)(uint32 unit, uint8 data);

typedef enum drv_uart_baudrate_e
{
    UART_BAUDRATE_9600,
    UART_BAUDRATE_19200,
    UART_BAUDRATE_38400,
    UART_BAUDRATE_57600,
    UART_BAUDRATE_115200,
    UART_BAUDRATE_END,
} drv_uart_baudrate_t;

/*
 * Macro Definition
 */

/*
 * Function Declaration
 */
 
/* Function Name:
 *      drv_uart_init
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
drv_uart_init(uint32 unit);

/* Function Name:
 *      drv_uart_clearfifo
 * Description:
 *      empty hw rx/tx fifo and reset queue information to default value
 * Input:
 *      que - queue pointer
 *      inch - serial out data
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
extern void
drv_uart_clearfifo(uint32 unit);

/* Function Name:
 *      drv_uart_getc
 * Description:
 *      Get the character from uart interface with timeout value in the specified device
 * Input:
 *      unit    - unit id
 *      timeout - timeout value (unit: milli-second), 0 mean no timeout
 * Output:
 *      pData   - pointer buffer of character from uart interface
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_BUSYWAIT_TIMEOUT - timeout and no get the character
 * Note:
 *      None
 */
extern int32
drv_uart_getc(uint32 unit, uint8 *pData, uint32 timeout);

/* Function Name:
 *      drv_uart_gets
 * Description:
 *      Get the character(s) from uart queue buffer with timeout value in the specified device
 * Input:
 *      unit    - unit id
 *      timeout - timeout value (unit: milli-second), 0 mean no timeout
 * Output:
 *      pData   - pointer buffer of character from uart interface
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_BUSYWAIT_TIMEOUT - timeout and no get the character
 * Note:
 *      None
 */
extern int32
drv_uart_gets(uint32 unit, uint8 *pData, uint32 expect_size,uint32 *rev_cnt,uint32 timeout);

/* Function Name:
 *      drv_uart_putc
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
drv_uart_putc(uint32 unit, uint8 data);

/* Function Name:
 *      drv_uart_puts
 * Description:
 *      Output the character(s) to uart queue buffer in the specified device
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
drv_uart_puts(uint32 unit, uint8 *data,uint32 size);

/* Function Name:
 *      drv_uart_baudrate_get
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
drv_uart_baudrate_get(uint32 unit, drv_uart_baudrate_t *pBaudrate);

/* Function Name:
 *      drv_uart_baudrate_set
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
drv_uart_baudrate_set(uint32 unit, drv_uart_baudrate_t baudrate);

#endif /* __DRV_UART_H__ */
