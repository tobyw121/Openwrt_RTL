/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * $Revision: 19547 $
 * $Date: 2011-07-12 14:41:07 +0800 (Tue, 12 Jul 2011) $
 *
 * Purpose : Definition those public STP APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) spanning tree (1D, 1w and 1s)
 *
 */

/*  
 * Include Files 
 */
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_bitop.h>
#include <common/util/rt_util.h>
#include <common/debug/rt_log.h>
#include <osal/sem.h>
#include <osal/lib.h>
#include <osal/memory.h>
#include <hal/chipdef/allmem.h>
#include <hal/chipdef/allreg.h>
#include <hal/chipdef/esw/rtk_esw_table_struct.h>
#include <hal/chipdef/esw/rtk_esw_reg_struct.h>
#include <hal/mac/reg.h>
#include <hal/mac/mem.h>
#include <hal/common/halctrl.h>
#include <dal/esw/dal_esw_stp.h>
#include <rtk/default.h>
#include <rtk/stp.h>

/* 
 * Data Declaration 
 */
static uint32               stp_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         stp_sem[RTK_MAX_NUM_OF_UNIT];

static uint32               *pMsti_valid[RTK_MAX_NUM_OF_UNIT];

/*
 * Macro Definition
 */
/* vlan semaphore handling */
#define STP_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(stp_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_DAL|MOD_STP),"semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)
#define STP_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(stp_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_DAL|MOD_STP),"semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)

#define MSTI_VALID_IS_SET(unit, msti)  BITMAP_IS_SET(pMsti_valid[unit], msti)
#define MSTI_VALID_IS_CLEAR(unit, msti)  BITMAP_IS_CLEAR(pMsti_valid[unit], msti)
#define MSTI_VALID_SET(unit, msti)  BITMAP_SET(pMsti_valid[unit], msti)
#define MSTI_VALID_CLEAR(unit, msti)  BITMAP_CLEAR(pMsti_valid[unit], msti)


/* 
 * Function Declaration 
 */
static int32 _dal_esw_stp_init_config(uint32 unit);

/* Function Name:
 *      dal_esw_stp_init
 * Description:
 *      Initialize stp module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      Must initialize stp module before calling any stp APIs.
 */
int32
dal_esw_stp_init(uint32 unit)
{
    int32   ret;
    uint32  num_of_mst_1bitlist;
    
    stp_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    stp_sem[unit] = osal_sem_mutex_create();
    if (0 == stp_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_STP), "semaphore create failed");
        return RT_ERR_FAILED;
    }
    
    /* Allocate and initilize memory for STP valid database */
    num_of_mst_1bitlist = (HAL_MAX_NUM_OF_MSTI(unit) + 31) >> 5;
    pMsti_valid[unit] = (uint32 *)osal_alloc(num_of_mst_1bitlist * sizeof(uint32));
    if (NULL == pMsti_valid[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_STP|MOD_DAL), "memory allocate failed");
        return RT_ERR_FAILED;
    }
    osal_memset(pMsti_valid[unit], 0, (num_of_mst_1bitlist * sizeof(uint32)));
    
    /* set init flag to complete init */
    stp_init[unit] = INIT_COMPLETED;
    
    if ((ret = _dal_esw_stp_init_config(unit)) != RT_ERR_OK)
    {
        stp_init[unit] = INIT_NOT_COMPLETED;
        osal_free(pMsti_valid[unit]);
        pMsti_valid[unit] = 0;
        RT_ERR(ret, (MOD_STP|MOD_DAL), "");
        return ret;
    }
    
    return RT_ERR_OK;
} /* end of dal_esw_stp_init */


/* Function Name:
 *      dal_esw_stp_mstpInstance_create
 * Description:
 *      Create one specified mstp instance of the specified device.
 * Input:
 *      unit - unit id
 *      msti - mstp instance
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_UNIT_ID    - invalid unit id
 *      RT_ERR_MSTI       - invalid msti
 *      RT_ERR_MSTI_EXIST - MSTI is already exist.
 * Note:
 *      The msti valid range is 0 .. RTK_STP_INSTANCE_ID_MAX-1
 */
