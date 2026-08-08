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
 * $Revision: 44251 $
 * $Date: 2013-11-07 11:07:02 +0800 (Thu, 07 Nov 2013) $
 *
 * Purpose : Definition those public External GPIO routing APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           (1) External GPIO
 *
 */

/*  
 * Include Files 
 */
#include <common/error.h>
#include <drv/gpio/ext_gpio.h>
#include <drv/rtl8231/rtl8231.h>
#include <drv/swcore/chip.h>
#include <common/debug/rt_log.h>
#include <osal/sem.h>

/* 
 * Symbol Definition 
 */

/*
 * Data Declaration 
 */
typedef struct drv_extGpio_devConfEntry_s
{
    uint32  valid;
    drv_extGpio_devConf_t data;
	osal_mutex_t         extGpio_sem;
    uint32  regVal[31]; /* keep the shadow value of the RTL8231 register 0~30 */
} drv_extGpio_devConfEntry_t;

static drv_extGpio_devConfEntry_t extGpioConfEntry[RTK_MAX_NUM_OF_UNIT][EXT_GPIO_DEV_ID_END];

#define EXTGPIO_SEM_LOCK(unit, dev)    \
do {\
    if (osal_sem_mutex_take(extGpioConfEntry[unit][dev].extGpio_sem, OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_DAL|MOD_PORT), "semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)
#define EXTGPIO_SEM_UNLOCK(unit, dev)   \
do {\
    if (osal_sem_mutex_give(extGpioConfEntry[unit][dev].extGpio_sem) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_DAL|MOD_PORT), "semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)



/*
 * Macro Definition
 */
#define IS_EXTGPIO_UNIT_DEV_INVALID(unit, dev)  ((extGpioConfEntry[unit][dev].valid) ? 0 : 1)

/*
 * Function Declaration
 */
//static int32 drv_extGpio_reg_read(uint32 unit, uint32 dev, uint32 reg, uint32 *pData);
//static int32 drv_extGpio_reg_write(uint32 unit, uint32 dev, uint32 reg, uint32 data);

/*
 * Function Body
 */
int32
drv_extGpio_reg_read(uint32 unit, uint32 dev, uint32 reg, uint32 *pData)
{
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((unit >= RTK_MAX_NUM_OF_UNIT), RT_ERR_INPUT);
    RT_PARAM_CHK((dev >= EXT_GPIO_DEV_ID_END), RT_ERR_INPUT);
    RT_PARAM_CHK(IS_EXTGPIO_UNIT_DEV_INVALID(unit, dev), RT_ERR_INPUT);
    RT_PARAM_CHK((reg > 31), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);

    switch (extGpioConfEntry[unit][dev].data.access_mode)
    {
        case EXT_GPIO_ACCESS_MODE_I2C:
            RT_ERR_CHK(drv_rtl8231_i2c_read(unit, extGpioConfEntry[unit][dev].data.address, reg, pData), ret);
            break;
        case EXT_GPIO_ACCESS_MODE_MDC:
            RT_ERR_CHK(drv_rtl8231_mdc_read(unit, extGpioConfEntry[unit][dev].data.address, extGpioConfEntry[unit][dev].data.page, reg, pData), ret);
            break;
        case EXT_GPIO_ACCESS_MODE_EXTRA:
            RT_ERR_CHK(drv_rtl8231_extra_read(unit, extGpioConfEntry[unit][dev].data.address, reg, pData), ret);
            break;
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    return RT_ERR_OK;
} /* end of drv_extGpio_reg_read */

int32
drv_extGpio_reg_write(uint32 unit, uint32 dev, uint32 reg, uint32 data)
{
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((unit >= RTK_MAX_NUM_OF_UNIT), RT_ERR_INPUT);
    RT_PARAM_CHK((dev >= EXT_GPIO_DEV_ID_END), RT_ERR_INPUT);
    RT_PARAM_CHK(IS_EXTGPIO_UNIT_DEV_INVALID(unit, dev), RT_ERR_INPUT);
    RT_PARAM_CHK((reg > 31), RT_ERR_INPUT);

    switch (extGpioConfEntry[unit][dev].data.access_mode)
    {
        case EXT_GPIO_ACCESS_MODE_I2C:
            RT_ERR_CHK(drv_rtl8231_i2c_write(unit, extGpioConfEntry[unit][dev].data.address, reg, data), ret);
            break;
        case EXT_GPIO_ACCESS_MODE_MDC:
            RT_ERR_CHK(drv_rtl8231_mdc_write(unit, extGpioConfEntry[unit][dev].data.address, extGpioConfEntry[unit][dev].data.page, reg, data), ret);
            break;
        case EXT_GPIO_ACCESS_MODE_EXTRA:
            RT_ERR_CHK(drv_rtl8231_extra_write(unit, extGpioConfEntry[unit][dev].data.address, reg, data), ret);
            break;
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }
    /* keep the register value to shadow */
    extGpioConfEntry[unit][dev].regVal[reg] = data;
    return RT_ERR_OK;
} /* end of drv_extGpio_reg_write */

/* Function Name:
 *      drv_extGpio_devReady_get
 * Description:
 *      Get external GPIO ready status
 * Input:
 *      unit - unit id
 *      dev  - external GPIO dev id
 * Output:
 *      pIsReady - the device ready status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
drv_extGpio_devReady_get(uint32 unit, uint32 dev, uint32 *pIsReady)
{
    uint32  regVal = 0;
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((unit >= RTK_MAX_NUM_OF_UNIT), RT_ERR_INPUT);
    RT_PARAM_CHK((dev >= EXT_GPIO_DEV_ID_END), RT_ERR_INPUT);
    RT_PARAM_CHK(IS_EXTGPIO_UNIT_DEV_INVALID(unit, dev), RT_ERR_INPUT);

    switch (extGpioConfEntry[unit][dev].data.access_mode)
    {
        case EXT_GPIO_ACCESS_MODE_I2C:
        case EXT_GPIO_ACCESS_MODE_MDC:
            RT_ERR_CHK(drv_extGpio_reg_read(unit, dev, RTL8231_LED_FUNC1_ADDR, &regVal), ret);
            if (RTL8231_LED_FUNC1_READY == (regVal & RTL8231_LED_FUNC1_READY))
                *pIsReady = 1;
            else
                *pIsReady = 0;
            break;
        case EXT_GPIO_ACCESS_MODE_EXTRA:
            RT_ERR_CHK(drv_rtl8231_extra_devReady_get(unit, extGpioConfEntry[unit][dev].data.address, &regVal), ret);
            *pIsReady = regVal;
            break;
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    return RT_ERR_OK;
} /* end of drv_extGpio_devReady_get */

/* Function Name:
 *      drv_extGpio_dev_get
 * Description:
 *      Get the external GPIO settings in the specified device of the unit
 * Input:
 *      unit  - unit id
 *      dev   - external GPIO dev id
 * Output:
 *      pData - initialize configuration data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Get the access mode and relatived information from driver.
 */
int32
drv_extGpio_dev_get(uint32 unit, uint32 dev, drv_extGpio_devConf_t *pData)
{
    RT_PARAM_CHK((unit >= RTK_MAX_NUM_OF_UNIT), RT_ERR_INPUT);
    RT_PARAM_CHK((dev >= EXT_GPIO_DEV_ID_END), RT_ERR_INPUT);
    RT_PARAM_CHK(IS_EXTGPIO_UNIT_DEV_INVALID(unit, dev), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);

    if (extGpioConfEntry[unit][dev].valid == 1)
    {
        pData->access_mode = extGpioConfEntry[unit][dev].data.access_mode;
        pData->address = extGpioConfEntry[unit][dev].data.address;
        pData->page = extGpioConfEntry[unit][dev].data.page;
        return RT_ERR_OK;
    }

    return RT_ERR_FAILED;
} /* end of drv_extGpio_dev_get */

/* Function Name:
 *      drv_extGpio_dev_init
 * Description:
 *      Initialize the external GPIO in the specified device of the unit
 * Input:
 *      unit  - unit id
 *      dev   - external GPIO dev id
 *      pData - initialize configuration data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Specified the access mode and relatived information to driver.
 */
int32
drv_extGpio_dev_init(uint32 unit, uint32 dev, drv_extGpio_devConf_t *pData)
{
    uint32  i;
    int32   ret;

    RT_PARAM_CHK((unit >= RTK_MAX_NUM_OF_UNIT), RT_ERR_INPUT);
    RT_PARAM_CHK((dev >= EXT_GPIO_DEV_ID_END), RT_ERR_INPUT);
    //RT_PARAM_CHK(IS_EXTGPIO_UNIT_DEV_INVALID(unit, dev), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);

    extGpioConfEntry[unit][dev].data.access_mode = pData->access_mode;
    extGpioConfEntry[unit][dev].data.address = pData->address;
    extGpioConfEntry[unit][dev].data.page = pData->page;
    extGpioConfEntry[unit][dev].valid = 1;

	extGpioConfEntry[unit][dev].extGpio_sem = osal_sem_mutex_create();

    if (extGpioConfEntry[unit][dev].data.access_mode == EXT_GPIO_ACCESS_MODE_I2C || 
        extGpioConfEntry[unit][dev].data.access_mode == EXT_GPIO_ACCESS_MODE_MDC)
    {
        /* keep the register value to shadow */
        for (i=0; i<31; i++)
        {
            RT_ERR_CHK(drv_extGpio_reg_read(unit, dev, i, &extGpioConfEntry[unit][dev].regVal[i]), ret);
        }
    }

    return RT_ERR_OK;
} /* end of drv_extGpio_dev_init */

/* Function Name:
 *      drv_extGpio_devEnable_get
 * Description:
 *      Get the external GPIO status in the specified device of the unit
 * Input:
 *      unit - unit id
 *      dev  - external GPIO dev id
 * Output:
 *      pEnable - the buffer pointer of the status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
drv_extGpio_devEnable_get(uint32 unit, uint32 dev, rtk_enable_t *pEnable)
{
    uint32  regVal = 0;
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((unit >= RTK_MAX_NUM_OF_UNIT), RT_ERR_INPUT);
    RT_PARAM_CHK((dev >= EXT_GPIO_DEV_ID_END), RT_ERR_INPUT);
    RT_PARAM_CHK(IS_EXTGPIO_UNIT_DEV_INVALID(unit, dev), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    switch (extGpioConfEntry[unit][dev].data.access_mode)
    {
        case EXT_GPIO_ACCESS_MODE_I2C:
        case EXT_GPIO_ACCESS_MODE_MDC:
            RT_ERR_CHK(drv_extGpio_reg_read(unit, dev, RTL8231_LED_FUNC0_ADDR, &regVal), ret);
            *pEnable = (regVal & RTL8231_LED_FUNC0_LED_START_MASK) >> RTL8231_LED_FUNC0_LED_START_OFFSET;
            break;
        case EXT_GPIO_ACCESS_MODE_EXTRA:
            RT_ERR_CHK(drv_rtl8231_extra_devEnable_get(unit, extGpioConfEntry[unit][dev].data.address, &regVal), ret);
            *pEnable = regVal;
            break;
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    return RT_ERR_OK;
} /* end of drv_extGpio_devEnable_get */

/* Function Name:
 *      drv_extGpio_devEnable_set
 * Description:
 *      Set the external GPIO status in the specified device of the unit
 * Input:
 *      unit   - unit id
 *      dev    - external GPIO dev id
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
drv_extGpio_devEnable_set(uint32 unit, uint32 dev, rtk_enable_t enable)
{
    uint32  regVal = 0;
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((unit >= RTK_MAX_NUM_OF_UNIT), RT_ERR_INPUT);
    RT_PARAM_CHK((dev >= EXT_GPIO_DEV_ID_END), RT_ERR_INPUT);
    RT_PARAM_CHK(IS_EXTGPIO_UNIT_DEV_INVALID(unit, dev), RT_ERR_INPUT);
    RT_PARAM_CHK(enable >= RTK_ENABLE_END, RT_ERR_INPUT);

    switch (extGpioConfEntry[unit][dev].data.access_mode)
    {
        case EXT_GPIO_ACCESS_MODE_I2C:
        case EXT_GPIO_ACCESS_MODE_MDC:
            RT_ERR_CHK(drv_extGpio_reg_read(unit, dev, RTL8231_LED_FUNC0_ADDR, &regVal), ret);
            regVal = (regVal & ~RTL8231_LED_FUNC0_LED_START_MASK) | (enable << RTL8231_LED_FUNC0_LED_START_OFFSET);
            RT_ERR_CHK(drv_extGpio_reg_write(unit, dev, RTL8231_LED_FUNC0_ADDR, regVal), ret);
            break;
        case EXT_GPIO_ACCESS_MODE_EXTRA:
            RT_ERR_CHK(drv_rtl8231_extra_devEnable_set(unit, extGpioConfEntry[unit][dev].data.address, enable), ret);
            break;
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    return RT_ERR_OK;
} /* end of drv_extGpio_devEnable_set */

/* Function Name:
 *      drv_extGpio_syncEnable_get
 * Description:
 *      Get the external GPIO sync configuration status in the specified device of the unit
 * Input:
 *      unit - unit id
 *      dev  - external GPIO dev id
 * Output:
 *      pEnable - the buffer pointer of the sync configuration status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
drv_extGpio_syncEnable_get(uint32 unit, uint32 dev, rtk_enable_t *pEnable)
{
    uint32  regVal = 0;
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((unit >= RTK_MAX_NUM_OF_UNIT), RT_ERR_INPUT);
    RT_PARAM_CHK((dev >= EXT_GPIO_DEV_ID_END), RT_ERR_INPUT);
    RT_PARAM_CHK(IS_EXTGPIO_UNIT_DEV_INVALID(unit, dev), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    switch (extGpioConfEntry[unit][dev].data.access_mode)
    {
        case EXT_GPIO_ACCESS_MODE_I2C:
        case EXT_GPIO_ACCESS_MODE_MDC:
        case EXT_GPIO_ACCESS_MODE_EXTRA:
            RT_ERR_CHK(drv_extGpio_reg_read(unit, dev, RTL8231_LED_FUNC0_ADDR, &regVal), ret);
            *pEnable = (regVal & RTL8231_LED_FUNC0_EN_SYNC_GPIO_MASK) >> RTL8231_LED_FUNC0_EN_SYNC_GPIO_OFFSET;
            break;
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    return RT_ERR_OK;
} /* end of drv_extGpio_syncEnable_get */

/* Function Name:
 *      drv_extGpio_syncEnable_set
 * Description:
 *      Set the external GPIO sync configuration status in the specified device of the unit
 * Input:
 *      unit   - unit id
 *      dev    - external GPIO dev id
 *      enable - the sync configuration status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
drv_extGpio_syncEnable_set(uint32 unit, uint32 dev, rtk_enable_t enable)
{
    uint32  regVal = 0;
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((unit >= RTK_MAX_NUM_OF_UNIT), RT_ERR_INPUT);
    RT_PARAM_CHK((dev >= EXT_GPIO_DEV_ID_END), RT_ERR_INPUT);
    RT_PARAM_CHK(IS_EXTGPIO_UNIT_DEV_INVALID(unit, dev), RT_ERR_INPUT);
    RT_PARAM_CHK(enable >= RTK_ENABLE_END, RT_ERR_INPUT);

    switch (extGpioConfEntry[unit][dev].data.access_mode)
    {
        case EXT_GPIO_ACCESS_MODE_I2C:
        case EXT_GPIO_ACCESS_MODE_MDC:
        case EXT_GPIO_ACCESS_MODE_EXTRA:
            RT_ERR_CHK(drv_extGpio_reg_read(unit, dev, RTL8231_LED_FUNC0_ADDR, &regVal), ret);
            regVal = (regVal & ~RTL8231_LED_FUNC0_EN_SYNC_GPIO_MASK) | (enable << RTL8231_LED_FUNC0_EN_SYNC_GPIO_OFFSET);
            RT_ERR_CHK(drv_extGpio_reg_write(unit, dev, RTL8231_LED_FUNC0_ADDR, regVal), ret);
            break;
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    return RT_ERR_OK;
} /* end of drv_extGpio_syncEnable_set */

/* Function Name:
 *      drv_extGpio_syncStatus_get
 * Description:
 *      Get the external GPIO sync progress status in the specified device of the unit
 * Input:
 *      unit - unit id
 *      dev  - external GPIO dev id
 * Output:
 *      pData - the buffer pointer of the sync progress status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      The output value 1 means in progress and 0 means completed.
 */
int32
drv_extGpio_syncStatus_get(uint32 unit, uint32 dev, uint32 *pData)
{
    uint32  regVal = 0;
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((unit >= RTK_MAX_NUM_OF_UNIT), RT_ERR_INPUT);
    RT_PARAM_CHK((dev >= EXT_GPIO_DEV_ID_END), RT_ERR_INPUT);
    RT_PARAM_CHK(IS_EXTGPIO_UNIT_DEV_INVALID(unit, dev), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);

    switch (extGpioConfEntry[unit][dev].data.access_mode)
    {
        case EXT_GPIO_ACCESS_MODE_I2C:
        case EXT_GPIO_ACCESS_MODE_MDC:
        case EXT_GPIO_ACCESS_MODE_EXTRA:
            RT_ERR_CHK(drv_extGpio_reg_read(unit, dev, RTL8231_GPIO_CTRL2_ADDR, &regVal), ret);
            *pData = (regVal & RTL8231_GPIO_CTRL2_SYNC_GPIO_MASK) >> RTL8231_GPIO_CTRL2_SYNC_GPIO_OFFSET;
            break;
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    return RT_ERR_OK;
} /* end of drv_extGpio_syncStatus_get */

/* Function Name:
 *      drv_extGpio_sync_start
 * Description:
 *      Trigger the external GPIO sync progress in the specified device of the unit
 * Input:
 *      unit - unit id
 *      dev  - external GPIO dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
drv_extGpio_sync_start(uint32 unit, uint32 dev)
{
    uint32  regVal = 0;
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((unit >= RTK_MAX_NUM_OF_UNIT), RT_ERR_INPUT);
    RT_PARAM_CHK((dev >= EXT_GPIO_DEV_ID_END), RT_ERR_INPUT);
    RT_PARAM_CHK(IS_EXTGPIO_UNIT_DEV_INVALID(unit, dev), RT_ERR_INPUT);

    switch (extGpioConfEntry[unit][dev].data.access_mode)
    {
        case EXT_GPIO_ACCESS_MODE_I2C:
        case EXT_GPIO_ACCESS_MODE_MDC:
        case EXT_GPIO_ACCESS_MODE_EXTRA:
            RT_ERR_CHK(drv_extGpio_reg_read(unit, dev, RTL8231_GPIO_CTRL2_ADDR, &regVal), ret);
            regVal = (regVal | RTL8231_LED_FUNC0_EN_SYNC_GPIO_MASK);
            RT_ERR_CHK(drv_extGpio_reg_write(unit, dev, RTL8231_GPIO_CTRL2_ADDR, regVal), ret);
            break;
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    return RT_ERR_OK;
} /* end of drv_extGpio_sync_start */

/* Function Name:
 *      drv_extGpio_pin_get
 * Description:
 *      Get the external GPIO pin function settings in the specified device of the unit
 * Input:
 *      unit   - unit id
 *      dev    - external GPIO dev id
 *      gpioId - gpio id
 * Output:
 *      pData  - the gpio pin configuration data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
drv_extGpio_pin_get(uint32 unit, uint32 dev, uint32 gpioId, drv_extGpio_conf_t *pData)
{
    uint32  regVal = 0, sel_gpio = 0;
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((unit >= RTK_MAX_NUM_OF_UNIT), RT_ERR_INPUT);
    RT_PARAM_CHK((dev >= EXT_GPIO_DEV_ID_END), RT_ERR_INPUT);
    RT_PARAM_CHK(IS_EXTGPIO_UNIT_DEV_INVALID(unit, dev), RT_ERR_INPUT);
    RT_PARAM_CHK((gpioId >= EXT_GPIO_ID_END), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);

    switch (extGpioConfEntry[unit][dev].data.access_mode)
    {
        case EXT_GPIO_ACCESS_MODE_I2C:
        case EXT_GPIO_ACCESS_MODE_MDC:
        case EXT_GPIO_ACCESS_MODE_EXTRA:
            /* GPIO Selection */
            RT_ERR_CHK(drv_extGpio_reg_read(unit, dev, RTL8231_GPIO_PIN_SEL_ADDR(gpioId), &regVal), ret);
            sel_gpio = (regVal >> (gpioId % 16)) & 0x1;
            if (sel_gpio)
            {
                /* Direction */
                if (gpioId < EXT_GPIO_ID32)
                {
                    RT_ERR_CHK(drv_extGpio_reg_read(unit, dev, RTL8231_GPIO_IO_SEL_ADDR(gpioId), &regVal), ret);
                    if ((regVal >> (gpioId % 16)) & 0x1)
                        pData->direction = GPIO_DIR_IN;
                    else
                        pData->direction = GPIO_DIR_OUT;
                }
                else
                {
                    RT_ERR_CHK(drv_extGpio_reg_read(unit, dev, RTL8231_GPIO_PIN_SEL2_ADDR, &regVal), ret);
                    if ((regVal >> ((gpioId % 32) + RTL8231_GPIO_PIN_SEL2_IOMASK_32_OFFSET)) & 0x1)
                        pData->direction = GPIO_DIR_IN;
                    else
                        pData->direction = GPIO_DIR_OUT;
                }
                /* Debounce */
                if (gpioId >= EXT_GPIO_ID31)
                {
                    RT_ERR_CHK(drv_extGpio_reg_read(unit, dev, RTL8231_LED_FUNC1_ADDR, &regVal), ret);
                    if ((regVal >> ((gpioId % 31) + RTL8231_LED_FUNC1_EN_DEBOUNCING_31_OFFSET)) & 0x1)
                        pData->debounce = 1;
                    else
                        pData->debounce = 0;
                }
                else
                {
                    pData->debounce = 0;
                }
                /* Inverter */
                if (gpioId < EXT_GPIO_ID32)
                {
                    RT_ERR_CHK(drv_extGpio_reg_read(unit, dev, RTL8231_GPIO_INV_SEL_ADDR(gpioId), &regVal), ret);
                    if ((regVal >> (gpioId % 16)) & 0x1)
                        pData->inverter = 1;
                    else
                        pData->inverter = 0;
                }
                else
                {
                    RT_ERR_CHK(drv_extGpio_reg_read(unit, dev, RTL8231_GPIO_PIN_SEL2_ADDR, &regVal), ret);
                    if ((regVal >> ((gpioId % 32) + RTL8231_GPIO_PIN_SEL2_INVMASK_32_OFFSET)) & 0x1)
                        pData->inverter = 1;
                    else
                        pData->inverter = 0;
                }
            }
            else
            {
                return RT_ERR_FAILED;
            }
            break;
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    return RT_ERR_OK;
} /* end of drv_extGpio_pin_get */

/* Function Name:
 *      drv_extGpio_pin_init
 * Description:
 *      Initialize the external GPIO pin function in the specified device of the unit
 * Input:
 *      unit   - unit id
 *      dev    - external GPIO dev id
 *      gpioId - gpio id
 *      pData  - the gpio pin configuration data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
drv_extGpio_pin_init(uint32 unit, uint32 dev, uint32 gpioId, drv_extGpio_conf_t *pData)
{
    uint32  regVal = 0, value;
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((unit >= RTK_MAX_NUM_OF_UNIT), RT_ERR_INPUT);
    RT_PARAM_CHK((dev >= EXT_GPIO_DEV_ID_END), RT_ERR_INPUT);
    RT_PARAM_CHK(IS_EXTGPIO_UNIT_DEV_INVALID(unit, dev), RT_ERR_INPUT);
    RT_PARAM_CHK((gpioId >= EXT_GPIO_ID_END), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);

    switch (extGpioConfEntry[unit][dev].data.access_mode)
    {
        case EXT_GPIO_ACCESS_MODE_I2C:
        case EXT_GPIO_ACCESS_MODE_MDC:
        case EXT_GPIO_ACCESS_MODE_EXTRA:
            /* GPIO Selection */
            RT_ERR_CHK(drv_extGpio_reg_read(unit, dev, RTL8231_GPIO_PIN_SEL_ADDR(gpioId), &regVal), ret);
            regVal = (regVal | (1 << (gpioId % 16)));
            RT_ERR_CHK(drv_extGpio_reg_write(unit, dev, RTL8231_GPIO_PIN_SEL_ADDR(gpioId), regVal), ret);

            /* Direction */
            if (pData->direction == GPIO_DIR_IN)
                value = 1;
            else
                value = 0;

            if (gpioId < EXT_GPIO_ID32)
            {
                RT_ERR_CHK(drv_extGpio_reg_read(unit, dev, RTL8231_GPIO_IO_SEL_ADDR(gpioId), &regVal), ret);
                regVal = (regVal & ~(1 << (gpioId % 16))) | (value << (gpioId % 16));
                RT_ERR_CHK(drv_extGpio_reg_write(unit, dev, RTL8231_GPIO_IO_SEL_ADDR(gpioId), regVal), ret);
            }
            else
            {
                RT_ERR_CHK(drv_extGpio_reg_read(unit, dev, RTL8231_GPIO_PIN_SEL2_ADDR, &regVal), ret);
                regVal = (regVal & ~(1 << ((gpioId % 32) + RTL8231_GPIO_PIN_SEL2_IOMASK_32_OFFSET))) | (value << ((gpioId % 32) + RTL8231_GPIO_PIN_SEL2_IOMASK_32_OFFSET));
                RT_ERR_CHK(drv_extGpio_reg_write(unit, dev, RTL8231_GPIO_PIN_SEL2_ADDR, regVal), ret);
            }

            /* Debounce */
            if (gpioId >= EXT_GPIO_ID31)
            {
                RT_ERR_CHK(drv_extGpio_reg_read(unit, dev, RTL8231_LED_FUNC1_ADDR, &regVal), ret);
                regVal = (regVal & ~(1 << ((gpioId % 31) + RTL8231_LED_FUNC1_EN_DEBOUNCING_31_OFFSET))) | (pData->debounce << ((gpioId % 31) + RTL8231_LED_FUNC1_EN_DEBOUNCING_31_OFFSET));
                RT_ERR_CHK(drv_extGpio_reg_write(unit, dev, RTL8231_LED_FUNC1_ADDR, regVal), ret);
            }

            /* Inverter */
            if (gpioId < EXT_GPIO_ID32)
            {
                RT_ERR_CHK(drv_extGpio_reg_read(unit, dev, RTL8231_GPIO_INV_SEL_ADDR(gpioId), &regVal), ret);
                regVal = (regVal & ~(1 << (gpioId % 16))) | (pData->inverter << (gpioId % 16));
                RT_ERR_CHK(drv_extGpio_reg_write(unit, dev, RTL8231_GPIO_INV_SEL_ADDR(gpioId), regVal), ret);
            }
            else
            {
                RT_ERR_CHK(drv_extGpio_reg_read(unit, dev, RTL8231_GPIO_PIN_SEL2_ADDR, &regVal), ret);
                regVal = (regVal & ~(1 << ((gpioId % 32) + RTL8231_GPIO_PIN_SEL2_INVMASK_32_OFFSET))) | (pData->inverter << ((gpioId % 32) + RTL8231_GPIO_PIN_SEL2_INVMASK_32_OFFSET));
                RT_ERR_CHK(drv_extGpio_reg_write(unit, dev, RTL8231_GPIO_PIN_SEL2_ADDR, regVal), ret);
            }
            break;
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    return RT_ERR_OK;
} /* end of drv_extGpio_pin_init */

/* Function Name:
 *      drv_extGpio_dataBit_get
 * Description:
 *      Get the external GPIO pin value in the specified device of the unit
 * Input:
 *      unit   - unit id
 *      dev    - external GPIO dev id
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
drv_extGpio_dataBit_get(uint32 unit, uint32 dev, uint32 gpioId, uint32 *pData)
{
    uint32  regVal = 0, chipId, chipRevId;
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((unit >= RTK_MAX_NUM_OF_UNIT), RT_ERR_INPUT);
    RT_PARAM_CHK((dev >= EXT_GPIO_DEV_ID_END), RT_ERR_INPUT);
    RT_PARAM_CHK(IS_EXTGPIO_UNIT_DEV_INVALID(unit, dev), RT_ERR_INPUT);
    if ((ret = drv_swcore_cid_get(unit, &chipId, &chipRevId)) != RT_ERR_OK)
        return RT_ERR_FAILED;

    if (CHIP_FAMILY_IS_RTL8350(chipId) || CHIP_FAMILY_IS_RTL8390(chipId))
        RT_PARAM_CHK((gpioId > 56), RT_ERR_INPUT);
    else
        RT_PARAM_CHK((gpioId >= EXT_GPIO_ID_END), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);

	EXTGPIO_SEM_LOCK(unit, dev);

    switch (extGpioConfEntry[unit][dev].data.access_mode)
    {
        case EXT_GPIO_ACCESS_MODE_I2C:
        case EXT_GPIO_ACCESS_MODE_MDC:
//            RT_ERR_CHK(drv_extGpio_reg_read(unit, dev, RTL8231_GPIO_CTRL_ADDR(gpioId), &regVal), ret);
            ret = drv_extGpio_reg_read(unit, dev, RTL8231_GPIO_CTRL_ADDR(gpioId), &regVal);
            if(ret != RT_ERR_OK)
            {
                EXTGPIO_SEM_UNLOCK(unit, dev);
                return ret;
            }            
            *pData = (regVal & (1 << (gpioId % 16))) >> (gpioId % 16);
            break;
        case EXT_GPIO_ACCESS_MODE_EXTRA:
            RT_ERR_CHK(drv_rtl8231_extra_dataBit_get(unit, dev, gpioId, &regVal), ret);
            *pData = regVal;
            break;
        default:
			EXTGPIO_SEM_UNLOCK(unit, dev);
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }

	EXTGPIO_SEM_UNLOCK(unit, dev);
    return RT_ERR_OK;
} /* end of drv_extGpio_dataBit_get */

/* Function Name:
 *      drv_extGpio_dataBit_set
 * Description:
 *      Set the external GPIO pin value in the specified device of the unit
 * Input:
 *      unit   - unit id
 *      dev    - external GPIO dev id
 *      gpioId - gpio id
 *      data  - the gpio pin value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
drv_extGpio_dataBit_set(uint32 unit, uint32 dev, uint32 gpioId, uint32 data)
{
    uint32  regVal = 0, chipId, chipRevId;
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((unit >= RTK_MAX_NUM_OF_UNIT), RT_ERR_INPUT);
    RT_PARAM_CHK((dev >= EXT_GPIO_DEV_ID_END), RT_ERR_INPUT);
    RT_PARAM_CHK(IS_EXTGPIO_UNIT_DEV_INVALID(unit, dev), RT_ERR_INPUT);
    if ((ret = drv_swcore_cid_get(unit, &chipId, &chipRevId)) != RT_ERR_OK)
        return RT_ERR_FAILED;

    if (CHIP_FAMILY_IS_RTL8350(chipId) || CHIP_FAMILY_IS_RTL8390(chipId))
        RT_PARAM_CHK((gpioId > 56), RT_ERR_INPUT);
    else
        RT_PARAM_CHK((gpioId >= EXT_GPIO_ID_END), RT_ERR_INPUT);

	EXTGPIO_SEM_LOCK(unit, dev);

    switch (extGpioConfEntry[unit][dev].data.access_mode)
    {
        case EXT_GPIO_ACCESS_MODE_I2C:
        case EXT_GPIO_ACCESS_MODE_MDC:
            //RT_ERR_CHK(drv_extGpio_reg_read(unit, dev, RTL8231_GPIO_CTRL_ADDR(gpioId), &regVal), ret);
            ret = drv_extGpio_reg_read(unit, dev, RTL8231_GPIO_CTRL_ADDR(gpioId), &regVal);
            if(ret != RT_ERR_OK)
            {
                EXTGPIO_SEM_UNLOCK(unit, dev);
                return ret;
            }               
            regVal = (regVal & ~(1 << (gpioId % 16))) | (data << (gpioId % 16));
//            RT_ERR_CHK(drv_extGpio_reg_write(unit, dev, RTL8231_GPIO_CTRL_ADDR(gpioId), regVal), ret);
            ret = drv_extGpio_reg_write(unit, dev, RTL8231_GPIO_CTRL_ADDR(gpioId), regVal);
            if(ret != RT_ERR_OK)
            {
                EXTGPIO_SEM_UNLOCK(unit, dev);
                return ret;
            }             
            break;
        case EXT_GPIO_ACCESS_MODE_EXTRA:
            RT_ERR_CHK(drv_rtl8231_extra_dataBit_set(unit, dev, gpioId, data), ret);
            break;
        default:
			EXTGPIO_SEM_UNLOCK(unit, dev);
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }

	EXTGPIO_SEM_UNLOCK(unit, dev);

    return RT_ERR_OK;
} /* end of drv_extGpio_dataBit_set */

/* Function Name:
 *      drv_extGpio_devRecovery_start
 * Description:
 *      Recovery the external GPIO status in the specified device of the unit
 * Input:
 *      unit   - unit id
 *      dev    - external GPIO dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
drv_extGpio_devRecovery_start(uint32 unit, uint32 dev)
{
    uint32  regVal = 0, i;
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((unit >= RTK_MAX_NUM_OF_UNIT), RT_ERR_INPUT);
    RT_PARAM_CHK((dev >= EXT_GPIO_DEV_ID_END), RT_ERR_INPUT);

    for (i=1; i<31; i++)
    {
        RT_ERR_CHK(drv_extGpio_reg_write(unit, dev, i, extGpioConfEntry[unit][dev].regVal[i]), ret);
    }

    /* Recovery the device to active */
    regVal = (extGpioConfEntry[unit][dev].regVal[0] & ~RTL8231_LED_FUNC0_LED_START_MASK) | (1 << RTL8231_LED_FUNC0_LED_START_OFFSET);
    RT_ERR_CHK(drv_extGpio_reg_write(unit, dev, RTL8231_LED_FUNC0_ADDR, regVal), ret);

    return RT_ERR_OK;
} /* end of drv_extGpio_devRecovery_start */

/* Function Name:
 *      drv_extGpio_direction_get
 * Description:
 *      Get the external GPIO pin direction in the specified device of the unit
 * Input:
 *      unit   - unit id
 *      dev    - external GPIO dev id
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
drv_extGpio_direction_get(uint32 unit, uint32 dev, uint32 gpioId, drv_gpio_direction_t *pData)
{
    uint32  sel_gpio = 0;
    uint32  regVal = 0, chipId, chipRevId;
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((unit >= RTK_MAX_NUM_OF_UNIT), RT_ERR_INPUT);
    RT_PARAM_CHK((dev >= EXT_GPIO_DEV_ID_END), RT_ERR_INPUT);
    RT_PARAM_CHK(IS_EXTGPIO_UNIT_DEV_INVALID(unit, dev), RT_ERR_INPUT);
    if ((ret = drv_swcore_cid_get(unit, &chipId, &chipRevId)) != RT_ERR_OK)
        return RT_ERR_FAILED;

    if (CHIP_FAMILY_IS_RTL8350(chipId) || CHIP_FAMILY_IS_RTL8390(chipId))
        RT_PARAM_CHK((gpioId > 56), RT_ERR_INPUT);
    else
        RT_PARAM_CHK((gpioId >= EXT_GPIO_ID_END), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);

    switch (extGpioConfEntry[unit][dev].data.access_mode)
    {
        case EXT_GPIO_ACCESS_MODE_MDC:
            /* GPIO Selection */
            RT_ERR_CHK(drv_extGpio_reg_read(unit, dev, RTL8231_GPIO_PIN_SEL_ADDR(gpioId), &regVal), ret);
            sel_gpio = (regVal >> (gpioId % 16)) & 0x1;
            if (sel_gpio)
            {
                /* Direction */
                if (gpioId < EXT_GPIO_ID32)
                {
                    RT_ERR_CHK(drv_extGpio_reg_read(unit, dev, RTL8231_GPIO_IO_SEL_ADDR(gpioId), &regVal), ret);
                    if ((regVal >> (gpioId % 16)) & 0x1)
                        *pData = GPIO_DIR_IN;
                    else
                        *pData = GPIO_DIR_OUT;
                }
                else
                {
                    RT_ERR_CHK(drv_extGpio_reg_read(unit, dev, RTL8231_GPIO_PIN_SEL2_ADDR, &regVal), ret);
                    if ((regVal >> ((gpioId % 32) + RTL8231_GPIO_PIN_SEL2_IOMASK_32_OFFSET)) & 0x1)
                        *pData = GPIO_DIR_IN;
                    else
                        *pData = GPIO_DIR_OUT;
                }
            }
            else
            {
                return RT_ERR_FAILED;
            }
            break;
        case EXT_GPIO_ACCESS_MODE_EXTRA:
            RT_ERR_CHK(drv_rtl8231_extra_direction_get(unit, dev, gpioId, &regVal), ret);
            *pData = regVal;
            break;
        case EXT_GPIO_ACCESS_MODE_I2C:
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    return RT_ERR_OK;
} /* end of drv_extGpio_direction_get */

/* Function Name:
 *      drv_extGpio_direction_set
 * Description:
 *      Set the external GPIO pin direction in the specified device of the unit
 * Input:
 *      unit   - unit id
 *      dev    - external GPIO dev id
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
drv_extGpio_direction_set(uint32 unit, uint32 dev, uint32 gpioId, drv_gpio_direction_t data)
{
    uint32  regVal = 0, value;
    uint32  chipId, chipRevId;
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((unit >= RTK_MAX_NUM_OF_UNIT), RT_ERR_INPUT);
    RT_PARAM_CHK((dev >= EXT_GPIO_DEV_ID_END), RT_ERR_INPUT);
    RT_PARAM_CHK(IS_EXTGPIO_UNIT_DEV_INVALID(unit, dev), RT_ERR_INPUT);
    if ((ret = drv_swcore_cid_get(unit, &chipId, &chipRevId)) != RT_ERR_OK)
        return RT_ERR_FAILED;

    if (CHIP_FAMILY_IS_RTL8350(chipId) || CHIP_FAMILY_IS_RTL8390(chipId))
        RT_PARAM_CHK((gpioId > 56), RT_ERR_INPUT);
    else
        RT_PARAM_CHK((gpioId >= EXT_GPIO_ID_END), RT_ERR_INPUT);

    switch (extGpioConfEntry[unit][dev].data.access_mode)
    {
        case EXT_GPIO_ACCESS_MODE_MDC:
            /* GPIO Selection */
            RT_ERR_CHK(drv_extGpio_reg_read(unit, dev, RTL8231_GPIO_PIN_SEL_ADDR(gpioId), &regVal), ret);
            regVal = (regVal | (1 << (gpioId % 16)));
            RT_ERR_CHK(drv_extGpio_reg_write(unit, dev, RTL8231_GPIO_PIN_SEL_ADDR(gpioId), regVal), ret);

            /* Direction */
            if (data == GPIO_DIR_IN)
                value = 1;
            else
                value = 0;

            if (gpioId < EXT_GPIO_ID32)
            {
                RT_ERR_CHK(drv_extGpio_reg_read(unit, dev, RTL8231_GPIO_IO_SEL_ADDR(gpioId), &regVal), ret);
                regVal = (regVal & ~(1 << (gpioId % 16))) | (value << (gpioId % 16));
                RT_ERR_CHK(drv_extGpio_reg_write(unit, dev, RTL8231_GPIO_IO_SEL_ADDR(gpioId), regVal), ret);
            }
            else
            {
                RT_ERR_CHK(drv_extGpio_reg_read(unit, dev, RTL8231_GPIO_PIN_SEL2_ADDR, &regVal), ret);
                regVal = (regVal & ~(1 << ((gpioId % 32) + RTL8231_GPIO_PIN_SEL2_IOMASK_32_OFFSET))) | (value << ((gpioId % 32) + RTL8231_GPIO_PIN_SEL2_IOMASK_32_OFFSET));
                RT_ERR_CHK(drv_extGpio_reg_write(unit, dev, RTL8231_GPIO_PIN_SEL2_ADDR, regVal), ret);
            }
            break;
        case EXT_GPIO_ACCESS_MODE_EXTRA:
            RT_ERR_CHK(drv_rtl8231_extra_direction_set(unit, dev, gpioId, data), ret);
            break;
        case EXT_GPIO_ACCESS_MODE_I2C:
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    return RT_ERR_OK;
} /* end of drv_extGpio_direction_set */

