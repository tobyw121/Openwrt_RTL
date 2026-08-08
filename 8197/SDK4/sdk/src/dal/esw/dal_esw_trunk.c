/* 
 * Copyright(c) Realtek Semiconductor Corporation, 2008 
 * All rights reserved. 
 * 
 * $Revision: 30053 $
 * $Date: 2012-06-19 14:12:07 +0800 (Tue, 19 Jun 2012) $
 *
 * Purpose : Definition those public TRUNK APIs and its data type in the SDK.
 * 
 * Feature : The file have include the following module and sub-modules
 *           1) Trunk
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
#include <dal/esw/dal_esw_trunk.h>
#include <dal/esw/dal_esw_port.h>
#include <rtk/default.h>
#include <rtk/trunk.h>

/* 
 * Symbol Definition 
 */

/* 
 * Data Declaration 
 */
static uint32               trunk_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         trunk_sem[RTK_MAX_NUM_OF_UNIT];
static rtk_portmask_t       *pTrunkMemberSet[RTK_MAX_NUM_OF_UNIT];

#if defined(CONFIG_TRUNK_FAILOVER_HANDLING)
static rtk_portmask_t       *pTrunkLinkUpMemberSet[RTK_MAX_NUM_OF_UNIT];
#endif

/*
 * Macro Definition
 */
/* vlan semaphore handling */
#define TRUNK_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(trunk_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_DAL|MOD_TRUNK), "semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)
#define TRUNK_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(trunk_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_DAL|MOD_TRUNK), "semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)


/* 
 * Function Declaration 
 */

/* 
 * Function Declaration 
 */
static int32 _dal_esw_trunk_init_config(uint32 unit);
#if defined(CONFIG_TRUNK_FAILOVER_HANDLING)
static int32 _dal_esw_trunk_setHashbyPortMask(uint32 unit, uint32 trk_gid);
#endif

/* Module Name    : port     */
/* Sub-module Name: Global */

/* Function Name:
 *      dal_esw_trunk_init
 * Description:
 *      Initialize trunk module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      Must initialize trunk module before calling any trunk APIs.
 */
int32
dal_esw_trunk_init(uint32 unit)
{
    int32   ret;
    
    trunk_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    trunk_sem[unit] = osal_sem_mutex_create();
    if (0 == trunk_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_TRUNK), "semaphore create failed");
        return RT_ERR_FAILED;
    }
    
    /* allocate memory for trunk database for this unit */
    pTrunkMemberSet[unit] = (rtk_portmask_t *)osal_alloc(HAL_MAX_NUM_OF_TRUNK(unit)*sizeof(rtk_portmask_t));
    if (NULL == pTrunkMemberSet[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_TRUNK|MOD_DAL), "memory allocate failed");
        return RT_ERR_FAILED;
    }
    /* reset trunk member */
    osal_memset(pTrunkMemberSet[unit], 0, HAL_MAX_NUM_OF_TRUNK(unit)*sizeof(rtk_portmask_t));
    
#if defined(CONFIG_TRUNK_FAILOVER_HANDLING)
    pTrunkLinkUpMemberSet[unit] = (rtk_portmask_t *)osal_alloc(HAL_MAX_NUM_OF_TRUNK(unit)*sizeof(rtk_portmask_t));
    if (NULL == pTrunkLinkUpMemberSet[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_TRUNK|MOD_DAL), "memory allocate failed");
        osal_free(pTrunkMemberSet[unit]);
        pTrunkMemberSet[unit] = NULL;
        return RT_ERR_FAILED;
    }
    /* reset trunk member */
    osal_memset(pTrunkLinkUpMemberSet[unit], 0, HAL_MAX_NUM_OF_TRUNK(unit)*sizeof(rtk_portmask_t));
#endif

    /* set init flag to complete init */
    trunk_init[unit] = INIT_COMPLETED;    
    
    /* initialize default configuration */
    if ((ret = _dal_esw_trunk_init_config(unit)) != RT_ERR_OK)
    {
        trunk_init[unit] = INIT_NOT_COMPLETED;
        osal_free(pTrunkMemberSet[unit]);
        pTrunkMemberSet[unit] = NULL;
#if defined(CONFIG_TRUNK_FAILOVER_HANDLING)
        osal_free(pTrunkLinkUpMemberSet[unit]);
        pTrunkLinkUpMemberSet[unit] = NULL;
#endif
        RT_ERR(ret, (MOD_TRUNK|MOD_DAL), "init default configuration failed");
        return ret;
    }
    
    return RT_ERR_OK;
} /* end of dal_esw_trunk_init */

/* Module Name    : Trunk                    */
/* Sub-module Name: User configuration trunk */

/* Function Name:
 *      dal_esw_trunk_distributionAlgorithm_get
 * Description:
 *      Get the distribution algorithm of the trunk group id from the specified device.
 * Input:
 *      unit          - unit id
 *      trk_gid       - trunk group id
 * Output:
 *      pAlgo_bitmask - pointer buffer of bitmask of the distribution algorithm
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk ID
 *      RT_ERR_NULL_PTRUNK_DISTRIBUTION_ALGO_SPA_BITOINTER - input parameter may be null pointer
 * Note:
 *      1. You can use OR opertions in following bits to decide your algorithm.
 *      - TRUNK_DISTRIBUTION_ALGO_SPA_BIT        (source port)
 *      - TRUNK_DISTRIBUTION_ALGO_SMAC_BIT       (source mac)
 *      - TRUNK_DISTRIBUTION_ALGO_DMAC_BIT       (destination mac)
 *      - TRUNK_DISTRIBUTION_ALGO_SIP_BIT        (source ip)
 *      - TRUNK_DISTRIBUTION_ALGO_DIP_BIT        (destination ip)
 *      - TRUNK_DISTRIBUTION_ALGO_SRC_L4PORT_BIT (source layer4 port)
 *      - TRUNK_DISTRIBUTION_ALGO_DST_L4PORT_BIT (destination layer4 port)
 */
