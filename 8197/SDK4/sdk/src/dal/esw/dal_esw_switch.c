/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * $Revision: 30053 $
 * $Date: 2012-06-19 14:12:07 +0800 (Tue, 19 Jun 2012) $
 *
 * Purpose : Definition those public global APIs and its data type in the SDK.
 *
 * Feature :  Parameter settings for the system-wise view 
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
#include <osal/time.h>
#include <ioal/mem32.h>
#include <hal/chipdef/allmem.h>
#include <hal/chipdef/allreg.h>
#include <hal/chipdef/esw/rtk_esw_table_struct.h>
#include <hal/chipdef/esw/rtk_esw_reg_struct.h>
#include <hal/mac/reg.h>
#include <hal/mac/mem.h>
#include <hal/mac/drv.h>
#include <hal/common/halctrl.h>
#include <dal/esw/dal_esw_switch.h>
#include <dal/esw/dal_esw_l2.h>
#include <rtk/default.h>
#include <rtk/switch.h>
#include <rtk/l2.h>

/* 
 * Data Declaration 
 */
static uint32               switch_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         switch_sem[RTK_MAX_NUM_OF_UNIT];
static osal_mutex_t         switch_software_reset_sem[RTK_MAX_NUM_OF_UNIT];
static uint32               switch_software_reset_counter[RTK_MAX_NUM_OF_UNIT];

/*
 * Macro Definition
 */
/* switch semaphore handling */
#define SWITCH_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(switch_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_DAL|MOD_SWITCH), "switch semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)
#define SWITCH_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(switch_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_DAL|MOD_SWITCH), "switch semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)

/* switch software reset semaphore handling */
#define SWITCH_SOFTWARE_RESET_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(switch_software_reset_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_DAL|MOD_SWITCH), "switch software reset semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)
#define SWITCH_SOFTWARE_RESET_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(switch_software_reset_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_DAL|MOD_SWITCH), "switch software reset semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)

/*
 * Function Declaration
 */
static int32 _dal_esw_switch_init_config(uint32 unit);

/* Module Name    : Switch     */
/* Sub-module Name: Switch parameter settings */

/* Function Name:
 *      _dal_esw_switch_init_config
 * Description:
 *      Initialize default configuration for switch module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      1. Module must be initialized before using all of APIs in this module
 */
static int32
_dal_esw_switch_init_config(uint32 unit)
{
    uint32  val = 0, temp;
    int32   ret = RT_ERR_FAILED;
    hal_control_t *pHal_control;

    if ((pHal_control = hal_ctrlInfo_get(unit)) == NULL)
    {
        return RT_ERR_FAILED;
    }

    if (pHal_control->chip_rev_id == CHIP_REV_ID_A)
    {
        if (PHY_MODEL_ID_RTL8212F == pHal_control->pPhy_ctrl[HAL_GET_MIN_GE_PORT(unit)]->phy_model_id)
        {
            if ((ret = reg_read(unit, ESW_GLOBAL_MAC_INTERFACE_CONTROL2r, &val)) != RT_ERR_OK)
            {
                return ret;
            }
            temp = 1;
            if ((ret = reg_field_set(unit, ESW_GLOBAL_MAC_INTERFACE_CONTROL2r, ESW_SEL_GMAC24_RGMII_RXC_DELAYf, &temp, &val)) != RT_ERR_OK)
            {
                return ret;
            }
            if ((ret = reg_field_set(unit, ESW_GLOBAL_MAC_INTERFACE_CONTROL2r, ESW_SEL_GMAC24_RGMII_TXC_DELAYf, &temp, &val)) != RT_ERR_OK)
            {
                return ret;
            }
            if ((ret = reg_field_set(unit, ESW_GLOBAL_MAC_INTERFACE_CONTROL2r, ESW_SEL_GMAC25_RGMII_RXC_DELAYf, &temp, &val)) != RT_ERR_OK)
            {
                return ret;
            }
            if ((ret = reg_field_set(unit, ESW_GLOBAL_MAC_INTERFACE_CONTROL2r, ESW_SEL_GMAC25_RGMII_TXC_DELAYf, &temp, &val)) != RT_ERR_OK)
            {
                return ret;
            }
            if ((ret = reg_write(unit, ESW_GLOBAL_MAC_INTERFACE_CONTROL2r, &val)) != RT_ERR_OK)
            {
                return ret;
            }
        }
    }

    return RT_ERR_OK;
} /* end of _dal_esw_switch_init_config */

/* Function Name:
 *      dal_esw_switch_init
 * Description:
 *      Initialize switch module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      1. Module must be initialized before using all of APIs in this module
 */
int32
dal_esw_switch_init(uint32 unit)
{
    int32   ret;

    switch_init[unit] = INIT_NOT_COMPLETED;

    /* create switch semaphore */
    switch_sem[unit] = osal_sem_mutex_create();
    if (0 == switch_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_SWITCH), "switch semaphore create failed");
        return RT_ERR_FAILED;
    }
    /* create switch software reset semaphore */
    switch_software_reset_sem[unit] = osal_sem_mutex_create();
    if (0 == switch_software_reset_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_SWITCH), "switch software reset semaphore create failed");
        return RT_ERR_FAILED;
    }
    /* reset the software reset counter */
    switch_software_reset_counter[unit] = 0;
        
    /* set init flag to complete init */
    switch_init[unit] = INIT_COMPLETED;
    
    if ((ret = _dal_esw_switch_init_config(unit)) != RT_ERR_OK)
    {
        switch_init[unit] = INIT_NOT_COMPLETED;
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "Switch default configuration init failed");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_esw_switch_init */

