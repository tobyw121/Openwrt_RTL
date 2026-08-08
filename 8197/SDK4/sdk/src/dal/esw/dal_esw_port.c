/* 
 * Copyright(c) Realtek Semiconductor Corporation, 2008 
 * All rights reserved. 
 * 
 * $Revision: 37198 $
 * $Date: 2013-02-25 18:42:57 +0800 (Mon, 25 Feb 2013) $
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
#include <common/rt_autoconf.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_bitop.h>
#include <common/util/rt_util.h>
#include <common/debug/rt_log.h>
#include <osal/time.h>
#include <osal/sem.h>
#include <osal/lib.h>
#include <osal/memory.h>
#include <osal/time.h>
#include <osal/spl.h>
#include <hal/chipdef/allmem.h>
#include <hal/chipdef/esw/rtk_esw_table_struct.h>
#include <hal/mac/reg.h>
#include <hal/mac/mem.h>
#include <hal/chipdef/allreg.h>
#include <hal/chipdef/esw/rtk_esw_reg_struct.h>
#include <hal/common/miim.h>
#include <hal/common/halctrl.h>
#include <dal/dal_common.h>
#include <dal/esw/dal_esw_port.h>
#include <dal/esw/dal_esw_vlan.h>
#include <dal/esw/dal_esw_switch.h>
#include <rtk/port.h>
#include <rtk/default.h>
#include <drv/intr/intr.h>
#include <ioal/mem32.h>

/* 
 * Symbol Definition 
 */

typedef struct dal_esw_mac_info_s
{
    uint8   admin_enable[RTK_MAX_NUM_OF_PORTS];
    uint8   green_enable[RTK_MAX_NUM_OF_PORTS];
#if defined(CONFIG_SDK_WA_88E6063_COMPATIBLE)
    uint8   tx_enable[RTK_MAX_NUM_OF_PORTS];
#endif
} dal_esw_mac_info_t;

typedef struct dal_esw_phy_info_s
{
    uint8   force_mode_speed[RTK_MAX_NUM_OF_PORTS];
    uint8   force_mode_duplex[RTK_MAX_NUM_OF_PORTS];
    uint8   force_mode_flowControl[RTK_MAX_NUM_OF_PORTS];
    uint8   auto_mode_pause[RTK_MAX_NUM_OF_PORTS];
    uint8   auto_mode_asy_pause[RTK_MAX_NUM_OF_PORTS];
    uint8   cross_over_mode[RTK_MAX_NUM_OF_PORTS];
} dal_esw_phy_info_t;


/* 
 * Data Declaration 
 */
static uint32               port_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         port_sem[RTK_MAX_NUM_OF_UNIT];
static dal_esw_mac_info_t   *pMac_info[RTK_MAX_NUM_OF_UNIT];
static dal_esw_phy_info_t   *pPhy_info[RTK_MAX_NUM_OF_UNIT];
static dal_link_change_callback_f   link_change_callback_f[RTK_MAX_NUM_OF_UNIT];

const static uint16 portIntraSerdesPage0Reg0_regidx[] = {ESW_SDS0_LOW_CH_PAGE0_REG0r, -1, ESW_SDS1_LOW_CH_PAGE0_REG0r};
const static uint16 portIntraSerdesPage0Reg4_regidx[] = {ESW_SDS0_LOW_CH_PAGE0_REG4r, -1, ESW_SDS1_LOW_CH_PAGE0_REG4r};

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

static osal_spinlock_t  port_spin_lock;
#if defined(CONFIG_SDK_KERNEL_LINUX_KERNEL_MODE)
#define PORT_SPIN_LOCK(unit)    osal_spl_spin_lock(&port_spin_lock)
#define PORT_SPIN_UNLOCK(unit)  osal_spl_spin_unlock(&port_spin_lock)
#else
#define PORT_SPIN_LOCK(unit)    
#define PORT_SPIN_UNLOCK(unit)  
#endif

#define ESW_LINK_STA_PORT           (0)
#define ESW_LINK_STA_PORT_MASK      (1 << ESW_LINK_STA_PORT)
#define ESW_LINK_MEDIA_PORT         (1)
#define ESW_LINK_MEDIA_PORT_MASK    (1 << ESW_LINK_MEDIA_PORT)
#define ESW_SPD_STA_PORT            (2)
#define ESW_SPD_STA_PORT_MASK       (0x3 << ESW_SPD_STA_PORT)
#define ESW_DUP_STA_PORT            (4)
#define ESW_DUP_STA_PORT_MASK       (1 << ESW_DUP_STA_PORT)
#define ESW_TX_PAUSE_PORT           (5)
#define ESW_TX_PAUSE_PORT_MASK      (1 << ESW_TX_PAUSE_PORT)
#define ESW_RX_PAUSE_PORT           (6)
#define ESW_RX_PAUSE_PORT_MASK      (1 << ESW_RX_PAUSE_PORT)

/* 
 * Function Declaration 
 */
static int32 _dal_esw_port_init_config(uint32 unit);
#if defined(CONFIG_SDK_DRIVER_NIC_USER_MODE)
static void _dal_esw_port_linkChange_isr(uint32 unit, void *isr_param);
#endif /* CONFIG_SDK_DRIVER_NIC_USER_MODE */

/* Function Name:
 *      dal_esw_port_init
 * Description:
 *      Initialize port module of the specified device.
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
dal_esw_port_init(uint32 unit)
{
    int32   ret;
    
    port_init[unit] = INIT_NOT_COMPLETED;

    /* init port spin-lock semaphore */
    port_spin_lock = 0;

    /* create semaphore */
    port_sem[unit] = osal_sem_mutex_create();
    if (0 == port_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PORT), "semaphore create failed");
        return RT_ERR_FAILED;
    }
    
    pMac_info[unit] = (dal_esw_mac_info_t *)osal_alloc(sizeof(dal_esw_mac_info_t));
    if (NULL == pMac_info[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PORT), "memory allocate failed");
        return RT_ERR_FAILED;
    }        
    
    osal_memset(pMac_info[unit], 0, sizeof(dal_esw_mac_info_t));

    pPhy_info[unit] = (dal_esw_phy_info_t *)osal_alloc(sizeof(dal_esw_phy_info_t));
    if (NULL == pPhy_info[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PORT), "memory allocate failed");
        osal_free(pMac_info[unit]);
        pMac_info[unit] = NULL;
        return RT_ERR_FAILED;
    }        

    osal_memset(pPhy_info[unit], 0, sizeof(dal_esw_phy_info_t));
    
    /* init callback function for link change */
    link_change_callback_f[unit] = 0;
    
    /* set init flag to complete init */
    port_init[unit] = INIT_COMPLETED;    
    
    if (( ret = _dal_esw_port_init_config(unit)) != RT_ERR_OK)
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
            port_init[unit] = INIT_NOT_COMPLETED;
            osal_free(pMac_info[unit]);
            pMac_info[unit] = NULL;
            osal_free(pPhy_info[unit]);
            pPhy_info[unit] = NULL;
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "LinkScan interrupt enable failed");
            return ret;
        }
        
        /* register callback */
        if (( ret = drv_intr_link_stat_register(unit, _dal_esw_port_linkChange_isr)) != RT_ERR_OK)
        {
            port_init[unit] = INIT_NOT_COMPLETED;
            osal_free(pMac_info[unit]);
            pMac_info[unit] = NULL;
            osal_free(pPhy_info[unit]);
            pPhy_info[unit] = NULL;
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "LinkScan interrupt handler installed failed");
            return ret;
        }
    }
#endif
    
    return RT_ERR_OK;
} /* end of dal_esw_port_init */

/* Module Name    : Port                                       */
/* Sub-module Name: Parameter settings for the port-based view */

/* Function Name:
 *      dal_esw_port_link_get
 * Description:
 *      Get the link status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pStatus - pointer to the link status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The link status of the port is as following:
 *      - LINKDOWN
 *      - LINKUP
 */
int32
dal_esw_port_link_get(uint32 unit, rtk_port_t port, rtk_port_linkStatus_t *pStatus)
{
    uint32  val;
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pStatus), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);

    PORT_SEM_LOCK(unit);
    
    /* Lock the switch software reset semaphore */
    dal_esw_switch_sofware_reset_sem(unit, 0); /* LOCK */
    if (HAL_IS_PHY_EXIST(unit, port))
    {
        /* get newest port link status */
        ret = hal_miim_read(unit, port, 0, 1, &val);
        val = (val>>2)&0x1; /* Move to bit[0], same as ESW_LINK_STA_PORT_MASK */
    }
    else
    {
        if (HAL_IS_FE_PORT(unit, port))
        {
            reg_array_read(unit, ESW_FE_PORT_LINK_STATUSr, port, REG_ARRAY_INDEX_NONE, &val);
            osal_time_usleep(20000);
            ret = reg_array_read(unit, ESW_FE_PORT_LINK_STATUSr, port, REG_ARRAY_INDEX_NONE, &val);
        }
        else
        {
            reg_array_read(unit, ESW_GE_PORT_LINK_STATUSr, port, REG_ARRAY_INDEX_NONE,  &val);
            osal_time_usleep(20000);
            ret = reg_array_read(unit, ESW_GE_PORT_LINK_STATUSr, port, REG_ARRAY_INDEX_NONE,  &val);
        }
    }
    /* get value from CHIP*/
    if (ret != RT_ERR_OK)
    {
        dal_esw_switch_sofware_reset_sem(unit, 1); /* UNLOCK */
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    /* Unlock the switch software reset semaphore */
    dal_esw_switch_sofware_reset_sem(unit, 1); /* UNLOCK */
    PORT_SEM_UNLOCK(unit);
   
    /* translate chip's value to definition */
    if (val & ESW_LINK_STA_PORT_MASK)
    {
        *pStatus = PORT_LINKUP;
    } 
    else 
    {
        *pStatus = PORT_LINKDOWN;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pStatus=%d", *pStatus);
    
    return RT_ERR_OK;
} /* end of dal_esw_port_link_get */

/* Function Name:
 *      dal_esw_port_linkMedia_get
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
 *          - PORT_LINKDOWN
 *          - PORT_LINKUP
 *      (2) The media type of the port is as following:
 *          - PORT_MEDIA_COPPER
 *          - PORT_MEDIA_FIBER
 *      (3) When the link status is link-down, the return media should be ignored.
 */
int32
dal_esw_port_linkMedia_get(uint32 unit, rtk_port_t port, rtk_port_linkStatus_t *pStatus, rtk_port_media_t *pMedia)
{
    uint32  val;
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pStatus), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pMedia), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);

    PORT_SEM_LOCK(unit);
    
    if (HAL_IS_FE_PORT(unit, port))
    {
        reg_array_read(unit, ESW_FE_PORT_LINK_STATUSr, port, REG_ARRAY_INDEX_NONE, &val);
        osal_time_usleep(10000);
        ret = reg_array_read(unit, ESW_FE_PORT_LINK_STATUSr, port, REG_ARRAY_INDEX_NONE, &val);
    }
    else
    {
        reg_array_read(unit, ESW_GE_PORT_LINK_STATUSr, port, REG_ARRAY_INDEX_NONE,  &val);
        osal_time_usleep(10000);
        ret = reg_array_read(unit, ESW_GE_PORT_LINK_STATUSr, port, REG_ARRAY_INDEX_NONE,  &val);
    }
    /* get value from CHIP*/
    if (ret != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);
   
    /* translate chip's value to definition */
    
    if (val & ESW_LINK_STA_PORT_MASK)
    {
        *pStatus = PORT_LINKUP;
        if (val & ESW_LINK_MEDIA_PORT_MASK)
            *pMedia = PORT_MEDIA_FIBER;
        else
            *pMedia = PORT_MEDIA_COPPER;
    } 
    else 
    {
        *pStatus = PORT_LINKDOWN;
        *pMedia = PORT_MEDIA_COPPER;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pStatus=%d, pMedia=%d", *pStatus, *pMedia);
    
    return RT_ERR_OK;
} /* end of dal_esw_port_linkMedia_get */