int32
dal_esw_trunk_distributionAlgorithm_get(uint32 unit, uint32 trk_gid, uint32 *pAlgo_bitmask)
{
    uint32 baseAddr, val, hashKey;
    int32 ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "unit=%d, trk_gid=%d",
           unit, trk_gid);
       
    /* check Init status */
    RT_INIT_CHK(trunk_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((trk_gid >= HAL_MAX_NUM_OF_TRUNK(unit)), RT_ERR_LA_TRUNK_ID);
    RT_PARAM_CHK((NULL == pAlgo_bitmask), RT_ERR_NULL_POINTER);
      
    TRUNK_SEM_LOCK(unit);

    baseAddr = ESW_LINK_AGGREGATION_GROUP0_PARAMETERr + trk_gid * 3;

    if((ret = reg_read(unit, baseAddr, &val)) != RT_ERR_OK)
    {
        TRUNK_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }

    if((ret =  reg_field_get(unit, baseAddr, ESW_LAGSPAHASHf, &hashKey, &val) != RT_ERR_OK))
    {
        TRUNK_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }
    
    if(hashKey)
        *pAlgo_bitmask = TRUNK_DISTRIBUTION_ALGO_SPA_BIT;
    else
        *pAlgo_bitmask = 0;

    if((ret = reg_field_get(unit, baseAddr, ESW_LAGL2HASHf, &hashKey, &val)) != RT_ERR_OK)
    {
        TRUNK_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }

    if(hashKey & 1)
        *pAlgo_bitmask |= TRUNK_DISTRIBUTION_ALGO_SMAC_BIT;
    if(hashKey & 2)
        *pAlgo_bitmask |= TRUNK_DISTRIBUTION_ALGO_DMAC_BIT;

    if((ret = reg_field_get(unit, baseAddr, ESW_LAGL3HASHf, &hashKey, &val)) != RT_ERR_OK)
    {
        TRUNK_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }

    if(hashKey & 1)
        *pAlgo_bitmask |= TRUNK_DISTRIBUTION_ALGO_SIP_BIT;
    if(hashKey & 2)
        *pAlgo_bitmask |= TRUNK_DISTRIBUTION_ALGO_DIP_BIT;

    if((ret = reg_field_get(unit, baseAddr, ESW_LAGL4HASHf, &hashKey, &val)) != RT_ERR_OK)
    {
        TRUNK_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }

    if(hashKey & 1)
        *pAlgo_bitmask |= TRUNK_DISTRIBUTION_ALGO_SRC_L4PORT_BIT;
    if(hashKey & 2)
        *pAlgo_bitmask |= TRUNK_DISTRIBUTION_ALGO_DST_L4PORT_BIT;

    TRUNK_SEM_UNLOCK(unit);    

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "pAlgo_bitmask=%x",
           *pAlgo_bitmask);
    
    return RT_ERR_OK;
} /*end of dal_esw_trunk_distributionAlgorithm_get*/


/* Function Name:
 *      dal_esw_trunk_distributionAlgorithm_set
 * Description:
 *      Set the distribution algorithm of the trunk group id from the specified device.
 * Input:
 *      unit         - unit id
 *      trk_gid      - trunk group id
 *      algo_bitmask - bitmask of the distribution algorithm
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_LA_TRUNK_ID - invalid trunk ID
 *      RT_ERR_LA_HASHMASK - invalid hash mask
 * Note:
 *      1. You can use OR opertions in following bits to decide your algorithm.
 *      - TRUNK_DISTRIBUTION_ALGO_SPA_BIT        (source port)
 *      - TRUNK_DISTRIBUTION_ALGO_SMAC_BIT       (source mac)
 *      - TRUNK_DISTRIBUTION_ALGO_DMAC_BIT       (destination mac)
 *      - TRUNK_DISTRIBUTION_ALGO_SIP_BIT        (source ip)
 *      - TRUNK_DISTRIBUTION_ALGO_DIP_BIT        (destination ip)
 *      - TRUNK_DISTRIBUTION_ALGO_SRC_L4PORT_BIT (source layer4 port)
 *      - TRUNK_DISTRIBUTION_ALGO_DST_L4PORT_BIT (destination layer4 port)
 */