/* Function Name:
 *      drv_extGpio_i2c_init
 * Description:
 *      Initialize the SCK/SDA in external GPIO pin in the specified device of the unit
 * Input:
 *      unit      - unit id
 *      dev       - external GPIO dev id
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
drv_extGpio_i2c_init(uint32 unit, uint32 dev, uint32 i2c_clock, uint32 i2c_data)
{
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((unit >= RTK_MAX_NUM_OF_UNIT), RT_ERR_INPUT);
    RT_PARAM_CHK((dev >= EXT_GPIO_DEV_ID_END), RT_ERR_INPUT);
    RT_PARAM_CHK(IS_EXTGPIO_UNIT_DEV_INVALID(unit, dev), RT_ERR_INPUT);
    RT_PARAM_CHK((i2c_clock >= EXT_GPIO_ID_END), RT_ERR_INPUT);
    RT_PARAM_CHK((i2c_data >= EXT_GPIO_ID_END), RT_ERR_INPUT);

    switch (extGpioConfEntry[unit][dev].data.access_mode)
    {
        case EXT_GPIO_ACCESS_MODE_EXTRA:
            RT_ERR_CHK(drv_rtl8231_extra_i2c_init(unit, dev, i2c_clock, i2c_data), ret);
            break;
        case EXT_GPIO_ACCESS_MODE_I2C:
        case EXT_GPIO_ACCESS_MODE_MDC:
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    return RT_ERR_OK;
} /* end of drv_extGpio_i2c_init */