/* Function Name:
 *      dal_esw_port_speedDuplex_get
 * Description:
 *      Get the negotiated port speed and duplex status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pSpeed  - pointer to the port speed
 *      pDuplex - pointer to the port duplex
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 *      RT_ERR_PORT_LINKDOWN - link down port status
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
dal_esw_port_speedDuplex_get(
    uint32            unit,
    rtk_port_t        port,
    rtk_port_speed_t  *pSpeed,
    rtk_port_duplex_t *pDuplex)
{
    uint32  val, tmpVal;
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pSpeed), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pDuplex), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);

    if (HAL_IS_FE_PORT(unit, port))
        ret = reg_array_read(unit, ESW_FE_PORT_LINK_STATUSr, port, REG_ARRAY_INDEX_NONE, &val);
    else
        ret = reg_array_read(unit, ESW_GE_PORT_LINK_STATUSr, port, REG_ARRAY_INDEX_NONE, &val);

    /* get value from CHIP*/
    if (ret != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);
   
    /* translate chip's value to definition */
    tmpVal = (val & ESW_SPD_STA_PORT_MASK)  >> ESW_SPD_STA_PORT;
    if (tmpVal == 0)
    {
        *pSpeed = PORT_SPEED_10M;
    } 
    else if (tmpVal == 1)
    {
        *pSpeed = PORT_SPEED_100M;
    }
    else
    {
        *pSpeed = PORT_SPEED_1000M;
    } 

    tmpVal = (val & ESW_DUP_STA_PORT_MASK)  >> ESW_DUP_STA_PORT;
    if (tmpVal == 0)
    {
        *pDuplex = PORT_HALF_DUPLEX;
    } 
    else
    {
        *pDuplex = PORT_FULL_DUPLEX;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pSpeed=%d, pDuplex=%d", *pSpeed, *pDuplex);
    
    return RT_ERR_OK;
} /* end of dal_esw_port_speedDuplex_get */

/* Function Name:
 *      dal_esw_port_flowctrl_get
 * Description:
 *      Get the negotiated flow control status of the specific port
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pTxStatus - pointer to the negotiation result of the Tx flow control
 *      pRxStatus - pointer to the negotiation result of the Rx flow control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 *      RT_ERR_PORT_LINKDOWN - link down port status
 * Note:
 *      None
 */
int32
dal_esw_port_flowctrl_get(
    uint32      unit,
    rtk_port_t  port,
    uint32      *pTxStatus,
    uint32      *pRxStatus)
{
    uint32  val, tmpVal;
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pTxStatus), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pRxStatus), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if (HAL_IS_FE_PORT(unit, port))
        ret = reg_array_read(unit, ESW_FE_PORT_LINK_STATUSr, port, REG_ARRAY_INDEX_NONE, &val);
    else
        ret = reg_array_read(unit, ESW_GE_PORT_LINK_STATUSr, port, REG_ARRAY_INDEX_NONE, &val);
        
    if (ret != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);
   
    /* translate chip's value to definition */
    tmpVal = (val & ESW_TX_PAUSE_PORT_MASK)  >> ESW_TX_PAUSE_PORT;
    if (tmpVal == 0)
    {
        *pTxStatus = DISABLED;
    } 
    else
    {
        *pTxStatus = ENABLED;
    }

    tmpVal = (val & ESW_RX_PAUSE_PORT_MASK)  >> ESW_RX_PAUSE_PORT;
    if (tmpVal == 0)
    {
        *pRxStatus = DISABLED;
    } 
    else
    {
        *pRxStatus = ENABLED;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pTxStatus=%d, pRxStatus=%d", *pTxStatus, *pRxStatus);
    
    return RT_ERR_OK;
} /* end of dal_esw_port_flowctrl_get */

/* Function Name:
 *      dal_esw_port_phyAutoNegoEnable_get
 * Description:
 *      Get PHY ability of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to PHY auto negotiation status
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
dal_esw_port_phyAutoNegoEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    uint32  value = 0;
    uint32  phyData0;
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
        if ((ret = reg_read(unit, (uint32)portIntraSerdesPage0Reg0_regidx[port-ge_fiber_base]
                            , &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }

        if (((value >> 12) & 0x1) == 1)
            *pEnable = ENABLED;
        else
            *pEnable = DISABLED;

        PORT_SEM_UNLOCK(unit);
        return ret;
    }

    PORT_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = hal_miim_read(unit, port, 0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }
            
    PORT_SEM_UNLOCK(unit);
    
    if (phyData0 & AutoNegotiationEnable_MASK)
    {
        *pEnable = ENABLED;
    }
    else
    {
        *pEnable = DISABLED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pEnable=%d", *pEnable); 
    
    return RT_ERR_OK;
} /* end of dal_esw_port_phyAutoNegoEnable_get */


/* Function Name:
 *      dal_esw_port_phyAutoNegoEnable_set
 * Description:
 *      Set PHY ability of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable PHY auto negotiation
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - input parameter out of range
 * Note:
 *      1. ENABLED : switch to PHY auto negotiation mode
 *         DISABLED: switch to PHY force mode
 *      2. Once the abilities of both auto-nego and force mode are set,
 *         you can freely swtich the mode without calling ability setting API again
 */
