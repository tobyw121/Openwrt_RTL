/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * $Revision: 17826 $
 * $Date: 2011-05-12 15:24:35 +0800 (Thu, 12 May 2011) $
 *
 * Purpose : Use to Management each device
 *
 * Feature : The file have include the following module and sub-modules
 *           1) Initialize system
 *           2) Initialize device
 *           3) Mangement Devices
 *
 */

/*
 * Include Files
 */
#include <common/rt_autoconf.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/debug/rt_log.h>
#include <common/util/rt_bitop.h>
#include <common/util/rt_util.h>
#include <osal/sem.h>
#include <osal/lib.h>
#include <osal/memory.h>
#include <osal/thread.h>
#include <ioal/mem32.h>
#include <drv/gpio/gpio.h>
#include <drv/gpio/ext_gpio.h>
#include <hal/common/halctrl.h>
#include <dal/dal_mgmt.h>
#include <dal/dal_mapper.h>
#include <dal/dal_common.h>
#if defined(CONFIG_SDK_AUTO_COMBO_MEDIA_BY_GPIO)
#include <dal/dal_losMon.h>
#endif
#include <rtk/port.h>
#include <rtk/default.h>

/* 
 * Symbol Definition
 */

/* LOS monitor control block */
typedef struct dal_losMon_cb_s {
    osal_thread_t       thread_id;
    uint32              scan_interval_us;
    uint32              los_swScan_units[((RTK_MAX_NUM_OF_UNIT - 1)/32) + 1];
    rtk_portmask_t      los_swScan_portmask[RTK_MAX_NUM_OF_UNIT];
    rtk_portmask_t      los_status[RTK_MAX_NUM_OF_UNIT];
    rtk_portmask_t      los_status_valid[RTK_MAX_NUM_OF_UNIT];
} dal_losMon_cb_t;


/*
 * Data Declaration
 */
static uint32   losMon_init;
static uint32   los_change_sem;
static dal_losMon_cb_t     *pLosMon_cb;


/*
 * Macro Declaration
 */ 

/*
 * Function Declaration
 */

static void _dal_losMon_thread(void *pInput);
static void _dal_losMon_extGpio_swScan(void);
static void _dal_losMon_gpio_swScan(void);
static int32 _dal_losMon_init_extGpio_config(void);
static int32 _dal_losMon_init_gpio_config(void);

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
 *      
 */ 
int32 dal_losMon_init(void)
{
    int32   ret = RT_ERR_FAILED;
    hal_control_t *pHal_control;

    /* init value */
    losMon_init = INIT_NOT_COMPLETED;

    if ((pHal_control = hal_ctrlInfo_get(0)) == NULL)
    {
        return RT_ERR_FAILED;
    }

    /* allocate memory for control block */
    pLosMon_cb = osal_alloc(sizeof(dal_losMon_cb_t));
    
    if (0 == pLosMon_cb){
        RT_LOG(LOG_DEBUG, MOD_DAL, "LOS monitor allocate memory failed");
        return RT_ERR_FAILED;
    }

    /* create semaphore for sync, this semaphore is empty in beginning */
    los_change_sem = osal_sem_create(0);
    
    if (0 == los_change_sem){
        osal_free(pLosMon_cb);
        RT_LOG(LOG_DEBUG, MOD_DAL, "LOS monitor semaphore create failed");
        return RT_ERR_FAILED;
    }

    osal_memset(pLosMon_cb, 0, sizeof(dal_losMon_cb_t));

    losMon_init = INIT_COMPLETED;

    if (LOSMON_GPIO_TYPE == EXT_GPIO)
    {
        if (( ret = _dal_losMon_init_extGpio_config()) != RT_ERR_OK)
        {
            losMon_init = INIT_NOT_COMPLETED;
            osal_free(pLosMon_cb);
            RT_LOG(LOG_DEBUG, MOD_DAL, "LOS monitor default external GPIO configuration init failed");
            return ret;
        }
    }
    else
    {
        if (( ret = _dal_losMon_init_gpio_config()) != RT_ERR_OK)
        {
            losMon_init = INIT_NOT_COMPLETED;
            osal_free(pLosMon_cb);
            RT_LOG(LOG_DEBUG, MOD_DAL, "LOS monitor default internal GPIO configuration init failed");
            return ret;
        }
    }

    return RT_ERR_OK;
    
} /* end of dal_losMon_init */

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
int32 dal_losMon_enable(uint32 scan_interval_us)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "scan_interval_us=%u", 
           scan_interval_us); 
    
    /* check Init status */
    RT_INIT_CHK(losMon_init);
    
    /* parameter check */
    RT_PARAM_CHK((scan_interval_us < RTK_LOSMON_SCAN_INTERVAL_MIN), RT_ERR_OUT_OF_RANGE);
    
    pLosMon_cb->scan_interval_us = scan_interval_us;
    
    if ((pLosMon_cb->thread_id) != 0)
    {
        RT_ERR(pLosMon_cb->thread_id, (MOD_DAL|MOD_PORT), "");
        return RT_ERR_THREAD_EXIST;
    }
    
    /* create thread */
    pLosMon_cb->thread_id = osal_thread_create("LOS Monitor Thread", RTK_DEFAULT_LOS_MON_STACK_SIZE, RTK_DEFAULT_LOS_MON_THREAD_PRI
                            , (void *)_dal_losMon_thread, NULL);
    
    if (0 == (pLosMon_cb->thread_id))
    {
        RT_ERR(pLosMon_cb->thread_id, (MOD_DAL|MOD_PORT), "");
        return RT_ERR_THREAD_CREATE_FAILED;
    }
    
    return RT_ERR_OK;
} /* end of dal_losMon_enable */

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
int32 dal_losMon_disable(void)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), ""); 
    
    /* check Init status */
    RT_INIT_CHK(losMon_init);
    
    /* parameter check */
    
    /* reset scan_interval_us to 0, thread will suicide after finish all waiting job */
    pLosMon_cb->scan_interval_us = 0;
    
    /* let thread continue */
    osal_sem_give(los_change_sem);
    
    return RT_ERR_OK;
    
} /* end of dal_losMon_disable */

