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
 * $Revision: 38841 $
 * $Date: 2013-04-23 20:07:54 +0800 (Tue, 23 Apr 2013) $
 *
 * Purpose : Implementation of the RTL8231 driver
 *
 * Feature : RTL8231 driver
 *
 */

/*  
 * Include Files 
 */
#include <soc/soc.h>
#include <soc/type.h>
#include <drv/rtl8231/rtl8231.h>
#include <drv/rtl8231/probe.h>

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
 *      drv_rtl8231_i2c_read
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
drv_rtl8231_i2c_read(uint32 unit, uint32 slave_addr, uint32 reg_addr, uint32 *pData)
{
    return RTL8231_CTRL(unit).i2c_read(unit, slave_addr, reg_addr, pData);
} /* end of drv_rtl8231_i2c_read */

/* Function Name:
 *      drv_rtl8231_i2c_write
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
drv_rtl8231_i2c_write(uint32 unit, uint32 slave_addr, uint32 reg_addr, uint32 data)
{
    return RTL8231_CTRL(unit).i2c_write(unit, slave_addr, reg_addr, data);
} /* end of drv_rtl8231_i2c_write */

/* Function Name:
 *      drv_rtl8231_mdc_read
 * Description:
 *      Read rtl8231 register via MAC indirect access mechanism. (MDC/MDIO)
 * Input:
 *      unit     - unit id
 *      phy_id   - PHY id
 *      page     - PHY page
 *      reg_addr - 8231 register address
 * Output:
 *      pData    - pointer buffer of data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - pData is a null pointer.
 * Note:
 *      1) valid page as following:
 *      - 0x1D is internal register page
 *      - 0x1E is system register page (default)
 */
int32 
drv_rtl8231_mdc_read(uint32 unit, uint32 phy_id, uint32 page, uint32 reg_addr, uint32 *pData)
{
    return RTL8231_CTRL(unit).mdc_read(unit, phy_id, page, reg_addr, pData);
} /* end of drv_rtl8231_mdc_read */

/* Function Name:
 *      drv_rtl8231_mdc_write
 * Description:
 *      Write rtl8231 register via MAC indirect access mechanism. (MDC/MDIO)
 * Input:
 *      unit     - unit id
 *      phy_id   - PHY id
 *      page     - PHY page
 *      reg_addr - 8231 register address
 *      data     - configure data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      1) valid page as following:
 *      - 0x1D is internal register page
 *      - 0x1E is system register page (default)
 */
int32 
drv_rtl8231_mdc_write(uint32 unit, uint32 phy_id, uint32 page, uint32 reg_addr, uint32 data)
{
    return RTL8231_CTRL(unit).mdc_write(unit, phy_id, page, reg_addr, data);
} /* end of drv_rtl8231_mdc_write */

/* Function Name:
 *      drv_rtl8231_init
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
drv_rtl8231_init(uint32 unit)
{
    return RTL8231_CTRL(unit).init(unit);
} /* end of drv_rtl8231_init */

/* Function Name:
 *      drv_rtl8231_mdcSem_register
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
drv_rtl8231_mdcSem_register(uint32 unit, drv_rtl8231_mdcSem_cb_f fMdcSemCb)
{
    return RTL8231_CTRL(unit).mdcSem_register(unit, fMdcSemCb);
} /* end of drv_rtl8231_mdcSem_register */

/* Function Name:
 *      drv_rtl8231_mdcSem_unregister
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
drv_rtl8231_mdcSem_unregister(uint32 unit, drv_rtl8231_mdcSem_cb_f fMdcSemCb)
{
    return RTL8231_CTRL(unit).mdcSem_unregister(unit, fMdcSemCb);
} /* end of drv_rtl8231_mdcSem_unregister */

