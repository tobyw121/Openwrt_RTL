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
 * $Revision: 48962 $
 * $Date: 2014-07-01 10:17:05 +0800 (Tue, 01 Jul 2014) $
 *
 * Purpose : chip symbol and data type definition in the SDK.
 *
 * Feature : chip symbol and data type definition
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

/* Function Name:
 *      drv_swcore_cid_get
 * Description:
 *      Get chip id and chip revision id.
 * Input:
 *      unit           - unit id
 * Output:
 *      pChip_id       - pointer buffer of chip id
 *      pChip_rev_id   - pointer buffer of chip revision id
 * Return:
 *      RT_ERR_OK      - OK
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32
drv_swcore_cid_get(uint32 unit, uint32 *pChip_id, uint32 *pChip_rev_id)
{
    int32 fd;
    rtcore_ioctl_t dio;

    /* parameter check */
    RT_PARAM_CHK((NULL == pChip_id), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pChip_rev_id), RT_ERR_NULL_POINTER);

    if ((fd = open(RTCORE_DEV_NAME, O_RDWR)) < 0)
        return RT_ERR_FAILED;

    dio.data[0] = unit;
    ioctl(fd, RTCORE_CID_GET, &dio);
    *pChip_id = dio.data[1];
    *pChip_rev_id = dio.data[2];
    
    close(fd);

    return dio.ret;

} /* end of drv_swcore_cid_get */

/* Function Name:
 *      drv_swcore_cid_cmp
 * Description:
 *      Compare cmp_id with the chip id of unit
 * Input:
 *      unit           - unit id
 *      cmp_id         - cmp id
 * Output:
 *      None
 * Return:
 *      0              - identical
 *      RT_ERR_FAILED  - not identical
 * Note:
 *      None
 */
int32
drv_swcore_cid_cmp(uint32 unit, uint32 cmp_id)
{
    int32 fd;
    rtcore_ioctl_t dio;

    if ((fd = open(RTCORE_DEV_NAME, O_RDWR)) < 0)
        return RT_ERR_FAILED;

    dio.data[0] = unit;
    dio.data[1] = cmp_id;
    ioctl(fd, RTCORE_CID_CMP, &dio);
        
    close(fd);

    return dio.ret;        
} /* end of drv_swcore_cid_cmp */


/* Function Name:
 *      drv_swcore_register_dump
 * Description:
 *      Dump Chip peripher registers
 * Input:
 *      unit           - unit id
 * Output:
 *      N/A
 * Return:
 *      RT_ERR_OK        - OK
 *      RT_ERR_FAILED   - Failed
 * Note:
 *      None
 */
int32
drv_swcore_register_dump(uint32 unit)
{
    int32 fd;
    rtcore_ioctl_t dio;

    if ((fd = open(RTCORE_DEV_NAME, O_RDWR)) < 0)
        return RT_ERR_FAILED;

    dio.data[0] = unit;
    ioctl(fd, RTCORE_DEBUG_REGISTER_DUMP, &dio);

    close(fd);

    return dio.ret;


}/* end of drv_swcore_register_dump */

