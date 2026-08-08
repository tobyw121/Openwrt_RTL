/*
 * Copyright(c) Realtek Semiconductor Corporation, 2009
 * All rights reserved.
 *
 * $Revision: 30053 $
 * $Date: 2012-06-19 14:12:07 +0800 (Tue, 19 Jun 2012) $
 *
 * Purpose : Definition those public dot1x APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) Unauth packet handling
 *           2) 802.1X port-based NAC
 *           3) 802.1X MAC-based NAC
 *           4) 802.1X parameter
 *           5) Parameter for trapped packets
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
#include <dal/esw/dal_esw_dot1x.h>
#include <dal/esw/dal_esw_l2.h>
#include <rtk/default.h>
#include <rtk/dot1x.h>
#include <rtk/l2.h>

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */
static uint32               dot1x_init[RTK_MAX_NUM_OF_UNIT];
static osal_mutex_t         dot1x_sem[RTK_MAX_NUM_OF_UNIT];

/*
 * Macro Declaration
 */
#define DOT1X_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(dot1x_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_DOT1X|MOD_DAL), "semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)

#define DOT1X_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(dot1x_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_DOT1X|MOD_DAL), "semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)

/*
 * Function Declaration
 */

/* Function Name:
 *      dal_esw_dot1x_init
 * Description:
 *      Initial the dot1x module of the specified device..
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
dal_esw_dot1x_init(uint32 unit)
{
    uint32  cpu_port, value;
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DOT1X|MOD_DAL), "unit=%d", unit); 
    
    dot1x_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    dot1x_sem[unit] = osal_sem_mutex_create();
    if (0 == dot1x_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DOT1X|MOD_DAL), "semaphore create failed");
        return RT_ERR_FAILED;
    }
    
    dot1x_init[unit] = INIT_COMPLETED;
    
    /* Configure dot1x settings for CPU port as following:
     * disabled, unauthorized, IN direction
     */
    cpu_port = HAL_GET_CPU_PORT(unit);
    value = 0;
    if ((ret = reg_array_field_write(unit, ESW_PORT_BASED_DOT1X_CONTROLr
                        , cpu_port, REG_ARRAY_INDEX_NONE, ESW_DOT1XPORTENf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_DOT1X), "");
        return ret;
    }
    value = 0;
    if ((ret = reg_array_field_write(unit, ESW_PORT_BASED_DOT1X_CONTROLr
                        , cpu_port, REG_ARRAY_INDEX_NONE, ESW_DOT1XPORTAUTHf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_DOT1X), "");
        return ret;
    }
    value = 1;
    if ((ret = reg_array_field_write(unit, ESW_PORT_EGRESS_DOT1X_CONTROLr
                        , cpu_port, REG_ARRAY_INDEX_NONE, ESW_DOT1XOPDIRf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_DOT1X), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_esw_dot1x_init */

/* Module Name    : Dot1x                  */
/* Sub-module Name: Unauth packet handling */

/* Function Name:
 *      dal_esw_dot1x_portUnauthPacketOper_get
 * Description:
 *      Get the configuration of unauthorized behavior for both 802.1x port and mac based network access control.
 * Input:
 *      unit           - unit id
 *      port           - port id
 * Output:
 *      pUnauth_action - The action of how to handle unauthorized packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *      Forwarding action for tagged unauth packet is as following
 *      - DOT1X_ACTION_DROP
 *      - DOT1X_ACTION_TRAP2CPU
 *      - DOT1X_ACTION_TO_GUEST_VLAN
 */
int32
dal_esw_dot1x_portUnauthPacketOper_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_dot1x_unauth_action_t   *pUnauthAction)
{
    return dal_esw_dot1x_portUnauthUntagPacketOper_get(unit, port, pUnauthAction);
} /* end of dal_esw_dot1x_portUnauthPacketOper_get */


/* Function Name:
 *      dal_esw_dot1x_portUnauthPacketOper_set
 * Description:
 *      Set the configuration of unauthorized behavior for both 802.1x port and mac based network access control.
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      unauth_action - The action of how to handle unauthorized packet
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 * Note:
 *      Forwarding action for tagged unauth packet is as following
 *      - DOT1X_ACTION_DROP
 *      - DOT1X_ACTION_TRAP2CPU
 *      - DOT1X_ACTION_TO_GUEST_VLAN
 */
int32
dal_esw_dot1x_portUnauthPacketOper_set(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_dot1x_unauth_action_t   unauthAction)
{
    int32   ret;
    
    if ((ret = dal_esw_dot1x_portUnauthTagPacketOper_set(unit, port, unauthAction)) != RT_ERR_OK)
    {
        return RT_ERR_OK;
    }
    
    if ((ret = dal_esw_dot1x_portUnauthUntagPacketOper_set(unit, port, unauthAction)) != RT_ERR_OK)
    {
        return RT_ERR_OK;
    }
    
    return ret;
} /* end of dal_esw_dot1x_portUnauthPacketOper_set */

/* Function Name:
 *      dal_esw_dot1x_portUnauthTagPacketOper_get
 * Description:
 *      Get forwarding action for tagged unauth packet on specified port.
 * Input:
 *      unit          - unit id
 *      port          - port id
 * Output:
 *      pUnauthAction - pointer to forwarding action for tagged unauth packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Forwarding action for tagged unauth packet is as following
 *      - DOT1X_ACTION_DROP
 *      - DOT1X_ACTION_TRAP2CPU
 *      - DOT1X_ACTION_TO_GUEST_VLAN
 */
int32
dal_esw_dot1x_portUnauthTagPacketOper_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_dot1x_unauth_action_t   *pUnauthAction)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DOT1X), "unit=%d, port=%d, pUnauthAction=%x"
            , unit, port, pUnauthAction);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pUnauthAction), RT_ERR_NULL_POINTER);
    
    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_GUEST_VLAN_ID_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_TAGDOT1XUNAUTHBHf, &value)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DOT1X), "");
        return ret;
    }
    
    DOT1X_SEM_UNLOCK(unit);
    
    switch (value)
    {
        case 0: /* 0b00 */
            *pUnauthAction = DOT1X_ACTION_DROP;
            break;
        
        case 1: /* 0b01 */
            *pUnauthAction = DOT1X_ACTION_TRAP2CPU;
            break;
        
        case 2: /* 0b10 */
            *pUnauthAction = DOT1X_ACTION_TO_GUEST_VLAN;
            break;
        
        default:
            return RT_ERR_FAILED;
    }
    
    return RT_ERR_OK;
} /* end of dal_esw_dot1x_portUnauthTagPacketOper_get */

