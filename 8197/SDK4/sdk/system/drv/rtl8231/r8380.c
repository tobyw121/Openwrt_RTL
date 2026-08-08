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
#include <drv/gpio/gpio.h>
#include <drv/gpio/ext_gpio.h>
#include <drv/rtl8231/r8380.h>
#include <drv/swcore/rtl8380.h>
#include <hal/common/halctrl.h>

/*
 * Symbol Definition
 */
#define CHECKBUSY_TIMES (3000)
#define EXTRA_GPIO_DEV_ADDR (0)

/*
 * Data Type Definition
 */
typedef enum GPIO_PIN_E
{  
    GPIO_A7 = 31,
    GPIO_A6 = 30,
    GPIO_A5 = 29,
    GPIO_A4 = 28,
    GPIO_A3 = 27,
    GPIO_A2 = 26,
    GPIO_A1 = 25,
    GPIO_A0 = 24,
    GPIO_B7 = 23,
    GPIO_B6 = 22,
    GPIO_B5 = 21,
    GPIO_B4 = 20,
    GPIO_B3 = 19,
    GPIO_B2 = 18,
    GPIO_B1 = 17,
    GPIO_B0 = 16,
    GPIO_C7 = 15,
    GPIO_C6 = 14,
    GPIO_C5 = 13,
    GPIO_C4 = 12,
    GPIO_C3 = 11,
    GPIO_C2 = 10,
    GPIO_C1 = 9,
    GPIO_C0 = 8,
    GPIO_END
}gpio_pin_t;


/*
 * Data Declaration
 */
static uint32   rtl8231_init[RTK_MAX_NUM_OF_UNIT];
static rtl8231_mdcSem_cb_entry_t _rtl8231_mdcSem_cb_tbl[RTK_MAX_NUM_OF_UNIT];
static uint32   smi_MDC;         /* GPIO used for SMI Clock Generation */
static uint32   smi_MDIO;        /* GPIO used for SMI Data signal */
static uint32   pin_i2c_clock = EXT_GPIO_ID2;   /* external GPIO used for I2C Clock Generation */
static uint32   pin_i2c_data = EXT_GPIO_ID1;    /* external GPIO used for I2C Data signal */
static uint32   mdc_delay = 2, i2c_delay = 5;


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
        return RT_ERR_FAILED;\
    }\
}

#define MDC_DELAY   (mdc_delay) /* 2 */
#define I2C_DELAY   (i2c_delay) /* 5 */
#define CLK_DURATION(clk)   { int i; for(i=0; i<clk; i++); } 


/*
 * Function Declaration
 */
static int32 smiInit(uint32 pinMDC, uint32 pinMDIO);
static void smiWrite(uint32 phyad, uint32 regad, uint32 data);
static void smiRead(uint32 phyad, uint32 regad, uint32* pData);
static void _i2c_start(void);
static void _i2c_stop(void);
static void _i2c_readBits(uint32 *pData, uint32 bitLen);
static void _i2c_writeBits(uint32 data, uint32 bitLen);
static void _i2c_i2cPin_direction_set(uint32 pin, drv_gpio_direction_t dir);
static void _i2c_i2cPin_data_set(uint32 pin, uint32 data);
static void _i2c_i2cPin_data_get(uint32 pin, uint32 *pData);
static void _i2c_writeBit(uint32 bitData);
static void _i2c_readBit(uint32 *pBitData);
static void _i2c_read_1(uint32 reg, uint32 *pData);


/* Function Name:
 *      r8380_rtl8231_init
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
r8380_rtl8231_init(uint32 unit)
{
    uint32  temp;
    uint32 mode;
	
    rtl8231_init[unit] = INIT_NOT_COMPLETED;

    /* Add GPIO output driver current */
    ioal_mem32_read(unit, RTL8380_IO_DRIVING_ABILITY_CTRL_ADDR, &mode);
    mode |= (0x2UL);
    ioal_mem32_write(unit, RTL8380_IO_DRIVING_ABILITY_CTRL_ADDR, mode);

    /* Enable RTL8380 GPIO-Simulated mode instead of Indirect access RTL8231 mode */
    ioal_mem32_read(unit, RTL8380_EXTRA_GPIO_CTRL_ADDR, &mode);
    mode &= ~(0x1UL);
    ioal_mem32_write(unit, RTL8380_EXTRA_GPIO_CTRL_ADDR, mode);

    /* Set MDC Period to 2.6MHz from default Value:5.6MHz */
    ioal_mem32_read(unit, RTL8380_EXTRA_GPIO_CTRL_ADDR, &mode);
    mode &= ~(0x3UL<<8);
    ioal_mem32_write(unit, RTL8380_EXTRA_GPIO_CTRL_ADDR, mode);

    /* Initialize the MDC/MDIO pin that is board dependent */
    smiInit(GPIO_A2, GPIO_A3);

    /* Select all the PINs as GPIO */
    smiWrite(EXTRA_GPIO_DEV_ADDR, RTL8231_GPIO_PIN_SEL0_ADDR, 0xffff);
    smiWrite(EXTRA_GPIO_DEV_ADDR, RTL8231_GPIO_PIN_SEL1_ADDR, 0xffff);
    smiRead(EXTRA_GPIO_DEV_ADDR, RTL8231_GPIO_PIN_SEL2_ADDR, &temp);
    temp |= 0x1f;
    smiWrite(EXTRA_GPIO_DEV_ADDR, RTL8231_GPIO_PIN_SEL2_ADDR, temp);

    /* Set all the PINs Direction as GPI, Because GPO is not safe */
    smiRead(EXTRA_GPIO_DEV_ADDR, RTL8231_GPIO_PIN_SEL2_ADDR, &temp);
    temp |= 0x1f<<5;
    smiWrite(EXTRA_GPIO_DEV_ADDR, RTL8231_GPIO_PIN_SEL2_ADDR, temp);
    smiWrite(EXTRA_GPIO_DEV_ADDR, RTL8231_GPIO_IO_SEL0_ADDR, 0xffff);
    smiWrite(EXTRA_GPIO_DEV_ADDR, RTL8231_GPIO_IO_SEL1_ADDR, 0xffff);

    rtl8231_init[unit] = INIT_COMPLETED;

    return RT_ERR_OK;
} /* end of r8380_rtl8231_init */

