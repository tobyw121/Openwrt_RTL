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
#include <hal/chipdef/ssw/rtk_ssw_table_struct.h>
#include <hal/chipdef/ssw/rtk_ssw_reg_struct.h>
#include <hal/common/halctrl.h>
#include <hal/mac/reg.h>
#include <hal/mac/mem.h>
#include <dal/ssw/dal_ssw_trap.h>
#include <rtk/default.h>
#include <rtk/trap.h>
/* 
 * Symbol Definition 
 */
#define     RMA_ADDR_PREFIX_LEN         5

/* 
 * Data Declaration 
 */
static uint32               trap_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         trap_sem[RTK_MAX_NUM_OF_UNIT];

const static uint16 rmaControl_regidx[] = {SSW_RESERVED_MULTICAST_ADDRESS_CONTROL0r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL0r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL0r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL0r,
                                           SSW_RESERVED_MULTICAST_ADDRESS_CONTROL0r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL0r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL0r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL0r,
                                           SSW_RESERVED_MULTICAST_ADDRESS_CONTROL0r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL0r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL0r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL0r,
                                           SSW_RESERVED_MULTICAST_ADDRESS_CONTROL0r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL0r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL0r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL0r,
                                           SSW_RESERVED_MULTICAST_ADDRESS_CONTROL1r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL1r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL1r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL1r,
                                           SSW_RESERVED_MULTICAST_ADDRESS_CONTROL1r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL1r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL1r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL1r,
                                           SSW_RESERVED_MULTICAST_ADDRESS_CONTROL1r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL1r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL1r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL1r,
                                           SSW_RESERVED_MULTICAST_ADDRESS_CONTROL1r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL1r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL1r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL1r,
                                           SSW_RESERVED_MULTICAST_ADDRESS_CONTROL2r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL2r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL2r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL2r,
                                           SSW_RESERVED_MULTICAST_ADDRESS_CONTROL2r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL2r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL2r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL2r,
                                           SSW_RESERVED_MULTICAST_ADDRESS_CONTROL2r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL2r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL2r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL2r,
                                           SSW_RESERVED_MULTICAST_ADDRESS_CONTROL2r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL2r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL2r, SSW_RESERVED_MULTICAST_ADDRESS_CONTROL2r,};

const static uint16 rma_fieldidx[] = {SSW_RMA00f, SSW_RMA01f, SSW_RMA02f, SSW_RMA03f, SSW_RMA04f, SSW_RMA05f, SSW_RMA06f, SSW_RMA07f, SSW_RMA08f, SSW_RMA09f, SSW_RMA0Af, SSW_RMA0Bf, SSW_RMA0Cf, SSW_RMA0Df, SSW_RMA0Ef, SSW_RMA0Ff,\
                                      SSW_RMA10f, SSW_RMA11f, SSW_RMA12f, SSW_RMA13f, SSW_RMA14f, SSW_RMA15f, SSW_RMA16f, SSW_RMA17f, SSW_RMA18f, SSW_RMA19f, SSW_RMA1Af, SSW_RMA1Bf, SSW_RMA1Cf, SSW_RMA1Df, SSW_RMA1Ef, SSW_RMA1Ff,\
                                      SSW_RMA20f, SSW_RMA21f, SSW_RMA22f, SSW_RMA23f, SSW_RMA24f, SSW_RMA25f, SSW_RMA26f, SSW_RMA27f, SSW_RMA28f, SSW_RMA29f, SSW_RMA2Af, SSW_RMA2Bf, SSW_RMA2Cf, SSW_RMA2Df, SSW_RMA2Ef, SSW_RMA2Ff};

const static uint16 trapReason_fieldidx[] = {SSW_RMA_TRAP_PRIORITYf, SSW_IPV4IGMP_TRAP_PRIORITYf, SSW_IPV6MLD_TRAP_PRIORITYf, SSW_DOT1XEAPOL_TRAP_PRIORITYf, SSW_VLANERR_TRAP_PRIORITYf,
                                             SSW_SLPCHANGE_TRAP_PRIORITYf, SSW_MULTICASTDLF_TRAP_PRIORITYf, SSW_CFI_TRAP_PRIORITYf, SSW_DOT1XUNAUTH_TRAP_PRIORITYf};

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

