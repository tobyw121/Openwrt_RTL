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
 * $Revision: 32771 $
 * $Date: 2012-09-18 15:46:47 +0800 (Tue, 18 Sep 2012) $
 *
 * Purpose : Definition those public Port APIs and its data type in the SDK.
 * 
 * Feature : The file have include the following module and sub-modules
 *           1) Port
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
#include <osal/thread.h>
#include <osal/lib.h>
#include <osal/memory.h>
#include <osal/time.h>
#include <hal/chipdef/allmem.h>
#include <hal/chipdef/ssw/rtk_ssw_table_struct.h>
#include <hal/mac/reg.h>
#include <hal/mac/mem.h>
#include <hal/chipdef/allreg.h>
#include <hal/chipdef/ssw/rtk_ssw_reg_struct.h>
#include <hal/common/miim.h>
#include <hal/common/halctrl.h>
#include <dal/dal_common.h>
#include <dal/ssw/dal_ssw_port.h>
#include <dal/ssw/dal_ssw_vlan.h>
#include <rtk/port.h>
#include <rtk/default.h>
#include <drv/intr/intr.h>

/* 
 * Symbol Definition 
 */
typedef struct dal_ssw_mac_info_s {
    uint8   admin_enable[RTK_MAX_NUM_OF_PORTS];
    uint8   green_enable[RTK_MAX_NUM_OF_PORTS];
} dal_ssw_mac_info_t;

typedef struct dal_ssw_phy_info_s {
    uint8   force_mode_speed[RTK_MAX_NUM_OF_PORTS];
    uint8   force_mode_duplex[RTK_MAX_NUM_OF_PORTS];
    uint8   force_mode_flowControl[RTK_MAX_NUM_OF_PORTS];
    uint8   auto_mode_pause[RTK_MAX_NUM_OF_PORTS];
    uint8   auto_mode_asy_pause[RTK_MAX_NUM_OF_PORTS];
    uint8   cross_over_mode[RTK_MAX_NUM_OF_PORTS];
} dal_ssw_phy_info_t;


/* 
 * Data Declaration 
 */
static uint32               port_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         port_sem[RTK_MAX_NUM_OF_UNIT];
static dal_ssw_mac_info_t   *pMac_info[RTK_MAX_NUM_OF_UNIT];
static dal_ssw_phy_info_t   *pPhy_info[RTK_MAX_NUM_OF_UNIT];
static dal_link_change_callback_f   link_change_callback_f[RTK_MAX_NUM_OF_UNIT];


const static uint16 portMacControl_regidx[] = {SSW_PORT_0_1_MAC_CONTROLr, SSW_PORT_0_1_MAC_CONTROLr,
                                               SSW_PORT_2_3_MAC_CONTROLr, SSW_PORT_2_3_MAC_CONTROLr,
                                               SSW_PORT_4_5_MAC_CONTROLr, SSW_PORT_4_5_MAC_CONTROLr,
                                               SSW_PORT_6_7_MAC_CONTROLr, SSW_PORT_6_7_MAC_CONTROLr,
                                               SSW_PORT_8_9_MAC_CONTROLr, SSW_PORT_8_9_MAC_CONTROLr,
                                               SSW_PORT_10_11_MAC_CONTROLr, SSW_PORT_10_11_MAC_CONTROLr,
                                               SSW_PORT_12_13_MAC_CONTROLr, SSW_PORT_12_13_MAC_CONTROLr,
                                               SSW_PORT_14_15_MAC_CONTROLr, SSW_PORT_14_15_MAC_CONTROLr,
                                               SSW_PORT_16_17_MAC_CONTROLr, SSW_PORT_16_17_MAC_CONTROLr,
                                               SSW_PORT_18_19_MAC_CONTROLr, SSW_PORT_18_19_MAC_CONTROLr,
                                               SSW_PORT_20_21_MAC_CONTROLr, SSW_PORT_20_21_MAC_CONTROLr,
                                               SSW_PORT_22_23_MAC_CONTROLr, SSW_PORT_22_23_MAC_CONTROLr,
                                               SSW_PORT_24_25_MAC_CONTROLr, SSW_PORT_24_25_MAC_CONTROLr,
                                               SSW_PORT_26_27_MAC_CONTROLr, SSW_PORT_26_27_MAC_CONTROLr,
                                               SSW_PORT_28_MAC_CONTROLr};

const static uint16 portSerdesControl_regidx[] = {SSW_SERDES_0_CONTROL0r, SSW_SERDES_0_CONTROL0r,
                                               SSW_SERDES_1_CONTROL0r, SSW_SERDES_1_CONTROL0r,
                                               SSW_SERDES_2_CONTROL0r, SSW_SERDES_2_CONTROL0r,
                                               SSW_SERDES_3_CONTROL0r, SSW_SERDES_3_CONTROL0r,
                                               SSW_SERDES_4_CONTROL0r, SSW_SERDES_4_CONTROL0r,
                                               SSW_SERDES_5_CONTROL0r, SSW_SERDES_5_CONTROL0r,
                                               SSW_SERDES_6_CONTROL0r, SSW_SERDES_6_CONTROL0r,
                                               SSW_SERDES_7_CONTROL0r, SSW_SERDES_7_CONTROL0r,
                                               SSW_SERDES_8_CONTROL0r, SSW_SERDES_8_CONTROL0r,
                                               SSW_SERDES_9_CONTROL0r, SSW_SERDES_9_CONTROL0r,
                                               SSW_SERDES_10_CONTROL0r, SSW_SERDES_10_CONTROL0r,
                                               SSW_SERDES_11_CONTROL0r, SSW_SERDES_11_CONTROL0r,
                                               SSW_INTRA_SERDES_0_CONTROL0r, SSW_INTRA_SERDES_0_CONTROL0r,
                                               SSW_INTRA_SERDES_1_CONTROL0r, SSW_INTRA_SERDES_1_CONTROL0r};
                                               
const static uint16 portMacSpeedStatus_regidx[] = {SSW_MAC_SPEED_STATUS0r, SSW_MAC_SPEED_STATUS0r, SSW_MAC_SPEED_STATUS0r, SSW_MAC_SPEED_STATUS0r, SSW_MAC_SPEED_STATUS0r, SSW_MAC_SPEED_STATUS0r, SSW_MAC_SPEED_STATUS0r, SSW_MAC_SPEED_STATUS0r
                                                  ,SSW_MAC_SPEED_STATUS0r, SSW_MAC_SPEED_STATUS0r, SSW_MAC_SPEED_STATUS0r, SSW_MAC_SPEED_STATUS0r, SSW_MAC_SPEED_STATUS0r, SSW_MAC_SPEED_STATUS0r, SSW_MAC_SPEED_STATUS0r, SSW_MAC_SPEED_STATUS0r
                                                  ,SSW_MAC_SPEED_STATUS1r, SSW_MAC_SPEED_STATUS1r, SSW_MAC_SPEED_STATUS1r, SSW_MAC_SPEED_STATUS1r, SSW_MAC_SPEED_STATUS1r, SSW_MAC_SPEED_STATUS1r, SSW_MAC_SPEED_STATUS1r, SSW_MAC_SPEED_STATUS1r
                                                  ,SSW_MAC_SPEED_STATUS1r, SSW_MAC_SPEED_STATUS1r, SSW_MAC_SPEED_STATUS1r, SSW_MAC_SPEED_STATUS1r, SSW_MAC_SPEED_STATUS1r};

const static uint16 portIsolationContro_regidx[] = {SSW_PORT_ISOLATION_CONTROL0r, SSW_PORT_ISOLATION_CONTROL1r, SSW_PORT_ISOLATION_CONTROL2r, SSW_PORT_ISOLATION_CONTROL3r, SSW_PORT_ISOLATION_CONTROL4r, SSW_PORT_ISOLATION_CONTROL5r, SSW_PORT_ISOLATION_CONTROL6r, SSW_PORT_ISOLATION_CONTROL7r, SSW_PORT_ISOLATION_CONTROL8r, SSW_PORT_ISOLATION_CONTROL9r,
                                                    SSW_PORT_ISOLATION_CONTROL10r, SSW_PORT_ISOLATION_CONTROL11r, SSW_PORT_ISOLATION_CONTROL12r, SSW_PORT_ISOLATION_CONTROL13r, SSW_PORT_ISOLATION_CONTROL14r, SSW_PORT_ISOLATION_CONTROL15r, SSW_PORT_ISOLATION_CONTROL16r, SSW_PORT_ISOLATION_CONTROL17r, SSW_PORT_ISOLATION_CONTROL18r, SSW_PORT_ISOLATION_CONTROL19r,
                                                    SSW_PORT_ISOLATION_CONTROL20r, SSW_PORT_ISOLATION_CONTROL21r, SSW_PORT_ISOLATION_CONTROL22r, SSW_PORT_ISOLATION_CONTROL23r, SSW_PORT_ISOLATION_CONTROL24r, SSW_PORT_ISOLATION_CONTROL25r, SSW_PORT_ISOLATION_CONTROL26r, SSW_PORT_ISOLATION_CONTROL27r, SSW_PORT_ISOLATION_CONTROL28r};

const static uint16 portIntraSerdesContro0_regidx[] = {SSW_INTRA_SERDES_0_FIBER_CONTROL0r, SSW_INTRA_SERDES_1_FIBER_CONTROL0r};
const static uint16 portIntraSerdesContro2_regidx[] = {SSW_INTRA_SERDES_0_FIBER_CONTROL2r, SSW_INTRA_SERDES_1_FIBER_CONTROL2r};

const static uint16 portTxRxEnable_fieldidx[] = {SSW_EN_TXRX_P0f, SSW_EN_TXRX_P1f, SSW_EN_TXRX_P2f, SSW_EN_TXRX_P3f, SSW_EN_TXRX_P4f, SSW_EN_TXRX_P5f, SSW_EN_TXRX_P6f, SSW_EN_TXRX_P7f, SSW_EN_TXRX_P8f, SSW_EN_TXRX_P9f,
                                                 SSW_EN_TXRX_P10f, SSW_EN_TXRX_P11f, SSW_EN_TXRX_P12f, SSW_EN_TXRX_P13f, SSW_EN_TXRX_P14f, SSW_EN_TXRX_P15f, SSW_EN_TXRX_P16f, SSW_EN_TXRX_P17f, SSW_EN_TXRX_P18f, SSW_EN_TXRX_P19f,
                                                 SSW_EN_TXRX_P20f, SSW_EN_TXRX_P21f, SSW_EN_TXRX_P22f, SSW_EN_TXRX_P23f, SSW_EN_TXRX_P24f, SSW_EN_TXRX_P25f, SSW_EN_TXRX_P26f, SSW_EN_TXRX_P27f, SSW_EN_TXRX_P28f};
const static uint16 portIsolation_fieldidx[] = {SSW_P0ISOf, SSW_P1ISOf, SSW_P2ISOf, SSW_P3ISOf, SSW_P4ISOf, SSW_P5ISOf, SSW_P6ISOf, SSW_P7ISOf, SSW_P8ISOf, SSW_P9ISOf,
                                                SSW_P10ISOf, SSW_P11ISOf, SSW_P12ISOf, SSW_P13ISOf, SSW_P14ISOf, SSW_P15ISOf, SSW_P16ISOf, SSW_P17ISOf, SSW_P18ISOf, SSW_P19ISOf,
                                                SSW_P20ISOf, SSW_P21ISOf, SSW_P22ISOf, SSW_P23ISOf, SSW_P24ISOf, SSW_P25ISOf, SSW_P26ISOf, SSW_P27ISOf, SSW_P28ISOf};