/* Function Name:
 *      dal_esw_switch_portMaxPktLen_get
 * Description:
 *      Get maximum packet length on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pLength - pointer to maximum packet length
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
dal_esw_switch_portMaxPktLen_get(uint32 unit, rtk_port_t port, uint32 *pLength)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(switch_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pLength), RT_ERR_NULL_POINTER);

    SWITCH_SEM_LOCK(unit);
    
    /*Get the max packet length the port can accept*/
    if((ret = reg_array_field_read(unit, 
                            ESW_PORT_PACKET_PARSER_MAXLEN_CONTROLr, port, REG_ARRAY_INDEX_NONE,
                             ESW_PMAXLENf, pLength)) != RT_ERR_OK)
    {
        SWITCH_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
    }

    SWITCH_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "pLength=%d", *pLength); 
    
    return RT_ERR_OK;
}   /*end of dal_esw_switch_portMaxPktLen_get*/

/* Function Name:
 *      dal_esw_switch_portMaxPktLen_set
 * Description:
 *      Set maximum packet length on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      length - maximum packet length
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
dal_esw_switch_portMaxPktLen_set(uint32 unit, rtk_port_t port, uint32 length)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "unit=%d, length=%d", unit, length); 
    
    /* check Init status */
    RT_INIT_CHK(switch_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(length > 9*1024, RT_ERR_OUT_OF_RANGE);

    SWITCH_SEM_LOCK(unit);

    /*Set the max packet length the port can accept*/
    if((ret = reg_array_field_write(unit, 
                            ESW_PORT_PACKET_PARSER_MAXLEN_CONTROLr, port, REG_ARRAY_INDEX_NONE,
                             ESW_PMAXLENf, &length)) != RT_ERR_OK)
    {
        SWITCH_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
    }

    SWITCH_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
}   /*end of dal_esw_switch_portMaxPktLen_set*/

/* Function Name:
 *      dal_esw_switch_portSnapMode_get
 * Description:
 *      Get SNAP mode on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pSnapMode - pointer to SNAP mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      SNAP mode is as following
 *      - SNAP_MODE_AAAA03000000
 *      - SNAP_MODE_AAAA03
 *  
 */
int32
dal_esw_switch_portSnapMode_get(uint32 unit, rtk_port_t port, rtk_snapMode_t *pSnapMode)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(switch_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pSnapMode), RT_ERR_NULL_POINTER);

    SWITCH_SEM_LOCK(unit);
    
    if((ret = reg_array_field_read(unit, 
                            ESW_PORT_PACKET_PARSER_VLAN_CONTROL0r, port, REG_ARRAY_INDEX_NONE, 
                            ESW_SNAPFLAGf, pSnapMode)) != RT_ERR_OK)
    {
        SWITCH_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
    }

    SWITCH_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "pSnapMode=%d", *pSnapMode);  
    
    return RT_ERR_OK;
}   /*end of dal_esw_switch_portSnapMode_get*/

/* Function Name:
 *      dal_esw_switch_portSnapMode_set
 * Description:
 *      Set SNAP mode on specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      snapMode - SNAP mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      SNAP mode is as following
 *      - SNAP_MODE_AAAA03000000
 *      - SNAP_MODE_AAAA03
 *  
 */
int32
dal_esw_switch_portSnapMode_set(uint32 unit, rtk_port_t port, rtk_snapMode_t snapMode)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "unit=%d, port=%d, snapMode=%d", unit, port, snapMode); 
    
    /* check Init status */
    RT_INIT_CHK(switch_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((snapMode >= SNAP_MODE_END), RT_ERR_INPUT);

    SWITCH_SEM_LOCK(unit);
    
    if((ret = reg_array_field_write(unit, 
                            ESW_PORT_PACKET_PARSER_VLAN_CONTROL0r, port, REG_ARRAY_INDEX_NONE,
                            ESW_SNAPFLAGf, &snapMode)) != RT_ERR_OK)
    {
        SWITCH_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
    }

    SWITCH_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
}/*end of dal_esw_switch_portSnapMode_set*/

/* Function Name:
 *      dal_esw_switch_chksumFailAction_get
 * Description:
 *      Get forwarding action of checksum error on specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      failType - checksum fail type
 * Output:
 *      pAction  - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Checksum fail type is as following
 *      - LAYER2_CHKSUM_FAIL
 *      - LAYER3_CHKSUM_FAIL
 *      - LAYER4_CHKSUM_FAIL
 *
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 */
int32
dal_esw_switch_chksumFailAction_get(
    uint32                              unit, 
    rtk_port_t                          port, 
    rtk_switch_chksum_fail_t            failType, 
    rtk_action_t                        *pAction)
{
   int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(switch_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((failType >= CHKSUM_FAIL_END), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);

    SWITCH_SEM_LOCK(unit);

    if(failType == LAYER2_CHKSUM_FAIL)
    {
        if((ret = reg_array_field_read(unit, 
                                ESW_PORT_PACKET_PARSER_CHECKSUM_CONTROLr, port, REG_ARRAY_INDEX_NONE,
                                ESW_PL2CRCERRDROPf, pAction)) != RT_ERR_OK)
        {
            SWITCH_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
            return ret;
        }
    }
    else if(failType == LAYER3_CHKSUM_FAIL)
    {
        if((ret = reg_array_field_read(unit, 
                                ESW_PORT_PACKET_PARSER_CHECKSUM_CONTROLr, port, REG_ARRAY_INDEX_NONE,
                                ESW_PL3CSKERRDROPf, pAction)) != RT_ERR_OK)
        {
            SWITCH_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
            return ret;
        }
    }
    else
    {
        if((ret = reg_array_field_read(unit, 
                                ESW_PORT_PACKET_PARSER_CHECKSUM_CONTROLr, port, REG_ARRAY_INDEX_NONE,
                                ESW_PL4CSKERRDROPf, pAction)) != RT_ERR_OK)
        {
            SWITCH_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
            return ret;
        }
    }  

    SWITCH_SEM_UNLOCK(unit);   
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "failType=%d", failType); 
    return RT_ERR_OK;
}/*end of dal_esw_switch_chksumFailAction_get*/

