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
 * $Revision: 36864 $
 * $Date: 2013-02-05 20:30:03 +0800 (Tue, 05 Feb 2013) $
 *
 * Purpose : DRV APIs definition.
 *
 * Feature : RTL8231 relative API
 *
 */

#ifndef __DRV_RTL8231_H__
#define __DRV_RTL8231_H__


/*
 * Include Files
 */
#include <common/error.h>
#include <drv/gpio/gpio.h>

/*
 * Symbol Definition
 */
#define RTL8231_LED_FUNC0_ADDR                                  (0x0000)
  #define RTL8231_LED_FUNC0_LED_START_OFFSET                    (1)
  #define RTL8231_LED_FUNC0_LED_START_MASK                      (0x1 << RTL8231_LED_FUNC0_LED_START_OFFSET)
  #define RTL8231_LED_FUNC0_EN_SYNC_GPIO_OFFSET                 (15)
  #define RTL8231_LED_FUNC0_EN_SYNC_GPIO_MASK                   (0x1 << RTL8231_LED_FUNC0_EN_SYNC_GPIO_OFFSET)
                                                                
#define RTL8231_LED_FUNC1_READY                                 (0x370)
#define RTL8231_LED_FUNC1_ADDR                                  (0x0001)
  #define RTL8231_LED_FUNC1_EN_DEBOUNCING_31_OFFSET             (10)
  #define RTL8231_LED_FUNC1_EN_DEBOUNCING_31_MASK               (0x1 << RTL8231_LED_FUNC1_EN_DEBOUNCING_31_OFFSET)
  #define RTL8231_LED_FUNC1_EN_DEBOUNCING_32_OFFSET             (11)
  #define RTL8231_LED_FUNC1_EN_DEBOUNCING_32_MASK               (0x1 << RTL8231_LED_FUNC1_EN_DEBOUNCING_32_OFFSET)
  #define RTL8231_LED_FUNC1_EN_DEBOUNCING_33_OFFSET             (12)
  #define RTL8231_LED_FUNC1_EN_DEBOUNCING_33_MASK               (0x1 << RTL8231_LED_FUNC1_EN_DEBOUNCING_33_OFFSET)
  #define RTL8231_LED_FUNC1_EN_DEBOUNCING_34_OFFSET             (13)
  #define RTL8231_LED_FUNC1_EN_DEBOUNCING_34_MASK               (0x1 << RTL8231_LED_FUNC1_EN_DEBOUNCING_34_OFFSET)
  #define RTL8231_LED_FUNC1_EN_DEBOUNCING_35_OFFSET             (14)
  #define RTL8231_LED_FUNC1_EN_DEBOUNCING_35_MASK               (0x1 << RTL8231_LED_FUNC1_EN_DEBOUNCING_35_OFFSET)
  #define RTL8231_LED_FUNC1_EN_DEBOUNCING_36_OFFSET             (15)
  #define RTL8231_LED_FUNC1_EN_DEBOUNCING_36_MASK               (0x1 << RTL8231_LED_FUNC1_EN_DEBOUNCING_36_OFFSET)
                                                                
#define RTL8231_GPIO_PIN_SEL_ADDR(gpio)                         ((0x0002) + ((gpio) >> 4))
#define RTL8231_GPIO_PIN_SEL0_ADDR                              (0x0002)
#define RTL8231_GPIO_PIN_SEL1_ADDR                              (0x0003)
  #define RTL8231_GPIO_PIN_SEL1_SELGPIO_31_OFFSET               (15)
  #define RTL8231_GPIO_PIN_SEL1_SELGPIO_31_MASK                 (0x1 << RTL8231_GPIO_PIN_SEL1_SELGPIO_31_OFFSET)