int32
dal_esw_trunk_distributionAlgorithm_set(uint32 unit, uint32 trk_gid, uint32 algo_bitmask)
{
    uint32 baseAddr, hashKey;
    int32 ret;
   
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "unit=%d, trk_gid=%d, pAlgo_bitmask=%x",
           unit, trk_gid, algo_bitmask);
    
    /* check Init status */
    RT_INIT_CHK(trunk_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((trk_gid >= HAL_MAX_NUM_OF_TRUNK(unit)), RT_ERR_LA_TRUNK_ID);
    RT_PARAM_CHK((algo_bitmask > 0x7f), RT_ERR_LA_HASHMASK);

    TRUNK_SEM_LOCK(unit);

    baseAddr = ESW_LINK_AGGREGATION_GROUP0_PARAMETERr + trk_gid * 3;    

    /*set sport hask key*/
    hashKey = 0;
    if(algo_bitmask & TRUNK_DISTRIBUTION_ALGO_SPA_BIT)
        hashKey = 1;
    if((ret =  reg_field_write(unit, baseAddr, ESW_LAGSPAHASHf, &hashKey)) !=RT_ERR_OK)
    {
        TRUNK_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }

    /*set smac/dmac hask key*/
    hashKey = 0;
    if(algo_bitmask & TRUNK_DISTRIBUTION_ALGO_SMAC_BIT)
        hashKey = 1;
    if(algo_bitmask & TRUNK_DISTRIBUTION_ALGO_DMAC_BIT)
        hashKey |= 2;
    if((ret = reg_field_write(unit, baseAddr, ESW_LAGL2HASHf, &hashKey)) !=RT_ERR_OK)
    {
        TRUNK_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }

    /*set sip/dip hask key*/
    hashKey = 0;
    if(algo_bitmask & TRUNK_DISTRIBUTION_ALGO_SIP_BIT)
        hashKey = 1;
    if(algo_bitmask & TRUNK_DISTRIBUTION_ALGO_DIP_BIT)
        hashKey |= 2;
    if((ret = reg_field_write(unit, baseAddr, ESW_LAGL3HASHf, &hashKey)) !=RT_ERR_OK)
    {
        TRUNK_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }

    /*set layer4 sport/dport hask key*/
    hashKey = 0;
    if(algo_bitmask & TRUNK_DISTRIBUTION_ALGO_SRC_L4PORT_BIT)
        hashKey = 1;
    if(algo_bitmask & TRUNK_DISTRIBUTION_ALGO_DST_L4PORT_BIT)
        hashKey |= 2;
    if((ret = reg_field_write(unit, baseAddr, ESW_LAGL4HASHf, &hashKey)) !=RT_ERR_OK)
    {
        TRUNK_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }

    TRUNK_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /*end of dal_esw_trunk_distributionAlgorithm_set*/

/* Function Name:
 *      dal_esw_trunk_hashMappingTable_get
 * Description:
 *      Get hash value to port array in the trunk group id from the specified device.
 * Input:
 *      unit             - unit id
 *      trk_gid          - trunk group id
 * Output:
 *      pHash2Port_array - pointer buffer of ports associate with the hash value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_trunk_hashMappingTable_get(
    uint32                   unit,
    uint32                   trk_gid,
    rtk_trunk_hashVal2Port_t *pHash2Port_array)
{
    uint32 baseAddr, port, i;
    int32  ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "unit=%d, trk_gid=%d",
           unit, trk_gid);
    
    /* check Init status */
    RT_INIT_CHK(trunk_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(trk_gid >= HAL_MAX_NUM_OF_TRUNK(unit), RT_ERR_LA_TRUNK_ID);
    RT_PARAM_CHK((NULL == pHash2Port_array), RT_ERR_NULL_POINTER);
       
    TRUNK_SEM_LOCK(unit);

    baseAddr = ESW_LINK_AGGREGATION_GROUP0_PARAMETER2r + trk_gid * 3;

    for (i = 0; i < HAL_MAX_NUM_OF_TRUNKHASHVAL(unit); i++)
    {
        if((ret = reg_array_field_read(unit, baseAddr, i, REG_ARRAY_INDEX_NONE, ESW_LAGHRPORTf, &port)) != RT_ERR_OK)
        {
            TRUNK_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
            return ret;
        }
        pHash2Port_array->value[i] = port;
    }

    TRUNK_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "pHash2Port_array=%x",
           *pHash2Port_array);
    
    return RT_ERR_OK;
} /*end of dal_esw_trunk_hashMappingTable_get*/

/* Function Name:
 *      dal_esw_trunk_hashMappingTable_set
 * Description:
 *      Set hash value to port array in the trunk group id from the specified device.
 * Input:
 *      unit             - unit id
 *      trk_gid          - trunk group id
 *      pHash2Port_array - ports associate with the hash value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID            - invalid unit id
 *      RT_ERR_LA_TRUNK_ID        - invalid trunk ID
 *      RT_ERR_NULL_POINTER       - input parameter may be null pointer
 *      RT_ERR_LA_TRUNK_NOT_EXIST - the trunk doesn't exist
 *      RT_ERR_LA_NOT_MEMBER_PORT - the port is not a member port of the trunk
 *      RT_ERR_LA_CPUPORT         - CPU port can not be aggregated port
 * Note:
 *      None
 */
int32
dal_esw_trunk_hashMappingTable_set(
    uint32                   unit,
    uint32                   trk_gid,
    rtk_trunk_hashVal2Port_t *pHash2Port_array)
{
    uint32  baseAddr, i, temp;
    uint32  hashVal_idx;
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "unit=%d, trk_gid=%d", unit, trk_gid);  
        
    /* check Init status */
    RT_INIT_CHK(trunk_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(trk_gid >= HAL_MAX_NUM_OF_TRUNK(unit), RT_ERR_LA_TRUNK_ID);
    RT_PARAM_CHK((NULL == pHash2Port_array), RT_ERR_NULL_POINTER);
    for (hashVal_idx = 0; hashVal_idx < HAL_MAX_NUM_OF_TRUNKHASHVAL(unit); hashVal_idx++)
    {
        RT_PARAM_CHK((pHash2Port_array->value[hashVal_idx] > (HAL_GET_MAX_ETHER_PORT(unit))), RT_ERR_PORT_ID);
    }
    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "pHash2Port_array=%x", pHash2Port_array);  

    TRUNK_SEM_LOCK(unit);
    
    baseAddr = ESW_LINK_AGGREGATION_GROUP0_PARAMETER2r + trk_gid * 3;
    
    for (i = 0; i < HAL_MAX_NUM_OF_TRUNKHASHVAL(unit); i++)
    {
        temp = pHash2Port_array->value[i];
        if((ret =  reg_array_field_write(unit, baseAddr, i, REG_ARRAY_INDEX_NONE, ESW_LAGHRPORTf, &temp)) != RT_ERR_OK)
        {
            TRUNK_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
            return ret;
        }
    }

    TRUNK_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /*end of dal_esw_trunk_hashMappingTable_set*/

/* Function Name:
 *      dal_esw_trunk_mode_get
 * Description:
 *      Get the trunk mode from the specified device.
 * Input:
 *      unit   - unit id
 * Output:
 *      pMode - pointer buffer of trunk mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. The enum of the trunk mode as following
 *         - TRUNK_MODE_NORMAL
 *         - TRUNK_MODE_DUMB
 *      2. Normal and dumb mode support 8 trunk members in each group.
 */