/* Function Name:
 *      drv_rtl8231_extra_devReady_get
 * Description:
 *      Get extra GPIO device ready status
 * Input:
 *      unit - unit id
 *      addr - extra GPIO address
 * Output:
 *      pIsReady - the device ready status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
drv_rtl8231_extra_devReady_get(uint32 unit, uint32 addr, uint32 *pIsReady)
{
    return RTL8231_CTRL(unit).extra_devReady_get(unit, addr, pIsReady);
} /* end of drv_rtl8231_extra_devReady_get */

/* Function Name:
 *      drv_rtl8231_extra_devEnable_get
 * Description:
 *      Get the external GPIO status in the specified device of the unit
 * Input:
 *      unit - unit id
 *      addr - external GPIO address
 * Output:
 *      pEnable - the buffer pointer of the status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
drv_rtl8231_extra_devEnable_get(uint32 unit, uint32 addr, rtk_enable_t *pEnable)
{
    return RTL8231_CTRL(unit).extra_devEnable_get(unit, addr, pEnable);
} /* end of drv_rtl8231_extra_devEnable_get */

/* Function Name:
 *      drv_rtl8231_extra_devEnable_set
 * Description:
 *      Set the external GPIO status in the specified device of the unit
 * Input:
 *      unit   - unit id
 *      addr   - external GPIO address
 *      enable - the status of the specified external GPIO device
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
drv_rtl8231_extra_devEnable_set(uint32 unit, uint32 addr, rtk_enable_t enable)
{
    return RTL8231_CTRL(unit).extra_devEnable_set(unit, addr, enable);
} /* end of drv_rtl8231_extra_devEnable_set */

/* Function Name:
 *      drv_rtl8231_extra_dataBit_get
 * Description:
 *      Get the external GPIO pin value in the specified device of the unit
 * Input:
 *      unit   - unit id
 *      addr   - external GPIO address
 *      gpioId - gpio id
 * Output:
 *      pData  - the buffer pointer of the gpio pin value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
drv_rtl8231_extra_dataBit_get(uint32 unit, uint32 addr, uint32 gpioId, uint32 *pData)
{
    return RTL8231_CTRL(unit).extra_dataBit_get(unit, addr, gpioId, pData);
} /* end of drv_rtl8231_extra_dataBit_get */

/* Function Name:
 *      drv_rtl8231_extra_dataBit_set
 * Description:
 *      Set the external GPIO pin value in the specified device of the unit
 * Input:
 *      unit   - unit id
 *      addr   - external GPIO address
 *      gpioId - gpio id
 *      data   - the gpio pin value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
drv_rtl8231_extra_dataBit_set(uint32 unit, uint32 addr, uint32 gpioId, uint32 data)
{
    return RTL8231_CTRL(unit).extra_dataBit_set(unit, addr, gpioId, data);
} /* end of drv_rtl8231_extra_dataBit_set */

/* Function Name:
 *      drv_rtl8231_extra_direction_get
 * Description:
 *      Get the external GPIO pin direction in the specified device of the unit
 * Input:
 *      unit   - unit id
 *      addr   - external GPIO address
 *      gpioId - gpio id
 * Output:
 *      pData  - the buffer pointer of the gpio pin direction
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
drv_rtl8231_extra_direction_get(uint32 unit, uint32 addr, uint32 gpioId, drv_gpio_direction_t *pData)
{
    return RTL8231_CTRL(unit).extra_direction_get(unit, addr, gpioId, pData);
} /* end of drv_rtl8231_extra_direction_get */

/* Function Name:
 *      drv_rtl8231_extra_direction_set
 * Description:
 *      Set the external GPIO pin direction in the specified device of the unit
 * Input:
 *      unit   - unit id
 *      addr   - external GPIO address
 *      gpioId - gpio id
 *      data   - the gpio pin direction
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
drv_rtl8231_extra_direction_set(uint32 unit, uint32 addr, uint32 gpioId, drv_gpio_direction_t data)
{
    return RTL8231_CTRL(unit).extra_direction_set(unit, addr, gpioId, data);
} /* end of drv_rtl8231_extra_direction_set */