/* Function Name:
 *      dal_esw_dot1x_portUnauthTagPacketOper_set
 * Description:
 *      Set forwarding action for tagged unauth packet on specified port.
 * Input:
 *      unit         - unit id
 *      port         - port id
 *      unauthAction - forwarding action for tagged unauth packet
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
 *      Forwarding action for tagged unauth packet is as following
 *      - DOT1X_ACTION_DROP
 *      - DOT1X_ACTION_TRAP2CPU
 *      - DOT1X_ACTION_TO_GUEST_VLAN
 */
int32
dal_esw_dot1x_portUnauthTagPacketOper_set(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_dot1x_unauth_action_t   unauthAction)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DOT1X), "unit=%d, port=%d, pUnauthAction=%d"
            , unit, port, unauthAction);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    switch (unauthAction)
    {
        case DOT1X_ACTION_DROP:
            value = 0;
            break;
        
        case DOT1X_ACTION_TRAP2CPU:
            value = 1;
            break;
        
        case DOT1X_ACTION_TO_GUEST_VLAN:
            value = 2;
            break;
        
        default:
            return RT_ERR_INPUT;
    }
    
    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_GUEST_VLAN_ID_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_TAGDOT1XUNAUTHBHf, &value)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DOT1X), "");
        return ret;
    }
    
    DOT1X_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_dot1x_portUnauthTagPacketOper_set */

/* Function Name:
 *      dal_esw_dot1x_portUnauthUntagPacketOper_get
 * Description:
 *      Get forwarding action for untagged unauth packet on specified port.
 * Input:
 *      unit          - unit id
 *      port          - port id
 * Output:
 *      pUnauthAction - pointer to forwarding action for untagged unauth packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Forwarding action for tagged unauth packet is as following
 *      - DOT1X_ACTION_DROP
 *      - DOT1X_ACTION_TRAP2CPU
 *      - DOT1X_ACTION_TO_GUEST_VLAN
 */