/* Function Name:
 *      dal_esw_switch_chksumFailAction_set
 * Description:
 *      Set forwarding action of checksum error on specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      failType - checksum fail type
 *      action   - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_FWD_ACTION       - invalid error forwarding action
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Checksum fail type is as following
 *      - LAYER2_CHKSUM_FAIL
 *      - LAYER3_CHKSUM_FAIL
 *      - LAYER4_CHKSUM_FAIL
 *
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 */
int32
dal_esw_switch_chksumFailAction_set(
    uint32                              unit, 
    rtk_port_t                          port, 
    rtk_switch_chksum_fail_t            failType, 
    rtk_action_t                        action)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "unit=%d, port=%d, action=%d", unit, port, action); 
    
    /* check Init status */
    RT_INIT_CHK(switch_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((failType >= CHKSUM_FAIL_END), RT_ERR_INPUT);
    RT_PARAM_CHK((action > ACTION_DROP), RT_ERR_INPUT);

    SWITCH_SEM_LOCK(unit);
    
    if(failType == LAYER2_CHKSUM_FAIL)
    {
        if((ret = reg_array_field_write(unit, 
                                ESW_PORT_PACKET_PARSER_CHECKSUM_CONTROLr, port, REG_ARRAY_INDEX_NONE,
                                ESW_PL2CRCERRDROPf, &action)) != RT_ERR_OK)
        {
            SWITCH_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
            return ret;
        }
    }
    else if(failType == LAYER3_CHKSUM_FAIL)
    {
        if((ret = reg_array_field_write(unit, 
                                ESW_PORT_PACKET_PARSER_CHECKSUM_CONTROLr, port, REG_ARRAY_INDEX_NONE,
                                ESW_PL3CSKERRDROPf, &action)) != RT_ERR_OK)
        {
            SWITCH_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
            return ret;
        }
    }
    else
    {
        if((ret = reg_array_field_write(unit, 
                                ESW_PORT_PACKET_PARSER_CHECKSUM_CONTROLr, port, REG_ARRAY_INDEX_NONE,
                                ESW_PL4CSKERRDROPf, &action)) != RT_ERR_OK)
        {
            SWITCH_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
            return ret;
        }
    }  

    SWITCH_SEM_UNLOCK(unit); 
    
    return RT_ERR_OK;
}/*end of dal_esw_switch_chksumFailAction_set*/

/* Function Name:
 *      dal_esw_switch_recalcCRCEnable_get
 * Description:
 *      Get enable status of recaculate CRC on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of recaculate CRC
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      When enable, mirrored packet with bad CRC will be recaculate at mirroring port.
 */
int32
dal_esw_switch_recalcCRCEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(switch_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    SWITCH_SEM_LOCK(unit);
    
    if(HAL_IS_CPU_PORT(unit, port))
    {
        if((ret = reg_field_read(unit, ESW_CPU_CRC_CONTROLr, ESW_CPUCRCENf, pEnable)) != RT_ERR_OK)
        {
            SWITCH_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
            return ret;
        }
    }
    else
    {
        if((ret = reg_array_field_read(unit, ESW_EGRESS_PORT_CRC_CALCULATE_CONTROLr,
                  port, REG_ARRAY_INDEX_NONE, ESW_P_CRCRCf, pEnable)) != RT_ERR_OK)
        {
            SWITCH_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
            return ret;
        }  
    } 

    SWITCH_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
}/*end of dal_esw_switch_recalcCRCEnable_get*/

/* Function Name:
 *      dal_esw_switch_recalcCRCEnable_set
 * Description:
 *      Set enable status of recaculate CRC on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of recaculate CRC
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
 *      When enable, mirrored packet with bad CRC will be recaculate at mirroring port.
 */
int32
dal_esw_switch_recalcCRCEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "unit=%d, port=%d, enable=%d", unit, port, enable); 
    
    /* check Init status */
    RT_INIT_CHK(switch_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);

    SWITCH_SEM_LOCK(unit);

    if(HAL_IS_CPU_PORT(unit, port))
    {
        if((ret = reg_field_write(unit, ESW_CPU_CRC_CONTROLr, ESW_CPUCRCENf, &enable)) != RT_ERR_OK)
        {
            SWITCH_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
            return ret;
        }
    }
    else
    {
        if((ret = reg_array_field_write(unit, ESW_EGRESS_PORT_CRC_CALCULATE_CONTROLr, 
                  port, REG_ARRAY_INDEX_NONE, ESW_P_CRCRCf, &enable)) != RT_ERR_OK)
        {
            SWITCH_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
            return ret;
        }     
    } 

    SWITCH_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /*end of dal_esw_switch_recalcCRCEnable_set*/

/* Module Name    : Switch     */
/* Sub-module Name: Management address and vlan configuration */