int32
dal_esw_trunk_mode_get(uint32 unit, rtk_trunk_mode_t *pMode)
{
    uint32  value;
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(trunk_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pMode), RT_ERR_NULL_POINTER);
    
    value = 0;
    TRUNK_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_field_read(unit, ESW_LINK_AGGREGATION_CONTROL0r, ESW_LAG_DUMBf, &value)) != RT_ERR_OK)
    {
        TRUNK_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }
    TRUNK_SEM_UNLOCK(unit);
    
    switch (value)
    {
        case 0:
            *pMode = TRUNK_MODE_NORMAL;
            break;
        case 1:
            *pMode = TRUNK_MODE_DUMB;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "pMode=%d", *pMode);
    
    return RT_ERR_OK;
} /* end of dal_esw_trunk_mode_get */


/* Function Name:
 *      dal_esw_trunk_mode_set
 * Description:
 *      Set the trunk mode to the specified device.
 * Input:
 *      unit - unit id
 *      mode - trunk mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_INPUT   - invalid input parameter 
 * Note:
 *      1. The enum of the trunk mode as following
 *      - TRUNK_MODE_NORMAL
 *      - TRUNK_MODE_DUMB
 *      2. Normal and dumb mode support 8 trunk members in each group.
 */
int32
dal_esw_trunk_mode_set(uint32 unit, rtk_trunk_mode_t mode)
{
    uint32  value;
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "unit=%d, mode=%d", unit, mode);  
    
    /* check Init status */
    RT_INIT_CHK(trunk_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((mode >= TRUNK_MODE_END), RT_ERR_INPUT); 
    
    switch (mode)
    {
        case TRUNK_MODE_NORMAL:
            value = 0; 
            break;
        case TRUNK_MODE_DUMB:
            value = 1; 
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    TRUNK_SEM_LOCK(unit);
    /* Set entry to CHIP*/
    if ((ret = reg_field_write(unit, ESW_LINK_AGGREGATION_CONTROL0r, ESW_LAG_DUMBf, &value)) != RT_ERR_OK)
    {
        TRUNK_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }
    TRUNK_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_trunk_mode_set */


/* Function Name:
 *      dal_esw_trunk_port_get
 * Description:
 *      Get the members of the trunk id from the specified device.
 * Input:
 *      unit                   - unit id
 *      trk_gid                - trunk group id
 * Output:
 *      pTrunk_member_portmask - pointer buffer of trunk member ports
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_trunk_port_get(uint32 unit, uint32 trk_gid, rtk_portmask_t *pTrunk_member_portmask)
{
    uint32  value;
    uint32  baseAddr;
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "unit=%d, trk_gid=%d", 
           unit, trk_gid);  
    
    /* check Init status */
    RT_INIT_CHK(trunk_init[unit]);
    
    /* parameter check */
     /* trunk_id is max_num - 1, so it will use "trk_gid > max_num_of_trunk" */
    RT_PARAM_CHK(trk_gid >= HAL_MAX_NUM_OF_TRUNK(unit), RT_ERR_LA_TRUNK_ID);
    RT_PARAM_CHK((NULL == pTrunk_member_portmask), RT_ERR_NULL_POINTER);

    osal_memset(pTrunk_member_portmask, 0, sizeof(rtk_portmask_t));
        
    TRUNK_SEM_LOCK(unit);
    
    baseAddr = ESW_LINK_AGGREGATION_GROUP0_PARAMETERr + trk_gid * 3;

    /*Check the trunk is valid*/
    if((ret = reg_field_read(unit, baseAddr, ESW_LAGVf, &value)) != RT_ERR_OK)
    {
        TRUNK_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }

    /*The Trunk is invalid*/
    if(!value)
    {
        TRUNK_SEM_UNLOCK(unit);        
        return RT_ERR_OK;
    }    
    
    /* get entry from CHIP*/
    if ((ret = reg_field_read(unit, baseAddr + 1, ESW_LAGPMf, &pTrunk_member_portmask->bits[0])) != RT_ERR_OK)
    {
        TRUNK_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }
    TRUNK_SEM_UNLOCK(unit);
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "pTrunk_member_portmask=%x", 
           pTrunk_member_portmask->bits[0]); 
    
    return RT_ERR_OK;
} /* end of dal_esw_trunk_port_get */


/* Function Name:
 *      dal_esw_trunk_port_set
 * Description:
 *      Set the members of the trunk id to the specified device.
 * Input:
 *      unit                   - unit id
 *      trk_gid                - trunk group id
 *      pTrunk_member_portmask - trunk member ports
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_LA_TRUNK_ID       - invalid trunk ID
 *      RT_ERR_LA_MEMBER_OVERLAP - the specified port mask is overlapped with other group
 *      RT_ERR_LA_PORTNUM_DUMB   - it can only aggregate at most four ports when 802.1ad dumb mode
 *      RT_ERR_LA_PORTNUM_NORMAL - it can only aggregate at most eight ports when 802.1ad normal mode
 * Note:
 *      None
 */