/* Function Name:
 *      drv_extGpio_i2c_get
 * Description:
 *      Get the SCK/SDA in external GPIO pin in the specified device of the unit
 * Input:
 *      unit       - unit id
 *      dev        - external GPIO dev id
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
drv_extGpio_i2c_get(uint32 unit, uint32 dev, uint32 *pI2c_clock, uint32 *pI2c_data)
{
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((unit >= RTK_MAX_NUM_OF_UNIT), RT_ERR_INPUT);
    RT_PARAM_CHK((dev >= EXT_GPIO_DEV_ID_END), RT_ERR_INPUT);
    RT_PARAM_CHK(IS_EXTGPIO_UNIT_DEV_INVALID(unit, dev), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pI2c_clock), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pI2c_data), RT_ERR_NULL_POINTER);

    switch (extGpioConfEntry[unit][dev].data.access_mode)
    {
        case EXT_GPIO_ACCESS_MODE_EXTRA:
            RT_ERR_CHK(drv_rtl8231_extra_i2c_get(unit, dev, pI2c_clock, pI2c_data), ret);
            break;
        case EXT_GPIO_ACCESS_MODE_I2C:
        case EXT_GPIO_ACCESS_MODE_MDC:
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    return RT_ERR_OK;
} /* end of drv_extGpio_i2c_get */