/* Function Name: 
 *      _dal_losMon_thread
 * Description: 
 *      Unregister callback function for LOS change notification 
 * Input:  
 *      None.
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_FAILED   - initialize fail
 *      RT_ERR_OK       - initialize success
 * Note: 
 *      
 */ 
static void _dal_losMon_thread(void *pInput)
{
    int32   ret;

    /* forever loop */
    while (pLosMon_cb->scan_interval_us != 0)
    {
        /* wait semaphore for LOS monitor interval */
        ret = osal_sem_take(los_change_sem, pLosMon_cb->scan_interval_us);

        if (LOSMON_GPIO_TYPE == EXT_GPIO)
            _dal_losMon_extGpio_swScan();
        else
            _dal_losMon_gpio_swScan();
    }
    
    osal_thread_exit(0);
    
    return;
} /* end of _dal_losMon_thread */

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
int32 dal_losMon_swScanPorts_set(uint32 unit, rtk_portmask_t *pSwScan_portmask)
{
    rtk_portmask_t    temp_portmask;

    /* check Init status */
    RT_INIT_CHK(losMon_init);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pSwScan_portmask), RT_ERR_NULL_POINTER);
    
    if (0 == RTK_PORTMASK_GET_PORT_COUNT(*pSwScan_portmask))
    {
        BITMAP_CLEAR(pLosMon_cb->los_swScan_units, unit);
    }
    else
    {
        BITMAP_SET(pLosMon_cb->los_swScan_units, unit);
    }

    RTK_PORTMASK_ASSIGN(temp_portmask, pLosMon_cb->los_swScan_portmask[unit]);
    RTK_PORTMASK_AND(temp_portmask, *pSwScan_portmask);
    RTK_PORTMASK_ASSIGN(pLosMon_cb->los_swScan_portmask[unit], *pSwScan_portmask);
    RTK_PORTMASK_AND(pLosMon_cb->los_status_valid[unit], temp_portmask);
    
    return RT_ERR_OK;
} /* end of dal_losMon_swScanPorts_set */

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
int32 dal_losMon_swScanPorts_get(uint32 unit, rtk_portmask_t *pSwScan_portmask)
{
    /* check Init status */
    RT_INIT_CHK(losMon_init);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pSwScan_portmask), RT_ERR_NULL_POINTER);

    RTK_PORTMASK_ASSIGN(*pSwScan_portmask, pLosMon_cb->los_swScan_portmask[unit]);
    
    return RT_ERR_OK;
} /* end of dal_linkMon_swScanPorts_get */

/* Function Name: 
 *      _dal_losMon_extGpio_swScan
 * Description: 
 *      LOS monitor external GPIO software scan routine. 
 * Input:  
 *      None.
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_FAILED   - initialize fail
 *      RT_ERR_OK       - initialize success
 * Note: 
 *      
 */ 