/* Function Name:
 *      r8380_rtl8231_mdcSem_register
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
r8380_rtl8231_mdcSem_register(uint32 unit, drv_rtl8231_mdcSem_cb_f fMdcSemCb)
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
} /* end of r8380_rtl8231_mdcSem_register */

/* Function Name:
 *      r8380_rtl8231_mdcSem_unregister
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
r8380_rtl8231_mdcSem_unregister(uint32 unit, drv_rtl8231_mdcSem_cb_f fMdcSemCb)
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
} /* end of r8380_rtl8231_mdcSem_unregister */


/* Function Name:
 *      r8380_rtl8231_mdc_read
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
r8380_rtl8231_mdc_read(uint32 unit, uint32 phy_id, uint32 page, uint32 reg_addr, uint32 *pData)
{
    uint32 mode;
    uint32  temp;

    /* check Init status */
    RT_INIT_CHK(rtl8231_init[unit]);

    RTL8231_MDC_SEM_LOCK(unit);

    /* Enable RTL8380 Indirect access RTL8231 mode */
    ioal_mem32_read(unit, RTL8380_EXTRA_GPIO_CTRL_ADDR, &mode);
    mode |= 0x1UL;
    ioal_mem32_write(unit, RTL8380_EXTRA_GPIO_CTRL_ADDR, mode);

    ioal_mem32_read(unit, RTL8380_DMY_REG5_ADDR, &mode);
    mode &= ~0x3UL;
    mode |= 0x1UL;
    ioal_mem32_write(unit, RTL8380_DMY_REG5_ADDR, mode);

    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * If RWOP = 0(read), then INDATA[15:0] = {Reserved & PORT_ID[4:0]}
     * If RWOP = 1(write), then INDATA[15:0] = DATA[15:0]
     */
    temp |= ((phy_id << RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_PHY_ADDR_OFFSET) & RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_PHY_ADDR_MASK);

    /* Select register number to access */
    temp |= ((reg_addr << RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_REG_OFFSET) & RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_REG_MASK);

    /* Read/Write operation
     * 0b0: read
     * 0b1: write
     */
    temp |= ((GPIO_READ << RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_RWOP_OFFSET) & RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_RWOP_MASK);

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    temp |= ((1 << RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_CMD_OFFSET) & RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_CMD_MASK);

    /* write register to active the read operation */
    ioal_mem32_write(unit, RTL8380_EXT_GPIO_INDRT_ACCESS_ADDR, temp);

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    RTL8231_MDC_BUSY_WAIT_LOOP(unit, RTL8380_EXT_GPIO_INDRT_ACCESS_ADDR, 0x1);

    /* get the read operation result to temp */
    ioal_mem32_read(unit, RTL8380_EXT_GPIO_INDRT_ACCESS_ADDR, &temp);

    /* fill the DATA[15:0] from temp to pData */
    (*pData) = (temp & RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_DATA_MASK) >> RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_DATA_OFFSET;

    RTL8231_MDC_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
}

