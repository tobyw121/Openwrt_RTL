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
 * Purpose : Definition those public dot1x APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *            1) 802.1X Port-based Network Access Control
 *            2) 802.1X MAC-based Network Access Control
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
#include <dal/ssw/dal_ssw_dot1x.h>
#include <dal/ssw/dal_ssw_l2.h>
#include <rtk/dot1x.h>

/* 
 * Symbol Definition 
 */


/* 
 * Data Declaration 
 */
static uint32               dot1x_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         dot1x_sem[RTK_MAX_NUM_OF_UNIT];

const static uint16 dot_1x_port_based_access_control_regidx[] = 
{SSW_DOT_1X_PORT_BASED_ACCESS_CONTROL0r,SSW_DOT_1X_PORT_BASED_ACCESS_CONTROL0r\
,SSW_DOT_1X_PORT_BASED_ACCESS_CONTROL0r,SSW_DOT_1X_PORT_BASED_ACCESS_CONTROL0r\
,SSW_DOT_1X_PORT_BASED_ACCESS_CONTROL0r,SSW_DOT_1X_PORT_BASED_ACCESS_CONTROL0r\
,SSW_DOT_1X_PORT_BASED_ACCESS_CONTROL0r,SSW_DOT_1X_PORT_BASED_ACCESS_CONTROL0r\
,SSW_DOT_1X_PORT_BASED_ACCESS_CONTROL1r,SSW_DOT_1X_PORT_BASED_ACCESS_CONTROL1r\
,SSW_DOT_1X_PORT_BASED_ACCESS_CONTROL1r,SSW_DOT_1X_PORT_BASED_ACCESS_CONTROL1r\
,SSW_DOT_1X_PORT_BASED_ACCESS_CONTROL1r,SSW_DOT_1X_PORT_BASED_ACCESS_CONTROL1r\
,SSW_DOT_1X_PORT_BASED_ACCESS_CONTROL1r,SSW_DOT_1X_PORT_BASED_ACCESS_CONTROL1r\
,SSW_DOT_1X_PORT_BASED_ACCESS_CONTROL2r,SSW_DOT_1X_PORT_BASED_ACCESS_CONTROL2r\
,SSW_DOT_1X_PORT_BASED_ACCESS_CONTROL2r,SSW_DOT_1X_PORT_BASED_ACCESS_CONTROL2r\
,SSW_DOT_1X_PORT_BASED_ACCESS_CONTROL2r,SSW_DOT_1X_PORT_BASED_ACCESS_CONTROL2r\
,SSW_DOT_1X_PORT_BASED_ACCESS_CONTROL2r,SSW_DOT_1X_PORT_BASED_ACCESS_CONTROL2r\
,SSW_DOT_1X_PORT_BASED_ACCESS_CONTROL3r,SSW_DOT_1X_PORT_BASED_ACCESS_CONTROL3r\
,SSW_DOT_1X_PORT_BASED_ACCESS_CONTROL3r,SSW_DOT_1X_PORT_BASED_ACCESS_CONTROL3r};

const static uint16 dot1xEn_fieldidx[] = {SSW_DOT1XPORT_P0_ENf,SSW_DOT1XPORT_P1_ENf,SSW_DOT1XPORT_P2_ENf,SSW_DOT1XPORT_P3_ENf,SSW_DOT1XPORT_P4_ENf,SSW_DOT1XPORT_P5_ENf,SSW_DOT1XPORT_P6_ENf,SSW_DOT1XPORT_P7_ENf,SSW_DOT1XPORT_P8_ENf,SSW_DOT1XPORT_P9_ENf\
                         ,SSW_DOT1XPORT_P10_ENf,SSW_DOT1XPORT_P11_ENf,SSW_DOT1XPORT_P12_ENf,SSW_DOT1XPORT_P13_ENf,SSW_DOT1XPORT_P14_ENf,SSW_DOT1XPORT_P15_ENf,SSW_DOT1XPORT_P16_ENf,SSW_DOT1XPORT_P17_ENf,SSW_DOT1XPORT_P18_ENf,SSW_DOT1XPORT_P19_ENf\
                         ,SSW_DOT1XPORT_P20_ENf,SSW_DOT1XPORT_P21_ENf,SSW_DOT1XPORT_P22_ENf,SSW_DOT1XPORT_P23_ENf,SSW_DOT1XPORT_P24_ENf,SSW_DOT1XPORT_P25_ENf,SSW_DOT1XPORT_P26_ENf,SSW_DOT1XPORT_P27_ENf};

