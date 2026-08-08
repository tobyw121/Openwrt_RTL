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
 * $Revision: 21577 $
 * $Date: 2011-08-27 12:02:46 +0800 (Sat, 27 Aug 2011) $
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
#include <hal/chipdef/ssw/rtk_ssw_table_struct.h>
#include <hal/chipdef/ssw/rtk_ssw_reg_struct.h>
#include <hal/mac/reg.h>
#include <hal/mac/mem.h>
#include <hal/common/halctrl.h>
#include <dal/ssw/dal_ssw_stp.h>
#include <rtk/default.h>
#include <rtk/stp.h>

/* 
 * Symbol Definition 
 */


/* 
 * Data Declaration 
 */
static uint32               stp_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         stp_sem[RTK_MAX_NUM_OF_UNIT];

static uint32               *pMsti_valid[RTK_MAX_NUM_OF_UNIT];

const static uint16 mstpStateControl_regidx[] = {SSW_MULTIPLE_SPANNING_TREE_STATE_CONTROL0r, SSW_MULTIPLE_SPANNING_TREE_STATE_CONTROL1r, 
                                                 SSW_MULTIPLE_SPANNING_TREE_STATE_CONTROL2r, SSW_MULTIPLE_SPANNING_TREE_STATE_CONTROL3r,
                                                 SSW_MULTIPLE_SPANNING_TREE_STATE_CONTROL4r, SSW_MULTIPLE_SPANNING_TREE_STATE_CONTROL5r,
                                                 SSW_MULTIPLE_SPANNING_TREE_STATE_CONTROL6r, SSW_MULTIPLE_SPANNING_TREE_STATE_CONTROL7r,
                                                 SSW_MULTIPLE_SPANNING_TREE_STATE_CONTROL8r, SSW_MULTIPLE_SPANNING_TREE_STATE_CONTROL9r,
                                                 SSW_MULTIPLE_SPANNING_TREE_STATE_CONTROL10r,SSW_MULTIPLE_SPANNING_TREE_STATE_CONTROL11r,
                                                 SSW_MULTIPLE_SPANNING_TREE_STATE_CONTROL12r,SSW_MULTIPLE_SPANNING_TREE_STATE_CONTROL13r,                                          
                                                 SSW_MULTIPLE_SPANNING_TREE_STATE_CONTROL14r,SSW_MULTIPLE_SPANNING_TREE_STATE_CONTROL15r,
                                                 SSW_MULTIPLE_SPANNING_TREE_STATE_CONTROL16r,SSW_MULTIPLE_SPANNING_TREE_STATE_CONTROL17r,
                                                 SSW_MULTIPLE_SPANNING_TREE_STATE_CONTROL18r,SSW_MULTIPLE_SPANNING_TREE_STATE_CONTROL19r,
                                                 SSW_MULTIPLE_SPANNING_TREE_STATE_CONTROL20r,SSW_MULTIPLE_SPANNING_TREE_STATE_CONTROL21r,
                                                 SSW_MULTIPLE_SPANNING_TREE_STATE_CONTROL22r,SSW_MULTIPLE_SPANNING_TREE_STATE_CONTROL23r,
                                                 SSW_MULTIPLE_SPANNING_TREE_STATE_CONTROL24r,SSW_MULTIPLE_SPANNING_TREE_STATE_CONTROL25r,
                                                 SSW_MULTIPLE_SPANNING_TREE_STATE_CONTROL26r,SSW_MULTIPLE_SPANNING_TREE_STATE_CONTROL27r,
                                                 SSW_MULTIPLE_SPANNING_TREE_STATE_CONTROL28r};

const static uint16 mstpState_fieldidx[] = {SSW_MSTI0_STATE_PNf, SSW_MSTI1_STATE_PNf, SSW_MSTI2_STATE_PNf, SSW_MSTI3_STATE_PNf, SSW_MSTI4_STATE_PNf, SSW_MSTI5_STATE_PNf, SSW_MSTI6_STATE_PNf, SSW_MSTI7_STATE_PNf, SSW_MSTI8_STATE_PNf, SSW_MSTI9_STATE_PNf\
                         , SSW_MSTI10_STATE_PNf, SSW_MSTI11_STATE_PNf, SSW_MSTI12_STATE_PNf, SSW_MSTI13_STATE_PNf, SSW_MSTI14_STATE_PNf, SSW_MSTI15_STATE_PNf};
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
static int32 _dal_ssw_stp_init_config(uint32 unit);

/* Module Name : STP */

/* Function Name:
 *      dal_ssw_stp_init
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
dal_ssw_stp_init(uint32 unit)
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
    
    if ((ret = _dal_ssw_stp_init_config(unit)) != RT_ERR_OK)
    {
        stp_init[unit] = INIT_NOT_COMPLETED;
        osal_free(pMsti_valid[unit]);
        pMsti_valid[unit] = 0;
        RT_ERR(ret, (MOD_STP|MOD_DAL), "");
        return ret;
    }
    
    return RT_ERR_OK;
} /* end of dal_ssw_stp_init */


/* Function Name:
 *      dal_ssw_stp_mstpInstance_create
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
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_MSTI     - invalid msti
 *      RT_ERR_MSTI_EXIST - MSTI is already exist.
 * Note:
 *      The msti valid range is 0 .. RTK_STP_INSTANCE_ID_MAX-1
 */
int32 
dal_ssw_stp_mstpInstance_create(uint32 unit, uint32 msti)
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
} /* end of dal_ssw_stp_mstpInstance_create */