/* Function Name:
 *      r8380_rtl8231_mdc_write
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
r8380_rtl8231_mdc_write(uint32 unit, uint32 phy_id, uint32 page, uint32 reg_addr, uint32 data)
{
    uint32 mode;
    uint32  temp;

    /* check Init status */
    RT_INIT_CHK(rtl8231_init[unit]);

    RTL8231_MDC_SEM_LOCK(unit);

    /* Enable RTL8380 Indirect access RTL8231 mode */
    ioal_mem32_read(unit, RTL8380_EXTRA_GPIO_CTRL_ADDR, &mode);
    mode |= 0x1UL;
    ioal_mem32_write(unit, RTL8380_EXTRA_GPIO_CTRL_ADDR, mode);

    ioal_mem32_read(unit, RTL8380_DMY_REG5_ADDR, &mode);
    mode &= ~0x3UL;
    mode |= 0x1UL;
    ioal_mem32_write(unit, RTL8380_DMY_REG5_ADDR, mode);


    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * If RWOP = 0(read), then INDATA[15:0] = {Reserved & PORT_ID[4:0]}
     * If RWOP = 1(write), then INDATA[15:0] = DATA[15:0]
     */
    temp |= ((phy_id << RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_PHY_ADDR_OFFSET) & RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_PHY_ADDR_MASK);

    /* Select register number to access */
    temp |= ((reg_addr << RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_REG_OFFSET) & RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_REG_MASK);

    /* Read/Write operation
     * 0b0: read
     * 0b1: write
     */
    temp |= ((GPIO_WRITE << RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_RWOP_OFFSET) & RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_RWOP_MASK);

    /* Input parameters:
     * If RWOP = 0(read), then GPIO_DATA[15:0] = input data[15:0]
     * If RWOP = 1(write), then GPIO_DATA [15:0] = output data[15:0]
     */
    temp |= ((data << RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_DATA_OFFSET) & RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_DATA_MASK);
    
    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    temp |= ((1 << RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_CMD_OFFSET) & RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_CMD_MASK);

    /* write register to active the read operation */
    ioal_mem32_write(unit, RTL8380_EXT_GPIO_INDRT_ACCESS_ADDR, temp);

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    RTL8231_MDC_BUSY_WAIT_LOOP(unit, RTL8380_EXT_GPIO_INDRT_ACCESS_ADDR, 0x1);

    RTL8231_MDC_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
}



/* Function Name:
 *      r8380_rtl8231_extra_devReady_get
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
r8380_rtl8231_extra_devReady_get(uint32 unit, uint32 addr, uint32 *pIsReady)
{
    uint32  temp;
    uint32  mode;

    /* Check init state */
    RT_INIT_CHK(rtl8231_init[unit]);

    RTL8231_MDC_SEM_LOCK(unit);

    /* Enable RTL8380 GPIO-Simulated mode instead of Indirect access RTL8231 mode */
    ioal_mem32_read(unit, RTL8380_EXTRA_GPIO_CTRL_ADDR, &mode);
    mode &= ~(0x1UL);
    ioal_mem32_write(unit, RTL8380_EXTRA_GPIO_CTRL_ADDR, mode);
	
    /* for Software generate waveform */
    smiRead(EXTRA_GPIO_DEV_ADDR, RTL8231_LED_FUNC1_ADDR, &temp);
    if (RTL8231_LED_FUNC1_READY == (temp & RTL8231_LED_FUNC1_READY))
        *pIsReady = 1;
    else
        *pIsReady = 0;

	
    RTL8231_MDC_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of r8380_rtl8231_extra_devReady_get */

/* Function Name:
 *      r8380_rtl8231_extra_devEnable_get
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
r8380_rtl8231_extra_devEnable_get(uint32 unit, uint32 addr, rtk_enable_t *pEnable)
{
    uint32  temp;
    uint32  mode;
	
    /* Check init state */
    RT_INIT_CHK(rtl8231_init[unit]);

    RTL8231_MDC_SEM_LOCK(unit);

    /* Enable RTL8380 GPIO-Simulated mode instead of Indirect access RTL8231 mode */
    ioal_mem32_read(unit, RTL8380_EXTRA_GPIO_CTRL_ADDR, &mode);
    mode &= ~(0x1UL);
    ioal_mem32_write(unit, RTL8380_EXTRA_GPIO_CTRL_ADDR, mode);

	
    /* for Software generate waveform */
    smiRead(EXTRA_GPIO_DEV_ADDR, RTL8231_LED_FUNC0_ADDR, &temp);
    *pEnable = ((temp & RTL8231_LED_FUNC0_LED_START_MASK) >> RTL8231_LED_FUNC0_LED_START_OFFSET);
    RTL8231_MDC_SEM_UNLOCK(unit);


    return RT_ERR_OK;
} /* end of r8380_rtl8231_extra_devEnable_get */

/* Function Name:
 *      r8380_rtl8231_extra_devEnable_set
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
r8380_rtl8231_extra_devEnable_set(uint32 unit, uint32 addr, rtk_enable_t enable)
{
    uint32  temp;
    uint32  mode;
	
    /* Check init state */
    RT_INIT_CHK(rtl8231_init[unit]);

    RTL8231_MDC_SEM_LOCK(unit);

    /* Enable RTL8380 GPIO-Simulated mode instead of Indirect access RTL8231 mode */
    ioal_mem32_read(unit, RTL8380_EXTRA_GPIO_CTRL_ADDR, &mode);
    mode &= ~(0x1UL);
    ioal_mem32_write(unit, RTL8380_EXTRA_GPIO_CTRL_ADDR, mode);


    /* for Software generate waveform */
    smiRead(EXTRA_GPIO_DEV_ADDR, RTL8231_LED_FUNC0_ADDR, &temp);
    temp = (temp & ~RTL8231_LED_FUNC0_LED_START_MASK) | (enable << RTL8231_LED_FUNC0_LED_START_OFFSET);
    smiWrite(EXTRA_GPIO_DEV_ADDR, RTL8231_LED_FUNC0_ADDR, temp);
    RTL8231_MDC_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of r8380_rtl8231_extra_devEnable_set */