int32
dal_esw_port_phyAutoNegoEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    uint32  reg_idx, val, reg0, reg4, temp;
    int32   ret;
    rtk_port_phy_ability_t ability;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, enable=%d", 
           unit, port, enable);     
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(enable >= RTK_ENABLE_END, RT_ERR_INPUT);    
    
    /* Fiber port read from intra serdes registers, not PHY */
    if (HAL_IS_SERDES_PORT(unit, port))
    {
        uint32  ge_fiber_base = 0;

        ge_fiber_base = HAL_GET_MIN_SERDES_PORT(unit);
        PORT_SEM_LOCK(unit);
        if ((ret = reg_read(unit, (uint32)portIntraSerdesPage0Reg0_regidx[port-ge_fiber_base]
                            , &reg0)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }

        if (((reg0 >> 12) & 0x1) == enable)
        {   /* No changed */
            PORT_SEM_UNLOCK(unit);
            return RT_ERR_OK;
        }
        /* Changed */
        if (DISABLED == enable)
        {
            if (pPhy_info[unit]->force_mode_duplex[port] == PORT_HALF_DUPLEX)
                reg0 &= ~(1 << 8);
            else
                reg0 |= (1 << 8);
            if (pPhy_info[unit]->force_mode_speed[port] == PORT_SPEED_100M)
            {
                reg0 &= ~(0x1 << 6);
                reg0 |= (0x1 << 13);
            }
            else
            {
                reg0 |= (0x1 << 6);
                reg0 &= ~(0x1 << 13);
            }

            reg0 &= ~(0x1 << 12);

            if ((ret = reg_read(unit, (uint32)portIntraSerdesPage0Reg4_regidx[port-ge_fiber_base]
                            , &reg4)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
                return ret;
            }

            if (ENABLED == pPhy_info[unit]->force_mode_flowControl[port])
                reg4 |= (0x3 << 7);
            else
                reg4 &= ~(0x3 << 7);

            if ((ret = reg_write(unit, (uint32)portIntraSerdesPage0Reg4_regidx[port-ge_fiber_base]
                                , &reg4)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
                return ret;
            }

            if ((ret = reg_write(unit, (uint32)portIntraSerdesPage0Reg0_regidx[port-ge_fiber_base]
                                , &reg0)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
                return ret;
            }
        }
        else
        {
            if ((ret = reg_read(unit, (uint32)portIntraSerdesPage0Reg4_regidx[port-ge_fiber_base]
                                , &reg4)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
                return ret;
            }

            if (ENABLED == pPhy_info[unit]->auto_mode_asy_pause[port])
                reg4 |= (1 << 8);
            else
                reg4 &= ~(1 << 8);
    
            if (ENABLED == pPhy_info[unit]->auto_mode_pause[port])
                reg4 |= (1 << 7);
            else
                reg4 &= ~(1 << 7);

            if ((ret = reg_write(unit, (uint32)portIntraSerdesPage0Reg4_regidx[port-ge_fiber_base]
                                , &reg4)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
                return ret;
            }

            reg0 |= (0x1 << 12);
            reg0 |= (0x1 << 9);

            if ((ret = reg_write(unit, (uint32)portIntraSerdesPage0Reg0_regidx[port-ge_fiber_base]
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

    PORT_SEM_LOCK(unit);
                  
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

        if (ENABLED == pPhy_info[unit]->force_mode_flowControl[port])
        {
            ability.FC = ENABLED;
            ability.AsyFC = ENABLED;
        }
        else
        {
            ability.FC = DISABLED;
            ability.AsyFC = DISABLED;
        }

        /* Need to configure the port property also when PHY is force mode */
        if (port <= 23)
            reg_idx = ESW_FE_PORT_PROPERTY_CONFIGUREr;
        else
            reg_idx = ESW_GE_PORT_PROPERTY_CONFIGUREr;

        if ((ret = reg_array_read(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, &val)) != RT_ERR_OK)
            return ret;
        temp = ability.AsyFC;
        if ((ret = reg_field_set(unit, reg_idx, ESW_EN_ASY_PAUSEf, &temp, &val)) != RT_ERR_OK)
            return ret;
        temp = ability.FC;
        if ((ret = reg_field_set(unit, reg_idx, ESW_EN_PAUSEf, &temp, &val)) != RT_ERR_OK)
            return ret;
        if ((ret = reg_array_write(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, &val)) != RT_ERR_OK)
            return ret;

        if ((ret = phy_autoNegoAbility_set(unit, port, &ability)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }            

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
    else
    {
        if ((ret = phy_autoNegoAbility_get(unit, port, &ability)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }

        if (ENABLED == pPhy_info[unit]->auto_mode_asy_pause[port])
            ability.AsyFC = ENABLED;
        else
            ability.AsyFC = DISABLED;

        if (ENABLED == pPhy_info[unit]->auto_mode_pause[port])
            ability.FC = ENABLED;
        else
            ability.FC = DISABLED;

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

    PORT_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_port_phyAutoNegoAbility_set */

/* Function Name:
 *      dal_esw_port_phyAutoNegoAbility_get
 * Description:
 *      Get PHY auto negotiation ability of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pAbility - pointer to the PHY ability
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
dal_esw_port_phyAutoNegoAbility_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_port_phy_ability_t  *pAbility)
{
    uint32  reg0, reg4;
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
        if ((ret = reg_read(unit, (uint32)portIntraSerdesPage0Reg0_regidx[port-ge_fiber_base]
                            , &reg0)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }

        enable = (reg0 >> 12) & 0x1;

        osal_memset(pAbility, 0, sizeof(rtk_port_phy_ability_t));

        if ((ret = reg_read(unit, (uint32)portIntraSerdesPage0Reg4_regidx[port-ge_fiber_base]
                        , &reg4)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }

        pAbility->Half_1000 = (reg4 >> 6) & 0x1;
        pAbility->Full_1000 = (reg4 >> 5) & 0x1;
        if (DISABLED == enable)
        {
            pAbility->FC    = pPhy_info[unit]->auto_mode_pause[port];
            pAbility->AsyFC = pPhy_info[unit]->auto_mode_asy_pause[port];
        }
        else
        {
            pAbility->FC    = (reg4 >> 7) & 0x1;
            pAbility->AsyFC = (reg4 >> 8) & 0x1;
        }

        PORT_SEM_UNLOCK(unit);
        return ret;
    }

    if ((ret = dal_esw_port_phyAutoNegoEnable_get(unit, port, &enable)) != RT_ERR_OK)
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
           Half_100=%d, Full_100=%d, Full_1000=%d, FC=%d, AsyFC=%d",
           pAbility->Half_10, pAbility->Full_10, pAbility->Half_100, pAbility->Full_100, 
           pAbility->Full_1000, pAbility->FC, pAbility->AsyFC);     
    
    return RT_ERR_OK;
} /* end of dal_esw_port_phyAutoNegoAbility_get */


/* Function Name:
 *      dal_esw_port_phyAutoNegoAbility_set
 * Description:
 *      Set PHY auto negotiation ability of the specific port
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      pAbility - pointer to the PHY ability
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
dal_esw_port_phyAutoNegoAbility_set(
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
           Full_1000=%d, FC=%d, AsyFC=%d", 
           pAbility->Half_10, pAbility->Full_10, pAbility->Half_100, pAbility->Full_100, 
           pAbility->Full_1000, pAbility->FC, pAbility->AsyFC);   
    
    /* Fiber port read from intra serdes registers, not PHY */
    if (HAL_IS_SERDES_PORT(unit, port))
    {
        uint32  ge_fiber_base = 0;

        ge_fiber_base = HAL_GET_MIN_SERDES_PORT(unit);
        PORT_SEM_LOCK(unit);
        if ((ret = reg_read(unit, (uint32)portIntraSerdesPage0Reg0_regidx[port-ge_fiber_base]
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

        if ((ret = reg_read(unit, (uint32)portIntraSerdesPage0Reg4_regidx[port-ge_fiber_base]
                        , &reg4)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }

        reg4 &= ~(0xF << 5);
        reg4 |= (pAbility->AsyFC << 8) | (pAbility->FC << 7);
        reg4 |= (pAbility->Half_1000 << 6) | (pAbility->Full_1000 << 5);

        if ((ret = reg_write(unit, (uint32)portIntraSerdesPage0Reg4_regidx[port-ge_fiber_base]
                        , &reg4)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }

        if (ENABLED == enable)
        {
            reg0 |= (1 << 9);
            if ((ret = reg_write(unit, (uint32)portIntraSerdesPage0Reg0_regidx[port-ge_fiber_base]
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

    if ((ret = dal_esw_port_phyAutoNegoEnable_get(unit, port, &enable)) != RT_ERR_OK)
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
}    /*end of dal_esw_port_phyAutoNegoAbility_set*/

/* Function Name:
 *      dal_esw_port_phyForceModeAbility_get
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
dal_esw_port_phyForceModeAbility_get(
    uint32              unit,
    rtk_port_t          port,
    rtk_port_speed_t    *pSpeed,
    rtk_port_duplex_t   *pDuplex,
    rtk_enable_t        *pFlowControl)
{
    uint32  value;
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
        PORT_SEM_LOCK(unit);
        if ((ret = reg_read(unit, (uint32)portIntraSerdesPage0Reg0_regidx[port-ge_fiber_base]
                            , &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }

        if ((((value >> 6) & 0x1) == 0) && (((value >> 13) & 0x1) == 1))
            *pSpeed = PORT_SPEED_100M;
        else
            *pSpeed = PORT_SPEED_1000M;

        if (((value >> 8) & 0x1) == 1)
            *pDuplex = PORT_FULL_DUPLEX;
        else
            *pDuplex = PORT_HALF_DUPLEX;

        if ((ret = reg_read(unit, (uint32)portIntraSerdesPage0Reg4_regidx[port-ge_fiber_base]
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

    if ((ret = dal_esw_port_phyAutoNegoEnable_get(unit, port, &enable)) != RT_ERR_OK)
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
}/* end of dal_esw_port_phyForceModeAbility_get */

/* Function Name:
 *      dal_esw_port_phyForceModeAbility_set
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
dal_esw_port_phyForceModeAbility_set(
    uint32              unit,
    rtk_port_t          port,
    rtk_port_speed_t    speed,
    rtk_port_duplex_t   duplex,
    rtk_enable_t        flowControl)
{
    uint32  value = 0;
    uint32  reg_idx, val, temp;
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

        RT_PARAM_CHK((speed != PORT_SPEED_1000M) && (speed != PORT_SPEED_100M), RT_ERR_CHIP_NOT_SUPPORTED);
        RT_PARAM_CHK(duplex != PORT_FULL_DUPLEX, RT_ERR_CHIP_NOT_SUPPORTED);

        ge_fiber_base = HAL_GET_MIN_SERDES_PORT(unit);
        PORT_SEM_LOCK(unit);
        /* config speed and duplex */
        if ((ret = reg_read(unit, (uint32)portIntraSerdesPage0Reg0_regidx[port-ge_fiber_base]
                            , &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }

        if (speed == PORT_SPEED_1000M)
        {
            value |= (0x1 << 6);
            value &= ~(0x1 << 13);
        }
        else
        {
            value &= ~(0x1 << 6);
            value |= (0x1 << 13);
        }
        
        if (duplex)
            value |= (0x1 << 8);
        else
            value &= ~(0x1 << 8);

        if ((ret = reg_write(unit, (uint32)portIntraSerdesPage0Reg0_regidx[port-ge_fiber_base]
                            , &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }

        /* config flow control */
        if ((ret = reg_read(unit, (uint32)portIntraSerdesPage0Reg4_regidx[port-ge_fiber_base]
                            , &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }

        value &= ~(0x3 << 7);
        if (ENABLED == flowControl)
            value |= (0x3 << 7);

        if ((ret = reg_write(unit, (uint32)portIntraSerdesPage0Reg4_regidx[port-ge_fiber_base]
                            , &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }

        PORT_SEM_UNLOCK(unit);
        return ret;
    }

    if ((ret = dal_esw_port_phyAutoNegoEnable_get(unit, port, &enable)) != RT_ERR_OK)
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

        /* Need to configure the port property also when PHY is force mode */
        if (port <= 23)
            reg_idx = ESW_FE_PORT_PROPERTY_CONFIGUREr;
        else
            reg_idx = ESW_GE_PORT_PROPERTY_CONFIGUREr;

        if ((ret = reg_array_read(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, &val)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }
        temp = ability.AsyFC;
        if ((ret = reg_field_set(unit, reg_idx, ESW_EN_ASY_PAUSEf, &temp, &val)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }
        temp = ability.FC;
        if ((ret = reg_field_set(unit, reg_idx, ESW_EN_PAUSEf, &temp, &val)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }
        if ((ret = reg_array_write(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, &val)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }

        if ((ret = phy_autoNegoAbility_set(unit, port, &ability)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }     
               
        /* E0005371 */
        /* Turn off and then turn on the power of port so the partner could detect the speed/duplex change */ 
        if (ENABLED == pMac_info[unit]->admin_enable[port])
        {
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
}/* end of dal_esw_port_phyForceModeAbility_set */

/* Function Name:
 *      dal_esw_port_phyReg_get
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
dal_esw_port_phyReg_get(
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
}/* end of dal_esw_port_phyReg_get */

/* Function Name:
 *      dal_esw_port_phyReg_set
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
dal_esw_port_phyReg_set(
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
}/* end of dal_esw_port_phyReg_set */

/* Function Name:
 *      dal_esw_port_cpuPortId_get
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
dal_esw_port_cpuPortId_get(uint32 unit, rtk_port_t *pPort)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d", unit);   

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pPort), RT_ERR_NULL_POINTER);

    *pPort = HAL_GET_CPU_PORT(unit);
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pPort=%d", *pPort);    
        
    return RT_ERR_OK;
}/* end of dal_esw_port_cpuPortId_get */

/* Function Name:
 *      dal_esw_port_isolation_get
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
dal_esw_port_isolation_get(uint32 unit, rtk_port_t port, rtk_portmask_t *pPortmask)
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
    osal_memset(pPortmask, 0, sizeof(rtk_portmask_t));
    if ((ret = reg_array_field_read(unit, ESW_PORT_ISOLATION_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_TISOf
                        , &pPortmask->bits[0])) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    
    PORT_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pPortmask=0x%x", pPortmask->bits[0]);    
    
    return RT_ERR_OK;
}/* end of dal_esw_port_isolation_get */

/* Function Name:
 *      dal_esw_port_isolation_set
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
dal_esw_port_isolation_set(uint32 unit, rtk_port_t port, rtk_portmask_t portmask)
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
    if ((ret = reg_array_field_write(unit, ESW_PORT_ISOLATION_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_TISOf
                        , &portmask.bits[0])) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    
    PORT_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
}/* end of dal_esw_port_isolation_set */

/* Function Name:
 *      dal_esw_port_isolation_add
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
dal_esw_port_isolation_add(uint32 unit, rtk_port_t port, rtk_port_t iso_port)
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
    
    ret = dal_esw_port_isolation_get(unit, port, &portmask);
    if (RT_ERR_OK != ret)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    
    RTK_PORTMASK_PORT_SET(portmask, iso_port);
    
    ret = dal_esw_port_isolation_set(unit, port, portmask);
    if (RT_ERR_OK != ret)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    
    return RT_ERR_OK;
}/* end of dal_esw_port_isolation_add */

/* Function Name:
 *      dal_esw_port_isolation_del
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
dal_esw_port_isolation_del(uint32 unit, rtk_port_t port, rtk_port_t iso_port)
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
    
    ret = dal_esw_port_isolation_get(unit, port, &portmask);
    if (RT_ERR_OK != ret)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    
    RTK_PORTMASK_PORT_CLEAR(portmask, iso_port);
    
    ret = dal_esw_port_isolation_set(unit, port, portmask);
    if (RT_ERR_OK != ret)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    
    return RT_ERR_OK;
}/* end of dal_esw_port_isolation_del */

/* Function Name:
 *      dal_esw_port_phyComboPortMedia_get
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
dal_esw_port_phyComboPortMedia_get(uint32 unit, rtk_port_t port, rtk_port_media_t *pMedia)
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

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pMedia=%d", *pMedia); 

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_esw_port_phyComboPortMedia_get */

/* Function Name:
 *      dal_esw_port_phyComboPortMedia_set
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
dal_esw_port_phyComboPortMedia_set(uint32 unit, rtk_port_t port, rtk_port_media_t media)
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
} /* end of dal_esw_port_phyComboPortMedia_set */

#if 0 /* The function is move to dal_esw_diag.c */
/* Function Name:
 *      dal_esw_port_macRemoteLoopbackEnable_get
 * Description:
 *      Get the mac remote loopback enable status of the specific port
 * Input:
 *      unit           - unit id
 *      port           - port id
 * Output:
 *      pEnable       - - pointer to the enable status of mac remote loopback
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
dal_esw_port_macRemoteLoopbackEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    uint32  value = 0;
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
    if ((ret = reg_array_field_read(unit, ESW_PORT_L2_MISC0r, port, REG_ARRAY_INDEX_NONE, 
                                    ESW_ENLBKPBf, &value)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    switch(value)
    {
        case 0:
            *pEnable = ENABLED;
            break;
        case 1:
            *pEnable = DISABLED;
            break;
        default:
            break;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pEnable=%d", *pEnable); 

    return RT_ERR_OK;
}/*end of dal_esw_port_macRemoteLoopbackEnable_get*/

/* Function Name:
 *      dal_esw_port_macRemoteLoopbackEnable_set
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
dal_esw_port_macRemoteLoopbackEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    uint32  value = 0;
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, enable=%d", 
           unit, port, enable); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable != DISABLED) && (enable != ENABLED), RT_ERR_INPUT);

    switch(enable)
    {
        case DISABLED:
            value = 1;
            break;
        case ENABLED:
            value = 0;
            break;
        default:
            RT_ERR(RT_ERR_INPUT, (MOD_DAL|MOD_PORT), "");
            return RT_ERR_INPUT;
    }

    PORT_SEM_LOCK(unit);

    /* Set value to Chip*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_L2_MISC0r, port, REG_ARRAY_INDEX_NONE, 
                                    ESW_ENLBKPBf, &value)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /*end of dal_esw_port_macRemoteLoopbackEnable_set*/
#endif

/* Function Name:
 *      dal_esw_port_backpressureEnable_get
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
 *      1. The mac back pressure enable status of the port is as following:
 *         - DISABLE   
 *         - ENABLE
 *      2. Used to support backpressure in half mode.
 */
int32
dal_esw_port_backpressureEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
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
    if((ret = reg_array_field_read(unit, ESW_PORT_L2_MISC0r, port, REG_ARRAY_INDEX_NONE, 
                                    ESW_ENBKPRSf, pEnable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pEnable=%d", *pEnable); 

    return RT_ERR_OK;
} /* end of dal_esw_port_backpressureEnable_get */

/* Function Name:
 *      dal_esw_port_backpressureEnable_set
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
 *      1. The mac back pressure enable status of the port is as following:
 *         - DISABLE   
 *         - ENABLE
 *      2. Used to support backpressure in half mode.
 */
int32
dal_esw_port_backpressureEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
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
    if ((ret = reg_array_field_write(unit, ESW_PORT_L2_MISC0r, port, REG_ARRAY_INDEX_NONE, 
                                    ESW_ENBKPRSf, &enable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_esw_port_backpressureEnable_set */

/* Function Name:
 *      dal_esw_port_linkChange_register
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
dal_esw_port_linkChange_register(uint32 unit, dal_link_change_callback_f link_change_callback)
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
} /* End of dal_esw_port_linkChange_register */

/* Function Name:
 *      dal_esw_port_linkChange_unregister
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
dal_esw_port_linkChange_unregister(uint32 unit)
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
} /* End of dal_esw_port_linkChange_unregister */

/* Function Name:
 *      dal_esw_port_adminEnable_get
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
dal_esw_port_adminEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
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
} /* end of dal_esw_port_adminEnable_get */

/* Function Name:
 *      dal_esw_port_adminEnable_set
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
dal_esw_port_adminEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  reg_idx, value, temp;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, port admin=%d", 
           unit, port, enable); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable != DISABLED) && (enable != ENABLED), RT_ERR_INPUT);

    PORT_SEM_LOCK(unit);

    if (enable == pMac_info[unit]->admin_enable[port])
    {
        /* no change and prevent to configure to chip */
        PORT_SEM_UNLOCK(unit);
        return RT_ERR_OK;
    }

    if (enable == ENABLED)
    {
        value = 1;
        /*Config Rx Enable*/
        if ((ret = reg_array_field_write(unit, ESW_PORT_L2_MISC0r, port, REG_ARRAY_INDEX_NONE, ESW_ENRXf, &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    
#if defined(CONFIG_SDK_WA_88E6063_COMPATIBLE)
        if (HAL_IS_FE_PORT(unit, port))
        {
            /*Config Tx Disable; and will enable by ISR*/
            value = 0;
            if ((ret = reg_array_field_write(unit, ESW_PORT_L2_MISC0r, port, REG_ARRAY_INDEX_NONE, ESW_ENTXf, &value)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }
        }
        else
        {
            /*Config Tx Enable*/
            value = 1;
            if ((ret = reg_array_field_write(unit, ESW_PORT_L2_MISC0r, port, REG_ARRAY_INDEX_NONE, ESW_ENTXf, &value)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }
        }
        pMac_info[unit]->tx_enable[port] = enable;
#else
        /*Config Tx Enable*/
        value = 1;
        if ((ret = reg_array_field_write(unit, ESW_PORT_L2_MISC0r, port, REG_ARRAY_INDEX_NONE, ESW_ENTXf, &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
#endif
        /* port enable */
        /* 1¡^PHY reg0 bit11=0 ¡]power down=0¡^ */
        if (!HAL_IS_SERDES_PORT(unit, port))
        {
            if ((ret = phy_enable_set(unit, port, enable)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }
        }
        /* 2¡^disable MAC force mode */
        if (port <= 23)
            reg_idx = ESW_FE_PORT_PROPERTY_CONFIGUREr;
        else
            reg_idx = ESW_GE_PORT_PROPERTY_CONFIGUREr;
        if ((ret = reg_array_read(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
        temp = 0;
        if ((ret = reg_field_set(unit, reg_idx, ESW_EN_MAC_FORCEf, &temp, &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
        temp = 0;
        if ((ret = reg_field_set(unit, reg_idx, ESW_EN_FORCE_LINKf, &temp, &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
        if ((ret = reg_array_write(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            return ret;
        }
    }
    else
    {
        /*Config Tx Enable*/
        value = 0;
        if ((ret = reg_array_field_write(unit, ESW_PORT_L2_MISC0r, port, REG_ARRAY_INDEX_NONE, ESW_ENTXf, &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
#if defined(CONFIG_SDK_WA_88E6063_COMPATIBLE)
        pMac_info[unit]->tx_enable[port] = enable;
#endif        

        PORT_SPIN_LOCK(unit);
        /* port disable */
        /* 1¡^PHY reg0 bit10=1 ¡]isolate=1¡^ */
        if (HAL_IS_PHY_EXIST(unit, port))
        {
            hal_miim_read(unit, port, 0, 0, &value);
            value = (value|0x400);
            if ((ret = hal_miim_write(unit, port, 0, 0, value)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }
        }
        /* 2¡^delay 1ms, PHY reg0 bit14=1 ¡]digital loopback=1¡^ */
        osal_time_mdelay(1);
        if (HAL_IS_PHY_EXIST(unit, port))
        {
            hal_miim_read(unit, port, 0, 0, &value);
            value = (value|0x4000);
            if ((ret = hal_miim_write(unit, port, 0, 0, value)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }
        }
        /* 3¡^delay 1ms, PHY reg0 bit11=1 ¡]power down=1¡^ */
        osal_time_mdelay(1);
        if (!HAL_IS_SERDES_PORT(unit, port))
        {
            if ((ret = phy_enable_set(unit, port, enable)) != RT_ERR_OK)
            {
                PORT_SPIN_UNLOCK(unit);
                PORT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }
        }
        /* 4¡^delay 1ms, force MAC linkdown */
        osal_time_mdelay(1);
        if (port <= 23)
            reg_idx = ESW_FE_PORT_PROPERTY_CONFIGUREr;
        else
            reg_idx = ESW_GE_PORT_PROPERTY_CONFIGUREr;
        if ((ret = reg_array_read(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, &value)) != RT_ERR_OK)
        {
            PORT_SPIN_UNLOCK(unit);
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
        temp = 1;
        if ((ret = reg_field_set(unit, reg_idx, ESW_EN_MAC_FORCEf, &temp, &value)) != RT_ERR_OK)
        {
            PORT_SPIN_UNLOCK(unit);
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
        temp = 0;
        if ((ret = reg_field_set(unit, reg_idx, ESW_EN_FORCE_LINKf, &temp, &value)) != RT_ERR_OK)
        {
            PORT_SPIN_UNLOCK(unit);
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
        if ((ret = reg_array_write(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, &value)) != RT_ERR_OK)
        {
            PORT_SPIN_UNLOCK(unit);
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
        /* 5¡^PHY reg0 bit14=0 ¡]digital loopback=0¡^ */
        if (HAL_IS_PHY_EXIST(unit, port))
        {
            hal_miim_read(unit, port, 0, 0, &value);
            value = (value&0xffffbfff);
            if ((ret = hal_miim_write(unit, port, 0, 0, value)) != RT_ERR_OK)
            {
                PORT_SPIN_UNLOCK(unit);
                PORT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }
        }
        PORT_SPIN_UNLOCK(unit);
        /* 6¡^PHY reg0 bit10=0 ¡]isolate=0¡^ */
        if (HAL_IS_PHY_EXIST(unit, port))
        {
            hal_miim_read(unit, port, 0, 0, &value);
            value = (value&0xfffffbff);
            if ((ret = hal_miim_write(unit, port, 0, 0, value)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }
        }
    }

    pMac_info[unit]->admin_enable[port] = enable;
    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;

} /* end of dal_esw_port_adminEnable_set */


/* Function Name:
 *      dal_esw_port_txEnable_get
 * Description:
 *      Get the TX enable status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pEnable - pointer to the port TX status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      The TX enable status of the port is as following:
 *      - DISABLED
 *      - ENABLED
 */
int32
dal_esw_port_txEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", 
           unit, port);
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);

    /*Config Tx Enable*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_L2_MISC0r, port, REG_ARRAY_INDEX_NONE, ESW_ENTXf, pEnable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pEnable=%d", *pEnable); 

    return RT_ERR_OK;

} /* end of dal_esw_port_txEnable_get */


/* Function Name:
 *      dal_esw_port_txEnable_set
 * Description:
 *      Set the TX enable status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - MAC TX configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      The TX enable status of the port is as following:
 *      - DISABLED
 *      - ENABLED
 */
int32
dal_esw_port_txEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;  

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, rx enable=%d", 
           unit, port, enable); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable != DISABLED) && (enable != ENABLED), RT_ERR_INPUT);

    PORT_SEM_LOCK(unit);

    if(enable == ENABLED)
        value = 1;
    else
        value = 0;

    /*Config Tx Enable*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_L2_MISC0r, port, REG_ARRAY_INDEX_NONE, ESW_ENTXf, &value)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;

} /* end of dal_esw_port_txEnable_set */


/* Function Name:
 *      dal_esw_port_rxEnable_get
 * Description:
 *      Get the RX enable status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pEnable - pointer to the port RX status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      The RX enable status of the port is as following:
 *      - DISABLED
 *      - ENABLED
 */
int32
dal_esw_port_rxEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", 
           unit, port);
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);

    /*Config Rx Enable*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_L2_MISC0r, port, REG_ARRAY_INDEX_NONE, ESW_ENRXf, pEnable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pEnable=%d", *pEnable); 

    return RT_ERR_OK;

} /* end of dal_esw_port_rxEnable_get */


/* Function Name:
 *      dal_esw_port_rxEnable_set
 * Description:
 *      Set the RX enable status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - MAC RX configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      The RX enable status of the port is as following:
 *      - DISABLED
 *      - ENABLED
 */
int32
dal_esw_port_rxEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;  

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, rx enable=%d", 
           unit, port, enable); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable != DISABLED) && (enable != ENABLED), RT_ERR_INPUT);

    PORT_SEM_LOCK(unit);

    if(enable == ENABLED)
        value = 1;
    else
        value = 0;

    /*Config Rx Enable*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_L2_MISC0r, port, REG_ARRAY_INDEX_NONE, ESW_ENRXf, &value)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;

} /* end of dal_esw_port_rxEnable_set */


#if 0 /* The function is move to dal_esw_diag.c */
/* Module Name    : Port */
/* Sub-module Name: RTCT */

/* Function Name:
 *      dal_esw_port_rtctResult_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NOT_FINISH    - RTCT not finish. Need to wait a while.
 *      RT_ERR_TIMEOUT       - RTCT test timeout in this port.
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      If linkType is PORT_SPEED_1000M, test result will be stored in ge_result. 
 *      If linkType is PORT_SPEED_10M or PORT_SPEED_100M, test result will be stored in fe_result.
 */
int32
dal_esw_port_rtctResult_get(uint32 unit, rtk_port_t port, rtk_rtctResult_t *pRtctResult)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", 
           unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pRtctResult), RT_ERR_NULL_POINTER);
    
    PORT_SEM_LOCK(unit);        

    if (port <= 23)
    {
        if ((ret = phy_rtctResult_get(unit, port, pRtctResult)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }
    else
    {
        /*Config Phy Register*/
        if (!HAL_IS_SERDES_PORT(unit, port))
        {
            if ((ret = phy_rtctResult_get(unit, port, pRtctResult)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }
        }        
    }

    PORT_SEM_UNLOCK(unit);
   
    return RT_ERR_OK;
} /*end of dal_esw_port_rtctResult_get*/

/* Function Name:
 *      dal_esw_port_rtctEnable_set
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
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      None
 */
int32 
dal_esw_port_rtctEnable_set(uint32 unit, rtk_portmask_t *pPortmask)
{
    int32 ret = RT_ERR_FAILED;
    hal_control_t *pHal_control;    
    rtk_port_t portId;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d", 
           unit); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pPortmask), RT_ERR_NULL_POINTER);
        
    if ((pHal_control = hal_ctrlInfo_get(unit)) == NULL)
    {
        return RT_ERR_FAILED;
    }

    PORT_SEM_LOCK(unit);

    /*init Port 0-23*/
    for(portId = HAL_GET_MIN_FE_PORT(unit); portId <= HAL_GET_MAX_FE_PORT(unit); portId++)
    {
        if(RTK_PORTMASK_IS_PORT_SET(*pPortmask, portId))
        {
            if (pMac_info[unit]->admin_enable[portId] != ENABLED)
            {
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "Port %d need be enabled first", portId);
                PORT_SEM_UNLOCK(unit);
                return RT_ERR_FAILED;
            }

            if ((ret = phy_rtct_start(unit, portId)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }
        }
    }

    /*init Port 24-27*/
    for(portId = HAL_GET_MIN_GE_PORT(unit); portId <= HAL_GET_MAX_GE_PORT(unit); portId++)
    {    
        if(RTK_PORTMASK_IS_PORT_SET(*pPortmask, portId))
        {
            /*Config Phy Register*/
            if (!HAL_IS_SERDES_PORT(unit, portId))
            {
                if ((ret = phy_rtct_start(unit, portId)) != RT_ERR_OK)
                {
                    PORT_SEM_UNLOCK(unit);
                    RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                    return ret;
                }
            }
        }  
    }

    PORT_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_port_rtctEnable_set */
#endif

/* Module Name    : Port */
/* Sub-module Name: UDLD */

/* Function Name:
 *      dal_esw_port_udldEnable_get
 * Description:
 *      Get enable status of UDLD on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of UDLD
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
dal_esw_port_udldEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", 
           unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);


    PORT_SEM_LOCK(unit);
    if ((ret = reg_array_field_read(unit, ESW_UDLD_PER_PORT_CONTROLr, port, 
                    REG_ARRAY_INDEX_NONE, ESW_P_UDLD_ENf, pEnable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pEnable=%d", *pEnable); 
    
    return RT_ERR_OK;
}   /*end of dal_esw_port_udldEnable_get*/

/* Function Name:
 *      dal_esw_port_udldEnable_set
 * Description:
 *      Set enable status of UDLD on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of UDLD
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_port_udldEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, enable=%d", 
           unit, port, enable); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);


    PORT_SEM_LOCK(unit);
    if ((ret = reg_array_field_write(unit, ESW_UDLD_PER_PORT_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_P_UDLD_ENf, &enable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    PORT_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
}   /*end of dal_esw_port_udldEnable_set */

/* Function Name:
 *      dal_esw_port_udldLinkUpAutoTriggerEnable_get
 * Description:
 *      Get enable status of link up auto trigger UDLD test.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of link up auto trigger UDLD
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
dal_esw_port_udldLinkUpAutoTriggerEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d", unit); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);
    if ((ret = reg_field_read(unit, ESW_UDLD_GLOBAL_CONTROLr, ESW_LINKUP_AUTO_TRIG_ENf, pEnable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pEnable=%d", *pEnable); 
    
    return RT_ERR_OK;
}   /*end of dal_esw_port_udldLinkUpAutoTriggerEnable_get*/

/* Function Name:
 *      dal_esw_port_udldLinkUpAutoTriggerEnable_set
 * Description:
 *      Set enable status of link up auto trigger UDLD test.
 * Input:
 *      unit   - unit id
 *      enable - enable status of link up auto trigger UDLD
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
dal_esw_port_udldLinkUpAutoTriggerEnable_set(uint32 unit, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, enable=%d", unit, enable); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);

    PORT_SEM_LOCK(unit);
    if ((ret = reg_field_write(unit, ESW_UDLD_GLOBAL_CONTROLr, ESW_LINKUP_AUTO_TRIG_ENf, &enable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    PORT_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
}/*end of dal_esw_port_udldLinkUpAutoTriggerEnable_set*/

/* Function Name:
 *      dal_esw_port_udldTrigger_start
 * Description:
 *      Trigger UDLD test on specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 * Note:
 *      None
 */
int32
dal_esw_port_udldTrigger_start(uint32 unit, rtk_port_t port)
{
    uint32  val;
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", 
           unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);

    PORT_SEM_LOCK(unit);
    val = ENABLED;
    if ((ret = reg_array_field_write(unit, ESW_UDLD_CPU_TRIGGER_CONTROLr, port, 
                    REG_ARRAY_INDEX_NONE, ESW_P_TRIGGERf, &val)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    PORT_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
}   /*end of dal_esw_port_udldTrigger_start*/

/* Function Name:
 *      dal_esw_port_udldAutoDisableFailedPortEnable_get
 * Description:
 *      Get UDLD test status of specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pStatus - pointer to UDLD test status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      UDLD test status is as following
 *      - UDLD_UNIDIR           UDLD test result is unidirection
 *      - UDLD_BIDIR            UDLD test result is bidirection
 *      - UDLD_UNDETERMINE      UDLD test is on going. Wait more time to get result.
 */
int32
dal_esw_port_udldStatus_get(uint32 unit, rtk_port_t port, rtk_port_udldStatus_t *pStatus)
{
    int32   ret;
    uint32 success_flag;
    uint32 udld_ip;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", 
           unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pStatus), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);
    if ((ret = reg_read(unit, ESW_UDLD_CPU_TRIGGER_STATUSr, &success_flag)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    
    if ((ret = reg_array_field_read(unit, ESW_UDLD_CPU_TRIGGER_STATUSr, port, 
            REG_ARRAY_INDEX_NONE, ESW_P_SUCCESS_FLAGf, &success_flag)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    
    if ((ret = reg_field_read(unit, ESW_UDLD_INTERRUPT_STATUSr, ESW_UDLD_IP0f - port, &udld_ip)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    if(success_flag == TRUE)
    {
        *pStatus = UDLD_BIDIR;
    }
    else if(udld_ip == TRUE)
    {
        *pStatus = UDLD_UNIDIR;
    }
    else
    {
        *pStatus = UDLD_UNDETERMINE;        
    }        
        
    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pStatus=%d", *pStatus); 

    return RT_ERR_OK;  

}   /*end of dal_esw_port_udldStatus_get*/

/* Function Name:
 *      dal_esw_port_udldAutoDisableFailedPortEnable_get
 * Description:
 *      Get enable status of auto disable UDLD failed port.
 *      If enabled, switch will automatically disable unidirectional port.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of auto disabled UDLD failed port
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
dal_esw_port_udldAutoDisableFailedPortEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d", unit); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);
    if ((ret = reg_field_read(unit, ESW_UDLD_GLOBAL_CONTROLr, ESW_EN_AUTO_DISABLEf, pEnable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pEnable=%d", *pEnable); 
    
    return RT_ERR_OK;
}   /*end of dal_esw_port_udldAutoDisableFailedPortEnable_get*/

/* Function Name:
 *      dal_esw_port_udldAutoDisableFailedPortEnable_set
 * Description:
 *      Set enable status of auto disable UDLD failed port.
 *      If enabled, switch will automatically disable unidirectional port.
 * Input:
 *      unit   - unit id
 *      enable - enable status of auto disabled UDLD failed port
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
dal_esw_port_udldAutoDisableFailedPortEnable_set(uint32 unit, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, enable=%d", unit, enable); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);

    PORT_SEM_LOCK(unit);
    if ((ret = reg_field_write(unit, ESW_UDLD_GLOBAL_CONTROLr, ESW_EN_AUTO_DISABLEf, &enable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    PORT_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
}   /*end of dal_esw_port_udldAutoDisableFailedPortEnable_set*/

/* Function Name:
 *      dal_esw_port_udldInterval_get
 * Description:
 *      Get UDLD echo generate interval.
 * Input:
 *      unit      - unit id
 * Output:
 *      pInterval - pointer to interval of UDLD echo generate (unit: second)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Following value is available on 8328M:
 *      2 second, 4 second, 8 second, 16 second
 */
int32
dal_esw_port_udldInterval_get(uint32 unit, rtk_port_udldInterval_t *pInterval)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d", unit); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pInterval), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);
    if ((ret = reg_field_read(unit, ESW_UDLD_GLOBAL_CONTROLr, ESW_TINTERVALf, pInterval)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    PORT_SEM_UNLOCK(unit);

    switch(*pInterval)
    {
        case 0:
            *pInterval = UDLD_INTERVAL_2S;
            break;
        case 1:
            *pInterval = UDLD_INTERVAL_4S;
            break;
        case 2:
            *pInterval = UDLD_INTERVAL_8S;
            break;
        case 3:
            *pInterval = UDLD_INTERVAL_16S;
            break;    
        default:
            break;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pInterval=%d", *pInterval); 
    
    return RT_ERR_OK;
}   /*end of dal_esw_port_udldInterval_get*/

/* Function Name:
 *      dal_esw_port_udldInterval_set
 * Description:
 *      Set UDLD echo generate interval.
 * Input:
 *      unit     - unit id
 *      interval - interval of UDLD echo generate (unit: second)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      Following value is available on 8328M:
 *      2 second, 4 second, 8 second, 16 second
 */
int32
dal_esw_port_udldInterval_set(uint32 unit, rtk_port_udldInterval_t interval)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, interval=%d", unit, interval); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((interval >= UDLD_INTERVAL_END), RT_ERR_INPUT);

    PORT_SEM_LOCK(unit);
    if ((ret = reg_field_write(unit, ESW_UDLD_GLOBAL_CONTROLr, ESW_TINTERVALf, &interval)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    PORT_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
}   /*end of dal_esw_port_udldInterval_set*/

/* Function Name:
 *      dal_esw_port_udldRetryCount_get
 * Description:
 *      Get retry count of UDLD.
 * Input:
 *      unit        - unit id
 * Output:
 *      pRetryCount - pointer to retry count of UDLD
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Following retryCount is available in 8328M:
 *      1, 2, 4, 8
 */
int32
dal_esw_port_udldRetryCount_get(uint32 unit, uint32 *pRetryCount)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d", unit); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pRetryCount), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);
    if ((ret = reg_field_read(unit, ESW_UDLD_GLOBAL_CONTROLr, ESW_TRYTIMESf, pRetryCount)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    PORT_SEM_UNLOCK(unit);

    switch(*pRetryCount)
    {
        case 0:
            *pRetryCount = 1;
            break;
        case 1:
            *pRetryCount = 2;
            break;
        case 2:
            *pRetryCount = 4;
            break;
        case 3:
            *pRetryCount = 8;
            break;   
        default:
            break;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pRetryCount=%d", *pRetryCount); 
    
    return RT_ERR_OK;
}   /*end of dal_esw_port_udldRetryCount_get*/

/* Function Name:
 *      dal_esw_port_udldRetryCount_set
 * Description:
 *      Set retry count of UDLD.
 * Input:
 *      unit       - unit id
 *      retryCount - retry count of UDLD
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Following retryCount is available in 8328M:
 *      1, 2, 4, 8
 */
int32
dal_esw_port_udldRetryCount_set(uint32 unit, uint32 retryCount)
{
    int32   ret = RT_ERR_FAILED;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, retryCount=%d", unit, retryCount); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    switch(retryCount)
    {
        case 1:
            retryCount = 0;
            break;
        case 2:
            retryCount = 1;
            break;
        case 4:
            retryCount = 2;
            break;
        case 8:
            retryCount = 3;
            break;   
        default:
            RT_ERR(RT_ERR_INPUT, (MOD_DAL|MOD_PORT), "");
            return RT_ERR_INPUT;
    }

    PORT_SEM_LOCK(unit);
    if ((ret = reg_field_write(unit, ESW_UDLD_GLOBAL_CONTROLr, ESW_TRYTIMESf, &retryCount)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    PORT_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
}   /*end of dal_esw_port_udldRetryCount_set*/

/* Function Name:
 *      dal_esw_port_udldLedIndicateEnable_get
 * Description:
 *      Get enable status of LED indication when unidirectional link detection.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of LED indication
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
dal_esw_port_udldLedIndicateEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d", unit); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);
    if ((ret = reg_field_read(unit, ESW_UDLD_GLOBAL_CONTROLr, ESW_UDLD_LED_INDICATION_ENf, pEnable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pEnable=%d", *pEnable); 
    
    return RT_ERR_OK;
}   /*end of dal_esw_port_udldLedIndicateEnable_get*/

/* Function Name:
 *      dal_esw_port_udldLedIndicateEnable_set
 * Description:
 *      Set enable status of LED indication when unidirectional link detection.
 * Input:
 *      unit   - unit id
 *      enable - enable status of LED indication
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
dal_esw_port_udldLedIndicateEnable_set(uint32 unit, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, enable=%d", unit, enable); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);
    
    PORT_SEM_LOCK(unit);
    if ((ret = reg_field_write(unit, ESW_UDLD_GLOBAL_CONTROLr, ESW_UDLD_LED_INDICATION_ENf, &enable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    PORT_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
}   /*end of dal_esw_port_udldLedIndicateEnable_set*/

/* Function Name:
 *      dal_esw_port_udldEchoAction_get
 * Description:
 *      Get udld echo packet action.
 * Input:
 *      unit   - unit id
 * Output:
 *      pAction - echo packet action
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
dal_esw_port_udldEchoAction_get(uint32 unit, rtk_port_udldEchoAction_t *pAction)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d", 
           unit); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);

    if ((ret = reg_field_read(unit, ESW_UDLD_GLOBAL_CONTROLr, 
                         ESW_ECHO_ACTIONf, pAction)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }    
   
    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pAction=%d", *pAction); 

    return RT_ERR_OK;
} /*end of dal_esw_port_udldEchoAction_get*/

/* Function Name:
 *      dal_esw_port_udldEchoAction_set
 * Description:
 *      Set udld echo packet action.
 * Input:
 *      unit   - unit id
 *      action - echo packet action
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
dal_esw_port_udldEchoAction_set(uint32 unit, rtk_port_udldEchoAction_t action)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, action=%d", 
           unit, action); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((action >= UDLD_ACTION_END), RT_ERR_INPUT);

    PORT_SEM_LOCK(unit);

    if ((ret = reg_field_write(unit, ESW_UDLD_GLOBAL_CONTROLr, 
                         ESW_ECHO_ACTIONf, &action)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }    
   
    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /*end of dal_esw_port_udldEchoAction_set*/
 

/* Function Name:
 *      dal_esw_port_udldLinkStatus_get
 * Description:
 *      Get udld port link status.
 * Input:
 *      unit   - unit id
 *      port  - port id
 * Output:
 *      pStatus - link status
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
dal_esw_port_udldLinkStatus_get(uint32 unit, rtk_port_t port, rtk_port_udldLinkStatus_t *pStatus)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", 
           unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pStatus), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_INPUT);

    PORT_SEM_LOCK(unit);

    if ((ret = reg_array_field_read(unit, ESW_UDLD_PER_PORT_STATUSr, port,
                         REG_ARRAY_INDEX_NONE, ESW_P_STATUSf, pStatus)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }    
   
    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pStatus=%d", *pStatus); 

    return RT_ERR_OK;
} /*end of dal_esw_port_udldLinkStatus_get*/

/* Function Name:
 *      dal_esw_port_udldLinkStatus_set
 * Description:
 *      Set udld link status.
 * Input:
 *      unit   - unit id
 *      port  - port id
 *      status - link status
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
dal_esw_port_udldLinkStatus_set(uint32 unit, rtk_port_t port, rtk_port_udldLinkStatus_t status)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, status=%d", 
           unit, port, status); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((status >= UDLD_LINK_STATUS_END), RT_ERR_INPUT);
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_INPUT);

    PORT_SEM_LOCK(unit);

    if ((ret = reg_array_field_write(unit, ESW_UDLD_PER_PORT_STATUSr, port,
                         REG_ARRAY_INDEX_NONE, ESW_P_STATUSf, &status)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }    
   
    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /*end of dal_esw_port_udldLinkStatus_get*/


/* Module Name    : Port */
/* Sub-module Name: RLDP */

 /* Function Name:
 *      dal_esw_port_rldpEnable_get
 * Description:
 *      Get enable status of RLDP on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of RLDP
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
dal_esw_port_rldpEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", 
           unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);
    if ((ret = reg_array_field_read(unit, ESW_PORT_REALTEK_SELF_LOOP_DETECTION_CONTROLr, 
                        port, REG_ARRAY_INDEX_NONE, ESW_RSLDENf, pEnable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pEnable=%d", *pEnable); 
    
    return RT_ERR_OK;
}   /*end of dal_esw_port_rldpEnable_get*/
 
 /* Function Name:
 *      dal_esw_port_rldpEnable_set
 * Description:
 *      Set enable status of RLDP on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of RLDP
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
dal_esw_port_rldpEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, enable=%d", 
           unit, port, enable); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);

    PORT_SEM_LOCK(unit);
    if ((ret = reg_array_field_write(unit, ESW_PORT_REALTEK_SELF_LOOP_DETECTION_CONTROLr, 
                        port, REG_ARRAY_INDEX_NONE, ESW_RSLDENf, &enable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    PORT_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
}   /*end of dal_esw_port_rldpEnable_set*/

/* Function Name:
 *      dal_esw_port_rldpStatus_get
 * Description:
 *      Get result of RLDP test on specified port.
 * Input:
 *      unit          - unit id
 *      port          - port id
 * Output:
 *      pNormalStatus - Status of normal loop test
 *      pSelfStatus   - Status of self loop test
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Result of RLDP self loop is as following
 *      - RLDP_FORWARDING
 *      - RLDP_LISTEN
 *      - RLDP_BLOCK
 *
 *      Result of RLDP normal loop is as following
 *      - RLDP_NORMAL_LOOP
 *      - RLDP_NO_NORMAL_LOOP
 */
int32
dal_esw_port_rldpStatus_get(
    uint32                          unit, 
    rtk_port_t                      port, 
    rtk_port_rldpNormalStatus_t     *pNormalStatus, 
    rtk_port_rldpSelfStatus_t       *pSelfStatus)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", 
           unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pNormalStatus), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pSelfStatus), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);

    if ((ret = reg_array_field_read(unit, ESW_PORT_REALTEK_SELF_LOOP_DETECTION_CONTROLr, 
                        port, REG_ARRAY_INDEX_NONE, ESW_NLFLAGf, pNormalStatus)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }    
    
    if ((ret = reg_array_field_read(unit, ESW_PORT_REALTEK_SELF_LOOP_DETECTION_CONTROLr, 
                        port, REG_ARRAY_INDEX_NONE, ESW_SLSTATEf, pSelfStatus)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    PORT_SEM_UNLOCK(unit);

    if(*pNormalStatus)
        *pNormalStatus =RLDP_NORMAL_LOOP;
    else
        *pNormalStatus =RLDP_NO_NORMAL_LOOP;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pNormalStatus=d, pSelfStatus=%d", 
                    *pNormalStatus, pSelfStatus); 
    
    return RT_ERR_OK;
}   /*end of dal_esw_port_rldpEnable_get*/

/* Function Name:
 *      dal_esw_port_rldpAutoBlockEnable_get
 * Description:
 *      Get enable status of auto blocking self loop port on specified port.
 *      When enabled, switch will automatically block self loop port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of auto blocking self loop port
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
dal_esw_port_rldpAutoBlockEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", 
           unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);
    if ((ret = reg_array_field_read(unit, ESW_PORT_REALTEK_SELF_LOOP_DETECTION_CONTROLr, 
                        port, REG_ARRAY_INDEX_NONE, ESW_LPSLPAUTOBLKf, pEnable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pEnable=%d", *pEnable); 
    
    return RT_ERR_OK;
}   /*end of dal_esw_port_rldpAutoBlockEnable_get*/
   
/* Function Name:
 *      dal_esw_port_rldpAutoBlockEnable_set
 * Description:
 *      Set enable status of auto blocking self loop port on specified port.
 *      When enabled, switch will automatically block self loop port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of auto blocking self loop port
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
dal_esw_port_rldpAutoBlockEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, enable=%d", 
           unit, port, enable); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);

    PORT_SEM_LOCK(unit);
    if ((ret = reg_array_field_write(unit, ESW_PORT_REALTEK_SELF_LOOP_DETECTION_CONTROLr, 
                        port, REG_ARRAY_INDEX_NONE, ESW_LPSLPAUTOBLKf, &enable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    PORT_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
}   /*end of dal_esw_port_rldpEnable_set*/

/* Function Name:
 *      dal_esw_port_rldpInterval_get
 * Description:
 *      Get interval of RLDP hello interval on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pInterval - pointer to interval of RLDP hello interval(unit: second)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Following is available interval for 8328M
 *      1, 2, 4, 8 second
 */
int32
dal_esw_port_rldpInterval_get(uint32 unit, rtk_port_t port, uint32 *pInterval)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d", unit); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pInterval), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);
    if ((ret = reg_array_field_read(unit, ESW_PORT_REALTEK_SELF_LOOP_DETECTION_CONTROLr, 
                    port, REG_ARRAY_INDEX_NONE, ESW_HELLOTIMEf, pInterval)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    PORT_SEM_UNLOCK(unit);

    switch(*pInterval)
    {
        case 0:
            *pInterval = 1;
            break;
        case 1:
            *pInterval = 2;
            break;
        case 2:
            *pInterval = 4;
            break;
        case 3:
            *pInterval = 8;
            break; 
        default:
            break;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pInterval=%d", *pInterval); 
    
    return RT_ERR_OK;
}   /*end of dal_esw_port_rldpInterval_get*/

/* Function Name:
 *      dal_esw_port_rldpInterval_set
 * Description:
 *      Set interval of RLDP hello interval on specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      interval - interval of RLDP hello interval(unit: second)
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
 *      Following is available interval for 8328M
 *      1, 2, 4, 8 second
 */
int32
dal_esw_port_rldpInterval_set(uint32 unit, rtk_port_t port, uint32 interval)
{
    int32   ret = RT_ERR_FAILED;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, interval=%d", unit, interval); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    switch(interval)
    {
        case 1:
            interval = 0;
            break;
        case 2:
            interval = 1;
            break;
        case 4:
            interval = 2;
            break;
        case 8:
            interval = 3;
            break;   
        default:
            RT_ERR(RT_ERR_INPUT, (MOD_DAL|MOD_PORT), "");
            return RT_ERR_INPUT;
    }

    PORT_SEM_LOCK(unit);
    if ((ret = reg_array_field_write(unit, ESW_PORT_REALTEK_SELF_LOOP_DETECTION_CONTROLr, 
                    port, REG_ARRAY_INDEX_NONE, ESW_HELLOTIMEf, &interval)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    PORT_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
}   /*end of dal_esw_port_rldpInterval_set*/

/* Function Name:
 *      dal_esw_port_rldpSelfLoopAgingTime_get
 * Description:
 *      Get aging time of self loop on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pAgingTime - pointer to aging time of self loop(unit: second)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Following is available aging time for 8328M
 *      1, 2, 4, 8 second
 */
int32
dal_esw_port_rldpSelfLoopAgingTime_get(uint32 unit, rtk_port_t port, uint32 *pAgingTime)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d", unit); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pAgingTime), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);
    if ((ret = reg_array_field_read(unit, ESW_PORT_REALTEK_SELF_LOOP_DETECTION_CONTROLr, 
                    port, REG_ARRAY_INDEX_NONE, ESW_SLTIMERf, pAgingTime)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    PORT_SEM_UNLOCK(unit);

    switch(*pAgingTime)
    {
        case 0:
            *pAgingTime = 1;
            break;
        case 1:
            *pAgingTime = 2;
            break;
        case 2:
            *pAgingTime = 4;
            break;
        case 3:
            *pAgingTime = 8;
            break;    
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pAgingTime=%d", *pAgingTime); 
    
    return RT_ERR_OK;
}   /*end of dal_esw_port_rldpSelfLoopAgingTime_get*/

/* Function Name:
 *      dal_esw_port_rldpSelfLoopAgingTime_set
 * Description:
 *      Set aging time of self loop on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      agingTime - aging time of self loop(unit: second)
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
 *      Following is available aging time for 8328M
 *      1, 2, 4, 8 second
 */
int32
dal_esw_port_rldpSelfLoopAgingTime_set(uint32 unit, rtk_port_t port, uint32 agingTime)
{
    int32   ret = RT_ERR_FAILED;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, agingTime=%d", unit, agingTime); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    switch(agingTime)
    {
        case 1:
            agingTime = 0;
            break;
        case 2:
            agingTime = 1;
            break;
        case 4:
            agingTime = 2;
            break;
        case 8:
            agingTime = 3;
            break;   
        default:
            RT_ERR(RT_ERR_INPUT, (MOD_DAL|MOD_PORT), "");
            return RT_ERR_INPUT;
    }

    PORT_SEM_LOCK(unit);
    if ((ret = reg_array_field_write(unit, ESW_PORT_REALTEK_SELF_LOOP_DETECTION_CONTROLr, 
                    port, REG_ARRAY_INDEX_NONE, ESW_SLTIMERf, &agingTime)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    PORT_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
}   /*end of dal_esw_port_rldpSelfLoopAgingTime_set*/

/* Function Name:
 *      dal_esw_port_rldpNormalLoopAgingTime_get
 * Description:
 *      Get aging time of normal loop on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pAgingTime - pointer to aging time of normal loop(unit: second)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Following is available aging time for 8328M
 *      1, 2, 4, 8 second
 */
int32
dal_esw_port_rldpNormalLoopAgingTime_get(uint32 unit, rtk_port_t port, uint32 *pAgingTime)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d", unit); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pAgingTime), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);
    if ((ret = reg_array_field_read(unit, ESW_PORT_REALTEK_SELF_LOOP_DETECTION_CONTROLr, 
                    port, REG_ARRAY_INDEX_NONE, ESW_NLTIMERf, pAgingTime)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    PORT_SEM_UNLOCK(unit);

    switch(*pAgingTime)
    {
        case 0:
            *pAgingTime = 1;
            break;
        case 1:
            *pAgingTime = 2;
            break;
        case 2:
            *pAgingTime = 4;
            break;
        case 3:
            *pAgingTime = 8;
            break;    
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pAgingTime=%d", *pAgingTime); 
    
    return RT_ERR_OK;
}   /*end of dal_esw_port_rldpNormalLoopAgingTime_get*/

/* Function Name:
 *      dal_esw_port_rldpNormalLoopAgingTime_set
 * Description:
 *      Set aging time of normal loop on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      agingTime - aging time of normal loop(unit: second)
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
 *      Following is available aging time for 8328M
 *      1, 2, 4, 8 second
 */
int32
dal_esw_port_rldpNormalLoopAgingTime_set(uint32 unit, rtk_port_t port, uint32 agingTime)
{
    int32   ret = RT_ERR_FAILED;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, agingTime=%d", unit, agingTime); 
    
    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    switch(agingTime)
    {
        case 1:
            agingTime = 0;
            break;
        case 2:
            agingTime = 1;
            break;
        case 4:
            agingTime = 2;
            break;
        case 8:
            agingTime = 3;
            break;   
        default:
            RT_ERR(RT_ERR_INPUT, (MOD_DAL|MOD_PORT), "");
            return RT_ERR_INPUT;
    }

    PORT_SEM_LOCK(unit);
    if ((ret = reg_array_field_write(unit, ESW_PORT_REALTEK_SELF_LOOP_DETECTION_CONTROLr, 
                    port, REG_ARRAY_INDEX_NONE, ESW_NLTIMERf, &agingTime)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    PORT_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
}   /*end of dal_esw_port_rldpSelfLoopAgingTime_set*/


/* Function Name:
 *      _dal_esw_port_init_config
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
_dal_esw_port_init_config(uint32 unit)
{
    uint32  val;
    int32   ret;
    rtk_port_t  port, max_port;
    rtk_portmask_t  portmask;
    rtk_port_phy_ability_t phy_ability;
    rtk_port_crossOver_mode_t   mode;
    rtk_port_speed_t    speed;
    rtk_port_duplex_t   duplex;

    /* Configure the bandgap from 625mV to 575mV for RTL8328M.
     * Write PHY0, Page1, Reg23[15 14] to 0x5F10
     */
    if (HAL_IS_PHY_EXIST(unit, 0))
    {
        if ((ret = hal_miim_write(unit, 0, 1, 23, 0x5F10)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "Configure bandgap for RTL8328M failed");
            return ret;
        }
    }

    for (port = HAL_GET_MIN_INT_FE_PORT(unit); port <= HAL_GET_MAX_INT_FE_PORT(unit); port++)
    {        
        if (!HAL_IS_INT_FE_PORT(unit, port)) 
        {
            continue;
        }
        if (HAL_IS_PHY_EXIST(unit, port))
        {
            /* configure link down power saving to disable for R0A */
            if (CHIP_REV_ID_A == HAL_GET_CHIP_REV_ID(unit))
            {
                if ((ret = hal_miim_write(unit, port, 0, 24, 0x0310)) != RT_ERR_OK)
                {
                    return ret;
                }
            }
        }
    }

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
            if ((ret = dal_esw_port_isolation_set(unit, port, portmask)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "Port init set port isolation portmask failed");
                return ret;
            }
        }
        
#if !defined(CONFIG_SDK_FPGA_PLATFORM)
        /* Config PHY in port that PHY exist */
        if (HAL_IS_PHY_EXIST(unit, port))
        {
            pMac_info[unit]->admin_enable[port] = RTK_ENABLE_END;
            if ((ret = dal_esw_port_adminEnable_set(unit, port, RTK_DEFAULT_PORT_ADMIN_ENABLE)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "Port init enable port failed");
                return ret;
            }
            
            if (!HAL_IS_CPU_PORT(unit, port))
            {
                if ((ret = dal_esw_port_phyAutoNegoAbility_set(unit, port, &phy_ability)) != RT_ERR_OK)            
                {
                    RT_ERR(ret, (MOD_DAL|MOD_PORT), "Port init set autonegotiation ability failed");
                    return ret;
                }
                
                if ((ret = dal_esw_port_phyAutoNegoEnable_set(unit, port, RTK_DEFAULT_PORT_AUTONEGO_ENABLE)) != RT_ERR_OK)            
                {
                    RT_ERR(ret, (MOD_DAL|MOD_PORT), "Port init enable PHY autonegotiation failed");
                    return ret;
                }
                mode = PORT_CROSSOVER_MODE_END;
                if ((ret = dal_esw_port_phyCrossOverMode_get(unit, port, &mode)) != RT_ERR_OK)
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
#endif
    }    

    val = 1;
    if ((ret = reg_field_write(unit, ESW_SMI_CONTROLr, ESW_ST_CHG_LATCH_ENf, &val)) != RT_ERR_OK)
        return ret;
    
    return RT_ERR_OK;
} /* end of _dal_esw_port_init_config */


/* Function Name:
 *      dal_esw_port_greenEnable_get
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
dal_esw_port_greenEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
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
} /* end of dal_esw_port_greenEnable_get */

/* Function Name:
 *      dal_esw_port_greenEnable_set
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
dal_esw_port_greenEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
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
} /* end of dal_esw_port_greenEnable_set */


#if defined(CONFIG_SDK_DRIVER_NIC_USER_MODE)
/* Function Name:
 *      _dal_esw_port_linkChange_isr
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
_dal_esw_port_linkChange_isr(uint32 unit, void *isr_param)
{
    rtk_portmask_t  changed_portmask;
    
    /* update first word of changed portmask */
    RTK_PORTMASK_WORD_SET(changed_portmask, 0, *((uint32 *)isr_param));
      
    /* if callback function exist, notify upper component the newest changed portmask */
    if (NULL != link_change_callback_f[unit])
    {
        link_change_callback_f[unit](unit, &changed_portmask);
    }
    
    return;
} /* end of _dal_esw_port_linkChange_isr */
#endif /* CONFIG_SDK_DRIVER_NIC_USER_MODE */

/* Function Name:
 *      dal_esw_port_linkdownPowerSaving_workaround
 * Description:
 *      Workaround for link down power saving problem of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is workaround for patch the link down power saving problem
 */
int32
dal_esw_port_linkdownPowerSaving_workaround(uint32 unit)
{
    uint32  tryCount, port;
    uint32  value, value1;
    uint32  flag = 0;
    rtk_portmask_t  portmask;

    for (port = 0; port < 16; port++)
    {
        if (!HAL_IS_PORT_EXIST(unit, port))
            continue;
        /* Read twice time due to chip have latch mechanism */
        hal_miim_read(unit, port, 0, 1, &value);
        hal_miim_read(unit, port, 0, 1, &value);

        if ((value & 0x4) == 0)
        {
            flag = 1;
            for (tryCount = 0; tryCount < 20; tryCount++)
            {
                hal_miim_read(unit, port, 0, 19, &value1);
                if (value1 & 0x8000)
                {
                    /* disable link down power saving mode */
                    hal_miim_write(unit, port, 0, 24, 0x0310);
                    break;
                }
            }
        }
    }
    
    if (flag)
    {
        osal_time_usleep(800 * 1000); /* 800ms */
        osal_memset(&portmask, 0, sizeof(rtk_portmask_t));
        portmask.bits[0] = 0xffff;
        hal_miim_portmask_write(unit, portmask, 0, 24, 0x8310);
    }

    return RT_ERR_OK;
} /* end of dal_esw_port_linkdownPowerSaving_workaround */

static uint32 macAutoMode_portmask = 0x0FFFFFFF;
static uint32 macForceMode_portmask = 0x00000000;
/* Function Name:
 *      dal_esw_port_backpressure_workaround
 * Description:
 *      Workaround for back pressure problem of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is workaround for patch the back pressure problem
 */
int32
dal_esw_port_backpressure_workaround(uint32 unit)
{
    uint32  port;
    uint32  reg_idx, val, phyVal, temp;
    uint32  local_10h, local_10f, local_100h, local_100f, local_1000f;
    uint32  remote_10h, remote_10f, remote_100h, remote_100f, remote_1000f;
    uint32  is_linkup, is_fullduplex, is_enbkprs, is_an;
    int32   ret = RT_ERR_FAILED;

    for (port = 0; port <= 27; port++)
    {
        if (!HAL_IS_PORT_EXIST(unit, port))
            continue;
        /* 1) Check the port have meet the condition or not? */
        if (macAutoMode_portmask & (1 << port))
        {
            if (port <= 23)
                reg_idx = ESW_FE_PORT_LINK_STATUSr;
            else
                reg_idx = ESW_GE_PORT_LINK_STATUSr;
            if ((ret = reg_array_read(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, &val)) != RT_ERR_OK)
                return ret;
            if ((ret = reg_field_get(unit, reg_idx, ESW_LINK_STAf, &is_linkup, &val)) != RT_ERR_OK)
                return ret;
            if ((ret = reg_field_get(unit, reg_idx, ESW_DUP_STAf, &is_fullduplex, &val)) != RT_ERR_OK)
                return ret;
            if ((ret = reg_array_read(unit, ESW_PORT_L2_MISC0r, port, REG_ARRAY_INDEX_NONE, &val)) != RT_ERR_OK)
                return ret;
            if ((ret = reg_field_get(unit, ESW_PORT_L2_MISC0r, ESW_ENBKPRSf, &is_enbkprs, &val)) != RT_ERR_OK)
                return ret;
            if ((ret = phy_autoNegoEnable_get(unit, port, &is_an)) != RT_ERR_OK)
                return ret;
            if ((is_linkup == 1) && (is_fullduplex == 0) && (is_enbkprs == 1) && (is_an == 1))
            {
                macForceMode_portmask |= (1 << port);
                macAutoMode_portmask &= ~(1 << port);
                /* Enable MAC force mode & start software polling PHY mechanism */
                //osal_printf("Port %d Enable MAC force mode & start software polling PHY mechanism\n", port);
                if ((ret = reg_field_write(unit, ESW_PHY_AUTO_ACCESS_MASKr, ESW_PHYAAM_28_0f, &macAutoMode_portmask)) != RT_ERR_OK)
                    return ret;
                if (port <= 23)
                    reg_idx = ESW_FE_PORT_PROPERTY_CONFIGUREr;
                else
                    reg_idx = ESW_GE_PORT_PROPERTY_CONFIGUREr;
                if ((ret = reg_array_read(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, &val)) != RT_ERR_OK)
                    return ret;
                temp = 1;
                if ((ret = reg_field_set(unit, reg_idx, ESW_EN_MAC_FORCEf, &temp, &val)) != RT_ERR_OK)
                    return ret;
                if ((ret = reg_field_set(unit, reg_idx, ESW_EN_FORCE_LINKf, &temp, &val)) != RT_ERR_OK)
                    return ret;
                if ((ret = reg_field_set(unit, reg_idx, ESW_EN_ASY_PAUSEf, &temp, &val)) != RT_ERR_OK)
                    return ret;
                if ((ret = reg_field_set(unit, reg_idx, ESW_EN_PAUSEf, &temp, &val)) != RT_ERR_OK)
                    return ret;
                /* Polling PHY reg4 & reg9 to ESW_MEDIA_CAPABILITYf
                 * 1000F <- PHY9.9 & PHY10.11
                 * 100F  <- PHY4.8 & PHY5.8
                 * 100H  <- PHY4.7 & PHY5.7
                 * 10F   <- PHY4.6 & PHY5.6
                 * 10H   <- PHY4.5 & PHY5.5
                 */
                temp = 0;
                if ((ret = reg_field_set(unit, reg_idx, ESW_MEDIA_CAPABILITYf, &temp, &val)) != RT_ERR_OK)
                    return ret;
                if ((ret = hal_miim_read(unit, port, 0, 4, &phyVal)) != RT_ERR_OK)
                    return ret;
                local_10h = (phyVal >> 5) & 0x1;
                local_10f = (phyVal >> 6) & 0x1;
                local_100h = (phyVal >> 7) & 0x1;
                local_100f = (phyVal >> 8) & 0x1;
                if ((ret = hal_miim_read(unit, port, 0, 5, &phyVal)) != RT_ERR_OK)
                    return ret;
                remote_10h = (phyVal >> 5) & 0x1;
                remote_10f = (phyVal >> 6) & 0x1;
                remote_100h = (phyVal >> 7) & 0x1;
                remote_100f = (phyVal >> 8) & 0x1;
                if ((ret = hal_miim_read(unit, port, 0, 9, &phyVal)) != RT_ERR_OK)
                    return ret;
                local_1000f = (phyVal >> 9) & 0x1;
                if ((ret = hal_miim_read(unit, port, 0, 10, &phyVal)) != RT_ERR_OK)
                    return ret;
                remote_1000f = (phyVal >> 11) & 0x1;
                val |= ((local_1000f & remote_1000f) << 8);
                val |= ((local_100f & remote_100f) << 7);
                val |= ((local_100h & remote_100h) << 6);
                val |= ((local_10f & remote_10f) << 5);
                val |= ((local_10h & remote_10h) << 4);
                if ((ret = reg_array_write(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, &val)) != RT_ERR_OK)
                    return ret;
            }
        }
        else
        {
            if ((ret = hal_miim_read(unit, port, 0, 1, &val)) != RT_ERR_OK)
                return ret;
            is_linkup = (val >> 2) & 0x1;
            if ((ret = reg_array_read(unit, ESW_PORT_L2_MISC0r, port, REG_ARRAY_INDEX_NONE, &val)) != RT_ERR_OK)
                return ret;
            if ((ret = reg_field_get(unit, ESW_PORT_L2_MISC0r, ESW_ENBKPRSf, &is_enbkprs, &val)) != RT_ERR_OK)
                return ret;
            if ((ret = phy_autoNegoEnable_get(unit, port, &is_an)) != RT_ERR_OK)
                return ret;
            /* Polling PHY reg4 & reg9
             * 1000F <- PHY9.9 & PHY10.11
             * 100F  <- PHY4.8 & PHY5.8
             * 100H  <- PHY4.7 & PHY5.7
             * 10F   <- PHY4.6 & PHY5.6
             * 10H   <- PHY4.5 & PHY5.5
             */
            if ((ret = hal_miim_read(unit, port, 0, 4, &val)) != RT_ERR_OK)
                return ret;
            local_10h = (val >> 5) & 0x1;
            local_10f = (val >> 6) & 0x1;
            local_100h = (val >> 7) & 0x1;
            local_100f = (val >> 8) & 0x1;
            if ((ret = hal_miim_read(unit, port, 0, 5, &val)) != RT_ERR_OK)
                return ret;
            remote_10h = (val >> 5) & 0x1;
            remote_10f = (val >> 6) & 0x1;
            remote_100h = (val >> 7) & 0x1;
            remote_100f = (val >> 8) & 0x1;
            if ((ret = hal_miim_read(unit, port, 0, 9, &val)) != RT_ERR_OK)
                return ret;
            local_1000f = (val >> 9) & 0x1;
            if ((ret = hal_miim_read(unit, port, 0, 10, &val)) != RT_ERR_OK)
                return ret;
            remote_1000f = (val >> 11) & 0x1;
            if (local_1000f & remote_1000f)
                is_fullduplex = 1;
            else if (local_100f & remote_100f)
                is_fullduplex = 1;
            else if (local_100h & remote_100h)
                is_fullduplex = 0;
            else if (local_10f & remote_10f)
                is_fullduplex = 1;
            else
                is_fullduplex = 0;
            if ((is_linkup == 0) || (is_fullduplex == 1) || (is_enbkprs == 0) || (is_an == 0))
            {
                macAutoMode_portmask |= (1 << port);
                macForceMode_portmask &= ~(1 << port);
                /* Disable MAC force mode & restart MAC auto polling PHY mechanism */
                //osal_printf("Port %d Disable MAC force mode & restart MAC auto polling PHY mechanism\n", port);
                if (port <= 23)
                    reg_idx = ESW_FE_PORT_PROPERTY_CONFIGUREr;
                else
                    reg_idx = ESW_GE_PORT_PROPERTY_CONFIGUREr;

                if ((ret = reg_array_read(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, &val)) != RT_ERR_OK)
                    return ret;
                temp = 0;
                if ((ret = reg_field_set(unit, reg_idx, ESW_EN_MAC_FORCEf, &temp, &val)) != RT_ERR_OK)
                    return ret;
                if ((ret = reg_field_set(unit, reg_idx, ESW_EN_FORCE_LINKf, &temp, &val)) != RT_ERR_OK)
                    return ret;
                temp = 1;
                if ((ret = reg_field_set(unit, reg_idx, ESW_EN_ASY_PAUSEf, &temp, &val)) != RT_ERR_OK)
                    return ret;
                if ((ret = reg_field_set(unit, reg_idx, ESW_EN_PAUSEf, &temp, &val)) != RT_ERR_OK)
                    return ret;
                if (port <= 23)
                {
                    temp = 0xf;
                    if ((ret = reg_field_set(unit, reg_idx, ESW_MEDIA_CAPABILITYf, &temp, &val)) != RT_ERR_OK)
                        return ret;
                }
                else
                {
                    temp = 0x1f;
                    if ((ret = reg_field_set(unit, reg_idx, ESW_MEDIA_CAPABILITYf, &temp, &val)) != RT_ERR_OK)
                        return ret;
                }
                if ((ret = reg_array_write(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, &val)) != RT_ERR_OK)
                    return ret;
                if ((ret = reg_field_write(unit, ESW_PHY_AUTO_ACCESS_MASKr, ESW_PHYAAM_28_0f, &macAutoMode_portmask)) != RT_ERR_OK)
                    return ret;
            }
        }
    }

    return RT_ERR_OK;
} /* end of dal_esw_port_backpressure_workaround */

/* Function Name:
 *      dal_esw_port_intraLinkDelay_workaround
 * Description:
 *      Workaround for intraLink delay problem of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is workaround for patch the intraLink delay problem
 */
int32
dal_esw_port_intraLinkDelay_workaround(uint32 unit)
{
    uint32  port, val, is_linkup, speed;
    int32   ret;
    hal_control_t *pHal_ctrl;

    /* Check whether device is exist in lower layer(MAL) */
    if ((pHal_ctrl = hal_ctrlInfo_get(unit)) == NULL)
    {
        return RT_ERR_FAILED; /* RT_ERR_CHIP_NOT_FOUND */
    }

    port = HAL_GET_MIN_GE_PORT(unit);
    if (PHY_MODEL_ID_RTL8212F == pHal_ctrl->pPhy_ctrl[port]->phy_model_id)
    {
        reg_array_read(unit, ESW_GE_PORT_LINK_STATUSr, port, REG_ARRAY_INDEX_NONE, &val);
        osal_time_usleep(10000);
        if ((ret = reg_array_read(unit, ESW_GE_PORT_LINK_STATUSr, port, REG_ARRAY_INDEX_NONE, &val)) != RT_ERR_OK)
            return ret;

        if ((ret = reg_field_get(unit, ESW_GE_PORT_LINK_STATUSr, ESW_LINK_STAf, &is_linkup, &val)) != RT_ERR_OK)
            return ret;
        if ((ret = reg_field_get(unit, ESW_GE_PORT_LINK_STATUSr, ESW_SPD_STAf, &speed, &val)) != RT_ERR_OK)
            return ret;
        if (is_linkup == 1)
        {
            if (speed == 1) /* 100M */
            {
                val = 0;
                if((ret = reg_field_write(unit, ESW_GLOBAL_MAC_INTERFACE_CONTROL2r, ESW_SEL_GMAC24_RGMII_TXC_DELAYf, &val)) != RT_ERR_OK)
                {
                    return ret;
                }
            }
            else /* 10M or 1000M */
            {
                val = 1;
                if((ret = reg_field_write(unit, ESW_GLOBAL_MAC_INTERFACE_CONTROL2r, ESW_SEL_GMAC24_RGMII_TXC_DELAYf, &val)) != RT_ERR_OK)
                {
                    return ret;
                }
            }
        }
    }

    return RT_ERR_OK;
} /* end of dal_esw_port_intraLinkDelay_workaround */

/* Function Name:
 *      dal_esw_port_phyCrossOverMode_get
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
dal_esw_port_phyCrossOverMode_get(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t *pMode)
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
}/* end of dal_esw_port_phyCrossOverMode_get */

/* Function Name:
 *      dal_esw_port_phyCrossOverMode_set
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
dal_esw_port_phyCrossOverMode_set(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t mode)
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
}/* end of dal_esw_port_phyCrossOverMode_set */

/* Function Name:
 *      dal_esw_port_flowCtrlEnable_get
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
dal_esw_port_flowCtrlEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
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

    if ((ret = dal_esw_port_phyAutoNegoEnable_get(unit, port, &nway_enable)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    if (ENABLED == nway_enable)
    {
        if ((ret = dal_esw_port_phyAutoNegoAbility_get(unit, port, &ability)) != RT_ERR_OK)
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
        if ((ret = dal_esw_port_phyForceModeAbility_get(unit, port, &speed, &duplex, &flowctrl_enable)) != RT_ERR_OK)
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }
        (*pEnable) = flowctrl_enable;
    }
    
    return RT_ERR_OK;
} /* end of dal_esw_port_flowCtrlEnable_get */

/* Function Name:
 *      dal_esw_port_flowCtrlEnable_set
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
dal_esw_port_flowCtrlEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
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

    if ((ret = dal_esw_port_phyAutoNegoAbility_get(unit, port, &ability)) != RT_ERR_OK)
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
    if ((ret = dal_esw_port_phyAutoNegoAbility_set(unit, port, &ability)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    if ((ret = dal_esw_port_phyForceModeAbility_get(unit, port, &speed, &duplex, &flowctrl_enable)) != RT_ERR_OK)
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
    if ((ret = dal_esw_port_phyForceModeAbility_set(unit, port, speed, duplex, flowctrl_enable)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }
    
    return RT_ERR_OK;
} /* end of dal_esw_port_flowCtrlEnable_set */

/* Function Name:
 *      dal_esw_port_phyComboPortFiberMedia_get
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
dal_esw_port_phyComboPortFiberMedia_get(uint32 unit, rtk_port_t port, rtk_port_fiber_media_t *pMedia)
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
} /* end of dal_esw_port_phyComboPortFiberMedia_get */

/* Function Name:
 *      dal_esw_port_phyComboPortFiberMedia_set
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
dal_esw_port_phyComboPortFiberMedia_set(uint32 unit, rtk_port_t port, rtk_port_fiber_media_t media)
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
} /* end of dal_esw_port_phyComboPortFiberMedia_set */

#if defined(CONFIG_SDK_WA_88E6063_COMPATIBLE)
/* Function Name:
 *      dal_esw_port_FeNway10MCompatible88E6063_workaround
 * Description:
 *      Workaround for link up issue with Marvell 88E6063 when connect in Nway 10M of the specified device.
 * Input:
 *      unit - unit id
 *      port - port id
 *      linkState - currently new link state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      1) The API is called by ISR and don't need to lock semaphore.
 *      2) The function is called when link change is happened and linkState is means the new state.
 */
int32 
dal_esw_port_FeNway10MCompatible88E6063_workaround(uint32 unit, rtk_port_t port, rtk_port_linkStatus_t linkState)
{
    uint32  val, tx_en;
    int32   ret;

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    if (HAL_IS_FE_PORT(unit, port))
    {
        if (PORT_LINKUP == linkState)
        {
            ret = reg_array_read(unit, ESW_FE_PORT_LINK_STATUSr, port, REG_ARRAY_INDEX_NONE, &val);
            if (((val & ESW_SPD_STA_PORT_MASK) >> ESW_SPD_STA_PORT) == 0)
            {
                /* linkup at 10M, need to delay 50ms */
                osal_time_mdelay(50);
                //osal_printf("unit=%u, port=%d, linkup at 10M (delay 50ms)\n", unit, port);

                /* Enable TX function in the port */
                if (ENABLED == pMac_info[unit]->tx_enable[port])
                {
                    tx_en = 1;
                    if ((ret = reg_array_field_write(unit, ESW_PORT_L2_MISC0r, port, REG_ARRAY_INDEX_NONE, ESW_ENTXf, &tx_en)) != RT_ERR_OK)
                    {
                        return ret;
                    }
                    //osal_printf("unit=%u, port=%d, enable TX function\n", unit, port);
                }
            }
            else
            {
                /* linkup at 100M, don't need to delay 50ms, directly enable TX function in the port */
                tx_en = 1;
                if ((ret = reg_array_field_write(unit, ESW_PORT_L2_MISC0r, port, REG_ARRAY_INDEX_NONE, ESW_ENTXf, &tx_en)) != RT_ERR_OK)
                {
                    return ret;
                }
                //osal_printf("unit=%u, port=%d, enable TX function\n", unit, port);
            } 
        }
        else
        {
            /* Disable TX function in the port */
            tx_en = 0;
            if ((ret = reg_array_field_write(unit, ESW_PORT_L2_MISC0r, port, REG_ARRAY_INDEX_NONE, ESW_ENTXf, &tx_en)) != RT_ERR_OK)
            {
                return ret;
            }
            //osal_printf("unit=%u, port=%d, disable TX function\n", unit, port);
        }
    }

    return RT_ERR_OK;
} /* end of dal_esw_port_FeNway10MCompatible88E6063_workaround */

#endif /* #if defined(CONFIG_SDK_WA_88E6063_COMPATIBLE) */

/* Function Name:
 *      dal_esw_port_linkDownPowerSavingEnable_get
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
dal_esw_port_linkDownPowerSavingEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
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
} /* end of dal_esw_port_linkDownPowerSavingEnable_get */

/* Function Name:
 *      dal_esw_port_linkDownPowerSavingEnable_set
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
dal_esw_port_linkDownPowerSavingEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
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
} /* end of dal_esw_port_linkDownPowerSavingEnable_set */
