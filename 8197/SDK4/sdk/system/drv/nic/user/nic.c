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
 * $Revision: 39976 $
 * $Date: 2013-06-05 09:26:45 +0800 (Wed, 05 Jun 2013) $
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
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <common/error.h>
#include <rtcore/rtcore.h>
#include <drv/nic/diag.h>

/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */


/*
 * Function Declaration
 */

/* Function Name:
 *      drv_nic_rx_start
 * Description:
 *      Start the rx action of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 * Applicable:
 *      8389, 8328
 * Note:
 *      None
 */
int32
drv_nic_rx_start(uint32 unit)
{
    int32 fd;
    rtcore_ioctl_t dio;

    if ((fd = open(RTCORE_DEV_NAME, O_RDWR)) < 0)
        return RT_ERR_FAILED;

    dio.data[0] = unit; 
    ioctl(fd, RTCORE_NIC_RX_START, &dio);
    
    close(fd);

    return dio.ret;
} /* end of drv_nic_rx_start */


/* Function Name:
 *      drv_nic_rx_stop
 * Description:
 *      Stop the rx action of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 * Applicable:
 *      8389, 8328
 * Note:
 *      None
 */
int32
drv_nic_rx_stop(uint32 unit)
{
    int32 fd;
    rtcore_ioctl_t dio;

    if ((fd = open(RTCORE_DEV_NAME, O_RDWR)) < 0)
        return RT_ERR_FAILED;

    dio.data[0] = unit; 
    ioctl(fd, RTCORE_NIC_RX_STOP, &dio);
    
    close(fd);

    return dio.ret;
} /* end of drv_nic_rx_stop */

/* Function Name:
 *      drv_nic_pkt_tx
 * Description:
 *      Transmit a packet via nic of the specified device.
 * Input:
 *      unit    - unit id
 *      pPacket - pointer to a single packet struct
 *      fTxCb   - pointer to a handler of transmited packets
 *      pCookie - application data returned with callback (can be null)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      When fTxCb is NULL, driver will free packet and not callback any more.
 */
int32
drv_nic_pkt_tx(uint32 unit, drv_nic_pkt_t *pPacket, drv_nic_tx_cb_f fTxCb, void *pCookie)
{
    int32 fd;
    rtcore_ioctl_t dio;

    if ((fd = open(RTCORE_DEV_NAME, O_RDWR)) < 0)
        return RT_ERR_FAILED;

    dio.data[0] = unit; 
    dio.data[1] = (uint32)pPacket; 
    dio.data[2] = (uint32)fTxCb; 
    dio.data[3] = (uint32)pCookie; 
    ioctl(fd, RTCORE_NIC_PKT_TX, &dio);
    
    close(fd);

    return dio.ret;
} /* end of drv_nic_pkt_tx */



/* Function Name:
 *      drv_nic_reset
 * Description:
 *      Reset nic of the specified device.
 * Input:
 *      unit    - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 */
int32
drv_nic_reset(uint32 unit)
{
    int32 fd;
    rtcore_ioctl_t dio;

    if ((fd = open(RTCORE_DEV_NAME, O_RDWR)) < 0)
        return RT_ERR_FAILED;

    dio.data[0] = unit; 
    ioctl(fd, RTCORE_NIC_RESET, &dio);
    
    close(fd);

    return dio.ret;
} /* end of drv_nic_reset */