static void _dal_losMon_extGpio_swScan(void)
{
    uint32  unit, i, j, gpioData;
    
    for (unit = 0; unit <= RTK_MAX_UNIT_ID; unit++)
    {
        for (i = 0; i < DAL_LOSMON_EXT_GPIO_DEV_NUM; i++)
        {
            for (j = 0; j < EXT_GPIO_ID_END; j++)
            {
                if (extGpioDevConf[i].conf[j].gpio == EXT_GPIO_ID_END)
                    break;

                if (RTK_PORTMASK_IS_PORT_SET(pLosMon_cb->los_swScan_portmask[unit], extGpioDevConf[i].conf[j].port))
                {
                    drv_extGpio_dataBit_get(unit, i, extGpioDevConf[i].conf[j].gpio, &gpioData);
                    if (gpioData == 0)
                    {
                        if (RTK_PORTMASK_IS_PORT_CLEAR(pLosMon_cb->los_status_valid[unit], extGpioDevConf[i].conf[j].port) ||
                            RTK_PORTMASK_IS_PORT_SET(pLosMon_cb->los_status[unit], extGpioDevConf[i].conf[j].port))
                        {
                            RTK_PORTMASK_PORT_CLEAR(pLosMon_cb->los_status[unit], extGpioDevConf[i].conf[j].port);
                            RTK_PORTMASK_PORT_SET(pLosMon_cb->los_status_valid[unit], extGpioDevConf[i].conf[j].port);
                            RT_DBG(LOG_DEBUG, MOD_DAL, "unit=%d, port=%d configure media to FIBER", unit, extGpioDevConf[i].conf[j].port);
                            RT_MAPPER(unit)->port_phyComboPortMedia_set(unit, extGpioDevConf[i].conf[j].port, PORT_MEDIA_FIBER);
                        }
                    }
                    else
                    {
                        if (RTK_PORTMASK_IS_PORT_CLEAR(pLosMon_cb->los_status_valid[unit], extGpioDevConf[i].conf[j].port) ||
                            RTK_PORTMASK_IS_PORT_CLEAR(pLosMon_cb->los_status[unit], extGpioDevConf[i].conf[j].port))
                        {
                            RTK_PORTMASK_PORT_SET(pLosMon_cb->los_status[unit], extGpioDevConf[i].conf[j].port);
                            RTK_PORTMASK_PORT_SET(pLosMon_cb->los_status_valid[unit], extGpioDevConf[i].conf[j].port);
                            RT_DBG(LOG_DEBUG, MOD_DAL, "unit=%d, port=%d configure media to COPPER", unit, extGpioDevConf[i].conf[j].port);
                            RT_MAPPER(unit)->port_phyComboPortMedia_set(unit, extGpioDevConf[i].conf[j].port, PORT_MEDIA_COPPER);
                        }
                    }
                }
            }
        }
    }
    return;
} /* end of _dal_losMon_extGpio_swScan */

/* Function Name: 
 *      _dal_losMon_gpio_swScan
 * Description: 
 *      LOS monitor internal GPIO software scan routine. 
 * Input:  
 *      None.
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_FAILED   - initialize fail
 *      RT_ERR_OK       - initialize success
 * Note: 
 *      
 */ 
static void _dal_losMon_gpio_swScan(void)
{
    uint32  unit, i, gpioData;
    
    for (unit = 0; unit <= RTK_MAX_UNIT_ID; unit++)
    {
        for (i = 0; i < DAL_LOSMON_NUM_OF_COMBO_PORT; i++)
        {
            if (RTK_PORTMASK_IS_PORT_SET(pLosMon_cb->los_swScan_portmask[unit], gpioDevConf.conf[i].port))
            {
                drv_gpio_dataBit_get(gpioDevConf.conf[i].gpio, &gpioData);
                if (gpioData == 0)
                {
                    if (RTK_PORTMASK_IS_PORT_CLEAR(pLosMon_cb->los_status_valid[unit], gpioDevConf.conf[i].port) ||
                        RTK_PORTMASK_IS_PORT_SET(pLosMon_cb->los_status[unit], gpioDevConf.conf[i].port))
                    {
                        RTK_PORTMASK_PORT_CLEAR(pLosMon_cb->los_status[unit], gpioDevConf.conf[i].port);
                        RTK_PORTMASK_PORT_SET(pLosMon_cb->los_status_valid[unit], gpioDevConf.conf[i].port);
                        RT_DBG(LOG_DEBUG, MOD_DAL, "unit=%d, port=%d configure media to FIBER", unit, gpioDevConf.conf[i].port);
                        RT_MAPPER(unit)->port_phyComboPortMedia_set(unit, gpioDevConf.conf[i].port, PORT_MEDIA_FIBER);
                    }
                }
                else
                {
                    if (RTK_PORTMASK_IS_PORT_CLEAR(pLosMon_cb->los_status_valid[unit], gpioDevConf.conf[i].port) ||
                        RTK_PORTMASK_IS_PORT_CLEAR(pLosMon_cb->los_status[unit], gpioDevConf.conf[i].port))
                    {
                        RTK_PORTMASK_PORT_SET(pLosMon_cb->los_status[unit], gpioDevConf.conf[i].port);
                        RTK_PORTMASK_PORT_SET(pLosMon_cb->los_status_valid[unit], gpioDevConf.conf[i].port);
                        RT_DBG(LOG_DEBUG, MOD_DAL, "unit=%d, port=%d configure media to COPPER", unit, gpioDevConf.conf[i].port);
                        RT_MAPPER(unit)->port_phyComboPortMedia_set(unit, gpioDevConf.conf[i].port, PORT_MEDIA_COPPER);
                    }
                }
            }
        }
    }
    return;
} /* end of _dal_losMon_gpio_swScan */