const static uint16 dot1xAuth_fieldidx[] = {SSW_DOT1X_P0_AUTHf,SSW_DOT1X_P1_AUTHf,SSW_DOT1X_P2_AUTHf,SSW_DOT1X_P3_AUTHf,SSW_DOT1X_P4_AUTHf,SSW_DOT1X_P5_AUTHf,SSW_DOT1X_P6_AUTHf,SSW_DOT1X_P7_AUTHf,SSW_DOT1X_P8_AUTHf,SSW_DOT1X_P9_AUTHf\
                         ,SSW_DOT1X_P10_AUTHf,SSW_DOT1X_P11_AUTHf,SSW_DOT1X_P12_AUTHf,SSW_DOT1X_P13_AUTHf,SSW_DOT1X_P14_AUTHf,SSW_DOT1X_P15_AUTHf,SSW_DOT1X_P16_AUTHf,SSW_DOT1X_P17_AUTHf,SSW_DOT1X_P18_AUTHf,SSW_DOT1X_P19_AUTHf\
                         ,SSW_DOT1X_P20_AUTHf,SSW_DOT1X_P21_AUTHf,SSW_DOT1X_P22_AUTHf,SSW_DOT1X_P23_AUTHf,SSW_DOT1X_P24_AUTHf,SSW_DOT1X_P25_AUTHf,SSW_DOT1X_P26_AUTHf,SSW_DOT1X_P27_AUTHf};

const static uint16 dot1xMac_fieldidx[] = {SSW_DOT1XMAC_P0_ENf,SSW_DOT1XMAC_P1_ENf,SSW_DOT1XMAC_P2_ENf,SSW_DOT1XMAC_P3_ENf,SSW_DOT1XMAC_P4_ENf,SSW_DOT1XMAC_P5_ENf,SSW_DOT1XMAC_P6_ENf,SSW_DOT1XMAC_P7_ENf,SSW_DOT1XMAC_P8_ENf,SSW_DOT1XMAC_P9_ENf\
                         ,SSW_DOT1XMAC_P10_ENf,SSW_DOT1XMAC_P11_ENf,SSW_DOT1XMAC_P12_ENf,SSW_DOT1XMAC_P13_ENf,SSW_DOT1XMAC_P14_ENf,SSW_DOT1XMAC_P15_ENf,SSW_DOT1XMAC_P16_ENf,SSW_DOT1XMAC_P17_ENf,SSW_DOT1XMAC_P18_ENf,SSW_DOT1XMAC_P19_ENf\
                         ,SSW_DOT1XMAC_P20_ENf,SSW_DOT1XMAC_P21_ENf,SSW_DOT1XMAC_P22_ENf,SSW_DOT1XMAC_P23_ENf,SSW_DOT1XMAC_P24_ENf,SSW_DOT1XMAC_P25_ENf,SSW_DOT1XMAC_P26_ENf,SSW_DOT1XMAC_P27_ENf};

const static uint16 dot1xPortDirection_fieldidx[] = {SSW_DOT1X_P0_OPDIRf,SSW_DOT1X_P1_OPDIRf,SSW_DOT1X_P2_OPDIRf,SSW_DOT1X_P3_OPDIRf,SSW_DOT1X_P4_OPDIRf,SSW_DOT1X_P5_OPDIRf,SSW_DOT1X_P6_OPDIRf,SSW_DOT1X_P7_OPDIRf,SSW_DOT1X_P8_OPDIRf,SSW_DOT1X_P9_OPDIRf\
                         ,SSW_DOT1X_P10_OPDIRf,SSW_DOT1X_P11_OPDIRf,SSW_DOT1X_P12_OPDIRf,SSW_DOT1X_P13_OPDIRf,SSW_DOT1X_P14_OPDIRf,SSW_DOT1X_P15_OPDIRf,SSW_DOT1X_P16_OPDIRf,SSW_DOT1X_P17_OPDIRf,SSW_DOT1X_P18_OPDIRf,SSW_DOT1X_P19_OPDIRf\
                         ,SSW_DOT1X_P20_OPDIRf,SSW_DOT1X_P21_OPDIRf,SSW_DOT1X_P22_OPDIRf,SSW_DOT1X_P23_OPDIRf,SSW_DOT1X_P24_OPDIRf,SSW_DOT1X_P25_OPDIRf,SSW_DOT1X_P26_OPDIRf,SSW_DOT1X_P27_OPDIRf};

