/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * $Revision: 21770 $
 * $Date: 2011-08-31 15:33:06 +0800 (Wed, 31 Aug 2011) $
 *
 * Purpose : Definition those public Trap APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *            1) Packets trap to CPU setting.
 *            2) RMA (Reserved MAC address).
 *
 */

/*  
 * Include Files 
 */
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_bitop.h>
#include <common/debug/rt_log.h>
#include <osal/sem.h>
#include <osal/lib.h>
#include <osal/memory.h>
#include <hal/chipdef/allmem.h>
#include <hal/chipdef/allreg.h>
#include <hal/chipdef/esw/rtk_esw_table_struct.h>
#include <hal/chipdef/esw/rtk_esw_reg_struct.h>
#include <hal/common/halctrl.h>
#include <hal/mac/reg.h>
#include <hal/mac/mem.h>
#include <dal/esw/dal_esw_trap.h>
#include <rtk/default.h>
#include <rtk/trap.h>
/* 
 * Symbol Definition 
 */
#define     RMA_ADDR_PREFIX_LEN         5
#define     ESW_UDL2RMA_ENTRY           4
#define     ESW_UDL34RMA_ENTRY          2
#define     ESW_MAX_NUM_OF_CFMMDLEVEL   7
/* 
 * Data Declaration 
 */
static uint32               trap_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         trap_sem[RTK_MAX_NUM_OF_UNIT];

/* prefix of reserve multicast address */
const static uint8 rma_prefix[5] = {0x01, 0x80, 0xC2, 0x00, 0x00};
/*
 * Macro Definition
 */
/* trap semaphore handling */
#define TRAP_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(trap_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_DAL|MOD_TRAP), "semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)
#define TRAP_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(trap_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_DAL|MOD_TRAP), "semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)



/*
 * Function Declaration
 */
static int32 _dal_esw_trap_init_config(uint32 unit);

/* Function Name:
 *      dal_esw_trap_init
 * Description:
 *      Initial the trap module of the specified device..
 * Input:
 *      unit - unit id
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
int32
dal_esw_trap_init(uint32 unit)
{
    int32   ret = RT_ERR_FAILED;

    trap_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    trap_sem[unit] = osal_sem_mutex_create();
    if (0 == trap_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_TRAP), "semaphore create failed");
        return RT_ERR_FAILED;
    }
    
    /* set init flag to complete init */
    trap_init[unit] = INIT_COMPLETED;

    if ((ret = _dal_esw_trap_init_config(unit)) != RT_ERR_OK)
    {
        trap_init[unit] = INIT_NOT_COMPLETED;
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "Trap default config initialize failed");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_esw_trap_init */

/* Module Name    : Trap                                    */
/* Sub-module Name: Configuration for traping packet to CPU */

/* Module Name    : Trap     */
/* Sub-module Name: RMA      */

/* Function Name:
 *      dal_esw_trap_rmaAction_get
 * Description:
 *      Get action of reserved multicast address(RMA) frame.
 * Input:
 *      unit        - unit id
 *      pRma_frame  - Reserved multicast address.
 * Output:
 *      pRma_action - RMA action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_INPUT        - Invalid input parameter
 *      RT_ERR_RMA_ADDR     - invalid rma mac address
 *      RT_ERR_NULL_POINTER - NULL pointer
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 * Note:
 *      None.
 */
