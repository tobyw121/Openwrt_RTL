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
 * $Revision: 37951 $
 * $Date: 2013-03-20 13:52:19 +0800 (Wed, 20 Mar 2013) $
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
 *      drv_nic_dbg_set
 * Description:
 *      Set NIC debug flags of the specified device.
 * Input:
 *      unit  - unit id
 *      flags - NIC debug flags
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 * Note:
 *      There are 4 BIT flags can be selected as following:
 *      - DEBUG_RX_RAW_LEN_BIT
 *      - DEBUG_RX_CPU_TAG_BIT
 *      - DEBUG_TX_RAW_LEN_BIT
 *      - DEBUG_TX_CPU_TAG_BIT
 */
int32
drv_nic_dbg_set(uint32 unit, uint32 flags)
{
    int32 fd;
    rtcore_ioctl_t dio;

    if ((fd = open(RTCORE_DEV_NAME, O_RDWR)) < 0)
        return RT_ERR_FAILED;

    dio.data[0] = unit;
    dio.data[1] = flags;
    ioctl(fd, RTCORE_NIC_DBG_SET, &dio);
    
    close(fd);

    return dio.ret;
}

/* Function Name:
 *      drv_nic_dbg_get
 * Description:
 *      Get NIC debug flags of the specified device.
 * Input:
 *      unit   - unit id
 * Output:
 *      pFlags - NIC debug flags
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 * Note:
 *      There are 4 BIT flags as following:
 *      - DEBUG_RX_RAW_LEN_BIT
 *      - DEBUG_RX_CPU_TAG_BIT
 *      - DEBUG_TX_RAW_LEN_BIT
 *      - DEBUG_TX_CPU_TAG_BIT
 */
int32
drv_nic_dbg_get(uint32 unit, uint32 *pFlags)
{
    int32 fd;
    rtcore_ioctl_t dio;

    if ((fd = open(RTCORE_DEV_NAME, O_RDWR)) < 0)
        return RT_ERR_FAILED;

    dio.data[0] = unit;
    ioctl(fd, RTCORE_NIC_DBG_GET, &dio);
    *pFlags = dio.data[1];
    
    close(fd);

    return dio.ret;
}

/* Function Name:
 *      drv_nic_cntr_dump
 * Description:
 *      Dump NIC debug counter information of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      There are 4 debug counters be dump as following:
 *      - nic_tx_success_cntr
 *      - nic_tx_failed_cntr
 *      - nic_rx_success_cntr
 *      - nic_rx_failed_cntr
 */
int32
drv_nic_cntr_dump(uint32 unit)
{
    int32 fd;
    rtcore_ioctl_t dio;

    if ((fd = open(RTCORE_DEV_NAME, O_RDWR)) < 0)
        return RT_ERR_FAILED;

    dio.data[0] = unit;
    ioctl(fd, RTCORE_NIC_CNTR_DUMP, &dio);
    
    close(fd);

    return dio.ret;
}

/* Function Name:
 *      drv_nic_cntr_clear
 * Description:
 *      Clear NIC debug counter information of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Clear following NIC debug counters
 *      - nic_tx_success_cntr
 *      - nic_tx_failed_cntr
 *      - nic_rx_success_cntr
 *      - nic_rx_failed_cntr
 */
int32
drv_nic_cntr_clear(uint32 unit)
{
    int32 fd;
    rtcore_ioctl_t dio;

    if ((fd = open(RTCORE_DEV_NAME, O_RDWR)) < 0)
        return RT_ERR_FAILED;

    dio.data[0] = unit;
    ioctl(fd, RTCORE_NIC_CNTR_CLEAR, &dio);
    
    close(fd);

    return dio.ret;
}

/* Function Name:
 *      drv_nic_ringbuf_dump
 * Description:
 *      Dump NIC buffer status of the specified device.
 * Input:
 *      unit   - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Following message are dump
 *      1) From SW View
 *      - Rx Ring Packet Header (FDPBase, CDPIdx, RDPIdx)
 *      - Tx Ring Packet Header (FDPBase, CDPIdx, RDPIdx)
 *      - Rx Ring mBuffer (FDPBase, CDPIdx, RDPIdx)
 *      - Tx Ring mBuffer (FDPBase, CDPIdx, RDPIdx)
 *      2) From HW View
 *      - Rx Ring Packet Header(CDPIdx)
 *      - Tx Ring Packet Header(CDPIdx)
 *      - Rx Ring mBuffer (CDPIdx)
 *      - Tx Ring mBuffer (CDPIdx)
 *      3) Register Information
 *      - CPUIIMR (CPU Interface Interrupt Mask Register)
 *      - CPUIISR (CPU Interface Interrupt Status Register)
 *      - CPUICR  (CPU Interface Control Register)
 */
int32
drv_nic_ringbuf_dump(uint32 unit)
{
    int32 fd;
    rtcore_ioctl_t dio;

    if ((fd = open(RTCORE_DEV_NAME, O_RDWR)) < 0)
        return RT_ERR_FAILED;

    dio.data[0] = unit;
    ioctl(fd, RTCORE_NIC_BUF_DUMP, &dio);
    
    close(fd);

    return dio.ret;
}

/* Function Name:
 *      drv_nic_pkthdr_mbuf_dump
 * Description:
 *      Dump NIC packet header and mbuf detail information of the specified device.
 * Input:
 *      unit  - unit id
 *      mode  - tx/rx mode
 *      start - start ring id
 *      end   - end ring id
 *      flags - dump flags
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      1) valid 'mode' value:
 *      - NIC_PKTHDR_MBUF_MODE_RX
 *      - NIC_PKTHDR_MBUF_MODE_TX
 *      2) valid ring id (start .. end)
 *      - Rx (0 .. 7)
 *      - Tx (0 .. 1)
 *      3) valid 'flags' value:
 *      - TRUE: include packet raw data
 *      - FALSE: exclude packet raw data
 */
int32
drv_nic_pkthdr_mbuf_dump(uint32 unit, uint32 mode, uint32 start, uint32 end, uint32 flags)
{
    int32 fd;
    rtcore_ioctl_t dio;

    if ((fd = open(RTCORE_DEV_NAME, O_RDWR)) < 0)
        return RT_ERR_FAILED;

    dio.data[0] = unit;
    dio.data[1] = mode;
    dio.data[2] = start;
    dio.data[3] = end;
    dio.data[4] = flags;
    ioctl(fd, RTCORE_NIC_PHMBUF_DUMP, &dio);
    
    close(fd);

    return dio.ret;
}

/* Function Name:
 *      drv_nic_rx_status_get
 * Description:
 *      Get NIC rx status of the specified device.
 * Input:
 *      unit    - unit id
 * Output:
 *      pStatus - rx status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 * Note:
 *      None
 */
int32
drv_nic_rx_status_get(uint32 unit, uint32 *pStatus)
{
    int32 fd;
    rtcore_ioctl_t dio;

    if ((fd = open(RTCORE_DEV_NAME, O_RDWR)) < 0)
        return RT_ERR_FAILED;

    dio.data[0] = unit;
    ioctl(fd, RTCORE_NIC_RX_STATUS_GET, &dio);
    *pStatus = dio.data[1];
    
    close(fd);

    return dio.ret;
}
