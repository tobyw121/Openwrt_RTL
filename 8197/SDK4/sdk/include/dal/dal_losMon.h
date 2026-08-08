/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * $Revision: 17826 $
 * $Date: 2011-05-12 15:24:35 +0800 (Thu, 12 May 2011) $
 *
 * Purpose : Monitor LOS status of combo port 
 *
 * Feature : 
 *
 */
#ifndef __DAL_LOSMON_H__ 
#define __DAL_LOSMON_H__

/*  
 * Include Files 
 */
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <drv/gpio/ext_gpio.h>
#include <drv/gpio/gpio.h>
#include <dal/dal_mapper.h>

/* 
 * Symbol Definition 
 */
/* LOS Signal depend on internal or external GPIO definition
 * 1) For use internal GPIO:
 *      1.1) define LOSMON_GPIO_TYPE to INT_GPIO
 *      1.2) fill the gpioDevConf configuration in dal_losMon_conf.c
 * 2) For use external GPIO:
 *      2.1) define LOSMON_GPIO_TYPE to EXT_GPIO
 *      2.2) fill the extGpioDevConf configuration in dal_losMon_conf.c
 */
#define INT_GPIO    0
#define EXT_GPIO    1
#define LOSMON_GPIO_TYPE INT_GPIO

#define DAL_LOSMON_EXT_GPIO_DEV_NUM     1
#define DAL_LOSMON_NUM_OF_COMBO_PORT    4

typedef struct dal_losMon_extGpio_conf_s
{
    uint32 gpio;
    uint32 direction;   /* GPIO_DIR_OUT or GPIO_DIR_IN */
    uint32 debounce;    /* 0: normal, 1: debounce */
    uint32 inverter;    /* 0: normal, 1: invert */
    uint32 port;        /* use for the port number */
} dal_losMon_extGpio_conf_t;

typedef struct dal_losMon_extGpio_devConf_s
{
    uint32 devId;   /* PHY ID or Slave Addr */
    dal_losMon_extGpio_conf_t conf[EXT_GPIO_ID_END];
} dal_losMon_extGpio_devConf_t;

typedef struct dal_losMon_gpio_conf_s
{
    gpioID gpio;                        /* (gpio-port << 16) + gpio-pin */
    drv_gpio_control_t function;        /* GPIO_CTRLFUNC_NORMAL or GPIO_CTRLFUNC_DEDICATE_PERIPHERAL */
    drv_gpio_direction_t direction;     /* GPIO_DIR_OUT or GPIO_DIR_IN */
    drv_gpio_interruptType_t interruptEnable;   /* GPIO_INT_DISABLE,
                                                   GPIO_INT_FALLING_EDGE, 
                                                   GPIO_INT_RISING_EDGE, 
                                                   GPIO_INT_BOTH_EDGE */
    uint32 port;        /* use for the port number */
} dal_losMon_gpio_conf_t;

typedef struct dal_losMon_gpio_devConf_s
{
    dal_losMon_gpio_conf_t conf[DAL_LOSMON_NUM_OF_COMBO_PORT];
} dal_losMon_gpio_devConf_t;

/* 
 * Data Declaration 
 */
extern const dal_losMon_extGpio_devConf_t extGpioDevConf[DAL_LOSMON_EXT_GPIO_DEV_NUM];
extern const dal_losMon_gpio_devConf_t  gpioDevConf;

/*
 * Macro Definition
 */

/* 
 * Function Declaration 
 */

/* Module Name : */

/* Function Name: 
 *      dal_losMon_init
 * Description: 
 *      Initial LOS Monitor component
 * Input:  
 *      None
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_FAILED   - initialize fail
 *      RT_ERR_OK       - initialize success
 * Note: 
 *      None
 */ 
extern int32
dal_losMon_init(void);

/* Function Name: 
 *      dal_losMon_enable
 * Description: 
 *      Enable LOS monitor thread
 * Input:  
 *      scan_interval_us        - scan interval in us.
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_OK       - initialize success
 *      RT_ERR_FAILED   - initialize fail
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - scan interval is too small
 * Note:
 *      When enable LOS monitor thread, all LOS status change will be handled by thread.
 *      
 */ 
extern int32
dal_losMon_enable(uint32 scan_interval_us);

/* Function Name: 
 *      dal_losMon_disable
 * Description: 
 *      Disable LOS scan thread
 * Input:  
 *      None.
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_OK       - initialize success
 *      RT_ERR_FAILED   - initialize fail
 *      RT_ERR_NOT_INIT - The module is not initial
 * Note: 
 *      When disable LOS monitor thread, all LOS status change will be handled by thread.
 */ 
extern int32
dal_losMon_disable(void);

/* Function Name: 
 *      dal_losMon_swScanPorts_set
 * Description: 
 *      Configure portmask of software LOS scan for certain unit
 * Input:  
 *      unit                - unit id
 *      pSwScan_portmask    - portmask for software scan
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_OK           - initialize success
 *      RT_ERR_FAILED       - initialize fail
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note: 
 *      
 */ 
extern int32
dal_losMon_swScanPorts_set(uint32 unit, rtk_portmask_t *pSwScan_portmask);

/* Function Name: 
 *      dal_losMon_swScanPorts_get
 * Description: 
 *      Get portmask of software LOS scan for certain unit
 * Input:  
 *      unit                - unit id
 * Output: 
 *      pSwScan_portmask    - portmask for software scan
 * Return: 
 *      RT_ERR_OK           - initialize success
 *      RT_ERR_FAILED       - initialize fail
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note: 
 *      
 */ 
extern int32
dal_losMon_swScanPorts_get(uint32 unit, rtk_portmask_t *pSwScan_portmask);

#endif /* __DAL_LOSMON_H__ */