/* Function Name:
 *      r8380_rtl8231_extra_dataBit_get
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
r8380_rtl8231_extra_dataBit_get(uint32 unit, uint32 addr, uint32 gpioId, uint32 *pData)
{
    uint32  temp;
    uint32  mode;
	
    /* Check init state */
    RT_INIT_CHK(rtl8231_init[unit]);

    RTL8231_MDC_SEM_LOCK(unit);

    /* Enable RTL8380 GPIO-Simulated mode instead of Indirect access RTL8231 mode */
    ioal_mem32_read(unit, RTL8380_EXTRA_GPIO_CTRL_ADDR, &mode);
    mode &= ~(0x1UL);
    ioal_mem32_write(unit, RTL8380_EXTRA_GPIO_CTRL_ADDR, mode);

	
    /* for Software generate waveform */
    smiRead(EXTRA_GPIO_DEV_ADDR, RTL8231_GPIO_CTRL_ADDR(gpioId), &temp);
    if (temp & (1 << (gpioId % 16)))
        *pData = 1;
    else
        *pData = 0;
    RTL8231_MDC_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of r8380_rtl8231_extra_dataBit_get */

/* Function Name:
 *      r8380_rtl8231_extra_dataBit_set
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
r8380_rtl8231_extra_dataBit_set(uint32 unit, uint32 addr, uint32 gpioId, uint32 data)
{
    uint32  temp;
    uint32  mode;
	
    /* Check init state */
    RT_INIT_CHK(rtl8231_init[unit]);

    RTL8231_MDC_SEM_LOCK(unit);

    /* Enable RTL8380 GPIO-Simulated mode instead of Indirect access RTL8231 mode */
    ioal_mem32_read(unit, RTL8380_EXTRA_GPIO_CTRL_ADDR, &mode);
    mode &= ~(0x1UL);
    ioal_mem32_write(unit, RTL8380_EXTRA_GPIO_CTRL_ADDR, mode);

	
    /* for Software generate waveform */
    smiRead(EXTRA_GPIO_DEV_ADDR, RTL8231_GPIO_CTRL_ADDR(gpioId), &temp);
    temp = (temp & ~(1 << (gpioId % 16))) | (data << (gpioId % 16));
    smiWrite(EXTRA_GPIO_DEV_ADDR, RTL8231_GPIO_CTRL_ADDR(gpioId), temp);
    RTL8231_MDC_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of r8380_rtl8231_extra_dataBit_set */

/* Function Name:
 *      r8380_rtl8231_extra_direction_get
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
r8380_rtl8231_extra_direction_get(uint32 unit, uint32 addr, uint32 gpioId, drv_gpio_direction_t *pData)
{
    uint32  temp;
    uint32  mode;
	
    /* Check init state */
    RT_INIT_CHK(rtl8231_init[unit]);

    RTL8231_MDC_SEM_LOCK(unit);

    /* Enable RTL8380 GPIO-Simulated mode instead of Indirect access RTL8231 mode */
    ioal_mem32_read(unit, RTL8380_EXTRA_GPIO_CTRL_ADDR, &mode);
    mode &= ~(0x1UL);
    ioal_mem32_write(unit, RTL8380_EXTRA_GPIO_CTRL_ADDR, mode);

	
    /* for Software generate waveform */
    if (gpioId < EXT_GPIO_ID32)
    {
        smiRead(EXTRA_GPIO_DEV_ADDR, RTL8231_GPIO_IO_SEL_ADDR(gpioId), &temp);
        if (temp & (1 << (gpioId % 16)))
            *pData = GPIO_DIR_IN;
        else
            *pData = GPIO_DIR_OUT;
    }
    else
    {
        smiRead(EXTRA_GPIO_DEV_ADDR, RTL8231_GPIO_PIN_SEL2_ADDR, &temp);
        if (temp & (1 << ((gpioId % 32) + RTL8231_GPIO_PIN_SEL2_IOMASK_32_OFFSET)))
            *pData = GPIO_DIR_IN;
        else
            *pData = GPIO_DIR_OUT;
    }
    RTL8231_MDC_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of r8380_rtl8231_extra_direction_get */

