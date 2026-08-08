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
#include <osal/sem.h>
#include <osal/lib.h>
#include <osal/memory.h>
#include <dal/dal_mgmt.h>
#include <dal/dal_mapper.h>
#include <dal/dal_linkMon.h>
#include <dal/dal_macMon.h>
#if defined(CONFIG_SDK_AUTO_COMBO_MEDIA_BY_GPIO)
#include <dal/dal_losMon.h>
#endif
#include <dal/dal_waMon.h>
#include <dal/ssw/dal_ssw_mapper.h>
#include <dal/esw/dal_esw_mapper.h>
#include <dal/cypress/dal_cypress_mapper.h>
#include <dal/maple/dal_maple_mapper.h>
#include <hal/chipdef/chip.h>
#include <hal/common/halctrl.h>

#if defined(CONFIG_SDK_RTL8390)
#include <dal/dal_linkFaultMon.h>
#endif

/*
 * Symbol Definition
 */
#undef  CONFIG_SDK_WA_LIMIT_LEARN_COUNT
#define CONFIG_SDK_WA_FORWARD_TABLE

/*
 * Data Declaration
 */
dal_mgmt_info_t         *pMgmt_node[RTK_MAX_NUM_OF_UNIT];
static uint32           mgmt_init = INIT_NOT_COMPLETED;
static osal_mutex_t     mgmt_sem = 0;

const static dal_mapper_info_t dal_mapper_database[] =
{
#if defined(CONFIG_SDK_RTL8389)
    {RTL8389M_CHIP_ID, &dal_ssw_mapper},
    {RTL8389L_CHIP_ID, &dal_ssw_mapper},
    {RTL8329M_CHIP_ID, &dal_ssw_mapper},
    {RTL8377M_CHIP_ID, &dal_ssw_mapper},
#endif /* end of defined(CONFIG_SDK_RTL8389) */

#if defined(CONFIG_SDK_RTL8328)
    {RTL8328M_CHIP_ID, &dal_esw_mapper},
    {RTL8328S_CHIP_ID, &dal_esw_mapper},
    {RTL8328L_CHIP_ID, &dal_esw_mapper},
#endif /* end of defined(CONFIG_SDK_RTL8328) */

#if defined(CONFIG_SDK_RTL8390)
    {RTL8352M_CHIP_ID, &dal_cypress_mapper},
    {RTL8353M_CHIP_ID, &dal_cypress_mapper},
    {RTL8391M_CHIP_ID, &dal_cypress_mapper},
    {RTL8392M_CHIP_ID, &dal_cypress_mapper},
    {RTL8393M_CHIP_ID, &dal_cypress_mapper},
    {RTL8396M_CHIP_ID, &dal_cypress_mapper},
    {RTL8352MES_CHIP_ID, &dal_cypress_mapper},
    {RTL8353MES_CHIP_ID, &dal_cypress_mapper},
    {RTL8392MES_CHIP_ID, &dal_cypress_mapper},
    {RTL8393MES_CHIP_ID, &dal_cypress_mapper},
    {RTL8396MES_CHIP_ID, &dal_cypress_mapper},
#endif /* end of defined(CONFIG_SDK_RTL8390) */

#if defined(CONFIG_SDK_RTL8380)
    {RTL8380M_CHIP_ID, &dal_maple_mapper},
    {RTL8330M_CHIP_ID, &dal_maple_mapper},
    {RTL8382M_CHIP_ID, &dal_maple_mapper},
    {RTL8332M_CHIP_ID, &dal_maple_mapper},
    {RTL8380MES_CHIP_ID, &dal_maple_mapper},
    {RTL8330MES_CHIP_ID, &dal_maple_mapper},
    {RTL8382MES_CHIP_ID, &dal_maple_mapper},
    {RTL8332MES_CHIP_ID, &dal_maple_mapper},
#endif
};
/*
 * Macro Declaration
 */
#define SEM_LOCK()\
do {\
    if (osal_sem_mutex_take(mgmt_sem, OSAL_SEM_WAIT_FOREVER) != 0)\
    {\
        RT_LOG(LOG_DEBUG, MOD_DAL, "semaphore lock failed");\
        return RT_ERR_FAILED;\
    }\
} while(0)

#define SEM_UNLOCK()\
do {\
    if (osal_sem_mutex_give(mgmt_sem) != RT_ERR_OK)\
    {\
        RT_LOG(LOG_DEBUG, MOD_DAL, "semaphore unlock failed");\
        return RT_ERR_FAILED;\
    }\
} while(0)
/*
 * Function Declaration
 */