#define RTL8231_GPIO_PIN_SEL2_ADDR                              (0x0004)
  #define RTL8231_GPIO_PIN_SEL2_SELGPIO_32_OFFSET               (0)
  #define RTL8231_GPIO_PIN_SEL2_SELGPIO_32_MASK                 (0x1 << RTL8231_GPIO_PIN_SEL2_SELGPIO_32_OFFSET)
  #define RTL8231_GPIO_PIN_SEL2_SELGPIO_33_OFFSET               (1)
  #define RTL8231_GPIO_PIN_SEL2_SELGPIO_33_MASK                 (0x1 << RTL8231_GPIO_PIN_SEL2_SELGPIO_33_OFFSET)
  #define RTL8231_GPIO_PIN_SEL2_SELGPIO_34_OFFSET               (2)
  #define RTL8231_GPIO_PIN_SEL2_SELGPIO_34_MASK                 (0x1 << RTL8231_GPIO_PIN_SEL2_SELGPIO_34_OFFSET)
  #define RTL8231_GPIO_PIN_SEL2_IOMASK_32_OFFSET                (5)
  #define RTL8231_GPIO_PIN_SEL2_IOMASK_32_MASK                  (0x1 << RTL8231_GPIO_PIN_SEL2_IOMASK_32_OFFSET)
  #define RTL8231_GPIO_PIN_SEL2_IOMASK_33_OFFSET                (6)
  #define RTL8231_GPIO_PIN_SEL2_IOMASK_33_MASK                  (0x1 << RTL8231_GPIO_PIN_SEL2_IOMASK_33_OFFSET)
  #define RTL8231_GPIO_PIN_SEL2_IOMASK_34_OFFSET                (7)
  #define RTL8231_GPIO_PIN_SEL2_IOMASK_34_MASK                  (0x1 << RTL8231_GPIO_PIN_SEL2_IOMASK_34_OFFSET)
  #define RTL8231_GPIO_PIN_SEL2_IOMASK_35_OFFSET                (8)
  #define RTL8231_GPIO_PIN_SEL2_IOMASK_35_MASK                  (0x1 << RTL8231_GPIO_PIN_SEL2_IOMASK_35_OFFSET)
  #define RTL8231_GPIO_PIN_SEL2_IOMASK_36_OFFSET                (9)
  #define RTL8231_GPIO_PIN_SEL2_IOMASK_36_MASK                  (0x1 << RTL8231_GPIO_PIN_SEL2_IOMASK_36_OFFSET)
  #define RTL8231_GPIO_PIN_SEL2_INVMASK_32_OFFSET               (10)
  #define RTL8231_GPIO_PIN_SEL2_INVMASK_32_MASK                 (0x1 << RTL8231_GPIO_PIN_SEL2_INVMASK_32_OFFSET)
  #define RTL8231_GPIO_PIN_SEL2_INVMASK_33_OFFSET               (11)
  #define RTL8231_GPIO_PIN_SEL2_INVMASK_33_MASK                 (0x1 << RTL8231_GPIO_PIN_SEL2_INVMASK_33_OFFSET)
  #define RTL8231_GPIO_PIN_SEL2_INVMASK_34_OFFSET               (12)
  #define RTL8231_GPIO_PIN_SEL2_INVMASK_34_MASK                 (0x1 << RTL8231_GPIO_PIN_SEL2_INVMASK_34_OFFSET)
  #define RTL8231_GPIO_PIN_SEL2_INVMASK_35_OFFSET               (13)
  #define RTL8231_GPIO_PIN_SEL2_INVMASK_35_MASK                 (0x1 << RTL8231_GPIO_PIN_SEL2_INVMASK_35_OFFSET)
  #define RTL8231_GPIO_PIN_SEL2_INVMASK_36_OFFSET               (14)
  #define RTL8231_GPIO_PIN_SEL2_INVMASK_36_MASK                 (0x1 << RTL8231_GPIO_PIN_SEL2_INVMASK_36_OFFSET)
                                                                
#define RTL8231_GPIO_IO_SEL_ADDR(gpio)                          ((0x0005) + ((gpio) >> 4))
#define RTL8231_GPIO_IO_SEL0_ADDR                               (0x0005)
#define RTL8231_GPIO_IO_SEL1_ADDR                               (0x0006)
  #define RTL8231_GPIO_IO_SEL1_IOMASK_31_OFFSET                 (15)
  #define RTL8231_GPIO_IO_SEL1_IOMASK_31_MASK                   (0x1 << RTL8231_GPIO_IO_SEL1_IOMASK_31_OFFSET)

#define RTL8231_GPIO_INV_SEL_ADDR(gpio)                         ((0x0007) + ((gpio) >> 4))
#define RTL8231_GPIO_INV_SEL0_ADDR                              (0x0007)
#define RTL8231_GPIO_INV_SEL1_ADDR                              (0x0008)
  #define RTL8231_GPIO_INV_SEL1_INVMASK_31_OFFSET               (15)
  #define RTL8231_GPIO_INV_SEL1_INVMASK_31_MASK                 (0x1 << RTL8231_GPIO_INV_SEL1_INVMASK_31_OFFSET)
                                                                
#define RTL8231_GPIO_CTRL_ADDR(gpio)                            ((0x001C) + ((gpio) >> 4))
#define RTL8231_GPIO_CTRL0_ADDR                                 (0x001C)
#define RTL8231_GPIO_CTRL1_ADDR                                 (0x001D)
  #define RTL8231_GPIO_CTRL1_DATA_31_OFFSET                     (15)
  #define RTL8231_GPIO_CTRL1_DATA_31_MASK                       (0x1 << RTL8231_GPIO_CTRL1_DATA_31_OFFSET)