#if 0 /* The function is move to dal_esw_diag.c */
const static uint16 portRLB_fieldidx[] = { SSW_EN_MAC_RLB_P0f, SSW_EN_MAC_RLB_P1f, SSW_EN_MAC_RLB_P2f, SSW_EN_MAC_RLB_P3f, SSW_EN_MAC_RLB_P4f, SSW_EN_MAC_RLB_P5f, SSW_EN_MAC_RLB_P6f, SSW_EN_MAC_RLB_P7f, SSW_EN_MAC_RLB_P8f, SSW_EN_MAC_RLB_P9f,
                                           SSW_EN_MAC_RLB_P10f, SSW_EN_MAC_RLB_P11f, SSW_EN_MAC_RLB_P12f, SSW_EN_MAC_RLB_P13f, SSW_EN_MAC_RLB_P14f, SSW_EN_MAC_RLB_P15f, SSW_EN_MAC_RLB_P16f, SSW_EN_MAC_RLB_P17f, SSW_EN_MAC_RLB_P18f, SSW_EN_MAC_RLB_P19f, 
                                           SSW_EN_MAC_RLB_P20f, SSW_EN_MAC_RLB_P21f, SSW_EN_MAC_RLB_P22f, SSW_EN_MAC_RLB_P23f, SSW_EN_MAC_RLB_P24f, SSW_EN_MAC_RLB_P25f, SSW_EN_MAC_RLB_P26f, SSW_EN_MAC_RLB_P27f, SSW_EN_MAC_RLB_P28f};
const static uint16 portLLB_fieldidx[] = { SSW_EN_MAC_LLB_P0f, SSW_EN_MAC_LLB_P1f, SSW_EN_MAC_LLB_P2f, SSW_EN_MAC_LLB_P3f, SSW_EN_MAC_LLB_P4f, SSW_EN_MAC_LLB_P5f, SSW_EN_MAC_LLB_P6f, SSW_EN_MAC_LLB_P7f, SSW_EN_MAC_LLB_P8f, SSW_EN_MAC_LLB_P9f,
                                           SSW_EN_MAC_LLB_P10f, SSW_EN_MAC_LLB_P11f, SSW_EN_MAC_LLB_P12f, SSW_EN_MAC_LLB_P13f, SSW_EN_MAC_LLB_P14f, SSW_EN_MAC_LLB_P15f, SSW_EN_MAC_LLB_P16f, SSW_EN_MAC_LLB_P17f, SSW_EN_MAC_LLB_P18f, SSW_EN_MAC_LLB_P19f, 
                                           SSW_EN_MAC_LLB_P20f, SSW_EN_MAC_LLB_P21f, SSW_EN_MAC_LLB_P22f, SSW_EN_MAC_LLB_P23f, SSW_EN_MAC_LLB_P24f, SSW_EN_MAC_LLB_P25f, SSW_EN_MAC_LLB_P26f, SSW_EN_MAC_LLB_P27f, SSW_EN_MAC_RLB_P28f};
#endif
const static uint16 portBackpressure_fieldidx[] = { SSW_EN_BP_P0f, SSW_EN_BP_P1f, SSW_EN_BP_P2f, SSW_EN_BP_P3f, SSW_EN_BP_P4f, SSW_EN_BP_P5f, SSW_EN_BP_P6f, SSW_EN_BP_P7f, SSW_EN_BP_P8f, SSW_EN_BP_P9f,
                                                    SSW_EN_BP_P10f, SSW_EN_BP_P11f, SSW_EN_BP_P12f, SSW_EN_BP_P13f, SSW_EN_BP_P14f, SSW_EN_BP_P15f, SSW_EN_BP_P16f, SSW_EN_BP_P17f, SSW_EN_BP_P18f, SSW_EN_BP_P19f, 
                                                    SSW_EN_BP_P20f, SSW_EN_BP_P21f, SSW_EN_BP_P22f, SSW_EN_BP_P23f, SSW_EN_BP_P24f, SSW_EN_BP_P25f, SSW_EN_BP_P26f, SSW_EN_BP_P27f, SSW_EN_BP_P28f};


/*
 * Macro Definition
 */
/* vlan semaphore handling */
#define PORT_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(port_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_DAL|MOD_PORT), "semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)
#define PORT_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(port_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_DAL|MOD_PORT), "semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)


/* 
 * Function Declaration 
 */
static int32 _dal_ssw_port_init_config(uint32 unit);

#if defined(CONFIG_SDK_DRIVER_NIC_USER_MODE)
static void _dal_ssw_port_linkChange_isr(uint32 unit, void *isr_param);
#endif /* CONFIG_SDK_DRIVER_NIC_USER_MODE */

/* Module Name    : port     */
/* Sub-module Name: Global */

/* Function Name:
 *      dal_ssw_port_init
 * Description:
 *      Initialize port module of the specified device.
 * Input:
 *      unit          - unit id
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
dal_ssw_port_init(uint32 unit)
{
    int32   ret;

    
    port_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    port_sem[unit] = osal_sem_mutex_create();
    if (0 == port_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PORT), "semaphore create failed");
        return RT_ERR_FAILED;
    }
    
    pMac_info[unit] = (dal_ssw_mac_info_t *)osal_alloc(sizeof(dal_ssw_mac_info_t));
    if (NULL == pMac_info[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PORT), "memory allocate failed");
        return RT_ERR_FAILED;
    }        
    
    osal_memset(pMac_info[unit], 0, sizeof(dal_ssw_mac_info_t));

    pPhy_info[unit] = (dal_ssw_phy_info_t *)osal_alloc(sizeof(dal_ssw_phy_info_t));
    if (NULL == pPhy_info[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PORT), "memory allocate failed");
        osal_free(pMac_info[unit]);
        pMac_info[unit] = NULL;
        return RT_ERR_FAILED;
    }        

    osal_memset(pPhy_info[unit], 0, sizeof(dal_ssw_phy_info_t));
    
    /* init callback function for link change */
    link_change_callback_f[unit] = 0;
    
    /* set init flag to complete init */
    port_init[unit] = INIT_COMPLETED;    
    
    if (( ret = _dal_ssw_port_init_config(unit)) != RT_ERR_OK)
    {
        port_init[unit] = INIT_NOT_COMPLETED;
        osal_free(pMac_info[unit]);
        pMac_info[unit] = NULL;
        osal_free(pPhy_info[unit]);
        pPhy_info[unit] = NULL;
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "Port default configuration init failed");
        return ret;
    }
    

#if defined(CONFIG_SDK_DRIVER_NIC_USER_MODE)
    /* linkscan callback handler */
    {
        /* enable interrupt */
        if (( ret = drv_intr_enable_set(unit, LINK_CHANGE_INTR)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "LinkScan interrupt enable failed");
            return ret;
        }
        
        
        /* register callback */
        if (( ret = drv_intr_link_stat_register(unit, _dal_ssw_port_linkChange_isr)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "LinkScan interrupt handler installed failed");
            return ret;
        }
        
    }
#endif
    
    return RT_ERR_OK;
}/* end of dal_ssw_port_init */

/* Function Name:
 *      dal_ssw_port_link_get
 * Description:
 *      Get the link status of the specific port
 * Input:
 *      unit                  - unit id
 *      port                  - port id
 * Output:
 *      pStatus              - pointer to the link status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_PORT_ID         - invalid port id
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 * Note:
 *      The link status of the port is as following:
 *      - LINKDOWN  
 *      - LINKUP    
 */
int32
dal_ssw_port_link_get(uint32 unit, rtk_port_t port, rtk_port_linkStatus_t *pStatus)
{
    int32   ret;
    rtk_portmask_t  portmask;
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pStatus), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    /* get value from CHIP*/
    reg_field_read(unit, SSW_MAC_LINK_STATUSr, SSW_LINK_STA_Pf, &portmask.bits[0]);
    if ((ret = reg_field_read(unit, SSW_MAC_LINK_STATUSr, SSW_LINK_STA_Pf, &portmask.bits[0])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    
    /* translate chip's value to definition */
    if (RTK_PORTMASK_IS_PORT_SET(portmask, port))
    {
        *pStatus = PORT_LINKUP;
    } 
    else 
    {
        *pStatus = PORT_LINKDOWN;
    }
    
    return RT_ERR_OK;
}/* end of dal_ssw_port_link_get */

/* Function Name:
 *      dal_ssw_port_linkMedia_get
 * Description:
 *      Get the link status with media information of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pStatus - pointer to the link status
 *      pMedia  - pointer to the media type
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      (1) The link status of the port is as following:
 *          - LINKDOWN  
 *          - LINKUP    
 *      (2) The media type of the port is as following:
 *          - PORT_MEDIA_COPPER
 *          - PORT_MEDIA_FIBER
 *      (3) When the link status is link-down, the return media should be ignored.
 */
int32
dal_ssw_port_linkMedia_get(uint32 unit, rtk_port_t port, rtk_port_linkStatus_t *pStatus, rtk_port_media_t *pMedia)
{
    uint32  value;
    int32   ret;
    rtk_portmask_t  portmask;
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pStatus), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pMedia), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    /* get value from CHIP*/
    reg_field_read(unit, SSW_MAC_LINK_STATUSr, SSW_LINK_STA_Pf, &portmask.bits[0]);
    if ((ret = reg_field_read(unit, SSW_MAC_LINK_STATUSr, SSW_LINK_STA_Pf, &portmask.bits[0])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    
    /* translate chip's value to definition */
    if (RTK_PORTMASK_IS_PORT_SET(portmask, port))
    {
        *pStatus = PORT_LINKUP;
        dal_ssw_port_phyReg_get(unit, port, 0, 15, &value);
        if (value & 0x8000)
            *pMedia = PORT_MEDIA_FIBER;
        else
            *pMedia = PORT_MEDIA_COPPER;
    } 
    else 
    {
        *pStatus = PORT_LINKDOWN;
        *pMedia = PORT_MEDIA_COPPER;
    }
    
    return RT_ERR_OK;
}/* end of dal_ssw_port_link_get */

/* Function Name:
 *      dal_ssw_port_txEnable_get
 * Description:
 *      Get the TX enable status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to the port TX status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      The TX enable status of the port is as following:
 *      - DISABLE   
 *      - ENABLE    
 */
int32
dal_ssw_port_txEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value[1];

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
       
    PORT_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, (uint32)portMacControl_regidx[port], (uint32)portTxRxEnable_fieldidx[port], &value[0])) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    if (value[0] & 0x1)
        *pEnable = ENABLED;
    else
        *pEnable = DISABLED;

    PORT_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
}/* end of dal_ssw_port_txEnable_get */

/* Function Name:
 *      dal_ssw_port_txEnable_set
 * Description:
 *      Set the TX enable status of the specific port
 * Input:
 *      unit           - unit id
 *      port           - port id
 *      enable        - enable status of TX
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      The TX enable status of the port is as following:
 *      - DISABLE   
 *      - ENABLE    
 */
int32
dal_ssw_port_txEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  value[1];

    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, enable=%d", unit, port, enable);
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable != DISABLED && enable != ENABLED), RT_ERR_INPUT);
       
    PORT_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, (uint32)portMacControl_regidx[port], (uint32)portTxRxEnable_fieldidx[port], &value[0])) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    
    if (ENABLED == enable)
    {
        BITMAP_SET(value, 0);
    } 
    else 
    {
        BITMAP_CLEAR(value, 0);
    }
    
    /* programming value on CHIP*/
    if ((ret = reg_field_write(unit, (uint32)portMacControl_regidx[port], (uint32)portTxRxEnable_fieldidx[port], &value[0])) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    PORT_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
}/* end of dal_ssw_port_txEnable_set */

/* Function Name:
 *      dal_ssw_port_rxEnable_get
 * Description:
 *      Get the RX enable status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to the port RX status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      The RX enable status of the port is as following:
 *      - DISABLE   
 *      - ENABLE    
 */
int32
dal_ssw_port_rxEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value[1];

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
       
    PORT_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, (uint32)portMacControl_regidx[port], (uint32)portTxRxEnable_fieldidx[port], &value[0])) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    if ((value[0] >> 1) & 0x1)
        *pEnable = ENABLED;
    else
        *pEnable = DISABLED;

    PORT_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
}/* end of dal_ssw_port_rxEnable_get */