/* Function Name:
 *      _dal_losMon_init_extGpio_config
 * Description:
 *      Initialize default external GPIO configuration for LOS monitor mechanism.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      1. Module must be initialized before using all of APIs in this module
 */
static int32
_dal_losMon_init_extGpio_config(void)
{
    uint32  val, i, j, addr;
    int32   ret;
    drv_extGpio_devConf_t   devData;
    drv_extGpio_conf_t      pinData;
    hal_control_t *pHal_ctrl;

    if ((pHal_ctrl = hal_ctrlInfo_get(0)) == NULL)
    {
        return RT_ERR_FAILED;
    }

    for (i = 0; i < DAL_LOSMON_EXT_GPIO_DEV_NUM; i++)
    {
        if (RTL8328M_CHIP_ID == pHal_ctrl->chip_id || RTL8328S_CHIP_ID == pHal_ctrl->chip_id || RTL8328L_CHIP_ID == pHal_ctrl->chip_id)
        {   /* 8328 */
            addr = 0x40014;
        }
        else
        {   /* 8389/8329 */
            addr = 0x1d08;
        }

        /* Enable the PHY_ID MDC/MDIO access */
        if ((ret = ioal_mem32_read(0, addr, &val)) != RT_ERR_OK)
            return RT_ERR_FAILED;
        val |= (1 << extGpioDevConf[i].devId);
        if ((ret = ioal_mem32_write(0, addr, val)) != RT_ERR_OK)
            return RT_ERR_FAILED;

        /* init the external GPIO device i, MDC/MDIO, PHY_ID x, Page 30 */
        devData.access_mode = EXT_GPIO_ACCESS_MODE_MDC;
        devData.address = extGpioDevConf[i].devId;
        devData.page = 30;
        if ((ret = drv_extGpio_dev_init(0, i, &devData)) != RT_ERR_OK)
            return RT_ERR_FAILED;

        for (j = 0; j < EXT_GPIO_ID_END; j++)
        {
            if (extGpioDevConf[i].conf[j].gpio == EXT_GPIO_ID_END)
                break;

            /* Init the necessary GPIO pin */
            pinData.direction = extGpioDevConf[i].conf[j].direction;
            pinData.debounce = extGpioDevConf[i].conf[j].debounce;
            pinData.inverter = extGpioDevConf[i].conf[j].inverter;
            if ((ret = drv_extGpio_pin_init(0, i, extGpioDevConf[i].conf[j].gpio, &pinData)) != RT_ERR_OK)
                return RT_ERR_FAILED;
        }
        if ((ret = drv_extGpio_devEnable_set(0, i, ENABLED)) != RT_ERR_OK)
            return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
} /* end of _dal_losMon_init_extGpio_config */

/* Function Name:
 *      _dal_losMon_init_gpio_config
 * Description:
 *      Initialize default internal GPIO configuration for LOS monitor mechanism.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      1. Module must be initialized before using all of APIs in this module
 */
static int32
_dal_losMon_init_gpio_config(void)
{
    uint32  i;
    int32   ret = RT_ERR_FAILED;

    for (i = 0; i < DAL_LOSMON_NUM_OF_COMBO_PORT; i++)
    {
        if ((ret = drv_gpio_init(gpioDevConf.conf[i].gpio, gpioDevConf.conf[i].function, gpioDevConf.conf[i].direction, gpioDevConf.conf[i].interruptEnable)) != RT_ERR_OK)
            return ret;
    }

    return RT_ERR_OK;
} /* end of _dal_losMon_init_gpio_config */
