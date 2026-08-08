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
 * $Revision: 36809 $
 * $Date: 2013-02-04 13:42:44 +0800 (Mon, 04 Feb 2013) $
 *
 * Purpose : Definition those public RTL8231 APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *            1) i2c read & write
 *            2) mdc read & write
 */

/*
 * Include Files
 */
#include <ioal/mem32.h>
#include <drv/rtl8231/r8328.h>
#include <drv/swcore/rtl8328.h>
#include <hal/common/halctrl.h>

/*
 * Symbol Definition
 */
#define CHECKBUSY_TIMES (3000)

/*
 * Data Type Definition
 */

/*
 * Data Declaration
 */
static uint32       rtl8231_init[RTK_MAX_NUM_OF_UNIT];
static rtl8231_mdcSem_cb_entry_t _rtl8231_mdcSem_cb_tbl[RTK_MAX_NUM_OF_UNIT];

/*
 * Macro Definition
 */
#define RTL8231_I2C_BUSY_WAIT_LOOP(unit, REG, MASK)\
{\
    uint32 i;\
    uint32 regVal;\
    for (i = 0; i < CHECKBUSY_TIMES; i++)\
    {\
        if (ioal_mem32_read(unit, REG, &regVal) != RT_ERR_OK)\
        {\
            return RT_ERR_FAILED;\
        }\
        if (0 == (regVal & MASK))\
        {\
            break;\
        }\
    }\
    if (CHECKBUSY_TIMES == i)\
    {\
        return RT_ERR_FAILED;\
    }\
}

/*
 * Function Declaration
 */

/* Function Name:
 *      r8328_rtl8231_i2c_read
 * Description:
 *      Read rtl8231 register via MAC indirect access mechanism. (I2C)
 * Input:
 *      unit       - unit id
 *      slave_addr - I2C slave address
 *      reg_addr   - 8231 register address
 * Output:
 *      pData      - pointer buffer of data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - pData is a null pointer.
 * Note:
 *      None
 */
int32 
r8328_rtl8231_i2c_read(uint32 unit, uint32 slave_addr, uint32 reg_addr, uint32 *pData)
{
    uint32  value = 0;

    /* Check init state */
    RT_INIT_CHK(rtl8231_init[unit]);

    value |= ((reg_addr << RTL8328_TRIGGER_EXTERNAL_GPIO_REG_ADDR_OFFSET) & RTL8328_TRIGGER_EXTERNAL_GPIO_REG_ADDR_MASK);
    value |= ((slave_addr << RTL8328_TRIGGER_EXTERNAL_GPIO_IC_ADDR_OFFSET) & RTL8328_TRIGGER_EXTERNAL_GPIO_IC_ADDR_MASK);
    value |= ((1 << RTL8328_TRIGGER_EXTERNAL_GPIO_WR_SEL_OFFSET) & RTL8328_TRIGGER_EXTERNAL_GPIO_WR_SEL_MASK);
    value |= ((1 << RTL8328_TRIGGER_EXTERNAL_GPIO_TRIGGER_OFFSET) & RTL8328_TRIGGER_EXTERNAL_GPIO_TRIGGER_MASK);

    ioal_mem32_write(unit, RTL8328_TRIGGER_EXTERNAL_GPIO_ADDR, value);
    RTL8231_I2C_BUSY_WAIT_LOOP(unit, RTL8328_TRIGGER_EXTERNAL_GPIO_ADDR, 0x1000);
    ioal_mem32_read(unit, RTL8328_EXTERNAL_DATA_READ_ADDR, pData);

    return RT_ERR_OK;
} /* end of r8328_rtl8231_i2c_read */

