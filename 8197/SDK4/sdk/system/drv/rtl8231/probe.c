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
 * Purpose : Definition those public RTL8231 APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) RTL8231 probe
 *
 */

/*
 * Include Files
 */
#include <drv/swcore/chip.h>
#include <drv/rtl8231/probe.h>
#include <drv/rtl8231/r8328.h>
#include <drv/rtl8231/r8380.h>
#include <drv/rtl8231/r8389.h>
#include <drv/rtl8231/r8390.h>


/*
 * Symbol Definition
 */
#define RTL8231_DB_SIZE (sizeof(rtl8231_db)/sizeof(cid_group_t))

/*
 * Data Declaration
 */
const static cid_group_t rtl8231_db[] =
{
    /* RTL8389 series chips */
    {RTL8389M_CHIP_ID, RTL8231_R8389},
    {RTL8329M_CHIP_ID, RTL8231_R8389},
    {RTL8377M_CHIP_ID, RTL8231_R8389},
    {RTL8389L_CHIP_ID, RTL8231_R8389},
    /* RTL8328 series chips */
    {RTL8328M_CHIP_ID, RTL8231_R8328},
    {RTL8328S_CHIP_ID, RTL8231_R8328},
    {RTL8328L_CHIP_ID, RTL8231_R8328},
    /* RTL8390 series chips */
    {RTL8352M_CHIP_ID, RTL8231_R8390},
    {RTL8353M_CHIP_ID, RTL8231_R8390},
    {RTL8391M_CHIP_ID, RTL8231_R8390},
    {RTL8392M_CHIP_ID, RTL8231_R8390},
    {RTL8393M_CHIP_ID, RTL8231_R8390},
    {RTL8396M_CHIP_ID, RTL8231_R8390},
    {RTL8352MES_CHIP_ID, RTL8231_R8390},
    {RTL8353MES_CHIP_ID, RTL8231_R8390},
    {RTL8392MES_CHIP_ID, RTL8231_R8390},
    {RTL8393MES_CHIP_ID, RTL8231_R8390},
    {RTL8396MES_CHIP_ID, RTL8231_R8390},
    /* RTL8380 series chips */
    {RTL8330M_CHIP_ID, RTL8231_R8380},
    {RTL8332M_CHIP_ID, RTL8231_R8380},
    {RTL8380M_CHIP_ID, RTL8231_R8380},
    {RTL8382M_CHIP_ID, RTL8231_R8380},
    {RTL8330MES_CHIP_ID, RTL8231_R8380},
    {RTL8332MES_CHIP_ID, RTL8231_R8380},
    {RTL8380MES_CHIP_ID, RTL8231_R8380},
    {RTL8382MES_CHIP_ID, RTL8231_R8380},
};