/* Function Name:
 *      r8380_rtl8231_extra_direction_set
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
r8380_rtl8231_extra_direction_set(uint32 unit, uint32 addr, uint32 gpioId, drv_gpio_direction_t data)
{
    uint32  temp, value;
    uint32  mode;
	
    /* Check init state */
    RT_INIT_CHK(rtl8231_init[unit]);

    RTL8231_MDC_SEM_LOCK(unit);

    /* Enable RTL8380 GPIO-Simulated mode instead of Indirect access RTL8231 mode */
    ioal_mem32_read(unit, RTL8380_EXTRA_GPIO_CTRL_ADDR, &mode);
    mode &= ~(0x1UL);
    ioal_mem32_write(unit, RTL8380_EXTRA_GPIO_CTRL_ADDR, mode);

	
    if (data == GPIO_DIR_IN)
        value = 1;
    else
        value = 0;

    /* for Software generate waveform */
    if (gpioId < EXT_GPIO_ID32)
    {
        smiRead(EXTRA_GPIO_DEV_ADDR, RTL8231_GPIO_IO_SEL_ADDR(gpioId), &temp);
        temp = (temp & ~(1 << (gpioId % 16))) | (value << (gpioId % 16));
        smiWrite(EXTRA_GPIO_DEV_ADDR, RTL8231_GPIO_IO_SEL_ADDR(gpioId), temp);
    }
    else
    {
        smiRead(EXTRA_GPIO_DEV_ADDR, RTL8231_GPIO_PIN_SEL2_ADDR, &temp);
        temp = (temp & ~(1 << ((gpioId % 32) + RTL8231_GPIO_PIN_SEL2_IOMASK_32_OFFSET))) | (value << ((gpioId % 32) + RTL8231_GPIO_PIN_SEL2_IOMASK_32_OFFSET));
        smiWrite(EXTRA_GPIO_DEV_ADDR, RTL8231_GPIO_PIN_SEL2_ADDR, temp);
    }
    RTL8231_MDC_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of r8380_rtl8231_extra_direction_set */

/* Function Name:
 *      r8380_rtl8231_extra_i2c_init
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
r8380_rtl8231_extra_i2c_init(uint32 unit, uint32 addr, uint32 i2c_clock, uint32 i2c_data)
{
    /* Check init state */
    RT_INIT_CHK(rtl8231_init[unit]);

    RTL8231_MDC_SEM_LOCK(unit);
    pin_i2c_clock = i2c_clock;
    pin_i2c_data = i2c_data;
    RTL8231_MDC_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of r8380_rtl8231_extra_i2c_init */

/* Function Name:
 *      r8380_rtl8231_extra_i2c_get
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
r8380_rtl8231_extra_i2c_get(uint32 unit, uint32 addr, uint32 *pI2c_clock, uint32 *pI2c_data)
{
    /* Check init state */
    RT_INIT_CHK(rtl8231_init[unit]);

    RTL8231_MDC_SEM_LOCK(unit);
    *pI2c_clock = pin_i2c_clock;
    *pI2c_data = pin_i2c_data;
    RTL8231_MDC_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of r8380_rtl8231_extra_i2c_get */

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
r8380_rtl8231_extra_i2c_read(uint32 unit, uint32 addr, uint32 reg, uint32 *pData)
{
    uint32 rawData=0, ACK;
    uint32  mode;
	
    /* Check init state */
    RT_INIT_CHK(rtl8231_init[unit]);

    RTL8231_MDC_SEM_LOCK(unit);

    /* Enable RTL8380 GPIO-Simulated mode instead of Indirect access RTL8231 mode */
    ioal_mem32_read(unit, RTL8380_EXTRA_GPIO_CTRL_ADDR, &mode);
    mode &= ~(0x1UL);
    ioal_mem32_write(unit, RTL8380_EXTRA_GPIO_CTRL_ADDR, mode);
	
    _i2c_start();                   /* Start I2C */

    _i2c_writeBits(0x0a, 4);        /* CTRL code: 4'b1010 */
    _i2c_writeBits(0x0, 3);         /* CTRL code: 3'b000 */
    _i2c_writeBits(0x0, 1);         /* 0: issue Write command */
    _i2c_readBits(&ACK, 1);         /* ACK for issuing Write command */
    if (0 != ACK)
    {
        return RT_ERR_FAILED;
    }

    _i2c_writeBits((reg&0xff), 8);  /* Set reg_addr[7:0] */
    _i2c_readBits(&ACK, 1);         /* ACK for setting reg_addr[7:0] */   
    if (0 != ACK)
    {
        return RT_ERR_FAILED;
    }

    _i2c_start();                   /* Start I2C */

    _i2c_writeBits(0x0a, 4);        /* CTRL code: 4'b1010 */
    _i2c_writeBits(0x0, 3);         /* CTRL code: 3'b000 */
    _i2c_writeBits(0x1, 1);         /* 1: issue READ command */
    _i2c_readBits(&ACK, 1);         /* ACK for issuing READ command*/
    if (0 != ACK)
    {
        return RT_ERR_FAILED;
    }

    _i2c_readBits(&rawData, 8);     /* Read DATA [7:0] */
    *pData = rawData&0xff; 

    _i2c_stop();

    RTL8231_MDC_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of r8380_rtl8231_extra_i2c_read */

/* Function Name:
 *      r8380_rtl8231_extra_i2c_write
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
r8380_rtl8231_extra_i2c_write(uint32 unit, uint32 addr, uint32 reg, uint32 data)
{
    uint32 ACK;
    uint32 rawData;
    uint32  mode;
	
    /* Enable RTL8380 GPIO-Simulated mode instead of Indirect access RTL8231 mode */
    ioal_mem32_read(unit, RTL8380_EXTRA_GPIO_CTRL_ADDR, &mode);
    mode &= ~(0x1UL);
    ioal_mem32_write(unit, RTL8380_EXTRA_GPIO_CTRL_ADDR, mode);
	

    _i2c_start();                   /* Start I2C */

    _i2c_writeBits(0x0a, 4);        /* CTRL code: 4'b1010 */
    _i2c_writeBits(0x4, 3);         /* CTRL code: 3'b100 */
    _i2c_writeBits(0x0, 1);         /* 0: issue WRITE command */
    _i2c_readBits(&ACK, 1);         /* ACK for issuing WRITE command*/
    if (0 != ACK)
    {
        return RT_ERR_FAILED;
    }
    
    _i2c_writeBits((reg&0xff), 8);  /* Set reg_addr[7:0] */
    _i2c_readBits(&ACK, 1);         /* ACK for setting reg_addr[7:0] */
    if (0 != ACK)
    {
        return RT_ERR_FAILED;
    }

    _i2c_writeBits(data&0xff, 8);   /* Write Data [7:0] out */
    _i2c_readBits(&ACK, 1);         /* ACK for writting data [7:0] */
    if (0 != ACK)
    {
        return RT_ERR_FAILED;
    }
    
    _i2c_stop();	

    /* Add a command to make write command to be right */
    _i2c_read_1(reg, &rawData);

    return RT_ERR_OK;
} /* end of r8380_rtl8231_extra_i2c_write */