int32
dal_esw_trap_rmaAction_get(uint32 unit, rtk_mac_t *pRma_frame, rtk_trap_rma_action_t *pRma_action)
{
    int32   ret;
    uint32  value;
    uint32 index;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d", unit);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pRma_action), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pRma_frame), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(osal_memcmp(pRma_frame->octet, rma_prefix, RMA_ADDR_PREFIX_LEN), RT_ERR_RMA_ADDR);
    RT_PARAM_CHK(pRma_frame->octet[5] > 0x2f, RT_ERR_RMA_ADDR);
    /*BPDU & Dotx1x have per-port control register*/
    RT_PARAM_CHK(pRma_frame->octet[5] == 0, RT_ERR_CHIP_NOT_SUPPORTED);
    RT_PARAM_CHK(pRma_frame->octet[5] == 3, RT_ERR_CHIP_NOT_SUPPORTED);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pRma_frame=%x-%x-%x-%x-%x-%x", 
           pRma_frame->octet[0], pRma_frame->octet[1], pRma_frame->octet[2], pRma_frame->octet[3], 
           pRma_frame->octet[4], pRma_frame->octet[5]);    

    /*BPDU & Dotx1x have per-port control register*/
    if((pRma_frame->octet[5] == 0) || (pRma_frame->octet[5] == 3))
    {
        *pRma_action = RMA_ACTION_FORWARD;
        return RT_ERR_OK;
     }

    index = pRma_frame->octet[5];
    
    TRAP_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((index % 4) == 0)
    {
        if((ret = reg_field_read(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMAACT0f,  &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if ((index % 4) == 1)
    {
        if((ret = reg_field_read(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMAACT1f,  &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if ((index % 4) == 2)
    {
        if((ret = reg_field_read(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMAACT2f,  &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if ((index % 4) == 3)
    {
        if((ret = reg_field_read(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMAACT3f,  &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }    

    TRAP_SEM_UNLOCK(unit);
    
    switch (value)
    {
        case 0:
            *pRma_action = RMA_ACTION_FORWARD;
            break;
        case 1:
            *pRma_action = RMA_ACTION_TRAP2CPU;
            break;
        case 2:
            *pRma_action = RMA_ACTION_DROP;
            break;
        case 3:
            *pRma_action = RMA_ACTION_COPY2CPU;
            break;
        default:
            return RT_ERR_FAILED;
    }
            
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pRma_action=%d", *pRma_action);    
            
    return RT_ERR_OK;
} /* end of dal_esw_trap_rmaAction_get */

/* Function Name:
 *      dal_esw_trap_rmaAction_set
 * Description:
 *      Set action of reserved multicast address(RMA) frame.
 * Input:
 *      unit       - unit id
 *      pRma_frame - Reserved multicast address.
 *      rma_action - RMA action
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID    - Invalid unit id
 *      RT_ERR_INPUT      - Invalid input parameter
 *      RT_ERR_RMA_ACTION - Invalid RMA action
 *      RT_ERR_RMA_ADDR     - invalid rma mac address
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 * Note:
 *      The supported Reserved Multicast Address frame:
 *      Assignment                                                                  Address
 *      RMA_BRG_GROUP (Bridge Group Address)                                        01-80-C2-00-00-00
 *      RMA_FD_PAUSE (IEEE Std 802.3, 1988 Edition, Full Duplex PAUSE operation)    01-80-C2-00-00-01
 *      RMA_SP_MCAST (IEEE Std 802.3ad Slow Protocols-Multicast address)            01-80-C2-00-00-02
 *      RMA_1X_PAE (IEEE Std 802.1X PAE address)                                    01-80-C2-00-00-03
 *      RMA_RESERVED04 (Reserved)                                                   01-80-C2-00-00-04
 *      RMA_MEDIA_ACCESS_USE (Media Access Method Specific Use)                     01-80-C2-00-00-05
 *      RMA_RESERVED06 (Reserved)                                                   01-80-C2-00-00-06
 *      RMA_RESERVED07 (Reserved)                                                   01-80-C2-00-00-07
 *      RMA_PVD_BRG_GROUP (Provider Bridge Group Address)                           01-80-C2-00-00-08
 *      RMA_RESERVED09 (Reserved)                                                   01-80-C2-00-00-09
 *      RMA_RESERVED0A (Reserved)                                                   01-80-C2-00-00-0A
 *      RMA_RESERVED0B (Reserved)                                                   01-80-C2-00-00-0B
 *      RMA_RESERVED0C (Reserved)                                                   01-80-C2-00-00-0C
 *      RMA_MVRP (Provider Bridge MVRP Address)                                     01-80-C2-00-00-0D
 *      RMA_1ab_LL_DISCOVERY (802.1ab Link Layer Discover Protocol Address)         01-80-C2-00-00-0E
 *      RMA_RESERVED0F (Reserved)                                                   01-80-C2-00-00-0F
 *      RMA_BRG_MNGEMENT (All LANs Bridge Management Group Address)                 01-80-C2-00-00-10
 *      RMA_LOAD_SERV_GENERIC_ADDR (Load Server Generic Address)                    01-80-C2-00-00-11
 *      RMA_LOAD_DEV_GENERIC_ADDR (Loadable Device Generic Address)                 01-80-C2-00-00-12
 *      RMA_RESERVED13 (Reserved)                                                   01-80-C2-00-00-13
 *      RMA_RESERVED14 (Reserved)                                                   01-80-C2-00-00-14
 *      RMA_RESERVED15 (Reserved)                                                   01-80-C2-00-00-15
 *      RMA_RESERVED16 (Reserved)                                                   01-80-C2-00-00-16
 *      RMA_RESERVED17 (Reserved)                                                   01-80-C2-00-00-17
 *      RMA_MANAGER_STA_GENERIC_ADDR (Generic Address for All Manager Stations)     01-80-C2-00-00-18
 *      RMA_RESERVED19 (Reserved)                                                   01-80-C2-00-00-19
 *      RMA_AGENT_STA_GENERIC_ADDR (Generic Address for All Agent Stations)         01-80-C2-00-00-1A
 *      RMA_RESERVED1B (Reserved)                                                   01-80-C2-00-00-1B
 *      RMA_RESERVED1C (Reserved)                                                   01-80-C2-00-00-1C
 *      RMA_RESERVED1D (Reserved)                                                   01-80-C2-00-00-1D
 *      RMA_RESERVED1E (Reserved)                                                   01-80-C2-00-00-1E
 *      RMA_RESERVED1F (Reserved)                                                   01-80-C2-00-00-1F
 *      RMA_GMRP (GMRP Address)                                                     01-80-C2-00-00-20
 *      RMA_GVRP (GVRP address)                                                     01-80-C2-00-00-21
 *      RMA_UNDEF_GARP22~2F (Undefined GARP address)                                01-80-C2-00-00-22
 *                                                                                ~ 01-80-C2-00-00-2F
 *
 *      The supported Reserved Multicast Address action:
 *      -   RMA_ACTION_FORWARD
 *      -   RMA_ACTION_DROP
 *      -   RMA_ACTION_TRAP2CPU
 */
int32
dal_esw_trap_rmaAction_set(uint32 unit, rtk_mac_t *pRma_frame, rtk_trap_rma_action_t rma_action)
{
    int32   ret;
    uint32  value;
    uint32 index;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, rma_action=%d", unit, rma_action);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pRma_frame), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(osal_memcmp(pRma_frame->octet, rma_prefix, RMA_ADDR_PREFIX_LEN), RT_ERR_RMA_ADDR);
    RT_PARAM_CHK(pRma_frame->octet[5] > 0x2f, RT_ERR_RMA_ADDR);
    /*BPDU & Dotx1x have per-port control register*/
    RT_PARAM_CHK(pRma_frame->octet[5] == 0, RT_ERR_CHIP_NOT_SUPPORTED);
    RT_PARAM_CHK(pRma_frame->octet[5] == 3, RT_ERR_CHIP_NOT_SUPPORTED);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pRma_frame=%x-%x-%x-%x-%x-%x", 
           pRma_frame->octet[0], pRma_frame->octet[1], pRma_frame->octet[2], pRma_frame->octet[3], 
           pRma_frame->octet[4], pRma_frame->octet[5]);    

    /*BPDU & Dotx1x have per-port control register*/
    if((pRma_frame->octet[5] == 0) || (pRma_frame->octet[5] == 3))
    {
        return RT_ERR_OK;
     }

    switch (rma_action)
    {
        case RMA_ACTION_FORWARD:
            value = 0;
            break;
        case RMA_ACTION_TRAP2CPU:
            value = 1;
            break;
        case RMA_ACTION_DROP:
            value = 2;
            break;
        case RMA_ACTION_COPY2CPU:
            value = 3;
            break;
        default:
            return RT_ERR_FAILED;
    }

    index = pRma_frame->octet[5];
    
    TRAP_SEM_LOCK(unit);
    /* set entry from CHIP*/
    if ((index % 4) == 0)
    {
        if((ret = reg_field_write(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMAACT0f,  &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if ((index % 4) == 1)
    {
        if((ret = reg_field_write(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMAACT1f,  &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if ((index % 4) == 2)
    {
        if((ret = reg_field_write(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMAACT2f,  &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if ((index % 4) == 3)
    {
        if((ret = reg_field_write(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMAACT3f,  &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }    
    
    TRAP_SEM_UNLOCK(unit);    
                        
    return RT_ERR_OK;
} /* end of dal_esw_trap_rmaAction_set */

/* Function Name:
 *      dal_esw_trap_rmaPri_get
 * Description:
 *      Get priority of packets trapped to CPU.
 * Input:
 *      unit       - unit id
 *      pRma_frame - Reserved multicast address.
 * Output:
 *      pPriority  - pointer to priority
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_RMA_ADDR         - invalid invalid RMA address
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 * Note:
 *      None
 */
int32
dal_esw_trap_rmaPri_get(uint32 unit, rtk_mac_t *pRma_frame, rtk_pri_t *pPriority)
{
    int32   ret;
    uint32  index;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d", unit);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pPriority), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pRma_frame), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(osal_memcmp(pRma_frame->octet, rma_prefix, RMA_ADDR_PREFIX_LEN), RT_ERR_RMA_ADDR);
    RT_PARAM_CHK(pRma_frame->octet[5] > 0x2f, RT_ERR_RMA_ADDR);
    /*BPDU & Dotx1x have per-port control register*/
    RT_PARAM_CHK(pRma_frame->octet[5] == 0, RT_ERR_CHIP_NOT_SUPPORTED);
    RT_PARAM_CHK(pRma_frame->octet[5] == 3, RT_ERR_CHIP_NOT_SUPPORTED);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pRma_frame=%x-%x-%x-%x-%x-%x", 
           pRma_frame->octet[0], pRma_frame->octet[1], pRma_frame->octet[2], pRma_frame->octet[3], 
           pRma_frame->octet[4], pRma_frame->octet[5]);    

    index = pRma_frame->octet[5];
    
    /*BPDU & Dotx1x have per-port control register*/
    if((pRma_frame->octet[5] == 0) || (pRma_frame->octet[5] == 3))
    {
        *pPriority = 0;
        return RT_ERR_OK;
    }
    
    TRAP_SEM_LOCK(unit);

    /* get entry from CHIP*/
    if ((index % 4) == 0)
    {
        if((ret = reg_field_read(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMARMAPRI0f,  pPriority)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if ((index % 4) == 1)
    {
        if((ret = reg_field_read(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMARMAPRI1f,  pPriority)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return RT_ERR_FAILED;
        }
    }
    else if ((index % 4) == 2)
    {
        if((ret = reg_field_read(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMARMAPRI2f,  pPriority)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if ((index % 4) == 3)
    {
        if((ret = reg_field_read(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMARMAPRI3f,  pPriority)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return RT_ERR_FAILED;
        }
    }        
    
    TRAP_SEM_UNLOCK(unit);
           
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pPriority=%d", *pPriority);    
            
    return RT_ERR_OK;
} /* end of dal_esw_trap_rmaPri_get */

/* Function Name:
 *      dal_esw_trap_rmaPri_set
 * Description:
 *      Set priority of packets trapped to CPU.
 * Input:
 *      unit       - unit id
 *      pRma_frame - Reserved multicast address.
 *      priority   - priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_RMA_ADDR         - invalid invalid RMA address
 *      RT_ERR_PRIORITY         - invalid priority value
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 * Note:
 *      None
 */
int32
dal_esw_trap_rmaPri_set(uint32 unit, rtk_mac_t *pRma_frame, rtk_pri_t priority)
{
    int32   ret;
    uint32  index;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, priority=%d", unit, priority);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pRma_frame), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((priority > RTK_DOT1P_PRIORITY_MAX), RT_ERR_PRIORITY);
    RT_PARAM_CHK(osal_memcmp(pRma_frame->octet, rma_prefix, RMA_ADDR_PREFIX_LEN), RT_ERR_RMA_ADDR);
    RT_PARAM_CHK(pRma_frame->octet[5] > 0x2f, RT_ERR_RMA_ADDR);
    /*BPDU & Dotx1x have per-port control register*/
    RT_PARAM_CHK(pRma_frame->octet[5] == 0, RT_ERR_CHIP_NOT_SUPPORTED);
    RT_PARAM_CHK(pRma_frame->octet[5] == 3, RT_ERR_CHIP_NOT_SUPPORTED);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pRma_frame=%x-%x-%x-%x-%x-%x", 
           pRma_frame->octet[0], pRma_frame->octet[1], pRma_frame->octet[2], pRma_frame->octet[3], 
           pRma_frame->octet[4], pRma_frame->octet[5]);    

    index = pRma_frame->octet[5];
    
    /*BPDU & Dotx1x have per-port control register*/
    if((pRma_frame->octet[5] == 0) || (pRma_frame->octet[5] == 3))
    {
        return RT_ERR_OK;
     }
    
    TRAP_SEM_LOCK(unit);

    /*Set entry to CHIP*/
    if ((index % 4) == 0)
    {
        if((ret = reg_field_write(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMARMAPRI0f,  &priority)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return RT_ERR_FAILED;
        }
    }
    else if ((index % 4) == 1)
    {
        if((ret = reg_field_write(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMARMAPRI1f,  &priority)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return RT_ERR_FAILED;
        }
    }
    else if ((index % 4) == 2)
    {
        if((ret = reg_field_write(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMARMAPRI2f,  &priority)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return RT_ERR_FAILED;
        }
    }
    else if ((index % 4) == 3)
    {
        if((ret = reg_field_write(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMARMAPRI3f,  &priority)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return RT_ERR_FAILED;
        }
    }        
    
    TRAP_SEM_UNLOCK(unit);
                       
    return RT_ERR_OK;
} /* end of dal_esw_trap_rmaPri_set */

/* Function Name:
 *      dal_esw_trap_rmaPriEnable_get
 * Description:
 *      Get priority enable status of packets trapped to CPU.
 * Input:
 *      unit       - unit id
 *      pRma_frame - Reserved multicast address.
 * Output:
 *      pEnable    - pointer to priority enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_RMA_ADDR         - invalid invalid RMA address
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 * Note:
 *      None
 */
int32
dal_esw_trap_rmaPriEnable_get(uint32 unit, rtk_mac_t *pRma_frame, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value;
    uint32  index;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d", unit);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pRma_frame), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(osal_memcmp(pRma_frame->octet, rma_prefix, RMA_ADDR_PREFIX_LEN), RT_ERR_RMA_ADDR);
    RT_PARAM_CHK(pRma_frame->octet[5] > 0x2f, RT_ERR_RMA_ADDR);
    /*BPDU & Dotx1x have per-port control register*/
    RT_PARAM_CHK(pRma_frame->octet[5] == 0, RT_ERR_CHIP_NOT_SUPPORTED);
    RT_PARAM_CHK(pRma_frame->octet[5] == 3, RT_ERR_CHIP_NOT_SUPPORTED);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pRma_frame=%x-%x-%x-%x-%x-%x", 
           pRma_frame->octet[0], pRma_frame->octet[1], pRma_frame->octet[2], pRma_frame->octet[3], 
           pRma_frame->octet[4], pRma_frame->octet[5]);    

    index = pRma_frame->octet[5];
    
    /*BPDU & Dotx1x have per-port control register*/
    if((pRma_frame->octet[5] == 0) || (pRma_frame->octet[5] == 3))
    {
        return RT_ERR_OK;
    }
    
    TRAP_SEM_LOCK(unit);

    /* get entry from CHIP*/
    if ((index % 4) == 0)
    {
        if((ret = reg_field_read(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMADFRMAPRI0f,  &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if ((index % 4) == 1)
    {
        if((ret = reg_field_read(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMADFRMAPRI1f,  &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if ((index % 4) == 2)
    {
        if((ret = reg_field_read(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMADFRMAPRI2f,  &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if ((index % 4) == 3)
    {
        if((ret = reg_field_read(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMADFRMAPRI3f,  &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }        
    
    TRAP_SEM_UNLOCK(unit);
           
    switch (value)
    {
        case 0:
            *pEnable = DISABLED;
            break;
        
        case 1:
            *pEnable = ENABLED;
            break;
        
        default:
            return RT_ERR_FAILED;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pEnable=%d", *pEnable);    
            
    return RT_ERR_OK;
} /* end of dal_esw_trap_rmaPriEnable_get */

/* Function Name:
 *      dal_esw_trap_rmaPriEnable_set
 * Description:
 *      Set priority enable status of packets trapped to CPU.
 * Input:
 *      unit       - unit id
 *      pRma_frame - Reserved multicast address.
 *      enable     - priority enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_RMA_ADDR         - invalid invalid RMA address
 *      RT_ERR_PRIORITY         - invalid priority value
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 * Note:
 *      None
 */
int32
dal_esw_trap_rmaPriEnable_set(uint32 unit, rtk_mac_t *pRma_frame, rtk_enable_t enable)
{
    int32   ret;
    uint32  index, value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, enable=%d", unit, enable);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pRma_frame), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(enable >= RTK_ENABLE_END, RT_ERR_INPUT);
    RT_PARAM_CHK(osal_memcmp(pRma_frame->octet, rma_prefix, RMA_ADDR_PREFIX_LEN), RT_ERR_RMA_ADDR);
    RT_PARAM_CHK(pRma_frame->octet[5] > 0x2f, RT_ERR_RMA_ADDR);
    /*BPDU & Dotx1x have per-port control register*/
    RT_PARAM_CHK(pRma_frame->octet[5] == 0, RT_ERR_CHIP_NOT_SUPPORTED);
    RT_PARAM_CHK(pRma_frame->octet[5] == 3, RT_ERR_CHIP_NOT_SUPPORTED);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pRma_frame=%x-%x-%x-%x-%x-%x", 
           pRma_frame->octet[0], pRma_frame->octet[1], pRma_frame->octet[2], pRma_frame->octet[3], 
           pRma_frame->octet[4], pRma_frame->octet[5]);    

    index = pRma_frame->octet[5];
    
    /*BPDU & Dotx1x have per-port control register*/
    if((pRma_frame->octet[5] == 0) || (pRma_frame->octet[5] == 3))
    {
        return RT_ERR_OK;
     }
    
    switch (enable)
    {
        case DISABLED:
            value = 0;
            break;
        
        case ENABLED:
            value = 1;
            break;
        
        default:
            return RT_ERR_INPUT;
    }

    TRAP_SEM_LOCK(unit);

    /*Set entry to CHIP*/
    if ((index % 4) == 0)
    {
        if((ret = reg_field_write(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMADFRMAPRI0f, &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }     
    }
    else if ((index % 4) == 1)
    {
        if((ret = reg_field_write(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMADFRMAPRI1f, &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }     
    }
    else if ((index % 4) == 2)
    {
        if((ret = reg_field_write(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMADFRMAPRI2f, &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }     
    }
    else if ((index % 4) == 3)
    {
        if((ret = reg_field_write(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMADFRMAPRI3f, &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }     
    }        
    
    TRAP_SEM_UNLOCK(unit);
                       
    return RT_ERR_OK;
} /* end of dal_esw_trap_rmaPriEnable_set */

/* Function Name:
 *      dal_esw_trap_rmaCpuTagAddEnable_get
 * Description:
 *      Get enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of CPU tag adding
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_trap_rmaCpuTagAddEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d", unit);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    TRAP_SEM_LOCK(unit);

    /*Get entry from CHIP*/
    if((ret = reg_field_read(unit, ESW_RESERVED_MULTICAST_ADDRESS_VID_CONTROL0r, 
                              ESW_RMACPUTAGf, pEnable)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }   
    
    TRAP_SEM_UNLOCK(unit);

    if(*pEnable == 1)
        *pEnable = ENABLED;
    else
        *pEnable = DISABLED;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pEnable=%d", *pEnable);
                       
    return RT_ERR_OK;
} /* end of dal_esw_trap_rmaCpuTagAddEnable_get */


/* Function Name:
 *      dal_esw_trap_rmaCpuTagAddEnable_set
 * Description:
 *      Set enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit   - unit id
 *      enable - enable status of CPU tag adding
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_trap_rmaCpuTagAddEnable_set(uint32 unit, rtk_enable_t enable)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, enable=%d", unit, enable);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(enable >= RTK_ENABLE_END, RT_ERR_INPUT);
    
    TRAP_SEM_LOCK(unit);

    /*Set entry to CHIP*/
    if((ret = reg_field_write(unit, ESW_RESERVED_MULTICAST_ADDRESS_VID_CONTROL0r, 
                              ESW_RMACPUTAGf, &enable)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }   
    
    TRAP_SEM_UNLOCK(unit);
                       
    return RT_ERR_OK;
} /* end of dal_esw_trap_rmaCpuTagAddEnable_set */

/* Function Name:
 *      dal_esw_trap_rmaVlanCheckEnable_get
 * Description:
 *      Get enable status of vlan checking on specified RMA frame.
 * Input:
 *      unit       - unit id
 *      pRma_frame - Reserved multicast address.
 * Output:
 *      pEnable    - pointer to enable status of vlan checking
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_RMA_ADDR         - invalid invalid RMA address
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 * Note:
 *      None
 */
int32
dal_esw_trap_rmaVlanCheckEnable_get(uint32 unit, rtk_mac_t *pRma_frame, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32 index;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d", unit);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pRma_frame), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(osal_memcmp(pRma_frame->octet, rma_prefix, RMA_ADDR_PREFIX_LEN), RT_ERR_RMA_ADDR);
    RT_PARAM_CHK(pRma_frame->octet[5] > 0x2f, RT_ERR_RMA_ADDR);
    /*BPDU & Dotx1x have per-port control register*/
    RT_PARAM_CHK(pRma_frame->octet[5] == 0, RT_ERR_CHIP_NOT_SUPPORTED);
    RT_PARAM_CHK(pRma_frame->octet[5] == 3, RT_ERR_CHIP_NOT_SUPPORTED);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pRma_frame=%x-%x-%x-%x-%x-%x", 
           pRma_frame->octet[0], pRma_frame->octet[1], pRma_frame->octet[2], pRma_frame->octet[3], 
           pRma_frame->octet[4], pRma_frame->octet[5]);    

    index = pRma_frame->octet[5];
    
    if((pRma_frame->octet[5] == 0) || (pRma_frame->octet[5] == 3))
    {
        *pEnable = DISABLED;
        return RT_ERR_OK;
    }
    
    TRAP_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((index % 4) == 0)
    {
        if((ret = reg_field_read(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMAVLANCARE0f,  pEnable)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if ((index % 4) == 1)
    {
        if((ret = reg_field_read(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMAVLANCARE1f,  pEnable)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if ((index % 4) == 2)
    {
        if((ret = reg_field_read(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMAVLANCARE2f,  pEnable)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if ((index % 4) == 3)
    {
        if((ret = reg_field_read(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMAVLANCARE3f,  pEnable)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }    

    TRAP_SEM_UNLOCK(unit);
            
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pEnable=%d", *pEnable);    
            
    return RT_ERR_OK;
} /* end of dal_esw_trap_rmaVlanCheckEnable_get */

/* Function Name:
 *      dal_esw_trap_rmaVlanCheckEnable_set
 * Description:
 *      Set enable status of vlan checking on specified RMA frame.
 * Input:
 *      unit       - unit id
 *      pRma_frame - Reserved multicast address.
 *      enable     - enable status of vlan checking
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_RMA_ADDR         - invalid invalid RMA address
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 * Note:
 *      None
 */
int32
dal_esw_trap_rmaVlanCheckEnable_set(uint32 unit, rtk_mac_t *pRma_frame, rtk_enable_t enable)
{
    int32   ret;
    uint32 index;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, enable=%d", unit, enable);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pRma_frame), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(osal_memcmp(pRma_frame->octet, rma_prefix, RMA_ADDR_PREFIX_LEN), RT_ERR_RMA_ADDR);
    RT_PARAM_CHK(pRma_frame->octet[5] > 0x2f, RT_ERR_RMA_ADDR);
    /*BPDU & Dotx1x have per-port control register*/
    RT_PARAM_CHK(pRma_frame->octet[5] == 0, RT_ERR_CHIP_NOT_SUPPORTED);
    RT_PARAM_CHK(pRma_frame->octet[5] == 3, RT_ERR_CHIP_NOT_SUPPORTED);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pRma_frame=%x-%x-%x-%x-%x-%x", 
           pRma_frame->octet[0], pRma_frame->octet[1], pRma_frame->octet[2], pRma_frame->octet[3], 
           pRma_frame->octet[4], pRma_frame->octet[5]);    

    index = pRma_frame->octet[5];
    
    if((pRma_frame->octet[5] == 0) || (pRma_frame->octet[5] == 3))
    {
        return RT_ERR_OK;
    }
    
    TRAP_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((index % 4) == 0)
    {
        if((ret = reg_field_write(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMAVLANCARE0f,  &enable)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if ((index % 4) == 1)
    {
        if((ret = reg_field_write(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMAVLANCARE1f,  &enable)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if ((index % 4) == 2)
    {
        if((ret = reg_field_write(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMAVLANCARE2f,  &enable)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if ((index % 4) == 3)
    {
        if((ret = reg_field_write(unit, ESW_RESERVED_MULTICAST_ADDRESS_GROUP0_CONTROL0r + (index / 4), 
                              ESW_RMAVLANCARE3f,  &enable)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }    

    TRAP_SEM_UNLOCK(unit);
                        
    return RT_ERR_OK;
} /* end of dal_esw_trap_rmaVlanCheckEnable_set */

/* Module Name    : Trap     */
/* Sub-module Name: User defined RMA */

/* Function Name:
 *      dal_esw_trap_userDefineRma_get
 * Description:
 *      Get user defined RMA.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined RMA entry
 * Output:
 *      pUserDefinedRma - pointer to content of user defined RMA
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_trap_userDefineRma_get(
    uint32                      unit, 
    uint32                      userDefine_idx, 
    rtk_trap_userDefinedRma_t   *pUserDefinedRma)
{
    int32   ret;
    uint32 val;
    uint32 baseAddr;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, userDefine_idx=%d", 
                unit, userDefine_idx);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((userDefine_idx >= ESW_UDL2RMA_ENTRY), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pUserDefinedRma), RT_ERR_NULL_POINTER);

    TRAP_SEM_LOCK(unit);
    baseAddr = ESW_USER_DEFINED_GROUP0_RESERVED_MULTICAST_ADDRESS_CONTROL0r + userDefine_idx * 4;

    /* get entry from CHIP*/
    if((ret = reg_read(unit, baseAddr, &val)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }    
    pUserDefinedRma->mac.octet[0] = (val >> 24) & 0xff;
    pUserDefinedRma->mac.octet[1] = (val >> 16) & 0xff;
    pUserDefinedRma->mac.octet[2] = (val >> 8) & 0xff;
    pUserDefinedRma->mac.octet[3] = val & 0xff;

    if((ret = reg_read(unit, baseAddr + 1, &val)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 
    pUserDefinedRma->mac.octet[4] = (val >> 24) & 0xff;
    pUserDefinedRma->mac.octet[5] = (val >> 16) & 0xff;
    pUserDefinedRma->macMask.octet[0] = (val >> 8) & 0xff;
    pUserDefinedRma->macMask.octet[1] = val & 0xff;

    if((ret = reg_read(unit, baseAddr + 2, &val)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 

    TRAP_SEM_UNLOCK(unit);
    
    pUserDefinedRma->macMask.octet[2] = (val >> 24) & 0xff;
    pUserDefinedRma->macMask.octet[3] = (val >> 16) & 0xff;
    pUserDefinedRma->macMask.octet[4] = (val >> 8) & 0xff;
    pUserDefinedRma->macMask.octet[5] = val & 0xff;    
                
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pUserDefinedRma->mac=%x-%x-%x-%x-%x-%x", 
           pUserDefinedRma->mac.octet[0], pUserDefinedRma->mac.octet[1], pUserDefinedRma->mac.octet[2], 
           pUserDefinedRma->mac.octet[3], pUserDefinedRma->mac.octet[4], pUserDefinedRma->mac.octet[5]);  

     RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pUserDefinedRma->macMask=%x-%x-%x-%x-%x-%x", 
           pUserDefinedRma->macMask.octet[0], pUserDefinedRma->macMask.octet[1], pUserDefinedRma->macMask.octet[2], 
           pUserDefinedRma->macMask.octet[3], pUserDefinedRma->macMask.octet[4], pUserDefinedRma->macMask.octet[5]);    
            
    return RT_ERR_OK;
} /* end of dal_esw_trap_userDefineRma_get */

/* Function Name:
 *      dal_esw_trap_userDefineRma_set
 * Description:
 *      Set user defined RMA.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined RMA entry
 *      pUserDefinedRma - to content of user defined RMA
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_trap_userDefineRma_set(
    uint32                      unit, 
    uint32                      userDefine_idx, 
    rtk_trap_userDefinedRma_t   *pUserDefinedRma)
{
    int32   ret;
    uint32  baseAddr, val;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, userDefine_idx=%d", 
                unit, userDefine_idx);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((userDefine_idx >= ESW_UDL2RMA_ENTRY), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pUserDefinedRma), RT_ERR_NULL_POINTER);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pUserDefinedRma->mac=%x-%x-%x-%x-%x-%x", 
       pUserDefinedRma->mac.octet[0], pUserDefinedRma->mac.octet[1], pUserDefinedRma->mac.octet[2], 
       pUserDefinedRma->mac.octet[3], pUserDefinedRma->mac.octet[4], pUserDefinedRma->mac.octet[5]);  

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pUserDefinedRma->macMask=%x-%x-%x-%x-%x-%x", 
           pUserDefinedRma->macMask.octet[0], pUserDefinedRma->macMask.octet[1], pUserDefinedRma->macMask.octet[2], 
           pUserDefinedRma->macMask.octet[3], pUserDefinedRma->macMask.octet[4], pUserDefinedRma->macMask.octet[5]);   

    TRAP_SEM_LOCK(unit);
    baseAddr = ESW_USER_DEFINED_GROUP0_RESERVED_MULTICAST_ADDRESS_CONTROL0r + userDefine_idx * 4;

    /* get entry from CHIP*/
    val = (pUserDefinedRma->mac.octet[0] << 24) | (pUserDefinedRma->mac.octet[1] << 16) | 
          (pUserDefinedRma->mac.octet[2] << 8) | (pUserDefinedRma->mac.octet[3]);
    if((ret = reg_write(unit, baseAddr, &val)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 

    val = (pUserDefinedRma->mac.octet[4] << 24) | (pUserDefinedRma->mac.octet[5] << 16) | 
          (pUserDefinedRma->macMask.octet[0] << 8) | (pUserDefinedRma->macMask.octet[1]);
    if((ret = reg_write(unit, baseAddr + 1, &val)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }   

    val = (pUserDefinedRma->macMask.octet[2] << 24) | (pUserDefinedRma->macMask.octet[3] << 16) | 
          (pUserDefinedRma->macMask.octet[4] << 8) | (pUserDefinedRma->macMask.octet[5]);
    if((ret = reg_write(unit, baseAddr + 2, &val)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }    
    
    TRAP_SEM_UNLOCK(unit); 
            
    return RT_ERR_OK;
} /* end of dal_esw_trap_userDefineRma_set */

/* Function Name:
 *      dal_esw_trap_userDefineRmaAction_get
 * Description:
 *      Get forwarding action of user defined RMA.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined RMA entry
 * Output:
 *      pActoin         - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 *      - ACTION_DROP
 */
int32
dal_esw_trap_userDefineRmaAction_get(uint32 unit, uint32 userDefine_idx, rtk_trap_rma_action_t *pAction)
{
    int32   ret;
    uint32 baseAddr, value;

   RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, userDefine_idx=%d", 
                unit, userDefine_idx);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((userDefine_idx >= ESW_UDL2RMA_ENTRY), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);

    baseAddr = ESW_USER_DEFINED_GROUP0_RESERVED_MULTICAST_ADDRESS_CONTROL0r + userDefine_idx * 4;
    TRAP_SEM_LOCK(unit);

    /* get entry from CHIP*/
    if((ret = reg_field_read(unit, baseAddr + 3,  ESW_UDRMAACTf, &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 

    TRAP_SEM_UNLOCK(unit);   

    switch (value)
    {
        case 0:
            *pAction = RMA_ACTION_FORWARD;
            break;
        case 1:
            *pAction = RMA_ACTION_TRAP2CPU;
            break;
        case 2:
            *pAction = RMA_ACTION_DROP;
            break;
        case 3:
            *pAction = RMA_ACTION_COPY2CPU;
            break;
        default:
            return RT_ERR_FAILED;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pAction=%d", *pAction);
          
    return RT_ERR_OK;
} /* end of dal_esw_trap_userDefineRmaAction_get */

/* Function Name:
 *      dal_esw_trap_userDefineRmaAction_set
 * Description:
 *      Set forwarding action of user defined RMA.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined RMA entry
 *      actoin          - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_FWD_ACTION       - invalid forwarding action
 * Note:
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 *      - ACTION_DROP
 */
int32
dal_esw_trap_userDefineRmaAction_set(uint32 unit, uint32 userDefine_idx, rtk_trap_rma_action_t action)
{
    int32   ret;
    uint32 baseAddr, value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, userDefine_idx=%d, action=%d", 
                unit, userDefine_idx, action);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((userDefine_idx >= ESW_UDL2RMA_ENTRY), RT_ERR_OUT_OF_RANGE);

    switch (action)
    {
        case RMA_ACTION_FORWARD:
            value = 0;
            break;
        case RMA_ACTION_TRAP2CPU:
            value = 1;
            break;
        case RMA_ACTION_DROP:
            value = 2;
            break;
        case RMA_ACTION_COPY2CPU:
            value = 3;
            break;
        default:
            return RT_ERR_FWD_ACTION;
    }

    baseAddr = ESW_USER_DEFINED_GROUP0_RESERVED_MULTICAST_ADDRESS_CONTROL0r + userDefine_idx * 4;
    TRAP_SEM_LOCK(unit);

    /* set entry from CHIP*/
    if((ret = reg_field_write(unit, baseAddr + 3,  ESW_UDRMAACTf, &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 

    TRAP_SEM_UNLOCK(unit);   
    
    return RT_ERR_OK;
} /* end of dal_esw_trap_userDefineRmaAction_set */

/* Function Name:
 *      dal_esw_trap_userDefineRmaPri_get
 * Description:
 *      Get priority of packets trapped to CPU.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined RMA entry
 * Output:
 *      pPriority       - pointer to priority
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_trap_userDefineRmaPri_get(uint32 unit, uint32 userDefine_idx, rtk_pri_t *pPriority)
{
    int32   ret;
    uint32  baseAddr;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, userDefine_idx=%d", unit, userDefine_idx);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((userDefine_idx >= ESW_UDL2RMA_ENTRY), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pPriority), RT_ERR_NULL_POINTER);

    baseAddr = ESW_USER_DEFINED_GROUP0_RESERVED_MULTICAST_ADDRESS_CONTROL0r + userDefine_idx * 4;
    TRAP_SEM_LOCK(unit);

    /* get entry from CHIP*/
    if((ret = reg_field_read(unit, baseAddr + 3,  ESW_UDRMARMAPRIf, pPriority)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }

    TRAP_SEM_UNLOCK(unit);   

     RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pPriority=%d", *pPriority);
          
    return RT_ERR_OK;
} /* end of dal_esw_trap_userDefineRmaPri_get */

/* Function Name:
 *      dal_esw_trap_userDefineRmaPri_set
 * Description:
 *      Set priority of packets trapped to CPU.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined RMA entry
 *      priority        - priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
int32
dal_esw_trap_userDefineRmaPri_set(uint32 unit, uint32 userDefine_idx, rtk_pri_t priority)
{
    int32   ret;
    uint32  baseAddr;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, userDefine_idx=%d, priority=%d", 
                unit, userDefine_idx, priority);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((userDefine_idx >= ESW_UDL2RMA_ENTRY), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((priority > RTK_DOT1P_PRIORITY_MAX), RT_ERR_PRIORITY);

    baseAddr = ESW_USER_DEFINED_GROUP0_RESERVED_MULTICAST_ADDRESS_CONTROL0r + userDefine_idx * 4;
    TRAP_SEM_LOCK(unit);

    /*Set Assigned priority*/
    if((ret = reg_field_write(unit, baseAddr + 3,  ESW_UDRMARMAPRIf, &priority)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }

    TRAP_SEM_UNLOCK(unit);   
          
    return RT_ERR_OK;
}   /* end of dal_esw_trap_userDefineRmaPri_set */

/* Function Name:
 *      dal_esw_trap_userDefineRmaPriEnable_get
 * Description:
 *      Get priority enable status of packets trapped to CPU.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined RMA entry
 * Output:
 *      pEnable         - pointer to priority enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_trap_userDefineRmaPriEnable_get(uint32 unit, uint32 userDefine_idx, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  baseAddr, value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, userDefine_idx=%d", unit, userDefine_idx);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((userDefine_idx >= ESW_UDL2RMA_ENTRY), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    baseAddr = ESW_USER_DEFINED_GROUP0_RESERVED_MULTICAST_ADDRESS_CONTROL0r + userDefine_idx * 4;
    TRAP_SEM_LOCK(unit);

    /* get entry from CHIP*/
    if((ret = reg_field_read(unit, baseAddr + 3,  ESW_UDRMADFRMAPRIf, &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 

    TRAP_SEM_UNLOCK(unit);   

    switch (value)
    {
        case 0:
            *pEnable = DISABLED;
            break;
        
        case 1:
            *pEnable = ENABLED;
            break;
        
        default:
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pEnable=%d", *pEnable);
          
    return RT_ERR_OK;
} /* end of dal_esw_trap_userDefineRmaPriEnable_get */

/* Function Name:
 *      dal_esw_trap_userDefineRmaPriEnable_set
 * Description:
 *      Set priority of packets trapped to CPU.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined RMA entry
 *      enable          - priority enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
int32
dal_esw_trap_userDefineRmaPriEnable_set(uint32 unit, uint32 userDefine_idx, rtk_enable_t enable)
{
    int32   ret;
    uint32  baseAddr, value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, userDefine_idx=%d, enable=%d", 
                unit, userDefine_idx, enable);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((userDefine_idx >= ESW_UDL2RMA_ENTRY), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(enable >= RTK_ENABLE_END, RT_ERR_INPUT);
    
    switch (enable)
    {
        case DISABLED:
            value = 0;
            break;
        
        case ENABLED:
            value = 1;
            break;
        
        default:
            return RT_ERR_INPUT;
    }
    
    baseAddr = ESW_USER_DEFINED_GROUP0_RESERVED_MULTICAST_ADDRESS_CONTROL0r + userDefine_idx * 4;
    TRAP_SEM_LOCK(unit);

    /*Enable priority assigned*/
    if((ret = reg_field_write(unit, baseAddr + 3,  ESW_UDRMADFRMAPRIf, &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 

    TRAP_SEM_UNLOCK(unit);   
          
    return RT_ERR_OK;
}   /* end of dal_esw_trap_userDefineRmaPriEnable_set */

/* Function Name:
 *      dal_esw_trap_userDefineRmaVlanCheckEnable_get
 * Description:
 *      Get enable status of vlan checking on specified user defined RMA frame.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined RMA entry
 * Output:
 *      pEnable         - pointer to enable status of vlan checking
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_trap_userDefineRmaVlanCheckEnable_get(uint32 unit, uint32 userDefine_idx, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32 baseAddr;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d", unit);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((userDefine_idx >= ESW_UDL2RMA_ENTRY), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    baseAddr = ESW_USER_DEFINED_GROUP0_RESERVED_MULTICAST_ADDRESS_CONTROL0r + userDefine_idx * 4;
    TRAP_SEM_LOCK(unit);

    /* get entry from CHIP*/
    if((ret = reg_field_read(unit, baseAddr + 3,  ESW_UDRVLANCAREf, pEnable)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 

    TRAP_SEM_UNLOCK(unit);   

     RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pEnable=%d", *pEnable);
          
    return RT_ERR_OK;
} /* end of dal_esw_trap_userDefineRmaVlanCheckEnable_get */

/* Function Name:
 *      dal_esw_trap_userDefineRmaVlanCheckEnable_set
 * Description:
 *      Set enable status of vlan checking on specified user defined RMA frame.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined RMA entry
 *      enable          - enable status of vlan checking
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_trap_userDefineRmaVlanCheckEnable_set(uint32 unit, uint32 userDefine_idx, rtk_enable_t enable)
{
    int32   ret;
    uint32 baseAddr;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, enable=%d", unit, enable);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((userDefine_idx >= ESW_UDL2RMA_ENTRY), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);

    baseAddr = ESW_USER_DEFINED_GROUP0_RESERVED_MULTICAST_ADDRESS_CONTROL0r + userDefine_idx * 4;
    TRAP_SEM_LOCK(unit);

    /* get entry from CHIP*/
    if((ret = reg_field_write(unit, baseAddr + 3,  ESW_UDRVLANCAREf, &enable)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 

    TRAP_SEM_UNLOCK(unit);   
          
    return RT_ERR_OK;
} /* end of dal_esw_trap_userDefineRmaVlanCheckEnable_set */

/* Function Name:
 *      dal_esw_trap_userDefineRmaStpBlockEnable_get
 * Description:
 *      Get enable status of STP status checking on specified user defined RMA frame.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined RMA entry
 * Output:
 *      pEnable         - pointer to enable status of STP status checking
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_trap_userDefineRmaStpBlockEnable_get(uint32 unit, uint32 userDefine_idx, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32 baseAddr;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d", unit);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((userDefine_idx >= ESW_UDL2RMA_ENTRY), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    baseAddr = ESW_USER_DEFINED_GROUP0_RESERVED_MULTICAST_ADDRESS_CONTROL0r + userDefine_idx * 4;
    TRAP_SEM_LOCK(unit);

    /* get entry from CHIP*/
    if((ret = reg_field_read(unit, baseAddr + 3,  ESW_BPDUf, pEnable)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 

    TRAP_SEM_UNLOCK(unit);   

     RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pEnable=%d", *pEnable);
          
    return RT_ERR_OK;
} /* end of dal_esw_trap_userDefineRmaStpBlockEnable_get */

/* Function Name:
 *      dal_esw_trap_userDefineRmaStpBlockEnable_set
 * Description:
 *      Set enable status of STP status checking on specified user defined RMA frame.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined RMA entry
 *      enable          - enable status of STP status checking
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_trap_userDefineRmaStpBlockEnable_set(uint32 unit, uint32 userDefine_idx, rtk_enable_t enable)
{
    int32   ret;
    uint32 baseAddr;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, enable=%d", unit, enable);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((userDefine_idx >= ESW_UDL2RMA_ENTRY), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);

    baseAddr = ESW_USER_DEFINED_GROUP0_RESERVED_MULTICAST_ADDRESS_CONTROL0r + userDefine_idx * 4;
    TRAP_SEM_LOCK(unit);

    /* get entry from CHIP*/
    if((ret = reg_field_write(unit, baseAddr + 3,  ESW_BPDUf, &enable)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 

    TRAP_SEM_UNLOCK(unit);   
          
    return RT_ERR_OK;
} /* end of dal_esw_trap_userDefineRmaStpBlockEnable_set */

/* Module Name    : Trap                         */
/* Sub-module Name: System-wise management frame */

/* Function Name:
 *      dal_esw_trap_mgmtFrameAction_get
 * Description:
 *      Get forwarding action of management frame.
 * Input:
 *      unit      - unit id
 *      frameType - type of management frame
 * Output:
 *      pActoin   - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Type of management frame is as following:
 *      - MGMT_TYPE_RIP
 *      - MGMT_TYPE_ICMP
 *      - MGMT_TYPE_ICMPV6
 *      - MGMT_TYPE_ARP
 *      - MGMT_TYPE_MLD
 *      - MGMT_TYPE_IGMP
 *      - MGMT_TYPE_BGP
 *      - MGMT_TYPE_OSPFV2
 *      - MGMT_TYPE_OSPFV3
 *      - MGMT_TYPE_SNMP
 *      - MGMT_TYPE_SSH
 *      - MGMT_TYPE_FTP
 *      - MGMT_TYPE_TFTP
 *      - MGMT_TYPE_TELNET
 *      - MGMT_TYPE_HTTP
 *      - MGMT_TYPE_HTTPS
 * 
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 *      - ACTION_DROP
 *      - ACTION_COPY2CPU
 */
int32
dal_esw_trap_mgmtFrameAction_get(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_action_t *pAction)
{
    int32   ret;
    uint32  baseAddr, value;
    uint32  chip_frameType;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, frameType=%d", unit, frameType);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((frameType > MGMT_TYPE_HTTPS), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);

    switch (frameType)
    {
        case MGMT_TYPE_RIP:
            chip_frameType = 0;
            break;
        case MGMT_TYPE_ICMP:
            chip_frameType = 2;
            break;
        case MGMT_TYPE_ICMPV6:
            chip_frameType = 11;
            break;
        case MGMT_TYPE_ARP:
            chip_frameType = 3;
            break;
        case MGMT_TYPE_MLD:
            chip_frameType = 4;
            break;
        case MGMT_TYPE_IGMP:
            chip_frameType = 5;
            break;
        case MGMT_TYPE_BGP:
            chip_frameType = 6;
            break;
        case MGMT_TYPE_OSPFV2:
            chip_frameType = 7;
            break;
        case MGMT_TYPE_OSPFV3:
            chip_frameType = 10;
            break;
        case MGMT_TYPE_SNMP:
            chip_frameType = 8;
            break;
        case MGMT_TYPE_SSH:
            chip_frameType = 12;
            break;
        case MGMT_TYPE_FTP:
            chip_frameType = 13;
            break;
        case MGMT_TYPE_TFTP:
            chip_frameType = 14;
            break;
        case MGMT_TYPE_TELNET:
            chip_frameType = 15;
            break;
        case MGMT_TYPE_HTTP:
            chip_frameType = 18;
            break;
        case MGMT_TYPE_HTTPS:
            chip_frameType = 19;
            break;
        default:
            return RT_ERR_RMA_MGMT_TYPE;
    }

    baseAddr = ESW_LAYER34_MANAGEMENT_PROTOCOL_CONTROL0r + chip_frameType / 4;
    TRAP_SEM_LOCK(unit);

    /* get entry from CHIP*/
    if((ret = reg_read(unit, baseAddr, &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 

    TRAP_SEM_UNLOCK(unit); 

    if ((chip_frameType % 4) == 3)
        value = value &BITMASK_8B;
    else if ((chip_frameType % 4) == 2)
        value = (value >> 8)&BITMASK_8B;
    else if ((chip_frameType % 4) == 1)
        value = (value >> 16)&BITMASK_8B;
    else if ((chip_frameType % 4) == 0)
        value = (value >> 24)&BITMASK_8B;

    /*get Action Value*/
    value = (value >> 5) & BITMASK_2B;

    switch (value)
    {
        case 0:
            *pAction = RMA_ACTION_FORWARD;
            break;
        case 1:
            *pAction = RMA_ACTION_TRAP2CPU;
            break;
        case 2:
            *pAction = RMA_ACTION_DROP;
            break;
        case 3:
            *pAction = RMA_ACTION_COPY2CPU;
            break;
        default:
            return RT_ERR_FAILED;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pAction=%d", *pAction);
          
    return RT_ERR_OK;
}   /*end of dal_esw_trap_mgmtFrameAction_get*/


/* Function Name:
 *      dal_esw_trap_mgmtFrameAction_set
 * Description:
 *      Set forwarding action of management frame.
 * Input:
 *      unit      - unit id
 *      frameType - type of management frame
 *      actoin    - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_FWD_ACTION       - invalid forwarding action
 * Note:
 *      Type of management frame is as following:
 *      - MGMT_TYPE_RIP
 *      - MGMT_TYPE_ICMP
 *      - MGMT_TYPE_ICMPV6
 *      - MGMT_TYPE_ARP
 *      - MGMT_TYPE_MLD
 *      - MGMT_TYPE_IGMP
 *      - MGMT_TYPE_BGP
 *      - MGMT_TYPE_OSPFV2
 *      - MGMT_TYPE_OSPFV3
 *      - MGMT_TYPE_SNMP
 *      - MGMT_TYPE_SSH
 *      - MGMT_TYPE_FTP
 *      - MGMT_TYPE_TFTP
 *      - MGMT_TYPE_TELNET
 *      - MGMT_TYPE_HTTP
 *      - MGMT_TYPE_HTTPS
 * 
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 *      - ACTION_DROP
 *      - ACTION_COPY2CPU
 */
int32
dal_esw_trap_mgmtFrameAction_set(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_action_t action)
{
    int32   ret;
    uint32  baseAddr, value, offset;
    uint32  tmpVal;
    uint32  chip_frameType;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, frameType=%d, action=%d", unit, frameType, action);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((frameType > MGMT_TYPE_HTTPS), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((action > ACTION_COPY2CPU), RT_ERR_FWD_ACTION);
    
    switch (frameType)
    {
        case MGMT_TYPE_RIP:
            chip_frameType = 0;
            break;
        case MGMT_TYPE_ICMP:
            chip_frameType = 2;
            break;
        case MGMT_TYPE_ICMPV6:
            chip_frameType = 11;
            break;
        case MGMT_TYPE_ARP:
            chip_frameType = 3;
            break;
        case MGMT_TYPE_MLD:
            chip_frameType = 4;
            break;
        case MGMT_TYPE_IGMP:
            chip_frameType = 5;
            break;
        case MGMT_TYPE_BGP:
            chip_frameType = 6;
            break;
        case MGMT_TYPE_OSPFV2:
            chip_frameType = 7;
            break;
        case MGMT_TYPE_OSPFV3:
            chip_frameType = 10;
            break;
        case MGMT_TYPE_SNMP:
            chip_frameType = 8;
            break;
        case MGMT_TYPE_SSH:
            chip_frameType = 12;
            break;
        case MGMT_TYPE_FTP:
            chip_frameType = 13;
            break;
        case MGMT_TYPE_TFTP:
            chip_frameType = 14;
            break;
        case MGMT_TYPE_TELNET:
            chip_frameType = 15;
            break;
        case MGMT_TYPE_HTTP:
            chip_frameType = 18;
            break;
        case MGMT_TYPE_HTTPS:
            chip_frameType = 19;
            break;
        default:
            return RT_ERR_RMA_MGMT_TYPE;
    }

    switch (action)
    {
        case RMA_ACTION_FORWARD:
            tmpVal = 0;
            break;
        case RMA_ACTION_TRAP2CPU:
            tmpVal = 1;
            break;
        case RMA_ACTION_DROP:
            tmpVal = 2;
            break;
        case RMA_ACTION_COPY2CPU:
            tmpVal = 3;
            break;
        default:
            return RT_ERR_FWD_ACTION;
    }

    baseAddr = ESW_LAYER34_MANAGEMENT_PROTOCOL_CONTROL0r + chip_frameType / 4;
    
    TRAP_SEM_LOCK(unit);

    /* get entry from CHIP*/
    if((ret = reg_read(unit, baseAddr, &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 

    /*Get the bit offset of the entry in the register word*/
    offset = (3 - (chip_frameType % 4)) * 8;

    value &= ~(BITMASK_2B << (offset + 5));
    value |= (tmpVal << (offset + 5));

    /*Set entry to CHIP*/
    if((ret = reg_write(unit, baseAddr, &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }

    TRAP_SEM_UNLOCK(unit); 

    
    return RT_ERR_OK;
}   /*end of dal_esw_trap_mgmtFrameAction_set*/

/* Function Name:
 *      dal_esw_trap_mgmtFramePri_get
 * Description:
 *      Get priority of trapped packet.
 * Input:
 *      unit      - unit id
 *      frameType - type of management frame
 * Output:
 *      pPriority - pointer to priority
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_trap_mgmtFramePri_get(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_pri_t *pPriority)
{
    int32   ret;
    uint32  baseAddr, value;
    uint32  chip_frameType;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, frameType=%d", unit, frameType);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((frameType > MGMT_TYPE_HTTPS), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pPriority), RT_ERR_NULL_POINTER);

    switch (frameType)
    {
        case MGMT_TYPE_RIP:
            chip_frameType = 0;
            break;
        case MGMT_TYPE_ICMP:
            chip_frameType = 2;
            break;
        case MGMT_TYPE_ICMPV6:
            chip_frameType = 11;
            break;
        case MGMT_TYPE_ARP:
            chip_frameType = 3;
            break;
        case MGMT_TYPE_MLD:
            chip_frameType = 4;
            break;
        case MGMT_TYPE_IGMP:
            chip_frameType = 5;
            break;
        case MGMT_TYPE_BGP:
            chip_frameType = 6;
            break;
        case MGMT_TYPE_OSPFV2:
            chip_frameType = 7;
            break;
        case MGMT_TYPE_OSPFV3:
            chip_frameType = 10;
            break;
        case MGMT_TYPE_SNMP:
            chip_frameType = 8;
            break;
        case MGMT_TYPE_SSH:
            chip_frameType = 12;
            break;
        case MGMT_TYPE_FTP:
            chip_frameType = 13;
            break;
        case MGMT_TYPE_TFTP:
            chip_frameType = 14;
            break;
        case MGMT_TYPE_TELNET:
            chip_frameType = 15;
            break;
        case MGMT_TYPE_HTTP:
            chip_frameType = 18;
            break;
        case MGMT_TYPE_HTTPS:
            chip_frameType = 19;
            break;
        default:
            return RT_ERR_RMA_MGMT_TYPE;
    }

    baseAddr = ESW_LAYER34_MANAGEMENT_PROTOCOL_CONTROL0r + chip_frameType / 4;
    TRAP_SEM_LOCK(unit);

    /* get entry from CHIP*/
    if((ret = reg_read(unit, baseAddr, &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 

    TRAP_SEM_UNLOCK(unit); 

    if ((chip_frameType % 4) == 3)
        value = value &BITMASK_8B;
    else if ((chip_frameType % 4) == 2)
        value = (value >> 8)&BITMASK_8B;
    else if ((chip_frameType % 4) == 1)
        value = (value >> 16)&BITMASK_8B;
    else if ((chip_frameType % 4) == 0)
        value = (value >> 24)&BITMASK_8B;

    /*get Priority Value*/
    *pPriority = value & BITMASK_3B;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pPriority=%d", *pPriority);
          
    return RT_ERR_OK;
}   /*end of dal_esw_trap_mgmtFramePri_get*/

/* Function Name:
 *      dal_esw_trap_mgmtFramePri_set
 * Description:
 *      Set priority of trapped packet.
 * Input:
 *      unit      - unit id
 *      frameType - type of management frame
 *      priority  - priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
int32
dal_esw_trap_mgmtFramePri_set(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_pri_t priority)
{
    int32   ret;
    uint32  baseAddr, value, offset;
    uint32  chip_frameType;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, frameType=%d, priority=%d", unit, frameType, priority);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((frameType > MGMT_TYPE_HTTPS), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((priority > RTK_DOT1P_PRIORITY_MAX), RT_ERR_PRIORITY);

    switch (frameType)
    {
        case MGMT_TYPE_RIP:
            chip_frameType = 0;
            break;
        case MGMT_TYPE_ICMP:
            chip_frameType = 2;
            break;
        case MGMT_TYPE_ICMPV6:
            chip_frameType = 11;
            break;
        case MGMT_TYPE_ARP:
            chip_frameType = 3;
            break;
        case MGMT_TYPE_MLD:
            chip_frameType = 4;
            break;
        case MGMT_TYPE_IGMP:
            chip_frameType = 5;
            break;
        case MGMT_TYPE_BGP:
            chip_frameType = 6;
            break;
        case MGMT_TYPE_OSPFV2:
            chip_frameType = 7;
            break;
        case MGMT_TYPE_OSPFV3:
            chip_frameType = 10;
            break;
        case MGMT_TYPE_SNMP:
            chip_frameType = 8;
            break;
        case MGMT_TYPE_SSH:
            chip_frameType = 12;
            break;
        case MGMT_TYPE_FTP:
            chip_frameType = 13;
            break;
        case MGMT_TYPE_TFTP:
            chip_frameType = 14;
            break;
        case MGMT_TYPE_TELNET:
            chip_frameType = 15;
            break;
        case MGMT_TYPE_HTTP:
            chip_frameType = 18;
            break;
        case MGMT_TYPE_HTTPS:
            chip_frameType = 19;
            break;
        default:
            return RT_ERR_RMA_MGMT_TYPE;
    }

    baseAddr = ESW_LAYER34_MANAGEMENT_PROTOCOL_CONTROL0r + chip_frameType / 4;
    
    TRAP_SEM_LOCK(unit);

    /* get entry from CHIP*/
    if((ret = reg_read(unit, baseAddr, &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 

    /*Get the bit offset of the entry in the register word*/
    offset = (3- (chip_frameType % 4)) * 8;

    /*Set Priority*/
    value &= ~(BITMASK_3B << offset);
    value |= priority << offset;

    /*Set entry to CHIP*/
    if((ret = reg_write(unit, baseAddr, &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }

    TRAP_SEM_UNLOCK(unit); 
    
    return RT_ERR_OK;
}   /*end of dal_esw_trap_mgmtFramePri_set*/

/* Function Name:
 *      dal_esw_trap_mgmtFramePriEnable_get
 * Description:
 *      Get priority enable status of trapped packet.
 * Input:
 *      unit      - unit id
 *      frameType - type of management frame
 * Output:
 *      pEnable   - pointer to priority enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_trap_mgmtFramePriEnable_get(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  baseAddr, value, dfPri;
    uint32  chip_frameType;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, frameType=%d", unit, frameType);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((frameType > MGMT_TYPE_HTTPS), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    switch (frameType)
    {
        case MGMT_TYPE_RIP:
            chip_frameType = 0;
            break;
        case MGMT_TYPE_ICMP:
            chip_frameType = 2;
            break;
        case MGMT_TYPE_ICMPV6:
            chip_frameType = 11;
            break;
        case MGMT_TYPE_ARP:
            chip_frameType = 3;
            break;
        case MGMT_TYPE_MLD:
            chip_frameType = 4;
            break;
        case MGMT_TYPE_IGMP:
            chip_frameType = 5;
            break;
        case MGMT_TYPE_BGP:
            chip_frameType = 6;
            break;
        case MGMT_TYPE_OSPFV2:
            chip_frameType = 7;
            break;
        case MGMT_TYPE_OSPFV3:
            chip_frameType = 10;
            break;
        case MGMT_TYPE_SNMP:
            chip_frameType = 8;
            break;
        case MGMT_TYPE_SSH:
            chip_frameType = 12;
            break;
        case MGMT_TYPE_FTP:
            chip_frameType = 13;
            break;
        case MGMT_TYPE_TFTP:
            chip_frameType = 14;
            break;
        case MGMT_TYPE_TELNET:
            chip_frameType = 15;
            break;
        case MGMT_TYPE_HTTP:
            chip_frameType = 18;
            break;
        case MGMT_TYPE_HTTPS:
            chip_frameType = 19;
            break;
        default:
            return RT_ERR_RMA_MGMT_TYPE;
    }

    baseAddr = ESW_LAYER34_MANAGEMENT_PROTOCOL_CONTROL0r + chip_frameType / 4;
    TRAP_SEM_LOCK(unit);

    /* get entry from CHIP*/
    if((ret = reg_read(unit, baseAddr, &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 

    TRAP_SEM_UNLOCK(unit); 

    if ((chip_frameType % 4) == 3)
        value = value &BITMASK_8B;
    else if ((chip_frameType % 4) == 2)
        value = (value >> 8)&BITMASK_8B;
    else if ((chip_frameType % 4) == 1)
        value = (value >> 16)&BITMASK_8B;
    else if ((chip_frameType % 4) == 0)
        value = (value >> 24)&BITMASK_8B;

    /*Get Priority Enable bit*/
    dfPri = (value >> 3) & BITMASK_1B;

    switch (dfPri)
    {
        case 0:
            *pEnable = DISABLED;
            break;
        
        case 1:
            *pEnable = ENABLED;
            break;
        
        default:
            return RT_ERR_FAILED;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pEnable=%d", *pEnable);
          
    return RT_ERR_OK;
}   /*end of dal_esw_trap_mgmtFramePriEnable_get*/

/* Function Name:
 *      dal_esw_trap_mgmtFramePriEnable_set
 * Description:
 *      Set priority enable status of trapped packet.
 * Input:
 *      unit      - unit id
 *      frameType - type of management frame
 *      enable    - priority enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
int32
dal_esw_trap_mgmtFramePriEnable_set(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_enable_t enable)
{
    int32   ret;
    uint32  baseAddr, value, offset, dfPri;
    uint32  chip_frameType;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, frameType=%d, enable=%d", unit, frameType, enable);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((frameType > MGMT_TYPE_HTTPS), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(enable >= RTK_ENABLE_END, RT_ERR_INPUT);
    
    switch (enable)
    {
        case DISABLED:
            dfPri = 0;
            break;
        
        case ENABLED:
            dfPri = 1;
            break;
        
        default:
            return RT_ERR_INPUT;
    }

    switch (frameType)
    {
        case MGMT_TYPE_RIP:
            chip_frameType = 0;
            break;
        case MGMT_TYPE_ICMP:
            chip_frameType = 2;
            break;
        case MGMT_TYPE_ICMPV6:
            chip_frameType = 11;
            break;
        case MGMT_TYPE_ARP:
            chip_frameType = 3;
            break;
        case MGMT_TYPE_MLD:
            chip_frameType = 4;
            break;
        case MGMT_TYPE_IGMP:
            chip_frameType = 5;
            break;
        case MGMT_TYPE_BGP:
            chip_frameType = 6;
            break;
        case MGMT_TYPE_OSPFV2:
            chip_frameType = 7;
            break;
        case MGMT_TYPE_OSPFV3:
            chip_frameType = 10;
            break;
        case MGMT_TYPE_SNMP:
            chip_frameType = 8;
            break;
        case MGMT_TYPE_SSH:
            chip_frameType = 12;
            break;
        case MGMT_TYPE_FTP:
            chip_frameType = 13;
            break;
        case MGMT_TYPE_TFTP:
            chip_frameType = 14;
            break;
        case MGMT_TYPE_TELNET:
            chip_frameType = 15;
            break;
        case MGMT_TYPE_HTTP:
            chip_frameType = 18;
            break;
        case MGMT_TYPE_HTTPS:
            chip_frameType = 19;
            break;
        default:
            return RT_ERR_RMA_MGMT_TYPE;
    }

    baseAddr = ESW_LAYER34_MANAGEMENT_PROTOCOL_CONTROL0r + chip_frameType / 4;
    
    TRAP_SEM_LOCK(unit);

    /* get entry from CHIP*/
    if((ret = reg_read(unit, baseAddr, &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 

    /*Get the bit offset of the entry in the register word*/
    offset = (3- (chip_frameType % 4)) * 8;

    /*Set Enable Priority Assignment*/
    value &= ~(BITMASK_1B << (offset + 3));
    value |= dfPri << (offset + 3);

    /*Set entry to CHIP*/
    if((ret = reg_write(unit, baseAddr, &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }

    TRAP_SEM_UNLOCK(unit); 
    
    return RT_ERR_OK;
}   /*end of dal_esw_trap_mgmtFramePriEnable_set*/

/* Function Name:
 *      dal_esw_trap_mgmtFrameVlanCheck_get
 * Description:
 *      Get enable status of vlan checking on management frame.
 * Input:
 *      unit      - unit id
 *      frameType - type of management frame
 * Output:
 *      pEnable   - pointer to enable status of vlan checking
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_trap_mgmtFrameVlanCheck_get(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  baseAddr, value;
    uint32  chip_frameType;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, frameType=%d", unit, frameType);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((frameType > MGMT_TYPE_HTTPS), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    switch (frameType)
    {
        case MGMT_TYPE_RIP:
            chip_frameType = 0;
            break;
        case MGMT_TYPE_ICMP:
            chip_frameType = 2;
            break;
        case MGMT_TYPE_ICMPV6:
            chip_frameType = 11;
            break;
        case MGMT_TYPE_ARP:
            chip_frameType = 3;
            break;
        case MGMT_TYPE_MLD:
            chip_frameType = 4;
            break;
        case MGMT_TYPE_IGMP:
            chip_frameType = 5;
            break;
        case MGMT_TYPE_BGP:
            chip_frameType = 6;
            break;
        case MGMT_TYPE_OSPFV2:
            chip_frameType = 7;
            break;
        case MGMT_TYPE_OSPFV3:
            chip_frameType = 10;
            break;
        case MGMT_TYPE_SNMP:
            chip_frameType = 8;
            break;
        case MGMT_TYPE_SSH:
            chip_frameType = 12;
            break;
        case MGMT_TYPE_FTP:
            chip_frameType = 13;
            break;
        case MGMT_TYPE_TFTP:
            chip_frameType = 14;
            break;
        case MGMT_TYPE_TELNET:
            chip_frameType = 15;
            break;
        case MGMT_TYPE_HTTP:
            chip_frameType = 18;
            break;
        case MGMT_TYPE_HTTPS:
            chip_frameType = 19;
            break;
        default:
            return RT_ERR_RMA_MGMT_TYPE;
    }

    baseAddr = ESW_LAYER34_MANAGEMENT_PROTOCOL_CONTROL0r + chip_frameType / 4;
    TRAP_SEM_LOCK(unit);

    /* get entry from CHIP*/
    if((ret = reg_read(unit, baseAddr, &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 

    TRAP_SEM_UNLOCK(unit); 

    if ((chip_frameType % 4) == 3)
        value = value &BITMASK_8B;
    else if ((chip_frameType % 4) == 2)
        value = (value >> 8)&BITMASK_8B;
    else if ((chip_frameType % 4) == 1)
        value = (value >> 16)&BITMASK_8B;
    else if ((chip_frameType % 4) == 0)
        value = (value >> 24)&BITMASK_8B;

    /*get Priority Value*/
    *pEnable = (value >> 7) & BITMASK_1B;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pEnable=%d", *pEnable);
          
    return RT_ERR_OK;
}   /*end of dal_esw_trap_mgmtFrameVlanCheck_get*/

/* Function Name:
 *      dal_esw_trap_mgmtFrameVlanCheck_set
 * Description:
 *      Set enable status of vlan checking on management frame.
 * Input:
 *      unit      - unit id
 *      frameType - type of management frame
 *      enable    - enable status of vlan checking
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_trap_mgmtFrameVlanCheck_set(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_enable_t enable)
{
    int32   ret;
    uint32  baseAddr, value, offset;
    uint32  chip_frameType;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, frameType=%d, enable=%d", unit, frameType, enable);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((frameType > MGMT_TYPE_HTTPS), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);

    switch (frameType)
    {
        case MGMT_TYPE_RIP:
            chip_frameType = 0;
            break;
        case MGMT_TYPE_ICMP:
            chip_frameType = 2;
            break;
        case MGMT_TYPE_ICMPV6:
            chip_frameType = 11;
            break;
        case MGMT_TYPE_ARP:
            chip_frameType = 3;
            break;
        case MGMT_TYPE_MLD:
            chip_frameType = 4;
            break;
        case MGMT_TYPE_IGMP:
            chip_frameType = 5;
            break;
        case MGMT_TYPE_BGP:
            chip_frameType = 6;
            break;
        case MGMT_TYPE_OSPFV2:
            chip_frameType = 7;
            break;
        case MGMT_TYPE_OSPFV3:
            chip_frameType = 10;
            break;
        case MGMT_TYPE_SNMP:
            chip_frameType = 8;
            break;
        case MGMT_TYPE_SSH:
            chip_frameType = 12;
            break;
        case MGMT_TYPE_FTP:
            chip_frameType = 13;
            break;
        case MGMT_TYPE_TFTP:
            chip_frameType = 14;
            break;
        case MGMT_TYPE_TELNET:
            chip_frameType = 15;
            break;
        case MGMT_TYPE_HTTP:
            chip_frameType = 18;
            break;
        case MGMT_TYPE_HTTPS:
            chip_frameType = 19;
            break;
        default:
            return RT_ERR_RMA_MGMT_TYPE;
    }

    baseAddr = ESW_LAYER34_MANAGEMENT_PROTOCOL_CONTROL0r + chip_frameType / 4;
    
    TRAP_SEM_LOCK(unit);

    /* get entry from CHIP*/
    if((ret = reg_read(unit, baseAddr, &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 

    /*Get the bit offset of the entry in the register word*/
    offset = (3- (chip_frameType % 4)) * 8;

    /*Set Priority*/
    value &= ~(BITMASK_1B << (offset + 7));
    value |= enable << (offset + 7);

    /*Set entry to CHIP*/
    if((ret = reg_write(unit, baseAddr, &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }

    TRAP_SEM_UNLOCK(unit); 

    
    return RT_ERR_OK;
}   /*end of dal_esw_trap_mgmtFrameVlanCheck_set*/

/* Module Name    : Trap                                      */
/* Sub-module Name: System-wise user defined management frame */

/* Function Name:
 *      dal_esw_trap_userDefineMgmt_get
 * Description:
 *      Get user defined management frame.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined management frame entry
 * Output:
 *      pUserDefine     - pointer to user defined management frame
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_trap_userDefineMgmt_get(uint32 unit, uint32 mgmt_idx, rtk_trap_userDefinedMgmt_t *pUserDefine)
{
    int32   ret;
    uint32 val, tmpVal;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, mgmt_idx=%d", unit, mgmt_idx);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((mgmt_idx >= ESW_UDL34RMA_ENTRY), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pUserDefine), RT_ERR_NULL_POINTER);

    osal_memset(pUserDefine, 0, sizeof(rtk_trap_userDefinedMgmt_t));

    TRAP_SEM_LOCK(unit);

    /*Get entry From CHIP*/
     if (mgmt_idx == 0)
    {
        if((ret = reg_read(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL0_ACTION_CONTROLr, &val)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        } 
        
        reg_field_get(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL0_ACTION_CONTROLr, ESW_ENSRCPORT_USRDEFPRO0f,
                      &tmpVal, &val);
        pUserDefine->srcL4PortCheck = tmpVal;
        reg_field_get(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL0_ACTION_CONTROLr, ESW_ENDSTPORT_USRDEFPRO0f,
                      &tmpVal, &val);
        pUserDefine->dstL4PortCheck = tmpVal;
        reg_field_get(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL0_ACTION_CONTROLr, ESW_SELTCPUDP_USRDEFPRO0f,
                      &tmpVal, &val);
        pUserDefine->layer4Proto = tmpVal;
        reg_field_get(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL0_ACTION_CONTROLr, ESW_ENCMPDIP_USERDEFPRO0f,
                      &tmpVal, &val);
        pUserDefine->dipCheckEnable = tmpVal;
        reg_field_get(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL0_ACTION_CONTROLr, ESW_ENCMPDMAC_USERDEFPRO0f,
                      &tmpVal, &val);
        pUserDefine->dmacCheckEnable = tmpVal;
        

        if((ret = reg_read(unit, ESW_USER_DEFINED_PROTOCOL0_LAYER4_PORT_NUMBER_CONTROL0r, &val)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        } 
        reg_field_get(unit, ESW_USER_DEFINED_PROTOCOL0_LAYER4_PORT_NUMBER_CONTROL0r,
                      ESW_DSTPORTMASK_USRDEFPRO0f, &tmpVal, &val);
        pUserDefine->mask_of_dstL4Port = tmpVal;
        reg_field_get(unit, ESW_USER_DEFINED_PROTOCOL0_LAYER4_PORT_NUMBER_CONTROL0r, ESW_DSTPORT_USRDEFPRO0f,
                      &tmpVal, &val);
        pUserDefine->dstL4Port = tmpVal;

        if((ret = reg_read(unit, ESW_USER_DEFINED_PROTOCOL0_LAYER4_PORT_NUMBER_CONTROL1r, &val)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        } 
        reg_field_get(unit, ESW_USER_DEFINED_PROTOCOL0_LAYER4_PORT_NUMBER_CONTROL1r,
                      ESW_SRCPORTMASK_USRDEFPRO0f, &tmpVal, &val);
        pUserDefine->mask_of_srcL4Port = tmpVal;
        reg_field_get(unit, ESW_USER_DEFINED_PROTOCOL0_LAYER4_PORT_NUMBER_CONTROL1r, ESW_SRCPORT_USRDEFPRO0f,
                      &tmpVal, &val);
        pUserDefine->srcL4Port = tmpVal;
    }
    else
    {
        if((ret = reg_read(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL1_ACTION_CONTROLr, &val)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        } 
        
        reg_field_get(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL1_ACTION_CONTROLr, ESW_ENSRCPORT_USRDEFPRO1f,
                      &tmpVal, &val);
        pUserDefine->srcL4PortCheck = tmpVal;
        reg_field_get(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL1_ACTION_CONTROLr, ESW_ENDSTPORT_USRDEFPRO1f,
                      &tmpVal, &val);
        pUserDefine->dstL4PortCheck = tmpVal;
        reg_field_get(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL1_ACTION_CONTROLr, ESW_SELTCPUDP_USRDEFPRO1f,
                      &tmpVal, &val);
        pUserDefine->layer4Proto = tmpVal;
        reg_field_get(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL1_ACTION_CONTROLr, ESW_ENCMPDIP_USERDEFPRO1f,
                      &tmpVal, &val);
        pUserDefine->dipCheckEnable = tmpVal;
        reg_field_get(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL1_ACTION_CONTROLr, ESW_ENCMPDMAC_USERDEFPRO1f,
                      &tmpVal, &val);
        pUserDefine->dmacCheckEnable = tmpVal;        

        if((ret = reg_read(unit, ESW_USER_DEFINED_PROTOCOL1_LAYER4_PORT_NUMBER_CONTROL0r, &val)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        } 
        reg_field_get(unit, ESW_USER_DEFINED_PROTOCOL1_LAYER4_PORT_NUMBER_CONTROL0r,
                      ESW_DSTPORTMASK_USRDEFPRO1f, &tmpVal, &val);
        pUserDefine->mask_of_dstL4Port = tmpVal;
        reg_field_get(unit, ESW_USER_DEFINED_PROTOCOL1_LAYER4_PORT_NUMBER_CONTROL0r, ESW_DSTPORT_USRDEFPRO1f,
                      &tmpVal, &val);
        pUserDefine->dstL4Port = tmpVal;

        if((ret = reg_read(unit, ESW_USER_DEFINED_PROTOCOL1_LAYER4_PORT_NUMBER_CONTROL1r, &val)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        } 
        reg_field_get(unit, ESW_USER_DEFINED_PROTOCOL1_LAYER4_PORT_NUMBER_CONTROL1r,
                      ESW_SRCPORTMASK_USRDEFPRO1f, &tmpVal, &val);
        pUserDefine->mask_of_srcL4Port = tmpVal;
        reg_field_get(unit, ESW_USER_DEFINED_PROTOCOL1_LAYER4_PORT_NUMBER_CONTROL1r, ESW_SRCPORT_USRDEFPRO1f,
                      &tmpVal, &val);
        pUserDefine->srcL4Port = tmpVal;
    }

    TRAP_SEM_UNLOCK(unit);   

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pUserDefine->dipCheckEnable=%d, pUserDefine->dmacCheckEnable=%d, \
        pUserDefine->dstL4Port=%d, pUserDefine->dstL4PortCheck=%d,\
        pUserDefine->layer4Proto=%d, pUserDefine->mask_of_dstL4Port=%d, pUserDefine->mask_of_srcL4Port=%d,\
        pUserDefine->srcL4Port=%d, pUserDefine->srcL4PortCheck=%d", 
        pUserDefine->dipCheckEnable, pUserDefine->dmacCheckEnable, pUserDefine->dstL4Port,
        pUserDefine->dstL4PortCheck, 
        pUserDefine->layer4Proto, pUserDefine->mask_of_dstL4Port, pUserDefine->mask_of_srcL4Port,
        pUserDefine->srcL4Port, pUserDefine->srcL4PortCheck);
          
    return RT_ERR_OK;
} /* end of dal_esw_trap_userDefineMgmt_get */

/* Function Name:
 *      dal_esw_trap_userDefineMgmt_set
 * Description:
 *      Set user defined management frame.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined management frame entry
 *      pUserDefine     - user defined management frame
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_trap_userDefineMgmt_set(uint32 unit, uint32 mgmt_idx, rtk_trap_userDefinedMgmt_t *pUserDefine)
{
    int32   ret;
    uint32  val, temp;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, mgmt_idx=%d", unit, mgmt_idx);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((mgmt_idx >= ESW_UDL34RMA_ENTRY), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pUserDefine), RT_ERR_NULL_POINTER);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pUserDefine->dipCheckEnable=%d, pUserDefine->dmacCheckEnable=%d, \
        pUserDefine->dstL4Port=%d, pUserDefine->dstL4PortCheck=%d, \
        pUserDefine->layer4Proto=%d, pUserDefine->mask_of_dstL4Port=%d, pUserDefine->mask_of_srcL4Port=%d,\
        pUserDefine->srcL4Port=%d, pUserDefine->srcL4PortCheck=%d", 
        pUserDefine->dipCheckEnable, pUserDefine->dmacCheckEnable, pUserDefine->dstL4Port,
        pUserDefine->dstL4PortCheck, 
        pUserDefine->layer4Proto, pUserDefine->mask_of_dstL4Port, pUserDefine->mask_of_srcL4Port,
        pUserDefine->srcL4Port, pUserDefine->srcL4PortCheck);

    TRAP_SEM_LOCK(unit);

    /*Set entry to CHIP*/
     if (mgmt_idx == 0)
    { 
        val= 0;
        reg_field_set(unit, ESW_USER_DEFINED_PROTOCOL0_LAYER4_PORT_NUMBER_CONTROL0r,
                      ESW_DSTPORTMASK_USRDEFPRO0f, &pUserDefine->mask_of_dstL4Port, &val);
        reg_field_set(unit, ESW_USER_DEFINED_PROTOCOL0_LAYER4_PORT_NUMBER_CONTROL0r, ESW_DSTPORT_USRDEFPRO0f,
                      &pUserDefine->dstL4Port , &val);

        if((ret = reg_write(unit, ESW_USER_DEFINED_PROTOCOL0_LAYER4_PORT_NUMBER_CONTROL0r, &val)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        } 
        
        val= 0;
        reg_field_set(unit, ESW_USER_DEFINED_PROTOCOL0_LAYER4_PORT_NUMBER_CONTROL1r,
                      ESW_SRCPORTMASK_USRDEFPRO0f, &pUserDefine->mask_of_srcL4Port, &val);
        reg_field_set(unit, ESW_USER_DEFINED_PROTOCOL0_LAYER4_PORT_NUMBER_CONTROL1r, ESW_SRCPORT_USRDEFPRO0f,
                      &pUserDefine->srcL4Port, &val);
        if((ret = reg_write(unit, ESW_USER_DEFINED_PROTOCOL0_LAYER4_PORT_NUMBER_CONTROL1r, &val)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        } 

        if((ret = reg_read(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL0_ACTION_CONTROLr, &val)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
        temp = ENABLED;
        reg_field_set(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL0_ACTION_CONTROLr, ESW_ENUSRDEFPRO0f, &temp,
                      &val);        
        reg_field_set(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL0_ACTION_CONTROLr, ESW_ENSRCPORT_USRDEFPRO0f,
                      &pUserDefine->srcL4PortCheck, &val);
        reg_field_set(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL0_ACTION_CONTROLr, ESW_ENDSTPORT_USRDEFPRO0f,
                      &pUserDefine->dstL4PortCheck, &val);
        reg_field_set(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL0_ACTION_CONTROLr, ESW_SELTCPUDP_USRDEFPRO0f,
                      &pUserDefine->layer4Proto, &val);
        reg_field_set(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL0_ACTION_CONTROLr, ESW_ENCMPDIP_USERDEFPRO0f,
                      &pUserDefine->dipCheckEnable, &val);
        reg_field_set(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL0_ACTION_CONTROLr, ESW_ENCMPDMAC_USERDEFPRO0f,
                      &pUserDefine->dmacCheckEnable, &val);
        
        if((ret = reg_write(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL0_ACTION_CONTROLr, &val)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else
    { 
        val= 0;
        reg_field_set(unit, ESW_USER_DEFINED_PROTOCOL1_LAYER4_PORT_NUMBER_CONTROL0r,
                      ESW_DSTPORTMASK_USRDEFPRO1f, &pUserDefine->mask_of_dstL4Port, &val);
        reg_field_set(unit, ESW_USER_DEFINED_PROTOCOL1_LAYER4_PORT_NUMBER_CONTROL0r, ESW_DSTPORT_USRDEFPRO1f,
                      &pUserDefine->dstL4Port , &val);

        if((ret = reg_write(unit, ESW_USER_DEFINED_PROTOCOL1_LAYER4_PORT_NUMBER_CONTROL0r, &val)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        } 
        
        val= 0;
        reg_field_set(unit, ESW_USER_DEFINED_PROTOCOL1_LAYER4_PORT_NUMBER_CONTROL1r,
                      ESW_SRCPORTMASK_USRDEFPRO1f, &pUserDefine->mask_of_srcL4Port, &val);
        reg_field_set(unit, ESW_USER_DEFINED_PROTOCOL1_LAYER4_PORT_NUMBER_CONTROL1r, ESW_SRCPORT_USRDEFPRO1f,
                      &pUserDefine->srcL4Port, &val);
        if((ret = reg_write(unit, ESW_USER_DEFINED_PROTOCOL1_LAYER4_PORT_NUMBER_CONTROL1r, &val)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        } 

        if((ret = reg_read(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL1_ACTION_CONTROLr, &val)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
        
        temp = ENABLED;
        reg_field_set(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL1_ACTION_CONTROLr, ESW_ENUSRDEFPRO1f, &temp,
                      &val);        
        reg_field_set(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL1_ACTION_CONTROLr, ESW_ENSRCPORT_USRDEFPRO1f,
                      &pUserDefine->srcL4PortCheck, &val);
        reg_field_set(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL1_ACTION_CONTROLr, ESW_ENDSTPORT_USRDEFPRO1f,
                      &pUserDefine->dstL4PortCheck, &val);
        reg_field_set(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL1_ACTION_CONTROLr, ESW_SELTCPUDP_USRDEFPRO1f,
                      &pUserDefine->layer4Proto, &val);
        reg_field_set(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL1_ACTION_CONTROLr, ESW_ENCMPDIP_USERDEFPRO1f,
                      &pUserDefine->dipCheckEnable, &val);
        reg_field_set(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL1_ACTION_CONTROLr, ESW_ENCMPDMAC_USERDEFPRO1f,
                      &pUserDefine->dmacCheckEnable, &val);
        
        if((ret = reg_write(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL1_ACTION_CONTROLr, &val)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }

    TRAP_SEM_UNLOCK(unit);   
          
    return RT_ERR_OK;
} /* end of dal_esw_trap_userDefineMgmt_set */

/* Function Name:
 *      dal_esw_trap_userDefineMgmtAction_get
 * Description:
 *      Get forwarding action of user defined management frame.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined management frame entry
 * Output:
 *      pActoin         - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 *      - ACTION_DROP
 */
int32
dal_esw_trap_userDefineMgmtAction_get(uint32 unit, uint32 mgmt_idx, rtk_action_t *pAction)
{
    int32   ret;
    uint32 val, tmpVal;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, mgmt_idx=%d", unit, mgmt_idx);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((mgmt_idx >= ESW_UDL34RMA_ENTRY), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);

    TRAP_SEM_LOCK(unit);

    /*Set entry to CHIP*/
    if (mgmt_idx == 0)
    {
        if((ret = reg_read(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL0_ACTION_CONTROLr, &val)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        } 
        reg_field_get(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL0_ACTION_CONTROLr, ESW_UD0ACTf,
                      &tmpVal, &val); 
    }
    else
    {
        if((ret = reg_read(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL1_ACTION_CONTROLr, &val)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        } 

        reg_field_get(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL1_ACTION_CONTROLr, ESW_UD1ACTf,
                      &tmpVal, &val); 
    }

    TRAP_SEM_UNLOCK(unit); 

    switch (tmpVal)
    {
        case 0:
            *pAction = RMA_ACTION_FORWARD;
            break;
        case 1:
            *pAction = RMA_ACTION_TRAP2CPU;
            break;
        case 2:
            *pAction = RMA_ACTION_DROP;
            break;
        case 3:
            *pAction = RMA_ACTION_COPY2CPU;
            break;
        default:
            return RT_ERR_FAILED;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pAction=%d", *pAction);
          
    return RT_ERR_OK;
} /* end of dal_esw_trap_userDefineMgmtAction_get */

/* Function Name:
 *      rtk_trap_userDefineMgmtAction_set
 * Description:
 *      Set forwarding action of user defined management frame.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined management frame entry
 *      actoin          - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_FWD_ACTION       - invalid forwarding action
 * Note:
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 *      - ACTION_DROP
 */
int32
dal_esw_trap_userDefineMgmtAction_set(uint32 unit, uint32 mgmt_idx, rtk_action_t action)
{
    int32   ret;
    uint32 val;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, mgmt_idx=%d, action=%d", 
                unit, mgmt_idx, action);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((mgmt_idx >= ESW_UDL34RMA_ENTRY), RT_ERR_OUT_OF_RANGE);

    switch (action)
    {
        case RMA_ACTION_FORWARD:
            val = 0;
            break;
        case RMA_ACTION_TRAP2CPU:
            val = 1;
            break;
        case RMA_ACTION_DROP:
            val = 2;
            break;
        case RMA_ACTION_COPY2CPU:
            val = 3;
            break;
        default:
            return RT_ERR_FAILED;
    }

    TRAP_SEM_LOCK(unit);

    /*Set entry to CHIP*/
    if (mgmt_idx == 0)
    {
        if((ret = reg_field_write(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL0_ACTION_CONTROLr, 
                    ESW_UD0ACTf, &val)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        } 
        
    }
    else
    {
        if((ret = reg_field_write(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL1_ACTION_CONTROLr, 
                    ESW_UD1ACTf, &val)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        } 
        
    }

    TRAP_SEM_UNLOCK(unit); 
         
    return RT_ERR_OK;
} /* end of dal_esw_trap_userDefineMgmtAction_set */

/* Function Name:
 *      dal_esw_trap_userDefineMgmtPri_get
 * Description:
 *      Get priority of packets trapped to CPU.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined management frame entry
 * Output:
 *      pPriority       - pointer to priority
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_trap_userDefineMgmtPri_get(uint32 unit, uint32 mgmt_idx, rtk_pri_t *pPriority)
{
    int32   ret;
    uint32  val;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, mgmt_idx=%d", unit, mgmt_idx);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((mgmt_idx >= ESW_UDL34RMA_ENTRY), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pPriority), RT_ERR_NULL_POINTER);

    TRAP_SEM_LOCK(unit);

    /*Get entry From CHIP*/
    if (mgmt_idx == 0)
    {
        if((ret = reg_read(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL0_ACTION_CONTROLr, &val)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        } 
        
        reg_field_get(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL0_ACTION_CONTROLr, ESW_UD0RMAPRIf,
                      pPriority, &val); 
    }
    else
    {
        if((ret = reg_read(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL1_ACTION_CONTROLr, &val)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        } 
        
        reg_field_get(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL1_ACTION_CONTROLr, ESW_UD1RMAPRIf,
                      pPriority, &val); 
    }

    TRAP_SEM_UNLOCK(unit); 

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pPriority=%d", *pPriority);
          
    return RT_ERR_OK;
} /* end of dal_esw_trap_userDefineMgmtPri_get */

/* Function Name:
 *      dal_esw_trap_userDefineMgmtPri_set
 * Description:
 *      Set priority of packets trapped to CPU.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined management frame entry
 *      priority        - priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
int32
dal_esw_trap_userDefineMgmtPri_set(uint32 unit, uint32 mgmt_idx, rtk_pri_t priority)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, mgmt_idx=%d, priority=%d", 
                unit, mgmt_idx, priority);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((mgmt_idx >= ESW_UDL34RMA_ENTRY), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((priority > RTK_DOT1P_PRIORITY_MAX), RT_ERR_OUT_OF_RANGE);


    TRAP_SEM_LOCK(unit);

    /*Set entry to CHIP*/
    if (mgmt_idx == 0)
    {
        if((ret = reg_field_write(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL0_ACTION_CONTROLr, 
                    ESW_UD0RMAPRIf, &priority)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        } 
    }
    else
    {
        if((ret = reg_field_write(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL1_ACTION_CONTROLr, 
                    ESW_UD1RMAPRIf, &priority)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        } 
    }

    TRAP_SEM_UNLOCK(unit); 
         
    return RT_ERR_OK;
} /* end of dal_esw_trap_userDefineMgmtPri_set */

/* Function Name:
 *      dal_esw_trap_userDefineMgmtPriEnable_get
 * Description:
 *      Get priority enable status of packets trapped to CPU.
 * Input:
 *      unit           - unit id
 *      userDefine_idx - index of user defined management frame entry
 * Output:
 *      pEnable        - pointer to priority enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_trap_userDefineMgmtPriEnable_get(uint32 unit, uint32 mgmt_idx, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  val, tmpVal;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, mgmt_idx=%d", unit, mgmt_idx);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((mgmt_idx >= ESW_UDL34RMA_ENTRY), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    TRAP_SEM_LOCK(unit);

    /*Get entry From CHIP*/
    if (mgmt_idx == 0)
    {
        if((ret = reg_read(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL0_ACTION_CONTROLr, &val)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        } 
        
        reg_field_get(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL0_ACTION_CONTROLr, ESW_UD0DFRMAPRIf, &tmpVal,
                      &val);
    }
    else
    {
        if((ret = reg_read(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL1_ACTION_CONTROLr, &val)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        } 
        
        reg_field_get(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL1_ACTION_CONTROLr, ESW_UD1DFRMAPRIf, &tmpVal,
                      &val);
    }

    TRAP_SEM_UNLOCK(unit); 

    switch (tmpVal)
    {
        case 0:
            *pEnable = DISABLED;
            break;
        
        case 1:
            *pEnable = ENABLED;
            break;
        
        default:
            return RT_ERR_FAILED;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pEnable=%d", *pEnable);
          
    return RT_ERR_OK;
} /* end of dal_esw_trap_userDefineMgmtPriEnable_get */

/* Function Name:
 *      dal_esw_trap_userDefineMgmtPriEnable_set
 * Description:
 *      Set priority enable status of packets trapped to CPU.
 * Input:
 *      unit           - unit id
 *      userDefine_idx - index of user defined management frame entry
 *      enable         - priority enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
int32
dal_esw_trap_userDefineMgmtPriEnable_set(uint32 unit, uint32 mgmt_idx, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, mgmt_idx=%d, enable=%d", 
                unit, mgmt_idx, enable);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((mgmt_idx >= ESW_UDL34RMA_ENTRY), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(enable >= RTK_ENABLE_END, RT_ERR_INPUT);
    
    switch (enable)
    {
        case DISABLED:
            value = 0;
            break;
        
        case ENABLED:
            value = 1;
            break;
        
        default:
            return RT_ERR_INPUT;
    }

    TRAP_SEM_LOCK(unit);

    /*Set entry to CHIP*/
    if (mgmt_idx == 0)
    {
        if((ret = reg_field_write(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL0_ACTION_CONTROLr, 
                    ESW_UD0DFRMAPRIf, &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        } 
    }
    else
    {
        if((ret = reg_field_write(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL1_ACTION_CONTROLr, 
                    ESW_UD1DFRMAPRIf, &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        } 
    }

    TRAP_SEM_UNLOCK(unit); 
         
    return RT_ERR_OK;
} /* end of dal_esw_trap_userDefineMgmtPriEnable_set */

/* Function Name:
 *      dal_esw_trap_userDefineMgmtVlanCheck_get
 * Description:
 *      Get enable status of vlan checking on specified user defined management frame.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined management frame entry
 *      enable          - enable status of vlan checking
 * Output:
 *      pEnable         - pointer to enable status of vlan checking
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_trap_userDefineMgmtVlanCheck_get(uint32 unit, uint32 mgmt_idx, rtk_enable_t *pEnable)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, mgmt_idx=%d", 
                unit, mgmt_idx);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((mgmt_idx >= ESW_UDL34RMA_ENTRY), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    TRAP_SEM_LOCK(unit);

    /*Get entry From CHIP*/
    if (mgmt_idx == 0)
    {
        if((ret = reg_field_read(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL0_ACTION_CONTROLr, 
                    ESW_UD0VLANCAREf, pEnable)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        } 
        
    }
    else
    {
        if((ret = reg_field_read(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL1_ACTION_CONTROLr, 
                    ESW_UD1VLANCAREf, pEnable)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        } 
        
    }

    TRAP_SEM_UNLOCK(unit); 

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pEnable=%d", *pEnable);
         
    return RT_ERR_OK;
} /* end of dal_esw_trap_userDefineMgmtPri_disable */

/* Function Name:
 *      dal_esw_trap_userDefineMgmtVlanCheck_set
 * Description:
 *      Set enable status of vlan checking on specified user defined management frame.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined management frame entry
 *      enable          - enable status of vlan checking
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_trap_userDefineMgmtVlanCheck_set(uint32 unit, uint32 mgmt_idx, rtk_enable_t enable)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, mgmt_idx=%d, enable=%d", 
                unit, mgmt_idx, enable);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((mgmt_idx >= ESW_UDL34RMA_ENTRY), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);

    TRAP_SEM_LOCK(unit);

    /*Set entry to CHIP*/
    if (mgmt_idx == 0)
    {
        if((ret = reg_field_write(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL0_ACTION_CONTROLr, 
                    ESW_UD0VLANCAREf, &enable)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        } 
        
    }
    else
    {
        if((ret = reg_field_write(unit, ESW_USER_DEFINED_LAYER34_PROTOCOL1_ACTION_CONTROLr, 
                    ESW_UD1VLANCAREf, &enable)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        } 
        
    }

    TRAP_SEM_UNLOCK(unit); 
         
    return RT_ERR_OK;
} /* end of dal_esw_trap_userDefineMgmtPri_disable */

/* Module Name    : Trap                                   */
/* Sub-module Name: Per port user defined management frame */

/* Function Name:
 *      dal_esw_trap_portMgmtFrameAction_get
 * Description:
 *      Get forwarding action of management frame on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      frameType - type of management frame
 * Output:
 *      pActoin   - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Type of management frame is as following:
 *      - MGMT_TYPE_DHCPV6
 *      - MGMT_TYPE_DHCP
 *      - MGMT_TYPE_DOT1X
 *      - MGMT_TYPE_BPDU
 * 
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 *      - ACTION_DROP
 *      - ACTION_COPY2CPU
 */
int32
dal_esw_trap_portMgmtFrameAction_get(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_action_t *pAction)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d, frameType=%d", 
                unit, port, frameType);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((frameType < MGMT_TYPE_DHCPV6), RT_ERR_RMA_MGMT_TYPE);
    RT_PARAM_CHK((frameType > MGMT_TYPE_BPDU), RT_ERR_RMA_MGMT_TYPE);
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);

    TRAP_SEM_LOCK(unit);

    /*Get entry From CHIP*/
    if(frameType == MGMT_TYPE_BPDU)
    {
        if((ret = reg_array_field_read(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_BPDUACTf, &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if(frameType == MGMT_TYPE_DOT1X)
    {
        if((ret = reg_array_field_read(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_IEEE1XACTf, &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if(frameType == MGMT_TYPE_DHCP)
    {
        if((ret = reg_array_field_read(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_DHCPACTf, &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else  /*MGMT_TYPE_DHCPV6*/
    {
        if((ret = reg_array_field_read(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_DHCPV6ACTf, &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }

    TRAP_SEM_UNLOCK(unit); 

    switch (value)
    {
        case 0:
            *pAction = RMA_ACTION_FORWARD;
            break;
        case 1:
            *pAction = RMA_ACTION_TRAP2CPU;
            break;
        case 2:
            *pAction = RMA_ACTION_DROP;
            break;
        case 3:
            *pAction = RMA_ACTION_COPY2CPU;
            break;
        default:
            return RT_ERR_FAILED;
    }   

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pAction=%d", *pAction);
         
    return RT_ERR_OK;
} /* end of dal_esw_trap_portMgmtFrameAction_get */

/* Function Name:
 *      dal_esw_trap_portMgmtFrameAction_set
 * Description:
 *      Set forwarding action of management frame on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      frameType - type of management frame
 *      actoin    - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_FWD_ACTION       - invalid forwarding action
 * Note:
 *      Type of management frame is as following:
 *      - MGMT_TYPE_DHCPV6
 *      - MGMT_TYPE_DHCP
 *      - MGMT_TYPE_DOT1X
 *      - MGMT_TYPE_BPDU
 * 
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 *      - ACTION_DROP
 *      - ACTION_COPY2CPU
 */
int32
dal_esw_trap_portMgmtFrameAction_set(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_action_t action)
{
    int32   ret;
    uint32 value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d, frameType=%d, action=%d", 
                unit, port, frameType, action);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((frameType < MGMT_TYPE_DHCPV6), RT_ERR_RMA_MGMT_TYPE);
    RT_PARAM_CHK((frameType > MGMT_TYPE_BPDU), RT_ERR_RMA_MGMT_TYPE);
    RT_PARAM_CHK((action > ACTION_COPY2CPU), RT_ERR_FWD_ACTION);

    switch (action)
    {
        case RMA_ACTION_FORWARD:
            value = 0;
            break;
        case RMA_ACTION_TRAP2CPU:
            value = 1;
            break;
        case RMA_ACTION_DROP:
            value = 2;
            break;
        case RMA_ACTION_COPY2CPU:
            value = 3;
            break;
        default:
            return RT_ERR_FAILED;
    }

    TRAP_SEM_LOCK(unit);

    /*Get entry From CHIP*/
    if(frameType == MGMT_TYPE_BPDU)
    {
        if((ret = reg_array_field_write(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_BPDUACTf, &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if(frameType == MGMT_TYPE_DOT1X)
    {
        if((ret = reg_array_field_write(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_IEEE1XACTf, &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if(frameType == MGMT_TYPE_DHCP)
    {
        if((ret = reg_array_field_write(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_DHCPACTf, &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else  /*MGMT_TYPE_DHCPV6*/
    {
        if((ret = reg_array_field_write(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_DHCPV6ACTf, &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }

    TRAP_SEM_UNLOCK(unit);      
         
    return RT_ERR_OK;
} /* end of dal_esw_trap_portMgmtFrameAction_set */

/* Function Name:
 *      dal_esw_trap_portMgmtFramePri_get
 * Description:
 *      Get priority of trapped packet on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      frameType - type of management frame
 * Output:
 *      pPriority - pointer to priority
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Type of management frame is as following:
 *      - MGMT_TYPE_DHCPV6
 *      - MGMT_TYPE_DHCP
 *      - MGMT_TYPE_DOT1X
 *      - MGMT_TYPE_BPDU
 */
int32
dal_esw_trap_portMgmtFramePri_get(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_pri_t *pPriority)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d, frameType=%d", 
                unit, port, frameType);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((frameType < MGMT_TYPE_DHCPV6), RT_ERR_RMA_MGMT_TYPE);
    RT_PARAM_CHK((frameType > MGMT_TYPE_BPDU), RT_ERR_RMA_MGMT_TYPE);
    RT_PARAM_CHK((NULL == pPriority), RT_ERR_NULL_POINTER);

    TRAP_SEM_LOCK(unit);

    /*Get entry From CHIP*/
    if(frameType == MGMT_TYPE_BPDU)
    {
        if((ret = reg_array_field_read(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_BPDUPRIf, pPriority)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if(frameType == MGMT_TYPE_DOT1X)
    {
        if((ret = reg_array_field_read(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_IEEE1XPRIf, pPriority)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if(frameType == MGMT_TYPE_DHCP)
    {
        if((ret = reg_array_field_read(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_DHCPPRIf, pPriority)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else  /*MGMT_TYPE_DHCPV6*/
    {
        if((ret = reg_array_field_read(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_DHCPV6PRIf, pPriority)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }

    TRAP_SEM_UNLOCK(unit); 

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pPriority=%d", *pPriority);
         
    return RT_ERR_OK;
} /* end of dal_esw_trap_portMgmtFramePri_get */

/* Function Name:
 *      dal_esw_trap_portMgmtFramePri_set
 * Description:
 *      Set priority of trapped packet on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      frameType - type of management frame
 *      priority  - priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      Type of management frame is as following:
 *      - MGMT_TYPE_DHCPV6
 *      - MGMT_TYPE_DHCP
 *      - MGMT_TYPE_DOT1X
 *      - MGMT_TYPE_BPDU
 */
int32
dal_esw_trap_portMgmtFramePri_set(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_pri_t priority)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d, frameType=%d, priority=%d", 
                unit, port, frameType, priority);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((frameType < MGMT_TYPE_DHCPV6), RT_ERR_RMA_MGMT_TYPE);
    RT_PARAM_CHK((frameType > MGMT_TYPE_BPDU), RT_ERR_RMA_MGMT_TYPE);
    RT_PARAM_CHK((priority > RTK_DOT1P_PRIORITY_MAX), RT_ERR_PRIORITY);

    TRAP_SEM_LOCK(unit);

    /*Set entry to CHIP*/
    if(frameType == MGMT_TYPE_BPDU)
    { 
        /*Set Priority*/
        if((ret = reg_array_field_write(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_BPDUPRIf, &priority)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if(frameType == MGMT_TYPE_DOT1X)
    { 
        /*Set Priority*/
        if((ret = reg_array_field_write(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_IEEE1XPRIf, &priority)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if(frameType == MGMT_TYPE_DHCP)
    { 
        /*Set Priority*/
        if((ret = reg_array_field_write(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_DHCPPRIf, &priority)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else  /*MGMT_TYPE_DHCPV6*/
    { 
        /*Set Priority*/
        if((ret = reg_array_field_write(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_DHCPV6PRIf, &priority)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }

    TRAP_SEM_UNLOCK(unit); 
    
    return RT_ERR_OK;
} /* end of dal_esw_trap_portMgmtFramePri_set */

/* Function Name:
 *      dal_esw_trap_portMgmtFramePriEnable_get
 * Description:
 *      Get priority enable status of trapped packet on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      frameType - type of management frame
 * Output:
 *      pEnable   - pointer to priority enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Type of management frame is as following:
 *      - MGMT_TYPE_DHCPV6
 *      - MGMT_TYPE_DHCP
 *      - MGMT_TYPE_DOT1X
 *      - MGMT_TYPE_BPDU
 */
int32
dal_esw_trap_portMgmtFramePriEnable_get(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d, frameType=%d", 
                unit, port, frameType);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((frameType < MGMT_TYPE_DHCPV6), RT_ERR_RMA_MGMT_TYPE);
    RT_PARAM_CHK((frameType > MGMT_TYPE_BPDU), RT_ERR_RMA_MGMT_TYPE);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    TRAP_SEM_LOCK(unit);

    /*Get entry From CHIP*/
    if(frameType == MGMT_TYPE_BPDU)
    {
        if((ret = reg_array_field_read(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_BPDUDFPRIf, &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if(frameType == MGMT_TYPE_DOT1X)
    {
        if((ret = reg_array_field_read(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_IEEE1XDFPRIf, &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if(frameType == MGMT_TYPE_DHCP)
    {
        if((ret = reg_array_field_read(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_DHCPDFPRIf, &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else  /*MGMT_TYPE_DHCPV6*/
    {
        if((ret = reg_array_field_read(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_DHCPV6DFPRIf, &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }

    TRAP_SEM_UNLOCK(unit); 

    switch (value)
    {
        case 0:
            *pEnable = DISABLED;
            break;
        
        case 1:
            *pEnable = ENABLED;
            break;
        
        default:
            return RT_ERR_FAILED;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pEnable=%d", *pEnable);
         
    return RT_ERR_OK;
} /* end of dal_esw_trap_portMgmtFramePriEnable_get */

/* Function Name:
 *      dal_esw_trap_portMgmtFramePriEnable_set
 * Description:
 *      Set priority enable status of trapped packet on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      frameType - type of management frame
 *      enable    - priority enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      Type of management frame is as following:
 *      - MGMT_TYPE_DHCPV6
 *      - MGMT_TYPE_DHCP
 *      - MGMT_TYPE_DOT1X
 *      - MGMT_TYPE_BPDU
 */
int32
dal_esw_trap_portMgmtFramePriEnable_set(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d, frameType=%d, enable=%d", 
                unit, port, frameType, enable);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((frameType < MGMT_TYPE_DHCPV6), RT_ERR_RMA_MGMT_TYPE);
    RT_PARAM_CHK((frameType > MGMT_TYPE_BPDU), RT_ERR_RMA_MGMT_TYPE);
    RT_PARAM_CHK(enable >= RTK_ENABLE_END, RT_ERR_INPUT);
    
    switch (enable)
    {
        case DISABLED:
            value = 0;
            break;
        
        case ENABLED:
            value = 1;
            break;
        
        default:
            return RT_ERR_INPUT;
    }

    TRAP_SEM_LOCK(unit);

    /*Set entry to CHIP*/
    if(frameType == MGMT_TYPE_BPDU)
    { 
        /*Enable Priority*/
        if((ret = reg_array_field_write(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_BPDUDFPRIf, &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if(frameType == MGMT_TYPE_DOT1X)
    { 
        /*Enable Priority*/
        if((ret = reg_array_field_write(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_IEEE1XDFPRIf, &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if(frameType == MGMT_TYPE_DHCP)
    { 
        /*Enable Priority*/
        if((ret = reg_array_field_write(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_DHCPDFPRIf, &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else  /*MGMT_TYPE_DHCPV6*/
    { 
        /*Enable Priority*/
        if((ret = reg_array_field_write(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_DHCPV6DFPRIf, &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }

    TRAP_SEM_UNLOCK(unit); 
    
    return RT_ERR_OK;
} /* end of dal_esw_trap_portMgmtFramePriEnable_set */

/* Function Name:
 *      dal_esw_trap_portMgmtFrameVlanCheck_get
 * Description:
 *      Get enable status of vlan checking on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      frameType - type of management frame
 * Output:
 *      pEnable   - pointer to enable status of vlan checking
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Type of management frame is as following:
 *      - MGMT_TYPE_DHCPV6
 *      - MGMT_TYPE_DHCP
 *      - MGMT_TYPE_DOT1X
 *      - MGMT_TYPE_BPDU
 */
int32
dal_esw_trap_portMgmtFrameVlanCheck_get(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_enable_t *pEnable)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d, frameType=%d", 
                unit, port, frameType);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((frameType < MGMT_TYPE_DHCPV6), RT_ERR_RMA_MGMT_TYPE);
    RT_PARAM_CHK((frameType > MGMT_TYPE_BPDU), RT_ERR_RMA_MGMT_TYPE);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    TRAP_SEM_LOCK(unit);

    /*Get entry From CHIP*/
    if(frameType == MGMT_TYPE_BPDU)
    {
        if((ret = reg_array_field_read(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_BPDUVLANCAREf, pEnable)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if(frameType == MGMT_TYPE_DOT1X)
    {
        if((ret = reg_array_field_read(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_IEEE1XVLANCAREf, pEnable)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if(frameType == MGMT_TYPE_DHCP)
    {
        if((ret = reg_array_field_read(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_DHCPVLANCAREf, pEnable)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else  /*MGMT_TYPE_DHCPV6*/
    {
        if((ret = reg_array_field_read(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_DHCPV6VLANCAREf, pEnable)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }

    TRAP_SEM_UNLOCK(unit); 

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pEnable=%d", *pEnable);
         
    return RT_ERR_OK;
} /* end of dal_esw_trap_portMgmtFrameVlanCheck_get */

/* Function Name:
 *      dal_esw_trap_portMgmtFrameVlanCheck_set
 * Description:
 *      Set enable status of vlan checking on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      frameType - type of management frame
 *      enable    - enable status of vlan checking
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Type of management frame is as following:
 *      - MGMT_TYPE_DHCPV6
 *      - MGMT_TYPE_DHCP
 *      - MGMT_TYPE_DOT1X
 *      - MGMT_TYPE_BPDU
 */
int32
dal_esw_trap_portMgmtFrameVlanCheck_set(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_enable_t enable)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d, frameType=%d, enable=%d", 
                unit, port, frameType, enable);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((frameType < MGMT_TYPE_DHCPV6), RT_ERR_RMA_MGMT_TYPE);
    RT_PARAM_CHK((frameType > MGMT_TYPE_BPDU), RT_ERR_RMA_MGMT_TYPE);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_NULL_POINTER);

    TRAP_SEM_LOCK(unit);

    /*Get entry From CHIP*/
    if(frameType == MGMT_TYPE_BPDU)
    {
        if((ret = reg_array_field_write(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_BPDUVLANCAREf, &enable)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if(frameType == MGMT_TYPE_DOT1X)
    {
        if((ret = reg_array_field_write(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_IEEE1XVLANCAREf, &enable)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if(frameType == MGMT_TYPE_DHCP)
    {
        if((ret = reg_array_field_write(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_DHCPVLANCAREf, &enable)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else  /*MGMT_TYPE_DHCPV6*/
    {
        if((ret = reg_array_field_write(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_DHCPV6VLANCAREf, &enable)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }

    TRAP_SEM_UNLOCK(unit); 
         
    return RT_ERR_OK;
} /* end of dal_esw_trap_portMgmtFrameVlanCheck_set */

/* Function Name:
 *      dal_esw_trap_portMgmtFrameCrossVlan_get
 * Description:
 *      Get enable status of cross vlan forwarding on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      frameType - type of management frame
 * Output:
 *      pEnable   - pointer to enable status of cross vlan forwarding
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Type of management frame is as following:
 *      - MGMT_TYPE_DHCPV6
 *      - MGMT_TYPE_DHCP
 *      - MGMT_TYPE_DOT1X
 *      - MGMT_TYPE_BPDU
 */
int32
dal_esw_trap_portMgmtFrameCrossVlan_get(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_enable_t *pEnable)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d, frameType=%d", 
                unit, port, frameType);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((frameType < MGMT_TYPE_DHCPV6), RT_ERR_RMA_MGMT_TYPE);
    RT_PARAM_CHK((frameType > MGMT_TYPE_BPDU), RT_ERR_RMA_MGMT_TYPE);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    TRAP_SEM_LOCK(unit);

    /*Get entry From CHIP*/
    if(frameType == MGMT_TYPE_BPDU)
    {
        if((ret = reg_array_field_read(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_BPDUCRSVLANf, pEnable)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if(frameType == MGMT_TYPE_DOT1X)
    {
        if((ret = reg_array_field_read(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_IEEE1XCRSVLANf, pEnable)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if(frameType == MGMT_TYPE_DHCP)
    {
        if((ret = reg_array_field_read(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_DHCPCRSVLANf, pEnable)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else  /*MGMT_TYPE_DHCPV6*/
    {
        if((ret = reg_array_field_read(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_DHCPV6CRSVLANf, pEnable)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }

    TRAP_SEM_UNLOCK(unit); 

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pEnable=%d", *pEnable);
         
    return RT_ERR_OK;
} /* end of dal_esw_trap_portMgmtFrameVlanCheck_get */


/* Function Name:
 *      dal_esw_trap_portMgmtFrameCrossVlan_set
 * Description:
 *      Set enable status of cross vlan forwarding on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      frameType - type of management frame
 *      enable    - enable status of cross vlan forwarding
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Type of management frame is as following:
 *      - MGMT_TYPE_DHCPV6
 *      - MGMT_TYPE_DHCP
 *      - MGMT_TYPE_DOT1X
 *      - MGMT_TYPE_BPDU
 */
int32
dal_esw_trap_portMgmtFrameCrossVlan_set(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_enable_t enable)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d, frameType=%d, enable=%d", 
                unit, port, frameType, enable);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((frameType < MGMT_TYPE_DHCPV6), RT_ERR_RMA_MGMT_TYPE);
    RT_PARAM_CHK((frameType > MGMT_TYPE_BPDU), RT_ERR_RMA_MGMT_TYPE);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_NULL_POINTER);

    TRAP_SEM_LOCK(unit);

    /*Get entry From CHIP*/
    if(frameType == MGMT_TYPE_BPDU)
    {
        if((ret = reg_array_field_write(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_BPDUCRSVLANf, &enable)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if(frameType == MGMT_TYPE_DOT1X)
    {
        if((ret = reg_array_field_write(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_IEEE1XCRSVLANf, &enable)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if(frameType == MGMT_TYPE_DHCP)
    {
        if((ret = reg_array_field_write(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_DHCPCRSVLANf, &enable)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else  /*MGMT_TYPE_DHCPV6*/
    {
        if((ret = reg_array_field_write(unit, ESW_PORT_RMA_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_DHCPV6CRSVLANf, &enable)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }

    TRAP_SEM_UNLOCK(unit); 
         
    return RT_ERR_OK;
} /* end of dal_esw_trap_portMgmtFrameCrossVlan_set */

/* Module Name    : Trap                               */
/* Sub-module Name: Packet with special flag or option */

/* Function Name:
 *      dal_esw_trap_ipWithOptionHeaderAction_get
 * Description:
 *      Get forwarding action of IP packet with IP option header.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      ipFamily - IP family(IPv4 or IPv6)
 * Output:
 *      pActoin  - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 */
int32
dal_esw_trap_ipWithOptionHeaderAction_get(
    uint32              unit, 
    rtk_port_t          port, 
    rtk_ip_family_t     ipFamily, 
    rtk_action_t        *pAction)
{
    int32   ret;
    uint32 value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d, ipFamily=%d", 
                unit, port, ipFamily);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((ipFamily >= IP_FAMILY_END), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);

    TRAP_SEM_LOCK(unit);

    /*Get entry From CHIP*/
    if(ipFamily == IPV4_FAMILY)
    {
        if((ret = reg_array_field_read(unit, ESW_PORT_PACKET_PARSER_OPTION_HEADER_CONTROLr,
                        port, REG_ARRAY_INDEX_NONE, ESW_IPV4OPTRAPf, &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if(ipFamily == IPV6_FAMILY)
    {
        if((ret = reg_array_field_read(unit, ESW_PORT_PACKET_PARSER_OPTION_HEADER_CONTROLr,
                        port, REG_ARRAY_INDEX_NONE, ESW_IPV6OPTRAPf, &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }

    TRAP_SEM_UNLOCK(unit); 

    if(value)
        *pAction = ACTION_TRAP2CPU;
    else
        *pAction = ACTION_FORWARD;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pAction=%d", *pAction);
         
    return RT_ERR_OK;
} /* end of dal_esw_trap_ipWithOptionHeaderAction_get */

/* Function Name:
 *      dal_esw_trap_ipWithOptionHeaderAction_set
 * Description:
 *      Set forwarding action of IP packet with IP option header.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      ipFamily - IP family(IPv4 or IPv6)
 *      actoin   - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_FWD_ACTION       - invalid forwarding action
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 */
int32
dal_esw_trap_ipWithOptionHeaderAction_set(
    uint32              unit, 
    rtk_port_t          port, 
    rtk_ip_family_t     ipFamily, 
    rtk_action_t        action)
{
    int32   ret;
    uint32 value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d, ipFamily=%d, action=%d", 
                unit, port, ipFamily, action);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((ipFamily >= IP_FAMILY_END), RT_ERR_INPUT);

     if(action == ACTION_TRAP2CPU)
        value = 1;
    else if(action == ACTION_FORWARD)
        value = 0;
    else
        return RT_ERR_FWD_ACTION;

    TRAP_SEM_LOCK(unit);

    /*Get entry From CHIP*/
    if(ipFamily == IPV4_FAMILY)
    {
        if((ret = reg_array_field_write(unit, ESW_PORT_PACKET_PARSER_OPTION_HEADER_CONTROLr,
                        port, REG_ARRAY_INDEX_NONE, ESW_IPV4OPTRAPf, &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }
    else if(ipFamily == IPV6_FAMILY)
    {
        if((ret = reg_array_field_write(unit, ESW_PORT_PACKET_PARSER_OPTION_HEADER_CONTROLr,
                        port, REG_ARRAY_INDEX_NONE, ESW_IPV6OPTRAPf, &value)) != RT_ERR_OK)
        {
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }

    TRAP_SEM_UNLOCK(unit);    
         
    return RT_ERR_OK;
} /* end of dal_esw_trap_ipWithOptionHeaderAction_set */    

/* Function Name:
 *      dal_esw_trap_ipWithOptionHeaderPri_get
 * Description:
 *      Get priority of packets trapped to CPU.
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pPriority - pointer to priority
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_trap_ipWithOptionHeaderPri_get(uint32 unit, rtk_port_t port, rtk_pri_t *pPriority)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d", 
                unit, port);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pPriority), RT_ERR_NULL_POINTER);

    TRAP_SEM_LOCK(unit);

    /*Get entry From CHIP*/
    if((ret = reg_array_field_read(unit, ESW_PORT_PACKET_PARSER_OPTION_HEADER_CONTROLr,
                    port, REG_ARRAY_INDEX_NONE, ESW_OPPRIf, pPriority)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }

    TRAP_SEM_UNLOCK(unit); 

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pPriority=%d", *pPriority);
         
    return RT_ERR_OK;
} /* end of dal_esw_trap_ipWithOptionHeaderPri_get */

/* Function Name:
 *      dal_esw_trap_ipWithOptionHeaderPri_set
 * Description:
 *      Set priority of packets trapped to CPU.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      priority - priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
int32
dal_esw_trap_ipWithOptionHeaderPri_set(uint32 unit, rtk_port_t port, rtk_pri_t priority)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d, priority=%d", 
                unit, port, priority);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((priority > RTK_DOT1P_PRIORITY_MAX), RT_ERR_PRIORITY);

    TRAP_SEM_LOCK(unit);

    /*Set entry to CHIP*/
    if((ret = reg_array_field_write(unit, ESW_PORT_PACKET_PARSER_OPTION_HEADER_CONTROLr,
                    port, REG_ARRAY_INDEX_NONE, ESW_OPPRIf, &priority)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }
    
    TRAP_SEM_UNLOCK(unit); 
         
    return RT_ERR_OK;
} /* end of dal_esw_trap_ipWithOptionHeaderPri_set */

/* Function Name:
 *      dal_esw_trap_ipWithOptionHeaderPriEnable_get
 * Description:
 *      Get priority enable status of packets trapped to CPU.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to priority enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_trap_ipWithOptionHeaderPriEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d", 
                unit, port);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    TRAP_SEM_LOCK(unit);

    /*Get entry From CHIP*/
    if((ret = reg_array_field_read(unit, ESW_PORT_PACKET_PARSER_OPTION_HEADER_CONTROLr,
                    port, REG_ARRAY_INDEX_NONE, ESW_DFROPPRIf, &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }

    TRAP_SEM_UNLOCK(unit); 

    switch (value)
    {
        case 0:
            *pEnable = DISABLED;
            break;
        
        case 1:
            *pEnable = ENABLED;
            break;
        
        default:
            return RT_ERR_FAILED;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pEnable=%d", *pEnable);
         
    return RT_ERR_OK;
} /* end of dal_esw_trap_ipWithOptionHeaderPriEnable_get */

/* Function Name:
 *      dal_esw_trap_ipWithOptionHeaderPriEnable_set
 * Description:
 *      Set priority enable status of packets trapped to CPU.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - priority enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
int32
dal_esw_trap_ipWithOptionHeaderPriEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d, enable=%d", 
                unit, port, enable);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(enable >= RTK_ENABLE_END, RT_ERR_INPUT);

    switch (enable)
    {
        case DISABLED:
            value = 0;
            break;
        
        case ENABLED:
            value = 1;
            break;
        
        default:
            return RT_ERR_INPUT;
    }

    TRAP_SEM_LOCK(unit);

    /*Set entry to CHIP*/
    if((ret = reg_array_field_write(unit, ESW_PORT_PACKET_PARSER_OPTION_HEADER_CONTROLr,
                    port, REG_ARRAY_INDEX_NONE, ESW_DFROPPRIf, &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }

    TRAP_SEM_UNLOCK(unit); 
         
    return RT_ERR_OK;
} /* end of dal_esw_trap_ipWithOptionHeaderPriEnable_set */

/* Function Name:
 *      dal_esw_trap_ipWithOptionHeaderAddCPUTagEnable_get
 * Description:
 *      Get enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of CPU tag adding
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_trap_ipWithOptionHeaderAddCPUTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d", 
                unit, port);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    TRAP_SEM_LOCK(unit);

    /*Get entry From CHIP*/
    if((ret = reg_array_field_read(unit, ESW_PORT_PACKET_PARSER_OPTION_HEADER_CONTROLr,
                    port, REG_ARRAY_INDEX_NONE, ESW_OPCPUTAGf, pEnable)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }

    TRAP_SEM_UNLOCK(unit); 

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pEnable=%d", *pEnable);
         
    return RT_ERR_OK;
} /* end of dal_esw_trap_ipWithOptionHeaderAction_get */

/* Function Name:
 *      dal_esw_trap_ipWithOptionHeaderAddCPUTagEnable_set
 * Description:
 *      Set enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of CPU tag adding
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_trap_ipWithOptionHeaderAddCPUTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d, enable=%d", 
                unit, port, enable);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);

    TRAP_SEM_LOCK(unit);

    /*Set entry From CHIP*/
    if((ret = reg_array_field_write(unit, ESW_PORT_PACKET_PARSER_OPTION_HEADER_CONTROLr,
                    port, REG_ARRAY_INDEX_NONE, ESW_OPCPUTAGf, &enable)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }

    TRAP_SEM_UNLOCK(unit); 

         
    return RT_ERR_OK;
} /* end of dal_esw_trap_ipWithOptionHeaderAddCPUTagEnable_set */

/* Function Name:
 *      dal_esw_trap_pktWithCFIAction_get
 * Description:
 *      Get forwarding action of ethernet packet with CFI set.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pActoin - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_FWD_ACTION       - invalid forwarding action
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 */
int32
dal_esw_trap_pktWithCFIAction_get(uint32 unit, rtk_port_t port, rtk_action_t *pAction)
{
    int32   ret;
    uint32 value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d", 
                unit, port);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);

    TRAP_SEM_LOCK(unit);

    /*Get entry From CHIP*/
    if((ret = reg_array_field_read(unit, ESW_PORT_PACKET_PARSER_CFI_CONTROLr,
                    port, REG_ARRAY_INDEX_NONE, ESW_ICFITRAPf, &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }

    TRAP_SEM_UNLOCK(unit); 

    if(value)
        *pAction = ACTION_TRAP2CPU;
    else
        *pAction = ACTION_FORWARD;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pAction=%d", *pAction);
         
    return RT_ERR_OK;
} /* end of dal_esw_trap_pktWithCFIAction_get */

/* Function Name:
 *      dal_esw_trap_pktWithCFIAction_set
 * Description:
 *      Set forwarding action of ethernet packet with CFI set.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      actoin - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_FWD_ACTION       - invalid forwarding action
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 */
int32
dal_esw_trap_pktWithCFIAction_set(uint32 unit, rtk_port_t port, rtk_action_t action)
{
    int32   ret;
    uint32 value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d, action=%d", 
                unit, port, action);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    if(action == ACTION_TRAP2CPU)
        value = 1;
    else if(action == ACTION_FORWARD)
        value = 0;
    else
        return RT_ERR_FWD_ACTION;

    TRAP_SEM_LOCK(unit);

    /*Set entry To CHIP*/
    if((ret = reg_array_field_write(unit, ESW_PORT_PACKET_PARSER_CFI_CONTROLr,
                    port, REG_ARRAY_INDEX_NONE, ESW_ICFITRAPf, &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }

    TRAP_SEM_UNLOCK(unit); 
    
    return RT_ERR_OK;
} /* end of dal_esw_trap_pktWithCFIAction_set */

/* Function Name:
 *      dal_esw_trap_pktWithCFIPri_get
 * Description:
 *      Get priority of packets trapped to CPU.
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pPriority - pointer to priority
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_trap_pktWithCFIPri_get(uint32 unit, rtk_port_t port, rtk_pri_t *pPriority)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d", 
                unit, port);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pPriority), RT_ERR_NULL_POINTER);

    TRAP_SEM_LOCK(unit);

    /*Get entry From CHIP*/
    if((ret = reg_array_field_read(unit, ESW_PORT_PACKET_PARSER_CFI_CONTROLr,
                    port, REG_ARRAY_INDEX_NONE, ESW_CFIPRIf, pPriority)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }

    TRAP_SEM_UNLOCK(unit); 

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pPriority=%d", *pPriority);
         
    return RT_ERR_OK;
} /* end of dal_esw_trap_pktWithCFIPri_get */

/* Function Name:
 *      dal_esw_trap_pktWithCFIPri_set
 * Description:
 *      Set priority of packets trapped to CPU.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      priority - priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
int32
dal_esw_trap_pktWithCFIPri_set(uint32 unit, rtk_port_t port, rtk_pri_t priority)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d, priority=%d", 
                unit, port, priority);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((priority > RTK_DOT1P_PRIORITY_MAX), RT_ERR_NULL_POINTER);

    TRAP_SEM_LOCK(unit);

    /*Set entry to CHIP*/
     if((ret = reg_array_field_write(unit, ESW_PORT_PACKET_PARSER_CFI_CONTROLr,
                    port, REG_ARRAY_INDEX_NONE, ESW_CFIPRIf, &priority)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }

    TRAP_SEM_UNLOCK(unit); 
         
    return RT_ERR_OK;
} /* end of dal_esw_trap_pktWithCFIPri_set */

/* Function Name:
 *      dal_esw_trap_pktWithCFIPriEnable_get
 * Description:
 *      Get priority enable status of packets trapped to CPU.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to priority enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_trap_pktWithCFIPriEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d", 
                unit, port);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    TRAP_SEM_LOCK(unit);

    /*Get entry From CHIP*/
    if((ret = reg_array_field_read(unit, ESW_PORT_PACKET_PARSER_CFI_CONTROLr,
                    port, REG_ARRAY_INDEX_NONE, ESW_DFRCFIPRIf, &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }

    TRAP_SEM_UNLOCK(unit); 

    switch (value)
    {
        case 0:
            *pEnable = DISABLED;
            break;
        
        case 1:
            *pEnable = ENABLED;
            break;
        
        default:
            return RT_ERR_FAILED;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pEnable=%d", *pEnable);
         
    return RT_ERR_OK;
} /* end of dal_esw_trap_pktWithCFIPriEnable_get */

/* Function Name:
 *      dal_esw_trap_pktWithCFIPriEnable_set
 * Description:
 *      Set priority enable status of packets trapped to CPU.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - priority enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
int32
dal_esw_trap_pktWithCFIPriEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d, enable=%d", 
                unit, port, enable);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(enable >= RTK_ENABLE_END, RT_ERR_INPUT);
    
    switch (enable)
    {
        case DISABLED:
            value = 0;
            break;
        
        case ENABLED:
            value = 1;
            break;
        
        default:
            return RT_ERR_INPUT;
    }

    TRAP_SEM_LOCK(unit);

    /*Set entry to CHIP*/
    if((ret = reg_array_field_write(unit, ESW_PORT_PACKET_PARSER_CFI_CONTROLr,
                    port, REG_ARRAY_INDEX_NONE, ESW_DFRCFIPRIf, &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }     

    TRAP_SEM_UNLOCK(unit); 
         
    return RT_ERR_OK;
} /* end of dal_esw_trap_pktWithCFIPriEnable_set */

/* Function Name:
 *      dal_esw_trap_pktWithCFIAddCPUTagEnable_get
 * Description:
 *      Get enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of CPU tag adding
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_trap_pktWithCFIAddCPUTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d", 
                unit, port);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    TRAP_SEM_LOCK(unit);

    /*Get entry From CHIP*/
    if((ret = reg_array_field_read(unit, ESW_PORT_PACKET_PARSER_CFI_CONTROLr,
                    port, REG_ARRAY_INDEX_NONE, ESW_CFICPUTAGf, pEnable)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }

    TRAP_SEM_UNLOCK(unit); 

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pEnable=%d", *pEnable);
         
    return RT_ERR_OK;
} /* end of dal_esw_trap_pktWithCFIAddCPUTagEnable_get */

/* Function Name:
 *      dal_esw_trap_pktWithCFIAddCPUTagEnable_set
 * Description:
 *      Set enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of CPU tag adding
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_trap_pktWithCFIAddCPUTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d, enable=%d", 
                unit, port, enable);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);

    TRAP_SEM_LOCK(unit);

    /*Get entry From CHIP*/
    if((ret = reg_array_field_write(unit, ESW_PORT_PACKET_PARSER_CFI_CONTROLr,
                    port, REG_ARRAY_INDEX_NONE, ESW_CFICPUTAGf, &enable)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }

    TRAP_SEM_UNLOCK(unit); 
         
    return RT_ERR_OK;
} /* end of dal_esw_trap_pktWithCFIAddCPUTagEnable_set */

/* Module Name    : Trap       */
/* Sub-module Name: CFM and OAM packet */
/* Function Name:
 *      dal_esw_trap_cfmFrameAction_get
 * Description:
 *      Get forwarding action of CFM frame on specified MD level.
 * Input:
 *      unit    - unit id
 *      level   - MD level
 * Output:
 *      pAction - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 *      - ACTION_COPY2CPU
 */
 int32
dal_esw_trap_cfmFrameAction_get(uint32 unit, uint32 level, rtk_action_t *pAction)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, level=%d", unit, level);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((level > ESW_MAX_NUM_OF_CFMMDLEVEL), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);

    TRAP_SEM_LOCK(unit);

    /* get entry from CHIP*/
    if((ret = reg_field_read(unit, ESW_CFM_TRAP_CONTROLr, 
                    ESW_MDLEL0ACTf -level , &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 

    TRAP_SEM_UNLOCK(unit); 

    switch (value)
    {
        case 0:
            *pAction = RMA_ACTION_FORWARD;
            break;
        case 1:
            *pAction = RMA_ACTION_DROP;
            break;
        case 2:
            *pAction = RMA_ACTION_TRAP2CPU;
            break;
        case 3:
            *pAction = RMA_ACTION_COPY2CPU;
            break;
        default:
            return RT_ERR_FAILED;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pAction=%d", *pAction);
          
    return RT_ERR_OK;
}   /*end of dal_esw_trap_cfmFrameAction_get*/

/* Function Name:
 *      dal_esw_trap_cfmFrameAction_set
 * Description:
 *      Set forwarding action of CFM frame on specified MD level.
 * Input:
 *      unit   - unit id
 *      level  - MD level
 *      action - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 *      - ACTION_COPY2CPU
 */
int32
dal_esw_trap_cfmFrameAction_set(uint32 unit, uint32 level, rtk_action_t action)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, level=%d, action=%d", 
                unit, level, action);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((level > ESW_MAX_NUM_OF_CFMMDLEVEL), RT_ERR_INPUT);
    switch (action)
    {
        case RMA_ACTION_FORWARD:
            value = 0;
            break;
        case RMA_ACTION_DROP:
            value = 1;
            break;
        case RMA_ACTION_TRAP2CPU:
            value = 2;
            break;
        case RMA_ACTION_COPY2CPU:
            value = 3;
            break;
        default:
            return RT_ERR_FAILED;
    }

    TRAP_SEM_LOCK(unit);

    /* Set entry to CHIP*/
    if((ret = reg_field_write(unit, ESW_CFM_TRAP_CONTROLr, 
                    ESW_MDLEL0ACTf -level , &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 

    TRAP_SEM_UNLOCK(unit); 
          
    return RT_ERR_OK;
}   /*end of dal_esw_trap_cfmFrameAction_set*/

/* Function Name:
 *      dal_esw_trap_cfmFrameTrapPri_get
 * Description:
 *      Get priority of CFM packets trapped to CPU.
 * Input:
 *      unit      - unit id
 * Output:
 *      pPriority - pointer to priority
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_trap_cfmFrameTrapPri_get(uint32 unit, rtk_pri_t *pPriority)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d", unit);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pPriority), RT_ERR_NULL_POINTER);

    TRAP_SEM_LOCK(unit);

    /* get entry from CHIP*/
     if((ret = reg_field_read(unit, ESW_CFM_TRAP_CONTROLr, 
                    ESW_CFMPRIf, pPriority)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }     

    TRAP_SEM_UNLOCK(unit); 

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pPriority=%d", *pPriority);
          
    return RT_ERR_OK;
}   /*end of dal_esw_trap_cfmFrameTrapPri_get*/

/* Function Name:
 *      dal_esw_trap_cfmFrameTrapPri_set
 * Description:
 *      Set priority of CFM packets trapped to CPU.
 * Input:
 *      unit     - unit id
 *      priority - priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
int32
dal_esw_trap_cfmFrameTrapPri_set(uint32 unit, rtk_pri_t priority)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, priority=%d", unit, priority);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((priority > RTK_DOT1P_PRIORITY_MAX), RT_ERR_PRIORITY);

    TRAP_SEM_LOCK(unit);

    /* set entry to CHIP*/
    if((ret = reg_field_write(unit, ESW_CFM_TRAP_CONTROLr, 
                    ESW_CFMPRIf, &priority)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }  

    TRAP_SEM_UNLOCK(unit); 
          
    return RT_ERR_OK;
}   /*end of dal_esw_trap_cfmFrameTrapPri_set*/

/* Function Name:
 *      dal_esw_trap_cfmFrameTrapPriEnable_get
 * Description:
 *      Get priority enable status of CFM packets trapped to CPU.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to priority enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_trap_cfmFrameTrapPriEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d", unit);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    TRAP_SEM_LOCK(unit);

    /* get entry from CHIP*/
    if((ret = reg_field_read(unit, ESW_CFM_TRAP_CONTROLr, 
                    ESW_CFMDFPRIf , &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 

    TRAP_SEM_UNLOCK(unit); 

    switch (value)
    {
        case 0:
            *pEnable = DISABLED;
            break;
        
        case 1:
            *pEnable = ENABLED;
            break;
        
        default:
            return RT_ERR_FAILED;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pEnable=%d", *pEnable);
    return RT_ERR_OK;
}   /*end of dal_esw_trap_cfmFrameTrapPriEnable_get */

/* Function Name:
 *      dal_esw_trap_cfmFrameTrapPriEnable_set
 * Description:
 *      Set priority enable status of CFM packets trapped to CPU.
 * Input:
 *      unit   - unit id
 *      enable - priority enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
int32
dal_esw_trap_cfmFrameTrapPriEnable_set(uint32 unit, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, enable=%d", unit, enable);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(enable >= RTK_ENABLE_END, RT_ERR_INPUT);

    switch (enable)
    {
        case DISABLED:
            value = 0;
            break;
        
        case ENABLED:
            value = 1;
            break;
        
        default:
            return RT_ERR_INPUT;
    }

    TRAP_SEM_LOCK(unit);

    /* set entry to CHIP*/
    if((ret = reg_field_write(unit, ESW_CFM_TRAP_CONTROLr, 
                    ESW_CFMDFPRIf, &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }  

    TRAP_SEM_UNLOCK(unit); 
          
    return RT_ERR_OK;
}   /*end of dal_esw_trap_cfmFrameTrapPriEnable_set*/

/* Function Name:
 *      dal_esw_trap_cfmFrameTrapAddCPUTagEnable_get
 * Description:
 *      Get enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of CPU tag adding
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_trap_cfmFrameTrapAddCPUTagEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d", unit);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    TRAP_SEM_LOCK(unit);

    /* get entry from CHIP*/
    if((ret = reg_field_read(unit, ESW_CFM_TRAP_CONTROLr, 
                    ESW_CFMINSERTCPUTAGf, pEnable)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 

    TRAP_SEM_UNLOCK(unit); 

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pEnable=%d", *pEnable);
          
    return RT_ERR_OK;
}   /*end of dal_esw_trap_cfmFrameTrapPri_get*/

/* Function Name:
 *      dal_esw_trap_cfmFrameTrapAddCPUTagEnable_set
 * Description:
 *      Set enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit   - unit id
 *      enable - enable status of CPU tag adding
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_trap_cfmFrameTrapAddCPUTagEnable_set(uint32 unit, rtk_enable_t enable)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, enable=%d", unit, enable);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);

    TRAP_SEM_LOCK(unit);

    /* get entry from CHIP*/
    if((ret = reg_field_write(unit, ESW_CFM_TRAP_CONTROLr, 
                    ESW_CFMINSERTCPUTAGf, &enable)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 

    TRAP_SEM_UNLOCK(unit); 
          
    return RT_ERR_OK;
}   /*end of dal_esw_trap_cfmFrameTrapAddCPUTagEnable_set*/

/* Function Name:
 *      dal_esw_trap_oamPDUAction_get
 * Description:
 *      Get forwarding action of trapped oam PDU on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pAction - pointer to forwarding action of trapped oam PDU
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 *      - ACTION_COPY2CPU
 */
int32
dal_esw_trap_oamPDUAction_get(uint32 unit, rtk_port_t port, rtk_action_t *pAction)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d", unit, port);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);

    TRAP_SEM_LOCK(unit);

    /* get entry from CHIP*/
    if((ret = reg_array_field_read(unit, ESW_PORT_OAM_PARSER_CONTROLr, 
                port, REG_ARRAY_INDEX_NONE, ESW_POAM_PDUTRAPf, &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 

    TRAP_SEM_UNLOCK(unit); 

    switch (value)
    {
        case 0:
            *pAction = RMA_ACTION_FORWARD;
            break;
        case 2:
            *pAction = RMA_ACTION_DROP;
            break;
        case 3:
            *pAction = RMA_ACTION_TRAP2CPU;
            break;
        default:
            return RT_ERR_FAILED;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pAction=%d", *pAction);
          
    return RT_ERR_OK;
}   /*end of dal_esw_trap_oamPDUAction_get*/

/* Function Name:
 *      dal_esw_trap_oamPDUAction_set
 * Description:
 *      Set forwarding action of trapped oam PDU on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      action - forwarding action of trapped oam PDU
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_FWD_ACTION       - invalid forwarding action
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 *      - ACTION_COPY2CPU
 */
int32
dal_esw_trap_oamPDUAction_set(uint32 unit, rtk_port_t port, rtk_action_t action)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d, action=%d", unit, port, action);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_INPUT);

     switch (action)
    {
        case RMA_ACTION_FORWARD:
            value = 0;
            break;
        case RMA_ACTION_DROP:
            value = 2;
            break;
        case RMA_ACTION_TRAP2CPU:
            value = 3;
            break;
        default:
            return RT_ERR_FAILED;
    }
     
    TRAP_SEM_LOCK(unit);

    /*Set entry to CHIP*/
    if((ret = reg_array_field_write(unit, ESW_PORT_OAM_PARSER_CONTROLr, 
                port, REG_ARRAY_INDEX_NONE, ESW_POAM_PDUTRAPf, &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 

    TRAP_SEM_UNLOCK(unit);    
          
    return RT_ERR_OK;
}   /*end of dal_esw_trap_oamPDUAction_set*/

/* Function Name:
 *      dal_esw_trap_oamPDUPri_get
 * Description:
 *      Get priority of trapped OAM PDU on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pPriority - pointer to priority
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_trap_oamPDUPri_get(uint32 unit, rtk_port_t port, rtk_pri_t *pPriority)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d", unit, port);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pPriority), RT_ERR_NULL_POINTER);

    TRAP_SEM_LOCK(unit);

    /* get entry from CHIP*/
    if((ret = reg_array_field_read(unit, ESW_PORT_OAM_TRAP_CONTROLr, 
                port, REG_ARRAY_INDEX_NONE, ESW_OAMPDUPRIf, pPriority)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 

    TRAP_SEM_UNLOCK(unit); 

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pPriority=%d", *pPriority);
          
    return RT_ERR_OK;
}   /*end of dal_esw_trap_oamPDUPri_get*/

/* Function Name:
 *      dal_esw_trap_oamPDUPri_set
 * Description:
 *      Set priority of trapped OAM PDU on specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      priority - priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
int32
dal_esw_trap_oamPDUPri_set(uint32 unit, rtk_port_t port, rtk_pri_t priority)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d, priority=%d", 
                unit, port, priority);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_INPUT);
    RT_PARAM_CHK((priority > RTK_DOT1P_PRIORITY_MAX), RT_ERR_NULL_POINTER);

    TRAP_SEM_LOCK(unit);

    /* Set entry from CHIP*/
    /*Set Priority*/
    if((ret = reg_array_field_write(unit, ESW_PORT_OAM_TRAP_CONTROLr, 
                port, REG_ARRAY_INDEX_NONE, ESW_OAMPDUPRIf, &priority)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 

    TRAP_SEM_UNLOCK(unit); 
    
    return RT_ERR_OK;
}   /*end of dal_esw_trap_oamPDUPri_set*/

/* Function Name:
 *      dal_esw_trap_oamPDUPriEnable_get
 * Description:
 *      Get priority enable status of trapped OAM PDU on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to priority enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_trap_oamPDUPriEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d", unit, port);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    TRAP_SEM_LOCK(unit);

    /* get entry from CHIP*/
    if((ret = reg_array_field_read(unit, ESW_PORT_OAM_TRAP_CONTROLr, 
                port, REG_ARRAY_INDEX_NONE, ESW_OAMPDUDFPRIf , &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 

    TRAP_SEM_UNLOCK(unit); 

    switch (value)
    {
        case 0:
            *pEnable = DISABLED;
            break;
        
        case 1:
            *pEnable = ENABLED;
            break;
        
        default:
            return RT_ERR_FAILED;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pEnable=%d", *pEnable);
          
    return RT_ERR_OK;
}   /*end of dal_esw_trap_oamPDUPriEnable_get*/

/* Function Name:
 *      dal_esw_trap_oamPDUPriEnable_set
 * Description:
 *      Set priority enable status of trapped OAM PDU on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - priority enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
int32
dal_esw_trap_oamPDUPriEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d, enable=%d", 
                unit, port, enable);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_INPUT);
    RT_PARAM_CHK(enable >= RTK_ENABLE_END, RT_ERR_INPUT);
    
    switch (enable)
    {
        case DISABLED:
            value = 0;
            break;
        
        case ENABLED:
            value = 1;
            break;
        
        default:
            return RT_ERR_INPUT;
    }

    TRAP_SEM_LOCK(unit);

    /*Enable Priority assignment*/
    if((ret = reg_array_field_write(unit, ESW_PORT_OAM_TRAP_CONTROLr, 
                port, REG_ARRAY_INDEX_NONE, ESW_OAMPDUDFPRIf, &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 

    TRAP_SEM_UNLOCK(unit); 
    
    return RT_ERR_OK;
}   /*end of dal_esw_trap_oamPDUPriEnable_set*/

/* Function Name:
 *      dal_esw_trap_oamPDUTrapAddCPUTagEnable_get
 * Description:
 *      Get enable status of CPU tag adding for trapped OAM PDU.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of CPU tag adding
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_trap_oamPDUTrapAddCPUTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d", unit, port);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    TRAP_SEM_LOCK(unit);

    /* get entry from CHIP*/
    if((ret = reg_array_field_read(unit, ESW_PORT_OAM_TRAP_CONTROLr, 
                port, REG_ARRAY_INDEX_NONE, ESW_OAMPDUCPUTAGf, pEnable)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 

    TRAP_SEM_UNLOCK(unit); 

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pEnable=%d", *pEnable);
          
    return RT_ERR_OK;
}   /*end of dal_esw_trap_oamPDUPri_get*/

/* Function Name:
 *      dal_esw_trap_oamPDUTrapAddCPUTagEnable_set
 * Description:
 *      Set enable status of CPU tag adding for trapped OAM PDU.
 * Input:
 *      unit   - unit id
 *      enable - enable status of CPU tag adding
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_trap_oamPDUTrapAddCPUTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, port=%d, enable=%d", unit, port, enable);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_INPUT);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);

    TRAP_SEM_LOCK(unit);

    /* set entry from CHIP*/
    if((ret = reg_array_field_write(unit, ESW_PORT_OAM_TRAP_CONTROLr, 
                port, REG_ARRAY_INDEX_NONE, ESW_OAMPDUCPUTAGf, &enable)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 

    TRAP_SEM_UNLOCK(unit); 
          
    return RT_ERR_OK;
}   /*end of dal_esw_trap_oamPDUTrapAddCPUTagEnable_set*/

/* Function Name:
 *      dal_esw_trap_mgmtIpCheck_get
 * Description:
 *      Get enable status of management ip type.
 * Input:
 *      unit   - unit id
 *      type   - type of management ip
 * Output:
 *      pEnable - pointer to enable status of management ip type
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 * Note:
 *      Type of management ip type is as following:
 *      - MGMT_IP_TYPE_IPV4
 *      - MGMT_IP_TYPE_IPV6
 */
int32
dal_esw_trap_mgmtIpCheck_get(uint32 unit, rtk_trap_mgmtIpType_t type, rtk_enable_t *pEnable)
{
    int32   ret = RT_ERR_FAILED;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, ipType=%d", unit, type);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((type >= MGMT_IP_TYPE_END), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    TRAP_SEM_LOCK(unit);

    switch (type)
    {
        case MGMT_IP_TYPE_IPV4:
            if((ret = reg_field_read(unit, ESW_IP_COMPARE_CONTROLr, ESW_IPV4_COMPARE_ENABLEf, &value)) != RT_ERR_OK)
            {
                TRAP_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
                return ret;
            } 
            break;

        case MGMT_IP_TYPE_IPV6:
            if((ret = reg_field_read(unit, ESW_IP_COMPARE_CONTROLr, ESW_IPV6_COMPARE_ENABLEf, &value)) != RT_ERR_OK)
            {
                TRAP_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
                return ret;
            } 
            break;
        default:
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return RT_ERR_INPUT;
    }

    TRAP_SEM_UNLOCK(unit); 

    switch (value)
    {
        case 0:
            *pEnable = DISABLED;
            break;
        
        case 1:
            *pEnable = ENABLED;
            break;
        
        default:
            return RT_ERR_FAILED;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pEnable=%d", *pEnable);
          
    return RT_ERR_OK;
} /* end of dal_esw_trap_mgmtIpCheck_get */

/* Function Name:
 *      dal_esw_trap_mgmtIpCheck_set
 * Description:
 *      Set enable status of management ip type.
 * Input:
 *      unit   - unit id
 *      type   - type of management ip
 *      enable - enable status of management ip type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 * Note:
 *      Type of management ip type is as following:
 *      - MGMT_IP_TYPE_IPV4
 *      - MGMT_IP_TYPE_IPV6
 */
int32
dal_esw_trap_mgmtIpCheck_set(uint32 unit, rtk_trap_mgmtIpType_t type, rtk_enable_t enable)
{
    int32   ret = RT_ERR_FAILED;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, type=%d, enable=%d", unit, type, enable);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((type >= MGMT_IP_TYPE_END), RT_ERR_INPUT);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);

    switch (enable)
    {
        case DISABLED:
            value = 0;
            break;
        
        case ENABLED:
            value = 1;
            break;
        
        default:
            return RT_ERR_INPUT;
    }

    TRAP_SEM_LOCK(unit);

    switch (type)
    {
        case MGMT_IP_TYPE_IPV4:
            if((ret = reg_field_write(unit, ESW_IP_COMPARE_CONTROLr, ESW_IPV4_COMPARE_ENABLEf, &value)) != RT_ERR_OK)
            {
                TRAP_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
                return ret;
            } 
            break;

        case MGMT_IP_TYPE_IPV6:
            if((ret = reg_field_write(unit, ESW_IP_COMPARE_CONTROLr, ESW_IPV6_COMPARE_ENABLEf, &value)) != RT_ERR_OK)
            {
                TRAP_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
                return ret;
            } 
            break;

        default:
            TRAP_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return RT_ERR_INPUT;

    }

    TRAP_SEM_UNLOCK(unit); 
          
    return RT_ERR_OK;
} /* end of dal_esw_trap_mgmtIpCheck_set */

/* Function Name:
 *      _dal_esw_trap_init_config
 * Description:
 *      Initialize config of trap module for the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Must initialize trap module before calling this API.
 */
static int32
_dal_esw_trap_init_config(uint32 unit)
{
    int32   ret;
    rtk_port_t  port, max_port;
    
    max_port = HAL_GET_MAX_PORT(unit);
    for (port = 0; port < max_port; port++)
    {
        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }
        
        if ((ret = dal_esw_trap_ipWithOptionHeaderAction_set(unit, port, IPV4_FAMILY, ACTION_FORWARD)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    
        if ((ret = dal_esw_trap_ipWithOptionHeaderAction_set(unit, port, IPV6_FAMILY, ACTION_FORWARD)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    
        if ((ret = dal_esw_trap_pktWithCFIAction_set(unit, port, ACTION_FORWARD)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
            return ret;
        }
    }

    return RT_ERR_OK;
} /* end of _dal_esw_trap_init_config */