/*
 * Macro Definition
 */
/* semaphore handling */
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


/*
 * Function Declaration
 */

/* Function Name:
 *      dal_ssw_dot1x_init
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
dal_ssw_dot1x_init(uint32 unit)
{
    RT_DBG(LOG_DEBUG, (MOD_DOT1X|MOD_DAL), "unit=%d", unit); 
    
    dot1x_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    dot1x_sem[unit] = osal_sem_mutex_create();
    if (0 == dot1x_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DOT1X|MOD_DAL), "semaphore create failed");
        return RT_ERR_FAILED;
    }
    
    /* set init flag to complete init */
    dot1x_init[unit] = INIT_COMPLETED;
    
    return RT_ERR_OK;
} /* end of dal_ssw_dot1x_init */

/* Function Name:
 *      dal_ssw_dot1x_unauthPacketOper_get
 * Description:
 *      Get the configuration of unauthorized behavior for both 802.1x port and mac based network access control.
 * Input:
 *      unit           - unit id
 * Output:
 *      pUnauth_action - The action of how to handle unauthorized packet 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    The action of how to handle unauthorized packet is as following:
 *    - DROP
 *    - TRAP2CPU
 */
int32
dal_ssw_dot1x_unauthPacketOper_get(uint32 unit, rtk_dot1x_unauth_action_t *pUnauth_action)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DOT1X|MOD_DAL), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pUnauth_action), RT_ERR_NULL_POINTER);
    
    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, SSW_DOT_1X_MAC_BASED_ACCESS_CONTROLr, SSW_DOT1X_UNAUTHBHf, &value)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    DOT1X_SEM_UNLOCK(unit);
    
    switch (value)
    {
        case 0: /* 0b0 */
            *pUnauth_action = DOT1X_ACTION_DROP;
            break;
        
        case 1: /* 0b1 */
            *pUnauth_action = DOT1X_ACTION_TRAP2CPU;
            break;
        
        default:
            return RT_ERR_FAILED;
    }

    RT_DBG(LOG_DEBUG, (MOD_DOT1X|MOD_DAL), "pUnauth_action=%d", *pUnauth_action);
    
    return RT_ERR_OK;
} /* end of dal_ssw_dot1x_unauthPacketOper_get */
    

/* Function Name:
 *      dal_ssw_dot1x_unauthPacketOper_set
 * Description:
 *      Set the configuration of unauthorized behavior for both 802.1x port and mac based network access control.
 * Input:
 *      unit          - unit id
 *      unauth_action - The action of how to handle unauthorized packet 
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_INPUT    - Invalid input parameter
 * Note:
 *    1. User can manually configure how packet should be handled when port or mac based network access control
 *       is unauthorized.
 *
 *    2. The action of how to handle unauthorized packet is as following:
 *       - DROP
 *       - TRAP2CPU
 */
int32
dal_ssw_dot1x_unauthPacketOper_set(uint32 unit, rtk_dot1x_unauth_action_t unauth_action)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DOT1X|MOD_DAL), "unit=%d, unauth_action=%d",
           unit, unauth_action);
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((unauth_action >= DOT1X_ACTION_END), RT_ERR_INPUT);
    
    switch (unauth_action)
    {
        case DOT1X_ACTION_DROP:
            value = 0;
            break;
        
        case DOT1X_ACTION_TRAP2CPU:
            value = 1;
            break;
        
        case DOT1X_ACTION_TO_GUEST_VLAN:
        default:
            return RT_ERR_INPUT;
    }

    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_write(unit, SSW_DOT_1X_MAC_BASED_ACCESS_CONTROLr, SSW_DOT1X_UNAUTHBHf, &value)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    DOT1X_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_dot1x_unauthPacketOper_set */