rtl8231_mapper_operation_t rtl8231_ops[RTL8231_CTRL_END] = 
{
    {   /* RTL8231_R8389 */
        .i2c_read = (int32 (*)(uint32 , uint32 , uint32 , uint32 *))rtl8231_common_unavail,
        .i2c_write = (int32 (*)(uint32 , uint32 , uint32 , uint32 ))rtl8231_common_unavail,
        .mdc_read = r8389_rtl8231_mdc_read,
        .mdc_write = r8389_rtl8231_mdc_write,
        .init = r8389_rtl8231_init,
        .mdcSem_register = r8389_rtl8231_mdcSem_register,
        .mdcSem_unregister = r8389_rtl8231_mdcSem_unregister,
        .extra_devReady_get = (int32 (*)(uint32 , uint32 , uint32 *))rtl8231_common_unavail,
        .extra_devEnable_get = (int32 (*)(uint32 , uint32 , rtk_enable_t *))rtl8231_common_unavail,
        .extra_devEnable_set = (int32 (*)(uint32 , uint32 , rtk_enable_t ))rtl8231_common_unavail,
        .extra_dataBit_get = (int32 (*)(uint32 , uint32 , uint32 , uint32 *))rtl8231_common_unavail,
        .extra_dataBit_set = (int32 (*)(uint32 , uint32 , uint32 , uint32 ))rtl8231_common_unavail,
        .extra_direction_get = (int32 (*)(uint32 , uint32 , uint32 , drv_gpio_direction_t *))rtl8231_common_unavail,
        .extra_direction_set = (int32 (*)(uint32 , uint32 , uint32 , drv_gpio_direction_t ))rtl8231_common_unavail,
        .extra_i2c_init = (int32 (*)(uint32 , uint32 , uint32 , uint32 ))rtl8231_common_unavail,
        .extra_i2c_get = (int32 (*)(uint32 , uint32 , uint32 *, uint32 *))rtl8231_common_unavail,
        .extra_i2c_read = (int32 (*)(uint32 , uint32 , uint32 , uint32 *))rtl8231_common_unavail,
        .extra_i2c_write = (int32 (*)(uint32 , uint32 , uint32 , uint32 ))rtl8231_common_unavail,
        .extra_read = (int32 (*)(uint32 , uint32 , uint32 , uint32 *))rtl8231_common_unavail,
        .extra_write = (int32 (*)(uint32 , uint32 , uint32 , uint32 ))rtl8231_common_unavail,
    },
    {   /* RTL8231_R8328 */
        .i2c_read = r8328_rtl8231_i2c_read,
        .i2c_write = r8328_rtl8231_i2c_write,
        .mdc_read = (int32 (*)(uint32 , uint32 , uint32 , uint32 , uint32 *))rtl8231_common_unavail,
        .mdc_write = (int32 (*)(uint32 , uint32 , uint32 , uint32 , uint32 ))rtl8231_common_unavail,
        .init = r8328_rtl8231_init,
        .mdcSem_register = r8328_rtl8231_mdcSem_register,
        .mdcSem_unregister = r8328_rtl8231_mdcSem_unregister,
        .extra_devReady_get = (int32 (*)(uint32 , uint32 , uint32 *))rtl8231_common_unavail,
        .extra_devEnable_get = (int32 (*)(uint32 , uint32 , rtk_enable_t *))rtl8231_common_unavail,
        .extra_devEnable_set = (int32 (*)(uint32 , uint32 , rtk_enable_t ))rtl8231_common_unavail,
        .extra_dataBit_get = (int32 (*)(uint32 , uint32 , uint32 , uint32 *))rtl8231_common_unavail,
        .extra_dataBit_set = (int32 (*)(uint32 , uint32 , uint32 , uint32 ))rtl8231_common_unavail,
        .extra_direction_get = (int32 (*)(uint32 , uint32 , uint32 , drv_gpio_direction_t *))rtl8231_common_unavail,
        .extra_direction_set = (int32 (*)(uint32 , uint32 , uint32 , drv_gpio_direction_t ))rtl8231_common_unavail,
        .extra_i2c_init = (int32 (*)(uint32 , uint32 , uint32 , uint32 ))rtl8231_common_unavail,
        .extra_i2c_get = (int32 (*)(uint32 , uint32 , uint32 *, uint32 *))rtl8231_common_unavail,
        .extra_i2c_read = (int32 (*)(uint32 , uint32 , uint32 , uint32 *))rtl8231_common_unavail,
        .extra_i2c_write = (int32 (*)(uint32 , uint32 , uint32 , uint32 ))rtl8231_common_unavail,
        .extra_read = (int32 (*)(uint32 , uint32 , uint32 , uint32 *))rtl8231_common_unavail,
        .extra_write = (int32 (*)(uint32 , uint32 , uint32 , uint32 ))rtl8231_common_unavail,
    },
    {   /* RTL8231_R8390 */
        .i2c_read = (int32 (*)(uint32 , uint32 , uint32 , uint32 *))rtl8231_common_unavail,
        .i2c_write = (int32 (*)(uint32 , uint32 , uint32 , uint32 ))rtl8231_common_unavail,
        .mdc_read = r8390_rtl8231_mdc_read,
        .mdc_write = r8390_rtl8231_mdc_write,
        .init = r8390_rtl8231_init,
        .mdcSem_register = r8390_rtl8231_mdcSem_register,
        .mdcSem_unregister = r8390_rtl8231_mdcSem_unregister,
        .extra_devReady_get = r8390_rtl8231_extra_devReady_get,
        .extra_devEnable_get = r8390_rtl8231_extra_devEnable_get,
        .extra_devEnable_set = r8390_rtl8231_extra_devEnable_set,
        .extra_dataBit_get = r8390_rtl8231_extra_dataBit_get,
        .extra_dataBit_set = r8390_rtl8231_extra_dataBit_set,
        .extra_direction_get = r8390_rtl8231_extra_direction_get,
        .extra_direction_set = r8390_rtl8231_extra_direction_set,
        .extra_i2c_init = (int32 (*)(uint32 , uint32 , uint32 , uint32 ))rtl8231_common_unavail,
        .extra_i2c_get = (int32 (*)(uint32 , uint32 , uint32 *, uint32 *))rtl8231_common_unavail,
        .extra_i2c_read = (int32 (*)(uint32 , uint32 , uint32 , uint32 *))rtl8231_common_unavail,
        .extra_i2c_write = (int32 (*)(uint32 , uint32 , uint32 , uint32 ))rtl8231_common_unavail,
        .extra_read = (int32 (*)(uint32 , uint32 , uint32 , uint32 *))rtl8231_common_unavail,
        .extra_write = (int32 (*)(uint32 , uint32 , uint32 , uint32 ))rtl8231_common_unavail,
    },
    {   /* RTL8231_R8380 */
        .i2c_read = (int32 (*)(uint32 , uint32 , uint32 , uint32 *))rtl8231_common_unavail,
        .i2c_write = (int32 (*)(uint32 , uint32 , uint32 , uint32 ))rtl8231_common_unavail,
        .mdc_read = r8380_rtl8231_mdc_read,
        .mdc_write = r8380_rtl8231_mdc_write,
        .init = r8380_rtl8231_init,
        .mdcSem_register = r8380_rtl8231_mdcSem_register,
        .mdcSem_unregister = r8380_rtl8231_mdcSem_unregister,
        .extra_devReady_get = r8380_rtl8231_extra_devReady_get,
        .extra_devEnable_get = r8380_rtl8231_extra_devEnable_get,
        .extra_devEnable_set = r8380_rtl8231_extra_devEnable_set,
        .extra_dataBit_get = r8380_rtl8231_extra_dataBit_get,
        .extra_dataBit_set = r8380_rtl8231_extra_dataBit_set,
        .extra_direction_get = r8380_rtl8231_extra_direction_get,
        .extra_direction_set = r8380_rtl8231_extra_direction_set,
        .extra_i2c_init = r8380_rtl8231_extra_i2c_init,
        .extra_i2c_get = r8380_rtl8231_extra_i2c_get,
        .extra_i2c_read = r8380_rtl8231_extra_i2c_read,
        .extra_i2c_write = r8380_rtl8231_extra_i2c_write,
        .extra_read = r8380_rtl8231_extra_read,
        .extra_write = r8380_rtl8231_extra_write,
    },
};  

uint32 rtl8231_if[RTK_MAX_NUM_OF_UNIT];
    
/*
 * Function Declaration
 */

/* Function Name:
 *      rtl8231_probe
 * Description:
 *      Probe rtl8231 module of the specified device.
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
rtl8231_probe(uint32 unit)
{
    uint32 i;
    
    for (i = 0; i < RTL8231_DB_SIZE; i++)
    {
        if(!drv_swcore_cid_cmp(unit, rtl8231_db[i].cid))
        {
            rtl8231_if[unit] = rtl8231_db[i].gid;                   
			return RT_ERR_OK;
        }
    }

    return RT_ERR_FAILED;
} /* end of rtl8231_probe */

/* Function Name: 
 *      rtl8231_common_unavail
 * Description: 
 *      Return chip not support
 * Input:  
 *      None
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_CHIP_NOT_SUPPORTED   - functions not supported by this chip model
 * Note: 
 *      None
 */ 
int32
rtl8231_common_unavail(void)
{
    return RT_ERR_CHIP_NOT_SUPPORTED;
} /* end of rtl8231_common_unavail */