/* Function Name:
 *      drv_extGpio_i2c_read
 * Description:
 *      Read the value of register in external GPIO pin in the specified device of the unit
 * Input:
 *      unit  - unit id
 *      dev   - external GPIO dev id
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
drv_extGpio_i2c_read(uint32 unit, uint32 dev, uint32 reg, uint32 *pData)
{
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((unit >= RTK_MAX_NUM_OF_UNIT), RT_ERR_INPUT);
    RT_PARAM_CHK((dev >= EXT_GPIO_DEV_ID_END), RT_ERR_INPUT);
    RT_PARAM_CHK(IS_EXTGPIO_UNIT_DEV_INVALID(unit, dev), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);

    switch (extGpioConfEntry[unit][dev].data.access_mode)
    {
        case EXT_GPIO_ACCESS_MODE_EXTRA:
            RT_ERR_CHK(drv_rtl8231_extra_i2c_read(unit, dev, reg, pData), ret);
            break;
        case EXT_GPIO_ACCESS_MODE_I2C:
        case EXT_GPIO_ACCESS_MODE_MDC:
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    return RT_ERR_OK;
} /* end of drv_extGpio_i2c_read */

/* Function Name:
 *      drv_extGpio_i2c_write
 * Description:
 *      Write the value of register in external GPIO pin in the specified device of the unit
 * Input:
 *      unit - unit id
 *      dev  - external GPIO dev id
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
drv_extGpio_i2c_write(uint32 unit, uint32 dev, uint32 reg, uint32 data)
{
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((unit >= RTK_MAX_NUM_OF_UNIT), RT_ERR_INPUT);
    RT_PARAM_CHK((dev >= EXT_GPIO_DEV_ID_END), RT_ERR_INPUT);
    RT_PARAM_CHK(IS_EXTGPIO_UNIT_DEV_INVALID(unit, dev), RT_ERR_INPUT);

    switch (extGpioConfEntry[unit][dev].data.access_mode)
    {
        case EXT_GPIO_ACCESS_MODE_EXTRA:
            RT_ERR_CHK(drv_rtl8231_extra_i2c_write(unit, dev, reg, data), ret);
            break;
        case EXT_GPIO_ACCESS_MODE_I2C:
        case EXT_GPIO_ACCESS_MODE_MDC:
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    return RT_ERR_OK;
} /* end of drv_extGpio_i2c_write */