/* Function Name:
 *      dal_ssw_dot1x_eapolFrame2CpuEnable_get
 * Description:
 *      Get the configuration of 802.1x EAPOL frame trap to CPU.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - The status of EAPOL frame trap to CPU
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    The status of EAPOL frame trap to CPU is as following:
 *    - DISABLED
 *    - ENABLED
 */
int32
dal_ssw_dot1x_eapolFrame2CpuEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DOT1X|MOD_DAL), "unit=%d", unit);
       
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, SSW_DOT_1X_MAC_BASED_ACCESS_CONTROLr, SSW_DOT1X_EAPOLTRAPf, pEnable)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    DOT1X_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DOT1X|MOD_DAL), "pEnable=%d", *pEnable);
    
    return RT_ERR_OK;
} /* end of dal_ssw_dot1x_eapolFrame2CpuEnable_get */

/* Function Name:
 *      dal_ssw_dot1x_eapolFrame2CpuEnable_set
 * Description:
 *      Set the configuration of 802.1x EAPOL frame trap to CPU.
 * Input:
 *      unit   - unit id
 *      enable - The status of EAPOL frame trap to CPU
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_INPUT    - Invalid input parameter
 * Note:
 *    1. To support 802.1x authentication functionality, EAPOL frame (ether type = 888E) has to
 *       be trapped to CPU.
 *
 *    2. The status of EAPOL frame trap to CPU is as following:
 *       - DISABLED
 *       - ENABLED
 */
int32
dal_ssw_dot1x_eapolFrame2CpuEnable_set(uint32 unit, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DOT1X|MOD_DAL), "unit=%d, enable=%d", unit, enable);
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(enable >= RTK_ENABLE_END, RT_ERR_INPUT);
    
    DOT1X_SEM_LOCK(unit);
    
    /* program value to CHIP*/
    if ((ret = reg_field_write(unit, SSW_DOT_1X_MAC_BASED_ACCESS_CONTROLr, SSW_DOT1X_EAPOLTRAPf, &enable)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    DOT1X_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_dot1x_eapolFrame2CpuEnable_set */

/* Function Name:
 *      dal_ssw_dot1x_portBasedEnable_get
 * Description:
 *      Get the status of 802.1x port-based network access control on a specific port.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pEnable - The status of 802.1x port-based network access control.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_NULL_POINTER  - NULL pointer
  * Note:
 *    1. If a port is 802.1x port based network access control "enabled",
 *       it should be authenticated so packets from that port wont be dropped or trapped to CPU.
 *
 *    2. The status of 802.1x port-based network access control is as following:
 *       - DISABLED
 *       - ENABLED
 */
int32
dal_ssw_dot1x_portBasedEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DOT1X|MOD_DAL), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, dot_1x_port_based_access_control_regidx[port], dot1xEn_fieldidx[port], pEnable)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    DOT1X_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DOT1X|MOD_DAL), "pEnable=%d", *pEnable);
    
    return RT_ERR_OK;
} /* end of dal_ssw_dot1x_portBasedEnable_get */

/* Function Name:
 *      dal_ssw_dot1x_portBasedEnable_set
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
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_INPUT    - Invalid input parameter
 * Note:
 *    1. If a port is 802.1x port based network access control "enabled",
 *       it should be authenticated so packets from that port wont be dropped or trapped to CPU.
 *
 *    2. The status of 802.1x port-based network access control is as following:
 *       - DISABLED
 *       - ENABLED
 */
int32
dal_ssw_dot1x_portBasedEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DOT1X|MOD_DAL), "unit=%d, port=%d, enable=%d",
           unit, port, enable);
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(enable >= RTK_ENABLE_END, RT_ERR_INPUT);
    
    DOT1X_SEM_LOCK(unit);
    
    /* program value to CHIP*/
    if ((ret = reg_field_write(unit, dot_1x_port_based_access_control_regidx[port], dot1xEn_fieldidx[port], &enable)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    DOT1X_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_dot1x_portBasedEnable_set */

/* Function Name:
 *      dal_ssw_dot1x_portBasedAuthStatus_get
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
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    The authenticated status of 802.1x port-based network access control is as following:
 *    - UNAUTH
 *    - AUTH
 */
int32
dal_ssw_dot1x_portBasedAuthStatus_get(
    uint32                    unit,
    rtk_port_t                port,
    rtk_dot1x_auth_status_t   *pPort_auth)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DOT1X|MOD_DAL), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pPort_auth), RT_ERR_NULL_POINTER);
    
    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, dot_1x_port_based_access_control_regidx[port], dot1xAuth_fieldidx[port], pPort_auth)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    DOT1X_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DOT1X|MOD_DAL), "pPort_auth=%d", *pPort_auth);
    
    return RT_ERR_OK;
} /* end of dal_ssw_dot1x_portBasedAuthStatus_get */