/* Function Name:
 *      dal_ssw_port_rxEnable_set
 * Description:
 *      Set the RX enable status of the specific port
 * Input:
 *      unit            - unit id
 *      port            - port id
 *      enable         - enable status of RX
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID   - invalid unit id
 *      RT_ERR_PORT_ID   - invalid port id
 *      RT_ERR_INPUT     - invalid input parameter
 * Note:
 *      The RX enable status of the port is as following:
 *      - DISABLE   
 *      - ENABLE    
 */
int32
dal_ssw_port_rxEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  value[1];

    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, enable=%d", unit, port, enable);
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable != DISABLED && enable != ENABLED), RT_ERR_INPUT);
    
    PORT_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, (uint32)portMacControl_regidx[port], (uint32)portTxRxEnable_fieldidx[port], &value[0])) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    
    if (ENABLED == enable)
    {
        BITMAP_SET(value, 1);
    } 
    else
    {
        BITMAP_CLEAR(value, 1);
    }
    
    /* programming value on CHIP*/
    if ((ret = reg_field_write(unit, (uint32)portMacControl_regidx[port], (uint32)portTxRxEnable_fieldidx[port], &value[0])) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    PORT_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
}/* end of dal_ssw_port_rxEnable_set */

/* Function Name:
 *      dal_ssw_port_specialCongest_set
 * Description:
 *      Set the congest seconds of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      second - congest timer (seconds)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      None
 */