int32
dal_esw_trunk_port_set(uint32 unit, uint32 trk_gid, rtk_portmask_t *pTrunk_member_portmask)
{
    uint32  grp_num, temp;
    uint32  num_of_port;
    uint32  baseAddr;
    int32   ret;
    rtk_port_t    represPort;
#if defined(CONFIG_TRUNK_FAILOVER_HANDLING)
    rtk_port_t trunkMemberPort;
#endif

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "unit=%d, trk_gid=%d", unit, trk_gid); 
        
    /* check Init status */
    RT_INIT_CHK(trunk_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(trk_gid >= HAL_MAX_NUM_OF_TRUNK(unit), RT_ERR_LA_TRUNK_ID);
    RT_PARAM_CHK((NULL == pTrunk_member_portmask), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(pTrunk_member_portmask->bits[0] >= (1 << (HAL_GET_MAX_PORT(unit))), RT_ERR_LA_PORTMASK);

    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "pTrunk_member_portmask=%x", pTrunk_member_portmask->bits[0]);     
    
    /* check number of trunk member */
    RT_PARAM_CHK(((num_of_port = RTK_PORTMASK_GET_PORT_COUNT(*pTrunk_member_portmask)) > 
                                HAL_MAX_NUM_OF_DUMB_TRUNKMEMBER(unit)), RT_ERR_LA_PORTNUM_DUMB);
    
    /* check whether new member set is overlap with other trunk */
    for (grp_num = 0; grp_num < HAL_MAX_NUM_OF_TRUNK(unit); grp_num++)
    {
        if (grp_num != trk_gid)
        {
            RT_PARAM_CHK((pTrunk_member_portmask->bits[0] & pTrunkMemberSet[unit][grp_num].bits[0]) 
                        , RT_ERR_LA_MEMBER_OVERLAP);
        }
    }        

    baseAddr = ESW_LINK_AGGREGATION_GROUP0_PARAMETERr + trk_gid * 3;
    
    TRUNK_SEM_LOCK(unit);
    /* Set entry from CHIP*/
    temp = pTrunk_member_portmask->bits[0];
    if ((ret = reg_field_write(unit, baseAddr + 1, ESW_LAGPMf, &temp)) != RT_ERR_OK)
    {
        TRUNK_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }

    /*if normal mode, set default represent port == first member port*/
    if(pTrunk_member_portmask->bits[0] == 0)
    {
        represPort = 0;                    
    }
    else
    {
        for(represPort = 0; represPort < HAL_GET_MAX_PORT(unit); represPort++)
        {
            if(RTK_PORTMASK_IS_PORT_SET(*pTrunk_member_portmask, represPort))
                break;
        }
    }

    if((ret = reg_field_write(unit, baseAddr, ESW_LAGPNf, &represPort)) != RT_ERR_OK)
    {
        TRUNK_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }    

    /*Valid the trunk*/
    temp = ENABLED;
    if((ret = reg_field_write(unit, baseAddr, ESW_LAGVf, &temp)) != RT_ERR_OK)
    {
        TRUNK_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }
    
    TRUNK_SEM_UNLOCK(unit);
    
    RTK_PORTMASK_ASSIGN(pTrunkMemberSet[unit][trk_gid], *pTrunk_member_portmask);
    
#if defined(CONFIG_TRUNK_FAILOVER_HANDLING)
    /* Maintain link up trunk member port mask */
    /* Reset port mask to empty */
    RTK_PORTMASK_RESET(pTrunkLinkUpMemberSet[unit][trk_gid]);
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "Clear pTrunkLinkUpMemberSet[%d][%d]=0x%x", unit, trk_gid, pTrunkLinkUpMemberSet[unit][trk_gid].bits[0]); 
    /* Check member port link status to create link up port bitmask */
    /* Scan all ports */
    for (trunkMemberPort = 0; trunkMemberPort < RTK_MAX_NUM_OF_PORTS; trunkMemberPort++)
    {
        /* If port is in the member port list */
        if (RTK_PORTMASK_IS_PORT_SET(pTrunkMemberSet[unit][trk_gid], trunkMemberPort))
        {
            rtk_port_linkStatus_t linkStatus;

            /*Check link status of the member port*/
            if ((ret = dal_esw_port_link_get(unit, trunkMemberPort, &linkStatus)))
            {
                RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "Get port link status fail");
                return ret;
            }
            dal_esw_trunk_port_link_notification(unit, trunkMemberPort, linkStatus);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "unit=%d, Trunk=%d, Trunk_member_port=0x%d", unit,trk_gid, trunkMemberPort); 
        }
    }
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "Updated pTrunkLinkUpMemberSet[%d][%d]=0x%x", unit, trk_gid, pTrunkLinkUpMemberSet[unit][trk_gid].bits[0]); 
#endif
    
    return RT_ERR_OK;
} /* end of dal_esw_trunk_port_set */


/* Function Name:
 *      dal_esw_trunk_port_link_notification
 * Description:
 *      Notify link state change of ports to trunk failover handling.
 * Input:
 *      unit            - unit id
 *      trunkMemberPort - link state change port 
 *      linkStatus      - the new link state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_LA_TRUNK_ID - invalid trunk ID
 * Note:
 *      None
 */
int32
dal_esw_trunk_port_link_notification(uint32 unit, rtk_port_t trunkMemberPort, rtk_port_linkStatus_t linkStatus)
{
#if defined(CONFIG_TRUNK_FAILOVER_HANDLING)
    uint32  ret;
    uint32  grp_num;
    rtk_trunk_mode_t trunkMode;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "unit=%d, Trunk_member_port=%d, LinkStatus=%d", unit, trunkMemberPort, linkStatus); 
        
    /* check Init status */
    RT_INIT_CHK(trunk_init[unit]);

    /* get trunk mode for checking number of trunk member */
    if ((ret = dal_esw_trunk_mode_get(unit, &trunkMode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "Get Trunk mode fail");
        return ret;
    }
    
    /* check number of trunk member */
    if (TRUNK_MODE_NORMAL != trunkMode )
    {
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "Only work on Normal mode");
        return ret;
    }

    /* Check all trunk to see if incoming port is belong to a trunk */
    for (grp_num = 0; grp_num < HAL_MAX_NUM_OF_TRUNK(unit); grp_num++)
    {
        /* If the port is in the trunk member list */
        if (RTK_PORTMASK_IS_PORT_SET(pTrunkMemberSet[unit][grp_num], trunkMemberPort))
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "Ready surve Tgid=%d, TrunkMemberPortMask=0x%x, LinkupPortMask=0x%x", 
                   grp_num, pTrunkMemberSet[unit][grp_num].bits[0], pTrunkLinkUpMemberSet[unit][grp_num].bits[0]);
            
            /* Update trunkLinkUpMemberSet */
            if (PORT_LINKUP == linkStatus)
            {
                /* Link up to add the port to the bitmask */
                RTK_PORTMASK_PORT_SET(pTrunkLinkUpMemberSet[unit][grp_num], trunkMemberPort);
            }
            else if (PORT_LINKDOWN == linkStatus)
            {
                /* Link down to add the port to the bitmask */
                RTK_PORTMASK_PORT_CLEAR(pTrunkLinkUpMemberSet[unit][grp_num], trunkMemberPort);
            }
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "Tgid=%d, TrunkMemberPortMask=0x%x, LinkupPortMask=0x%x", 
                   grp_num, pTrunkMemberSet[unit][grp_num].bits[0], pTrunkLinkUpMemberSet[unit][grp_num].bits[0]);

            /* Update the hash mapping table */
            ret = _dal_esw_trunk_setHashbyPortMask(unit, grp_num);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "Tgid=%d break", grp_num);
            break;
        }
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "finished");
    return RT_ERR_OK;