/* Function Name:
 *      dal_ssw_trap_init
 * Description:
 *      Initial the trap module of the specified device..
 * Input:
 *      unit     - unit id
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
int32
dal_ssw_trap_init(uint32 unit)
{
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
    
    return RT_ERR_OK;
} /* end of dal_ssw_trap_init */

/* Function Name:
 *      dal_ssw_trap_1xMacChangePort2CpuEnable_get
 * Description:
 *      Get the configuration about when 802.1x MAC-based authenticated MAC address changes port
 *      whether it need be trapped to CPU.
 * Input:
 *      unit                - unit id
 * Output:
 *      pEnable            - status of authenticated MAC address change port trap to CPU
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_NULL_POINTER  - NULL pointer
 * Note:
 *      The status of authenticated MAC address change port trap to CPU:
 *      - DISABLED
 *      - ENABLED
 */
int32
dal_ssw_trap_1xMacChangePort2CpuEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    TRAP_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_field_read(unit, SSW_L2_TABLE_CONTROLr, SSW_EN1XMAC_CH_PORT_TRAP_CPUf, pEnable)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }
    TRAP_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pEnable=%d", *pEnable);
    
    return RT_ERR_OK;
} /* end of dal_ssw_trap_1xMacChangePort2CpuEnable_get */

/* Function Name:
 *      dal_ssw_trap_1xMacChangePort2CpuEnable_set
 * Description:
 *      Set the configuration about when 802.1x MAC-based authenticated MAC address changes port
 *      whether it need be trapped to CPU.
 * Input:
 *      unit            - unit id
 *      enable          - status of authenticated MAC address change port trap to CPU
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID   - Invalid unit id
 *      RT_ERR_INPUT     - Invalid input parameter
 * Note:
 *      The status of authenticated MAC address change port trap to CPU:
 *      - DISABLED
 *      - ENABLED
 */
int32
dal_ssw_trap_1xMacChangePort2CpuEnable_set(uint32 unit, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, enable=%d", unit, enable);
    
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(enable >= RTK_ENABLE_END, RT_ERR_INPUT);
    
    TRAP_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_field_write(unit, SSW_L2_TABLE_CONTROLr, SSW_EN1XMAC_CH_PORT_TRAP_CPUf, &enable)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }
    
    TRAP_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_trap_1xMacChangePort2CpuEnable_set */

/* Function Name:
 *      dal_ssw_trap_igmpCtrlPkt2CpuEnable_get
 * Description:
 *      Get the configuration about whether IGMP control packets need be trapped to CPU.
 * Input:
 *      unit                - unit id
 * Output:
 *      pEnable            - status of IGMP control packet trap to CPU
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_NULL_POINTER  - NULL pointer
 * Note:
 *      The status of IGMP control packet trap to CPU:
 *      - DISABLED
 *      - ENABLED
 */
int32
dal_ssw_trap_igmpCtrlPkt2CpuEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    TRAP_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_field_read(unit, SSW_L2_TABLE_CONTROLr, SSW_IGMP_TRAP_CPUf, pEnable)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }
    TRAP_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pEnable=%x", *pEnable);
    
    return RT_ERR_OK;
} /* end of dal_ssw_trap_igmpCtrlPkt2CpuEnable_get */

/* Function Name:
 *      dal_ssw_trap_igmpCtrlPkt2CpuEnable_set
 * Description:
 *      Set the configuration about whether IGMP control packets need be trapped to CPU.
 * Input:
 *      unit                - unit id
 *      enable              - status of IGMP control packet trap to CPU
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_INPUT     - Invalid input parameter
 * Note:
 *      The status of IGMP control packet trap to CPU:
 *      - DISABLED
 *      - ENABLED
 */
