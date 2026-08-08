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
 * $Revision: 30020 $
 * $Date: 2012-06-18 15:09:58 +0800 (Mon, 18 Jun 2012) $
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
#include <common/rt_autoconf.h>
#include <common/debug/rt_log.h>
#include <osal/sem.h>
#include <ioal/mem32.h>
#include <drv/rtl8231/r8390.h>
#include <drv/swcore/rtl8390.h>
#include <hal/common/halctrl.h>

/*
 * Symbol Definition
 */
#define CHECKBUSY_TIMES (150000)

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
#if defined(CONFIG_SDK_RTL8231) && defined(CONFIG_SDK_KERNEL_LINUX_KERNEL_MODE)
#define RTL8231_MDC_SEM_LOCK(unit)  _rtl8231_mdcSem_cb_tbl[unit].mdcSem_callback(unit, 0)
#define RTL8231_MDC_SEM_UNLOCK(unit)    _rtl8231_mdcSem_cb_tbl[unit].mdcSem_callback(unit, 1)
#else
#define RTL8231_MDC_SEM_LOCK(unit)
#define RTL8231_MDC_SEM_UNLOCK(unit)
#endif

#define GPIO_READ       0
#define GPIO_WRITE      1

#define RTL8231_MDC_BUSY_WAIT_LOOP(unit, REG, MASK)\
{\
    uint32 i;\
    uint32 regVal;\
    for (i = 0; i < CHECKBUSY_TIMES; i++)\
    {\
        if (ioal_mem32_read(unit, REG, &regVal) != RT_ERR_OK)\
        {\
            RTL8231_MDC_SEM_UNLOCK(unit);\
            return RT_ERR_FAILED;\
        }\
        if (0 == (regVal & MASK))\
        {\
            break;\
        }\
    }\
    if (CHECKBUSY_TIMES == i)\
    {\
        RTL8231_MDC_SEM_UNLOCK(unit);\
        return RT_ERR_BUSYWAIT_TIMEOUT;\
    }\
}

/*
 * Function Declaration
 */

/* Function Name:
 *      r8390_rtl8231_init
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
r8390_rtl8231_init(uint32 unit)
{
    uint32  value = 0;

    rtl8231_init[unit] = INIT_NOT_COMPLETED;

    /* Do some initialize if need */

    ioal_mem32_read(unit, RTL8390_LED_GLB_CTRL_ADDR, &value);
    value &= ~(RTL8390_LED_GLB_CTRL_EXT_GPIO_EN_MASK);
    value |= (0x4 << RTL8390_LED_GLB_CTRL_EXT_GPIO_EN_OFFSET);
    ioal_mem32_write(unit, RTL8390_LED_GLB_CTRL_ADDR, value);

    rtl8231_init[unit] = INIT_COMPLETED;
    return RT_ERR_OK;
} /* end of r8390_rtl8231_init */

/* Function Name:
 *      r8390_rtl8231_mdcSem_register
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
r8390_rtl8231_mdcSem_register(uint32 unit, drv_rtl8231_mdcSem_cb_f fMdcSemCb)
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
} /* end of r8390_rtl8231_mdcSem_register */

/* Function Name:
 *      r8390_rtl8231_mdcSem_unregister
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
r8390_rtl8231_mdcSem_unregister(uint32 unit, drv_rtl8231_mdcSem_cb_f fMdcSemCb)
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
} /* end of r8390_rtl8231_mdcSem_unregister */

/* Function Name:
 *      r8390_rtl8231_mdc_read
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
r8390_rtl8231_mdc_read(uint32 unit, uint32 phy_id, uint32 page, uint32 reg_addr, uint32 *pData)
{
    uint32  temp;

    /* check Init status */
    RT_INIT_CHK(rtl8231_init[unit]);

    RTL8231_MDC_SEM_LOCK(unit);

    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * If RWOP = 0(read), then INDATA[15:0] = {Reserved & PORT_ID[4:0]}
     * If RWOP = 1(write), then INDATA[15:0] = DATA[15:0]
     */
    temp |= ((phy_id << RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_PHY_ADDR_OFFSET) & RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_PHY_ADDR_MASK);

    /* Select register number to access */
    temp |= ((reg_addr << RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_REG_OFFSET) & RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_REG_MASK);

    /* Read/Write operation
     * 0b0: read
     * 0b1: write
     */
    temp |= ((GPIO_READ << RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_RWOP_OFFSET) & RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_RWOP_MASK);

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    temp |= ((1 << RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_CMD_OFFSET) & RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_CMD_MASK);

    /* write register to active the read operation */
    ioal_mem32_write(unit, RTL8390_EXT_GPIO_INDRT_ACCESS_ADDR, temp);

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    RTL8231_MDC_BUSY_WAIT_LOOP(unit, RTL8390_EXT_GPIO_INDRT_ACCESS_ADDR, 0x1);

    /* get the read operation result to temp */
    ioal_mem32_read(unit, RTL8390_EXT_GPIO_INDRT_ACCESS_ADDR, &temp);

    /* fill the DATA[15:0] from temp to pData */
    (*pData) = (temp & RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_DATA_MASK) >> RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_DATA_OFFSET;

    RTL8231_MDC_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}