/* Function Name:
 *      r8328_rtl8231_i2c_write
 * Description:
 *      Write rtl8231 register via MAC indirect access mechanism. (I2C)
 * Input:
 *      unit       - unit id
 *      slave_addr - I2C slave address
 *      reg_addr   - 8231 register address
 *      data       - configure data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32 
r8328_rtl8231_i2c_write(uint32 unit, uint32 slave_addr, uint32 reg_addr, uint32 data)
{
    uint32  value = 0;

    /* Check init state */
    RT_INIT_CHK(rtl8231_init[unit]);

    value |= ((reg_addr << RTL8328_TRIGGER_EXTERNAL_GPIO_REG_ADDR_OFFSET) & RTL8328_TRIGGER_EXTERNAL_GPIO_REG_ADDR_MASK);
    value |= ((slave_addr << RTL8328_TRIGGER_EXTERNAL_GPIO_IC_ADDR_OFFSET) & RTL8328_TRIGGER_EXTERNAL_GPIO_IC_ADDR_MASK);
    value |= ((0 << RTL8328_TRIGGER_EXTERNAL_GPIO_WR_SEL_OFFSET) & RTL8328_TRIGGER_EXTERNAL_GPIO_WR_SEL_MASK);
    value |= ((1 << RTL8328_TRIGGER_EXTERNAL_GPIO_TRIGGER_OFFSET) & RTL8328_TRIGGER_EXTERNAL_GPIO_TRIGGER_MASK);

    ioal_mem32_write(unit, RTL8328_EXTERNAL_DATA_WRITE_ADDR, data);
    ioal_mem32_write(unit, RTL8328_TRIGGER_EXTERNAL_GPIO_ADDR, value);
    RTL8231_I2C_BUSY_WAIT_LOOP(unit, RTL8328_TRIGGER_EXTERNAL_GPIO_ADDR, 0x1000);

    return RT_ERR_OK;
} /* end of r8328_rtl8231_i2c_write */

/* Function Name:
 *      r8328_rtl8231_init
 * Description:
 *      Initialize rtl8231 driver.
 * Input:
 *      unit     - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32 
r8328_rtl8231_init(uint32 unit)
{
    rtl8231_init[unit] = INIT_NOT_COMPLETED;

    /* Do some initialize if need */

    rtl8231_init[unit] = INIT_COMPLETED;

    return RT_ERR_OK;
} /* end of r8328_rtl8231_init */

/* Function Name:
 *      r8328_rtl8231_mdcSem_register
 * Description:
 *      Register the rtl8231 MDC/MDIO semaphore callback.
 * Input:
 *      unit      - unit id
 *      fMdcSemCb - rtl8231 MDC/MDIO semaphore callback function
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
r8328_rtl8231_mdcSem_register(uint32 unit, drv_rtl8231_mdcSem_cb_f fMdcSemCb)
{
    /* Check init state */
    RT_INIT_CHK(rtl8231_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(NULL == fMdcSemCb, RT_ERR_NULL_POINTER);

    if (NULL == _rtl8231_mdcSem_cb_tbl[unit].mdcSem_callback)
    {
        _rtl8231_mdcSem_cb_tbl[unit].mdcSem_callback = fMdcSemCb;
    }
    else
    {
        /* Handler is already existing */
        return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
} /* end of r8328_rtl8231_mdcSem_register */

/* Function Name:
 *      r8328_rtl8231_mdcSem_unregister
 * Description:
 *      Unregister the rtl8231 MDC/MDIO semaphore callback.
 * Input:
 *      unit      - unit id
 *      fMdcSemCb - rtl8231 MDC/MDIO semaphore callback function
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
r8328_rtl8231_mdcSem_unregister(uint32 unit, drv_rtl8231_mdcSem_cb_f fMdcSemCb)
{
    /* Check init state */
    RT_INIT_CHK(rtl8231_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(NULL == fMdcSemCb, RT_ERR_NULL_POINTER);

    if (_rtl8231_mdcSem_cb_tbl[unit].mdcSem_callback == fMdcSemCb)
    {
        _rtl8231_mdcSem_cb_tbl[unit].mdcSem_callback = NULL;
    }
    else
    {
        /* Handler is nonexistent */
        return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
} /* end of r8328_rtl8231_mdcSem_unregister */