int32
dal_ssw_trap_igmpCtrlPkt2CpuEnable_set(uint32 unit, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, enable=%d", unit, enable);
    
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((enable != DISABLED && enable != ENABLED), RT_ERR_INPUT);
    
    TRAP_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_field_write(unit, SSW_L2_TABLE_CONTROLr, SSW_IGMP_TRAP_CPUf, &enable)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }
    
    TRAP_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_trap_igmpCtrlPkt2CpuEnable_set */

/* Function Name:
 *      dal_ssw_trap_l2McastPkt2CpuEnable_get
 * Description:
 *      Get the configuration about whether L2 multicast packets lookup miss need be trapped to CPU.
 * Input:
 *      unit                - unit id
 * Output:
 *      pEnable            - status of L2 multicast packet trap to CPU
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_NULL_POINTER  - NULL pointer
 * Note:
 *      The status of L2 multicast packet trap to CPU:
 *      - DISABLED
 *      - ENABLED
 */
int32
dal_ssw_trap_l2McastPkt2CpuEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    TRAP_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_field_read(unit, SSW_L2_TABLE_CONTROLr, SSW_L2_MULTICAST_TRAP_CPUf, pEnable)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }
    TRAP_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pEnable=%d", *pEnable);
    
    return RT_ERR_OK;
} /* end of dal_ssw_trap_l2McastPkt2CpuEnable_get */

/* Function Name:
 *      dal_ssw_trap_l2McastPkt2CpuEnable_set
 * Description:
 *      Set the configuration about whether L2 multicast packets lookup miss need be trapped to CPU.
 * Input:
 *      unit                - unit id
 *      enable              - status of L2 multicast packet trap to CPU
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_INPUT         - Invalid input parameter
 * Note:
 *      The status of L2 multicast packet trap to CPU:
 *      - DISABLED
 *      - ENABLED
 */
int32
dal_ssw_trap_l2McastPkt2CpuEnable_set(uint32 unit, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, enable=%d", unit, enable);
    
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((enable != DISABLED && enable != ENABLED), RT_ERR_INPUT);
    
    TRAP_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_field_write(unit, SSW_L2_TABLE_CONTROLr, SSW_L2_MULTICAST_TRAP_CPUf, &enable)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }    
    TRAP_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_trap_l2McastPkt2CpuEnable_set */

/* Function Name:
 *      dal_ssw_trap_ipMcastPkt2CpuEnable_get
 * Description:
 *      Get the configuration about whether IP multicast packet lookup miss need be trapped to CPU.
 * Input:
 *      unit                - unit id
 * Output:
 *      pEnable            - status of IP multicast packet trap to CPU
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_NULL_POINTER  - NULL pointer
 * Note:
 *      The status of IP multicast packet trap to CPU:
 *      - DISABLED
 *      - ENABLED
 */
int32
dal_ssw_trap_ipMcastPkt2CpuEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    TRAP_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_field_read(unit, SSW_L2_TABLE_CONTROLr, SSW_IP_MULTICAST_TRAP_CPUf, pEnable)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }
    TRAP_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pEnable=%d", *pEnable);
    
    return RT_ERR_OK;
} /* end of dal_ssw_trap_ipMcastPkt2CpuEnable_get */

/* Function Name:
 *      dal_ssw_trap_ipMcastPkt2CpuEnable_set
 * Description:
 *      Set the configuration about whether IP multicast packet lookup miss need be trapped to CPU.
 * Input:
 *      unit                - unit id
 *      enable              - status of IP multicast packet trap to CPU
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_INPUT         - Invalid input parameter
 * Note:
 *      The status of IP multicast packet trap to CPU:
 *      - DISABLED
 *      - ENABLED
 */