int32
dal_esw_dot1x_portUnauthUntagPacketOper_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_dot1x_unauth_action_t   *pUnauthAction)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DOT1X), "unit=%d, port=%d, pUnauthAction=%x"
            , unit, port, pUnauthAction);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pUnauthAction), RT_ERR_NULL_POINTER);
    
    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_GUEST_VLAN_ID_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_UNTAGDOT1XUNAUTHBHf, &value)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DOT1X), "");
        return ret;
    }
    
    DOT1X_SEM_UNLOCK(unit);
    
    switch (value)
    {
        case 0: /* 0b00 */
            *pUnauthAction = DOT1X_ACTION_DROP;
            break;
        
        case 1: /* 0b01 */
            *pUnauthAction = DOT1X_ACTION_TRAP2CPU;
            break;
        
        case 2: /* 0b10 */
            *pUnauthAction = DOT1X_ACTION_TO_GUEST_VLAN;
            break;
        
        default:
            return RT_ERR_FAILED;
    }
    
    return RT_ERR_OK;
} /* end of dal_esw_dot1x_portUnauthUntagPacketOper_get */

/* Function Name:
 *      dal_esw_dot1x_portUnauthUntagPacketOper_set
 * Description:
 *      Set forwarding action for untagged unauth packet on specified port.
 * Input:
 *      unit         - unit id
 *      port         - port id
 *      unauthAction - forwarding action for untagged unauth packet
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
 *      Forwarding action for tagged unauth packet is as following
 *      - DOT1X_ACTION_DROP
 *      - DOT1X_ACTION_TRAP2CPU
 *      - DOT1X_ACTION_TO_GUEST_VLAN
 */
int32
dal_esw_dot1x_portUnauthUntagPacketOper_set(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_dot1x_unauth_action_t   unauthAction)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DOT1X), "unit=%d, port=%d, pUnauthAction=%d"
            , unit, port, unauthAction);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    switch (unauthAction)
    {
        case DOT1X_ACTION_DROP:
            value = 0;
            break;
        
        case DOT1X_ACTION_TRAP2CPU:
            value = 1;
            break;
        
        case DOT1X_ACTION_TO_GUEST_VLAN:
            value = 2;
            break;
        
        default:
            return RT_ERR_INPUT;
    }
    
    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_GUEST_VLAN_ID_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_UNTAGDOT1XUNAUTHBHf, &value)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DOT1X), "");
        return ret;
    }
    
    DOT1X_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_dot1x_portUnauthUntagPacketOper_set */

/* Module Name    : Dot1x                 */
/* Sub-module Name: 802.1X port-based NAC */

/* Function Name:
 *      dal_esw_dot1x_portBasedEnable_get
 * Description:
 *      Get the status of 802.1x port-based network access control on a specific port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - The status of 802.1x port-based network access control.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    1. If a port is 802.1x port based network access control "enabled",
 *       it should be authenticated so packets from that port wont be dropped or trapped to CPU.
 *
 *    2. The status of 802.1x port-based network access control is as following:
 *       - DISABLED
 *       - ENABLED
 */
int32
dal_esw_dot1x_portBasedEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DOT1X), "unit=%d, port=%d, pEnable=%x"
            , unit, port, pEnable);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_BASED_DOT1X_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_DOT1XPORTENf, &value)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DOT1X), "");
        return ret;
    }
    
    DOT1X_SEM_UNLOCK(unit);
    
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
    
    return RT_ERR_OK;
} /* end of dal_esw_dot1x_portBasedEnable_get */

/* Function Name:
 *      dal_esw_dot1x_portBasedEnable_set
 * Description:
 *      Set the status of 802.1x port-based network access control on a specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - The status of 802.1x port-based network access control.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - Invalid port id
 * Note:
 *    1. If a port is 802.1x port based network access control "enabled",
 *       it should be authenticated so packets from that port wont be dropped or trapped to CPU.
 *
 *    2. The status of 802.1x port-based network access control is as following:
 *       - DISABLED
 *       - ENABLED
 */
int32
dal_esw_dot1x_portBasedEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DOT1X), "unit=%d, port=%d, enable=%u"
            , unit, port, enable);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
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

    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_BASED_DOT1X_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_DOT1XPORTENf, &value)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DOT1X), "");
        return ret;
    }
    
    DOT1X_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_dot1x_portBasedEnable_set */

/* Function Name:
 *      dal_esw_dot1x_portBasedAuthStatus_get
 * Description:
 *      Get the authenticated status of 802.1x port-based network access control on a specific port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pPort_auth - The status of 802.1x port-based network access controlx is authenticated
 *                    or unauthenticated.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    The authenticated status of 802.1x port-based network access control is as following:
 *    - UNAUTH
 *    - AUTH
 */