/* Function Name:
 *      r8380_rtl8231_extra_read
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
r8380_rtl8231_extra_read(uint32 unit, uint32 addr, uint32 reg_addr, uint32 *pData)
{
    uint32  temp;
    uint32  mode;
	
    /* Check init state */
    RT_INIT_CHK(rtl8231_init[unit]);

    RTL8231_MDC_SEM_LOCK(unit);

    /* Enable RTL8380 GPIO-Simulated mode instead of Indirect access RTL8231 mode */
    ioal_mem32_read(unit, RTL8380_EXTRA_GPIO_CTRL_ADDR, &mode);
    mode &= ~(0x1UL);
    ioal_mem32_write(unit, RTL8380_EXTRA_GPIO_CTRL_ADDR, mode);

	
    /* for Software generate waveform */
    //smiRead(EXTRA_GPIO_DEV_ADDR, reg_addr, &temp);
    smiRead(addr, reg_addr, &temp); /* addr is assigned by customer */
    *pData = temp;
    RTL8231_MDC_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of r8380_rtl8231_extra_read */

/* Function Name:
 *      r8380_rtl8231_extra_write
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
r8380_rtl8231_extra_write(uint32 unit, uint32 addr, uint32 reg_addr, uint32 data)
{
    uint32  mode;
	
    /* Check init state */
    RT_INIT_CHK(rtl8231_init[unit]);

    RTL8231_MDC_SEM_LOCK(unit);

    /* Enable RTL8380 GPIO-Simulated mode instead of Indirect access RTL8231 mode */
    ioal_mem32_read(unit, RTL8380_EXTRA_GPIO_CTRL_ADDR, &mode);
    mode &= ~(0x1UL);
    ioal_mem32_write(unit, RTL8380_EXTRA_GPIO_CTRL_ADDR, mode);

	
    /* for Software generate waveform */
    //smiWrite(EXTRA_GPIO_DEV_ADDR, reg_addr, data);
    smiWrite(addr, reg_addr, data); /* addr is assigned by customer */
    RTL8231_MDC_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of r8380_rtl8231_extra_write */

//----------- smi APIs --------------
#define GPIO_CTRL_REG_BASE (0xb8003500)
#define GPIO_PABC_CNR   (GPIO_CTRL_REG_BASE +0x0)
#define GPIO_PABC_DIR   (GPIO_CTRL_REG_BASE +0x8)
#define GPIO_PABC_DATA  (GPIO_CTRL_REG_BASE +0xc)

static int32 smiInit(uint32 pinMDC, uint32 pinMDIO)
{
    int32 val;

    smi_MDC = pinMDC;
    smi_MDIO = pinMDIO;

    /* configure as gpio pin*/
    val = REG32(GPIO_PABC_CNR);
    REG32(GPIO_PABC_CNR) = val & (~((1<<pinMDC) | (1<<pinMDIO)));

    /*configure pin direction*/
    val = REG32(GPIO_PABC_DIR);
    REG32(GPIO_PABC_DIR) = val | ((GPIO_DIR_OUT<<pinMDC) | (GPIO_DIR_OUT<<pinMDIO));
    /*disable interrupt*/
    //cpu_disable_interrupt();
#if 0    
    {
        uint32  reg = 0;
        uint32  data = 0;
        I2C_read(reg, &data);
        I2C_write(reg, data);
    }
#endif
    return RT_ERR_OK;
}

static void _setGpioDir(uint32 pin, drv_gpio_direction_t dir)
{
    uint32 val;
    /*configure pin direction*/
    val = REG32(GPIO_PABC_DIR);
    if(dir == GPIO_DIR_IN)
        REG32(GPIO_PABC_DIR) = val & (~(1<<pin));
    else
        REG32(GPIO_PABC_DIR) = val | (1<<pin);
    return;
}