/* Function Name:
 *      dal_ssw_stp_mstpInstance_destroy
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
dal_ssw_stp_mstpInstance_destroy(uint32 unit, uint32 msti)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STP), "unit=%d, msti=%d", unit, msti); 

    /* check Init status */
    RT_INIT_CHK(stp_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(msti >= HAL_MAX_NUM_OF_MSTI(unit), RT_ERR_MSTI);
    RT_PARAM_CHK(MSTI_VALID_IS_CLEAR(unit, msti), RT_ERR_MSTI_NOT_EXIST);
    
    
    STP_SEM_LOCK(unit);
    /* clear valid bit of MSTI */
    MSTI_VALID_CLEAR(unit, msti);
    
    STP_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_stp_mstpInstance_destroy */


/* Function Name:
 *      dal_ssw_stp_isMstpInstanceExist_get
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
dal_ssw_stp_isMstpInstanceExist_get(uint32 unit, uint32 msti, uint32 *pMsti_exist)
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
} /* end of dal_ssw_stp_isMstpInstanceExist_get */


/* Function Name:
 *      dal_ssw_stp_mstpState_get
 * Description:
 *      Get port spanning tree state of the msti from the specified device.
 * Input:
 *      unit        - unit id
 *      msti        - multiple spanning tree instance
 *      port        - port id
 * Output:
 *      pStp_state - pointer buffer of spanning tree state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_MSTI         - invalid msti
 *      RT_ERR_MSTI_NOT_EXIST   - MSTI is not exist
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. For single spanning tree mode, input CIST0 (msti=0).
 *      2. Spanning tree state as following
 *          - STP_STATE_DISABLED
 *          - STP_STATE_BLOCKING
 *          - STP_STATE_LEARNING
 *          - STP_STATE_FORWARDING
 */
int32
dal_ssw_stp_mstpState_get(uint32 unit, uint32 msti, rtk_port_t port, rtk_stp_state_t *pStp_state)
{
    int32   ret;
    uint32  value;
    
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
    if ((ret = reg_field_read(unit, (uint32)mstpStateControl_regidx[port], (uint32)mstpState_fieldidx[msti], &value)) != RT_ERR_OK)
    {
        STP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_STP), "");        
        return ret;
    }
    STP_SEM_UNLOCK(unit);
    
    /* translate chip value */
    switch (value)
    {
        case 0:
            *pStp_state = STP_STATE_DISABLED;
            break;
        case 1:
            *pStp_state = STP_STATE_BLOCKING;
            break;
        case 2:
            *pStp_state = STP_STATE_LEARNING;
            break;
        case 3:
            *pStp_state = STP_STATE_FORWARDING;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STP), "pStp_state=%x", *pStp_state); 
    
    return RT_ERR_OK;
} /*  dal_ssw_stp_mstpState_get */


/* Function Name:
 *      dal_ssw_stp_mstpState_set
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
 *          - STP_STATE_DISABLED
 *          - STP_STATE_BLOCKING
 *          - STP_STATE_LEARNING
 *          - STP_STATE_FORWARDING
 */
int32
dal_ssw_stp_mstpState_set(uint32 unit, uint32 msti, rtk_port_t port, rtk_stp_state_t stp_state)
{
    uint32  ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STP), "unit=%d, msti=%d, port=%d, stp_state=%d", 
           unit, msti, port, stp_state); 
    
    /* check Init status */
    RT_INIT_CHK(stp_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((stp_state >= STP_STATE_END), RT_ERR_MSTP_STATE);
    RT_PARAM_CHK((msti >= HAL_MAX_NUM_OF_MSTI(unit)), RT_ERR_MSTI);
    RT_PARAM_CHK(MSTI_VALID_IS_CLEAR(unit, msti), RT_ERR_MSTI_NOT_EXIST);
    
    /* translate chip value */
    switch (stp_state)
    {
        case STP_STATE_DISABLED:
            value = 0;
            break;
        case STP_STATE_BLOCKING:
            value = 1;
            break;
        case STP_STATE_LEARNING:
            value = 2;
            break;
        case STP_STATE_FORWARDING:
            value = 3;
            break;
        default:
            return RT_ERR_FAILED;
    }    
    
    STP_SEM_LOCK(unit);
    if ((ret = reg_field_write(unit, (uint32)mstpStateControl_regidx[port], (uint32)mstpState_fieldidx[msti], &value)) != RT_ERR_OK)
    {
        STP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_STP), "");
        return ret;
    }
    STP_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_stp_mstpState_set */

/* Function Name:
 *      _dal_ssw_stp_init_config
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
_dal_ssw_stp_init_config(uint32 unit)
{
    int32   ret;
    uint32  port, max_port;
    
    if ((ret = dal_ssw_stp_mstpInstance_create(unit, RTK_DEFAULT_MSTI)) != RT_ERR_OK)
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
            if ((ret = dal_ssw_stp_mstpState_set(unit, RTK_DEFAULT_MSTI, port, STP_STATE_FORWARDING)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_STP|MOD_DAL), "");
                return ret;
            }
        } 
        else 
        {
            if ((ret = dal_ssw_stp_mstpState_set(unit, RTK_DEFAULT_MSTI, port, RTK_DEFAULT_STP_PORT_STATE)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_STP|MOD_DAL), "");
                return ret;
            }            
        }
    }
    
    return RT_ERR_OK;
} /* end of _dal_ssw_stp_init_config */