int32
dal_esw_dot1x_portBasedAuthStatus_get(uint32 unit, rtk_port_t port, rtk_dot1x_auth_status_t *pPort_auth)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DOT1X), "unit=%d, port=%d, pPort_auth=%x"
            , unit, port, pPort_auth);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pPort_auth), RT_ERR_NULL_POINTER);
    
    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_BASED_DOT1X_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_DOT1XPORTAUTHf, &value)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DOT1X), "");
        return ret;
    }
    
    DOT1X_SEM_UNLOCK(unit);
    
    switch (value)
    {
        case 0:
            *pPort_auth = UNAUTH;
            break;
        
        case 1:
            *pPort_auth = AUTH;
            break;
        
        default:
            return RT_ERR_FAILED;
    }
    
    return RT_ERR_OK;
} /* end of dal_esw_dot1x_portBasedAuthStatus_get */

/* Function Name:
 *      dal_esw_dot1x_portBasedAuthStatus_set
 * Description:
 *      Set the authenticated status of 802.1x port-based network access control on a specific
 *      port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      port_auth - The status of 802.1x port-based network access control is authenticated
 *                 or unauthenticated.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - Invalid port id
 * Note:
 *    The authenticated status of 802.1x port-based network access control is as following:
 *    - UNAUTH
 *    - AUTH
 */
int32
dal_esw_dot1x_portBasedAuthStatus_set(uint32 unit, rtk_port_t port, rtk_dot1x_auth_status_t port_auth)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DOT1X), "unit=%d, port=%d, port_auth=%u"
            , unit, port, port_auth);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    switch (port_auth)
    {
        case UNAUTH:
            value = 0;
            break;
        
        case AUTH:
            value = 1;
            break;
        
        default:
            return RT_ERR_INPUT;
    }

    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_BASED_DOT1X_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_DOT1XPORTAUTHf, &value)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DOT1X), "");
        return ret;
    }
    
    DOT1X_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_dot1x_portBasedAuthStatus_set */

/* Function Name:
 *      dal_esw_dot1x_portBasedDirection_get
 * Description:
 *      Get the operate controlled direction 802.1x port-based network access control on a specific
 *      port.
 * Input:
 *      unit            - unit id
 *      port            - port id
 *
 * Output:
 *      pPort_direction - The controlled direction of 802.1x port-based network access control is BOTH
 *                        or IN.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - Invalid port id
 * Note:
 *    The operate controlled direction of 802.1x port-based network access control is as following:
 *    - BOTH
 *    - IN
 */
int32
dal_esw_dot1x_portBasedDirection_get(uint32 unit, rtk_port_t port, rtk_dot1x_direction_t *pPort_direction)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DOT1X), "unit=%d, port=%d, pPort_direction=%x"
            , unit, port, pPort_direction);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pPort_direction), RT_ERR_NULL_POINTER);
    
    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_EGRESS_DOT1X_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_DOT1XOPDIRf, &value)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DOT1X), "");
        return ret;
    }
    
    DOT1X_SEM_UNLOCK(unit);
    
    switch (value)
    {
        case 0:
            *pPort_direction = BOTH;
            break;
        
        case 1:
            *pPort_direction = IN;
            break;
        
        default:
            return RT_ERR_FAILED;
    }
    
    return RT_ERR_OK;
} /* end of dal_esw_dot1x_portBasedDirection_get */

/* Function Name:
 *      dal_esw_dot1x_portBasedDirection_set
 * Description:
 *      Set the operate controlled direction 802.1x port-based network access control on a specific
 *      port.
 * Input:
 *      unit           - unit id
 *      port           - port id
 *      port_direction - The controlled direction of 802.1x port-based network access control is BOTH
 *                        or IN.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - Invalid port id
 * Note:
 *    The operate controlled direction of 802.1x port-based network access control is as following:
 *    - BOTH
 *    - IN
 */
int32
dal_esw_dot1x_portBasedDirection_set(uint32 unit, rtk_port_t port, rtk_dot1x_direction_t port_direction)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DOT1X), "unit=%d, port=%d, port_direction=%u"
            , unit, port, port_direction);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    switch (port_direction)
    {
        case BOTH:
            value = 0;
            break;
        
        case IN:
            value = 1;
            break;
        
        default:
            return RT_ERR_INPUT;
    }

    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_EGRESS_DOT1X_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_DOT1XOPDIRf, &value)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DOT1X), "");
        return ret;
    }
    
    DOT1X_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_dot1x_portBasedDirection_set */