/* Function Name:
 *      r8390_rtl8231_mdc_write
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
r8390_rtl8231_mdc_write(uint32 unit, uint32 phy_id, uint32 page, uint32 reg_addr, uint32 data)
{
    uint32  temp;

    /* check Init status */
    RT_INIT_CHK(rtl8231_init[unit]);

    RTL8231_MDC_SEM_LOCK(unit);

    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * If RWOP = 0(read), then INDATA[15:0] = {Reserved & PORT_ID[4:0]}
     * If RWOP = 1(write), then INDATA[15:0] = DATA[15:0]
     */
    temp |= ((phy_id << RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_PHY_ADDR_OFFSET) & RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_PHY_ADDR_MASK);

    /* Select register number to access */
    temp |= ((reg_addr << RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_REG_OFFSET) & RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_REG_MASK);

    /* Read/Write operation
     * 0b0: read
     * 0b1: write
     */
    temp |= ((GPIO_WRITE << RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_RWOP_OFFSET) & RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_RWOP_MASK);

    /* Input parameters:
     * If RWOP = 0(read), then GPIO_DATA[15:0] = input data[15:0]
     * If RWOP = 1(write), then GPIO_DATA [15:0] = output data[15:0]
     */
    temp |= ((data << RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_DATA_OFFSET) & RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_DATA_MASK);

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    temp |= ((1 << RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_CMD_OFFSET) & RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_CMD_MASK);

    /* write register to active the read operation */
    ioal_mem32_write(unit, RTL8390_EXT_GPIO_INDRT_ACCESS_ADDR, temp);

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    RTL8231_MDC_BUSY_WAIT_LOOP(unit, RTL8390_EXT_GPIO_INDRT_ACCESS_ADDR, 0x1);

    RTL8231_MDC_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}

/* Function Name:
 *      r8390_rtl8231_extra_devReady_get
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
r8390_rtl8231_extra_devReady_get(uint32 unit, uint32 addr, uint32 *pIsReady)
{
#if 0
    uint32  temp;

    /* Check init state */
    RT_INIT_CHK(rtl8231_init[unit]);

    RTL8231_MDC_SEM_LOCK(unit);
    ioal_mem32_read(unit, RTL8390_LED_GLB_CTRL_ADDR, &temp);
    if (temp & RTL8390_LED_GLB_CTRL_EXT_GPIO_EN_MASK)
        *pIsReady = 1;
    else
        *pIsReady = 0;
    RTL8231_MDC_SEM_UNLOCK(unit);
#endif

    return RT_ERR_CHIP_NOT_SUPPORTED;
} /* end of r8390_rtl8231_extra_devReady_get */

/* Function Name:
 *      r8390_rtl8231_extra_devEnable_get
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
r8390_rtl8231_extra_devEnable_get(uint32 unit, uint32 addr, rtk_enable_t *pEnable)
{
#if 0
    uint32  temp;

    /* Check init state */
    RT_INIT_CHK(rtl8231_init[unit]);

    RTL8231_MDC_SEM_LOCK(unit);
    ioal_mem32_read(unit, RTL8390_LED_GLB_CTRL_ADDR, &temp);
    if (temp & RTL8390_LED_GLB_CTRL_EXT_GPIO_EN_MASK)
        *pEnable = ENABLED;
    else
        *pEnable = DISABLED;
    RTL8231_MDC_SEM_UNLOCK(unit);
#endif

    return RT_ERR_CHIP_NOT_SUPPORTED;
} /* end of r8390_rtl8231_extra_devEnable_get */

/* Function Name:
 *      r8390_rtl8231_extra_devEnable_set
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
r8390_rtl8231_extra_devEnable_set(uint32 unit, uint32 addr, rtk_enable_t enable)
{
#if 0
    uint32  temp;

    /* Check init state */
    RT_INIT_CHK(rtl8231_init[unit]);

    RTL8231_MDC_SEM_LOCK(unit);
    ioal_mem32_read(unit, RTL8390_LED_GLB_CTRL_ADDR, &temp);
    temp &= ~(RTL8390_LED_GLB_CTRL_EXT_GPIO_EN_MASK);
    if (ENABLED == enable)
    {
        /* enable EXT_GPIO_EN[0] mapping to 8231_MDX controller(2). */
        temp |= (0x1 << RTL8390_LED_GLB_CTRL_EXT_GPIO_EN_OFFSET);
        /* enable EXT_GPIO_EN[1] mapping to 8231_MDX controller(3). */
        temp |= (0x2 << RTL8390_LED_GLB_CTRL_EXT_GPIO_EN_OFFSET);
    }
    ioal_mem32_write(unit, RTL8390_LED_GLB_CTRL_ADDR, temp);
    RTL8231_MDC_SEM_UNLOCK(unit);