/* Function Name:
 *      dal_esw_switch_mgmtVlanId_get
 * Description:
 *      Get management vlan of switch.
 * Input:
 *      unit - unit id
 * Output:
 *      pVid - pointer to vlan id
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
dal_esw_switch_mgmtVlanId_get(uint32 unit, rtk_vlan_t *pVid)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "unit=%d", unit); 
    
    /* check Init status */
    RT_INIT_CHK(switch_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pVid), RT_ERR_NULL_POINTER);

    SWITCH_SEM_LOCK(unit);
    
    if((ret = reg_field_read(unit, ESW_RESERVED_MULTICAST_ADDRESS_VID_CONTROL1r, 
                            ESW_MAN_IVIDf, pVid)) != RT_ERR_OK)
    {
        SWITCH_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
     }

    SWITCH_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "pVid=%d", *pVid); 
    
    return RT_ERR_OK;
} /*end of dal_esw_switch_mgmtVlanId_get*/

/* Function Name:
 *      dal_esw_switch_mgmtVlanId_set
 * Description:
 *      Set management vlan of switch.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_VLAN_VID         - Invalid vlan id
 * Note:
 *      None
 */
int32
dal_esw_switch_mgmtVlanId_set(uint32 unit, rtk_vlan_t vid)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "unit=%d, vid=%d", unit, vid); 
    
    /* check Init status */
    RT_INIT_CHK(switch_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((vid < RTK_VLAN_ID_MIN) || (vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    SWITCH_SEM_LOCK(unit);    
    
    if((ret = reg_field_write(unit, ESW_RESERVED_MULTICAST_ADDRESS_VID_CONTROL1r, 
                            ESW_MAN_IVIDf, &vid)) != RT_ERR_OK)
    {
        SWITCH_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
     }

    SWITCH_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /*end of dal_esw_switch_mgmtVlanId_set*/

/* Function Name:
 *      dal_esw_switch_outerMgmtVlanId_get
 * Description:
 *      Get outer vlan of switch.
 * Input:
 *      unit      - unit id
 * Output:
 *      pOuterVid - pointer to outer vlan id
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
dal_esw_switch_outerMgmtVlanId_get(uint32 unit, rtk_vlan_t *pOuterVid)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "unit=%d", unit); 
    
    /* check Init status */
    RT_INIT_CHK(switch_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pOuterVid), RT_ERR_NULL_POINTER);

    SWITCH_SEM_LOCK(unit);
    
    /*Get management outer vid*/  
    if((ret = reg_field_read(unit, ESW_RESERVED_MULTICAST_ADDRESS_VID_CONTROL1r, 
                            ESW_MAN_OVIDf, pOuterVid)) != RT_ERR_OK)
    {
        SWITCH_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
     }

    SWITCH_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "pOuterVid=%d", *pOuterVid); 
    
    return RT_ERR_OK;
} /*end of dal_esw_switch_outerMgmtVlanId_get*/


/* Function Name:
 *      dal_esw_switch_outerMgmtVlanId_set
 * Description:
 *      Set outer vlan of switch.
 * Input:
 *      unit     - unit id
 *      outerVid - outer vlan id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_VLAN_VID         - Invalid vlan id
 * Note:
 *  
 */
int32
dal_esw_switch_outerMgmtVlanId_set(uint32 unit, rtk_vlan_t outerVid)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "unit=%d, outerVid=%d", unit, outerVid); 
    
    /* check Init status */
    RT_INIT_CHK(switch_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((outerVid < RTK_VLAN_ID_MIN) || (outerVid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    SWITCH_SEM_LOCK(unit);    

    /*Set management outer vid*/
    if((ret = reg_field_write(unit, ESW_RESERVED_MULTICAST_ADDRESS_VID_CONTROL1r, 
                            ESW_MAN_OVIDf, &outerVid)) != RT_ERR_OK)
    {
        SWITCH_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
     }

    SWITCH_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /*end of dal_esw_switch_outerMgmtVlanId_set*/


/* Function Name:
 *      dal_esw_switch_mgmtMacAddr_get
 * Description:
 *      Get Mac address of switch.
 * Input:
 *      unit    - unit id
 * Output:
 *      pMac - pointer to Mac address
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *  
 */
int32
dal_esw_switch_mgmtMacAddr_get(uint32 unit, rtk_mac_t *pMac)
{
    int32   ret;
    uint32 val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "unit=%d", unit);     
    
    /* check Init status */
    RT_INIT_CHK(switch_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pMac), RT_ERR_NULL_POINTER);    

    SWITCH_SEM_LOCK(unit);    

    if((ret = reg_read(unit, ESW_SWITCH_MANAGEMENT_MAC_ADDRESS0r, &val)) != RT_ERR_OK)    
    {
        SWITCH_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
     }
    pMac->octet[0] = (val >> 24) & 0xff;
    pMac->octet[1] = (val >> 16) & 0xff;
    pMac->octet[2] = (val >> 8) & 0xff;
    pMac->octet[3] = (val >> 0) & 0xff;

    if((ret = reg_read(unit, ESW_SWITCH_MANAGEMENT_MAC_ADDRESS1r, &val)) != RT_ERR_OK)    
    {
        SWITCH_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
     }
    pMac->octet[4] = (val >> 24) & 0xff;
    pMac->octet[5] = (val >> 16) & 0xff;

    SWITCH_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pMac=%x-%x-%x-%x-%x-%x",
           pMac->octet[0], pMac->octet[1], pMac->octet[2],
           pMac->octet[3], pMac->octet[4], pMac->octet[5]); 
    
    return RT_ERR_OK;
} /*end of dal_esw_switch_mgmtMacAddr_get*/