/* Module Name    : Dot1x                */
/* Sub-module Name: 802.1x MAC-based NAC */

/* Function Name:
 *      dal_esw_dot1x_macBasedEnable_get
 * Description:
 *      Get the status of 802.1x MAC-based network access control on a specific port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - The status of 802.1x MAC-based network access control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    1. If a port is 802.1x MAC based network access control "enabled",
 *       the incoming packets should be authenticated so packets from that port wont be dropped
 *       or trapped to CPU.
 *
 *    2. The status of 802.1x MAC-based network access control is as following:
 *       - DISABLED
 *       - ENABLED
 */
int32
dal_esw_dot1x_macBasedEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DOT1X), "unit=%d, port=%d, pEnable=%x"
            , unit, port, pEnable);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_MAC_BASED_DOT1X_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_DOT1XMACENf, &value)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DOT1X), "");
        return ret;
    }
    
    DOT1X_SEM_UNLOCK(unit);
    
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
    
    return RT_ERR_OK;
} /* end of dal_esw_dot1x_macBasedEnable_get */

/* Function Name:
 *      dal_esw_dot1x_macBasedEnable_set
 * Description:
 *      Set the status of 802.1x MAC-based network access control on a specific port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - The status of 802.1x MAC-based network access control
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - Invalid unit id
 *      RT_ERR_PORT_ID - Invalid port id
 * Note:
 *    1. If a port is 802.1x MAC based network access control "enabled",
 *       the incoming packets should be authenticated so packets from that port wont be dropped
 *       or trapped to CPU.
 *
 *    2. The status of 802.1x MAC-based network access control is as following:
 *       - DISABLED
 *       - ENABLED
 */
int32
dal_esw_dot1x_macBasedEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DOT1X), "unit=%d, port=%d, enable=%u"
            , unit, port, enable);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
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

    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_MAC_BASED_DOT1X_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_DOT1XMACENf, &value)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DOT1X), "");
        return ret;
    }
    
    DOT1X_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_dot1x_macBasedEnable_set */

/* Function Name:
 *      dal_esw_dot1x_macBasedAuthMac_add
 * Description:
 *      Add an authenticated MAC to ASIC
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      vid       - vlan id
 *      pAuth_mac - The authenticated MAC
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_VLAN_ID  - Invalid vlan id
 *      RT_ERR_MAC      - Invalid MAC address
 * Note:
 *      None.
 */
int32
dal_esw_dot1x_macBasedAuthMac_add(
    uint32      unit,
    rtk_port_t  port,
    rtk_vlan_t  vid,
    rtk_mac_t   *pAuth_mac)
{
    int32   ret;
    rtk_l2_ucastAddr_t l2_addr;
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    osal_memcpy(&(l2_addr.mac), pAuth_mac, sizeof(rtk_mac_t));
    l2_addr.vid = vid;
    
    if (dal_esw_l2_addr_get(unit, &l2_addr) != RT_ERR_OK)
    {
        l2_addr.port = port;
        l2_addr.trk_gid = 0;
        l2_addr.flags = 0;
        l2_addr.state = 0;
        l2_addr.auth = 1;
        ret = dal_esw_l2_addr_add(unit, &l2_addr);
    }
    else
    {
        l2_addr.auth = 1;
        ret = dal_esw_l2_addr_set(unit, &l2_addr);
    }
    
    return ret;
} /* end of dal_esw_dot1x_macBasedAuthMac_add */

/* Function Name:
 *      dal_esw_dot1x_macBasedAuthMac_del
 * Description:
 *      Delete an authenticated MAC from ASIC
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      vid       - vlan id
 *      pAuth_mac - The authenticated MAC
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_VLAN_ID  - Invalid vlan id
 *      RT_ERR_MAC      - Invalid MAC address
 * Note:
 *      None.
 */
int32
dal_esw_dot1x_macBasedAuthMac_del(
    uint32      unit,
    rtk_port_t  port,
    rtk_vlan_t  vid,
    rtk_mac_t   *pAuth_mac)
{
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    return dal_esw_l2_addr_del(unit, vid, pAuth_mac);
} /* end of dal_esw_dot1x_macBasedAuthMac_del */

/* Function Name:
 *      dal_esw_dot1x_macBasedDirection_get
 * Description:
 *      Get the operate controlled direction 802.1x mac-based network access control on system.
 *
 * Input:
 *      unit           - unit id
 *
 * Output:
 *      pMac_direction - The controlled direction of 802.1x mac-based network access control is BOTH
 *                       or IN.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 * Note:
 *    The operate controlled direction of 802.1x mac-based network access control is as following:
 *    - BOTH
 *    - IN
 */