#else
    return RT_ERR_DRIVER_NOT_FOUND;
#endif

} /* end of dal_esw_trunk_port_link_notification */

/* Function Name:
 *      dal_esw_trunk_representPort_get
 * Description:
 *      Get represent port of trunk.
 * Input:
 *      unit        - unit id
 *      trunk_id    - trunk id
 * Output:
 *      pRepresPort - pointer to represent port of trunk
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_LA_TRUNK_ID      - invalid trunk id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_trunk_representPort_get(uint32 unit, uint32 trunk_id, rtk_port_t *pRepresPort)
{
    uint32  baseAddr;
    int32   ret;
    rtk_trunk_mode_t  trunkMode;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "unit=%d, trk_gid=%d", 
           unit, trunk_id);  
    
    /* check Init status */
    RT_INIT_CHK(trunk_init[unit]);
    
    /* parameter check */
     /* trunk_id is max_num - 1, so it will use "trk_gid > max_num_of_trunk" */
    RT_PARAM_CHK(trunk_id >= HAL_MAX_NUM_OF_TRUNK(unit), RT_ERR_LA_TRUNK_ID);
    RT_PARAM_CHK((NULL == pRepresPort), RT_ERR_NULL_POINTER);

    /* get trunk mode for checking number of trunk member */
    if ((ret = dal_esw_trunk_mode_get(unit, &trunkMode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }

    /*Only support normal mode*/
    RT_PARAM_CHK((TRUNK_MODE_DUMB == trunkMode), RT_ERR_FAILED);
        
    TRUNK_SEM_LOCK(unit);

    baseAddr = ESW_LINK_AGGREGATION_GROUP0_PARAMETERr + trunk_id * 3;

    if((ret = reg_field_read(unit, baseAddr, ESW_LAGPNf, pRepresPort)) != RT_ERR_OK)
    {
        TRUNK_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }
    
    TRUNK_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "pRepresPortunit=%d, trk_gid=%d", *pRepresPort);  
    
    return RT_ERR_OK;
} /* end of dal_esw_trunk_representPort_get */

/* Function Name:
 *      dal_esw_trunk_representPort_set
 * Description:
 *      Set represent port of trunk.
 * Input:
 *      unit       - unit id
 *      trunk_id   - trunk id
 *      represPort - represent port of trunk
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_LA_TRUNK_ID      - invalid trunk id
 *      RT_ERR_PORT_ID          - invalid port id
 * Note:
 *      None
 */