static void _setGpioData(uint32 pin, uint32 bitData)
{
    uint32 val;
    val = REG32(GPIO_PABC_DATA);
    if(bitData == 0)
        REG32(GPIO_PABC_DATA) = val & (~(1<<pin));
    else
        REG32(GPIO_PABC_DATA) = val | (1<<pin);
    return;
}

static void _getGpioData(uint32 pin, uint32 *pBitData)
{
    uint32 val;
    val = REG32(GPIO_PABC_DATA);
    if(val & (1<<pin))
        *pBitData = 1;
    else
        *pBitData = 0;
    return;
}

static void _smi_writeBit(uint32 bitData)
{
    _setGpioData(smi_MDIO, bitData);

    CLK_DURATION(MDC_DELAY);
    _setGpioData(smi_MDC, 0);
    CLK_DURATION(MDC_DELAY);
    _setGpioData(smi_MDC, 1);
    return;
}

static void smi_write_bits(uint32 data, uint32 bitLen)
{
    int i;
    for(i=bitLen-1; i>=0; i--)
        _smi_writeBit(data & (1<<i));
    return;
}

static void _smi_readBit(uint32 *pBitData)
{
    CLK_DURATION(MDC_DELAY);
    _setGpioData(smi_MDC, 0);
    CLK_DURATION(MDC_DELAY);
    _setGpioData(smi_MDC, 1);

    _getGpioData(smi_MDIO, pBitData);
    return;
}

static void smi_read_bits(uint32 *pData, uint32 bitLen)
{  
    int i;
    uint32 bitData;
    uint32 rawData = 0;
    
    for(i=bitLen-1; i>=0; i--)
    {
        _smi_readBit(&bitData);
        rawData |= (bitData<<i);
    }
    
    *pData = rawData & 0xffff;
    return;
}

static void _smi_start(void)
{
    _setGpioDir(smi_MDC, GPIO_DIR_OUT);
    _setGpioDir(smi_MDIO, GPIO_DIR_OUT);
    smi_write_bits(0xffffffff, 32);
    return;
}

static void _smiZbit(void)
{
    _setGpioDir(smi_MDIO, GPIO_DIR_IN);

    CLK_DURATION(MDC_DELAY);
    _setGpioData(smi_MDC, 0);
    CLK_DURATION(MDC_DELAY);
    _setGpioData(smi_MDC, 1);
    return;
}

static void smiWrite(uint32 phyad, uint32 regad, uint32 data)
{   
    _smi_start();               /*output 32bit 1 preamble*/
    smi_write_bits(0x5, 4);     /*output 0b0101, write ST+writeOP*/
    
    smi_write_bits(phyad, 5);   /*output 5bit phy address*/
    smi_write_bits(regad, 5);   /*output 5bit reg address*/ 

    smi_write_bits(0x2, 2);     /*output 0b10, write TA*/

    smi_write_bits(data, 16);   /*output data*/

    _smiZbit();
    return;
}

static void smiRead(uint32 phyad, uint32 regad, uint32* pData)
{   
    uint32 data = 0;

    _smi_start();               /*output 32bit 1 preamble*/
    smi_write_bits(0x6, 4);     /*output 0b0110, write ST+readOP*/
    
    smi_write_bits(phyad, 5);   /*output 5bit phy address*/
    smi_write_bits(regad, 5);   /*output 5bit reg address*/ 

    _smiZbit();;                /*output 0b10, write TA*/
    //smi_read_bits(&data, 1);

    /*read data*/
    data = 0;
    smi_read_bits(&data, 16);

    _smiZbit();

    *pData = data;
    return;
}

//--------------- i2c APIs --------
static void _i2c_start(void)
{
    _i2c_i2cPin_direction_set(pin_i2c_clock, GPIO_DIR_OUT);
    _i2c_i2cPin_direction_set(pin_i2c_data, GPIO_DIR_OUT);

    CLK_DURATION(I2C_DELAY);
    _i2c_i2cPin_data_set(pin_i2c_clock, 1);
    CLK_DURATION(I2C_DELAY);
    _i2c_i2cPin_data_set(pin_i2c_data, 1);
    CLK_DURATION(I2C_DELAY);

    _i2c_i2cPin_data_set(pin_i2c_data, 0);
    CLK_DURATION(I2C_DELAY);
    _i2c_i2cPin_data_set(pin_i2c_clock, 0);
    CLK_DURATION(I2C_DELAY);
}

static void _i2c_stop(void)
{
    /* Add a Stop */
    CLK_DURATION(I2C_DELAY);
    _i2c_i2cPin_data_set(pin_i2c_clock, 1);
    _i2c_i2cPin_data_set(pin_i2c_data, 0);
    CLK_DURATION(I2C_DELAY);

    _i2c_i2cPin_data_set(pin_i2c_data, 1);
    CLK_DURATION(I2C_DELAY);
    _i2c_i2cPin_data_set(pin_i2c_clock, 0);

    /* change GPIO pin to Output only */
    _i2c_i2cPin_direction_set(pin_i2c_data, GPIO_DIR_IN);
    _i2c_i2cPin_direction_set(pin_i2c_clock, GPIO_DIR_IN);
}