int32
dal_esw_dot1x_macBasedDirection_get(uint32 unit, rtk_dot1x_direction_t *pMac_direction)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DOT1X), "unit=%d, pMac_direction=%x"
            , unit, pMac_direction);
    
    RT_PARAM_CHK((NULL == pMac_direction), RT_ERR_NULL_POINTER);
    
    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, ESW_DOT1X_NETWORK_ACCESS_RELATED_CONTROLr
                        , ESW_DOT1XMACOPDIRf, &value)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DOT1X), "");
        return ret;
    }
    
    DOT1X_SEM_UNLOCK(unit);
    
    switch (value)
    {
        case 0:
            *pMac_direction = BOTH;
            break;
        
        case 1:
            *pMac_direction = IN;
            break;
        
        default:
            return RT_ERR_FAILED;
    }
    
    return RT_ERR_OK;
} /* end of dal_esw_dot1x_macBasedDirection_get */

/* Function Name:
 *      dal_esw_dot1x_macBasedDirection_set
 * Description:
 *      Set the operate controlled direction 802.1x mac-based network access control on system.
 *
 * Input:
 *      unit          - unit id
 *      mac_direction - The controlled direction of 802.1x mac-based network access control is BOTH
 *                      or IN.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 * Note:
 *    The operate controlled direction of 802.1x mac-based network access control is as following:
 *    - BOTH
 *    - IN
 */
int32
dal_esw_dot1x_macBasedDirection_set(uint32 unit, rtk_dot1x_direction_t mac_direction)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DOT1X), "unit=%d, mac_direction=%u"
            , unit, mac_direction);
    
    
    switch (mac_direction)
    {
        case BOTH:
            value = 0;
            break;
        
        case IN:
            value = 1;
            break;
        
        default:
            return RT_ERR_INPUT;
    }

    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_write(unit, ESW_DOT1X_NETWORK_ACCESS_RELATED_CONTROLr
                        , ESW_DOT1XMACOPDIRf, &value)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DOT1X), "");
        return ret;
    }
    
    DOT1X_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_dot1x_macBasedDirection_set */

/* Module Name    : Dot1x            */
/* Sub-module Name: 802.1X parameter */

/* Function Name:
 *      dal_esw_dot1x_portGuestVlan_get
 * Description:
 *      Get guest vlan on specified port.
 * Input:
 *      unit        - unit id
 *      port        - port id
 * Output:
 *      pGuest_vlan - pointer to guest vlan id
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
dal_esw_dot1x_portGuestVlan_get(uint32 unit, rtk_port_t port, rtk_vlan_t *pGuest_vlan)
{
    int32   ret;
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DOT1X), "unit=%d, port=%d, pGuest_vlan=%x"
            , unit, port, pGuest_vlan);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pGuest_vlan), RT_ERR_NULL_POINTER);
    
    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_GUEST_VLAN_ID_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_GVLANIDf, pGuest_vlan)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DOT1X), "");
        return ret;
    }
    
    DOT1X_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_dot1x_portGuestVlan_get */

/* Function Name:
 *      dal_esw_dot1x_portGuestVlan_set
 * Description:
 *      Set guest vlan on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      guest_vlan - guest vlan id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_VLAN_VID         - invalid vlan id
 * Note:
 *      None
 */
int32
dal_esw_dot1x_portGuestVlan_set(uint32 unit, rtk_port_t port, rtk_vlan_t guest_vlan)
{
    int32   ret;
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DOT1X), "unit=%d, port=%d, guest_vlan=%u"
            , unit, port, guest_vlan);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((guest_vlan < RTK_VLAN_ID_MIN) || (guest_vlan > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    
    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_GUEST_VLAN_ID_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_GVLANIDf, &guest_vlan)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DOT1X), "");
        return ret;
    }
    
    DOT1X_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_dot1x_portGuestVlan_set */

/* Function Name:
 *      dal_esw_dot1x_guestVlanBehavior_get
 * Description:
 *      Get forwarding behavior for host in guest vlan.
 * Input:
 *      unit      - unit id
 * Output:
 *      pBehavior - pointer to Forwarding behavior
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Forwarding behavior is as following
 *      - DISALLOW_TO_AUTH_DA
 *      - ALLOW_TO_AUTH_DA
 */