int32 
dal_esw_stp_mstpInstance_create(uint32 unit, uint32 msti)
{    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STP), "unit=%d, msti=%d", unit, msti); 
    
    /* check Init status */
    RT_INIT_CHK(stp_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(msti >= HAL_MAX_NUM_OF_MSTI(unit), RT_ERR_MSTI);
    RT_PARAM_CHK(MSTI_VALID_IS_SET(unit, msti), RT_ERR_MSTI_EXIST);    
    
    STP_SEM_LOCK(unit);
    /* Set valid bit of MSTI */
    MSTI_VALID_SET(unit, msti);
    
    STP_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /*  dal_esw_stp_mstpInstance_create */


/* Function Name:
 *      dal_esw_stp_mstpInstance_destroy
 * Description:
 *      Destroy one specified mstp instance from the specified device.
 * Input:
 *      unit - unit id
 *      msti - mstp instance
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_MSTI     - invalid msti
 *      RT_ERR_MSTI_NOT_EXIST   - msti is not exist
 * Note:
 *      The msti valid range is 0 .. RTK_STP_INSTANCE_ID_MAX-1
 */
int32
dal_esw_stp_mstpInstance_destroy(uint32 unit, uint32 msti)
{
    uint32 max_port;
    uint32 port;
    int32  ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STP), "unit=%d, msti=%d", unit, msti); 

    /* check Init status */
    RT_INIT_CHK(stp_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(msti >= HAL_MAX_NUM_OF_MSTI(unit), RT_ERR_MSTI);
    RT_PARAM_CHK(MSTI_VALID_IS_CLEAR(unit, msti), RT_ERR_MSTI_NOT_EXIST);        
    
    /*set to default port state before remove the instance*/
    max_port = HAL_GET_MAX_PORT(unit);
    for (port = 0; port < max_port; port++)
    {
        
        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }
        
        if (HAL_IS_CPU_PORT(unit, port))
        {
            if ((ret = dal_esw_stp_mstpState_set(unit, msti, port, STP_STATE_FORWARDING)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_STP|MOD_DAL), "");
                return ret;
            }
        } 
        else 
        {
            if ((ret = dal_esw_stp_mstpState_set(unit, msti, port, RTK_DEFAULT_STP_PORT_STATE)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_STP|MOD_DAL), "");
                return ret;
            }            
        }
    }

    STP_SEM_LOCK(unit);
    
    /* clear valid bit of MSTI */
    MSTI_VALID_CLEAR(unit, msti);
    
    STP_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_stp_mstpInstance_destroy */


/* Function Name:
 *      dal_esw_stp_isMstpInstanceExist_get
 * Description:
 *      Check one specified mstp instance is existing or not in the specified device.
 * Input:
 *      unit         - unit id
 *      msti         - mstp instance
 * Output:
 *      pMsti_exist - mstp instance exist or not?
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_MSTI         - invalid msti
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The pMsti_exist value as following:
 *      0: this mstp instance not exist
 *      1: this mstp instance exist
 */
int32 
dal_esw_stp_isMstpInstanceExist_get(uint32 unit, uint32 msti, uint32 *pMsti_exist)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STP), "unit=%d, msti=%d", unit, msti); 
    
    /* check Init status */
    RT_INIT_CHK(stp_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((msti >= HAL_MAX_NUM_OF_MSTI(unit)), RT_ERR_MSTI);
    RT_PARAM_CHK((NULL == pMsti_exist), RT_ERR_NULL_POINTER);
    
    STP_SEM_LOCK(unit);
    /* clear valid bit of MSTI */
    *pMsti_exist = (MSTI_VALID_IS_SET(unit, msti))?1:0;
    
    STP_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STP), "pMsti_exist=%x", *pMsti_exist); 
    
    return RT_ERR_OK;
} /* end of dal_esw_stp_isMstpInstanceExist_get */


/* Function Name:
 *      dal_esw_stp_mstpState_get
 * Description:
 *      Get port spanning tree state of the msti from the specified device.
 * Input:
 *      unit       - unit id
 *      msti       - multiple spanning tree instance
 *      port       - port id
 * Output:
 *      pStp_state - pointer buffer of spanning tree state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_MSTI         - invalid msti
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. For single spanning tree mode, input CIST0 (msti=0).
 *      2. Spanning tree state as following
 *         - STP_STATE_DISABLED
 *         - STP_STATE_BLOCKING
 *         - STP_STATE_LEARNING
 *         - STP_STATE_FORWARDING
 */
int32
dal_esw_stp_mstpState_get(uint32 unit, uint32 msti, rtk_port_t port, rtk_stp_state_t *pStp_state)
{
    int32   ret;
    spt_entry_t pStpEntry;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STP), "unit=%d, msti=%d, port=%d", 
           unit, msti, port); 
    
    /* check Init status */
    RT_INIT_CHK(stp_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pStp_state), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((msti >= HAL_MAX_NUM_OF_MSTI(unit)), RT_ERR_MSTI);
    RT_PARAM_CHK(MSTI_VALID_IS_CLEAR(unit, msti), RT_ERR_MSTI_NOT_EXIST);

    STP_SEM_LOCK(unit);

    osal_memset(&pStpEntry, 0, sizeof(spt_entry_t));
    
    if((ret = table_read(unit, ESW_SPTt, msti, (uint32 *) &pStpEntry)) != RT_ERR_OK)
    {
        STP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_STP), "");        
        return ret;
    }

    if(port <= 15)
    {
        if ((ret = table_field_get(unit, ESW_SPTt, ESW_SPT_PORT0_SPT_STATEf - port, pStp_state, (uint32 *) &pStpEntry)) != RT_ERR_OK)
        {
            STP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_STP), "");  
            return ret;
        }
    }
    else
    {
        if ((ret = table_field_get(unit, ESW_SPTt, ESW_SPT_PORT16_SPT_STATEf - port + 16, pStp_state, (uint32 *) &pStpEntry)) != RT_ERR_OK)
        {
            STP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_STP), "");  
            return ret;
        }
    }    
    
    STP_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STP), "pStp_state=%x", *pStp_state); 
    
    return RT_ERR_OK;
} /* end of dal_esw_stp_mstpState_get */