int32
dal_ssw_trap_ipMcastPkt2CpuEnable_set(uint32 unit, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, enable=%d", unit, enable);
    
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((enable != DISABLED && enable != ENABLED), RT_ERR_INPUT);
    
    TRAP_SEM_LOCK(unit);
    /* write entry to CHIP*/
    if ((ret = reg_field_write(unit, SSW_L2_TABLE_CONTROLr, SSW_IP_MULTICAST_TRAP_CPUf, &enable)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }    
    TRAP_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_trap_ipMcastPkt2CpuEnable_set */

/* Function Name:
 *      dal_ssw_trap_rmaAction_get
 * Description:
 *      Get action of reserved multicast address(RMA) frame.
 * Input:
 *      unit                - unit id
 *      pRma_frame         - Reserved multicast address.
 * Output:
 *      pRma_action        - RMA action
 *                              -   RMA_ACTION_FORWARD
 *                              -   RMA_ACTION_DROP
 *                              -   RMA_ACTION_TRAP2CPU
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_INPUT         - Invalid input parameter
 *      RT_ERR_NULL_POINTER  - NULL pointer
 * Note:
 *      None.
 */
int32
dal_ssw_trap_rmaAction_get(uint32 unit, rtk_mac_t *pRma_frame, rtk_trap_rma_action_t *pRma_action)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d", unit);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pRma_action), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pRma_frame), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(osal_memcmp(pRma_frame->octet, rma_prefix, RMA_ADDR_PREFIX_LEN), RT_ERR_RMA_ADDR);
    RT_PARAM_CHK(pRma_frame->octet[5] > 0x2f, RT_ERR_RMA_ADDR);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pRma_frame=%x-%x-%x-%x-%x-%x", 
           pRma_frame->octet[0], pRma_frame->octet[1], pRma_frame->octet[2], pRma_frame->octet[3], 
           pRma_frame->octet[4], pRma_frame->octet[5]);    
    
    TRAP_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_field_read(unit, rmaControl_regidx[pRma_frame->octet[5]], rma_fieldidx[pRma_frame->octet[5]], &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }
    TRAP_SEM_UNLOCK(unit);
    
    switch (value)
    {
        case 0:
            *pRma_action = RMA_ACTION_FORWARD;
            break;
        case 1:
            *pRma_action = RMA_ACTION_DROP;
            break;
        case 2:
            *pRma_action = RMA_ACTION_TRAP2CPU;
            break;
        default:
            return RT_ERR_FAILED;
    }
            
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pRma_action=%d", *pRma_action);    
            
    return RT_ERR_OK;
} /* end of dal_ssw_trap_rmaAction_get */

/* Function Name:
 *      dal_ssw_trap_rmaAction_set
 * Description:
 *      Set action of reserved multicast address(RMA) frame.
 * Input:
 *      unit                - unit id
 *      pRma_frame         - Reserved multicast address.
 *      rma_action          - RMA action
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_INPUT         - Invalid input parameter
 *      RT_ERR_RMA_ACTION    - Invalid RMA action
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
dal_ssw_trap_rmaAction_set(uint32 unit, rtk_mac_t *pRma_frame, rtk_trap_rma_action_t rma_action)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d", unit);
       
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pRma_frame), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(rma_action >= RMA_ACTION_END, RT_ERR_RMA_ACTION);
    RT_PARAM_CHK(osal_memcmp(pRma_frame->octet, rma_prefix, RMA_ADDR_PREFIX_LEN), RT_ERR_RMA_ADDR);
    RT_PARAM_CHK(pRma_frame->octet[5] > 0x2f, RT_ERR_RMA_ADDR);

    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pRma_frame=%x-%x-%x-%x-%x-%x, rma_action=%d", 
           pRma_frame->octet[0], pRma_frame->octet[1], pRma_frame->octet[2], pRma_frame->octet[3], 
           pRma_frame->octet[4], pRma_frame->octet[5], rma_action);
    
    switch (rma_action)
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
        default:
            return RT_ERR_FAILED;
    }
       
    TRAP_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_field_write(unit, rmaControl_regidx[pRma_frame->octet[5]], rma_fieldidx[pRma_frame->octet[5]], &value)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }
    TRAP_SEM_UNLOCK(unit);
            
    return RT_ERR_OK;
} /* end of dal_ssw_trap_rmaAction_set */