int32
dal_esw_dot1x_guestVlanBehavior_get(uint32 unit, rtk_dot1x_guestVlanBehavior_t *pBehavior)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DOT1X), "unit=%d, pBehavior=%x"
            , unit, pBehavior);
    
    RT_PARAM_CHK((NULL == pBehavior), RT_ERR_NULL_POINTER);
    
    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, ESW_DOT1X_NETWORK_ACCESS_RELATED_CONTROLr
                        , ESW_GVOPDIRf, &value)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DOT1X), "");
        return ret;
    }
    
    DOT1X_SEM_UNLOCK(unit);
    
    switch (value)
    {
        case 0:
            *pBehavior = DISALLOW_TO_AUTH_DA;
            break;
        
        case 1:
            *pBehavior = ALLOW_TO_AUTH_DA;
            break;
       
        default:
            return RT_ERR_FAILED;
    }
    
    return RT_ERR_OK;
} /* end of dal_esw_dot1x_guestVlanBehavior_get */

/* Function Name:
 *      dal_esw_dot1x_guestVlanBehavior_set
 * Description:
 *      Set forwarding behavior for host in guest vlan.
 * Input:
 *      unit     - unit id
 *      behavior - Forwarding behavior
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Forwarding behavior is as following
 *      - DISALLOW_TO_AUTH_DA
 *      - ALLOW_TO_AUTH_DA
 */
int32
dal_esw_dot1x_guestVlanBehavior_set(uint32 unit, rtk_dot1x_guestVlanBehavior_t behavior)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DOT1X), "unit=%d, behavior=%u"
            , unit, behavior);
    
    
    switch (behavior)
    {
        case DISALLOW_TO_AUTH_DA:
            value = 0;
            break;
        
        case ALLOW_TO_AUTH_DA:
            value = 1;
            break;
        
        default:
            return RT_ERR_INPUT;
    }

    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_write(unit, ESW_DOT1X_NETWORK_ACCESS_RELATED_CONTROLr
                        , ESW_GVOPDIRf, &value)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DOT1X), "");
        return ret;
    }
    
    DOT1X_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_dot1x_guestVlanBehavior_set */

/* Function Name:
 *      dal_esw_dot1x_guestVlanRouteBehavior_get
 * Description:
 *      Get forwarding behavior of packets in guest vlan and routed.
 * Input:
 *      unit       - unit id
 * Output:
 *      pFwdAction - pointer to forwarding behavior
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Forwarding behavior is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 */
int32
dal_esw_dot1x_guestVlanRouteBehavior_get(uint32 unit, rtk_action_t *pFwdAction)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DOT1X), "unit=%d, pFwdAction=%x"
            , unit, pFwdAction);
    
    RT_PARAM_CHK((NULL == pFwdAction), RT_ERR_NULL_POINTER);
    
    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, ESW_DOT1X_NETWORK_ACCESS_RELATED_CONTROLr
                        , ESW_GVRTBHf, &value)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DOT1X), "");
        return ret;
    }
    
    DOT1X_SEM_UNLOCK(unit);
    
    switch (value)
    {
        case 0:
            *pFwdAction = ACTION_DROP;
            break;
        
        case 1:
            *pFwdAction = ACTION_FORWARD;
            break;
        
        case 2:
            *pFwdAction = ACTION_TRAP2CPU;
            break;
       
        default:
            return RT_ERR_FAILED;
    }
    
    return RT_ERR_OK;
} /* end of dal_esw_dot1x_guestVlanRouteBehavior_get */

/* Function Name:
 *      dal_esw_dot1x_guestVlanRouteBehavior_set
 * Description:
 *      Set forwarding behavior of packets in guest vlan and routed.
 * Input:
 *      unit      - unit id
 *      fwdAction - forwarding behavior
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_FWD_ACTION       - invalid forwarding action
 * Note:
 *      Forwarding behavior is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 */
int32
dal_esw_dot1x_guestVlanRouteBehavior_set(uint32 unit, rtk_action_t fwdAction)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DOT1X), "unit=%d, fwdAction=%u"
            , unit, fwdAction);
    
    
    switch (fwdAction)
    {
        case ACTION_DROP:
            value = 0;
            break;
        
        case ACTION_FORWARD:
            value = 1;
            break;
        
        case ACTION_TRAP2CPU:
            value = 2;
            break;
        
        default:
            return RT_ERR_INPUT;
    }

    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_write(unit, ESW_DOT1X_NETWORK_ACCESS_RELATED_CONTROLr
                        , ESW_GVRTBHf, &value)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DOT1X), "");
        return ret;
    }
    
    DOT1X_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_dot1x_guestVlanRouteBehavior_set */

