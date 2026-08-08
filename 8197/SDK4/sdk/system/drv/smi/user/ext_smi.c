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
 * $Revision: 37221 $
 * $Date: 2013-02-26 14:25:41 +0800 (Tue, 26 Feb 2013) $
 *
 * Purpose : DRV APIs definition.
 *
 * Feature : GPIO relative API
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
#include <drv/gpio/ext_gpio.h>

/* 
 * Symbol Definition 
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
int32 drv_extSmi_dev_init(uint32 unit, uint32 periferal, uint32 phyAddrSCK, uint32 phyAddrSDA, uint32 gpioIdSCK, uint32 gpioIdSDA)
{
    int32 fd;
    rtcore_ioctl_t dio;

    if ((fd = open(RTCORE_DEV_NAME, O_RDWR)) < 0)
        return RT_ERR_FAILED;

    dio.data[0] = unit;
    dio.data[1] = periferal;
    dio.data[2] = phyAddrSCK;
    dio.data[3] = phyAddrSDA;
    dio.data[4] = gpioIdSCK;
    dio.data[5] = gpioIdSDA;
    ioctl(fd, RTCORE_EXTSMI_DEV_INIT, &dio);
    
    close(fd);

    return dio.ret;
}


int32 drv_extSmi_read(uint32 unit, uint32 periferal, uint32 mAddrs, uint32 *pRdata)
{
    int32 fd;
    rtcore_ioctl_t dio;

    if ((fd = open(RTCORE_DEV_NAME, O_RDWR)) < 0)
        return RT_ERR_FAILED;

    dio.data[0] = unit;
    dio.data[1] = periferal;
    dio.data[2] = mAddrs;
    ioctl(fd, RTCORE_EXTSMI_READ, &dio);
    *pRdata = dio.data[3];
    
    close(fd);

    return dio.ret;
}

int32 drv_extSmi_write(uint32 unit, uint32 periferal, uint32 mAddrs, uint32 wData)
{
    int32 fd;
    rtcore_ioctl_t dio;

    if ((fd = open(RTCORE_DEV_NAME, O_RDWR)) < 0)
        return RT_ERR_FAILED;

    dio.data[0] = unit;
    dio.data[1] = periferal;
    dio.data[2] = mAddrs;
    dio.data[3] = wData;
    ioctl(fd, RTCORE_EXTSMI_WRITE, &dio);
    
    close(fd);

    return dio.ret;
}

