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
 * $Revision: 42819 $
 * $Date: 2013-09-13 16:29:42 +0800 (Fri, 13 Sep 2013) $
 *
 * Purpose : Implementation of the uart driver
 *
 * Feature : uart driver
 *
 */

/*  
 * Include Files 
 */
#include <unistd.h>
#include <fcntl.h>
#include <drv/uart/uart.h>
#include <drv/uart/probe.h>
#include <sys/ioctl.h>
#include <common/error.h>
#include <rtcore/rtcore.h>


/* 
 * Symbol Definition 
 */

/*
 * Macro Definition
 */

/*
 * Function Declaration
 */

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
int32
drv_uart_getc(uint32 unit, uint8 *pData, uint32 timeout)
{
    int32 fd;
    rtcore_ioctl_t dio;

    if ((fd = open(RTCORE_DEV_NAME, O_RDWR)) < 0)
        return RT_ERR_FAILED;

    dio.data[0] = unit;
    dio.data[2] = timeout;
    ioctl(fd, RTCORE_UART1_CHAR_GET, &dio);
    *pData = dio.data[1];
    
    close(fd);

    return dio.ret;

} /* end of drv_uart_getc */


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
int32
drv_uart_putc(uint32 unit, uint8 data)
{
    int32 fd;
    rtcore_ioctl_t dio;

    if ((fd = open(RTCORE_DEV_NAME, O_RDWR)) < 0)
        return RT_ERR_FAILED;

    dio.data[0] = unit;
    dio.data[1] = data;
    ioctl(fd, RTCORE_UART1_CHAR_PUT, &dio);
    
    close(fd);

    return dio.ret;

} /* end of drv_uart_putc */


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
int32
drv_uart_baudrate_get(uint32 unit, drv_uart_baudrate_t *pBaudrate)
{
    int32 fd;
    rtcore_ioctl_t dio;

    if ((fd = open(RTCORE_DEV_NAME, O_RDWR)) < 0)
        return RT_ERR_FAILED;

    dio.data[0] = unit;
    ioctl(fd, RTCORE_UART1_BAUDRATE_GET, &dio);
    
    *pBaudrate = dio.data[1];
    
    close(fd);

    return dio.ret;

} /* end of drv_uart_baudrate_get */

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
int32
drv_uart_baudrate_set(uint32 unit, drv_uart_baudrate_t baudrate)
{
    int32 fd;
    rtcore_ioctl_t dio;

    if ((fd = open(RTCORE_DEV_NAME, O_RDWR)) < 0)
        return RT_ERR_FAILED;

    dio.data[0] = unit;
    dio.data[1] = baudrate;
    ioctl(fd, RTCORE_UART1_BAUDRATE_SET, &dio);
    
    close(fd);

    return dio.ret;

} /* end of drv_uart_baudrate_set */