/* Module Name    : Dot1x                         */
/* Sub-module Name: Parameter for trapped packets */

/* Function Name:
 *      dal_esw_dot1x_trapPri_get
 * Description:
 *      Get priority of trapped packets.
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
dal_esw_dot1x_trapPri_get(uint32 unit, rtk_pri_t *pPriority)
{
    int32   ret;
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DOT1X), "unit=%d, pPriority=%x"
            , unit, pPriority);
    
    RT_PARAM_CHK((NULL == pPriority), RT_ERR_NULL_POINTER);
    
    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, ESW_DOT1X_NETWORK_ACCESS_RELATED_CONTROLr
                        , ESW_DOT1XTRAPPRIf, pPriority)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DOT1X), "");
        return ret;
    }
    
    DOT1X_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_dot1x_trapPri_get */

/* Function Name:
 *      dal_esw_dot1x_trapPri_set
 * Description:
 *      Set priority of trapped packet.
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
dal_esw_dot1x_trapPri_set(uint32 unit, rtk_pri_t priority)
{
    int32   ret;
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DOT1X), "unit=%d, priority=%u"
            , unit, priority);
    
    RT_PARAM_CHK((priority > HAL_INTERNAL_PRIORITY_MAX(unit)), RT_ERR_QOS_INT_PRIORITY);
    
    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_write(unit, ESW_DOT1X_NETWORK_ACCESS_RELATED_CONTROLr
                        , ESW_DOT1XTRAPPRIf, &priority)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DOT1X), "");
        return ret;
    }
    
    DOT1X_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_dot1x_trapPri_set */

/* Function Name:
 *      dal_esw_dot1x_trapPriEnable_get
 * Description:
 *      Get priority status of trapped packets.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to trap priority status
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
dal_esw_dot1x_trapPriEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value;

    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DOT1X), "unit=%d", unit);
    
    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, ESW_DOT1X_NETWORK_ACCESS_RELATED_CONTROLr
                        , ESW_DOT1XDEFPRIf, &value)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DOT1X), "");
        return ret;
    }
    
    DOT1X_SEM_UNLOCK(unit);
    
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
    
    return RT_ERR_OK;
} /* end of dal_esw_dot1x_trapPriEnable_get */

/* Function Name:
 *      dal_esw_dot1x_trapPriEnable_set
 * Description:
 *      Set priority status of trapped packet.
 * Input:
 *      unit   - unit id
 *      enable - configure trap priority status
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
dal_esw_dot1x_trapPriEnable_set(uint32 unit, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DOT1X), "unit=%d, enable=%u", unit, enable);
    
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
    
    DOT1X_SEM_LOCK(unit);
    
    /* set value from CHIP*/
    if ((ret = reg_field_write(unit, ESW_DOT1X_NETWORK_ACCESS_RELATED_CONTROLr
                        , ESW_DOT1XDEFPRIf, &value)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DOT1X), "");
        return ret;
    }
    
    DOT1X_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_dot1x_trapPriEnable_set */

/* Function Name:
 *      dal_esw_dot1x_trapAddCPUTagEnable_get
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
dal_esw_dot1x_trapAddCPUTagEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DOT1X), "unit=%d, pEnable=%x"
            , unit, pEnable);
    
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, ESW_DOT1X_NETWORK_ACCESS_RELATED_CONTROLr
                        , ESW_DOT1XCPUTAGf, &value)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DOT1X), "");
        return ret;
    }
    
    DOT1X_SEM_UNLOCK(unit);
    
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
    
    return RT_ERR_OK;
} /* end of dal_esw_dot1x_trapAddCPUTagEnable_get */

/* Function Name:
 *      dal_esw_dot1x_trapAddCPUTagEnable_set
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
dal_esw_dot1x_trapAddCPUTagEnable_set(uint32 unit, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DOT1X), "unit=%d, enable=%u"
            , unit, enable);
    
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
    
    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_write(unit, ESW_DOT1X_NETWORK_ACCESS_RELATED_CONTROLr
                        , ESW_DOT1XCPUTAGf, &value)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DOT1X), "");
        return ret;
    }
    
    DOT1X_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_dot1x_trapAddCPUTagEnable_set */