/* Function Name:
 *      dal_ssw_trap_reasonTrapToCPUPriority_get
 * Description:
 *      Get priority value of a packet that trapped to CPU port according to specific reason.
 * Input:
 *      unit                - unit id
 *      type                - reason that trap to CPU port.
 * Output:
 *      pPriority          - configured internal priority for such reason.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - Invalid input parameter
 *      RT_ERR_NULL_POINTER     - NULL pointer
 * Note:
 *      Currently the trap reason that supported are listed as follows:
 *      TRAP_REASON_RMA
 *      TRAP_REASON_IPV4IGMP,
 *      TRAP_REASON_IPV6MLD,
 *      TRAP_REASON_1XEAPOL,
 *      TRAP_REASON_VLANERR,
 *      TRAP_REASON_SLPCHANGE,
 *      TRAP_REASON_MULTICASTDLF,
 *      TRAP_REASON_CFI,
 *      TRAP_REASON_1XUNAUTH.
 */
int32
dal_ssw_trap_reasonTrapToCPUPriority_get(uint32 unit, rtk_trap_reason_type_t type, rtk_pri_t *pPriority)
{
    int32   ret;    
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, type=%d", 
                       unit, type);
    
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(type >= TRAP_REASON_END, RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pPriority), RT_ERR_NULL_POINTER);
        
    TRAP_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_field_read(unit, SSW_CPU_REASON_TO_PRIORITY_CONTROLr, trapReason_fieldidx[type], pPriority)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);        
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 
    TRAP_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pPriority=%d", *pPriority);
    
    return RT_ERR_OK;
} /* end of dal_ssw_trap_reasonTrapToCPUPriority_get */   

/* Function Name:
 *      dal_ssw_trap_reasonTrapToCPUPriority_set
 * Description:
 *      Set priority value of a packet that trapped to CPU port according to specific reason.
 * Input:
 *      unit                - unit id
 *      type                - reason that trap to CPU port.
 *      priority            - internal priority that is going to be set for specific trap reason.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - Invalid input parameter
 * Note:
 *      Currently the trap reason that supported are listed as follows:
 *      TRAP_REASON_RMA
 *      TRAP_REASON_IPV4IGMP,
 *      TRAP_REASON_IPV6MLD,
 *      TRAP_REASON_1XEAPOL,
 *      TRAP_REASON_VLANERR,
 *      TRAP_REASON_SLPCHANGE,
 *      TRAP_REASON_MULTICASTDLF,
 *      TRAP_REASON_CFI,
 *      TRAP_REASON_1XUNAUTH.
 */
int32
dal_ssw_trap_reasonTrapToCPUPriority_set(uint32 unit, rtk_trap_reason_type_t type, rtk_pri_t priority)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, type=%d\
                       priority=%d", unit, type, priority);
    
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(type >= TRAP_REASON_END, RT_ERR_INPUT);
    RT_PARAM_CHK(priority > HAL_INTERNAL_PRIORITY_MAX(unit), RT_ERR_QOS_INT_PRIORITY);
    
    TRAP_SEM_LOCK(unit);
    /* write value to CHIP*/
    if ((ret = reg_field_write(unit, SSW_CPU_REASON_TO_PRIORITY_CONTROLr, trapReason_fieldidx[type], &priority)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);        
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    } 
    TRAP_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_trap_reasonTrapToCPUPriority_set */ 

/* Function Name:
 *      dal_ssw_trap_pkt2CpuEnable_get
 * Description:
 *      Get the configuration about whether specific packet type needed to be trapped to CPU.
 * Input:
 *      unit                - unit id
 *      type                - packet type that trapped to CPU port.      
 * Output:
 *      pEnable             - status of special packet type trap to CPU
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT         - Invalid input parameter
 * Note:
 *      Currently the trap packet type that supported are listed as follows:
 *      TRAP_TYPE_1XMAC_PORTCHG
 *      TRAP_TYPE_IPV4_IGMP       
 *      TRAP_TYPE_IPMC_DLF         
 *      TRAP_TYPE_L2MC_DLF          
 *      TRAP_TYPE_IPV6_MLD         
 *      TRAP_TYPE_CFI_1 
 */