/* Function Name:
 *      dal_ssw_dot1x_portBasedAuthStatus_set
 * Description:
 *      Set the authenticated status of 802.1x port-based network access control on a specific
 *      port.
 * Input:
 *      unit      - unit id  
 *      port      - port id
 *      port_auth - The status of 802.1x port-based network access control is authenticated or unauthenticated.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_INPUT    - Invalid input parameter
 * Note:
 *    The authenticated status of 802.1x port-based network access control is as following:
 *    - UNAUTH
 *    - AUTH
 */
int32
dal_ssw_dot1x_portBasedAuthStatus_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_dot1x_auth_status_t port_auth)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DOT1X|MOD_DAL), "unit=%d, port=%d, port_auth=%d",
           unit, port, port_auth);
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(port_auth >= AUTH_STATUS_END, RT_ERR_INPUT);
    
    DOT1X_SEM_LOCK(unit);
    
    /* program value to CHIP*/
    if ((ret = reg_field_write(unit, dot_1x_port_based_access_control_regidx[port], dot1xAuth_fieldidx[port], &port_auth)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    DOT1X_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_dot1x_portBasedAuthStatus_set */

/* Function Name:
 *      dal_ssw_dot1x_portBasedDirection_get
 * Description:
 *      Get the operate controlled direction of 802.1x Port-based network access control on a specific port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pDirection - The operate controlled direction  of 802.1x Port-based network access control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    None.
 */
int32
dal_ssw_dot1x_portBasedDirection_get(uint32 unit, rtk_port_t port, uint32 *pDirection)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DOT1X|MOD_DAL), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pDirection), RT_ERR_NULL_POINTER);
    
    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, dot_1x_port_based_access_control_regidx[port], dot1xPortDirection_fieldidx[port], pDirection)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    DOT1X_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DOT1X|MOD_DAL), "pDirection=%d", *pDirection);
    
    return RT_ERR_OK;
} /* end of dal_ssw_dot1x_PortBasedDirection_get */

/* Function Name:
 *      dal_ssw_dot1x_portBasedDirection_set
 * Description:
 *      Set the operate controlled direction of 802.1x Port-based network access control on a specific port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      direction - The operate controlled direction of 802.1x Port-based network access control
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_INPUT    - Invalid input parameter
 * Note:
 *    None.
 */