#define RTL8231_GPIO_CTRL2_ADDR                                 (0x001E)
  #define RTL8231_GPIO_CTRL2_DATA_32_OFFSET                     (0)
  #define RTL8231_GPIO_CTRL2_DATA_32_MASK                       (0x1 << RTL8231_GPIO_CTRL2_DATA_32_OFFSET)
  #define RTL8231_GPIO_CTRL2_DATA_33_OFFSET                     (1)
  #define RTL8231_GPIO_CTRL2_DATA_33_MASK                       (0x1 << RTL8231_GPIO_CTRL2_DATA_33_OFFSET)
  #define RTL8231_GPIO_CTRL2_DATA_34_OFFSET                     (2)
  #define RTL8231_GPIO_CTRL2_DATA_34_MASK                       (0x1 << RTL8231_GPIO_CTRL2_DATA_34_OFFSET)
  #define RTL8231_GPIO_CTRL2_DATA_35_OFFSET                     (3)
  #define RTL8231_GPIO_CTRL2_DATA_35_MASK                       (0x1 << RTL8231_GPIO_CTRL2_DATA_35_OFFSET)
  #define RTL8231_GPIO_CTRL2_DATA_36_OFFSET                     (4)
  #define RTL8231_GPIO_CTRL2_DATA_36_MASK                       (0x1 << RTL8231_GPIO_CTRL2_DATA_36_OFFSET)
  #define RTL8231_GPIO_CTRL2_SYNC_GPIO_OFFSET                   (15)
  #define RTL8231_GPIO_CTRL2_SYNC_GPIO_MASK                     (0x1 << RTL8231_GPIO_CTRL2_SYNC_GPIO_OFFSET)

typedef int32 (*drv_rtl8231_mdcSem_cb_f)(uint32 unit, uint32 type);

/*
 * Data Type Definition
 */
typedef struct rtl8231_mdcSem_cb_entry_s
{
    drv_rtl8231_mdcSem_cb_f mdcSem_callback;
} rtl8231_mdcSem_cb_entry_t;

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
extern int32 
drv_rtl8231_i2c_read(uint32 unit, uint32 slave_addr, uint32 reg_addr, uint32 *pData);

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
extern int32 
drv_rtl8231_i2c_write(uint32 unit, uint32 slave_addr, uint32 reg_addr, uint32 data);

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
extern int32 
drv_rtl8231_mdc_read(uint32 unit, uint32 phy_id, uint32 page, uint32 reg_addr, uint32 *pData);

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
extern int32 
drv_rtl8231_mdc_write(uint32 unit, uint32 phy_id, uint32 page, uint32 reg_addr, uint32 data);

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
extern int32 
drv_rtl8231_init(uint32 unit);

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
extern int32 
drv_rtl8231_mdcSem_register(uint32 unit, drv_rtl8231_mdcSem_cb_f fMdcSemCb);

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
extern int32 
drv_rtl8231_mdcSem_unregister(uint32 unit, drv_rtl8231_mdcSem_cb_f fMdcSemCb);

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
extern int32
drv_rtl8231_extra_devReady_get(uint32 unit, uint32 addr, uint32 *pIsReady);

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
extern int32
drv_rtl8231_extra_devEnable_get(uint32 unit, uint32 addr, rtk_enable_t *pEnable);

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
extern int32
drv_rtl8231_extra_devEnable_set(uint32 unit, uint32 addr, rtk_enable_t enable);

/* Function Name:
 *      drv_extGpio_syncEnable_get
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
extern int32
drv_rtl8231_extra_dataBit_get(uint32 unit, uint32 addr, uint32 gpioId, uint32 *pData);

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
extern int32
drv_rtl8231_extra_dataBit_set(uint32 unit, uint32 addr, uint32 gpioId, uint32 data);

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
extern int32
drv_rtl8231_extra_direction_get(uint32 unit, uint32 addr, uint32 gpioId, drv_gpio_direction_t *pData);

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
extern int32
drv_rtl8231_extra_direction_set(uint32 unit, uint32 addr, uint32 gpioId, drv_gpio_direction_t data);

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
extern int32
drv_rtl8231_extra_i2c_init(uint32 unit, uint32 addr, uint32 i2c_clock, uint32 i2c_data);

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
extern int32
drv_rtl8231_extra_i2c_get(uint32 unit, uint32 addr, uint32 *pI2c_clock, uint32 *pI2c_data);

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
extern int32
drv_rtl8231_extra_i2c_read(uint32 unit, uint32 addr, uint32 reg, uint32 *pData);

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
extern int32
drv_rtl8231_extra_i2c_write(uint32 unit, uint32 addr, uint32 reg, uint32 data);

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
extern int32 
drv_rtl8231_extra_read(uint32 unit, uint32 addr, uint32 reg_addr, uint32 *pData);

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
extern int32 
drv_rtl8231_extra_write(uint32 unit, uint32 addr, uint32 reg_addr, uint32 data);

#endif /* __DRV_RTL8231_H__ */