static int32 dal_mgmt_attachDevice(uint32 unit);
static dal_mapper_t *dal_mgmt_find_mapper(uint32 unit);

/* Module Name : */

/* Function Name:
 *      dal_mgmt_init
 * Description:
 *      Initilize DAL(semaphore, database clear)
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED   - initialize fail
 *      RT_ERR_OK       - initialize success
 * Note:
 *      RTK must call this function before do other kind of action.
 */
int32 dal_mgmt_init(void)
{
    int32   ret;

    /* if was initilized, return RT_ERROR */
    if (INIT_COMPLETED == mgmt_init){
        RT_LOG(LOG_DEBUG, MOD_DAL, "DAL mgmt is alread inited");
        return RT_ERR_FAILED;
    }

    /* Initialize the MGMT database */
    osal_memset(pMgmt_node, 0, sizeof(pMgmt_node));

    /* create semaphore */
    mgmt_sem = osal_sem_mutex_create();
    if (0 == mgmt_sem){
        RT_LOG(LOG_DEBUG, MOD_DAL, "semaphore create failed");
        return RT_ERR_FAILED;
    }

    if ((ret = dal_linkMon_init()) != RT_ERR_OK)
        return ret;

#if defined(CONFIG_SDK_AUTO_COMBO_MEDIA_BY_GPIO)
    if ((ret = dal_losMon_init()) != RT_ERR_OK)
        return ret;
#endif

#if defined(CONFIG_SDK_RTL8328)
#if defined(CONFIG_SDK_WA_LIMIT_LEARN_COUNT) || defined(CONFIG_SDK_WA_FORWARD_TABLE)
    if ((ret = dal_macMon_init()) != RT_ERR_OK)
        return ret;
#endif
#if defined(CONFIG_SDK_WA_LINKDOWN_PWR_SAVING) || defined(CONFIG_SDK_WA_BACK_PRESSURE) || defined(CONFIG_SDK_WA_INTRALINK_DELAY) || defined(CONFIG_SDK_WA_EEE_COMPATIBLE) || defined(CONFIG_SDK_WA_RTL8231_RESET) || defined(CONFIG_SDK_WA_PKTBUF_WATCHDOG)
    if ((ret = dal_waMon_init()) != RT_ERR_OK)
        return ret;
#endif
#endif
#if defined(CONFIG_SDK_RTL8380)
    if ((ret = dal_waMon_init()) != RT_ERR_OK)
        return ret;
#endif
#if (defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8350)) && !defined(__MODEL_USER__)
    if ((ret = dal_waMon_init()) != RT_ERR_OK)
        return ret;
#endif

#if defined(CONFIG_SDK_KERNEL_LINUX_KERNEL_MODE)
#if defined(CONFIG_SDK_RTL8390) && !defined(__MODEL_USER__)
    if ((ret = dal_linkFaultMon_init()) != RT_ERR_OK)
        return ret;
#endif
#endif
    /* if above thing work correct, Set Init flag on and return RT_ERROR_NONE */
    mgmt_init   = INIT_COMPLETED;
    return RT_ERR_OK;

} /* end of dal_mgmt_init */


/* Function Name:
 *      dal_mgmt_initDevice
 * Description:
 *      Initilize specified device(hook related driver, initialize database of device in MGMT,
 *      execute initialized function of each component
 * Input:
 *      unit    - the unit to be initialized
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED   - initialize fail
 *      RT_ERR_OK       - initialize success
 *      RT_ERR_UNIT_ID  - error unit id
 * Note:
 *      RTK must call this function before do other kind of action on this unit.
 */
