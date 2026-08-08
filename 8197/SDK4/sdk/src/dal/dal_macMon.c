/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * $Revision: 30053 $
 * $Date: 2012-06-19 14:12:07 +0800 (Tue, 19 Jun 2012) $
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
#include <osal/thread.h>
#include <hal/chipdef/allmem.h>
#include <hal/chipdef/esw/rtk_esw_table_struct.h>
#include <hal/common/halctrl.h>
#include <hal/mac/mem.h>
#include <dal/esw/dal_esw_l2.h>
#include <dal/esw/dal_esw_port.h>
#include <dal/dal_common.h>
#include <dal/dal_macMon.h>
#include <rtk/default.h>

/* 
 * Symbol Definition
 */
#undef  CONFIG_SDK_WA_LIMIT_LEARN_COUNT
#define CONFIG_SDK_WA_FORWARD_TABLE

/* workaround monitor control block */
typedef struct dal_macMon_cb_s {
    osal_thread_t       thread_id;
    uint32              scan_interval_us;
} dal_macMon_cb_t;


/*
 * Data Declaration
 */
static uint32   macMon_init;
static uint32   mac_change_sem;
static dal_macMon_cb_t   *pMacMon_cb;

/*
 * Macro Declaration
 */ 

/*
 * Function Declaration
 */

static void _dal_macMon_thread(void *pInput);


/* Module Name : */

/* Function Name: 
 *      dal_macMon_init
 * Description: 
 *      Initial Mac Constraint Workaround Monitor Component
 * Input:  
 *      None
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_FAILED - initialize fail
 *      RT_ERR_OK     - initialize success
 * Note: 
 *      
 */ 
int32 dal_macMon_init(void)
{
    
    /* init value */
    macMon_init = INIT_NOT_COMPLETED;
    
    /* allocate memory for control block */
    pMacMon_cb = osal_alloc(sizeof(dal_macMon_cb_t));
    
    if (0 == pMacMon_cb){
        RT_LOG(LOG_DEBUG, MOD_DAL, "mac constraint workaround monitor allocate memory failed");
        return RT_ERR_FAILED;
    }
    
    /* create semaphore for sync, this semaphore is empty in beginning */
    mac_change_sem = osal_sem_create(0);
    
    if (0 == mac_change_sem){
        osal_free(pMacMon_cb);
        RT_LOG(LOG_DEBUG, MOD_DAL, "mac constraint workaround monitor semaphore create failed");
        return RT_ERR_FAILED;
    }
    
    
    osal_memset(pMacMon_cb, 0, sizeof(dal_macMon_cb_t));
    
    macMon_init = INIT_COMPLETED;
    
    return RT_ERR_OK;
    
} /* end of dal_macMon_init */

/* Function Name: 
 *      dal_macMon_enable
 * Description: 
 *      Enable mac constraint workaround monitor thread
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
 *      When enable mac constraint workaround monitor thread
 *      
 */ 
int32 dal_macMon_enable(uint32 scan_interval_us)
{
    uint32  unit = 0;
    hal_control_t *pHal_ctrl;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "scan_interval_us=%u", 
           scan_interval_us); 
    
    /* check Init status */
    RT_INIT_CHK(macMon_init);
    
    /* parameter check */
    RT_PARAM_CHK((scan_interval_us < RTK_MACMON_SCAN_INTERVAL_MIN), RT_ERR_OUT_OF_RANGE);
    
    pMacMon_cb->scan_interval_us = scan_interval_us;
    
    if ((pMacMon_cb->thread_id) != 0)
    {
        RT_ERR(pMacMon_cb->thread_id, (MOD_DAL|MOD_PORT), "");
        return RT_ERR_THREAD_EXIST;
    }
    
    /* Check whether device is exist in lower layer(HAL) */
    if ((pHal_ctrl = hal_ctrlInfo_get(unit)) == NULL)
    {
        return RT_ERR_FAILED;
    }

    if ((RTL8328M_CHIP_ID == pHal_ctrl->chip_id || RTL8328S_CHIP_ID == pHal_ctrl->chip_id || RTL8328L_CHIP_ID == pHal_ctrl->chip_id) &&
        (CHIP_REV_ID_A == pHal_ctrl->chip_rev_id || CHIP_REV_ID_B == pHal_ctrl->chip_rev_id))
    {
        /* create thread */
        pMacMon_cb->thread_id = osal_thread_create("MAC Monitor Thread", RTK_DEFAULT_MAC_MON_STACK_SIZE, RTK_DEFAULT_MAC_MON_THREAD_PRI
                                , (void *)_dal_macMon_thread, NULL);
        
        if (0 == (pMacMon_cb->thread_id))
        {
            RT_ERR(pMacMon_cb->thread_id, (MOD_DAL|MOD_PORT), "");
            return RT_ERR_THREAD_CREATE_FAILED;
        }
    }
    
    return RT_ERR_OK;
} /* end of dal_macMon_enable */

/* Function Name: 
 *      dal_macMon_disable
 * Description: 
 *      Disable mac constraint workaround scan thread
 * Input:  
 *      None.
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_OK       - initialize success
 *      RT_ERR_FAILED   - initialize fail
 *      RT_ERR_NOT_INIT - The module is not initial
 * Note: 
 *      When disable mac constraint workaround monitor thread
 */ 