int32
dal_esw_trunk_representPort_set(uint32 unit, uint32 trunk_id, rtk_port_t represPort)
{
    uint32  baseAddr, temp;
    int32   ret;
    rtk_trunk_mode_t  trunkMode;
    rtk_portmask_t trunk_member_portmask;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "unit=%d, trk_gid=%d, represPort=%d", 
           unit, trunk_id, represPort);  
    
    /* check Init status */
    RT_INIT_CHK(trunk_init[unit]);
    
    /* parameter check */
     /* trunk_id is max_num - 1, so it will use "trk_gid > max_num_of_trunk" */
    RT_PARAM_CHK(trunk_id >= HAL_MAX_NUM_OF_TRUNK(unit), RT_ERR_LA_TRUNK_ID);
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, represPort), RT_ERR_PORT_ID);

    /* get trunk mode for checking number of trunk member */
    if ((ret = dal_esw_trunk_mode_get(unit, &trunkMode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }

     /*Only support normal mode*/
    RT_PARAM_CHK((TRUNK_MODE_NORMAL != trunkMode), RT_ERR_FAILED);

    /*get the member port of the trunk */
    if ((ret = dal_esw_trunk_port_get(unit, trunk_id, &trunk_member_portmask)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }
    
    /*check the represPort is the trunk member port */
    RT_PARAM_CHK(!RTK_PORTMASK_IS_PORT_SET(trunk_member_portmask, represPort), RT_ERR_PORT_ID);
        
    TRUNK_SEM_LOCK(unit);

    baseAddr = ESW_LINK_AGGREGATION_GROUP0_PARAMETERr + trunk_id * 3;

    /*Set represent port of trunk*/
    if((ret = reg_field_write(unit, baseAddr, ESW_LAGPNf, &represPort)) != RT_ERR_OK)
    {
        TRUNK_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }

    /*Valid the trunk*/
    temp = ENABLED;
    if((ret = reg_field_write(unit, baseAddr, ESW_LAGVf, &temp)) != RT_ERR_OK)
    {
        TRUNK_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }

    TRUNK_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /*end of dal_esw_trunk_representPort_set*/

/* Function Name:
 *      dal_esw_trunk_floodMode_get
 * Description:
 *      Get flood mode of trunk.
 * Input:
 *      unit       - unit id
 *      trunk_id   - trunk id
 * Output:
 *      pFloodMode - pointer to flood mode of trunk
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_LA_TRUNK_ID      - invalid trunk id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Flood mode is as following
 *      - FLOOD_PORT_BY_HASH
 *      - FLOOD_PORT_BY_CONFIG
 */
int32
dal_esw_trunk_floodMode_get(uint32 unit, uint32 trunk_id, rtk_trunk_floodMode_t *pFloodMode)
{
    uint32  baseAddr;
    int32   ret;
    rtk_trunk_mode_t  trunkMode;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "unit=%d, trk_gid=%d", 
           unit, trunk_id);  
    
    /* check Init status */
    RT_INIT_CHK(trunk_init[unit]);
    
    /* parameter check */
     /* trunk_id is max_num - 1, so it will use "trk_gid > max_num_of_trunk" */
    RT_PARAM_CHK(trunk_id >= HAL_MAX_NUM_OF_TRUNK(unit), RT_ERR_LA_TRUNK_ID);
    RT_PARAM_CHK((NULL == pFloodMode), RT_ERR_NULL_POINTER);

    /* get trunk mode for checking number of trunk member */
    if ((ret = dal_esw_trunk_mode_get(unit, &trunkMode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }

    /*Only support normal mode*/
    RT_PARAM_CHK((TRUNK_MODE_NORMAL != trunkMode), RT_ERR_FAILED);
        
    TRUNK_SEM_LOCK(unit);

    baseAddr = ESW_LINK_AGGREGATION_GROUP0_PARAMETERr + trunk_id * 3;

    if((ret = reg_field_read(unit, baseAddr, ESW_LAGFLOODASf, pFloodMode)) != RT_ERR_OK)
     {
        TRUNK_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }

    TRUNK_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "pFloodMode=%d", *pFloodMode);  
    
    return RT_ERR_OK;
} /*end of dal_esw_trunk_floodMode_get*/


/* Function Name:
 *      dal_esw_trunk_floodMode_set
 * Description:
 *      Set flood mode of trunk.
 * Input:
 *      unit      - unit id
 *      trunk_id  - trunk id
 *      floodMode - flood mode of trunk
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_LA_TRUNK_ID      - invalid trunk id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Flood mode is as following
 *      - FLOOD_PORT_BY_HASH
 *      - FLOOD_PORT_BY_CONFIG
 */
int32
dal_esw_trunk_floodMode_set(uint32 unit, uint32 trunk_id, rtk_trunk_floodMode_t floodMode)
{
    uint32  baseAddr;
    int32   ret;
    rtk_trunk_mode_t  trunkMode;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "unit=%d, trk_gid=%d, floodMode=%d", 
           unit, trunk_id, floodMode);  
    
    /* check Init status */
    RT_INIT_CHK(trunk_init[unit]);
    
    /* parameter check */
     /* trunk_id is max_num - 1, so it will use "trk_gid > max_num_of_trunk" */
    RT_PARAM_CHK(trunk_id >= HAL_MAX_NUM_OF_TRUNK(unit), RT_ERR_LA_TRUNK_ID);
    RT_PARAM_CHK((floodMode >= FLOOD_MODE_END), RT_ERR_INPUT);

    /* get trunk mode for checking number of trunk member */
    if ((ret = dal_esw_trunk_mode_get(unit, &trunkMode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }

    /*Only support normal mode*/
    RT_PARAM_CHK((TRUNK_MODE_NORMAL != trunkMode), RT_ERR_FAILED);
        
    TRUNK_SEM_LOCK(unit);

    baseAddr = ESW_LINK_AGGREGATION_GROUP0_PARAMETERr + trunk_id * 3;

    if((ret = reg_field_write(unit, baseAddr, ESW_LAGFLOODASf, &floodMode)) != RT_ERR_OK)
     {
        TRUNK_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }

    TRUNK_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /*end of dal_esw_trunk_floodMode_set*/


/* Function Name:
 *      dal_esw_trunk_floodPort_get
 * Description:
 *      Get flooding port of trunk.
 * Input:
 *      unit       - unit id
 *      trunk_id   - trunk id
 * Output:
 *      pFloodPort - pointer to flooding port of trunk
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_LA_TRUNK_ID      - invalid trunk id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_trunk_floodPort_get(uint32 unit, uint32 trunk_id, rtk_port_t *pFloodPort)
{
    uint32  baseAddr;
    int32   ret;
    rtk_trunk_mode_t  trunkMode;
    rtk_trunk_floodMode_t floodMode;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "unit=%d, trk_gid=%d", 
           unit, trunk_id);  
    
    /* check Init status */
    RT_INIT_CHK(trunk_init[unit]);
    
    /* parameter check */
     /* trunk_id is max_num - 1, so it will use "trk_gid > max_num_of_trunk" */
    RT_PARAM_CHK(trunk_id >= HAL_MAX_NUM_OF_TRUNK(unit), RT_ERR_LA_TRUNK_ID);
    RT_PARAM_CHK((NULL == pFloodPort), RT_ERR_NULL_POINTER);

    /* get trunk mode for checking number of trunk member */
    if ((ret = dal_esw_trunk_mode_get(unit, &trunkMode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }

    /*Only support normal mode*/
    RT_PARAM_CHK((TRUNK_MODE_NORMAL != trunkMode), RT_ERR_FAILED);

    /* get trunk flood mode */
    if ((ret = dal_esw_trunk_floodMode_get(unit, trunk_id, &floodMode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }

    /*Only support config mode*/
    RT_PARAM_CHK((FLOOD_MODE_BY_CONFIG != floodMode), RT_ERR_FAILED);
        
    TRUNK_SEM_LOCK(unit);

    baseAddr = ESW_LINK_AGGREGATION_GROUP0_PARAMETERr + trunk_id * 3;

    if((ret = reg_field_read(unit, baseAddr, ESW_LAGFLOODPORTf, pFloodPort)) != RT_ERR_OK)
    {
        TRUNK_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }

    TRUNK_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "pFloodPort=%d", *pFloodPort);  
    
    return RT_ERR_OK;
} /*end of dal_esw_trunk_floodPort_get*/

/* Function Name:
 *      dal_esw_trunk_floodPort_set
 * Description:
 *      Set flooding port of trunk.
 * Input:
 *      unit      - unit id
 *      trunk_id  - trunk id
 *      floodPort - flooding port of trunk
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_LA_TRUNK_ID      - invalid trunk id
 *      RT_ERR_PORT_ID          - invalid port id
 * Note:
 *      None
 */
int32
dal_esw_trunk_floodPort_set(uint32 unit, uint32 trunk_id, rtk_port_t floodPort)
{
    uint32  baseAddr;
    int32   ret;
    rtk_trunk_mode_t  trunkMode;
    rtk_portmask_t trunk_member_portmask;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "unit=%d, trk_gid=%d, floodPort=%d", 
           unit, trunk_id, floodPort);  
    
    /* check Init status */
    RT_INIT_CHK(trunk_init[unit]);
    
    /* parameter check */
     /* trunk_id is max_num - 1, so it will use "trk_gid > max_num_of_trunk" */
    RT_PARAM_CHK(trunk_id >= HAL_MAX_NUM_OF_TRUNK(unit), RT_ERR_LA_TRUNK_ID);
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, floodPort), RT_ERR_PORT_ID);

    /* get trunk mode for checking number of trunk member */
    if ((ret = dal_esw_trunk_mode_get(unit, &trunkMode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }

     /*Only support normal mode*/
    RT_PARAM_CHK((TRUNK_MODE_NORMAL != trunkMode), RT_ERR_FAILED);

    /*get the member port of the trunk */
    if ((ret = dal_esw_trunk_port_get(unit, trunk_id, &trunk_member_portmask)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }
    
    /*check the flood port is the trunk member port */
    RT_PARAM_CHK(!RTK_PORTMASK_IS_PORT_SET(trunk_member_portmask, floodPort), RT_ERR_PORT_ID);
        
    TRUNK_SEM_LOCK(unit);

    baseAddr = ESW_LINK_AGGREGATION_GROUP0_PARAMETERr + trunk_id * 3;

    /*Set flood port of trunk*/
    if((ret = reg_field_write(unit, baseAddr, ESW_LAGFLOODPORTf, &floodPort)) != RT_ERR_OK)
    {
        TRUNK_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }

    TRUNK_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /*end of dal_esw_trunk_floodPort_set*/


/* Function Name:
 *      _dal_esw_trunk_init_config
 * Description:
 *      Initialize default configuration for trunk module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Must initialize trunk module before calling this API
 */
static int32
_dal_esw_trunk_init_config(uint32 unit)
{
    uint32  tgid;
    int32   ret;
    rtk_portmask_t portmask;
    
    portmask.bits[0] = RTK_DEFAULT_TRUNK_MEMBER_PORTMASK;
    
    for (tgid = 0; tgid < HAL_MAX_NUM_OF_TRUNK(unit); tgid++)
    {
        if ((ret = dal_esw_trunk_port_set(unit, tgid, &portmask)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_TRUNK|MOD_DAL), "");
            return ret;
        }     

        if ((ret = dal_esw_trunk_distributionAlgorithm_set(unit, tgid, RTK_DEFAULT_TRUNK_DISTRIBUTION_ALGORITHM)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_TRUNK|MOD_DAL), "");
            return ret;
        }   
    }

    return RT_ERR_OK;
}   /* end of _dal_esw_trunk_init_config */

#if defined(CONFIG_TRUNK_FAILOVER_HANDLING)
/* Function Name:
 *      _dal_esw_trunk_setHashbyPortMask
 * Description:
 *      Update Hash mapping table by trunk group ID with link up member port list
 * Input:
 *      unit    - unit id
 *      trk_gid - trunk group id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
static int32
_dal_esw_trunk_setHashbyPortMask(uint32 unit, uint32 trk_gid)
{
    uint32  hashVal_idx;
    uint32  numOfLinkupPort = 0;
    int32   ret;
    rtk_port_t member_port, trunk_member[HAL_MAX_NUM_OF_TRUNKMEMBER(unit)];
    rtk_trunk_hashVal2Port_t hashVal;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "Enter HashHandling Tgid=%d, TrunkLinkUpMemberSet=0x%x", trk_gid, pTrunkLinkUpMemberSet[unit][trk_gid]);
    
    osal_memset(&hashVal, 0, sizeof(rtk_trunk_hashVal2Port_t));

    for (member_port = 0; member_port < RTK_MAX_NUM_OF_PORTS; member_port++)
    {
        if (RTK_PORTMASK_IS_PORT_SET(pTrunkLinkUpMemberSet[unit][trk_gid], member_port))
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "HashHandling number=%d, trunk_member=%d", numOfLinkupPort, member_port);
            trunk_member[numOfLinkupPort] = member_port;
            numOfLinkupPort++;
        }
    }
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "HashHandling total numOfLinkupPort=%d", numOfLinkupPort);
    if (numOfLinkupPort > 0)
    {
        for (hashVal_idx = 0; hashVal_idx < HAL_MAX_NUM_OF_TRUNKHASHVAL(unit); hashVal_idx++)
        {
            hashVal.value[hashVal_idx] = trunk_member[hashVal_idx - (hashVal_idx / numOfLinkupPort) * numOfLinkupPort];
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "Tgid=%d, hashVal=%d", trk_gid, hashVal.value[hashVal_idx]);
        }
    }
    if ((ret = dal_esw_trunk_hashMappingTable_set(unit, trk_gid, &hashVal)))
    {
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }
    return RT_ERR_OK;
} /* end of _dal_esw_trunk_setHashbyPortMask */
#endif
