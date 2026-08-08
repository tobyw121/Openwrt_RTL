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
 * $Revision: 40042 $
 * $Date: 2013-06-06 14:05:58 +0800 (Thu, 06 Jun 2013) $
 *
 * Purpose : DRV APIs definition.
 *
 * Feature : SMI relative API
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
#include <drv/swcore/chip.h>
#include <common/debug/rt_log.h>
#include <drv/gpio/gpio.h>
#include <drv/smi/smi.h>

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */

/*
 * Macro Declaration
 */

/*
 * Function Declaration
 */

/* Function Name:
 *      drv_smi_init
 * Description:
 *      SMI init function
 * Input:
 *      portSCK - SCK port id
 *      pinSCK  - SCK pin
 *      portSDA - SDA port id
 *      pinSDA  - SDA pin
 *      dev     - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 drv_smi_init(uint32 portSCK, uint32 pinSCK, uint32 portSDA, uint32 pinSDA, uint32 dev)
{
    int32 fd;
    rtcore_ioctl_t dio;

    if ((fd = open(RTCORE_DEV_NAME, O_RDWR)) < 0)
        return RT_ERR_FAILED;

    dio.data[0] = dev;
    dio.data[1] = portSCK;
    dio.data[2] = pinSCK;
    dio.data[3] = portSDA;
    dio.data[4] = pinSDA;
    
    ioctl(fd, RTCORE_SMI_INIT, &dio);
    
    close(fd);

    return dio.ret;
} /* end of drv_smi_init */

/* Function Name:
 *      drv_smi_group_get
 * Description:
 *      SMI init function
 * Input:
 *      portSCK - SCK port id
 *      pinSCK  - SCK pin
 *      portSDA - SDA port id
 *      pinSDA  - SDA pin
 *      dev     - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 drv_smi_group_get(uint32 * pPortSCK, uint32 * pPinSCK, uint32 * pPortSDA, uint32 * pPinSDA, uint32 dev)
{
    int32 fd;
    rtcore_ioctl_t dio;

    if ((fd = open(RTCORE_DEV_NAME, O_RDWR)) < 0)
        return RT_ERR_FAILED;

    dio.data[0] = dev;
    ioctl(fd, RTCORE_SMI_GROUP_GET, &dio);
    *pPortSCK = dio.data[1];
    *pPinSCK = dio.data[2];
    *pPortSDA = dio.data[3];
    *pPinSDA = dio.data[4];
    
    close(fd);

    return dio.ret;
} /* end of drv_smi_group_get */

/* Function Name:
 *      drv_smi_type_get
 * Description:
 *      SMI init function
 * Input:
 *      portSCK - SCK port id
 *      pinSCK  - SCK pin
 *      portSDA - SDA port id
 *      pinSDA  - SDA pin
 *      dev     - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 drv_smi_type_get(uint32 * ptype, uint32 * pchipid, uint32 * pdelay, uint32 dev)
{
    int32 fd;
    rtcore_ioctl_t dio;

    if ((fd = open(RTCORE_DEV_NAME, O_RDWR)) < 0)
        return RT_ERR_FAILED;

    dio.data[0] = dev;
    ioctl(fd, RTCORE_SMI_TYPE_GET, &dio);
    *ptype = dio.data[1];
    *pchipid = dio.data[2];
    *pdelay = dio.data[3];
    
    close(fd);

    return dio.ret;
} /* end of drv_smi_type_get */

/* Function Name:
 *      drv_smi_type_set
 * Description:
 *      SMI init function
 * Input:
 *      portSCK - SCK port id
 *      pinSCK  - SCK pin
 *      portSDA - SDA port id
 *      pinSDA  - SDA pin
 *      dev     - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 drv_smi_type_set(uint32 smi_type, uint32 chipid, uint32 delay, uint32 dev)
{
    int32 fd;
    rtcore_ioctl_t dio;

    if ((fd = open(RTCORE_DEV_NAME, O_RDWR)) < 0)
        return RT_ERR_FAILED;

    dio.data[0] = dev;
    dio.data[1] = smi_type;
    dio.data[2] = chipid;
    dio.data[3] = delay;
    
    ioctl(fd, RTCORE_SMI_TYPE_SET, &dio);
    
    close(fd);

    return dio.ret;
} /* end of drv_smi_type_set */

/* Function Name:
 *      drv_smi_read
 * Description:
 *      SMI read wrapper function
 * Input:
 *      mAddrs  - address
 *      dev     - dev id
 * Output:
 *      pRdata  - data read
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
int32 drv_smi_read(uint32 mAddrs, uint32 *pRdata, uint32 dev)
{
    int32 fd;
    rtcore_ioctl_t dio;

    if ((fd = open(RTCORE_DEV_NAME, O_RDWR)) < 0)
        return RT_ERR_FAILED;

    dio.data[0] = dev;
    dio.data[1] = mAddrs;
    ioctl(fd, RTCORE_SMI_READ, &dio);
    *pRdata = dio.data[2];
    
    close(fd);

    return dio.ret;
} /* end of drv_smi_read */

/* Function Name:
 *      drv_smi_write
 * Description:
 *      SMI write wrapper function
 * Input:
 *      mAddrs  - address
 *      wData   - data write
 *      dev     - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
int32 drv_smi_write(uint32 mAddrs, uint32 wData, uint32 dev)
{
    int32 fd;
    rtcore_ioctl_t dio;

    if ((fd = open(RTCORE_DEV_NAME, O_RDWR)) < 0)
        return RT_ERR_FAILED;

    dio.data[0] = dev;
    dio.data[1] = mAddrs;
    dio.data[2] = wData;
    
    ioctl(fd, RTCORE_SMI_WRITE, &dio);
    
    close(fd);

    return dio.ret;
} /* end of drv_smi_write */