#endif

    return RT_ERR_CHIP_NOT_SUPPORTED;
} /* end of r8390_rtl8231_extra_devEnable_set */

/* Function Name:
 *      r8390_rtl8231_extra_dataBit_get
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
r8390_rtl8231_extra_dataBit_get(uint32 unit, uint32 addr, uint32 gpioId, uint32 *pData)
{
#if 0
    uint32  temp;

    /* Check init state */
    RT_INIT_CHK(rtl8231_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(gpioId > 56, RT_ERR_INPUT);

    RTL8231_MDC_SEM_LOCK(unit);
    ioal_mem32_read(unit, RTL8390_EXT_GPIO_DATA_CTRL_ADDR(gpioId), &temp);
    (*pData) = (temp & RTL8390_EXT_GPIO_DATA_CTRL_EXT_GPIO_DATA_MASK(gpioId)) >> RTL8390_EXT_GPIO_DATA_CTRL_EXT_GPIO_DATA_OFFSET(gpioId);
    RTL8231_MDC_SEM_UNLOCK(unit);
#endif

    return RT_ERR_CHIP_NOT_SUPPORTED;
} /* end of r8390_rtl8231_extra_dataBit_get */

/* Function Name:
 *      r8390_rtl8231_extra_dataBit_set
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
r8390_rtl8231_extra_dataBit_set(uint32 unit, uint32 addr, uint32 gpioId, uint32 data)
{
#if 0
    uint32  temp;

    /* Check init state */
    RT_INIT_CHK(rtl8231_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(gpioId > 56, RT_ERR_INPUT);

    RTL8231_MDC_SEM_LOCK(unit);
    ioal_mem32_read(unit, RTL8390_EXT_GPIO_DATA_CTRL_ADDR(gpioId), &temp);
    temp = (temp & ~(1 << (gpioId % 32))) | (data << (gpioId % 32));
    ioal_mem32_write(unit, RTL8390_EXT_GPIO_DATA_CTRL_ADDR(gpioId), temp);
    RTL8231_MDC_SEM_UNLOCK(unit);
#endif

    return RT_ERR_CHIP_NOT_SUPPORTED;
} /* end of r8390_rtl8231_extra_dataBit_set */

/* Function Name:
 *      r8390_rtl8231_extra_direction_get
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
r8390_rtl8231_extra_direction_get(uint32 unit, uint32 addr, uint32 gpioId, drv_gpio_direction_t *pData)
{
#if 0
    uint32  temp;

    /* Check init state */
    RT_INIT_CHK(rtl8231_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(gpioId > 56, RT_ERR_INPUT);

    RTL8231_MDC_SEM_LOCK(unit);
    ioal_mem32_read(unit, RTL8390_EXT_GPIO_DIR_CTRL_ADDR(gpioId), &temp);
    (*pData) = (temp & RTL8390_EXT_GPIO_DIR_CTRL_EXT_GPIO_DIR_MASK(gpioId)) >> RTL8390_EXT_GPIO_DIR_CTRL_EXT_GPIO_DIR_OFFSET(gpioId);
    RTL8231_MDC_SEM_UNLOCK(unit);
#endif

    return RT_ERR_CHIP_NOT_SUPPORTED;
} /* end of r8390_rtl8231_extra_direction_get */

/* Function Name:
 *      r8390_rtl8231_extra_direction_set
 * Description:
 *      Set the external GPIO pin direction in the specified device of the unit
 * Input:
 *      unit   - unit id
 *      addr   - external GPIO address
 *      gpioId - gpio id
 *      data  - the gpio pin direction
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
r8390_rtl8231_extra_direction_set(uint32 unit, uint32 addr, uint32 gpioId, drv_gpio_direction_t data)
{
#if 0
    uint32  temp, value;

    /* Check init state */
    RT_INIT_CHK(rtl8231_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(gpioId > 56, RT_ERR_INPUT);

    RTL8231_MDC_SEM_LOCK(unit);
    if (data == GPIO_DIR_IN)
        value = 0;
    else
        value = 1;

    ioal_mem32_read(unit, RTL8390_EXT_GPIO_DIR_CTRL_ADDR(gpioId), &temp);
    temp = (temp & ~(1 << (gpioId % 32))) | (value << (gpioId % 32));
    ioal_mem32_write(unit, RTL8390_EXT_GPIO_DIR_CTRL_ADDR(gpioId), temp);
    RTL8231_MDC_SEM_UNLOCK(unit);
#endif

    return RT_ERR_CHIP_NOT_SUPPORTED;
} /* end of r8390_rtl8231_extra_direction_set */