int32 dal_mgmt_initDevice(uint32 unit)
{
    int32   ret;
    hal_control_t *pHal_ctrl;

    /* Check  whether DAL MGMT was initialized */
    RT_INIT_CHK(mgmt_init);

    /* Check whether device is exist in lower layer(MAL) */
    if ((pHal_ctrl = hal_ctrlInfo_get(unit)) == NULL)
    {
        return RT_ERR_CHIP_NOT_FOUND;
    }

    /* Hook related driver and initialize database of device via dal_mgmt_attachDevice.
       If fail, return error code.
     */
    ret = dal_mgmt_attachDevice(unit);
    if (RT_ERR_OK != ret){
        RT_DBG(LOG_MAJOR_ERR, (MOD_INIT|MOD_DAL), "dal_mgmt_attachDevice Failed!!");
        return ret;
    }
    RT_DBG(LOG_EVENT, (MOD_INIT|MOD_DAL), "dal_mgmt_attachDevice Completed!!");

    /* Call init function of this device. if fail, return error code */
    ret = RT_MAPPER(unit)->_init(unit);
    if (RT_ERR_OK != ret){
        RT_DBG(LOG_MAJOR_ERR, (MOD_INIT|MOD_DAL), "RT_MAPPER(unit)->_init Failed!!");
        return ret;
    }
    RT_DBG(LOG_EVENT, (MOD_INIT|MOD_DAL), "RT_MAPPER(unit)->_init Completed!!");

    ret = dal_linkMon_devInit(unit);
    if (RT_ERR_OK != ret){
        RT_DBG(LOG_MAJOR_ERR, (MOD_INIT|MOD_DAL), "dal_linkMon_devInit Failed!!");
        return ret;
    }
    RT_DBG(LOG_EVENT, (MOD_INIT|MOD_DAL), "dal_linkMon_devInit Completed!!");

#if defined(CONFIG_SDK_AUTO_COMBO_MEDIA_BY_GPIO)
    {
    rtk_portmask_t  los_swScan_portmask;

    los_swScan_portmask = pHal_ctrl->pDev_info->pPortinfo->ge_combo.portmask;
    ret = dal_losMon_swScanPorts_set(unit, &los_swScan_portmask);
    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_MAJOR_ERR, (MOD_INIT|MOD_DAL), "dal_losMon_swScanPorts_set Failed!!");
        return ret;
    }
    RT_DBG(LOG_EVENT, (MOD_INIT|MOD_DAL), "dal_losMon_swScanPorts_set Success!!");

    /* Combo port LOS fiber-prefered mechanism */
    ret = dal_losMon_enable(RTK_LOSMON_SCAN_INTERVAL_MIN);
    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_MAJOR_ERR, (MOD_INIT|MOD_DAL), "dal_losMon_enable Failed!!");
        return ret;
    }
    RT_DBG(LOG_EVENT, (MOD_INIT|MOD_DAL), "dal_losMon_enable Success!!");
    }
#endif

#if defined(CONFIG_SDK_RTL8328)
#if defined(CONFIG_SDK_WA_LIMIT_LEARN_COUNT) || defined(CONFIG_SDK_WA_FORWARD_TABLE)
    /* Use macMon thread to patch code for RTL8328M/RTL8328S/RTL8328L chips */
    if (RTL8328M_CHIP_ID == pHal_ctrl->chip_id || RTL8328S_CHIP_ID == pHal_ctrl->chip_id || RTL8328L_CHIP_ID == pHal_ctrl->chip_id)
    {
        ret = dal_macMon_enable(1000000);
        if (RT_ERR_OK != ret)
        {
            RT_DBG(LOG_MAJOR_ERR, (MOD_INIT|MOD_DAL), "dal_macMon_enable Failed!!");
            return ret;
        }
        RT_DBG(LOG_EVENT, (MOD_INIT|MOD_DAL), "dal_macMon_enable Success!!");
    }
#endif
#if defined(CONFIG_SDK_WA_LINKDOWN_PWR_SAVING) || defined(CONFIG_SDK_WA_BACK_PRESSURE) || defined(CONFIG_SDK_WA_INTRALINK_DELAY) || defined(CONFIG_SDK_WA_EEE_COMPATIBLE) || defined(CONFIG_SDK_WA_RTL8231_RESET) || defined(CONFIG_SDK_WA_PKTBUF_WATCHDOG)
    /* Use waMon thread to patch code for RTL8328M/RTL8328S chips */
    if (RTL8328M_CHIP_ID == pHal_ctrl->chip_id || RTL8328S_CHIP_ID == pHal_ctrl->chip_id || RTL8328L_CHIP_ID == pHal_ctrl->chip_id)
    {
        ret = dal_waMon_enable(1000000);
        if (RT_ERR_OK != ret)
        {
            RT_DBG(LOG_MAJOR_ERR, (MOD_INIT|MOD_DAL), "dal_waMon_enable Failed!!");
            return ret;
        }
        RT_DBG(LOG_EVENT, (MOD_INIT|MOD_DAL), "dal_waMon_enable Success!!");
    }
#endif
#endif