/* Function Name:
 *      drv_rtl8231_extra_i2c_init
 * Description:
 *      Initialize the SCK/SDA in external GPIO pin in the specified device of the unit
 * Input:
 *      unit      - unit id
 *      addr      - external GPIO address
 *      i2c_clock - i2c SCK pin in external GPIO device
 *      i2c_data  - i2c SDA pin in external GPIO device
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
drv_rtl8231_extra_i2c_init(uint32 unit, uint32 addr, uint32 i2c_clock, uint32 i2c_data)
{
    return RTL8231_CTRL(unit).extra_i2c_init(unit, addr, i2c_clock, i2c_data);
} /* end of drv_rtl8231_extra_i2c_init */

/* Function Name:
 *      drv_rtl8231_extra_i2c_get
 * Description:
 *      Get the SCK/SDA in external GPIO pin in the specified device of the unit
 * Input:
 *      unit       - unit id
 *      addr       - external GPIO address
 * Output:
 *      pI2c_clock - buffer pointer of i2c SCK pin in external GPIO device
 *      pI2c_data  - buffer pointer of i2c SDA pin in external GPIO device
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
drv_rtl8231_extra_i2c_get(uint32 unit, uint32 addr, uint32 *pI2c_clock, uint32 *pI2c_data)
{
    return RTL8231_CTRL(unit).extra_i2c_get(unit, addr, pI2c_clock, pI2c_data);
} /* end of drv_rtl8231_extra_i2c_get */

/* Function Name:
 *      drv_rtl8231_extra_i2c_read
 * Description:
 *      Read the value of register in external GPIO pin in the specified device of the unit
 * Input:
 *      unit  - unit id
 *      addr  - external GPIO address
 *      reg   - register to read
 * Output:
 *      pData - buffer pointer of data value in external GPIO device
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
drv_rtl8231_extra_i2c_read(uint32 unit, uint32 addr, uint32 reg, uint32 *pData)
{
    return RTL8231_CTRL(unit).extra_i2c_read(unit, addr, reg, pData);
} /* end of drv_rtl8231_extra_i2c_read */

/* Function Name:
 *      drv_rtl8231_extra_i2c_write
 * Description:
 *      Write the value of register in external GPIO pin in the specified device of the unit
 * Input:
 *      unit - unit id
 *      addr - external GPIO address
 *      reg  - register to read
 *      data - data value in external GPIO device
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
drv_rtl8231_extra_i2c_write(uint32 unit, uint32 addr, uint32 reg, uint32 data)
{
    return RTL8231_CTRL(unit).extra_i2c_write(unit, addr, reg, data);
} /* end of drv_rtl8231_extra_i2c_write */

/* Function Name:
 *      drv_rtl8231_extra_read
 * Description:
 *      Read rtl8231 register from extra GPIO device. (EXTRA)
 * Input:
 *      unit     - unit id
 *      addr     - external GPIO address
 *      reg_addr - 8231 register address
 * Output:
 *      pData    - pointer buffer of data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - pData is a null pointer.
 * Note:
 *      None
 */
int32 
drv_rtl8231_extra_read(uint32 unit, uint32 addr, uint32 reg_addr, uint32 *pData)
{
    return RTL8231_CTRL(unit).extra_read(unit, addr, reg_addr, pData);
} /* end of drv_rtl8231_extra_read */

/* Function Name:
 *      drv_rtl8231_extra_write
 * Description:
 *      Write rtl8231 register to extra GPIO device. (EXTRA)
 * Input:
 *      unit     - unit id
 *      addr     - external GPIO address
 *      reg_addr - 8231 register address
 *      data     - pointer buffer of data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32 
drv_rtl8231_extra_write(uint32 unit, uint32 addr, uint32 reg_addr, uint32 data)
{
    return RTL8231_CTRL(unit).extra_write(unit, addr, reg_addr, data);
} /* end of drv_rtl8231_extra_write */