int32
dal_ssw_dot1x_portBasedDirection_set(uint32 unit, rtk_port_t port, uint32 direction)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DOT1X|MOD_DAL), "unit=%d, port=%d, direction=%d",
           unit, port, direction);
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(direction >= DIRECTION_END, RT_ERR_INPUT);
    
    DOT1X_SEM_LOCK(unit);
    
    /* program value to CHIP*/
    if ((ret = reg_field_write(unit, dot_1x_port_based_access_control_regidx[port], dot1xPortDirection_fieldidx[port], &direction)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    DOT1X_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_dot1x_PortBasedDirection_set */

/* Function Name:
 *      dal_ssw_dot1x_macBasedEnable_get
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
 *      RT_ERR_UNIT_ID      - Invalid unit id
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
dal_ssw_dot1x_macBasedEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DOT1X|MOD_DAL), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, SSW_DOT_1X_MAC_BASED_ACCESS_CONTROLr, dot1xMac_fieldidx[port], pEnable)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    DOT1X_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DOT1X|MOD_DAL), "pEnable=%d", *pEnable);
    
    return RT_ERR_OK;
} /* end of dal_ssw_dot1x_macBasedEnable_get */

/* Function Name:
 *      dal_ssw_dot1x_macBasedEnable_set
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
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_INPUT    - Invalid input parameter
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
dal_ssw_dot1x_macBasedEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DOT1X|MOD_DAL), "unit=%d, port=%d, enable=%d",
           unit, port, enable);
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(enable >= AUTH_STATUS_END, RT_ERR_INPUT);
    
    DOT1X_SEM_LOCK(unit);
    
    /* program value to CHIP*/
    if ((ret = reg_field_write(unit, SSW_DOT_1X_MAC_BASED_ACCESS_CONTROLr, dot1xMac_fieldidx[port], &enable)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    DOT1X_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_dot1x_macBasedEnable_set */

/* Function Name:
 *      dal_ssw_dot1x_macBasedAuthMac_add
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
 *      RT_ERR_UNIT_ID           - Invalid unit id
 *      RT_ERR_PORT_ID           - Invalid port id
 *      RT_ERR_VLAN_ID           - Invalid vlan id
 *      RT_ERR_MAC               - Invalid MAC address
 *      RT_ERR_L2_NO_EMPTY_ENTRY - No empty entry in L2 table
 * Note:
 *      None.
 */
int32
dal_ssw_dot1x_macBasedAuthMac_add(
    uint32      unit,
    rtk_port_t  port,
    rtk_vlan_t  vid,
    rtk_mac_t   *pAuth_mac)
{    
    return dal_ssw_l2_authAddr_add(unit, vid, pAuth_mac, port, FALSE);
} /* end of dal_ssw_dot1x_macBasedAuthMac_add */

/* Function Name:
 *      dal_ssw_dot1x_macBasedAuthMac_del
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
 *      RT_ERR_UNIT_ID           - Invalid unit id
 *      RT_ERR_PORT_ID           - Invalid port id
 *      RT_ERR_VLAN_ID           - Invalid vlan id
 *      RT_ERR_MAC               - Invalid MAC address
 *      RT_ERR_L2_NO_EMPTY_ENTRY - No empty entry in L2 table
 * Note:
 *      None.
 */
int32
dal_ssw_dot1x_macBasedAuthMac_del(
    uint32      unit,
    rtk_port_t  port,
    rtk_vlan_t  vid,
    rtk_mac_t   *pAuth_mac)
{
    return dal_ssw_l2_addr_del(unit, vid, pAuth_mac);
} /* end of dal_ssw_dot1x_macBasedAuthMac_del */

/* Function Name:
 *      dal_ssw_dot1x_macBasedDirection_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    The operate controlled direction of 802.1x mac-based network access control is as following:
 *    - BOTH
 *    - IN
 */
int32
dal_ssw_dot1x_macBasedDirection_get(uint32 unit, rtk_dot1x_direction_t *pMac_direction)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DOT1X|MOD_DAL), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pMac_direction), RT_ERR_NULL_POINTER);
    
    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, SSW_DOT_1X_MAC_BASED_ACCESS_CONTROLr, SSW_DOT1XMACOPDIRf, pMac_direction)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    DOT1X_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DOT1X|MOD_DAL), "pMac_direction=%d", *pMac_direction);
    
    return RT_ERR_OK;
} /* end of dal_ssw_dot1x_macBasedDirection_get */
    

/* Function Name:
 *      dal_ssw_dot1x_macBasedDirection_set
 * Description:
 *      Set the operate controlled direction 802.1x mac-based network access control on system.
 *     
 * Input:
 *      unit          - unit id  
 *      mac_direction - The controlled direction of 802.1x mac-based network access control is BOTH
 *                       or IN.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_INPUT    - Invalid input parameter
 * Note:
 *    The operate controlled direction of 802.1x mac-based network access control is as following:
 *    - BOTH
 *    - IN
 */
int32
dal_ssw_dot1x_macBasedDirection_set(uint32 unit, rtk_dot1x_direction_t mac_direction)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DOT1X|MOD_DAL), "unit=%d, mac_direction=%d",
           unit, mac_direction);
    
    /* check Init status */
    RT_INIT_CHK(dot1x_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((mac_direction >= DIRECTION_END), RT_ERR_INPUT);
    
    DOT1X_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_write(unit, SSW_DOT_1X_MAC_BASED_ACCESS_CONTROLr, SSW_DOT1XMACOPDIRf, &mac_direction)) != RT_ERR_OK)
    {
        DOT1X_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    DOT1X_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_dot1x_macBasedDirection_set */