/* Function Name:
 *      dal_esw_switch_mgmtMacAddr_set
 * Description:
 *      Set Mac address of switch.
 * Input:
 *      unit   - unit id
 *      pMac - pointer to Mac address
 * Output:
 *      None
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
dal_esw_switch_mgmtMacAddr_set(uint32 unit, rtk_mac_t *pMac)
{
    int32   ret;
    uint32 val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "unit=%d", unit);     
    
    /* check Init status */
    RT_INIT_CHK(switch_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pMac), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(((pMac->octet[0] & BITMASK_1B) != 0), RT_ERR_MAC);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pMac=%x-%x-%x-%x-%x-%x",
           pMac->octet[0], pMac->octet[1], pMac->octet[2],
           pMac->octet[3], pMac->octet[4], pMac->octet[5]); 

    SWITCH_SEM_LOCK(unit);    

    val = (pMac->octet[0] << 24) | (pMac->octet[1] << 16) | (pMac->octet[2] << 8) | pMac->octet[3];
    if((ret = reg_write(unit, ESW_SWITCH_MANAGEMENT_MAC_ADDRESS0r, &val)) != RT_ERR_OK)    
    {
        SWITCH_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
     }

    val = (pMac->octet[4] << 24) | (pMac->octet[5] << 16);
    if((ret = reg_write(unit, ESW_SWITCH_MANAGEMENT_MAC_ADDRESS1r, &val)) != RT_ERR_OK)    
     {
        SWITCH_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
     }

    SWITCH_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /*end of dal_esw_switch_mgmtMacAddr_set*/


/* Function Name:
 *      dal_esw_switch_IPv4Addr_get
 * Description:
 *      Get IPv4 address of switch.
 * Input:
 *      unit    - unit id
 * Output:
 *      pIpAddr - pointer to IPv4 address
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *  
 */
int32
dal_esw_switch_IPv4Addr_get(uint32 unit, uint32 *pIpAddr)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "unit=%d", unit);     
    
    /* check Init status */
    RT_INIT_CHK(switch_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pIpAddr), RT_ERR_NULL_POINTER);    

    SWITCH_SEM_LOCK(unit);    

    if((ret = reg_read(unit, ESW_SWTICH_IPV4_IP_ADDRESS0r, pIpAddr)) != RT_ERR_OK)    
    {
        SWITCH_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
     }

    SWITCH_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "pIpAddr=%d", *pIpAddr);     
    return RT_ERR_OK;
}  /*end of dal_esw_switch_IPv4Addr_get*/


/* Function Name:
 *      dal_esw_switch_IPv4Addr_set
 * Description:
 *      Set IPv4 address of switch.
 * Input:
 *      unit   - unit id
 *      ipAddr - IPv4 address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_IPV4_ADDRESS     - invalid IPv4 address
 * Note:
 *      None
 */
int32
dal_esw_switch_IPv4Addr_set(uint32 unit, uint32 ipAddr)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "unit=%d, ipAddr=%d", unit, ipAddr);     
    
    /* check Init status */
    RT_INIT_CHK(switch_init[unit]);

    SWITCH_SEM_LOCK(unit);    
        
    if((ret = reg_write(unit, ESW_SWTICH_IPV4_IP_ADDRESS0r, &ipAddr)) != RT_ERR_OK)
    {
        SWITCH_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
     }

    SWITCH_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /*end of dal_esw_switch_IPv4Addr_set*/

/* Function Name:
 *      dal_esw_switch_IPv6Addr_get
 * Description:
 *      Get IPv6 address of switch.
 * Input:
 *      unit      - unit id
 * Output:        
 *      pIpv6Addr - pointer to IPv6 address
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
dal_esw_switch_IPv6Addr_get(uint32 unit, rtk_ipv6_addr_t *pIpv6Addr)
{
    int32   ret;
    int32 idx;
    uint32 val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "unit=%d", unit);     
    
    /* check Init status */
    RT_INIT_CHK(switch_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pIpv6Addr), RT_ERR_NULL_POINTER);    

    SWITCH_SEM_LOCK(unit);    

    for(idx = 0; idx < 4; idx++)
    {
        if((ret = reg_read(unit, ESW_SWTICH_IPV6_IP_ADDRESS0_0r + idx, &val)) != RT_ERR_OK)
        {
            SWITCH_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
            return ret;
        }

        pIpv6Addr->ipv6_addr[4*idx] = (val >> 24)&0xff;
        pIpv6Addr->ipv6_addr[4*idx+1] = (val >> 16)&0xff;
        pIpv6Addr->ipv6_addr[4*idx+2] = (val >> 8)&0xff;
        pIpv6Addr->ipv6_addr[4*idx+3] = val & 0xff;
    }

    SWITCH_SEM_UNLOCK(unit);   

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "dip=0x%x:%x:%x:%x", pIpv6Addr->ipv6_addr[0], 
        pIpv6Addr->ipv6_addr[1], pIpv6Addr->ipv6_addr[2], pIpv6Addr->ipv6_addr[3]);   
    
    return RT_ERR_OK;
}  /*end of dal_esw_switch_IPv6Addr_get*/


/* Function Name:
 *      dal_esw_switch_IPv6Addr_set
 * Description:
 *      Set IPv6 address of switch.
 * Input:
 *      unit     - unit id
 *      ipv6Addr - IPv6 address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_IPV6_ADDRESS     - invalid IPv6 address
 * Note:
 *      None
 */