static void _i2c_readBits(uint32 *pData, uint32 bitLen)
{
    int i;
    uint32 bitData;
    uint32 rawData = 0;

    _i2c_i2cPin_direction_set(pin_i2c_data, GPIO_DIR_IN);
    CLK_DURATION(I2C_DELAY);

    /*read data*/
    for(i=bitLen-1; i>=0; i--)
    {
        _i2c_readBit(&bitData);
        rawData |= (bitData<<i);
    }
    
    *pData = rawData & ((1<<bitLen)-1);

    _i2c_i2cPin_direction_set(pin_i2c_data, GPIO_DIR_OUT);
}

static void _i2c_writeBits(uint32 data, uint32 bitLen)
{
    int i;
    for(i=bitLen-1; i>=0; i--)
        _i2c_writeBit(data & (1<<i));
}

static void _i2c_i2cPin_direction_set(uint32 pin, drv_gpio_direction_t dir)
{
    uint32 temp;

    /* for Software generate waveform */
    if (pin < EXT_GPIO_ID32)
    {
        smiRead(EXTRA_GPIO_DEV_ADDR, RTL8231_GPIO_IO_SEL_ADDR(pin), &temp);
        if (dir == GPIO_DIR_IN)
            temp |= (1<<(pin & 0xF));
        else
            temp &= (~(1<<(pin & 0xF)));
        smiWrite(EXTRA_GPIO_DEV_ADDR, RTL8231_GPIO_IO_SEL_ADDR(pin), temp);
    }
    else
    {
        smiRead(EXTRA_GPIO_DEV_ADDR, RTL8231_GPIO_PIN_SEL2_ADDR, &temp);
        if (dir == GPIO_DIR_IN)
            temp |= (1<<((pin & 0x1F) + RTL8231_GPIO_PIN_SEL2_IOMASK_32_OFFSET));
        else
            temp &= (~(1<<((pin & 0x1F) + RTL8231_GPIO_PIN_SEL2_IOMASK_32_OFFSET)));
        smiWrite(EXTRA_GPIO_DEV_ADDR, RTL8231_GPIO_PIN_SEL2_ADDR, temp);
    }
}

static void _i2c_i2cPin_data_set(uint32 pin, uint32 data)
{
    uint32 temp;

    /* for Software generate waveform */
    smiRead(EXTRA_GPIO_DEV_ADDR, RTL8231_GPIO_CTRL_ADDR(pin), &temp);
    if(data==0)
        temp &= (~(1<<(pin & 0xF)));
    else
        temp |= (1<<(pin & 0xF));
    smiWrite(EXTRA_GPIO_DEV_ADDR, RTL8231_GPIO_CTRL_ADDR(pin), temp);
}

static void _i2c_i2cPin_data_get(uint32 pin, uint32 *pData)
{
    uint32 temp;

    /* for Software generate waveform */
    smiRead(EXTRA_GPIO_DEV_ADDR, RTL8231_GPIO_CTRL_ADDR(pin), &temp);
    if (temp & (1 << (pin & 0xF)))
        *pData = 1;
    else
        *pData = 0;
}

static void _i2c_writeBit(uint32 bitData)
{
    _i2c_i2cPin_data_set(pin_i2c_data, bitData);
    CLK_DURATION(I2C_DELAY);

    _i2c_i2cPin_data_set(pin_i2c_clock, 1);
    CLK_DURATION(I2C_DELAY);
    _i2c_i2cPin_data_set(pin_i2c_clock, 0);
    CLK_DURATION(I2C_DELAY); 
}

static void _i2c_readBit(uint32 *pBitData)
{
    uint32 data;

    _i2c_i2cPin_data_set(pin_i2c_clock, 1);
    CLK_DURATION(I2C_DELAY);

    _i2c_i2cPin_data_get(pin_i2c_data, &data);
    
    _i2c_i2cPin_data_set(pin_i2c_clock, 0);
    CLK_DURATION(I2C_DELAY); 

    *pBitData = (data == 0) ? 0:1;
}

static void _i2c_read_1(uint32 reg, uint32 *pData)
{
    uint32 rawData=0, ACK;

    _i2c_start();			        /* Start I2C */

    _i2c_writeBits(0x0a, 4);        /* CTRL code: 4'b1010 */
    _i2c_writeBits(0x0, 3);         /* CTRL code: 3'b000 */
    _i2c_writeBits(0x0, 1);         /* 0: issue Write command */
    _i2c_readBits(&ACK, 1);         /* ACK for issuing Write command*/
    if (0 != ACK)
    {
        return;
    }

    _i2c_writeBits((reg&0xff), 8);  /* Set reg_addr[7:0] */
    _i2c_readBits(&ACK, 1);         /* ACK for setting reg_addr[7:0] */   
    if (0 != ACK)
    {
        return;
    }

    _i2c_readBits(&rawData, 8);     /* Read DATA [7:0] */
    *pData = rawData&0xff; 

    _i2c_writeBits(0x0, 1);		    /* ACK by CPU */

    _i2c_stop();

    return;
}