#if defined(CONFIG_SDK_RTL8380)
    /* Use waMon thread to patch code for RTL8332M/RTL8330M chips */
    if (HAL_IS_RTL8380_FAMILY_ID(unit) || HAL_IS_RTL8330_FAMILY_ID(unit))
    {
        ret = dal_waMon_enable(1000000);
        if (RT_ERR_OK != ret)
        {
            RT_DBG(LOG_MAJOR_ERR, (MOD_INIT|MOD_DAL), "dal_waMon_enable Failed!!");
            return ret;
        }
        RT_DBG(LOG_EVENT, (MOD_INIT|MOD_DAL), "dal_waMon_enable Success!!");
    }
#endif

#if (defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8350)) && !defined(__MODEL_USER__)
		/* Use waMon thread to patch code for RTL839xM/RTL838xM chips */
		if (HAL_IS_RTL8390_FAMILY_ID(unit) || HAL_IS_RTL8350_FAMILY_ID(unit))
		{
			ret = dal_waMon_enable(1000000);
			if (RT_ERR_OK != ret)
			{
				RT_DBG(LOG_MAJOR_ERR, (MOD_INIT|MOD_DAL), "dal_waMon_enable Failed!!");
				return ret;
			}
			RT_DBG(LOG_EVENT, (MOD_INIT|MOD_DAL), "dal_waMon_enable Success!!");
		}
#endif

    return ret;

} /* dal_mgmt_initDevice() */


/* Function Name:
 *      dal_mgmt_attachDevice
 * Description:
 *      Initilize specified device(hook related driver, initialize database of device in MGMT,
 *      execute initialized function of each component
 * Input:
 *      unit    - the unit to be initialized
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED       - initialize fail
 *      RT_ERR_OK           - initialize success
 * Note:
 *      RTK must call this function before do other kind of action on this unit.
 */
static int32 dal_mgmt_attachDevice(uint32 unit)
{

    /* Check  whether DAL MGMT was initialized */
    RT_INIT_CHK(mgmt_init);

    /* Lock semaphore */
    SEM_LOCK();

    /* Check whether device was initilized, */
    if (0 != RT_MGMT(unit)){
        SEM_UNLOCK();
        /* if inited, return error */
        RT_LOG(LOG_DEBUG, MOD_DAL, "this unit is alread inited");
        return RT_ERR_FAILED;
    }

    /* Allocate memory for device database and initilize it. */
    pMgmt_node[unit] = (dal_mgmt_info_t *)osal_alloc(sizeof(dal_mgmt_info_t));
    if (0 == pMgmt_node[unit]){
        SEM_UNLOCK();
        RT_LOG(LOG_DEBUG, MOD_DAL, "failed to create management node");
        return RT_ERR_FAILED;
    }
    osal_memset(pMgmt_node[unit], 0, sizeof(dal_mgmt_info_t));

    if ((RT_MAPPER(unit) = dal_mgmt_find_mapper(unit)) == NULL)
    {
        osal_free(pMgmt_node[unit]);
        pMgmt_node[unit] = 0;
        SEM_UNLOCK();
        RT_LOG(LOG_DEBUG, MOD_DAL, "failed to find mapper");
        return RT_ERR_FAILED;
    }

    RT_MGMT(unit)->init = INIT_COMPLETED;

    SEM_UNLOCK();

    /* (TBD)Execute platform init function(will be used to init some information and capability) */


    return RT_ERR_OK;

} /* end of dal_mgmt_attachDevice */

/* Function Name:
 *      dal_mgmt_attachDevice
 * Description:
 *      Initilize specified device(hook related driver, initialize database of device in MGMT,
 *      execute initialized function of each component
 * Input:
 *      unit    - the unit to be initialized
 * Output:
 *      None
 * Return:
 *      NULL       - FAILED to find mapper for this chip
 * Note:
 *      RTK must call this function before do other kind of action on this unit.
 */
static dal_mapper_t *
dal_mgmt_find_mapper(uint32 unit)
{
    uint32  mapper_size = sizeof(dal_mapper_database)/sizeof(dal_mapper_info_t);
    uint32  mapper_index;
    hal_control_t *pHal_ctrl = hal_ctrlInfo_get(unit);

    for (mapper_index = 0; mapper_index < mapper_size; mapper_index++)
    {
        if (dal_mapper_database[mapper_index].chip_id == pHal_ctrl->chip_id)
        {
            return dal_mapper_database[mapper_index].pMapper;
        }
    }

    return NULL;
} /* end of dal_mgmt_find_mapper */