int32 dal_macMon_disable(void)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), ""); 
    
    /* check Init status */
    RT_INIT_CHK(macMon_init);
    
    /* parameter check */
    
    /* reset scan_interval_us to 0, thread will suicide after finish all waiting job */
    pMacMon_cb->scan_interval_us = 0;
    
    /* let thread continue */
    osal_sem_give(mac_change_sem);
    
    return RT_ERR_OK;
    
} /* end of dal_macMon_disable */

/* Function Name: 
 *      _dal_macMon_thread
 * Description: 
 *      Unregister callback function for mac constraint notification 
 * Input:  
 *      None.
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_FAILED - initialize fail
 *      RT_ERR_OK     - initialize success
 * Note: 
 *      
 */ 
static void _dal_macMon_thread(void *pInput)
{
#define SCAN_ENTRY_SIZE_IN_ONE_TIME     (1024)
    uint32  unit = 0, aging = 300;
#if defined(CONFIG_SDK_RTL8328)
#if defined(CONFIG_SDK_WA_LIMIT_LEARN_COUNT)
    uint32  old_aging = 0;
    uint32  block_idx = 0;
#endif
#endif
    uint32  total_block = 0;
    uint32  l2_tableSize;
    uint32  l2cam_tableSize;
    hal_control_t *pHal_ctrl;

    /* Check whether device is exist in lower layer(HAL) */
    if ((pHal_ctrl = hal_ctrlInfo_get(unit)) == NULL)
    {
        return; /* RT_ERR_CHIP_NOT_FOUND */
    }

    /* get the l2 and l2cam table size */
    if (table_size_get(unit, ESW_CAM_ISFTIDXt, &l2cam_tableSize) != RT_ERR_OK)
    {
        return;
    }
    
    if (table_size_get(unit, ESW_L2_ISFTIDXt, &l2_tableSize) != RT_ERR_OK)
    {
        return;
    }

    total_block = ((l2cam_tableSize + l2_tableSize - 1)/SCAN_ENTRY_SIZE_IN_ONE_TIME + 1);

    /* forever loop */
    while (pMacMon_cb->scan_interval_us != 0)
    {
        dal_esw_l2_aging_get(unit, &aging);

        /* wait semaphore for mac constraint workaround scan interval */
        osal_sem_take(mac_change_sem, pMacMon_cb->scan_interval_us);

        if (RTL8328M_CHIP_ID == pHal_ctrl->chip_id || RTL8328S_CHIP_ID == pHal_ctrl->chip_id || RTL8328L_CHIP_ID == pHal_ctrl->chip_id)
        {
#if defined(CONFIG_SDK_RTL8328)
#if defined(CONFIG_SDK_WA_LIMIT_LEARN_COUNT)
            /* Scan L2 table for patching mac limiting count problem in RTL8328M/RTL8328S
             * When aging value of the l2 entry is equal 1, delete the entry to avoid 
             * the aging value become 0 and cause the mac limiting count wrong.
             */
            if (CHIP_REV_ID_A == pHal_ctrl->chip_rev_id)
            {
                if (old_aging != aging)
                {
                    old_aging = aging;
                    RT_LOG(LOG_DEBUG, MOD_DAL, "mac constraint workaround process whole table");
                    dal_esw_l2_limitLearningCnt_workaround(unit);
                    block_idx = (block_idx + 1) % total_block;
                }
                else
                {
                    /* Scan the mac address table 1K range in one time */
                    RT_LOG(LOG_DEBUG, MOD_DAL, "mac constraint workaround process entry %d to %d", block_idx * SCAN_ENTRY_SIZE_IN_ONE_TIME, (block_idx + 1) * SCAN_ENTRY_SIZE_IN_ONE_TIME - 1);
                    dal_esw_l2_limitLearningCntByRange_workaround(unit, block_idx * SCAN_ENTRY_SIZE_IN_ONE_TIME, (block_idx + 1) * SCAN_ENTRY_SIZE_IN_ONE_TIME - 1);
                    block_idx = (block_idx + 1) % total_block;
                }
            }
#endif
#if defined(CONFIG_SDK_WA_FORWARD_TABLE)
            /* Scan forward table for patching portmask problem in RTL8328M/RTL8328S/RTL8328L
             * Rewrite the shadow data to forward table periodical in R0A & R0B chip
             */
            if (CHIP_REV_ID_A == pHal_ctrl->chip_rev_id || CHIP_REV_ID_B == pHal_ctrl->chip_rev_id)
            {
                RT_LOG(LOG_DEBUG, MOD_DAL, "forward table workaround process whole table");
                dal_esw_l2_forwardTablePortmask_workaround(unit);
            }
#endif
#endif
        }

        if (aging >= 300 || aging == 0)
            pMacMon_cb->scan_interval_us = 10 * 1000000;
        else
            pMacMon_cb->scan_interval_us = aging * 1000000 / 30;
    }
    
    osal_thread_exit(0);
    
    return;
} /* end of _dal_macMon_thread */