/* Function Name:
 *      dal_esw_stp_mstpState_set
 * Description:
 *      Set port spanning tree state of the msti to the specified device.
 * Input:
 *      unit      - unit id
 *      msti      - multiple spanning tree instance
 *      port      - port id
 *      stp_state - spanning tree state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_UNIT_ID    - invalid unit id
 *      RT_ERR_MSTI       - invalid msti
 *      RT_ERR_PORT_ID    - invalid port id
 *      RT_ERR_MSTP_STATE - invalid spanning tree status
 * Note:
 *      1. For single spanning tree mode, input CIST0 (msti=0).
 *      2. Spanning tree state as following
 *         - STP_STATE_DISABLED
 *         - STP_STATE_BLOCKING
 *         - STP_STATE_LEARNING
 *         - STP_STATE_FORWARDING
 */
int32
dal_esw_stp_mstpState_set(uint32 unit, uint32 msti, rtk_port_t port, rtk_stp_state_t stp_state)
{
    int32   ret;
    spt_entry_t pStpEntry;
    uint32 val = stp_state;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STP), "unit=%d, msti=%d, port=%d, stp_state=%d", 
           unit, msti, port, stp_state); 
    
    /* check Init status */
    RT_INIT_CHK(stp_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((stp_state >= STP_STATE_END), RT_ERR_MSTP_STATE);
    RT_PARAM_CHK((msti >= HAL_MAX_NUM_OF_MSTI(unit)), RT_ERR_MSTI);
    RT_PARAM_CHK(MSTI_VALID_IS_CLEAR(unit, msti), RT_ERR_MSTI_NOT_EXIST);

    STP_SEM_LOCK(unit);

    osal_memset(&pStpEntry, 0, sizeof(spt_entry_t));
    
    if((ret = table_read(unit, ESW_SPTt, msti, (uint32 *) &pStpEntry)) != RT_ERR_OK)
    {
        STP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_STP), "");        
        return ret;
    }

    if(port <= 15)
    {
        if ((ret = table_field_set(unit, ESW_SPTt, ESW_SPT_PORT0_SPT_STATEf - port, &val, (uint32 *) &pStpEntry)) != RT_ERR_OK)
        {
            STP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_STP), "");  
            return ret;
        }
    }
    else
    {
        if ((ret = table_field_set(unit, ESW_SPTt, ESW_SPT_PORT16_SPT_STATEf - port + 16, &val, (uint32 *) &pStpEntry)) != RT_ERR_OK)
        {
            STP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_STP), "");  
            return ret;
        }
    }  
    
    if((ret = table_write(unit, ESW_SPTt, msti, (uint32 *) &pStpEntry)) != RT_ERR_OK)
    {
        STP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_STP), "");        
        return ret;
    }
    STP_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_stp_mstpState_set */

/* Function Name:
 *      _dal_esw_stp_init_config
 * Description:
 *      Initialize default configuration for stp module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Must initialize stp module before calling this API
 */
static int32
_dal_esw_stp_init_config(uint32 unit)
{
    int32   ret;
    uint32  port, max_port;
    
    if ((ret = dal_esw_stp_mstpInstance_create(unit, RTK_DEFAULT_MSTI)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_STP|MOD_DAL), "");
        return ret;
    }
    
    max_port = HAL_GET_MAX_PORT(unit);
    for (port = 0; port < max_port; port++)
    {
        
        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }
        
        if (HAL_IS_CPU_PORT(unit, port))
        {
            if ((ret = dal_esw_stp_mstpState_set(unit, RTK_DEFAULT_MSTI, port, STP_STATE_FORWARDING)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_STP|MOD_DAL), "");
                return ret;
            }
        } 
        else 
        {
            if ((ret = dal_esw_stp_mstpState_set(unit, RTK_DEFAULT_MSTI, port, RTK_DEFAULT_STP_PORT_STATE)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_STP|MOD_DAL), "");
                return ret;
            }            
        }
    }
    
    return RT_ERR_OK;
} /* end of _dal_esw_stp_init_config */