int32
dal_ssw_trap_pkt2CpuEnable_get(uint32 unit, rtk_trap_type_t type, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  reg_field;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, type=%d", unit, type);
    
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((type >= TRAP_TYPE_CONTROL_END), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    switch (type)
    {
        case TRAP_TYPE_1XMAC_PORTCHG:
            reg_field = SSW_EN1XMAC_CH_PORT_TRAP_CPUf;
            break;
        case TRAP_TYPE_IPV4_IGMP:
            reg_field = SSW_IGMP_TRAP_CPUf;
            break;
        case TRAP_TYPE_L2MC_DLF:
            reg_field = SSW_L2_MULTICAST_TRAP_CPUf;
            break;
        case TRAP_TYPE_IPMC_DLF:
            reg_field = SSW_IP_MULTICAST_TRAP_CPUf;  
            break;
        case TRAP_TYPE_IPV6_MLD:
            reg_field = SSW_IPV6_MLD_TRAP_CPUf;
            break;
        case TRAP_TYPE_CFI_1:
            reg_field = SSW_CFI_1_TRAP_CPUf;
            break;
        default:
            return RT_ERR_INPUT;
    }

    TRAP_SEM_LOCK(unit);

    if ((ret = reg_field_read(unit, SSW_L2_TABLE_CONTROLr, reg_field, pEnable)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }    

    TRAP_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "pEnable=%d", *pEnable);
    
    return RT_ERR_OK;
} /* end of dal_ssw_trap_pkt2CpuEnable_get */

/* Function Name:
 *      dal_ssw_trap_pkt2CpuEnable_set
 * Description:
 *      Set the configuration about whether specific packet type needed to be trapped to CPU.
 * Input:
 *      unit                - unit id
 *      type                - packet type that trapped to CPU port.
 *      enable              - status of specific packet type trap to CPU
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT         - Invalid input parameter
 * Note:
 *      Currently the trap packet type that supported are listed as follows:
 *      TRAP_TYPE_1XMAC_PORTCHG
 *      TRAP_TYPE_IPV4_IGMP       
 *      TRAP_TYPE_IPMC_DLF         
 *      TRAP_TYPE_L2MC_DLF          
 *      TRAP_TYPE_IPV6_MLD         
 *      TRAP_TYPE_CFI_1 
 */
int32
dal_ssw_trap_pkt2CpuEnable_set(uint32 unit, rtk_trap_type_t type, rtk_enable_t enable)
{
    int32   ret;
    uint32  reg_field;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRAP), "unit=%d, type=%d, enable=%d", 
           unit, type, enable);
    
    /* check Init status */
    RT_INIT_CHK(trap_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((type >= TRAP_TYPE_CONTROL_END), RT_ERR_INPUT);
    RT_PARAM_CHK((ENABLED != enable && DISABLED != enable), RT_ERR_INPUT);
       
    switch (type)
    {
        case TRAP_TYPE_1XMAC_PORTCHG:
            reg_field = SSW_EN1XMAC_CH_PORT_TRAP_CPUf;
            break;
        case TRAP_TYPE_IPV4_IGMP:
            reg_field = SSW_IGMP_TRAP_CPUf;
            break;
        case TRAP_TYPE_L2MC_DLF:
            reg_field = SSW_L2_MULTICAST_TRAP_CPUf;
            break;
        case TRAP_TYPE_IPMC_DLF:
            reg_field = SSW_IP_MULTICAST_TRAP_CPUf;  
            break;
        case TRAP_TYPE_IPV6_MLD:
            reg_field = SSW_IPV6_MLD_TRAP_CPUf;
            break;
        case TRAP_TYPE_CFI_1:
            reg_field = SSW_CFI_1_TRAP_CPUf;
            break;
        default:
            return RT_ERR_INPUT;
    }

    TRAP_SEM_LOCK(unit);

    if ((ret = reg_field_write(unit, SSW_L2_TABLE_CONTROLr, reg_field, &enable)) != RT_ERR_OK)
    {
        TRAP_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRAP), "");
        return ret;
    }    

    TRAP_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_trap_pkt2CpuEnable_set */