int32
dal_ssw_port_specialCongest_set(uint32 unit, rtk_port_t port, uint32 second)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, second=%d", unit, port, second);
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    PORT_SEM_LOCK(unit);
    
    /* set value from CHIP*/
    if ((ret = reg_array_field_write(unit, SSW_PORT_GMAC_CONTROLr, port, REG_ARRAY_INDEX_NONE, SSW_CONGEST_SUSTAIN_Tf, &second)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    
    PORT_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_port_specialCongest_set */

/* Function Name:
 *      dal_ssw_port_speedDuplex_get
 * Description:
 *      Get the negotiated port speed and duplex status of the specific port
 * Input:
 *      unit                  - unit id
 *      port                  - port id
 * Output:                    
 *      pSpeed               - pointer to the port speed
 *      pDuplex              - pointer to the port duplex
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_PORT_ID         - invalid port id
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 *      RT_ERR_PORT_LINKDOWN   - link down port status
 * Note:
 *      1. The speed type of the port is as following:
 *         - PORT_SPEED_10M
 *         - PORT_SPEED_100M
 *         - PORT_SPEED_1000M
 *
 *      2. The duplex mode of the port is as following:
 *         - HALF_DUPLEX
 *         - FULL_DUPLEX
 */
int32
dal_ssw_port_speedDuplex_get(
    uint32            unit,
    rtk_port_t        port,
    rtk_port_speed_t  *pSpeed,
    rtk_port_duplex_t *pDuplex)
{
    int32   ret;
    uint32  speed; 
    uint32  duplex[1];
    rtk_port_linkStatus_t  link_status;

    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", unit, port);
       
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pSpeed), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pDuplex), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    /* Check Link status */
    if ((ret = dal_ssw_port_link_get(unit, port, &link_status)) != RT_ERR_OK)
    {   
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    
    /* if link status is down, should not process anymore and return error */
    if (PORT_LINKDOWN == link_status)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "port link down");
        return RT_ERR_PORT_LINKDOWN;
    }
       
    PORT_SEM_LOCK(unit);
    
    /* get speed value from CHIP*/
    if ((ret = reg_field_read(unit, (uint32)portMacSpeedStatus_regidx[port], SSW_SPD_STA_Pf, &speed)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    
    /* get duplex value from CHIP*/
    if ((ret = reg_field_read(unit, SSW_MAC_DUPLEX_STATUSr, SSW_DUP_STA_Pf, &duplex[0])) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    
    PORT_SEM_UNLOCK(unit);
    
    /* translate chip's value to definition */
    if( BITMAP_IS_SET(duplex, port))
    {
        *pDuplex = PORT_FULL_DUPLEX;
    }
    else
    {
        *pDuplex = PORT_HALF_DUPLEX;
    }
    
    /* extract port's speed value */
    speed = (speed >> ((port&BITMASK_4B)*2)) & BITMASK_2B;
    switch (speed)
    {
        case 0x0:
            *pSpeed = PORT_SPEED_10M;
            break;
        case 0x1:
            *pSpeed = PORT_SPEED_100M;
            break;
        case 0x2:
            *pSpeed = PORT_SPEED_1000M;
            break;
        case 0x3:
            *pSpeed = PORT_SPEED_1000M;
            break;
        default:
            return RT_ERR_FAILED;
    }        
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pSpeed=%d, pDuplex=%d", 
           *pSpeed, *pDuplex);
    
    return RT_ERR_OK;
}/* end of dal_ssw_port_speedDuplex_get */

/* Function Name:
 *      dal_ssw_port_flowctrl_get
 * Description:
 *      Get the negotiated flow control status of the specific port
 * Input:
 *      unit         - unit id
 *      port         - port id
 * Output:
 *      pTxStatus     - pointer to the negotiation result of the Tx flow control 
 *      pRxStatus     - pointer to the negotiation result of the Rx flow control 
 * Return:
 *      RT_ERR_OK 
 *      RT_ERR_FAILED  
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_PORT_ID         - invalid port id
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer    
 *      RT_ERR_PORT_LINKDOWN   - link down port status
 * Note:
 *      None  
 */ 
int32
dal_ssw_port_flowctrl_get(
    uint32            unit,
    rtk_port_t        port,   
    uint32            *pTxStatus,
    uint32            *pRxStatus)
{
    int32   ret;
    uint32  rxPause[1], txPause[1];
    rtk_port_linkStatus_t  link_status;

    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", unit, port);  
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pTxStatus), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pRxStatus), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    /* Check Link status */
    if ((ret = dal_ssw_port_link_get(unit, port, &link_status)) != RT_ERR_OK)
    {   
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    
    /* if link status is down, should not process anymore and return error */
    if (PORT_LINKDOWN == link_status)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "port link down");
        return RT_ERR_PORT_LINKDOWN;
    }
    
    PORT_SEM_LOCK(unit);
    
    /* get speed value from CHIP*/
    if ((ret = reg_field_read(unit, SSW_MAC_TX_PAUSE_STATUSr, SSW_TX_PAUSE_STA_Pf, &txPause[0])) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    
    /* get duplex value from CHIP*/
    if ((ret = reg_field_read(unit, SSW_MAC_RX_PAUSE_STATUSr, SSW_RX_PAUSE_STA_Pf, &rxPause[0])) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    
    PORT_SEM_UNLOCK(unit);
    
    /* translate chip's value to definition */
    if( BITMAP_IS_SET(txPause, port))
    {
        *pTxStatus = ENABLED;
    }
    else
    {
        *pTxStatus = DISABLED;
    }
    
    if( BITMAP_IS_SET(rxPause, port))
    {
        *pRxStatus = ENABLED;
    }
    else
    {
        *pRxStatus = DISABLED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pTxStatus=%d, pRxStatus=%d", 
           *pTxStatus, *pRxStatus);    
    
    return RT_ERR_OK;
}/* end of dal_ssw_port_flowctrl_get */

/* Function Name:
 *      dal_ssw_port_phyAutoNegoEnable_get
 * Description:
 *      Get PHY ability of the specific port
 * Input:
 *      unit                 - unit id
 *      port                 - port id 
 * Output:                   
 *      pEnable             - pointer to PHY auto negotiation status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_PORT_ID        - invalid port id
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_ssw_port_phyAutoNegoEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    uint32  value = 0;
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    /* Fiber port read from intra serdes registers, not PHY */
    if (HAL_IS_SERDES_PORT(unit, port))
    {
        uint32  ge_fiber_base = 0;

        ge_fiber_base = HAL_GET_MIN_SERDES_PORT(unit);
        PORT_SEM_LOCK(unit);
        if ((ret = reg_field_read(unit, (uint32)portIntraSerdesContro0_regidx[port-ge_fiber_base], SSW_FIB_ANENf
                            , &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }

        if (DISABLED == value)
            *pEnable = DISABLED;
        else
            *pEnable = ENABLED;
        PORT_SEM_UNLOCK(unit);
        return ret;
    }

    PORT_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = phy_autoNegoEnable_get(unit, port, pEnable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }
            
    PORT_SEM_UNLOCK(unit);
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pEnable=%d", *pEnable); 
    
    return RT_ERR_OK;
}/* end of dal_ssw_port_phyAutoNegoEnable_get */

/* Function Name:
 *      dal_ssw_port_phyAutoNegoEnable_set
 * Description:
 *      Set PHY ability of the specific port
 * Input:
 *      unit                 - unit id
 *      port                 - port id
 *      enable               - enable PHY auto negotiation
 * Output:                   
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_PORT_ID        - invalid port id
 *      RT_ERR_INPUT          - input parameter out of range
 * Note:
 *      1. ENABLED : switch to PHY auto negotiation mode
 *         DISABLED: switch to PHY force mode
 *      2. Once the abilities of both auto-nego and force mode are set,
 *         you can freely swtich the mode without calling ability setting API again
 */
int32
dal_ssw_port_phyAutoNegoEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    uint32  value = 0;
    int32   ret;
    rtk_port_phy_ability_t ability;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, enable=%d", 
           unit, port, enable);     
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(enable >= RTK_ENABLE_END, RT_ERR_INPUT);
    
    /* Fiber port write to intra serdes registers, not PHY */
    if (HAL_IS_SERDES_PORT(unit, port))
    {
        uint32  ge_fiber_base = 0;

        ge_fiber_base = HAL_GET_MIN_SERDES_PORT(unit);
        PORT_SEM_LOCK(unit);
        if ((ret = reg_read(unit, (uint32)portIntraSerdesContro0_regidx[port-ge_fiber_base]
                            , &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }

        /* Configure AN bit depend on input 'enable' argument */
        value &= ~(9 << 1);
        if (ENABLED == enable)
            value |= (0x9 << 1);
        /* Configure AN_RESTART bit */
        value |= (12 << 1);

        if ((ret = reg_write(unit, (uint32)portIntraSerdesContro0_regidx[port-ge_fiber_base]
                            , &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }

        PORT_SEM_UNLOCK(unit);
        return ret;
    }
    
    PORT_SEM_LOCK(unit);

    if (ENABLED == enable)
    {
        if ((ret = phy_autoNegoAbility_get(unit, port, &ability)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }
        
        ability.FC = pPhy_info[unit]->auto_mode_pause[port];
        ability.AsyFC = pPhy_info[unit]->auto_mode_asy_pause[port];
        
        if ((ret = phy_autoNegoAbility_set(unit, port, &ability)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }            
    }

    if ((ret = phy_autoNegoEnable_set(unit, port, enable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }    
                  
    if (DISABLED == enable)
    {
        if ((ret = phy_duplex_set(unit, port, pPhy_info[unit]->force_mode_duplex[port])) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }
        
        if ((ret = phy_speed_set(unit, port, pPhy_info[unit]->force_mode_speed[port])) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }
       
        if ((ret = phy_autoNegoAbility_get(unit, port, &ability)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }
            
        ability.FC = pPhy_info[unit]->force_mode_flowControl[port]; /* ENABLED */
        ability.AsyFC = pPhy_info[unit]->force_mode_flowControl[port]; /* ENABLED */
            
        if ((ret = phy_autoNegoAbility_set(unit, port, &ability)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }            
        
        /* E0005371 */
        if (ENABLED == pMac_info[unit]->admin_enable[port])
        {
            /* Turn off and then turn on the power of port so the partner could detect the speed/duplex change */ 
            if ((ret = phy_enable_set(unit, port, DISABLED)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
                return ret;
            } 
                   
            osal_time_usleep(200000);
    
            if ((ret = phy_enable_set(unit, port, ENABLED)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
                return ret;
            }
        }
    }

    PORT_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
}/* end of dal_ssw_port_phyAutoNegoEnable_set */

/* Function Name:
 *      dal_ssw_port_phyAutoNegoAbility_get
 * Description:
 *      Get PHY auto negotiation ability of the specific port
 * Input:
 *      unit                 - unit id
 *      port                 - port id
 * Output:                   
 *      pAbility            - pointer to the PHY ability
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_PORT_ID        - invalid port id
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_ssw_port_phyAutoNegoAbility_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_port_phy_ability_t  *pAbility)
{
    uint32  value;
    int32   ret; 
    rtk_enable_t  enable;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", 
           unit, port);  
        
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pAbility), RT_ERR_NULL_POINTER);
    
    /* Fiber port read from intra serdes registers, not PHY */
    if (HAL_IS_SERDES_PORT(unit, port))
    {
        uint32  ge_fiber_base = 0;

        ge_fiber_base = HAL_GET_MIN_SERDES_PORT(unit);
        PORT_SEM_LOCK(unit);
        if ((ret = reg_read(unit, (uint32)portIntraSerdesContro0_regidx[port-ge_fiber_base]
                            , &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }

        enable = (value >> 12) & 0x1;

        osal_memset(pAbility, 0, sizeof(rtk_port_phy_ability_t));
        pAbility->Full_1000 = 1;

        if ((ret = reg_field_read(unit, (uint32)portIntraSerdesContro2_regidx[port-ge_fiber_base], SSW_TX_CFG_REGf
                            , &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }

        if (DISABLED == enable)
        {
            pAbility->FC    = pPhy_info[unit]->auto_mode_pause[port];
            pAbility->AsyFC = pPhy_info[unit]->auto_mode_asy_pause[port];
        }
        else
        {
            pAbility->FC    = (value >> 7) & 0x1;
            pAbility->AsyFC = (value >> 8) & 0x1;
        }
        
        PORT_SEM_UNLOCK(unit);
        return ret;
    }

    if ((ret = dal_ssw_port_phyAutoNegoEnable_get(unit, port, &enable)) != RT_ERR_OK)
    {    
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }
        
    osal_memset(pAbility, 0, sizeof(rtk_port_phy_ability_t));
    
    PORT_SEM_LOCK(unit);
    
    if ((ret = phy_autoNegoAbility_get(unit, port, pAbility)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);    
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }
       
    if (DISABLED == enable)
    {
        pAbility->FC    = pPhy_info[unit]->auto_mode_pause[port];
        pAbility->AsyFC = pPhy_info[unit]->auto_mode_asy_pause[port];
    }
           
    PORT_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Half_10=%d, Full_10=%d, \
           Half_100=%d, Full_100=%d, Half_1000=%d, Full_1000=%d, FC=%d, AsyFC=%d",
           pAbility->Half_10, pAbility->Full_10, pAbility->Half_100, pAbility->Full_100, 
           pAbility->Half_1000, pAbility->Full_1000, pAbility->FC, pAbility->AsyFC);     
    
    return RT_ERR_OK;
}/* end of dal_ssw_port_phyAutoNegoAbility_get */

/* Function Name:
 *      dal_ssw_port_phyAutoNegoAbility_set
 * Description:
 *      Set PHY auto negotiation ability of the specific port
 * Input:
 *      unit               - unit id
 *      port               - port id
 *      pAbility          - pointer to the PHY ability
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. You can set these abilities no matter which mode PHY currently stays on
 */
int32
dal_ssw_port_phyAutoNegoAbility_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_port_phy_ability_t  *pAbility)
{
    uint32  reg0, reg4;
    int32   ret;     
    rtk_enable_t    enable;    
    
    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", unit, port);
    
    /* check Init status */   
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pAbility), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(HAL_IS_FE_PORT(unit, port) && pAbility->Half_1000, RT_ERR_PHY_AUTO_ABILITY);
    RT_PARAM_CHK(HAL_IS_FE_PORT(unit, port) && pAbility->Full_1000, RT_ERR_PHY_AUTO_ABILITY);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Half_10=%d, Full_10=%d, Half_100=%d, Full_100=%d, \
           Half_1000=%d, Full_1000=%d, FC=%d, AsyFC=%d", 
           pAbility->Half_10, pAbility->Full_10, pAbility->Half_100, pAbility->Full_100, 
           pAbility->Half_1000, pAbility->Full_1000, pAbility->FC, pAbility->AsyFC);   
    
    /* Fiber port read from intra serdes registers, not PHY */
    if (HAL_IS_SERDES_PORT(unit, port))
    {
        uint32  ge_fiber_base = 0;

        ge_fiber_base = HAL_GET_MIN_SERDES_PORT(unit);
        PORT_SEM_LOCK(unit);
        if ((ret = reg_read(unit, (uint32)portIntraSerdesContro0_regidx[port-ge_fiber_base]
                            , &reg0)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }

        enable = (reg0 >> 12) & 0x1;

        if (DISABLED == enable)
        {
            pPhy_info[unit]->auto_mode_pause[port] = pAbility->FC;
            pPhy_info[unit]->auto_mode_asy_pause[port] = pAbility->AsyFC;
            pAbility->FC = pPhy_info[unit]->force_mode_flowControl[port];
            pAbility->AsyFC = pPhy_info[unit]->force_mode_flowControl[port];
        }

        if ((ret = reg_read(unit, (uint32)portIntraSerdesContro2_regidx[port-ge_fiber_base]
                        , &reg4)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }

        reg4 &= ~(0xF << 5);
        reg4 |= (pAbility->AsyFC << 8) | (pAbility->FC << 7);
        reg4 |= (pAbility->Half_1000 << 6) | (pAbility->Full_1000 << 5);

        if ((ret = reg_write(unit, (uint32)portIntraSerdesContro2_regidx[port-ge_fiber_base]
                        , &reg4)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }

        if (ENABLED == enable)
        {
            reg0 |= (1 << 9);
            if ((ret = reg_write(unit, (uint32)portIntraSerdesContro0_regidx[port-ge_fiber_base]
                                , &reg0)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
                return ret;
            }
        }

        PORT_SEM_UNLOCK(unit);
        return ret;
    }

    if ((ret = dal_ssw_port_phyAutoNegoEnable_get(unit, port, &enable)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    if (DISABLED == enable)
    {
        pPhy_info[unit]->auto_mode_pause[port] = pAbility->FC;
        pPhy_info[unit]->auto_mode_asy_pause[port] = pAbility->AsyFC;
        pAbility->FC = pPhy_info[unit]->force_mode_flowControl[port];
        pAbility->AsyFC = pPhy_info[unit]->force_mode_flowControl[port];
    }
    
    PORT_SEM_LOCK(unit);

    if ((ret = phy_autoNegoAbility_set(unit, port, pAbility)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);    
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }   

    if (ENABLED == enable)
    {
        pPhy_info[unit]->auto_mode_pause[port] = pAbility->FC;
        pPhy_info[unit]->auto_mode_asy_pause[port] = pAbility->AsyFC;
    }
        
    PORT_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
}/* end of dal_ssw_port_phyAutoNegoAbility_set */

/* Function Name:
 *      dal_ssw_port_phyForceModeAbility_get
 * Description:
 *      Get PHY ability status of the specific port
 * Input:
 *      unit                 - unit id
 *      port                 - port id
 * Output:                   
 *      pSpeed              - pointer to the port speed
 *      pDuplex             - pointer to the port duplex
 *      pFlowControl        - pointer to the flow control enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_PORT_ID        - invalid port id
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_ssw_port_phyForceModeAbility_get(
    uint32              unit,
    rtk_port_t          port,
    rtk_port_speed_t    *pSpeed,
    rtk_port_duplex_t   *pDuplex,
    rtk_enable_t        *pFlowControl)
{
    uint32  value = 0;
    int32   ret;
    rtk_enable_t    enable;
    rtk_port_phy_ability_t  ability;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", unit, port);  
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pSpeed), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pDuplex), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pFlowControl), RT_ERR_NULL_POINTER);
    
    /* Fiber port read from intra serdes registers, not PHY */
    if (HAL_IS_SERDES_PORT(unit, port))
    {
        uint32  ge_fiber_base = 0;

        ge_fiber_base = HAL_GET_MIN_SERDES_PORT(unit);
        *pSpeed = PORT_SPEED_1000M;
        *pDuplex = PORT_FULL_DUPLEX;
        PORT_SEM_LOCK(unit);
        if ((ret = reg_field_read(unit, (uint32)portIntraSerdesContro2_regidx[port-ge_fiber_base], SSW_TX_CFG_REGf
                            , &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }

        if (((value >> 7) & 0x3) == 0)
            *pFlowControl = DISABLED;
        else
            *pFlowControl = ENABLED;
        PORT_SEM_UNLOCK(unit);
        return ret;
    }

    if ((ret = dal_ssw_port_phyAutoNegoEnable_get(unit, port, &enable)) != RT_ERR_OK)
    {    
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }
    
    PORT_SEM_LOCK(unit);
    
    if (ENABLED == enable)
    {
        *pSpeed = pPhy_info[unit]->force_mode_speed[port];
        *pDuplex = pPhy_info[unit]->force_mode_duplex[port];
        *pFlowControl = pPhy_info[unit]->force_mode_flowControl[port];
    }
    else
    {
        if ((ret = phy_speed_get(unit, port, pSpeed)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }

        if ((ret = phy_duplex_get(unit, port, pDuplex)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }

        if ((ret = phy_autoNegoAbility_get(unit, port, &ability)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }
        
        *pFlowControl = ability.FC;
    }
            
    PORT_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pSpeed=%d, pDuplex=%d \
           pFlowControl=%d", *pSpeed, *pDuplex, *pFlowControl);    
    
    return RT_ERR_OK;
}/* end of dal_ssw_port_phyForceModeAbility_get */

/* Function Name:
 *      dal_ssw_port_phyForceModeAbility_set
 * Description:
 *      Set the port speed/duplex mode/pause/asy_pause in the PHY force mode
 * Input:
 *      unit                  - unit id
 *      port                  - port id
 *      speed                 - port speed
 *      duplex                - port duplex mode
 *      flowControl           - enable flow control
 * Output:                    
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_PORT_ID         - invalid port id
 *      RT_ERR_PHY_SPEED       - invalid PHY speed setting         
 *      RT_ERR_PHY_DUPLEX      - invalid PHY duplex setting         
 *      RT_ERR_INPUT           - invalid input parameter
 * Note:
 *      1. You can set these abilities no matter which mode PHY currently stays on
 *
 *      2. The speed type of the port is as following:
 *         - PORT_SPEED_10M
 *         - PORT_SPEED_100M
 *
 *      3. The duplex mode of the port is as following:
 *         - HALF_DUPLEX
 *         - FULL_DUPLEX
 */
int32
dal_ssw_port_phyForceModeAbility_set(
    uint32              unit,
    rtk_port_t          port,
    rtk_port_speed_t    speed,
    rtk_port_duplex_t   duplex,
    rtk_enable_t        flowControl)
{
    uint32  value = 0;
    int32   ret;
    rtk_enable_t    enable;
    rtk_port_phy_ability_t ability;

    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, speed=%d, duplex=%d \
           flowControl=%d", unit, port, speed, duplex, flowControl);    
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(speed >= PORT_SPEED_END, RT_ERR_PHY_SPEED);
    RT_PARAM_CHK(HAL_IS_FE_PORT(unit, port) && speed >= PORT_SPEED_1000M, RT_ERR_PHY_SPEED);
    RT_PARAM_CHK((!HAL_IS_GE_COMBO_PORT(unit, port)) && (!HAL_IS_SERDES_PORT(unit, port)) && speed == PORT_SPEED_1000M, RT_ERR_CHIP_NOT_SUPPORTED);
    RT_PARAM_CHK(duplex >= PORT_DUPLEX_END, RT_ERR_PHY_DUPLEX);
    RT_PARAM_CHK(flowControl >= RTK_ENABLE_END, RT_ERR_INPUT);

    /* Fiber port write to intra serdes registers, not PHY */
    if (HAL_IS_SERDES_PORT(unit, port))
    {
        uint32  ge_fiber_base = 0;

        RT_PARAM_CHK(speed != PORT_SPEED_1000M, RT_ERR_CHIP_NOT_SUPPORTED);
        RT_PARAM_CHK(duplex != PORT_FULL_DUPLEX, RT_ERR_CHIP_NOT_SUPPORTED);

        ge_fiber_base = HAL_GET_MIN_SERDES_PORT(unit);
        PORT_SEM_LOCK(unit);
        if ((ret = reg_field_read(unit, (uint32)portIntraSerdesContro2_regidx[port-ge_fiber_base], SSW_TX_CFG_REGf
                            , &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }

        value &= ~(0x3 << 7);
        if (ENABLED == flowControl)
            value |= (0x3 << 7);

        if ((ret = reg_field_write(unit, (uint32)portIntraSerdesContro2_regidx[port-ge_fiber_base], SSW_TX_CFG_REGf
                            , &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }

        PORT_SEM_UNLOCK(unit);
        return ret;
    }

    if ((ret = dal_ssw_port_phyAutoNegoEnable_get(unit, port, &enable)) != RT_ERR_OK)
    {    
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }
    
    PORT_SEM_LOCK(unit);
    if (DISABLED == enable)
    {
        if ((ret = phy_speed_set(unit, port, speed)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }

        if ((ret = phy_duplex_set(unit, port, duplex)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }

        if ((ret = phy_autoNegoAbility_get(unit, port, &ability)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }               
        
        if (ENABLED == flowControl)
        {
            ability.FC = ENABLED;
            ability.AsyFC = ENABLED;
        }
        else
        {
            ability.FC = DISABLED;
            ability.AsyFC = DISABLED;
        }

        if ((ret = phy_autoNegoAbility_set(unit, port, &ability)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }     
               
        /* E0005371 */
        if (ENABLED == pMac_info[unit]->admin_enable[port])
        {
            /* Turn off and then turn on the power of port so the partner could detect the speed/duplex change */ 
            if ((ret = phy_enable_set(unit, port, DISABLED)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
                return ret;
            } 
                   
            osal_time_usleep(200000);
    
            if ((ret = phy_enable_set(unit, port, ENABLED)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
                return ret;
            }
        }
        /* End of E0005371 */               
    }
    pPhy_info[unit]->force_mode_speed[port] = speed;
    pPhy_info[unit]->force_mode_duplex[port] = duplex;
    pPhy_info[unit]->force_mode_flowControl[port] = flowControl;
    PORT_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
}/* end of dal_ssw_port_phyForceModeAbility_set */

/* Function Name:
 *      dal_ssw_port_phyReg_get
 * Description:
 *      Get PHY register data of the specific port
 * Input:
 *      unit                - unit id
 *      port                - port id
 *      page                - page id
 *      reg                 - reg id
 * Output:                  
 *      pData              - pointer to the PHY reg data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_PHY_PAGE_ID   - invalid page id
 *      RT_ERR_PHY_REG_ID    - invalid reg id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_ssw_port_phyReg_get(
    uint32              unit, 
    rtk_port_t          port,
    uint32              page,
    rtk_port_phy_reg_t  reg,
    uint32              *pData)
{
    int32 ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, page=0x%x, reg=0x%x", 
           unit, port, page, reg);   
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    //RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);
    
    /* Check Link status */
    ret = hal_miim_read(unit, port, page, reg, pData);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pData=0x%x", *pData);        
    
    return ret;
}/* end of dal_ssw_port_phyReg_get */

/* Function Name:
 *      dal_ssw_port_phyReg_set
 * Description:
 *      Set PHY register data of the specific port
 * Input:
 *      unit               - unit id
 *      port               - port id
 *      page               - page id
 *      reg                - reg id
 *      data               - reg data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid page id
 *      RT_ERR_PHY_REG_ID   - invalid reg id
 * Note:
 *      None
 */
int32
dal_ssw_port_phyReg_set(
    uint32              unit, 
    rtk_port_t          port,
    uint32              page,
    rtk_port_phy_reg_t  reg,
    uint32              data)
{
    int32 ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, page=0x%x, reg=0x%x \
           data=0x%x", unit, port, page, reg, data);    
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    //RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    /* Check Link status */
    ret = hal_miim_write(unit, port, page, reg, data);
    
    return ret;
}/* end of dal_ssw_port_phyReg_set */

/* Function Name:
 *      dal_ssw_port_cpuPortId_get
 * Description:
 *      Get CPU port id of the specific unit
 * Input:
 *      unit                 - unit id    
 * Output:                   
 *      pPort               - pointer to CPU port id
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_ssw_port_cpuPortId_get(uint32 unit, rtk_port_t *pPort)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d", unit);   

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pPort), RT_ERR_NULL_POINTER);

    *pPort = HAL_GET_CPU_PORT(unit);
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pPort=%d", *pPort);    
        
    return RT_ERR_OK;
}/* end of dal_ssw_port_cpuPortId_get */

/* Function Name:
 *      dal_ssw_port_isolation_get
 * Description:
 *      Get the portmask of the port isolation
 * Input:
 *      unit                - unit id
 *      port                - port id
 * Output:                  
 *      pPortmask          - pointer to the portmask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      1. Default value of each port is 1
 *      2. Enable port isolation in the certain ports if relative portmask bits are set to 1
 */
int32
dal_ssw_port_isolation_get(uint32 unit, rtk_port_t port, rtk_portmask_t *pPortmask)
{
    int32 ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pPortmask), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
       
    PORT_SEM_LOCK(unit);
    
    /* get speed value from CHIP*/
    if ((ret = reg_field_read(unit, (uint32)portIsolationContro_regidx[port], (uint32)portIsolation_fieldidx[port]
                        , &pPortmask->bits[0])) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    
    PORT_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pPortmask=0x%x", pPortmask->bits[0]);    
    
    return RT_ERR_OK;
}/* end of dal_ssw_port_isolation_get */

/* Function Name:
 *      dal_ssw_port_isolation_set
 * Description:
 *      Set the portmask of the port isolation
 * Input:
 *      unit            - unit id
 *      port            - port id
 *      portmask        - pointer to the portmask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID   - invalid unit id
 *      RT_ERR_PORT_ID   - invalid port id
 *      RT_ERR_PORT_MASK - invalid port mask
 * Note:
 *      1. Default value of each port is 1
 *      2. Enable port isolation in the certain ports if relative portmask bits are set to 1
 */
int32
dal_ssw_port_isolation_set(uint32 unit, rtk_port_t port, rtk_portmask_t portmask)
{
    int32 ret;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, portmask=0x%x", 
           unit, port, portmask.bits[0]);
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
        
    PORT_SEM_LOCK(unit);
    
    /* get speed value from CHIP*/
    if ((ret = reg_field_write(unit, (uint32)portIsolationContro_regidx[port], (uint32)portIsolation_fieldidx[port]
                        , &portmask.bits[0])) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    
    PORT_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
}/* end of dal_ssw_port_isolation_set */

/* Function Name:
 *      dal_ssw_port_isolation_add
 * Description:
 *      Add an isolation port to the certain port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      iso_port      - isolation port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      1. Default value of each port is 1
 *      2. Port and iso_port will be isolated when this API is called
 *      3. The iso_port to the relative portmask bit will be set to 1
 */
int32
dal_ssw_port_isolation_add(uint32 unit, rtk_port_t port, rtk_port_t iso_port)
{
    int32 ret;
    rtk_portmask_t  portmask;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, iso_port=%d", 
           unit, port, iso_port);    
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, iso_port), RT_ERR_PORT_ID);
    
    osal_memset(&portmask, 0, sizeof(rtk_portmask_t));
    
    ret = dal_ssw_port_isolation_get(unit, port, &portmask);
    if (RT_ERR_OK != ret)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    
    RTK_PORTMASK_PORT_SET(portmask, iso_port);
    
    ret = dal_ssw_port_isolation_set(unit, port, portmask);
    if (RT_ERR_OK != ret)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    
    return RT_ERR_OK;
}/* end of dal_ssw_port_isolation_add */

/* Function Name:
 *      dal_ssw_port_isolation_del
 * Description:
 *      Delete an existing isolation port of the certain port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      iso_port      - isolation port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      1. Default value of each port is 1
 *      2. Isolated status between the port and the iso_port is removed when this API is called
 *      3. The iso_port to the relative portmask bit will be set to 0
 */
int32
dal_ssw_port_isolation_del(uint32 unit, rtk_port_t port, rtk_port_t iso_port)
{
    int32 ret;
    rtk_portmask_t  portmask;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, iso_port=%d", 
           unit, port, iso_port); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, iso_port), RT_ERR_PORT_ID);
    
    osal_memset(&portmask, 0, sizeof(rtk_portmask_t));
    
    ret = dal_ssw_port_isolation_get(unit, port, &portmask);
    if (RT_ERR_OK != ret)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    
    RTK_PORTMASK_PORT_CLEAR(portmask, iso_port);
    
    ret = dal_ssw_port_isolation_set(unit, port, portmask);
    if (RT_ERR_OK != ret)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    
    return RT_ERR_OK;
}/* end of dal_ssw_port_isolation_del */

/* Function Name:
 *      dal_ssw_port_phyComboPortMedia_get
 * Description:
 *      Get PHY port media of the specific port
 * Input:
 *      unit                 - unit id
 *      port                 - port id
 * Output:                   
 *      pMedia              - pointer to the port media
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_PORT_ID        - invalid port id
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_ssw_port_phyComboPortMedia_get(uint32 unit, rtk_port_t port, rtk_port_media_t *pMedia)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", 
           unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pMedia), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);

    /* get value from CHIP*/
    if ((ret = phy_media_get(unit, port, pMedia)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_ssw_port_phyComboPortMedia_get */

/* Function Name:
 *      dal_ssw_port_phyComboPortMedia_set
 * Description:
 *      Set PHY port media of the specific port
 * Input:
 *      unit               - unit id
 *      port               - port id
 *      pMedia            - pointer to the port media
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. You can set these port media which mode PHY currently stays on
 */
int32
dal_ssw_port_phyComboPortMedia_set(uint32 unit, rtk_port_t port, rtk_port_media_t media)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, media=%d", 
           unit, port, media); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((media >= PORT_MEDIA_END), RT_ERR_INPUT);

    PORT_SEM_LOCK(unit);

    /* set value from CHIP*/
    if ((ret = phy_media_set(unit, port, media)) != RT_ERR_OK && ret != RT_ERR_CHIP_NOT_SUPPORTED)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return ret;
} /* end of dal_ssw_port_phyComboPortMedia_set */

/* Function Name:
 *      dal_ssw_port_adminEnable_get
 * Description:
 *      Get port admin status of the specific port
 * Input:
 *      unit               - unit id
 *      port               - port id
 *      pEnable           - pointer to the port admin status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_ssw_port_adminEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", 
           unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);

    *pEnable = pMac_info[unit]->admin_enable[port];

    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pEnable=%d", *pEnable); 
    
    return RT_ERR_OK;
} /* end of dal_ssw_port_adminEnable_get */

/* Function Name:
 *      dal_ssw_port_adminEnable_set
 * Description:
 *      Set port admin configuration of the specific port
 * Input:
 *      unit               - unit id
 *      port               - port id
 *      enable             - port admin configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_ssw_port_adminEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  value[1];  

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, port admin=%d", 
           unit, port, enable); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable != DISABLED && enable != ENABLED), RT_ERR_INPUT);

    PORT_SEM_LOCK(unit);
            
    if (enable == pMac_info[unit]->admin_enable[port])
    {
        /* no change and prevent to configure to chip */
        PORT_SEM_UNLOCK(unit);
        return RT_ERR_OK;
    }

    if (ENABLED == enable)
    {
        /* get value from CHIP*/
        if ((ret = reg_field_read(unit, (uint32)portMacControl_regidx[port], (uint32)portTxRxEnable_fieldidx[port], &value[0])) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }

        BITMAP_SET(value, 1);
        BITMAP_SET(value, 0);

        /* programming value on CHIP*/
        if ((ret = reg_field_write(unit, (uint32)portMacControl_regidx[port], (uint32)portTxRxEnable_fieldidx[port], &value[0])) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }        
    }

    if (!HAL_IS_SERDES_PORT(unit, port))
    {
        if ((ret = phy_enable_set(unit, port, enable)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

    if (DISABLED == enable)    
    {
        /* get value from CHIP*/
        if ((ret = reg_field_read(unit, (uint32)portMacControl_regidx[port], (uint32)portTxRxEnable_fieldidx[port], &value[0])) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }

        BITMAP_CLEAR(value, 1);
        BITMAP_CLEAR(value, 0);

        /* programming value on CHIP*/
        if ((ret = reg_field_write(unit, (uint32)portMacControl_regidx[port], (uint32)portTxRxEnable_fieldidx[port], &value[0])) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

    pMac_info[unit]->admin_enable[port] = enable;
    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;

} /* end of dal_ssw_port_adminEnable_set */

#if 0 /* The function is move to dal_esw_diag.c */
/* Function Name:
 *      dal_ssw_port_macRemoteLoopbackEnable_get
 * Description:
 *      Get the mac remote loopback enable status of the specific port
 * Input:
 *      unit           - unit id
 *      port           - port id
 * Output:
 *      pEnable        - pointer to the enable status of mac remote loopback
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      1. The mac remote loopback enable status of the port is as following:
 *         - DISABLE   
 *         - ENABLE
 *      2. Remote loopback is used to loopback packet RX to switch core back to the outer interface.
 */
int32
dal_ssw_port_macRemoteLoopbackEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", 
           unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
   
    PORT_SEM_LOCK(unit);

    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, (uint32)portMacControl_regidx[port], (uint32)portRLB_fieldidx[port], pEnable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pEnable=%d", *pEnable); 

    return RT_ERR_OK;
} /* end of dal_ssw_port_macRemoteLoopbackEnable_get */

/* Function Name:
 *      dal_ssw_port_macRemoteLoopbackEnable_set
 * Description:
 *      Set the mac remote loopback enable status of the specific port
 * Input:
 *      unit           - unit id
 *      port           - port id
 *      enable        - enable status of mac remote loopback
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      1. The mac remote loopback enable status of the port is as following:
 *         - DISABLE   
 *         - ENABLE
 *      2. Remote loopback is used to loopback packet RX to switch core back to the outer interface.  
 */
int32
dal_ssw_port_macRemoteLoopbackEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, enable=%d", 
           unit, port, enable); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable != DISABLED && enable != ENABLED), RT_ERR_INPUT);

    PORT_SEM_LOCK(unit);

    /* get value from CHIP*/
    if ((ret = reg_field_write(unit, (uint32)portMacControl_regidx[port], (uint32)portRLB_fieldidx[port], &enable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_ssw_port_macRemoteLoopbackEnable_set */

/* Function Name:
 *      dal_ssw_port_macLocalLoopbackEnable_get
 * Description:
 *      Get the mac local loopback enable status of the specific port
 * Input:
 *      unit           - unit id
 *      port           - port id
 * Output:
 *      pEnable       - - pointer to the enable status of mac local loopback
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      1. The mac local loopback enable status of the port is as following:
 *         - DISABLE   
 *         - ENABLE
 *      2. Local loopback is used to loopback packet TX from switch core back to switch core.
 */
int32
dal_ssw_port_macLocalLoopbackEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", 
           unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);

    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, (uint32)portMacControl_regidx[port], (uint32)portLLB_fieldidx[port], pEnable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pEnable=%d", *pEnable); 

    return RT_ERR_OK;
} /* end of dal_ssw_port_macLocalLoopbackEnable_get */

/* Function Name:
 *      dal_ssw_port_macLocalLoopbackEnable_set
 * Description:
 *      Set the mac local loopback enable status of the specific port
 * Input:
 *      unit           - unit id
 *      port           - port id
 *      enable        - enable status of mac local loopback
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      1. The mac local loopback enable status of the port is as following:
 *         - DISABLE   
 *         - ENABLE
 *      2. Local loopback is used to loopback packet TX from switch core back to switch core.
 */
int32
dal_ssw_port_macLocalLoopbackEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, enable=%d", 
           unit, port, enable); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable != DISABLED && enable != ENABLED), RT_ERR_INPUT);

    PORT_SEM_LOCK(unit);

    /* get value from CHIP*/
    if ((ret = reg_field_write(unit, (uint32)portMacControl_regidx[port], (uint32)portLLB_fieldidx[port], &enable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_ssw_port_macLocalLoopbackEnable_set */
#endif

/* Function Name:
 *      dal_ssw_port_backpressureEnable_get
 * Description:
 *      Get the half duplex backpressure enable status of the specific port
 * Input:
 *      unit           - unit id
 *      port           - port id
 * Output:
 *      pEnable       - - pointer to the enable status of backpressure in half duplex mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      1. The mac local loopback enable status of the port is as following:
 *         - DISABLE   
 *         - ENABLE
 *      2. Used to support backpressure in half mode.
 */
int32
dal_ssw_port_backpressureEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", 
           unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);

    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, (uint32)portMacControl_regidx[port], (uint32)portBackpressure_fieldidx[port], pEnable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pEnable=%d", *pEnable); 

    return RT_ERR_OK;
} /* end of dal_ssw_port_backpressureEnable_get */

/* Function Name:
 *      dal_ssw_port_backpressureEnable_set
 * Description:
 *      Set the half duplex backpressure enable status of the specific port
 * Input:
 *      unit           - unit id
 *      port           - port id
 *      enable        - enable status of backpressure in half duplex mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      1. The mac local loopback enable status of the port is as following:
 *         - DISABLE   
 *         - ENABLE
 *      2. Used to support backpressure in half mode.
 */
int32
dal_ssw_port_backpressureEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, enable=%d", 
           unit, port, enable); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable != DISABLED && enable != ENABLED), RT_ERR_INPUT);

    PORT_SEM_LOCK(unit);

    /* get value from CHIP*/
    if ((ret = reg_field_write(unit, (uint32)portMacControl_regidx[port], (uint32)portBackpressure_fieldidx[port], &enable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_ssw_port_backpressureEnable_set */

/* Function Name:
 *      dal_ssw_port_linkChange_register
 * Description:
 *      Register callback function for notification of link change 
 * Input:
 *      unit           - unit id
 *      link_change_callback      - Callback function for link change
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 */
int32
dal_ssw_port_linkChange_register(uint32 unit, dal_link_change_callback_f link_change_callback)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, link_change_callback=%x", 
           unit, link_change_callback); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == link_change_callback), RT_ERR_NULL_POINTER);
    
    PORT_SEM_LOCK(unit);

    link_change_callback_f[unit] = link_change_callback;

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* End of dal_ssw_port_linkChange_register */

/* Function Name:
 *      dal_ssw_port_linkChange_unregister
 * Description:
 *      Unregister callback function for notification of link change 
 * Input:
 *      unit           - unit id
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 * Note:
 */
int32
dal_ssw_port_linkChange_unregister(uint32 unit)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d", 
           unit); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */


    PORT_SEM_LOCK(unit);

    link_change_callback_f[unit] = NULL;

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* End of dal_ssw_port_linkChange_unregister */


#if 0 /* The function is move to dal_esw_diag.c */
/* Module Name    : Port */
/* Sub-module Name: RTCT */

/* Function Name:
 *      dal_ssw_port_rtctResult_get
 * Description:
 *      Get test result of RTCT.
 * Input:
 *      unit        - unit id
 *      port        - the port for retriving RTCT test result
 * Output:
 *      pRtctResult - RTCT result 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_RTCT_NOT_FINISH   - RTCT not finish. Need to wait a while.
 *      RT_ERR_TIMEOUT      - RTCT test timeout in this port.
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      If linkType is PORT_SPEED_1000M, test result will be stored in ge_result. 
 *      If linkType is PORT_SPEED_10M or PORT_SPEED_100M, test result will be stored in fe_result.
 */
int32
dal_ssw_port_rtctResult_get(uint32 unit, rtk_port_t port, rtk_rtctResult_t *pRtctResult)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", 
           unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pRtctResult), RT_ERR_NULL_POINTER);

    osal_memset(pRtctResult, 0, sizeof(rtk_rtctResult_t));

    PORT_SEM_LOCK(unit);

    /* Get RTCT Result */
    if ((ret = phy_rtctResult_get(unit, port, pRtctResult)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /*end of dal_ssw_port_rtctResult_get*/

/* Function Name:
 *      dal_ssw_port_rtct_start
 * Description:
 *      Start RTCT for ports. 
 *      When enable RTCT, the port won't transmit and receive normal traffic.
 * Input:
 *      unit      - unit id
 *      pPortmask - the ports for RTCT test
 * Output:
 *      None
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
dal_ssw_port_rtct_start(uint32 unit, rtk_portmask_t *pPortmask)
{
    rtk_port_t  port, max_port;
    int32       ret = RT_ERR_FAILED;

    RT_PARAM_CHK((NULL == pPortmask), RT_ERR_NULL_POINTER);
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, *pPortmask=0x%x", 
           unit, *pPortmask); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    PORT_SEM_LOCK(unit);
    max_port = HAL_GET_MAX_PORT(unit);
    for (port = 0; port < max_port; port++)
    {
        if (RTK_PORTMASK_IS_PORT_SET(*pPortmask, port))
        {
            if ((ret = phy_rtct_start(unit, port)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }
        }
    }
    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_ssw_port_rtct_start */
#endif

/* Function Name:
 *      _dal_ssw_port_init_config
 * Description:
 *      Initialize default configuration for port module of the specified device.
 * Input:
 *      unit          - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      1. Module must be initialized before using all of APIs in this module
 */
static int32
_dal_ssw_port_init_config(uint32 unit)
{
    uint32  val;
    int32   ret;
    rtk_port_t  port, max_port;
    rtk_portmask_t  portmask;
    rtk_port_phy_ability_t phy_ability;
    hal_control_t   *pHalCtrl;
    rtk_port_crossOver_mode_t   mode;
    rtk_port_speed_t    speed;
    rtk_port_duplex_t   duplex;

    /* Initial serdes for green feature */
    val = 0;
    reg_field_write(unit, SSW_SERDES_0_CONTROL5r, SSW_REG_TX_AMPf, &val);
    reg_field_write(unit, SSW_SERDES_1_CONTROL5r, SSW_REG_TX_AMPf, &val);
    reg_field_write(unit, SSW_SERDES_2_CONTROL5r, SSW_REG_TX_AMPf, &val);
    reg_field_write(unit, SSW_SERDES_3_CONTROL5r, SSW_REG_TX_AMPf, &val);
    reg_field_write(unit, SSW_SERDES_4_CONTROL5r, SSW_REG_TX_AMPf, &val);
    reg_field_write(unit, SSW_SERDES_5_CONTROL5r, SSW_REG_TX_AMPf, &val);
    reg_field_write(unit, SSW_SERDES_6_CONTROL5r, SSW_REG_TX_AMPf, &val);
    reg_field_write(unit, SSW_SERDES_7_CONTROL5r, SSW_REG_TX_AMPf, &val);
    reg_field_write(unit, SSW_SERDES_8_CONTROL5r, SSW_REG_TX_AMPf, &val);
    reg_field_write(unit, SSW_SERDES_9_CONTROL5r, SSW_REG_TX_AMPf, &val);
    reg_field_write(unit, SSW_SERDES_10_CONTROL5r, SSW_REG_TX_AMPf, &val);
    reg_field_write(unit, SSW_SERDES_11_CONTROL5r, SSW_REG_TX_AMPf, &val);
    val = 1;
    reg_field_write(unit, SSW_PAIR_0_1_INTRALINK_SERDES_CONTROLr, SSW_EN_SGMII_TXC0f, &val);

    /* Some serdes control register value seting for RTL8218 + RTL8389M/RTL8389L */
    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

#if 0 /* U-boot do it */
    max_port = HAL_GET_MAX_PORT(unit);
    HAL_GET_ALL_PORTMASK(unit, portmask);
    for (port = 0; port < max_port; port = port + 2)
    {
        uint32  serdes_idx = port/2;

        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }

        if (HAL_IS_PHY_EXIST(unit, port) && (PHY_MODEL_ID_RTL8218 == pHalCtrl->pPhy_ctrl[port]->phy_model_id))
        {
            if (RTL8389L_CHIP_ID == pHalCtrl->chip_id)
            {
/* #ifdef CONFIG_4L_PCB */
#define Serdes_CTRL_REG1_0_val1 0x003051f3
#define Serdes_CTRL_REG1_0_val  0x003051d3
#define Serdes_CTRL_REG4_0_val  0x20001401
#define Serdes_CTRL_REG2_0_val  0x79218030
#define Serdes_CTRL_REG5_0_val  0x0020051f
#define Serdes_CTRL_REG0_0_val1 0x47622fe3
#define Serdes_CTRL_REG0_0_val2 0x47622feb
#define Serdes_CTRL_REG6_0_val  0x00602000

                /* Serdes Patch for RTL8389L + RTL8218 */
                unsigned int serdes_ctrl_reg0_idx = portSerdesControl_regidx[port];
                unsigned int serdes_ctrl_reg1_idx = portSerdesControl_regidx[port] + 1;
                unsigned int serdes_ctrl_reg2_idx = portSerdesControl_regidx[port] + 2;
                unsigned int serdes_ctrl_reg4_idx = portSerdesControl_regidx[port] + 4;
                unsigned int serdes_ctrl_reg5_idx = portSerdesControl_regidx[port] + 5;
                unsigned int serdes_ctrl_reg6_idx = portSerdesControl_regidx[port] + 6;

                /* xxx */
                val = Serdes_CTRL_REG1_0_val1;
                reg_write(unit, serdes_ctrl_reg1_idx, &val);
                val = Serdes_CTRL_REG1_0_val;
                reg_write(unit, serdes_ctrl_reg1_idx, &val);
                
                /* Rx Equalizer Boost */
                val = Serdes_CTRL_REG4_0_val;
                reg_write(unit, serdes_ctrl_reg4_idx, &val);
                
                /* xxx */
                val = Serdes_CTRL_REG2_0_val;
                reg_write(unit, serdes_ctrl_reg2_idx, &val);
                
                /* xxx */
                val = Serdes_CTRL_REG5_0_val;
                reg_write(unit, serdes_ctrl_reg5_idx, &val);
        
                /* xxx */
                if ((serdes_idx & 0x1) == 0x0)
                {
                    val = Serdes_CTRL_REG0_0_val1;
                    reg_write(unit, serdes_ctrl_reg0_idx, &val);
                    val = Serdes_CTRL_REG0_0_val2;
                    reg_write(unit, serdes_ctrl_reg0_idx, &val);
                    val = Serdes_CTRL_REG6_0_val;
                    reg_write(unit, serdes_ctrl_reg6_idx, &val);
                }
                else
                {
                    val = 0x46622fe4;
                    reg_write(unit, serdes_ctrl_reg0_idx, &val);
                    val = 0x00305dfc;
                    reg_write(unit, serdes_ctrl_reg1_idx, &val);
                }

                /* 25M_CLK driving strength larger one step */
                //REG32_CHG(serdes_ctrl_reg6_idx, (0x1 << 16), (0x1 << 16));
                //REG32_CHK(serdes_ctrl_reg6_idx, 0x00612000);
            }
            else
            {
                /* Serdes Patch for RTL8389M + RTL8218 */
                unsigned int serdes_ctrl_reg0_idx = portSerdesControl_regidx[port];
                unsigned int serdes_ctrl_reg1_idx = portSerdesControl_regidx[port] + 1;
                unsigned int serdes_ctrl_reg2_idx = portSerdesControl_regidx[port] + 2;
                unsigned int serdes_ctrl_reg4_idx = portSerdesControl_regidx[port] + 4;
                unsigned int serdes_ctrl_reg5_idx = portSerdesControl_regidx[port] + 5;
                uint32  reg_value = 0;
                //unsigned int serdes_ctrl_reg6_idx = portSerdesControl_regidx[port] + 6;
        
                /* xxx */
                reg_read(unit, serdes_ctrl_reg1_idx, &reg_value);
                val = 1;
                reg_field_set(unit, serdes_ctrl_reg1_idx, REG_CP4OP, &val, &reg_value);
                val = 3;
                reg_field_set(unit, serdes_ctrl_reg1_idx, REG_CMU_CP_SEL, &val, &reg_value);
                val = 2;
                reg_field_set(unit, serdes_ctrl_reg1_idx, REG_CMU_CLKRDY, &val, &reg_value);
                val = 0;
                reg_field_set(unit, serdes_ctrl_reg1_idx, REG_CMUEN, &val, &reg_value);
                reg_write(unit, serdes_ctrl_reg1_idx, &reg_value);
                
                /* Rx Equalizer Boost */
                reg_read(unit, serdes_ctrl_reg4_idx, &reg_value);
                val = 4;
                reg_field_set(unit, serdes_ctrl_reg4_idx, REG_RX_AMP, &val, &reg_value);
                reg_write(unit, serdes_ctrl_reg4_idx, &reg_value);

                /* xxx */
                reg_read(unit, serdes_ctrl_reg2_idx, &reg_value);
                val = 1;
                reg_field_set(unit, serdes_ctrl_reg2_idx, REG_H_KVCO, &val, &reg_value);
                val = 0;
                reg_field_set(unit, serdes_ctrl_reg2_idx, REG_EQ_CP, &val, &reg_value);
                val = 0;
                reg_field_set(unit, serdes_ctrl_reg2_idx, REG_EMPHAS_EN, &val, &reg_value);
                reg_write(unit, serdes_ctrl_reg2_idx, &reg_value);

                /* xxx */
                reg_read(unit, serdes_ctrl_reg5_idx, &reg_value);
                val = 5;
                reg_field_set(unit, serdes_ctrl_reg5_idx, REG_TX_EMP, &val, &reg_value);
                val = 6;
                reg_field_set(unit, serdes_ctrl_reg5_idx, REG_TX_AMP, &val, &reg_value);
                reg_write(unit, serdes_ctrl_reg5_idx, &reg_value);

                /* xxx */
                if ((serdes_idx & 0x1) == 0x0)
                {
                    reg_read(unit, serdes_ctrl_reg0_idx, &reg_value);
                    val = 4;
                    reg_field_set(unit, serdes_ctrl_reg0_idx, REG_CDR_CP, &val, &reg_value);
                    val = 2;
                    reg_field_set(unit, serdes_ctrl_reg0_idx, REG_CALIB_TIME, &val, &reg_value);
                    val = 1;
                    reg_field_set(unit, serdes_ctrl_reg0_idx, REG_ADP_EQ_OFF, &val, &reg_value);
                    val = 0;
                    reg_field_set(unit, serdes_ctrl_reg0_idx, RX_EN, &val, &reg_value);
                    val = 0;
                    reg_field_set(unit, serdes_ctrl_reg0_idx, REG_PDOWN, &val, &reg_value);
                    val = 1;
                    reg_field_set(unit, serdes_ctrl_reg0_idx, REG_POW_PCIX, &val, &reg_value);
                    val = 1;
                    reg_field_set(unit, serdes_ctrl_reg0_idx, REG_TX_EN, &val, &reg_value);
                    reg_write(unit, serdes_ctrl_reg0_idx, &reg_value);

                    reg_read(unit, serdes_ctrl_reg0_idx, &reg_value);
                    val = 1;
                    reg_field_set(unit, serdes_ctrl_reg0_idx, RX_EN, &val, &reg_value);
                    reg_write(unit, serdes_ctrl_reg0_idx, &reg_value);
                }
                else
                {
                    reg_read(unit, serdes_ctrl_reg0_idx, &reg_value);
                    val = 4;
                    reg_field_set(unit, serdes_ctrl_reg0_idx, REG_CDR_CP, &val, &reg_value);
                    val = 2;
                    reg_field_set(unit, serdes_ctrl_reg0_idx, REG_CALIB_TIME, &val, &reg_value);
                    val = 1;
                    reg_field_set(unit, serdes_ctrl_reg0_idx, REG_ADP_EQ_OFF, &val, &reg_value);
                    val = 0;
                    reg_field_set(unit, serdes_ctrl_reg0_idx, RX_EN, &val, &reg_value);
                    val = 0;
                    reg_field_set(unit, serdes_ctrl_reg0_idx, REG_PDOWN, &val, &reg_value);
                    val = 1;
                    reg_field_set(unit, serdes_ctrl_reg0_idx, REG_POW_PCIX, &val, &reg_value);
                    val = 1;
                    reg_field_set(unit, serdes_ctrl_reg0_idx, REG_TX_EN, &val, &reg_value);
                    reg_write(unit, serdes_ctrl_reg0_idx, &reg_value);

                    reg_read(unit, serdes_ctrl_reg0_idx, &reg_value);
                    val = 1;
                    reg_field_set(unit, serdes_ctrl_reg0_idx, REG_PDOWN, &val, &reg_value);
                    val = 0;
                    reg_field_set(unit, serdes_ctrl_reg0_idx, REG_POW_PCIX, &val, &reg_value);
                    val = 0;
                    reg_field_set(unit, serdes_ctrl_reg0_idx, REG_TX_EN, &val, &reg_value);
                    reg_write(unit, serdes_ctrl_reg0_idx, &reg_value);
                }
        
                /* 25M_CLK driving strength larger one step */
                //REG32_CHG(serdes_ctrl_reg6_addr, (0x1 << 16), (0x1 << 16));
                //REG32_CHK(serdes_ctrl_reg6_addr, 0x00612000);
            }
        }
    }
#endif

    phy_ability.Half_10 = RTK_DEFAULT_PORT_10HALF_CAPABLE;
    phy_ability.Full_10 = RTK_DEFAULT_PORT_10FULL_CAPABLE;
    phy_ability.Half_100 = RTK_DEFAULT_PORT_100HALF_CAPABLE;
    phy_ability.Full_100 = RTK_DEFAULT_PORT_100FULL_CAPABLE;
    phy_ability.FC = RTK_DEFAULT_PORT_PAUSE_CAPABILITY;
    phy_ability.AsyFC = RTK_DEFAULT_PORT_ASYPAUSE_CAPABILITY;
    
    max_port = HAL_GET_MAX_PORT(unit);
    HAL_GET_ALL_PORTMASK(unit, portmask);

    for (port = 0; port < max_port; port++)
    {
        if (!HAL_IS_PORT_EXIST(unit, port)) 
        {
            continue;
        }
        
        if (HAL_IS_FE_PORT(unit, port))
        {
            phy_ability.Half_1000 = DISABLED;
            phy_ability.Full_1000 = DISABLED;
        }
        else
        {
            phy_ability.Half_1000 = RTK_DEFAULT_PORT_1000HALF_CAPABLE;
            phy_ability.Full_1000 = RTK_DEFAULT_PORT_1000FULL_CAPABLE;
        }

        /* Config MAC */
        if (!HAL_IS_CPU_PORT(unit, port))
        {
            if ((ret = dal_ssw_port_isolation_set(unit, port, portmask)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "Port init set port isolation portmask failed");
                return ret;
            }
        }
        
        /* Config PHY in port that PHY exist */
        if (HAL_IS_PHY_EXIST(unit, port))
        {
            pPhy_info[unit]->auto_mode_pause[port] = RTK_DEFAULT_PORT_PAUSE_CAPABILITY;
            pPhy_info[unit]->auto_mode_asy_pause[port] = RTK_DEFAULT_PORT_ASYPAUSE_CAPABILITY;
            pMac_info[unit]->admin_enable[port] = RTK_ENABLE_END;
            if ((ret = dal_ssw_port_adminEnable_set(unit, port, RTK_DEFAULT_PORT_ADMIN_ENABLE)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "Port init enable port failed");
                return ret;
            }
            
            if (!HAL_IS_CPU_PORT(unit, port) && !HAL_IS_SERDES_PORT(unit, port))
            {
                if ((ret = dal_ssw_port_phyAutoNegoAbility_set(unit, port, &phy_ability)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_PORT), "Port init set autonegotiation ability failed");
                    return ret;
                }
                
                if ((ret = dal_ssw_port_phyAutoNegoEnable_set(unit, port, RTK_DEFAULT_PORT_AUTONEGO_ENABLE)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_PORT), "Port init enable PHY autonegotiation failed");
                    return ret;
                }
                mode = PORT_CROSSOVER_MODE_END;
                if ((ret = dal_ssw_port_phyCrossOverMode_get(unit, port, &mode)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_PORT), "Port init get PHY cross over mode failed");
                    return ret;
                }
                pPhy_info[unit]->cross_over_mode[port] = mode;
                speed = PORT_SPEED_END;
                if ((ret = phy_speed_get(unit, port, &speed)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_PORT), "Port init get PHY speed failed");
                    return ret;
                }
                pPhy_info[unit]->force_mode_speed[port] = speed;
                duplex = PORT_DUPLEX_END;
                if ((ret = phy_duplex_get(unit, port, &duplex)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_PORT), "Port init get PHY duplex failed");
                    return ret;
                }
                pPhy_info[unit]->force_mode_duplex[port] = duplex;
            }
        }
    }
        
    return RT_ERR_OK;
}/* end of _dal_ssw_port_init_config */

/* Function Name:
 *      dal_ssw_port_greenEnable_get
 * Description:
 *      Get the statue of green feature of the specific port in the specific unit
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pEnable - pointer to status of green feature
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_ssw_port_greenEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    if ((ret = phy_greenEnable_get(unit, port, pEnable)) != RT_ERR_OK)
    {
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_ssw_port_greenEnable_get */

/* Function Name:
 *      dal_ssw_port_greenEnable_set
 * Description:
 *      Set the statue of green feature of the specific port in the specific unit
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of green feature
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      None
 */
int32
dal_ssw_port_greenEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, enable=%d", 
           unit, port, enable); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable != DISABLED && enable != ENABLED), RT_ERR_INPUT);

    PORT_SEM_LOCK(unit);
    pMac_info[unit]->green_enable[port] = (ENABLED == enable)? ENABLED : DISABLED;
    PORT_SEM_UNLOCK(unit);

    /* Configure if PHY supported green feature */
    ret = phy_greenEnable_set(unit, port, enable);

    return RT_ERR_OK;
} /* end of dal_ssw_port_greenEnable_set */


#if defined(CONFIG_SDK_DRIVER_NIC_USER_MODE)
/* Function Name:
 *      _dal_ssw_port_linkChange_isr
 * Description:
 *      switch interrupt
 * Input:
 *      isr_param - isr callback parameter
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      OSAL_INT_HANDLED
 * Note:
 *      None
 */
static void
_dal_ssw_port_linkChange_isr(uint32 unit, void *isr_param)
{
    rtk_portmask_t  changed_portmask;
    
    
    
    /* update first word of changed portmask */
    RTK_PORTMASK_WORD_SET(changed_portmask, 0, *((uint32 *)isr_param));
    
      
    /* if callback function exist, notify upper component the newest changed portmask */
    if (NULL != link_change_callback_f[unit])
    {
        link_change_callback_f[unit](unit, &changed_portmask);
    }
    
    return ;
} /* end of _dal_ssw_port_linkChange_isr */
#endif /* CONFIG_SDK_DRIVER_NIC_USER_MODE */

/* Function Name:
 *      dal_ssw_port_phyCrossOverMode_get
 * Description:
 *      Get cross over mode in the specified port.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pMode - pointer to cross over mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Following value is valid
 *      - PORT_CROSSOVER_MODE_AUTO
 *      - PORT_CROSSOVER_MODE_MDI
 *      - PORT_CROSSOVER_MODE_MDIX
 */
int32
dal_ssw_port_phyCrossOverMode_get(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t *pMode)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pMode), RT_ERR_NULL_POINTER);
    
    PORT_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = phy_crossOverMode_get(unit, port, pMode)) != RT_ERR_OK && (ret != RT_ERR_PHY_FIBER_LINKUP))
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }
    else if (ret == RT_ERR_PHY_FIBER_LINKUP)
    {
        /* Return from shadow database */
        *pMode = pPhy_info[unit]->cross_over_mode[port];
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
    }

    PORT_SEM_UNLOCK(unit);
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pMode=%d", *pMode); 
    
    return RT_ERR_OK;
}/* end of dal_ssw_port_phyCrossOverMode_get */

/* Function Name:
 *      dal_ssw_port_phyCrossOverMode_set
 * Description:
 *      Set cross over mode in the specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 *      mode - cross over mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      Following value is valid
 *      - PORT_CROSSOVER_MODE_AUTO
 *      - PORT_CROSSOVER_MODE_MDI
 *      - PORT_CROSSOVER_MODE_MDIX
 */
int32
dal_ssw_port_phyCrossOverMode_set(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t mode)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, mode=%d", 
           unit, port, mode);     
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(mode >= PORT_CROSSOVER_MODE_END, RT_ERR_INPUT);
    
    
    PORT_SEM_LOCK(unit);

    /* set value from CHIP*/
    if ((ret = phy_crossOverMode_set(unit, port, mode)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    /* Update the shadow database */
   pPhy_info[unit]->cross_over_mode[port] = mode;
   PORT_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
}/* end of dal_ssw_port_phyCrossOverMode_set */

/* Function Name:
 *      dal_ssw_port_flowCtrlEnable_get
 * Description:
 *      Get the flow control status of the specific port
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pEnable - pointer to the status of the flow control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      The API get the flow control status by port based, no matter N-WAY is enabled or disabled.
 */
int32
dal_ssw_port_flowCtrlEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    rtk_enable_t    nway_enable, flowctrl_enable;
    rtk_port_phy_ability_t  ability;
    rtk_port_speed_t    speed;
    rtk_port_duplex_t   duplex;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", unit, port);     

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((pEnable == NULL), RT_ERR_NULL_POINTER);

    if ((ret = dal_ssw_port_phyAutoNegoEnable_get(unit, port, &nway_enable)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    if (ENABLED == nway_enable)
    {
        if ((ret = dal_ssw_port_phyAutoNegoAbility_get(unit, port, &ability)) != RT_ERR_OK)
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }
        if (ability.FC && ability.AsyFC)
            (*pEnable) = ENABLED;
        else
            (*pEnable) = DISABLED;
    }
    else
    {
        if ((ret = dal_ssw_port_phyForceModeAbility_get(unit, port, &speed, &duplex, &flowctrl_enable)) != RT_ERR_OK)
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }
        (*pEnable) = flowctrl_enable;
    }
    
    return RT_ERR_OK;
} /* end of dal_ssw_port_flowCtrlEnable_get */

/* Function Name:
 *      dal_ssw_port_flowCtrlEnable_set
 * Description:
 *      Set the flow control status to the specific port
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      enable    - enable status of flow control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      The API is apply the flow control status by port based, no matter N-WAY is enabled or disabled.
 */
int32
dal_ssw_port_flowCtrlEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    rtk_enable_t    flowctrl_enable;
    rtk_port_phy_ability_t  ability;
    rtk_port_speed_t    speed;
    rtk_port_duplex_t   duplex;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, enable=%d", 
           unit, port, enable);     

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable != DISABLED) && (enable != ENABLED), RT_ERR_INPUT);

    if ((ret = dal_ssw_port_phyAutoNegoAbility_get(unit, port, &ability)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }
    if (ENABLED == enable)
    {
        ability.FC = 1;
        ability.AsyFC = 1;
    }
    else
    {
        ability.FC = 0;
        ability.AsyFC = 0;
    }
    if ((ret = dal_ssw_port_phyAutoNegoAbility_set(unit, port, &ability)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    if ((ret = dal_ssw_port_phyForceModeAbility_get(unit, port, &speed, &duplex, &flowctrl_enable)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }
    if (ENABLED == enable)
    {
        flowctrl_enable = ENABLED;
    }
    else
    {
        flowctrl_enable = DISABLED;
    }
    if ((ret = dal_ssw_port_phyForceModeAbility_set(unit, port, speed, duplex, flowctrl_enable)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }
    
    return RT_ERR_OK;
} /* end of dal_ssw_port_flowCtrlEnable_set */

/* Function Name:
 *      dal_ssw_port_phyComboPortFiberMedia_get
 * Description:
 *      Get PHY port fiber media of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:                   
 *      pMedia - pointer to the port fiber media
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. fiber media type value is as following:
 *      - PORT_FIBER_MEDIA_1000
 *      - PORT_FIBER_MEDIA_100
 *      - PORT_FIBER_MEDIA_AUTO
 */
int32
dal_ssw_port_phyComboPortFiberMedia_get(uint32 unit, rtk_port_t port, rtk_port_fiber_media_t *pMedia)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", 
           unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pMedia), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);

    /* get value from CHIP*/
    if ((ret = phy_fiber_media_get(unit, port, pMedia)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pMedia=%d", *pMedia); 

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_ssw_port_phyComboPortFiberMedia_get */

/* Function Name:
 *      dal_ssw_port_phyComboPortFiberMedia_set
 * Description:
 *      Set PHY port fiber media of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      pMedia - pointer to the port fiber media
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. fiber media type value is as following:
 *      - PORT_FIBER_MEDIA_1000
 *      - PORT_FIBER_MEDIA_100
 *      - PORT_FIBER_MEDIA_AUTO
 */
int32
dal_ssw_port_phyComboPortFiberMedia_set(uint32 unit, rtk_port_t port, rtk_port_fiber_media_t media)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, media=%d", 
           unit, port, media); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((media >= PORT_MEDIA_END), RT_ERR_INPUT);

    PORT_SEM_LOCK(unit);

    /* set value from CHIP*/
    if ((ret = phy_fiber_media_set(unit, port, media)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return ret;
} /* end of dal_ssw_port_phyComboPortFiberMedia_set */

/* Function Name:
 *      dal_ssw_port_linkDownPowerSavingEnable_get
 * Description:
 *      Get the statue of link-down power saving of the specific port in the specific unit
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to status of link-down power saving
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_ssw_port_linkDownPowerSavingEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);
    if ((ret = phy_linkDownPowerSavingEnable_get(unit, port, pEnable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }
    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_ssw_port_linkDownPowerSavingEnable_get */

/* Function Name:
 *      dal_ssw_port_linkDownPowerSavingEnable_set
 * Description:
 *      Set the statue of link-down power saving of the specific port in the specific unit
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of link-down power saving
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - input parameter out of range
 * Note:
 *      None
 */
int32
dal_ssw_port_linkDownPowerSavingEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, enable=%d", 
           unit, port, enable); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable != DISABLED && enable != ENABLED), RT_ERR_INPUT);

    /* Configure if PHY supported green feature */
    PORT_SEM_LOCK(unit);
    if ((ret = phy_linkDownPowerSavingEnable_set(unit, port, enable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }
    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_ssw_port_linkDownPowerSavingEnable_set */

/* Function Name:
 *      dal_ssw_port_gigaLiteEnable_get
 * Description:
 *      Get the statue of Giga Lite the specific port in the specific unit
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to status of Giga Lite
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_ssw_port_gigaLiteEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);
    if ((ret = phy_gigaLiteEnable_get(unit, port, pEnable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }
    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_ssw_port_gigaLiteEnable_get */

/* Function Name:
 *      dal_ssw_port_gigaLiteEnable_set
 * Description:
 *      Set the statue of Giga Lite of the specific port in the specific unit
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of Giga Lite
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - input parameter out of range
 * Note:
 *      None
 */
int32
dal_ssw_port_gigaLiteEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, enable=%d", 
           unit, port, enable); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable != DISABLED && enable != ENABLED), RT_ERR_INPUT);

    /* Configure if PHY supported green feature */
    PORT_SEM_LOCK(unit);
    if ((ret = phy_gigaLiteEnable_set(unit, port, enable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }
    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_ssw_port_gigaLiteEnable_set */