int32
dal_esw_switch_IPv6Addr_set(uint32 unit, rtk_ipv6_addr_t ipv6Addr)
{
    int32   ret;
    int32   idx, val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "unit=%d, ipv6Addr:0x%x:%x:%x:%x", unit,
        ipv6Addr.ipv6_addr[0], ipv6Addr.ipv6_addr[1], ipv6Addr.ipv6_addr[2], ipv6Addr.ipv6_addr[3]);     
    
    /* check Init status */
    RT_INIT_CHK(switch_init[unit]);
    
    SWITCH_SEM_LOCK(unit);    
    
    for(idx = 0; idx < 4; idx++)
    {
        val = (ipv6Addr.ipv6_addr[4*idx]<<24)|(ipv6Addr.ipv6_addr[4*idx+1]<<16)|
              (ipv6Addr.ipv6_addr[4*idx+2]<<8)|(ipv6Addr.ipv6_addr[4*idx+3]);
        if((ret = reg_write(unit, ESW_SWTICH_IPV6_IP_ADDRESS0_0r + idx, &val)) != RT_ERR_OK)
        {
            SWITCH_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
            return ret;
        }
    } 

    SWITCH_SEM_UNLOCK(unit);   
    
    return RT_ERR_OK;
} /*end of dal_esw_switch_IPv6Addr_set*/

/* Function Name:
 *      dal_esw_switch_hwInterfaceDelayEnable_get
 * Description:
 *      Get the delay state of the specified type in the specified unit.
 * Input:
 *      unit    - unit id
 *      type    - interface delay type
 * Output:
 *      pEnable - pointer to enable state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_switch_hwInterfaceDelayEnable_get(uint32 unit, rtk_switch_delayType_t type, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "unit=%d, type=%d", unit, type);
    
    /* check Init status */
    RT_INIT_CHK(switch_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((type >= DELAY_TYPE_END), RT_ERR_INPUT);    
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);    

    SWITCH_SEM_LOCK(unit);    

    switch (type)
    {
        case DELAY_TYPE_INTRA_LINK0_RX:
            if((ret = reg_field_read(unit, ESW_GLOBAL_MAC_INTERFACE_CONTROL2r, ESW_SEL_GMAC24_RGMII_RXC_DELAYf, pEnable)) != RT_ERR_OK)
            {
                SWITCH_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
                return ret;
            }
            break;
        case DELAY_TYPE_INTRA_LINK0_TX:
            if((ret = reg_field_read(unit, ESW_GLOBAL_MAC_INTERFACE_CONTROL2r, ESW_SEL_GMAC24_RGMII_TXC_DELAYf, pEnable)) != RT_ERR_OK)
            {
                SWITCH_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
                return ret;
            }
            break;
        case DELAY_TYPE_INTRA_LINK1_RX:
            if((ret = reg_field_read(unit, ESW_GLOBAL_MAC_INTERFACE_CONTROL2r, ESW_SEL_GMAC25_RGMII_RXC_DELAYf, pEnable)) != RT_ERR_OK)
            {
                SWITCH_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
                return ret;
            }
            break;
        case DELAY_TYPE_INTRA_LINK1_TX:
            if((ret = reg_field_read(unit, ESW_GLOBAL_MAC_INTERFACE_CONTROL2r, ESW_SEL_GMAC25_RGMII_TXC_DELAYf, pEnable)) != RT_ERR_OK)
            {
                SWITCH_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
                return ret;
            }
            break;
        default:
            SWITCH_SEM_UNLOCK(unit);
            return RT_ERR_FAILED;
    }

    SWITCH_SEM_UNLOCK(unit);   

    return RT_ERR_OK;
} /* end of dal_esw_switch_hwInterfaceDelayEnable_get */

/* Function Name:
 *      dal_esw_switch_hwInterfaceDelayEnable_set
 * Description:
 *      Set the delay state of the specified type in the specified unit.
 * Input:
 *      unit   - unit id
 *      type   - interface delay type
 *      enable - enable state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 * Note:
 *      None
 */
int32
dal_esw_switch_hwInterfaceDelayEnable_set(uint32 unit, rtk_switch_delayType_t type, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "unit=%d, type=%d", unit, type);
    
    /* check Init status */
    RT_INIT_CHK(switch_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((type >= DELAY_TYPE_END), RT_ERR_INPUT);    
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);

    SWITCH_SEM_LOCK(unit);    
    
    switch (type)
    {
        case DELAY_TYPE_INTRA_LINK0_RX:
            if((ret = reg_field_write(unit, ESW_GLOBAL_MAC_INTERFACE_CONTROL2r, ESW_SEL_GMAC24_RGMII_RXC_DELAYf, &enable)) != RT_ERR_OK)
            {
                SWITCH_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
                return ret;
            }
            break;
        case DELAY_TYPE_INTRA_LINK0_TX:
            if((ret = reg_field_write(unit, ESW_GLOBAL_MAC_INTERFACE_CONTROL2r, ESW_SEL_GMAC24_RGMII_TXC_DELAYf, &enable)) != RT_ERR_OK)
            {
                SWITCH_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
                return ret;
            }
            break;
        case DELAY_TYPE_INTRA_LINK1_RX:
            if((ret = reg_field_write(unit, ESW_GLOBAL_MAC_INTERFACE_CONTROL2r, ESW_SEL_GMAC25_RGMII_RXC_DELAYf, &enable)) != RT_ERR_OK)
            {
                SWITCH_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
                return ret;
            }
            break;
        case DELAY_TYPE_INTRA_LINK1_TX:
            if((ret = reg_field_write(unit, ESW_GLOBAL_MAC_INTERFACE_CONTROL2r, ESW_SEL_GMAC25_RGMII_TXC_DELAYf, &enable)) != RT_ERR_OK)
            {
                SWITCH_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
                return ret;
            }
            break;
        default:
            SWITCH_SEM_UNLOCK(unit);
            return RT_ERR_FAILED;
    }

    SWITCH_SEM_UNLOCK(unit);   
    
    return RT_ERR_OK;
} /* end of dal_esw_switch_hwInterfaceDelayEnable_set */

/* Function Name:
 *      dal_esw_switch_pkt2CpuFormat_get
 * Description:
 *      Get the packet to CPU format in the specified unit.
 * Input:
 *      unit    - unit id
 * Output:
 *      pFormat - pointer to packet format
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_switch_pkt2CpuFormat_get(uint32 unit, rtk_pktFormat_t *pFormat)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "unit=%d", unit); 
    
    /* check Init status */
    RT_INIT_CHK(switch_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pFormat), RT_ERR_NULL_POINTER);

    SWITCH_SEM_LOCK(unit);
    
    if((ret = reg_field_read(unit, ESW_PACKET_TO_CPU_PORT_FORMAT_CONTROLr, 
                            ESW_TOCPUPKTFMTf, pFormat)) != RT_ERR_OK)
    {
        SWITCH_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
     }

    SWITCH_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "pFormat=%d", *pFormat); 
    
    return RT_ERR_OK;
} /*end of dal_esw_switch_pkt2CpuFormat_get*/

/* Function Name:
 *      dal_esw_switch_pkt2CpuFormat_set
 * Description:
 *      Set the packet to CPU format in the specified unit.
 * Input:
 *      unit   - unit id
 *      format - packet format
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_switch_pkt2CpuFormat_set(uint32 unit, rtk_pktFormat_t format)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "unit=%d, format=%d", unit, format); 
    
    /* check Init status */
    RT_INIT_CHK(switch_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((format >= PKT_FORMAT_END), RT_ERR_INPUT);

    SWITCH_SEM_LOCK(unit);    
    
    if((ret = reg_field_write(unit, ESW_PACKET_TO_CPU_PORT_FORMAT_CONTROLr, 
                            ESW_TOCPUPKTFMTf, &format)) != RT_ERR_OK)
    {
        SWITCH_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
     }

    SWITCH_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /*end of dal_esw_switch_pkt2CpuFormat_set*/

/* Function Name:
 *      dal_esw_switch_sofware_reset_sem
 * Description:
 *      Take/Give software reset semaphore resource in specified device.
 * Input:
 *      unit - unit id
 *      type - semaphore type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 *      RT_ERR_NOT_INIT - The module is not initial
 * Note:
 *      The type value 0 mean lock the semaphore; 1 mean unlock the semaphore.
 */
int32 
dal_esw_switch_sofware_reset_sem(uint32 unit, uint32 type)
{
    /* check Init status */
    RT_INIT_CHK(switch_init[unit]);

    if (type == 0) /* LOCK */
    {
        SWITCH_SOFTWARE_RESET_SEM_LOCK(unit);
    }
    else
    {
        SWITCH_SOFTWARE_RESET_SEM_UNLOCK(unit);
    }

    return RT_ERR_OK;
} /* end of dal_esw_switch_sofware_reset_sem */

/* Function Name:
 *      dal_esw_switch_softwareResetCounter_get
 * Description:
 *      Get the switch software reset counter of the specified device.
 * Input:
 *      unit     - unit id
 * Output:
 *      pCounter - pointer buffer of software reset counter
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_switch_softwareResetCounter_get(uint32 unit, uint32 *pCounter)
{
    /* check Init status */
    RT_INIT_CHK(switch_init[unit]);

    SWITCH_SEM_LOCK(unit);    
    *pCounter = switch_software_reset_counter[unit];
    SWITCH_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_esw_switch_softwareResetCounter_get */

/* Function Name:
 *      dal_esw_switch_pktbuf_watchdog
 * Description:
 *      Monitor for packet buffer problem of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is monitor for detect packet buffer problem and patch it.
 */
int32
dal_esw_switch_pktbuf_watchdog(uint32 unit)
{
    uint32  curr_sys_val, peak_sys_val;
    uint32  peak_port_rx_val, peak_port_tx_val, port;
    uint32  port_linkChangeIsrCtrl, reset_global_control, port_linkChangeStatus;
    rtk_enable_t    portLearningState[29], enable;
    rtk_l2_flushCfg_t   flushCfg;
    int32   ret;

    //RT_DBG(LOG_WARNING, (MOD_DAL|MOD_SWITCH), "dal_esw_switch_pktbuf_watchdog(unit=%d)", unit); 

    /* check currently system packet buffer in valid range [0x1D, 0x500] or not? */
    ret = ioal_mem32_read(unit, 0x290028, &curr_sys_val);
    if (ret != RT_ERR_OK)
        return RT_ERR_FAILED;
    curr_sys_val &= 0x7FF;
    if ((curr_sys_val < 0x1D) || (curr_sys_val > 0x500))
    {
        //RT_DBG(LOG_WARNING, (MOD_DAL|MOD_SWITCH), "check currently system packet buffer(unit=%d) = 0x%x", unit, curr_sys_val);
        goto software_reset;
    }

    /* check peak system packet buffer in valid range <= 0x500 or not? */
    ret = ioal_mem32_read(unit, 0x6C0000, &peak_sys_val);
    if (ret != RT_ERR_OK)
        return RT_ERR_FAILED;
    peak_sys_val &= 0x7FF;
    if (peak_sys_val > 0x500)
    {
        //RT_DBG(LOG_WARNING, (MOD_DAL|MOD_SWITCH), "check peak system packet buffer(unit=%d) = 0x%x", unit, peak_sys_val); 
        goto software_reset;
    }

    for (port = 0; port <= 28; port++)
    {
        if (!HAL_IS_PORT_EXIST(unit, port))
            continue;

        /* check peak port rx packet buffer in valid range <= 0x500 or not? */
        ret = ioal_mem32_read(unit, 0xEC0004+port*0x100, &peak_port_rx_val);
        if (ret != RT_ERR_OK)
            return RT_ERR_FAILED;
        peak_port_rx_val &= 0x7FF;
        if (peak_port_rx_val > 0x500)
        {
            //RT_DBG(LOG_WARNING, (MOD_DAL|MOD_SWITCH), "check peak port rx packet buffer(unit=%d, port=%d) = 0x%x", unit, port, peak_port_rx_val); 
            goto software_reset;
        }

        /* check peak port tx packet buffer in valid range <= 0x500 or not? */
        ret = ioal_mem32_read(unit, 0xEB0008+port*0x100, &peak_port_tx_val);
        if (ret != RT_ERR_OK)
            return RT_ERR_FAILED;
        peak_port_tx_val &= 0x7FF;
        if (peak_port_tx_val > 0x500)
        {
            //RT_DBG(LOG_WARNING, (MOD_DAL|MOD_SWITCH), "check peak port tx packet buffer(unit=%d, port=%d) = 0x%x", unit, port, peak_port_tx_val); 
            goto software_reset;
        }
    }

    return RT_ERR_OK;

software_reset:
    //RT_DBG(LOG_WARNING, (MOD_DAL|MOD_SWITCH), "software_reset(unit=%d)", unit); 
    /* save per-port learning state to shadow */
    for (port = 0; port <= 28; port++)
    {
        if (!HAL_IS_PORT_EXIST(unit, port))
            continue;

        ret = reg_array_field_read(unit, ESW_PORT_LAYER2_MAC_ADDRESS_LEARNING_CONTROL0r, 
                        port, REG_ARRAY_INDEX_NONE, ESW_SML_ENf, &enable);
        if (ret != RT_ERR_OK)
            return RT_ERR_FAILED;
        portLearningState[port] = enable;
    }
    /* configure all ports learning state to disable */
    enable = 0;
    for (port = 0; port <= 28; port++)
    {
        if (!HAL_IS_PORT_EXIST(unit, port))
            continue;

        ret = reg_array_field_write(unit, ESW_PORT_LAYER2_MAC_ADDRESS_LEARNING_CONTROL0r, 
                        port, REG_ARRAY_INDEX_NONE, ESW_SML_ENf, &enable);
        if (ret != RT_ERR_OK)
            return RT_ERR_FAILED;
    }
    /* flush all ports dynamic unicast mac address */
    osal_memset(&flushCfg, 0, sizeof(rtk_l2_flushCfg_t));
    flushCfg.flushByPort = 1;
    flushCfg.portOrTrunk = ENABLED; /* ENABLED: Flush by port */
    for (port = 0; port <= 28; port++)
    {
        if (!HAL_IS_PORT_EXIST(unit, port))
            continue;

        flushCfg.port = port;
        dal_esw_l2_ucastAddr_flush(unit, &flushCfg);
    }
    /* Delay some time to make sure the address is delete by chip and counter minus to 0 */
    osal_time_usleep(50 * 1000); /* wait 50ms */
    /* Lock the switch software reset semaphore */
    SWITCH_SOFTWARE_RESET_SEM_LOCK(unit);
    /* Check the indirect access semaphore in phy, table and storm function */
    MEM_SEM_LOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
    MEM_SEM_LOCK(unit, INDIRECT_CTRL_GROUP_STORM);
    PHY_SEM_LOCK(unit);

    /* save per-port link change interrupt register */
    ioal_mem32_read(unit, 0x660008, &port_linkChangeIsrCtrl);
    /* configure per-port (port 0~28) link change interrupt register to 0x0 */
    ioal_mem32_write(unit, 0x660008, 0);
    /* SW_Reset */
    ioal_mem32_read(unit, 0x20000, &reset_global_control);
    reset_global_control &= ~0x1;
    ioal_mem32_write(unit, 0x20000, reset_global_control);
    /* Delay some time to make sure SW_Reset is completed */
    osal_time_usleep(10);
    switch_software_reset_counter[unit]++;
    //RT_DBG(LOG_WARNING, (MOD_DAL|MOD_SWITCH), "switch_software_reset_counter[%d] = %d)", unit, switch_software_reset_counter[unit]);
    /* unlock the indirect access semaphore in phy, table and storm function */
    PHY_SEM_UNLOCK(unit);
    MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_STORM);
    MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
    /* wait 10ms for mac polling PHY update one round completed */
    osal_time_usleep(20000);
    /* Unlock the switch software reset semaphore */
    SWITCH_SOFTWARE_RESET_SEM_UNLOCK(unit);
    /* Read the link change status and clear it */
    ioal_mem32_read(unit, 0x660034, &port_linkChangeStatus);
    ioal_mem32_write(unit, 0x660034, port_linkChangeStatus);

    /* restore per-port learning state from shadow */
    for (port = 0; port <= 28; port++)
    {
        if (!HAL_IS_PORT_EXIST(unit, port))
            continue;

        enable = portLearningState[port];
        reg_array_field_write(unit, ESW_PORT_LAYER2_MAC_ADDRESS_LEARNING_CONTROL0r, 
                        port, REG_ARRAY_INDEX_NONE, ESW_SML_ENf, &enable);
    }

    /* restore per-port link change interrupt register */
    ioal_mem32_write(unit, 0x660008, port_linkChangeIsrCtrl);

    return RT_ERR_OK;
} /* end of dal_esw_switch_pktbuf_watchdog */
