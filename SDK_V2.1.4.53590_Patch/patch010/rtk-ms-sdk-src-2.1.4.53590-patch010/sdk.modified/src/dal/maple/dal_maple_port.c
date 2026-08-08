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
 * $Revision: 54865 $
 * $Date: 2015-01-14 10:30:45 +0800 (Wed, 14 Jan 2015) $
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
#include <hal/chipdef/maple/rtk_maple_table_struct.h>
#include <hal/mac/reg.h>
#include <hal/mac/mem.h>
#include <hal/chipdef/allreg.h>
#include <hal/chipdef/maple/rtk_maple_reg_struct.h>
#include <hal/common/miim.h>
#include <hal/common/halctrl.h>
#include <dal/dal_common.h>
#include <dal/maple/dal_maple_port.h>
#include <dal/maple/dal_maple_vlan.h>
#include <rtk/port.h>
#include <rtk/default.h>
#include <drv/intr/intr.h>
#include <drv/swcore/rtl8380.h>
#include <ioal/mem32.h>
#include <dal/dal_waMon.h>
#include <hal/phy/phy_8380.h>

/*
 * Symbol Definition
 */
typedef struct dal_maple_mac_info_s
{
    uint8   admin_enable[RTK_MAX_NUM_OF_PORTS];
    uint8   green_enable[RTK_MAX_NUM_OF_PORTS];
    uint8   gigaLite_enable[RTK_MAX_NUM_OF_PORTS];
} dal_maple_mac_info_t;

typedef struct dal_maple_phy_info_s
{
    uint8   force_mode_speed[RTK_MAX_NUM_OF_PORTS];
    uint8   force_mode_duplex[RTK_MAX_NUM_OF_PORTS];
    uint8   force_mode_flowControl[RTK_MAX_NUM_OF_PORTS];
    uint8   auto_mode_pause[RTK_MAX_NUM_OF_PORTS];
    uint8   auto_mode_asy_pause[RTK_MAX_NUM_OF_PORTS];
    uint8   cross_over_mode[RTK_MAX_NUM_OF_PORTS];
} dal_maple_phy_info_t;

/*
 * Data Declaration
 */
static uint32               port_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         port_sem[RTK_MAX_NUM_OF_UNIT];
static dal_maple_mac_info_t   *pMac_info[RTK_MAX_NUM_OF_UNIT];
static dal_maple_phy_info_t   *pPhy_info[RTK_MAX_NUM_OF_UNIT];
static dal_link_change_callback_f   link_change_callback_f[RTK_MAX_NUM_OF_UNIT];

/*Auto Recovery Debug Counter*/
extern uint32 pktBuf_watchdog_cnt;
extern uint32 macSerdes_watchdog_cnt;
extern uint32 phy_watchdog_cnt;
extern uint32 fiber_rx_watchdog_cnt;

extern int32 drv_nic_reset(uint32 unit);
extern int32 dal_waMon_phyReconfig_portMaskSet(uint32 unit, rtk_port_t port);




/*
 * Macro Definition
 */
/* port semaphore handling */
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
static int32 _dal_maple_port_init_config(uint32 unit);

#if defined(CONFIG_SDK_DRIVER_NIC_USER_MODE)
static void _dal_maple_port_linkChange_isr(uint32 unit, void *isr_param);
#endif /* CONFIG_SDK_DRIVER_NIC_USER_MODE */

/* Module Name    : port   */
/* Sub-module Name: Global */

/* Function Name:
 *      dal_maple_port_init
 * Description:
 *      Initialize port module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      1) Module must be initialized before using all of APIs in this module
 */
int32
dal_maple_port_init(uint32 unit)
{
    int32   ret;
    uint32 value;

    port_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    port_sem[unit] = osal_sem_mutex_create();
    if (0 == port_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PORT), "semaphore create failed");
        return RT_ERR_FAILED;
    }

    pMac_info[unit] = (dal_maple_mac_info_t *)osal_alloc(sizeof(dal_maple_mac_info_t));
    if (NULL == pMac_info[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PORT), "memory allocate failed");
        return RT_ERR_FAILED;
    }
    osal_memset(pMac_info[unit], 0, sizeof(dal_maple_mac_info_t));

    pPhy_info[unit] = (dal_maple_phy_info_t *)osal_alloc(sizeof(dal_maple_phy_info_t));
    if (NULL == pPhy_info[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PORT), "memory allocate failed");
        osal_free(pMac_info[unit]);
        pMac_info[unit] = NULL;
        return RT_ERR_FAILED;
    }

    osal_memset(pPhy_info[unit], 0, sizeof(dal_maple_phy_info_t));

    /* init callback function for link change */
    link_change_callback_f[unit] = 0;

    /* set init flag to complete init */
    port_init[unit] = INIT_COMPLETED;


    /******************Auto Recovery Init***********************/
    /*Enable Auto Software Queue Reset featrue*/
    value = 0x1;
    if ((ret = reg_field_write(unit, MAPLE_AUTO_SWQRST_CTRLr, MAPLE_SWQRST_SYS_THR_ENf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    value = 0x1;
    if ((ret = reg_field_write(unit, MAPLE_AUTO_SWQRST_CTRLr, MAPLE_SWQRST_P_THR_ENf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    /*Set system threshold*/
    value = 0x7c9;
    if ((ret = reg_field_write(unit, MAPLE_AUTO_SWQRST_CTRLr, MAPLE_SWQRST_SYS_THRf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }


    /******************Giga-Lite MAC Polling Access Always Enable for 838X***********************/
    if (HAL_IS_RTL8380_FAMILY_ID(unit))
    {
        /*Port0-Port23*/
        value = 0x1;
        if ((ret = reg_field_write(unit,
                              MAPLE_SMI_GLB_CTRLr,
                              MAPLE_SMI_GLITE_ACCESS_23_0f,
                              &value)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }

        /*Port24-Port27*/
        value = 0x1;
        if ((ret = reg_field_write(unit,
                              MAPLE_SMI_GLB_CTRLr,
                              MAPLE_SMI_GLITE_ACCESS_27_24f,
                              &value)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

    /******************Port Settings Init***********************/
    if (( ret = _dal_maple_port_init_config(unit)) != RT_ERR_OK)
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
        if (( ret = drv_intr_link_stat_register(unit, _dal_maple_port_linkChange_isr)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "LinkScan interrupt handler installed failed");
            return ret;
        }

    }
#endif

    return RT_ERR_OK;
} /* end of dal_maple_port_init */

/* Function Name:
 *      dal_maple_port_link_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The link status of the port is as following:
 *      - LINKDOWN
 *      - LINKUP
 */
int32
dal_maple_port_link_get(uint32 unit, rtk_port_t port, rtk_port_linkStatus_t *pStatus)
{
    int32   ret;
    uint32  val;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", unit, port);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pStatus), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);

    /* get value from CHIP*/
    if (port >= 0 && port < 28)
    {
        reg_field_read(unit, MAPLE_MAC_LINK_STSr, MAPLE_LINK_STS_27_0f, &val);
        if ((ret = reg_field_read(unit, MAPLE_MAC_LINK_STSr, MAPLE_LINK_STS_27_0f, &val)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
        val = (val >> port) & 0x1;
    }

    if (port == 28)
    {
        if ((ret = reg_array_field_read(unit, MAPLE_MAC_FORCE_MODE_CTRLr, port, \
            REG_ARRAY_INDEX_NONE, MAPLE_FORCE_LINK_ENf, &val)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

    /* translate chip's value to definition */
    if (TRUE == val)
    {
        *pStatus = PORT_LINKUP;
    }
    else
    {
        *pStatus = PORT_LINKDOWN;
    }

    PORT_SEM_UNLOCK(unit);
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pStatus=%d", *pStatus);

    return RT_ERR_OK;
} /* end of dal_maple_port_link_get */

/* Function Name:
 *      dal_maple_port_txEnable_set
 * Description:
 *      Set the TX enable status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of TX
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      The TX enable status of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
int32
dal_maple_port_txEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  val;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, enable=%d", unit, port, enable);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable != DISABLED && enable != ENABLED), RT_ERR_INPUT);

    PORT_SEM_LOCK(unit);

    /* programming value on CHIP*/
    if ((ret = reg_array_field_read(unit,
                          MAPLE_MAC_PORT_CTRLr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          MAPLE_TXRX_ENf,
                          &val)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    if (ENABLED == enable)
    {
        val |= 0x1UL;
    }
    else
    {
        val &= (~0x1UL);
    }

    if ((ret = reg_array_field_write(unit,
                          MAPLE_MAC_PORT_CTRLr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          MAPLE_TXRX_ENf,
                          &val)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_maple_port_txEnable_set */

/* Function Name:
 *      dal_maple_port_rxEnable_set
 * Description:
 *      Set the RX enable status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of RX
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      The RX enable status of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
int32
dal_maple_port_rxEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  val;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, enable=%d", unit, port, enable);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable != DISABLED && enable != ENABLED), RT_ERR_INPUT);

    PORT_SEM_LOCK(unit);

    /* programming value on CHIP*/
    if ((ret = reg_array_field_read(unit,
                          MAPLE_MAC_PORT_CTRLr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          MAPLE_TXRX_ENf,
                          &val)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    if (ENABLED == enable)
    {
        val |= 0x2;
    }
    else
    {
        val &= (~0x2UL);
    }

    if ((ret = reg_array_field_write(unit,
                          MAPLE_MAC_PORT_CTRLr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          MAPLE_TXRX_ENf,
                          &val)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_maple_port_rxEnable_set */

/* Function Name:
 *      dal_maple_port_txEnable_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The TX enable status of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
int32
dal_maple_port_txEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", unit, port);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);

    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit,
                      MAPLE_MAC_PORT_CTRLr,
                      port,
                      REG_ARRAY_INDEX_NONE,
                      MAPLE_TXRX_ENf,
                      &value)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    if (1 == (value & 0x1))
        *pEnable = ENABLED;
    else
        *pEnable = DISABLED;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pEnable=%d", *pEnable);

    return RT_ERR_OK;
} /* end of dal_maple_port_txEnable_get */

/* Function Name:
 *      dal_maple_port_rxEnable_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The RX enable status of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
int32
dal_maple_port_rxEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", unit, port);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);

    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit,
                      MAPLE_MAC_PORT_CTRLr,
                      port,
                      REG_ARRAY_INDEX_NONE,
                      MAPLE_TXRX_ENf,
                      &value)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    if (0x2 == (value & 0x2))
        *pEnable = ENABLED;
    else
        *pEnable = DISABLED;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pEnable=%d", *pEnable);

    return RT_ERR_OK;
} /* end of dal_maple_port_rxEnable_get */

/* Function Name:
 *      dal_maple_port_specialCongest_set
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
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
dal_maple_port_specialCongest_set(uint32 unit, rtk_port_t port, uint32 second)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, second=%d",
           unit, port, second);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(second > RTK_PORT_SPEC_CONGEST_TIME_MAX, RT_ERR_INPUT);

    PORT_SEM_LOCK(unit);

    if ((ret = reg_array_field_write(unit, MAPLE_SC_P_ENr, port, REG_ARRAY_INDEX_NONE, MAPLE_CNGST_SUST_TMR_LMTf, &second)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_maple_port_specialCongest_set */

/* Function Name:
 *      dal_maple_port_speedDuplex_get
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
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 *      RT_ERR_PORT_LINKDOWN - link down port status
 * Note:
 *      1) The speed type of the port is as following:
 *         - PORT_SPEED_10M
 *         - PORT_SPEED_100M
 *         - PORT_SPEED_1000M
 *      2) The duplex mode of the port is as following:
 *         - HALF_DUPLEX
 *         - FULL_DUPLEX
 */
int32
dal_maple_port_speedDuplex_get(
    uint32            unit,
    rtk_port_t        port,
    rtk_port_speed_t  *pSpeed,
    rtk_port_duplex_t *pDuplex)
{
    int32   ret;
    uint32  speed;
    uint32  duplex = 0, duplex_27_0;
    rtk_port_linkStatus_t  link_status;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", unit, port);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pSpeed), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pDuplex), RT_ERR_NULL_POINTER);

    /* Check Link status */
    if ((ret = dal_maple_port_link_get(unit, port, &link_status)) != RT_ERR_OK)
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

    if (port >= 0 && port < 28)
    {
        /* get speed value from CHIP*/
        if ((ret = reg_array_field_read(unit,
                              MAPLE_MAC_LINK_SPD_STSr,
                              port,
                              REG_ARRAY_INDEX_NONE,
                              MAPLE_SPD_STS_27_0f,
                              &speed)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }

        /* get duplex value from CHIP*/
        if ((ret = reg_field_read(unit,
                              MAPLE_MAC_LINK_DUP_STSr,
                              MAPLE_DUP_STS_27_0f,
                              &duplex_27_0)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
        duplex = ((duplex_27_0 >> port) & 0x1);

    }

    if (port == 28)
    {
        if ((ret = reg_array_field_read(unit, MAPLE_MAC_FORCE_MODE_CTRLr, port, \
            REG_ARRAY_INDEX_NONE, MAPLE_SPD_SELf, &speed)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }

        if ((ret = reg_array_field_read(unit, MAPLE_MAC_FORCE_MODE_CTRLr, port, \
            REG_ARRAY_INDEX_NONE, MAPLE_DUP_SELf, &speed)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

    PORT_SEM_UNLOCK(unit);

    /* extract port's speed value */
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
            if ((24 == port) || (26 == port))
            {
                *pSpeed = PORT_SPEED_2G;  /*only for port24 & port26*/
            }
            else
            {
                *pSpeed = PORT_SPEED_500M;
            }
            break;
        default:
            return RT_ERR_FAILED;
    }

    /* translate chip's value to definition */
    if (1 == duplex)
    {
        *pDuplex = PORT_FULL_DUPLEX;
    }
    else
    {
        *pDuplex = PORT_HALF_DUPLEX;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pSpeed=%d, pDuplex=%d",
           *pSpeed, *pDuplex);

    return RT_ERR_OK;
} /* end of dal_maple_port_speedDuplex_get */

/* Function Name:
 *      dal_maple_port_flowctrl_get
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
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 *      RT_ERR_PORT_LINKDOWN - link down port status
 * Note:
 *      None
 */
int32
dal_maple_port_flowctrl_get(
    uint32            unit,
    rtk_port_t        port,
    uint32            *pTxStatus,
    uint32            *pRxStatus)
{
    int32   ret;
    uint32 val;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d",
           unit, port);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pTxStatus), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pRxStatus), RT_ERR_NULL_POINTER);


    PORT_SEM_LOCK(unit);

    if ((ret = reg_field_read(unit, MAPLE_MAC_RX_PAUSE_STSr, MAPLE_RX_PAUSE_STS_27_0f, &val)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    *pRxStatus = (val & (1<<port)) ? 1 : 0;

    if ((ret = reg_field_read(unit, MAPLE_MAC_TX_PAUSE_STSr, MAPLE_TX_PAUSE_STS_27_0f, &val)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    *pTxStatus = (val & (1<<port)) ? 1 : 0;

    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pRxStatus=%d, pTxStatus=%d", *pRxStatus, *pTxStatus);

    return RT_ERR_OK;
} /* end of dal_maple_port_flowctrl_get */

/* Function Name:
 *      dal_maple_port_phyAutoNegoEnable_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_maple_port_phyAutoNegoEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", unit, port);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);

    /* get value from CHIP*/
    if ((ret = phy_autoNegoEnable_get(unit, port, pEnable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pEnable=%d", *pEnable);

    return RT_ERR_OK;
} /* end of dal_maple_port_phyAutoNegoEnable_get */

/* Function Name:
 *      dal_maple_port_phyAutoNegoEnablePortmask_set
 * Description:
 *      Set PHY ability of the specific port(s)
 * Input:
 *      unit        - unit id
 *      portMask    - list of ports
 *      enable      - enable PHY auto negotiation
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - input parameter out of range
 * Note:
 *      1) ENABLED : switch to PHY auto negotiation mode
 *         DISABLED: switch to PHY force mode
 *      2) Once the abilities of both auto-nego and force mode are set,
 *         you can freely swtich the mode without calling ability setting API again
 */
int32
dal_maple_port_phyAutoNegoEnablePortmask_set(uint32 unit, rtk_portmask_t portMask, rtk_enable_t enable)
{
    int32   ret,port;
    rtk_port_phy_ability_t ability;
    uint32  reg_idx,temp;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, enable=%d",
           unit, enable);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    for(port=0;port<RTK_MAX_NUM_OF_PORTS;port++)
    {
        if(!RTK_PORTMASK_IS_PORT_SET(portMask,port))
            continue;
        RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    }
    RT_PARAM_CHK(enable >= RTK_ENABLE_END, RT_ERR_INPUT);

    PORT_SEM_LOCK(unit);

    for(port=0;port<RTK_MAX_NUM_OF_PORTS;port++)
    {
        if(!RTK_PORTMASK_IS_PORT_SET(portMask,port))
            continue;
        if ((ret = phy_autoNegoEnable_set(unit, port, enable)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

    /*Delay 100ms*/
    osal_time_usleep(100000);

    if (ENABLED == enable)
    {
        for(port=0;port<RTK_MAX_NUM_OF_PORTS;port++)
        {
            if(!RTK_PORTMASK_IS_PORT_SET(portMask,port))
                continue;

            /* Need to configure [MAC_FORCE_MODE_CTRL] register  when PHY is force mode */
            reg_idx = MAPLE_MAC_FORCE_MODE_CTRLr;

            temp = 0x0;     /*Reset MAC FORCE Flow Control to Disable [MAC FLOW CONTROL FORCE ENABLE]*/
            ret = reg_array_field_write(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, MAPLE_MAC_FORCE_FC_ENf, &temp);
            if ((ret != RT_ERR_OK)&&(ret != RT_ERR_CHIP_NOT_SUPPORTED))
            {
                PORT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }

            /* get value from CHIP*/
            if ((ret = phy_autoNegoAbility_get(unit, port, &ability)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }


            ability.FC = pPhy_info[unit]->auto_mode_pause[port];
            ability.AsyFC = pPhy_info[unit]->auto_mode_asy_pause[port];


            /* get value from CHIP*/
            if ((ret = phy_autoNegoAbility_set(unit, port, &ability)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }
        }
    }

    if (DISABLED == enable)
    {
       for(port=0;port<RTK_MAX_NUM_OF_PORTS;port++)
        {
            if(!RTK_PORTMASK_IS_PORT_SET(portMask,port))
                continue;

            /* get value from CHIP*/
            if ((ret = phy_duplex_set(unit, port, pPhy_info[unit]->force_mode_duplex[port])) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }

            /* get value from CHIP*/
            if ((ret = phy_speed_set(unit, port, pPhy_info[unit]->force_mode_speed[port])) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }

            if ((ret = phy_autoNegoAbility_get(unit, port, &ability)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }

            ability.FC = pPhy_info[unit]->force_mode_flowControl[port]; /* ENABLED */
            ability.AsyFC = pPhy_info[unit]->force_mode_flowControl[port]; /* ENABLED */


            /* Need to configure [MAC_FORCE_MODE_CTRL] register  when PHY is force mode */
            reg_idx = MAPLE_MAC_FORCE_MODE_CTRLr;

            temp = 0x1;     /*Set MAC FORCE Flow Control*/
            ret = reg_array_field_write(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, MAPLE_MAC_FORCE_FC_ENf, &temp);
            if ((ret != RT_ERR_OK)&&(ret != RT_ERR_CHIP_NOT_SUPPORTED))
            {
                PORT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }

            temp = ability.AsyFC;
            ret = reg_array_field_write(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, MAPLE_RX_PAUSE_ENf, &temp);
            if ((ret != RT_ERR_OK)&&(ret != RT_ERR_CHIP_NOT_SUPPORTED))
            {
                PORT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }

            temp = ability.FC;
            ret = reg_array_field_write(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, MAPLE_TX_PAUSE_ENf, &temp);
            if ((ret != RT_ERR_OK)&&(ret != RT_ERR_CHIP_NOT_SUPPORTED))
            {
                PORT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }

            /* get value from CHIP*/
            if ((ret = phy_autoNegoAbility_set(unit, port, &ability)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }

            /* E0005371 */
            if (ENABLED == pMac_info[unit]->admin_enable[port])
            {
                /* Turn off and then turn on the power of port so the partner could detect the speed/duplex change */
                if ((ret = phy_enable_set(unit, port, DISABLED)) != RT_ERR_OK)
                {
                    PORT_SEM_UNLOCK(unit);
                    RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                    return ret;
                }

                osal_time_usleep(200000);

                if ((ret = phy_enable_set(unit, port, ENABLED)) != RT_ERR_OK)
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
} /* end of dal_maple_port_phyAutoNegoEnablePortmask_set */


/* Function Name:
 *      dal_maple_port_phyAutoNegoEnable_set
 * Description:
 *      Set PHY ability of the specific port(s)
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable PHY auto negotiation
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - input parameter out of range
 * Note:
 *      1) ENABLED : switch to PHY auto negotiation mode
 *         DISABLED: switch to PHY force mode
 *      2) Once the abilities of both auto-nego and force mode are set,
 *         you can freely swtich the mode without calling ability setting API again
 */
int32
dal_maple_port_phyAutoNegoEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtk_portmask_t myPortMask;

    RTK_PORTMASK_RESET(myPortMask);
    RTK_PORTMASK_PORT_SET(myPortMask,port);

    return dal_maple_port_phyAutoNegoEnablePortmask_set(unit, myPortMask, enable);

} /* end of dal_maple_port_phyAutoNegoEnable_set */

/* Function Name:
 *      dal_maple_port_phyAutoNegoAbility_get
 * Description:
 *      Get PHY auto negotiation ability of the specific port
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pAbility - pointer to the PHY ability
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
dal_maple_port_phyAutoNegoAbility_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_port_phy_ability_t  *pAbility)
{
    int32   ret;
    rtk_enable_t  enable;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d",
           unit, port);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pAbility), RT_ERR_NULL_POINTER);

    if ((ret = dal_maple_port_phyAutoNegoEnable_get(unit, port, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL | MOD_PORT), "");
        return ret;
    }

    osal_memset(pAbility, 0, sizeof(rtk_port_phy_ability_t));

    PORT_SEM_LOCK(unit);

    if ((ret = phy_autoNegoAbility_get(unit, port, pAbility)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
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
} /* end of dal_maple_port_phyAutoNegoAbility_get */

/* Function Name:
 *      dal_maple_port_phyAutoNegoAbility_set
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1) You can set these abilities no matter which mode PHY currently stays on
 */
int32
dal_maple_port_phyAutoNegoAbility_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_port_phy_ability_t  *pAbility)
{
    int32   ret;
    rtk_enable_t    enable;
    uint32  reg_idx,temp;


    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", unit, port);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pAbility), RT_ERR_NULL_POINTER);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Half_10=%d, Full_10=%d, Half_100=%d, Full_100=%d, \
           Half_1000=%d, Full_1000=%d, FC=%d, AsyFC=%d",
           pAbility->Half_10, pAbility->Full_10, pAbility->Half_100, pAbility->Full_100,
           pAbility->Half_1000, pAbility->Full_1000, pAbility->FC, pAbility->AsyFC);

    if ((ret = dal_maple_port_phyAutoNegoEnable_get(unit, port, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL | MOD_PORT), "dal_maple_port_phyAutoNegoEnable_get(unit=%d, port=%d) failed!!",\
               unit, port);
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

    /* Need to configure [MAC_FORCE_MODE_CTRL] register  when PHY is force mode */
    reg_idx = MAPLE_MAC_FORCE_MODE_CTRLr;

    temp = 0x0;     /*Reset MAC FORCE Flow Control to Disable [MAC FLOW CONTROL FORCE ENABLE]*/
    ret = reg_array_field_write(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, MAPLE_MAC_FORCE_FC_ENf, &temp);
    if ((ret != RT_ERR_OK)&&(ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    if ((ret = phy_autoNegoAbility_set(unit, port, pAbility)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    if (ENABLED == enable)
    {
        pPhy_info[unit]->auto_mode_pause[port] = pAbility->FC;
        pPhy_info[unit]->auto_mode_asy_pause[port] = pAbility->AsyFC;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_maple_port_phyAutoNegoAbility_set */

/* Function Name:
 *      dal_maple_port_phyForceModeAbility_get
 * Description:
 *      Get PHY ability status of the specific port
 * Input:
 *      unit         - unit id
 *      port         - port id
 * Output:
 *      pSpeed       - pointer to the port speed
 *      pDuplex      - pointer to the port duplex
 *      pFlowControl - pointer to the flow control enable status
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
dal_maple_port_phyForceModeAbility_get(
    uint32              unit,
    rtk_port_t          port,
    rtk_port_speed_t    *pSpeed,
    rtk_port_duplex_t   *pDuplex,
    rtk_enable_t        *pFlowControl)
{
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

    if ((ret = dal_maple_port_phyAutoNegoEnable_get(unit, port, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL | MOD_PORT), "");
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
        /* get value from CHIP*/
        if ((ret = phy_speed_get(unit, port, pSpeed)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }

        /* get value from CHIP*/
        if ((ret = phy_duplex_get(unit, port, pDuplex)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }

        if ((ret = phy_autoNegoAbility_get(unit, port, &ability)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }

        *pFlowControl = ability.FC;
    }

    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pSpeed=%d, pDuplex=%d \
           pFlowControl=%d", *pSpeed, *pDuplex, *pFlowControl);

    return RT_ERR_OK;
} /* end of dal_maple_port_phyForceModeAbility_get */

/* Function Name:
 *      dal_maple_port_phyForceModeAbility_set
 * Description:
 *      Set the port speed/duplex mode/pause/asy_pause in the PHY force mode
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      speed       - port speed
 *      duplex      - port duplex mode
 *      flowControl - enable flow control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_PORT_ID    - invalid port id
 *      RT_ERR_PHY_SPEED  - invalid PHY speed setting
 *      RT_ERR_PHY_DUPLEX - invalid PHY duplex setting
 *      RT_ERR_INPUT      - invalid input parameter
 * Note:
 *      1) You can set these abilities no matter which mode PHY currently stays on
 *      2) The speed type of the port is as following:
 *         - PORT_SPEED_10M
 *         - PORT_SPEED_100M
 *      3) The duplex mode of the port is as following:
 *         - HALF_DUPLEX
 *         - FULL_DUPLEX
 */
int32
dal_maple_port_phyForceModeAbility_set(
    uint32              unit,
    rtk_port_t          port,
    rtk_port_speed_t    speed,
    rtk_port_duplex_t   duplex,
    rtk_enable_t        flowControl)
{
    int32   ret;
    rtk_enable_t    enable;
    rtk_port_phy_ability_t ability;
    uint32  reg_idx,temp;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, speed=%d, duplex=%d \
           flowControl=%d", unit, port, speed, duplex, flowControl);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(speed >= PORT_SPEED_END, RT_ERR_PHY_SPEED);
    RT_PARAM_CHK((!HAL_IS_GE_COMBO_PORT(unit, port)) && (!HAL_IS_SERDES_PORT(unit, port)) &&\
		(!HAL_IS_GE_PORT(unit, port)) && speed == PORT_SPEED_1000M, RT_ERR_CHIP_NOT_SUPPORTED);
    RT_PARAM_CHK(duplex >= PORT_DUPLEX_END, RT_ERR_PHY_DUPLEX);
    RT_PARAM_CHK(flowControl >= RTK_ENABLE_END, RT_ERR_INPUT);

    if ((ret = dal_maple_port_phyAutoNegoEnable_get(unit, port, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL | MOD_PORT), "");
        return ret;
    }

    PORT_SEM_LOCK(unit);

    pPhy_info[unit]->force_mode_speed[port] = speed;
    pPhy_info[unit]->force_mode_duplex[port] = duplex;
    pPhy_info[unit]->force_mode_flowControl[port] = flowControl;

    if (DISABLED == enable)
    {
        /* get value from CHIP*/
        if ((ret = phy_speed_set(unit, port, speed)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }

        if ((ret = phy_duplex_set(unit, port, duplex)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }


        if ((ret = phy_autoNegoAbility_get(unit, port, &ability)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
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


        /* Need to configure [MAC_FORCE_MODE_CTRL] register  when PHY is force mode */
        reg_idx = MAPLE_MAC_FORCE_MODE_CTRLr;

        temp = 0x1;     /*Always set MAC FORCE Flow Control*/
        ret = reg_array_field_write(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, MAPLE_MAC_FORCE_FC_ENf, &temp);
        if ((ret != RT_ERR_OK)&&(ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }

        temp = ability.AsyFC;
        ret = reg_array_field_write(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, MAPLE_RX_PAUSE_ENf, &temp);
        if ((ret != RT_ERR_OK)&&(ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }

        temp = ability.FC;
        ret = reg_array_field_write(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, MAPLE_TX_PAUSE_ENf, &temp);
        if ((ret != RT_ERR_OK)&&(ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }


        if ((ret = phy_autoNegoAbility_set(unit, port, &ability)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }

        /* E0005371 */
        if (ENABLED == pMac_info[unit]->admin_enable[port])
        {
            /* Turn off and then turn on the power of port so the partner could detect the speed/duplex change */
            if ((ret = phy_enable_set(unit, port, DISABLED)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }

            osal_time_usleep(200000);

            if ((ret = phy_enable_set(unit, port, ENABLED)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }
        }
        /* End of E0005371 */
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_maple_port_phyForceModeAbility_set */

/* Function Name:
 *      dal_maple_port_phyMasterSlave_get
 * Description:
 *      Get PHY configuration of master/slave mode of the specific port
 * Input:
 *      unit                - unit id
 *      port                - port id
 * Output:
 *      pMasterSlaveCfg     - pointer to the PHY master slave configuration
 *      pMasterSlaveActual  - pointer to the PHY master slave actual link status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      This function only works on giga port to get its master/slave mode configuration.
 */
int32
dal_maple_port_phyMasterSlave_get(
    uint32              unit,
    rtk_port_t          port,
    rtk_port_masterSlave_t   *pMasterSlaveCfg,
    rtk_port_masterSlave_t   *pMasterSlaveActual)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", unit, port);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pMasterSlaveCfg), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pMasterSlaveActual), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);

	ret = phy_masterSlave_get(unit, port, pMasterSlaveCfg, pMasterSlaveActual);

    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "*pMasterSlaveCfg=%d, *pMasterSlaveActual=%d", *pMasterSlaveCfg, *pMasterSlaveActual);

    return ret;
}/* end of dal_maple_port_phyMasterSlave_get */

/* Function Name:
 *      dal_maple_port_phyMasterSlave_set
 * Description:
 *      Set PHY configuration of master/slave mode of the specific port
 * Input:
 *      unit                - unit id
 *      port                - port id
 *      masterSlave         - PHY master slave configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_INPUT         - invalid input parameter
 * Note:
 *      None
 */
int32
dal_maple_port_phyMasterSlave_set(
    uint32              unit,
    rtk_port_t          port,
    rtk_port_masterSlave_t   masterSlave)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, masterSlave=%d", unit, port, masterSlave);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(masterSlave >= PORT_MASTER_SLAVE_END, RT_ERR_INPUT);

    PORT_SEM_LOCK(unit);

	ret = phy_masterSlave_set(unit, port, masterSlave);

    PORT_SEM_UNLOCK(unit);

    return ret;
}/* end of dal_maple_port_phyMasterSlave_set */


/* Function Name:
 *      dal_maple_port_phyReg_get
 * Description:
 *      Get PHY register data of the specific port
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      page  - page id
 *      reg   - reg id
 * Output:
 *      pData - pointer to the PHY reg data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid page id
 *      RT_ERR_PHY_REG_ID   - invalid reg id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_maple_port_phyReg_get(
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
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);

    if ((ret = hal_miim_read(unit, port, page, reg, pData)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pData=0x%x", *pData);

    return ret;
	
} /* end of dal_maple_port_phyReg_get */

/* Function Name:
 *      dal_maple_port_phyReg_set
 * Description:
 *      Set PHY register data of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 *      page - page id
 *      reg  - reg id
 *      data - reg data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_PORT_ID     - invalid port id
 *      RT_ERR_PHY_PAGE_ID - invalid page id
 *      RT_ERR_PHY_REG_ID  - invalid reg id
 * Note:
 *      None
 */
int32
dal_maple_port_phyReg_set(
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
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);

    PORT_SEM_LOCK(unit);

    if ((ret = hal_miim_write(unit, port, page, reg, data)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return ret;
} /* end of dal_maple_port_phyReg_set */


/* Function Name:
 *      dal_maple_port_phyMmdReg_get
 * Description:
 *      Get PHY MMD register data of the specific port
 * Input:
 *      unit                - unit id
 *      port                - port id
 *      mmdAddr             - mmd device address
 *      mmdReg              - mmd reg id
 * Output:
 *      pData              - pointer to the PHY reg data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_maple_port_phyMmdReg_get(
    uint32              unit,
    rtk_port_t          port,
    uint32              mmdAddr,
    uint32              mmdReg,
    uint32              *pData)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, mmdAddr=0x%x, mmdReg=0x%x, reg=0x%x",
           unit, port, mmdAddr, mmdReg);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);

    if ((ret = hal_miim_mmd_read(unit, port, mmdAddr, mmdReg, pData)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    PORT_SEM_UNLOCK(unit);
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pData=0x%x", *pData);

    return ret;
}    /* end of dal_maple_port_phyMmdReg_get */

/* Function Name:
 *      dal_maple_port_phyMmdReg_set
 * Description:
 *      Set PHY MMD register data of the specific port
 * Input:
 *      unit               - unit id
 *      port               - port id
 *      mmdAddr            - mmd device address
 *      mmdReg             - mmd reg id
 *      data               - reg data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      None
 */
int32
dal_maple_port_phyMmdReg_set(
    uint32              unit,
    rtk_port_t          port,
    uint32              mmdAddr,
    uint32              mmdReg,
    uint32              data)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, mmdAddr=0x%x, mmdReg=0x%x \
           data=0x%x", unit, port, mmdAddr, mmdReg, data);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);

    PORT_SEM_LOCK(unit);

    if ((ret = hal_miim_mmd_write(unit, port, mmdAddr, mmdReg, data)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return ret;
}    /* end of dal_maple_port_phyMmdReg_set */

/* Function Name:
 *      dal_maple_port_phymaskMmdReg_set
 * Description:
 *      Set PHY MMD register data of the specific portmask
 * Input:
 *      unit               - unit id
 *      pPortmask          - pointer to the portmask
 *      mmdAddr            - mmd device address
 *      mmdReg             - mmd reg id
 *      data               - reg data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_maple_port_phymaskMmdReg_set(
    uint32              unit,
    rtk_portmask_t      *pPortmask,
    uint32              mmdAddr,
    uint32              mmdReg,
    uint32              data)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, portmask=0x%8x 0x%8x, mmdAddr=0x%x, mmdReg=0x%x \
           data=0x%x", unit, pPortmask->bits[1], pPortmask->bits[0], mmdAddr, mmdReg, data);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pPortmask), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);

    if ((ret = hal_miim_mmd_portmask_write(unit, *pPortmask, mmdAddr, mmdReg, data)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return ret;
}    /* end of dal_maple_port_phymaskMmdReg_set */


/* Function Name:
 *      dal_maple_port_cpuPortId_get
 * Description:
 *      Get CPU port id of the specific unit
 * Input:
 *      unit  - unit id
 * Output:
 *      pPort - pointer to CPU port id
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_maple_port_cpuPortId_get(uint32 unit, rtk_port_t *pPort)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pPort), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);

    if ((ret = reg_field_read(unit, MAPLE_MAC_CPU_PORT_CTRLr, MAPLE_CPU_PORTf, &value)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    switch (value)
    {
        case 0:
            *pPort = 24;
            break;
        case 1:
            *pPort = 26;
            break;
        case 2:
            *pPort = 28;
            break;
        default:
            return RT_ERR_FAILED;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pPort=%d", *pPort);

    return RT_ERR_OK;
} /* end of dal_maple_port_cpuPortId_get */

/* Function Name:
 *      dal_maple_port_cpuPortId_set
 * Description:
 *      Set CPU port id of the specific unit
 * Input:
 *      unit - unit id
 *      port - CPU port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 * Note:
 *      None
 */
int32
dal_maple_port_cpuPortId_set(uint32 unit, rtk_port_t port)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(((24 != port) && (26 != port) && (28 != port)), RT_ERR_PORT_ID);

    switch (port)
    {
        case 24:
            value = 0;
            break;
        case 26:
            value = 1;
            break;
        case 28:
            value = 2;
            break;
        default:
            return RT_ERR_FAILED;
    }

    PORT_SEM_LOCK(unit);

    if ((ret = reg_field_write(unit, MAPLE_MAC_CPU_PORT_CTRLr, MAPLE_CPU_PORTf, &value)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "port=%d", port);

    return RT_ERR_OK;
} /* end of dal_maple_port_cpuPortId_set */

/* Function Name:
 *      dal_maple_port_isolation_get
 * Description:
 *      Get the portmask of the port isolation
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pPortmask - pointer to the portmask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1) Default value of each port is 1
 *      2) Enable port isolation in the certain ports if relative portmask bits are set to 1
 */
int32
dal_maple_port_isolation_get(uint32 unit, rtk_port_t port, rtk_portmask_t *pPortmask)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", unit, port);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pPortmask), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);

    /* get speed value from CHIP*/
    if ((ret = reg_array_field_read(unit, MAPLE_PORT_ISO_CTRLr, REG_ARRAY_INDEX_NONE, port, MAPLE_P_ISO_MBR_0f
                        , &pPortmask->bits[0])) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pPortmask=0x%x", pPortmask->bits[0]);

    return RT_ERR_OK;
} /* end of dal_maple_port_isolation_get */

/* Function Name:
 *      dal_maple_port_isolation_set
 * Description:
 *      Set the portmask of the port isolation
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      portmask - pointer to the portmask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT  - The module is not initial
 *      RT_ERR_PORT_ID   - invalid port id
 *      RT_ERR_PORT_MASK - invalid port mask
 * Note:
 *      1) Default value of each port is 1
 *      2) Enable port isolation in the certain ports if relative portmask bits are set to 1
 */
int32
dal_maple_port_isolation_set(uint32 unit, rtk_port_t port, rtk_portmask_t portmask)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, portmask=0x%x",
           unit, port, portmask.bits[0]);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);

    PORT_SEM_LOCK(unit);

    /* write isolation mask to CHIP*/
    if ((ret = reg_array_field_write(unit, MAPLE_PORT_ISO_CTRLr, REG_ARRAY_INDEX_NONE, port, MAPLE_P_ISO_MBR_0f
                        , &portmask.bits[0])) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_maple_port_isolation_set */

/* Function Name:
 *      dal_maple_port_isolation_add
 * Description:
 *      Add an isolation port to the certain port
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      iso_port - isolation port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 * Note:
 *      1) Default value of each port is 1
 *      2) Port and iso_port will be isolated when this API is called
 *      3) The iso_port to the relative portmask bit will be set to 1
 */
int32
dal_maple_port_isolation_add(uint32 unit, rtk_port_t port, rtk_port_t iso_port)
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

    ret = dal_maple_port_isolation_get(unit, port, &portmask);
    if (RT_ERR_OK != ret)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    RTK_PORTMASK_PORT_SET(portmask, iso_port);

    ret = dal_maple_port_isolation_set(unit, port, portmask);
    if (RT_ERR_OK != ret)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_maple_port_isolation_add */

/* Function Name:
 *      dal_maple_port_isolation_del
 * Description:
 *      Delete an existing isolation port of the certain port
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      iso_port - isolation port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 * Note:
 *      1) Default value of each port is 1
 *      2) Isolated status between the port and the iso_port is removed when this API is called
 *      3) The iso_port to the relative portmask bit will be set to 0
 */
int32
dal_maple_port_isolation_del(uint32 unit, rtk_port_t port, rtk_port_t iso_port)
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

    ret = dal_maple_port_isolation_get(unit, port, &portmask);
    if (RT_ERR_OK != ret)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    RTK_PORTMASK_PORT_CLEAR(portmask, iso_port);

    ret = dal_maple_port_isolation_set(unit, port, portmask);
    if (RT_ERR_OK != ret)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_maple_port_isolation_del */

/* Function Name:
 *      dal_maple_port_phyComboPortMedia_get
 * Description:
 *      Get PHY port media of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pMedia - pointer to the port media
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
dal_maple_port_phyComboPortMedia_get(uint32 unit, rtk_port_t port, rtk_port_media_t *pMedia)
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
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_maple_port_phyComboPortMedia_get */

/* Function Name:
 *      dal_maple_port_phyComboPortMedia_set
 * Description:
 *      Set PHY port media of the specific port
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      media - port media
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      1) You can set these port media which mode PHY currently stays on
 */
int32
dal_maple_port_phyComboPortMedia_set(uint32 unit, rtk_port_t port, rtk_port_media_t media)
{
    int32 ret;
    uint32 reg_val, polling_msk;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, media=%d",
           unit, port, media);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((media >= PORT_MEDIA_END), RT_ERR_INPUT);

    PORT_SEM_LOCK(unit);

    /*Save polling mask*/
    if ((ret = reg_read(unit, MAPLE_SMI_POLL_CTRLr, &polling_msk)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "Save Port:%d  MAC polling fail (0x%x)", port, ret);
        return ret;
    }

    /*Disable MAC polling PHY*/
    reg_val = 0x0;
    if ((ret = reg_write(unit, MAPLE_SMI_POLL_CTRLr, &reg_val)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "Port %d disable MAC polling fail (0x%x)", port, ret);
	 return ret;
    }

    /* set value into CHIP*/
    if ((ret = phy_media_set(unit, port, media)) != RT_ERR_OK)
    {
        /*Restore MAC polling PHY*/
	 reg_write(unit, MAPLE_SMI_POLL_CTRLr, &polling_msk);

        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }


    /*Restore MAC polling PHY*/
    if ((ret = reg_write(unit, MAPLE_SMI_POLL_CTRLr, &polling_msk)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "port %d Restore MAC polling fail (0x%x)", port, ret);
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_maple_port_phyComboPortMedia_set */

/* Function Name:
 *      dal_maple_port_adminEnable_get
 * Description:
 *      Get port admin status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to the port admin status
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
dal_maple_port_adminEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
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
} /* end of dal_maple_port_adminEnable_get */

/* Function Name:
 *      dal_maple_port_adminEnable_set
 * Description:
 *      Set port admin configuration of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - port admin configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
dal_maple_port_adminEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, port admin=%d",
           unit, port, enable);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable != DISABLED && enable != ENABLED), RT_ERR_INPUT);
    PORT_SEM_LOCK(unit);

    if (ENABLED == enable)
    {
        value = 0x3;
        /* programming value on CHIP*/
        if ((ret = reg_array_field_write(unit,
                          MAPLE_MAC_PORT_CTRLr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          MAPLE_TXRX_ENf,
                          &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

#if !defined (__MODEL_USER__)
    if (HAL_IS_PHY_EXIST(unit, port))
    {
        if ((ret = phy_enable_set(unit, port, enable)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }
#endif

    if (DISABLED == enable)
    {
        /* programming value on CHIP*/
        value = 0x0;
        if ((ret = reg_array_field_write(unit,
                          MAPLE_MAC_PORT_CTRLr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          MAPLE_TXRX_ENf,
                          &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

    pMac_info[unit]->admin_enable[port] = enable;
    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_maple_port_adminEnable_set */

/* Function Name:
 *      dal_maple_port_backpressureEnable_get
 * Description:
 *      Get the half duplex backpressure enable status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to the enable status of backpressure in half duplex mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1) The mac local loopback enable status of the port is as following:
 *         - DISABLE
 *         - ENABLE
 *      2) Used to support backpressure in half mode.
 */
int32
dal_maple_port_backpressureEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
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
    if ((ret = reg_array_field_read(unit,
                          MAPLE_MAC_PORT_CTRLr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                         MAPLE_BKPRES_ENf,
                          pEnable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pEnable=%d", *pEnable);

    return RT_ERR_OK;
} /* end of dal_maple_port_backpressureEnable_get */

/* Function Name:
 *      dal_maple_port_backpressureEnable_set
 * Description:
 *      Set the half duplex backpressure enable status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of backpressure in half duplex mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      1) The mac local loopback enable status of the port is as following:
 *         - DISABLE
 *         - ENABLE
 *      2) Used to support backpressure in half mode.
 */
int32
dal_maple_port_backpressureEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
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

    /* set value to CHIP*/
    if ((ret = reg_array_field_write(unit,
                          MAPLE_MAC_PORT_CTRLr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          MAPLE_BKPRES_ENf,
                          &enable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_maple_port_backpressureEnable_set */

/* Function Name:
 *      dal_maple_port_linkChange_register
 * Description:
 *      Register callback function for notification of link change
 * Input:
 *      unit                 - unit id
 *      link_change_callback - Callback function for link change
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_maple_port_linkChange_register(uint32 unit, dal_link_change_callback_f link_change_callback)
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
} /* End of dal_maple_port_linkChange_register */

/* Function Name:
 *      dal_maple_port_linkChange_unregister
 * Description:
 *      Unregister callback function for notification of link change
 * Input:
 *      unit - unit id
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 * Note:
 */
int32
dal_maple_port_linkChange_unregister(uint32 unit)
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
} /* End of dal_maple_port_linkChange_unregister */

/* Function Name:
 *      _dal_maple_port_init_config
 * Description:
 *      Initialize default configuration for port module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      1) Module must be initialized before using all of APIs in this module
 */

static int32
_dal_maple_port_init_config(uint32 unit)
{
    int32   ret;
    rtk_port_t  port, max_port;
    rtk_portmask_t  portmask,portmask2;
    rtk_port_phy_ability_t phy_ability;
    //rtk_port_crossOver_mode_t   mode;
    rtk_port_speed_t    speed;
    rtk_port_duplex_t   duplex;
    rtk_enable_t   gigaLite;

    phy_ability.Half_10 = RTK_DEFAULT_PORT_10HALF_CAPABLE;
    phy_ability.Full_10 = RTK_DEFAULT_PORT_10FULL_CAPABLE;
    phy_ability.Half_100 = RTK_DEFAULT_PORT_100HALF_CAPABLE;
    phy_ability.Full_100 = RTK_DEFAULT_PORT_100FULL_CAPABLE;
    phy_ability.FC = RTK_DEFAULT_PORT_PAUSE_CAPABILITY;
    phy_ability.AsyFC = RTK_DEFAULT_PORT_ASYPAUSE_CAPABILITY;

    max_port = HAL_GET_MAX_PORT(unit);
    HAL_GET_ALL_PORTMASK(unit, portmask);
    RTK_PORTMASK_RESET(portmask2);

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

#if !defined(CONFIG_SDK_FPGA_PLATFORM)
        /* Config PHY in port that PHY exist */
        if (HAL_IS_PHY_EXIST(unit, port))
        {
            pMac_info[unit]->admin_enable[port] = RTK_ENABLE_END;
            if ((ret = dal_maple_port_adminEnable_set(unit, port, RTK_DEFAULT_PORT_ADMIN_ENABLE)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "Port init enable port failed");
                return ret;
            }

             if (!HAL_IS_CPU_PORT(unit, port) && !HAL_IS_SERDES_PORT(unit, port))
            {
                if ((ret = dal_maple_port_phyAutoNegoAbility_set(unit, port, &phy_ability)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_PORT), "Port init set autonegotiation ability failed");
                    return ret;
                }

                RTK_PORTMASK_PORT_SET(portmask2,port);

#if 0 /* The dal_maple_port_phyCrossOverMode_get API is not ready, Fixed Me!!! */

                mode = PORT_CROSSOVER_MODE_END;
                if ((ret = dal_maple_port_phyCrossOverMode_get(unit, port, &mode)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_PORT), "Port init get PHY cross over mode failed");
                    return ret;
                }
                pPhy_info[unit]->cross_over_mode[port] = mode;
#endif


                speed = PORT_SPEED_END;
                /* get value from CHIP*/
                if ((ret = phy_speed_get(unit, port, &speed)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                    return ret;
                }

                pPhy_info[unit]->force_mode_speed[port] = PORT_SPEED_100M;
                duplex = PORT_DUPLEX_END;
                /* get value from CHIP*/
                if ((ret = phy_duplex_get(unit, port, &duplex)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                    return ret;
                }

                pPhy_info[unit]->force_mode_duplex[port] = duplex;

                if ((ret = phy_gigaLiteEnable_get(unit, port, &gigaLite)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                    return ret;
                }

                pMac_info[unit]->gigaLite_enable[port] = gigaLite;
            }
        }
#endif
    }

    if ((ret = dal_maple_port_phyAutoNegoEnablePortmask_set(unit, portmask2, RTK_DEFAULT_PORT_AUTONEGO_ENABLE)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "Port init enable PHY autonegotiation failed");
        return ret;
    }


    return RT_ERR_OK;
} /* end of _dal_maple_port_init_config */


/* Function Name:
 *      dal_maple_port_linkDownGreenEnable_get
 * Description:
 *      Get the statue of linkdown green feature of the specific port in the specific unit
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to status of linkdown green feature
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
dal_maple_port_linkDownGreenEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    *pEnable = pMac_info[unit]->green_enable[port];

    return RT_ERR_OK;
} /* end of dal_maple_port_linkDownGreenEnable_get */

/* Function Name:
 *      dal_maple_port_linkDownGreenEnable_set
 * Description:
 *      Set the statue of linkdown green feature of the specific port in the specific unit
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of linkdown green feature
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
dal_maple_port_linkDownGreenEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
#if 0
    uint32  port0 = 0;
    uint32  val = 0;
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, enable=%d",
           unit, port, enable);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable != DISABLED && enable != ENABLED), RT_ERR_INPUT);

    PORT_SEM_LOCK(unit);

    /* per-serdes */
    if ((ret = reg_read(unit, (uint32)portSerdesControl_regidx[port], &val)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    pMac_info[unit]->green_enable[port] = (ENABLED == enable)? ENABLED : DISABLED;
    if (HAL_IS_PHY_EXIST(unit, port))
    {
        if (HAL_IS_PHY_8218(unit, port))
        {
            port0 = port & ~(0x3);
            if (pMac_info[unit]->green_enable[port0] == ENABLED &&
                pMac_info[unit]->green_enable[port0+1] == ENABLED &&
                pMac_info[unit]->green_enable[port0+2] == ENABLED &&
                pMac_info[unit]->green_enable[port0+3] == ENABLED)
            {
                val = (val & (~(0xF << 0))) | (0x4 << 0);
            }
            else
            {
                val = (val & (~(0xF << 0))) | (0xB << 0);
            }
            if ((ret = reg_write(unit, (uint32)portSerdesControl_regidx[port0], val)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }
        }
        else if (HAL_IS_PHY_8214_8214F(unit, port))
        {
            port0 = port & ~(0x1);
            if (pMac_info[unit]->green_enable[port0] == ENABLED &&
                pMac_info[unit]->green_enable[port0+1] == ENABLED)
            {
                val = (val & (~(0xF << 0))) | (0x4 << 0);
            }
            else
            {
                val = (val & (~(0xF << 0))) | (0xB << 0);
            }
            if ((ret = reg_write(unit, (uint32)portSerdesControl_regidx[port0], val)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }
        }
    }

    PORT_SEM_UNLOCK(unit);

    /* Configure if PHY supported green feature */
    ret = phy_greenEnable_set(unit, port, enable);
#endif

    return RT_ERR_OK;
} /* end of dal_maple_port_linkDownGreenEnable_set */

#if defined(CONFIG_SDK_DRIVER_NIC_USER_MODE)
/* Function Name:
 *      _dal_maple_port_linkChange_isr
 * Description:
 *      switch link change interrupt
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
_dal_maple_port_linkChange_isr(uint32 unit, void *isr_param)
{
    rtk_portmask_t  changed_portmask;

    RTK_PORTMASK_WORD_SET(changed_portmask, 0, *((uint32 *)isr_param));
    RTK_PORTMASK_WORD_SET(changed_portmask, 1, *((uint32 *)isr_param+1));

    /* if callback function exist, notify upper component the newest changed portmask */
    if (NULL != link_change_callback_f[unit])
    {
        link_change_callback_f[unit](unit, &changed_portmask);
    }

    return ;
} /* end of _dal_maple_port_linkChange_isr */
#endif /* CONFIG_SDK_DRIVER_NIC_USER_MODE */

/* Function Name:
 *      dal_maple_port_vlanBasedIsolationEntry_get
 * Description:
 *      Get the vlan-based isolation entry in the specific unit
 * Input:
 *      unit   - unit id
 *      index  - index id
 * Output:
 *      pEntry - pointer to vlan-based port isolation entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_maple_port_vlanBasedIsolationEntry_get(uint32 unit, uint32 index, rtk_port_vlanIsolationEntry_t *pEntry)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, index=%d", unit, index);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(index > HAL_MAX_NUM_OF_VLAN_PORT_ISO_ENTRY(unit), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pEntry), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);

    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, MAPLE_PORT_ISO_VB_ISO_PM_CTRLr,
                        REG_ARRAY_INDEX_NONE, index, MAPLE_VALIDf, &pEntry->enable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "ret=0x%x", ret);
        return ret;
    }

    if ((ret = reg_array_field_read(unit, MAPLE_PORT_ISO_VB_ISO_PM_CTRLr,
                        REG_ARRAY_INDEX_NONE, index, MAPLE_VIDf, &pEntry->vid)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "ret=0x%x", ret);
        return ret;
    }

    if ((ret = reg_array_field_read(unit, MAPLE_PORT_ISO_VB_ISO_PM_CTRLr,
                        REG_ARRAY_INDEX_NONE, index, MAPLE_VB_ISO_MBRf, &pEntry->portmask.bits[0])) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "ret=0x%x", ret);
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "valid=%d, vid=%d, pPortmask[0]=0x%x",
        pEntry->enable, pEntry->vid, pEntry->portmask.bits[0]);

    return RT_ERR_OK;
} /* end of dal_maple_port_vlanBasedIsolationEntry_get */

/* Function Name:
 *      dal_maple_port_vlanBasedIsolationEntry_set
 * Description:
 *      Set the vlan-based isolation entry in the specific unit
 * Input:
 *      unit   - unit id
 *      index  - index id
 *      pEntry - pointer to vlan-based port isolation entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 *      RT_ERR_VLAN_VID - invalid vid
 * Note:
 *      None
 */
int32
dal_maple_port_vlanBasedIsolationEntry_set(uint32 unit, uint32 index, rtk_port_vlanIsolationEntry_t *pEntry)
{
    int32   ret;
    uint32  i, vid;
#if 0
    rtk_enable_t enable;
#endif
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, index=%d", unit, index);
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "valid=%d, vid=%d, pPortmask[0]=0x%x",
        pEntry->enable, pEntry->vid, pEntry->portmask.bits[0]);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(index > HAL_MAX_NUM_OF_VLAN_PORT_ISO_ENTRY(unit), RT_ERR_INPUT);
    RT_PARAM_CHK((pEntry->enable >= RTK_ENABLE_END), RT_ERR_INPUT);
    RT_PARAM_CHK((pEntry->vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    PORT_SEM_LOCK(unit);

    /* Search exist entry */
    for (i = 0; i < HAL_MAX_NUM_OF_VLAN_PORT_ISO_ENTRY(unit); i++)
    {
        if ((ret = reg_array_field_read(unit, MAPLE_PORT_ISO_VB_ISO_PM_CTRLr,
                            REG_ARRAY_INDEX_NONE, i, MAPLE_VIDf, &vid)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "ret=0x%x", ret);
            return ret;
        }

        if (vid != 0 && vid == pEntry->vid)
        {
            if (index != i)
            {
                PORT_SEM_UNLOCK(unit);
                return RT_ERR_PORT_VLAN_ISO_VID_EXIST_IN_OTHER_IDX;
            }
        }
    }

    /* set value to CHIP*/
    if ((ret = reg_array_field_write(unit, MAPLE_PORT_ISO_VB_ISO_PM_CTRLr,
                        REG_ARRAY_INDEX_NONE, index, MAPLE_VALIDf, &pEntry->enable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "ret=0x%x", ret);
        return ret;
    }

    if ((ret = reg_array_field_write(unit, MAPLE_PORT_ISO_VB_ISO_PM_CTRLr,
                        REG_ARRAY_INDEX_NONE, index, MAPLE_VIDf, &pEntry->vid)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "ret=0x%x", ret);
        return ret;
    }

    if ((ret = reg_array_field_write(unit, MAPLE_PORT_ISO_VB_ISO_PM_CTRLr,
                        REG_ARRAY_INDEX_NONE, index, MAPLE_VB_ISO_MBRf, &pEntry->portmask.bits[0])) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "ret=0x%x", ret);
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_maple_port_vlanBasedIsolationEntry_set */

/* Function Name:
 *      dal_maple_port_vlanBasedIsolation_vlanSource_get
 * Description:
 *      Get comparing VID type of VLAN-based port isolation
 * Input:
 *      unit   - unit id
 * Output:
 *      pVlanSrc - point to vlan isolation source
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_maple_port_vlanBasedIsolation_vlanSource_get(uint32 unit, rtk_port_vlanIsolationSrc_t *pVlanSrc)
{
    int32   ret;
    uint32  value;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pVlanSrc), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);

    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, MAPLE_PORT_ISO_VB_CTRLr, MAPLE_VLAN_TYPEf, &value)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    switch(value)
    {
        case 0:
            *pVlanSrc = VLAN_ISOLATION_SRC_INNER;
            break;
        case 1:
            *pVlanSrc = VLAN_ISOLATION_SRC_OUTER;
            break;
        case 2:
            *pVlanSrc = VLAN_ISOLATION_SRC_FORWARD;
            break;
        default:
            return RT_ERR_FAILED;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pVlanSrc=%d", *pVlanSrc);

    return RT_ERR_OK;
} /* end of dal_maple_port_vlanBasedIsolation_vlanSource_get */

/* Function Name:
 *      dal_maple_port_vlanBasedIsolation_vlanSource_set
 * Description:
 *      Set comparing VID type of VLAN-based port isolation
 * Input:
 *      unit    - unit id
 *      vlanSrc - vlan isolation source
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
dal_maple_port_vlanBasedIsolation_vlanSource_set(uint32 unit, rtk_port_vlanIsolationSrc_t vlanSrc)
{
    int32   ret;
    uint32 value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, vlanSrc=%d", unit, vlanSrc);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((vlanSrc >= VLAN_ISOLATION_SRC_END), RT_ERR_INPUT);

    switch (vlanSrc)
    {
        case VLAN_ISOLATION_SRC_INNER:
            value = 0;
            break;
        case VLAN_ISOLATION_SRC_OUTER:
            value = 1;
            break;
        case VLAN_ISOLATION_SRC_FORWARD:
            value = 2;
            break;
        default:
            return RT_ERR_INPUT;
    }

    PORT_SEM_LOCK(unit);

    /* set value to CHIP*/
    if ((ret = reg_field_write(unit, MAPLE_PORT_ISO_VB_CTRLr, MAPLE_VLAN_TYPEf, &value)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_maple_port_vlanBasedIsolation_vlanSource_set */

/* Function Name:
 *      dal_maple_port_phyComboPortFiberMedia_get
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
dal_maple_port_phyComboPortFiberMedia_get(uint32 unit, rtk_port_t port, rtk_port_fiber_media_t *pMedia)
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
} /* end of dal_maple_port_phyComboPortFiberMedia_get */


/* Function Name:
 *      dal_maple_port_phyComboPortFiberMedia_set
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
dal_maple_port_phyComboPortFiberMedia_set(uint32 unit, rtk_port_t port, rtk_port_fiber_media_t media)
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
} /* end of dal_maple_port_phyComboPortFiberMedia_set */

/* Function Name:
 *      dal_maple_port_linkDownPowerSavingEnable_get
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
dal_maple_port_linkDownPowerSavingEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
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
} /* end of dal_maple_port_linkDownPowerSavingEnable_get */

/* Function Name:
 *      dal_maple_port_linkDownPowerSavingEnable_set
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
dal_maple_port_linkDownPowerSavingEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
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
} /* end of dal_maple_port_linkDownPowerSavingEnable_set */


/* Function Name:
 *      dal_maple_port_gigaLiteEnable_get
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
dal_maple_port_gigaLiteEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
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
} /* end of dal_maple_port_gigaLiteEnable_get */

/* Function Name:
 *      dal_maple_port_gigaLiteEnable_set
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
dal_maple_port_gigaLiteEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, enable=%d",
           unit, port, enable);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable != DISABLED && enable != ENABLED), RT_ERR_INPUT);

    /* Configure if PHY supported giga-lite feature */
    PORT_SEM_LOCK(unit);

    pMac_info[unit]->gigaLite_enable[port] = enable;

    /* Set PHY enable GigaLite ability */
    if ((ret = phy_gigaLiteEnable_set(unit, port, enable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }


    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_maple_port_gigaLiteEnable_set */


/* Function Name:
 *      dal_maple_port_greenEnable_get
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
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_maple_port_greenEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);

    if ((ret = phy_greenEnable_get(unit, port, pEnable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        return ret;
    }
    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_maple_port_greenEnable_get */

/* Function Name:
 *      dal_maple_port_greenEnable_set
 * Description:
 *      Set the status of green feature of the specific port in the specific unit
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of green feature
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      None
 */
int32
dal_maple_port_greenEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
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

    /* Configure if PHY supported green feature */
    if ((ret = phy_greenEnable_set(unit, port, enable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;

	
} /* end of dal_maple_port_greenEnable_set */


/* Function Name:
 *      dal_maple_port_phyCrossOverMode_get
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
dal_maple_port_phyCrossOverMode_get(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t *pMode)
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
    if ((ret = phy_crossOverMode_get(unit, port, pMode)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pMode=%d", *pMode);

    return RT_ERR_OK;
}/* end of dal_maple_port_phyCrossOverMode_get */

/* Function Name:
 *      dal_maple_port_phyCrossOverMode_set
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
dal_maple_port_phyCrossOverMode_set(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t mode)
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
}/* end of dal_maple_port_phyCrossOverMode_set */

/* Function Name:
 *      dal_maple_port_phyCrossOverStatus_get
 * Description:
 *      Get cross over status in the specified port.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pStatus - pointer to cross over mode status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_PHY_FIBER_LINKUP - This feature is not supported in this mode 
 * Note:
 *      Following value is valid
 *      - PORT_CROSSOVER_STATUS_MDI
 *      - PORT_CROSSOVER_STATUS_MDIX
 */
int32
dal_maple_port_phyCrossOverStatus_get(uint32 unit, rtk_port_t port, rtk_port_crossOver_status_t *pStatus)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", unit, port);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pStatus), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);

    /* get value from CHIP*/
    if ((ret = phy_crossOverStatus_get(unit, port, pStatus)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pStatus=%d", *pStatus);

    return RT_ERR_OK;
}/* end of dal_maple_port_phyCrossOverStatus_get */

/* Function Name:
 *      dal_maple_port_flowCtrlEnable_get
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
dal_maple_port_flowCtrlEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
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

    if ((ret = dal_maple_port_phyAutoNegoEnable_get(unit, port, &nway_enable)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    if (ENABLED == nway_enable)
    {
        if ((ret = dal_maple_port_phyAutoNegoAbility_get(unit, port, &ability)) != RT_ERR_OK)
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
        if ((ret = dal_maple_port_phyForceModeAbility_get(unit, port, &speed, &duplex, &flowctrl_enable)) != RT_ERR_OK)
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }
        (*pEnable) = flowctrl_enable;
    }

    return RT_ERR_OK;
} /* end of dal_maple_port_flowCtrlEnable_get */

/* Function Name:
 *      dal_maple_port_flowCtrlEnable_set
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
dal_maple_port_flowCtrlEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
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

    if ((ret = dal_maple_port_phyAutoNegoAbility_get(unit, port, &ability)) != RT_ERR_OK)
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
    if ((ret = dal_maple_port_phyAutoNegoAbility_set(unit, port, &ability)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    if ((ret = dal_maple_port_phyForceModeAbility_get(unit, port, &speed, &duplex, &flowctrl_enable)) != RT_ERR_OK)
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
    if ((ret = dal_maple_port_phyForceModeAbility_set(unit, port, speed, duplex, flowctrl_enable)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_maple_port_flowCtrlEnable_set */



/* Function Name:
 *      dal_maple_port_linkMedia_get
 * Description:
 *      Get link status and media
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      pStatus - link status
 *      pMedia - link media
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
dal_maple_port_linkMedia_get(uint32 unit, rtk_port_t port,
    rtk_port_linkStatus_t *pStatus, rtk_port_media_t *pMedia)
{
    uint32  val;
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", unit, port);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);

    PORT_SEM_LOCK(unit);

    /* get value from CHIP*/
    if ((ret = reg_read(unit, MAPLE_MAC_LINK_STSr, &val)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    if ((ret = reg_read(unit, MAPLE_MAC_LINK_STSr, &val)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    if (0 == (val & (1UL<<port)))
        *pStatus = PORT_LINKDOWN;
    else
    {
        *pStatus = PORT_LINKUP;
        if ((ret = reg_read(unit, MAPLE_MAC_LINK_MEDIA_STSr, &val)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }

        if (0 == (val & (1UL<<port)))
            *pMedia = PORT_MEDIA_COPPER;
        else
            *pMedia = PORT_MEDIA_FIBER;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_maple_port_linkMedia_get */



/************************************ WATCH DOG For Recovery****************************************/
/* Function Name:
 *      _dal_maple_mac_serdes_rst
 * Description:
 *      Reset Serdes and original patch are kept.
 * Input:
 *      unit    - unit id
 *      sds_num    - serdes id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_OUT_OF_RANGE - Serdes index is not support.
 * Note:
 *      None
 */
static int32
_dal_maple_mac_serdes_rst(uint32  unit, uint32 sds_num)
{
    int32 ret;
    uint32 val;

    /*rx reset*/
    ioal_mem32_read(unit, 0xF3A4+sds_num*0x100, &val);
    val |= 1UL<<9;
    ioal_mem32_write(unit, 0xF3A4+sds_num*0x100, val);
    val &= ~(1UL<<9);
    ioal_mem32_write(unit, 0xF3A4+sds_num*0x100, val);

    /*cmu reset*/
    val = 0x4040;
    ioal_mem32_write(unit, 0xF380+sds_num*0x100, val);
    val = 0x4740;
    ioal_mem32_write(unit, 0xF380+sds_num*0x100, val);
    val = 0x47c0;
    ioal_mem32_write(unit, 0xF380+sds_num*0x100, val);
    val = 0x4000;
    ioal_mem32_write(unit, 0xF380+sds_num*0x100, val);

    /*software reset*/
    val = 0x7146;
    ioal_mem32_write(unit, 0xE78C+sds_num*0x200, val);
    val = 0x7106;
    ioal_mem32_write(unit, 0xE78C+sds_num*0x200, val);

    /*tx & rx reset*/
    val = 0x0c00;
    ioal_mem32_write(unit, 0xE780+sds_num*0x200, val);
    val = 0x0c03;
    ioal_mem32_write(unit, 0xE780+sds_num*0x200, val);

    ret = RT_ERR_OK;
    return ret;
}   /* end of _dal_maple_mac_serdes_rst */



/* Function Name:
 *      _dal_maple_phy_serdes_rst
 * Description:
 *      Reset PHY Serdes.
 * Input:
 *      unit    - unit id
 *      sds_num    - serdes id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_OUT_OF_RANGE - Serdes index is not support.
 * Note:
 *      None
 */
static int32
_dal_maple_phy_serdes_rst(uint32  unit, uint32 sds_num)
{
    int32 ret;
    uint32 port;

    if (HAL_IS_RTL8380_FAMILY_ID(unit))
    {
        if(sds_num >= 6)
            return RT_ERR_FAILED;

        /*Init kill waring*/
        port = 0;

        if((0 == sds_num) || (1 == sds_num))
        {
            port = 0;
        }
        else if((2 == sds_num) || (3 == sds_num))
        {
            port = 16;
        }
        else if((4 == sds_num) || (5 == sds_num))
        {
            port = 24;
        }

        /*Reset 18b or 14fc PHY Serdes*/
        hal_miim_write(unit,  port,  0x0000, 0x1e, 0x0008);
        hal_miim_write(unit,  port,  0x0464,  0x17, 0x84f5);
        hal_miim_write(unit,  port,  0x0464,  0x17, 0x04f5);

        hal_miim_write(unit,  port,  0x042D,  0x11, 0xC015);
        hal_miim_write(unit,  port,  0x042D,  0x11, 0xC014);

        hal_miim_write(unit,  port,  0x0467,  0x14, 0x1415);
        hal_miim_write(unit,  port,  0x0467,  0x14, 0x3C3D);
        hal_miim_write(unit,  port,  0x0467,  0x14, 0x3C3F);
        hal_miim_write(unit,  port,  0x0467,  0x14, 0x0000);

        hal_miim_write(unit,  port,  0x0261,  0x10, 0x6000);
        hal_miim_write(unit,  port,  0x0261,  0x10, 0x0000);

        hal_miim_write(unit,  port,  0x0404,  0x13, 0x7146);
        hal_miim_write(unit,  port,  0x0404,  0x13, 0x7106);

        hal_miim_write(unit,  port,  0x0424,  0x13, 0x7146);
        hal_miim_write(unit,  port,  0x0424,  0x13, 0x7106);

        hal_miim_write(unit,  port,  0x0000,  0x1e, 0x0000);

    }

    ret = RT_ERR_OK;
    return ret;
}   /* end of _dal_maple_phy_serdes_rst */


/************************************PKTBUFFER WATCH DOG****************************************/
/************************************PKTBUFFER WATCH DOG--->Auto QueQue Reset********************/
/* Function Name:
 *      _dal_maple_port_auto_swqrst_check
 * Description:
 *      Auto Software Queue Reset check.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is monitor for auto software queue reset, if happens, reset nic.
 */
static int32 _dal_maple_port_auto_swqrst_check(uint32 unit)
{
    uint32 value;

    ioal_mem32_read(unit, 0xD050, &value);
    if((value>>29) & 0x1)
    {
    	pktBuf_watchdog_cnt++;
       if(pktBuf_watchdog_cnt >= 64)
        pktBuf_watchdog_cnt = 0;

        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "[PKTBUFFER-WATCHDOG] --->Auto Software Queue Reset\n");

        /* Do the nic-reset operation*/
        drv_nic_reset(unit);
        value |= (1UL<<29);
        ioal_mem32_write(unit, 0xD050, value);
    }
    return RT_ERR_OK;
}

/************************************PKTBUFFER WATCH DOG--->PHY Counter Check********************/
/* Function Name:
 *      _dal_maple_port_phy_counter_frcclk_off
 * Description:
 *      Phy counter monitor force clock off.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is used to turn off force tx clk of specified port.
 */
static int32 _dal_maple_port_phy_counter_frcclk_off(uint32 unit, rtk_port_t port)
{
    int32 ret;
    uint32 phy_data;

    /*838x*/
    if (HAL_IS_RTL8380_FAMILY_ID(unit))
    {
        ret = hal_miim_read(unit,  port,  0xc40,  21, &phy_data);
        if(ret != RT_ERR_OK)
            return ret;

        phy_data &= ~(1UL<<13);

        ret = hal_miim_write(unit,  port,  0xc40,  21, phy_data);
        if(ret != RT_ERR_OK)
            return ret;
    }

    /*833x*/
    if (HAL_IS_RTL8330_FAMILY_ID(unit))
    {
        /*18b internal phy*/
        if((port>=8)&&(port<=15))
        {  /*It is better to check PHYID*/
            ret = hal_miim_read(unit,  port,  0xc40,  21, &phy_data);
            if(ret != RT_ERR_OK)
                return ret;

            phy_data &= ~(1UL<<13);

            ret = hal_miim_write(unit,  port,  0xc40,  21, phy_data);
            if(ret != RT_ERR_OK)
                return ret;
        }
    }

    ret =  RT_ERR_OK;
    return ret;
}




/* Function Name:
 *      _dal_maple_port_phy_counter_clr_counter
 * Description:
 *      Phy counter monitor to clear phy counter.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is used to to clear phy counter of specified port.
 */
static int32 _dal_maple_port_phy_counter_clr_counter(uint32 unit, rtk_port_t port)
{
    int32 ret;
    uint32 phy_data;

    if (HAL_IS_RTL8380_FAMILY_ID(unit))
    {
        /*8218B & 8214FC*/
        /*It is better to check PHYID*/
        phy_data = 0x70;
        ret = hal_miim_write(unit,  port,  0xc80,  17, phy_data);
        if(ret != RT_ERR_OK)
            return ret;
    }

    if (HAL_IS_RTL8330_FAMILY_ID(unit))
    {
        /*It is better to check PHYID*/
        if((port>=0)&&(port<=7))
        {
             /*8208*/
             /*While clear, phyid should be 0*/
            ret = hal_miim_read(unit,  0,  65,  20, &phy_data);
            if(ret != RT_ERR_OK)
                return ret;

            phy_data &= ~(1UL<<port);

            ret = hal_miim_write(unit,  0,  65,  20, phy_data);
            if(ret != RT_ERR_OK)
                return ret;

            ret = hal_miim_read(unit,  0,  65,  20, &phy_data);
            if(ret != RT_ERR_OK)
                return ret;

            phy_data |= (1UL<<port);

            ret = hal_miim_write(unit,  0,  65,  20, phy_data);
            if(ret != RT_ERR_OK)
                return ret;
        }
        else if((port>=8)&&(port<=15))
        {
            /*Internal phy*/
            phy_data = 0x70;
            ret = hal_miim_write(unit,  port,  0xc80,  17, phy_data);
            if(ret != RT_ERR_OK)
                return ret;
        }
        else if((port>=16)&&(port<=23))
        {
            /*08L*/
             /*While clear, phyid should be 16*/
            ret = hal_miim_read(unit,  16,  65,  20, &phy_data);
            if(ret != RT_ERR_OK)
                return ret;

            phy_data &= ~(1UL<<(port-16));

            ret = hal_miim_write(unit,  16,  65,  20, phy_data);
            if(ret != RT_ERR_OK)
                return ret;

            ret = hal_miim_read(unit,  16,  65,  20, &phy_data);
            if(ret != RT_ERR_OK)
                return ret;

            phy_data |= (1UL<<(port-16));

            ret = hal_miim_write(unit,  16,  65,  20, phy_data);
            if(ret != RT_ERR_OK)
                return ret;
        }
        else  if((port>=24)&&(port<=27))
        {
            /*8214B or 8212B*/
            phy_data = 0x4012;
            ret = hal_miim_write(unit,  port,  6,  1, phy_data);
            if(ret != RT_ERR_OK)
                return ret;
        }
    }

    ret =  RT_ERR_OK;
    return ret;
}




/* Function Name:
 *      _dal_maple_port_phy_counter_monitor_begin
 * Description:
 *      Phy counter monitor force clock on.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is used to turn on force tx clk of specified port.
 */
static int32 _dal_maple_port_phy_counter_monitor_begin(uint32 unit, rtk_port_t port, uint8 speed)
{
    int32 ret;
    uint32 phy_data;

    /*838X*/
    if (HAL_IS_RTL8380_FAMILY_ID(unit))
    {
        if(speed == 0x1)
        {
            /*A: writephy  portID  reg_21[5:3]  page:0xc40  data:4 */
            ret = hal_miim_read(unit,  port,  0xc40,  21, &phy_data);
            if(ret != RT_ERR_OK)
                return ret;
            phy_data &= ~(0x7UL<<3);
            phy_data |= 0x4UL<<3;
            ret = hal_miim_write(unit,  port,  0xc40,  21, phy_data);
            if(ret != RT_ERR_OK)
                return ret;

           /* B: writephy  portID  reg_21[13]  page:0xc40  data:1 */
            ret = hal_miim_read(unit,  port,  0xc40,  21, &phy_data);
            if(ret != RT_ERR_OK)
                return ret;
            phy_data &= ~(0x1UL<<13);
            phy_data |= 0x1UL<<13;
            ret = hal_miim_write(unit,  port,  0xc40,  21, phy_data);
            if(ret != RT_ERR_OK)
                return ret;

            /* C:Clear Counter writephy  portID  reg_17  page:0xc80  data:0x70 */
            phy_data = 0x70;
            ret = hal_miim_write(unit,  port,  0xc80,  17, phy_data);
            if(ret != RT_ERR_OK)
                return ret;
        }
        else if(speed == 0x0)
        {
             /* A: writephy  portID  reg_21[5:3]  page:0xc40  data:5 */
            ret = hal_miim_read(unit,  port,  0xc40,  21, &phy_data);
            if(ret != RT_ERR_OK)
                return ret;
            phy_data &= ~(0x7UL<<3);
            phy_data |= 0x5UL<<3;
            ret = hal_miim_write(unit,  port,  0xc40,  21, phy_data);
            if(ret != RT_ERR_OK)
                return ret;

           /* B: writephy  portID  reg_21[13]  page:0xc40  data:1 */
            ret = hal_miim_read(unit,  port,  0xc40,  21, &phy_data);
            if(ret != RT_ERR_OK)
                return ret;
            phy_data &= ~(0x1UL<<13);
            phy_data |= 0x1UL<<13;
            ret = hal_miim_write(unit,  port,  0xc40,  21, phy_data);
            if(ret != RT_ERR_OK)
                return ret;

            /* C:Clear Counter writephy  portID  reg_17  page:0xc80  data:0x70 */
            phy_data = 0x70;
            ret = hal_miim_write(unit,  port,  0xc80,  17, phy_data);
            if(ret != RT_ERR_OK)
                return ret;
        }
        else
        {
            _dal_maple_port_phy_counter_frcclk_off(unit, port);
            _dal_maple_port_phy_counter_clr_counter(unit, port);
        }
    }

    /*833X*/
    if (HAL_IS_RTL8330_FAMILY_ID(unit))
    {
        /*Set internal PHY force clk*/
        if((port>=8)&&(port<=15))
        {
            if(speed == 0x1)
            {
                /*A: writephy  portID  reg_21[5:3]  page:0xc40  data:4 */
                ret = hal_miim_read(unit,  port,  0xc40,  21, &phy_data);
                if(ret != RT_ERR_OK)
                    return ret;
                phy_data &= ~(0x7UL<<3);
                phy_data |= 0x4UL<<3;
                ret = hal_miim_write(unit,  port,  0xc40,  21, phy_data);
                if(ret != RT_ERR_OK)
                    return ret;

               /* B: writephy  portID  reg_21[13]  page:0xc40  data:1 */
                ret = hal_miim_read(unit,  port,  0xc40,  21, &phy_data);
                if(ret != RT_ERR_OK)
                    return ret;
                phy_data &= ~(0x1UL<<13);
                phy_data |= 0x1UL<<13;
                ret = hal_miim_write(unit,  port,  0xc40,  21, phy_data);
                if(ret != RT_ERR_OK)
                    return ret;

            }
            else if(speed == 0x0)
            {
                 /* A: writephy  portID  reg_21[5:3]  page:0xc40  data:5 */
                ret = hal_miim_read(unit,  port,  0xc40,  21, &phy_data);
                if(ret != RT_ERR_OK)
                    return ret;
                phy_data &= ~(0x7UL<<3);
                phy_data |= 0x5UL<<3;
                ret = hal_miim_write(unit,  port,  0xc40,  21, phy_data);
                if(ret != RT_ERR_OK)
                    return ret;

               /* B: writephy  portID  reg_21[13]  page:0xc40  data:1 */
                ret = hal_miim_read(unit,  port,  0xc40,  21, &phy_data);
                if(ret != RT_ERR_OK)
                    return ret;
                phy_data &= ~(0x1UL<<13);
                phy_data |= 0x1UL<<13;
                ret = hal_miim_write(unit,  port,  0xc40,  21, phy_data);
                if(ret != RT_ERR_OK)
                    return ret;

            }
        }

        ret = _dal_maple_port_phy_counter_clr_counter(unit, port);
        if(ret != RT_ERR_OK)
            return ret;

    }

    ret =  RT_ERR_OK;
    return ret;
}


/* Function Name:
 *      _dal_maple_port_phy_counter_monitor_end
 * Description:
 *      Phy counter monitor to clear phy counter.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is used to to clear phy counter of specified port.
 */
static int32 _dal_maple_port_phy_counter_monitor_end(uint32 unit, rtk_port_t port)
{
    int32 ret;
    uint32 phy_data;
    rtk_port_t port_id;

    if (HAL_IS_RTL8380_FAMILY_ID(unit))
    {
        phy_data = 0;

        /*Read  PKTGEN RXERR CNT*/
        ret = hal_miim_read(unit,  port,  0xc81,  18, &phy_data);
        if(ret != RT_ERR_OK)
            return ret;

        /*If out of range means TX CRC Error*/
         if(phy_data > 100)
         {
        	pktBuf_watchdog_cnt++;
               if(pktBuf_watchdog_cnt >= 64)
                pktBuf_watchdog_cnt = 0;

            /* Do the nic-reset operation*/
            drv_nic_reset(unit);

            /* Because of Queue Reset, Clear All port counters*/
            for(port_id = 0; port_id < 28; port_id++)
            {
                if(!HAL_IS_PHY_EXIST(unit, port_id))
                    continue;

                 if (HAL_IS_CPU_PORT(unit, port_id) || HAL_IS_SERDES_PORT(unit, port_id))
                    continue;

                    _dal_maple_port_phy_counter_clr_counter(unit, port_id);
            }
         }
    }

    if (HAL_IS_RTL8330_FAMILY_ID(unit))
    {
        phy_data = 0;

        /*It is better to check PHYID*/
        if(((port>=0)&&(port<=7)) || ((port>=16)&&(port<=23)))
        {
            /*Read Counter*/
            ret = hal_miim_read(unit,  port,  65,  24, &phy_data);
            if(ret != RT_ERR_OK)
                return ret;
        }
        else if((port>=8)&&(port<=15))
        {
            /*Read  PKTGEN RXERR CNT*/
            ret = hal_miim_read(unit,  port,  0xc81,  18, &phy_data);
            if(ret != RT_ERR_OK)
                return ret;
        }
        else if((port>=24)&&(port<=27))
        {
            /*8214B or 8212B*/
            ret = hal_miim_read(unit,  port,  6,  9, &phy_data);
            if(ret != RT_ERR_OK)
                return ret;
        }

         if(phy_data > 100)
         {
        	pktBuf_watchdog_cnt++;
               if(pktBuf_watchdog_cnt >= 64)
                pktBuf_watchdog_cnt = 0;

            /* Do the nic-reset operation*/
            drv_nic_reset(unit);

            /* Because of Queue Reset, Clear All port counters*/
            for(port_id = 0; port_id < 28; port_id++)
            {
                if(!HAL_IS_PHY_EXIST(unit, port_id))
                    continue;

                 if (HAL_IS_CPU_PORT(unit, port_id) || HAL_IS_SERDES_PORT(unit, port_id))
                    continue;

                    _dal_maple_port_phy_counter_clr_counter(unit, port_id);
            }
         }
    }

    ret =  RT_ERR_OK;
    return ret;
}


/* Function Name:
 *      _dal_maple_port_phy_counter_check
 * Description:
 *      Phy counter monitor phy counter.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is used to to check phy counter of specified port.
 */
static uint8 link_pre_sts[28];
static uint8 link_pre_spd[28];
static uint8 link_pre_dpx[28];
static uint8 link_pre_mda[28];
int32 _dal_maple_port_phy_counter_check(uint32 unit)
{
    int32 ret;
    rtk_port_t port;

    //uint32 phy_data;
    uint32 reg_val;

    uint8 link_sts0,link_sts1;
    uint32 speed_val;
    uint8 speed;
    uint8 duplex;
    uint32 link_media;

    uint8 link_temp;
    uint8 spd_temp;
    uint8 dpx_temp;
    uint8 media_temp;

     /*Solution:
        Note0: must care about linkmon&walmon race condition
        Note1: only confined to status linkup on full duplex state
        1: First time detected linkup, clear counter & force clk;
        2: Second time detected linkup, check counter whether out of range
            if Yes, do queue reset operation*/
    for(port = 0; port < 28; port++)
    {
        if(!HAL_IS_PHY_EXIST(unit, port))
            continue;

         if (HAL_IS_CPU_PORT(unit, port) || HAL_IS_SERDES_PORT(unit, port))
            continue;

#if 0
        /*Check PHY can be accessed or not yet?*/
        ret = hal_miim_read(unit,  port,  0x0,  2, &phy_data);
        if(ret != RT_ERR_OK)
            return ret;
        if(phy_data != 0x1c)
           continue;
#endif

        /*Link status First Time Read*/
        if ((ret = reg_field_read(unit, MAPLE_MAC_LINK_STSr, MAPLE_LINK_STS_27_0f, &reg_val)) != RT_ERR_OK)
            return ret;
        link_sts0 = (reg_val >> port) & 0x1;

        /*Link status Second Time Read*/
        if ((ret = reg_field_read(unit, MAPLE_MAC_LINK_STSr, MAPLE_LINK_STS_27_0f, &reg_val)) != RT_ERR_OK)
            return ret;
        link_sts1 = (reg_val >> port) & 0x1;

        /*Link Speed*/
        if ((ret = reg_array_field_read(unit, MAPLE_MAC_LINK_SPD_STSr, port, REG_ARRAY_INDEX_NONE, \
                    MAPLE_SPD_STS_27_0f,&speed_val)) != RT_ERR_OK)
            return ret;
        speed = speed_val & 0x3;

        /*Link Duplex*/
        if ((ret = reg_field_read(unit, MAPLE_MAC_LINK_DUP_STSr, MAPLE_DUP_STS_27_0f, &reg_val)) != RT_ERR_OK)
            return ret;
        duplex = (reg_val >> port) & 0x1;

        /*Link Media*/
        if ((ret = reg_field_read(unit, MAPLE_MAC_LINK_MEDIA_STSr, MAPLE_MEDIA_STS_27_0f, &reg_val)) != RT_ERR_OK)
            return ret;
        link_media = (reg_val >> port) & 0x1;

        /*Backup previous status*/
        link_temp =  link_pre_sts[port];
        spd_temp = link_pre_spd[port];
        dpx_temp = link_pre_dpx[port];
        media_temp = link_pre_mda[port];

         /*Save link status*/
         link_pre_sts[port] = link_sts1;
        link_pre_spd[port] = speed;
        link_pre_dpx[port] = duplex;
        link_pre_mda[port] = link_media;

        /*Current Port Link down*/
        if(link_sts1 == 0)
        {
            if(link_temp == 1)
            {
                ret = _dal_maple_port_phy_counter_frcclk_off(unit, port); /*Close Force Clock*/
                if(ret != RT_ERR_OK)
                    return ret;
            }
        }
        else/*Current Port Link up*/
        {
            /*If link status changes, taken as latch low*/
            if(link_temp == 0)
            {
                link_sts0 = 0;
            }
            else
            {
                 /*If speed or duplex or media change, taken as latch low*/
                if((spd_temp!= speed) || (dpx_temp != duplex) || (media_temp != link_media))
                {
                    link_sts0 = 0;
                }
            }

            if(link_sts0 == 0)
            {
                 /*Half Duplex or Fiber mode, donnot care*/
                 if((0 == duplex) || (0x1 == link_media))
                 {
                    ret = _dal_maple_port_phy_counter_frcclk_off(unit, port); /*Close Force Clock*/
                    if(ret != RT_ERR_OK)
                        return ret;
                 }
                 else/*Full Duplex & Copper*/
                 {
                    /*First time detected*/
                    ret = _dal_maple_port_phy_counter_monitor_begin(unit, port, speed);
                    if(ret != RT_ERR_OK)
                        return ret;
                 }
            }
            else
            {
                 if((0 != duplex) && (0x1 != link_media))
                 {
                    /*Second Time detected*/
                    ret = _dal_maple_port_phy_counter_monitor_end(unit, port);
                    if(ret != RT_ERR_OK)
                        return ret;
                 }
            }
        }
    }

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_maple_port_pktbuf_watchdog
 * Description:
 *      Monitor for packet buffer problem.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is monitor for detect packet buffer problem and recover it.
 */
int32
dal_maple_port_pktbuf_watchdog(uint32 unit)
{
    int32 ret;

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    PORT_SEM_LOCK(unit);

    /***************************1: Auto Software Queue Reset Start**************/
    if((ret = _dal_maple_port_auto_swqrst_check(unit))!= RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    /***************************1: Auto Software Queue Reset  End**************/

    /***************************2: PHY  Counter Check Start********************/
    if((ret = _dal_maple_port_phy_counter_check(unit))!= RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    /***************************2: PHY  Counter Check End*********************/

    PORT_SEM_UNLOCK(unit);
    return RT_ERR_OK;
}/* end of dal_maple_port_pktbuf_watchdog */



/************************************SERDES WATCH DOG****************************************/
/* Function Name:
 *      _dal_maple_port_serdes_linkdown_check
 * Description:
 *      Monitor for serdes link down problem.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is monitor serdes link down and recover it.
 */
static uint32 sds_rst_flag[]={0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
static int32
_dal_maple_port_serdes_linkdown_check(uint32 unit)
{
    int32 ret;
    int32 sds_mde;
    uint32 val;
    uint32 value0,value1,value2;
    uint32 sds_idx;
    uint32 sds_sts[]={0xF3F4, 0xF4F4, 0xF5F4, 0xF6F4, 0xF7F4, 0xF8F4};

    if (HAL_IS_RTL8380_FAMILY_ID(unit))
    {
        /*Check link status*/
        for(sds_idx = 0; sds_idx<sizeof(sds_sts)/sizeof(uint32); sds_idx++)
        {
            /*Check Serdes use or not?*/
            if((sds_idx == 0) || (sds_idx == 1))
            {
                if(!HAL_IS_PHY_EXIST(unit, 0))
                continue;
            }
            if((sds_idx == 2) || (sds_idx == 3))
            {
                if(!HAL_IS_PHY_EXIST(unit, 16))
                continue;
            }
            if((sds_idx == 4) || (sds_idx == 5))
            {
                if(!HAL_IS_PHY_EXIST(unit, 24))
                continue;

                if (HAL_IS_CPU_PORT(unit, 24) || HAL_IS_SERDES_PORT(unit, 24))
                    continue;
            }

            /*Check sds0-5 mode*/
            ioal_mem32_read(unit, 0x28, &val);
            if(sds_idx == 0x5)
            {
                continue;
            }
            else if(sds_idx == 0x4)
            {
                sds_mde = (val >> 5) & 0x1F;
            }
            else if(sds_idx == 0x3)
            {
                sds_mde = (val >> 10) & 0x1F;
            }
            else if(sds_idx == 0x2)
            {
                sds_mde = (val >> 15) & 0x1F;
            }
            else if(sds_idx == 0x1)
            {
                sds_mde = (val >> 20) & 0x1F;
            }
            else if(sds_idx == 0x0)
            {
                sds_mde = (val >> 25) & 0x1F;
            }
            else
            {
                sds_mde = 0x0;
            }
            /*Only process 5G-QSGMII*/
            if(sds_mde != 0x6)
                continue;

            /*5G Serdes mode*/
            ioal_mem32_read(unit, sds_sts[sds_idx], &value0);
            ioal_mem32_read(unit, sds_sts[sds_idx], &value1);
            ioal_mem32_read(unit, sds_sts[sds_idx], &value2);
            if((0x1FF == value1) && (0x1FF == value2))
            {
                /*Serdes linkup, work right!*/
                sds_rst_flag[sds_idx] = 0;
            }
            else
            {
                /*Serdes linkdown, work wrong!*/
                if(sds_rst_flag[sds_idx] == 0)
                {
                    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "[SERDES-WATCHDOG] --->Serdes Link Down--->5G-Serdes[%d]!\n", sds_idx);

                    /*First linkdown, work wrong!*/
                   _dal_maple_mac_serdes_rst(unit, sds_idx);
                    sds_rst_flag[sds_idx]++;
                	  macSerdes_watchdog_cnt++;
                   if(macSerdes_watchdog_cnt >= 64)
                    macSerdes_watchdog_cnt = 0;
                }
                else
                {
                    sds_rst_flag[sds_idx]++;
                    if(sds_rst_flag[sds_idx] == 5)
                    {
                        /*Not expected case occurs, Reset PHY  & MAC serdes*/
                        sds_rst_flag[sds_idx] = 1;
                       _dal_maple_mac_serdes_rst(unit, sds_idx);
                       _dal_maple_phy_serdes_rst(unit, sds_idx);
                    	  macSerdes_watchdog_cnt++;
                       if(macSerdes_watchdog_cnt >= 64)
                        macSerdes_watchdog_cnt = 0;
                    }
                }
            }
        }
    }
    else if (HAL_IS_RTL8330_FAMILY_ID(unit))
    {
        /*Check link status*/
        for(sds_idx = 0; sds_idx<sizeof(sds_sts)/sizeof(uint32); sds_idx++)
        {
            /*Check Serdes use or not?*/
            if((sds_idx == 0) || (sds_idx == 1))
            {
                if(!HAL_IS_PHY_EXIST(unit, 0))
                continue;
            }
            if((sds_idx == 2) || (sds_idx == 3))
            {
                if(!HAL_IS_PHY_EXIST(unit, 16))
                continue;
            }
            if((sds_idx == 4) || (sds_idx == 5))
            {
                if(!HAL_IS_PHY_EXIST(unit, 24))
                continue;

                if (HAL_IS_CPU_PORT(unit, 24) || HAL_IS_SERDES_PORT(unit, 24))
                    continue;
            }

           /*Check sds0-5 mode*/
            ioal_mem32_read(unit, 0x28, &val);
            if(sds_idx == 0x5)
            {
                /*2.5G Serdes mode-RSGMII*/
                sds_mde = (val >> 0) & 0x1F;
                if(0x1 != sds_mde)
                    continue;
            }
            else if(sds_idx == 0x4)
            {
                /*2.5G Serdes mode-RSGMII*/
                sds_mde = (val >> 5) & 0x1F;
                if(0x1 != sds_mde)
                    continue;
            }
            else if(sds_idx == 0x3)
            {
                /*2.5G Serdes mode-RS8MII*/
                sds_mde = (val >> 10) & 0x1F;
                if(0x9 != sds_mde)
                    continue;
            }
            else if(sds_idx == 0x2)
            {
                /*2.5G Serdes mode-RS8MII*/
                sds_mde = (val >> 15) & 0x1F;
                if(0x9 != sds_mde)
                    continue;
            }
            else if(sds_idx == 0x1)
            {
                /*2.5G Serdes mode-RS8MII*/
                sds_mde = (val >> 20) & 0x1F;
                if(0x9 != sds_mde)
                    continue;
            }
            else if(sds_idx == 0x0)
            {
                /*2.5G Serdes mode-RS8MII*/
                sds_mde = (val >> 25) & 0x1F;
                if(0x9 != sds_mde)
                    continue;
            }
            else
            {
                continue;
            }

            if((sds_idx>=0) && (sds_idx<=3))
            {
                /*2.5G Serdes mode-RS8MII*/
                ioal_mem32_read(unit, sds_sts[sds_idx], &value0);
                ioal_mem32_read(unit, sds_sts[sds_idx], &value1);
                ioal_mem32_read(unit, sds_sts[sds_idx], &value2);
                if((0x1FF == value1) && (0x1FF == value2))
                {
                    /*Serdes linkup, work right!*/
                    sds_rst_flag[sds_idx] = 0;
                }
                else
                {
                    /*Serdes linkdown, work wrong!*/
                    if(sds_rst_flag[sds_idx] == 0)
                    {
                        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "[SERDES-WATCHDOG] --->Serdes Link Down--->2.5GRS8MII-Serdes[%d]!", sds_idx);
                        /*First linkdown, work wrong!*/
                       _dal_maple_mac_serdes_rst(unit, sds_idx);
                        sds_rst_flag[sds_idx]++;
                    	  macSerdes_watchdog_cnt++;
                       if(macSerdes_watchdog_cnt >= 64)
                        macSerdes_watchdog_cnt = 0;
                    }
                    else
                    {
                        sds_rst_flag[sds_idx]++;
                        if(sds_rst_flag[sds_idx] == 5)
                        {
                            sds_rst_flag[sds_idx] = 0;
                        }
                    }
                }
            }
            else if((sds_idx>=4) && (sds_idx<=5))
            {
                /*2.5G Serdes mode-RSGMII*/
                ioal_mem32_read(unit, sds_sts[sds_idx], &value0);
                ioal_mem32_read(unit, sds_sts[sds_idx], &value1);
                ioal_mem32_read(unit, sds_sts[sds_idx], &value2);
                if((0x133 == value1) && (0x133 == value2))
                {
                    /*Serdes linkup*/
                    sds_rst_flag[sds_idx] = 0;
                }
                else
                {
                    /*Serdes linkdown, work wrong!*/
                    if(sds_rst_flag[sds_idx] == 0)
                    {
                        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "[SERDES-WATCHDOG] --->Serdes Link Down--->2.5GRSGMII-Serdes[%d]!", sds_idx);
                        /*First linkdown, work wrong!*/
                       _dal_maple_mac_serdes_rst(unit, sds_idx);
                        sds_rst_flag[sds_idx]++;
                    	  macSerdes_watchdog_cnt++;
                       if(macSerdes_watchdog_cnt >= 64)
                        macSerdes_watchdog_cnt = 0;
                    }
                    else
                    {
                        sds_rst_flag[sds_idx]++;
                        if(sds_rst_flag[sds_idx] == 5)
                        {
                            sds_rst_flag[sds_idx] = 0;
                        }
                    }
                }
            }
        }
    }

   ret =  RT_ERR_OK;
    return ret;
}



/* Function Name:
 *      dal_maple_port_serdes_watchdog
 * Description:
 *      Monitor for serdes link status.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is monitor serdes link down and recover it.
 */
int32
dal_maple_port_serdes_watchdog(uint32 unit)
{
    int32 ret;

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    PORT_SEM_LOCK(unit);


    /***************************1: Serdes Link Down Start*********************/
    if((ret = _dal_maple_port_serdes_linkdown_check(unit))!= RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    /***************************1: Serdes Link Down End*********************/

    PORT_SEM_UNLOCK(unit);
    return RT_ERR_OK;
}/* end of dal_maple_port_serdes_watchdog */


/************************************PHY WATCH DOG****************************************/
/************************************PHY WATCH DOG--->PHY Reset Check*********************/
/* Function Name:
 *      _dal_maple_port_phy_reset_check
 * Description:
 *      Phy Reset monitor.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is monitor for Phy Reset or not and recover it.
 */
static uint32 esdCheckOrder_rtl8380 = 0;
static uint32 esdRepatchFlag_rtl8380[32];


static int32 _dal_maple_port_phy_reset_check(uint32 unit)
{
    int32 ret;
    rtk_port_t port;
    uint32 phy_data;
    uint32 phy_access_flag;
    uint32 phy_rst_flag;
    uint32 reg_val;
    uint32 polling_msk;
    uint32 port_idx;
    rtk_port_t port_list[] = {0, 16, 24}; /*Internal Phy no need to check*/
    uint32 sds_no;
    uint32 sds_from,sds_to;

    /*only for RTL838X*/
    if (HAL_IS_RTL8380_FAMILY_ID(unit))
    {
        /*Check all phys chips have ever rested except internal phy?*/
	  for(port_idx = esdCheckOrder_rtl8380; port_idx == esdCheckOrder_rtl8380; esdCheckOrder_rtl8380++)
        {
           if(2 < esdCheckOrder_rtl8380)
   		esdCheckOrder_rtl8380 = 0;

            /* Port check */
            port = port_list[esdCheckOrder_rtl8380];
            if(!HAL_IS_PHY_EXIST(unit, port))
                continue;

             if (HAL_IS_CPU_PORT(unit, port) || HAL_IS_SERDES_PORT(unit, port))
                continue;

            /*Check PHY can be accessed or not yet?*/
            phy_access_flag = 0;
            ret = hal_miim_read(unit,  port,  0x0,  2, &phy_data);
            if(ret != RT_ERR_OK)
            {
                return ret;
            }
            if(phy_data == 0x1c)
            {
                phy_access_flag = 0x1;
            }

            /*PHY can be accessed*/
            if(0x1 == phy_access_flag)
            {
                /*Check whether External PHY has been reseted or not*/
                phy_rst_flag = 0x0;
                if(phy_chk_rst_status(unit, port, &phy_rst_flag) == RT_ERR_OK)
                {
                    /*This Phy really has been reseted*/
                    if(0x1 == phy_rst_flag)
                    {
                        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "[PHY-WATCHDOG] --->PHY RESET--->Port:%d found Reset!", port);

                    	  phy_watchdog_cnt++;
                       if(phy_watchdog_cnt >= 64)
                        phy_watchdog_cnt = 0;

                        /*Reset PHY again*/
                        ret = hal_miim_write(unit,  port,  0x0,  30, 0x8);
                        if(ret != RT_ERR_OK)
                        {
                            return ret;
                        }
                        ret = hal_miim_write(unit,  port,  0x262,  16, 0x1);
                        if(ret != RT_ERR_OK)
                        {
                            return ret;
                        }
                        ret = hal_miim_write(unit,  port,  0x0,  30, 0);
                        if(ret != RT_ERR_OK)
                        {
                            return ret;
                        }

                        /*Delay until PHY reset done*/
                        osal_time_usleep(100 * 1000); /* delay 100mS */


                        /*Re-Patch PHYs which have been reseted*/
                        /*Save polling mask*/
                        if ((ret = reg_read(unit, MAPLE_SMI_POLL_CTRLr, &polling_msk)) != RT_ERR_OK)
                        {
                            return ret;
                        }

                        /*Disable MAC polling PHY*/
                        reg_val = 0x0;
                        if ((ret = reg_write(unit, MAPLE_SMI_POLL_CTRLr, &reg_val)) != RT_ERR_OK)
                        {
                            return ret;
                        }

                        /*Do the PHY Re-Patch*/
                        if ((ret = phy_patch_set(unit, port)) != RT_ERR_OK)
                        {
                            /*Restore Polling mask to protect*/
                            reg_write(unit, MAPLE_SMI_POLL_CTRLr, &polling_msk);
                            return ret;
                        }


#if 0
                        /*Run the callback function*/
                        if ((ret = dal_waMon_phyReconfig_portMaskSet(unit, port)) != RT_ERR_OK)
                        {
                            /*Restore Polling mask to protect*/
                            reg_write(unit, MAPLE_SMI_POLL_CTRLr, &polling_msk);
                            return ret;
                        }
#else
			   esdRepatchFlag_rtl8380[port] = 0x1;
#endif

                        /*Restore MAC polling PHY*/
                        reg_val = polling_msk;
                        if ((ret = reg_write(unit, MAPLE_SMI_POLL_CTRLr, &reg_val)) != RT_ERR_OK)
                        {
                            return ret;
                        }

                        /*If any phy has been detected reseted yet, Reset  Serdes*/
                        sds_from = 0;
                        sds_to = 0;
                        if(port == 0)
                        {
                            sds_from = 0;
                            sds_to = 1;
                        }
                        else if(port == 16)
                        {
                            sds_from = 2;
                            sds_to = 3;
                        }
                        else if(port == 24)
                        {
                            sds_from = 4;
                            sds_to = 5;
                        }
                        for(sds_no = sds_from; sds_no <= sds_to; sds_no++)
                        {
                           _dal_maple_mac_serdes_rst(unit, sds_no);
                        }
                    }
                }
            }
        }
    }

    return RT_ERR_OK;
}


/************************************PHY WATCH DOG*******************************/
/* Function Name:
 *      dal_maple_port_phy_watchdog
 * Description:
 *      Monitor for phy.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is monitor for phy and recover it.
 */
int32
dal_maple_port_phy_watchdog(uint32 unit)
{
    int32 ret;
    uint32 port_idx;

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    PORT_SEM_LOCK(unit);

    /***************************1: PHY RESET Start*********************/
    if((ret = _dal_maple_port_phy_reset_check(unit))!= RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    /***************************1: PHY RESET  End**********************/

    PORT_SEM_UNLOCK(unit);

    /*Run the callback function*/
    for(port_idx = 0; port_idx < 28; port_idx++)
    {
    	if(0x1 == esdRepatchFlag_rtl8380[port_idx])
	{
    		dal_waMon_phyReconfig_portMaskSet(unit, port_idx);
		esdRepatchFlag_rtl8380[port_idx] = 0x0;
	}
    }

    return RT_ERR_OK;
}/* end of dal_maple_port_phy_watchdog */


/************************************WATCH DOG Debug Counter*******************************/
/* Function Name:
 *      dal_maple_port_watchdog_debug
 * Description:
 *      Set watchdog debug counter.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is used to set watchdog debug counter.
 */
int32
dal_maple_port_watchdog_debug(uint32 unit)
{
    uint32 int_val;
    uint32 reg_val;

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    PORT_SEM_LOCK(unit);

    ioal_mem32_read(unit, 0x0058, &int_val);
    ioal_mem32_write(unit, 0x0058, 0x3);
    reg_val = 0;
    reg_val = (pktBuf_watchdog_cnt & 0x3F)<<8;
    reg_val |= (macSerdes_watchdog_cnt & 0x3F)<<20;
    reg_val |= (phy_watchdog_cnt & 0x3F)<<2;
    ioal_mem32_write(unit, 0xad60, reg_val);
    ioal_mem32_write(unit, 0x0058, int_val);

    PORT_SEM_UNLOCK(unit);
    return RT_ERR_OK;
}/* end of dal_maple_port_watchdog_debug */


#if defined(CONFIG_SDK_WA_FIBER_RX_WATCHDOG)
/* Function Name:
 *      dal_maple_port_fiber_rx_watchdog
 * Description:
 *      check fiber RX is normal or not
 * Input:
 *      None.
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED - initialize fail
 *      RT_ERR_OK     - initialize success
 * Note:
 *
 */
static uint32 fiberRxOrder = 0;
static uint8 fiberRxWDSts[28];

int32 dal_maple_port_fiber_rx_watchdog(uint32 unit)
{
    uint32  port_num = 0;
    uint32  port_index;
    int32   phy_id;
    uint32  backup_reg30_data;//, backup_reg10_data;
    uint32  error_count, chk_error, chk_loop, reg_data;
    rtk_enable_t backup_enable;
    rtk_port_media_t portMedia;
    
    int32   chk_flag;

    chk_flag = 0;

    port_num = 28;

    if (fiberRxOrder >= port_num)
        fiberRxOrder = 0;

    PORT_SEM_LOCK(unit);

    /* Scan from last fiber port index, per-time chk_flag ports number */
    for(port_index = fiberRxOrder; (port_index < port_num) && (chk_flag < 1); port_index++)
    {
        chk_error = 0;

        if(HAL_IS_GE_COMBO_PORT(unit, port_index))
        {
            ++chk_flag;
             
            phy_drv_chk(unit, port_index, &phy_id);
            if((phy_id == RT_PHYDRV_RTL8218FB_MP) || (phy_id == RT_PHYDRV_RTL8214FC_MP))
            {
                uint32  clear_flag;
                uint32  reset_flag;

               clear_flag = 1;
               reset_flag = 0;

                phy_media_get(unit, port_index, &portMedia);
                if(portMedia == PORT_MEDIA_FIBER) /*Checking Media is Fiber*/
                {
                    /* only work for giga */
                    hal_miim_read(unit, port_index, 0, 0, &reg_data);
                    if ((0 == ((reg_data >> 13) & 0x1)) && (1 == ((reg_data >> 6) & 0x1)))
                    {
                             /*Checking Port is Link Down*/
                            hal_miim_read(unit, port_index, 0, 1, &reg_data);
                            hal_miim_read(unit, port_index, 0, 1, &reg_data);
                            if((reg_data & 0x4) == 0)
                            {
                                /*Backup MAC Polling PHY status*/
                                hal_miim_pollingEnable_get(unit, port_index, &backup_enable);
                                
                                 /*Disable MAC Polling PHY status*/
                                hal_miim_pollingEnable_set(unit, port_index, DISABLED);

                                /*Backup Page 0 Reg 30*/
                                hal_miim_read(unit, port_index, 0, 30, &backup_reg30_data);

                                /*Write page 0 reg 30 = 0x3*/
                                hal_miim_write(unit, port_index, 0, 30, 0x3); 

                                /*Write page 0xf reg 0x10 = 0x10*/
                                hal_miim_write(unit, port_index, 0xf, 0x10, 0x10); 

                                /*Check Error counter three times*/
                                for(chk_loop = 0; chk_loop < 3; chk_loop++)
                                {
                                    /*Read Error counter*/
                                    hal_miim_read(unit, port_index, 0xf, 0x11, &error_count);
                                    if(error_count == 0xffff)
                                        chk_error++;
                                }

                                /*Error happen*/
                                if(chk_error > 2)
                                { 
                                   reset_flag = 1;
                                }
                                else
                                {
                                   chk_error = 0;
   
                                   for(chk_loop = 0; chk_loop < 3; ++chk_loop)
                                   {
                                       hal_miim_read(unit, port_index, 0xf, 0x16, &reg_data);
                                       if (0x100 == (reg_data & 0x100))
                                       {
                                           if (0x10 != (reg_data & 0x10))
                                           {
                                               ++chk_error;
                                           }
                                       }
                                   }
   
                                   if (3 == chk_error)
                                   {
                                       if (1 == fiberRxWDSts[port_index])
                                       {
                                           reset_flag = 1;
                                       }
                                       else
                                       {
                                           fiberRxWDSts[port_index] = 1;
                                           clear_flag = 0;
                                       }
                                   }
                               }

                                /*Restore Page 0 Reg 30*/
                                hal_miim_write(unit, port_index, 0, 30, backup_reg30_data);
                                /*Restore MAC Polling PHY status*/
                                hal_miim_pollingEnable_set(unit, port_index, backup_enable);
                        }
                    }
                }

               /*Reset RX*/
               if (1 == reset_flag)
               {
                   fiber_rx_watchdog_cnt++;
                    
                   /*Backup MAC Polling PHY status*/
                   hal_miim_pollingEnable_get(unit, port_index, &backup_enable);
                   /*Disable MAC Polling PHY status*/
                   hal_miim_pollingEnable_set(unit, port_index, DISABLED);
                   
                    /*Backup Page 0 Reg 30*/
                    hal_miim_read(unit, port_index, 0, 30, &backup_reg30_data);
                    
                   hal_miim_write(unit, port_index, 0, 30, 0x3); /*Write page 0 reg 30 = 0x3*/
                   

                #if 1
                hal_miim_read(unit, port_index, 0xc, 0x16, &reg_data);
                reg_data |= (1 << 15);
                hal_miim_write(unit, port_index, 0xc, 0x16, reg_data);
                osal_time_usleep(100 * 1000);
                hal_miim_read(unit, port_index, 0xc, 0x16, &reg_data);
                reg_data &= ~(1 << 15);
                hal_miim_write(unit, port_index, 0xc, 0x16, reg_data);
                #else
                   hal_miim_read(unit, port_index, 0x8, 0x10, &backup_reg10_data);
                   reg_data = (backup_reg10_data & (0xffffffd));
                   hal_miim_write(unit, port_index, 0x8, 0x10, reg_data); /*Set bit1 to 0 to reset RX*/
                   reg_data = (backup_reg10_data | 0x2);
                   hal_miim_write(unit, port_index, 0x8, 0x10, reg_data); /*Set bit1 to 1 to reset RX*/
                #endif


                   
                   
                    /*Restore Page 0 Reg 30*/
                    hal_miim_write(unit, port_index, 0, 30, backup_reg30_data);
                    
                    /*Restore MAC Polling PHY status*/
                    hal_miim_pollingEnable_set(unit, port_index, backup_enable);
                    
               }
   
               if ((1 == clear_flag) || (1 == reset_flag))
               {
                   fiberRxWDSts[port_index] = 0;
               }
               
            }
        }
        /*Check SerDes RX for Direct Fiber module*/
        else if(HAL_IS_SERDES_PORT(unit, port_index))
        {
            ++chk_flag;
         
            /*For RTL838X & RTL833X*/
            if (HAL_IS_RTL8380_FAMILY_ID(unit) || HAL_IS_RTL8330_FAMILY_ID(unit))
            {
               uint32  reg;
               uint32  regData0;
               uint32  regData1;
               uint32  new_reg;
               uint32  clear_flag;
               uint32  reset_flag;
               
               clear_flag = 1;
               reset_flag = 0;
               
                reg_data = 0;
                
                if (24 == port_index)
                {
                    reg_write(unit, MAPLE_SDS4_EXT_REG24r, &reg_data);

                    reg_field_read(unit, MAPLE_SDS4_FIB_REG0r, MAPLE_CFG_FIB_SPD_RD_00f, &regData0);
                    reg_field_read(unit, MAPLE_SDS4_FIB_REG0r, MAPLE_CFG_FIB_SPD_RD_01f, &regData1);

                    reg = MAPLE_SDS4_EXT_REG25r;
                    new_reg = MAPLE_SDS4_EXT_REG29r;
                }
                else if (26 == port_index)
                {
                    reg_field_read(unit, MAPLE_SDS5_FIB_REG0r, MAPLE_CFG_FIB_SPD_RD_00f, &regData0);
                    reg_field_read(unit, MAPLE_SDS5_FIB_REG0r, MAPLE_CFG_FIB_SPD_RD_01f, &regData1);
                    
                    reg_write(unit, MAPLE_SDS5_EXT_REG24r, &reg_data);
                    
                    reg = MAPLE_SDS5_EXT_REG25r;
                    new_reg = MAPLE_SDS5_EXT_REG29r;
                }
                else
                    continue;

                if ((0 == regData0) && (1 == regData1))
                {
                        /* read & clear */
                        reg_field_read(unit, reg, MAPLE_MUX_SYMBOLERR_CNTf, &reg_data);

                        for(chk_loop = 0; chk_loop < 3; ++chk_loop)
                        {
                            reg_field_read(unit, reg, MAPLE_MUX_SYMBOLERR_CNTf, &reg_data);
                            if (0xFF == reg_data)
                            {
                                chk_error++;
                            }
                            osal_time_usleep(200);
                        }

                        if (chk_error >= 2)
                        { 
                            reset_flag = 1;
                        }
                        else
                        {
                            chk_error = 0;

                            for(chk_loop = 0; chk_loop < 3; ++chk_loop)
                            {
                                reg_read(unit, new_reg, &reg_data);
                                if (0x100 == (reg_data & 0x100))
                                {
                                    if (0x10 != (reg_data & 0x10))
                                    {
                                        ++chk_error;
                                    }
                                }
                            }

                            if (3 == chk_error)
                            {
                                if (1 == fiberRxWDSts[port_index])
                                {
                                    reset_flag = 1;
                                }
                                else
                                {
                                    fiberRxWDSts[port_index] = 1;
                                    clear_flag = 0;
                                }
                            }
                        }
                    }

                 /*Reset RX*/
                if(reset_flag)
                {
                    /*Error happen*/
                    fiber_rx_watchdog_cnt++;

                    if (24 == port_index)
                    {
                        reg = MAPLE_SDS4_REG0r;
                    }
                    else if (26 == port_index)
                    {
                        reg = MAPLE_SDS5_REG0r;
                    }

                    reg_data = 0;
                    reg_field_write(unit, reg, MAPLE_SDS_EN_RXf, &reg_data);

                    osal_time_usleep(200);

                    reg_data = 1;
                    reg_field_write(unit, reg, MAPLE_SDS_EN_RXf, &reg_data);

                }

               if ((1 == clear_flag) || (1 == reset_flag))
               {
                   fiberRxWDSts[port_index] = 0;
               }

            }
        }

    }

    /*Back up last time port_index*/
    fiberRxOrder = port_index;

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}
#endif  /* CONFIG_SDK_WA_FIBER_RX_WATCHDOG */


/************************************ComboPort Fiber Mode FC not work*******************************/
#if defined(CONFIG_SDK_WA_COMBO_FLOWCONTROL)
/* Function Name:
 *      _dal_maple_port_comboPort_fc_workaround
 * Description:
 *      Monitor for phy(combo port) flow control on fiber mode.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is Monitor for phy(combo port) flow control on fiber mode.
 */
int32 _dal_maple_port_comboPort_fc_workaround(uint32 unit)
{
    int32 ret;
    uint32 port_id;
    uint32 reg_val;
    uint32 speed_val;
    uint32 link_sts;
    uint32 link_media;
    uint32 link_speed;

    uint32 phy_data0;
    uint32 phy_data1;

    uint32 phy_data;

    uint32 tx_pause;
    uint32 rx_pause;

    uint32  reg_idx;
    uint32  temp;

    /*From Combo Port, right now portid range:24-27*/
    for(port_id = 0; port_id < 28; port_id++)
    {
        if(!HAL_IS_PHY_EXIST(unit, port_id))
            continue;

         if (HAL_IS_CPU_PORT(unit, port_id) || HAL_IS_SERDES_PORT(unit, port_id))
            continue;

	 if(!HAL_IS_GE_COMBO_PORT(unit, port_id))
            continue;

        /*Link Status First*/
        if ((ret = reg_field_read(unit, MAPLE_MAC_LINK_STSr, MAPLE_LINK_STS_27_0f, &reg_val)) != RT_ERR_OK)
            return ret;

        /*Link Status Second*/
        if ((ret = reg_field_read(unit, MAPLE_MAC_LINK_STSr, MAPLE_LINK_STS_27_0f, &reg_val)) != RT_ERR_OK)
            return ret;
        link_sts = (reg_val >> port_id) & 0x1;

	 if(0 == link_sts)
	 	continue;

        /*Page0 Reg0:BIT12--NWAY Enable*/
        ret = hal_miim_read(unit,  port_id,  0x0,  0, &phy_data);
        if(ret != RT_ERR_OK)
            return ret;
        phy_data = (phy_data>>12) & 0x1;

	 if(0 == phy_data)
	 	continue;

        /*Link Media*/
        if ((ret = reg_field_read(unit, MAPLE_MAC_LINK_MEDIA_STSr, MAPLE_MEDIA_STS_27_0f, &reg_val)) != RT_ERR_OK)
            return ret;
        link_media = (reg_val >> port_id) & 0x1;

        /*Link Speed*/
        if ((ret = reg_array_field_read(unit, MAPLE_MAC_LINK_SPD_STSr, port_id, REG_ARRAY_INDEX_NONE, \
                    MAPLE_SPD_STS_27_0f,&speed_val)) != RT_ERR_OK)
            return ret;
        link_speed = speed_val & 0x3;

        /*Page0 Reg4*/
        ret = hal_miim_read(unit,  port_id,  0x0,  4, &phy_data0);
        if(ret != RT_ERR_OK)
            return ret;

        /*Page0 Reg5*/
        ret = hal_miim_read(unit,  port_id,  0x0,  5, &phy_data1);
        if(ret != RT_ERR_OK)
            return ret;


        /*If LINK-UP-Fiber-1000M*/
        if((0x1 == link_media) && (0x2 == link_speed))
        {
            /*TX Pause*/
            tx_pause = (~((phy_data0>>7) & 0x1)) & ((phy_data0>>8) & 0x1) & ((phy_data1>>7) & 0x1) & ((phy_data1>>8) & 0x1);
            tx_pause |= ((phy_data0>>7) & 0x1) & ((phy_data1>>7) & 0x1);
            /*RX Pause*/
            rx_pause = ((phy_data0>>7) & 0x1) & ((phy_data0>>8) & 0x1) & (~((phy_data1>>7) & 0x1)) & ((phy_data1>>8) & 0x1);
            rx_pause |= ((phy_data0>>7) & 0x1) & ((phy_data1>>7) & 0x1);
        }
        else
        {
            /*TX Pause*/
            tx_pause = (~((phy_data0>>10) & 0x1)) & ((phy_data0>>11) & 0x1) & ((phy_data1>>10) & 0x1) & ((phy_data1>>11) & 0x1);
            tx_pause |= ((phy_data0>>10) & 0x1) & ((phy_data1>>10) & 0x1);
            /*RX Pause*/
            rx_pause = ((phy_data0>>10) & 0x1) & ((phy_data0>>11) & 0x1) & (~((phy_data1>>10) & 0x1)) & ((phy_data1>>11) & 0x1);
            rx_pause |= ((phy_data0>>10) & 0x1) & ((phy_data1>>10) & 0x1);
        }

        /* Need to configure [MAC_FORCE_MODE_CTRL]*/
        reg_idx = MAPLE_MAC_FORCE_MODE_CTRLr;

        temp = 0x1;     /*Always set MAC FORCE Flow Control*/
        ret = reg_array_field_write(unit, reg_idx, port_id, REG_ARRAY_INDEX_NONE, MAPLE_MAC_FORCE_FC_ENf, &temp);
        if ((ret != RT_ERR_OK)&&(ret != RT_ERR_CHIP_NOT_SUPPORTED))
            return ret;

        temp = rx_pause;
        ret = reg_array_field_write(unit, reg_idx, port_id, REG_ARRAY_INDEX_NONE, MAPLE_RX_PAUSE_ENf, &temp);
        if ((ret != RT_ERR_OK)&&(ret != RT_ERR_CHIP_NOT_SUPPORTED))
            return ret;

        temp = tx_pause;
        ret = reg_array_field_write(unit, reg_idx, port_id, REG_ARRAY_INDEX_NONE, MAPLE_TX_PAUSE_ENf, &temp);
        if ((ret != RT_ERR_OK)&&(ret != RT_ERR_CHIP_NOT_SUPPORTED))
            return ret;

    }

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_maple_port_comboPort_workaround
 * Description:
 *      Monitor for phy(combo port) flow control on fiber mode.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is Monitor for phy(combo port) flow control on fiber mode.
 */
int32
dal_maple_port_comboPort_workaround(uint32 unit)
{
    int32 ret;

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    PORT_SEM_LOCK(unit);

    if((ret = _dal_maple_port_comboPort_fc_workaround(unit))!= RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);
    return RT_ERR_OK;
}/* end of dal_maple_port_comboPort_workaround */
#endif

/* Function Name:
 *      dal_maple_port_downSpeedEnable_get
 * Description:
 *      Get down speed status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pEnable - down speed status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      None
 */
int32
dal_maple_port_downSpeedEnable_get(uint32 unit, rtk_port_t port,
    rtk_enable_t *pEnable)
{
    int32   ret, retError;
	rtk_enable_t  pollSts;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d,port=%d",unit, port);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    /* function body */
    PORT_SEM_LOCK(unit);

    /* back up MAC polling status*/
    if ((ret = hal_miim_pollingEnable_get(unit, port, &pollSts)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "port %d disable MAC polling fail (0x%x)", port, ret);
        return ret;
    }

	/* disable MAC polling */
    if ((ret = hal_miim_pollingEnable_set(unit, port, DISABLED)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "port %d disable MAC polling fail (0x%x)", port, ret);
        return ret;
    }

    /* get value from CHIP*/
    if ((ret = phy_downSpeedEnable_get(unit, port, pEnable)) != RT_ERR_OK)
    {
        /* Restore MAC polling status*/
        if ((retError = hal_miim_pollingEnable_set(unit, port, pollSts)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(retError, (MOD_DAL|MOD_PORT), "port %d enable MAC polling fail (0x%x)", port, retError);
            return retError;
        }
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "port %d Error Code: 0x%X", port, ret);
        return ret;
    }
	
    /* Restore MAC polling status*/
    if ((ret = hal_miim_pollingEnable_set(unit, port, pollSts)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "port %d enable MAC polling fail (0x%x)", port, ret);
       return ret;
    }
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pEnable=%d", *pEnable);

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /* end of dal_maple_port_downSpeedEnable_get */

/* Function Name:
 *      dal_maple_port_downSpeedEnable_set
 * Description:
 *      Set down speed status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - down speed status
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
dal_maple_port_downSpeedEnable_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable)
{
    int32   ret, retError;
	rtk_enable_t  pollSts;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d,port=%d,enable=%d",unit, port, enable);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);

    /* function body */
    PORT_SEM_LOCK(unit);

    /* back up MAC polling status*/
    if ((ret = hal_miim_pollingEnable_get(unit, port, &pollSts)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "port %d disable MAC polling fail (0x%x)", port, ret);
        return ret;
    }

	/* disable MAC polling */
    if ((ret = hal_miim_pollingEnable_set(unit, port, DISABLED)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "port %d disable MAC polling fail (0x%x)", port, ret);
        return ret;
    }

    /* set value from CHIP*/
    if ((ret = phy_downSpeedEnable_set(unit, port, enable)) != RT_ERR_OK)
    {
        /* Restore MAC polling status*/
       if ((retError = hal_miim_pollingEnable_set(unit, port, pollSts)) != RT_ERR_OK)
       {
           PORT_SEM_UNLOCK(unit);
           RT_ERR(retError, (MOD_DAL|MOD_PORT), "port %d enable MAC polling fail (0x%x)", port, retError);
           return retError;
        }
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "port %d Error Code: 0x%X", port, ret);
        return ret;
    }
	
    /* Restore MAC polling status*/
    if ((ret = hal_miim_pollingEnable_set(unit, port, pollSts)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "port %d enable MAC polling fail (0x%x)", port, ret);
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /* end of dal_maple_port_downSpeedEnable_set */


/* Function Name:
 *      dal_maple_port_fiberDownSpeedEnable_get
 * Description:
 *      Get fiber down speed status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pEnable - fiber down speed status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      None
 */
int32
dal_maple_port_fiberDownSpeedEnable_get(uint32 unit, rtk_port_t port,
    rtk_enable_t *pEnable)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d,port=%d",unit, port);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((!HAL_IS_SERDES_PORT(unit, port)) && (!HAL_IS_GE_COMBO_PORT(unit, port)), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);


    /* function body */
    PORT_SEM_LOCK(unit);

    /* get value from CHIP*/
    if ((ret = phy_fiberDownSpeedEnable_get(unit, port, pEnable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "port %d Error Code: 0x%X", port, ret);
        return ret;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pEnable=%d", *pEnable);

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /* end of dal_maple_port_fiberDownSpeedEnable_get */

/* Function Name:
 *      dal_maple_port_fiberDownSpeedEnable_set
 * Description:
 *      Set fiber down speed status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - fiber down speed status
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
dal_maple_port_fiberDownSpeedEnable_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d,port=%d,enable=%d",unit, port, enable);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);

    /* function body */
    PORT_SEM_LOCK(unit);

    /* set value from CHIP*/
    if ((ret = phy_fiberDownSpeedEnable_set(unit, port, enable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "port %d Error Code: 0x%X", port, ret);
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /* end of dal_maple_port_fiberDownSpeedEnable_set */


/* Function Name:
 *      dal_maple_port_fiberOAMLoopBack_set
 * Description:
 *      Set OAM Loopback featrue of the specific Fiber-port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - fiber down speed status
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
dal_maple_port_fiberOAMLoopBack_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable)
{
    int32   ret;

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d,port=%d,enable=%d",unit, port, enable);
    RT_PARAM_CHK((!HAL_IS_SERDES_PORT(unit, port)) && (!HAL_IS_GE_COMBO_PORT(unit, port)), RT_ERR_PORT_ID);


    /*Step1: Disable MAC RX or Enable MAC RX */
    if(enable == DISABLED)
    	dal_maple_port_rxEnable_set(unit,  port,  ENABLED);
    else
    	dal_maple_port_rxEnable_set(unit,  port,  DISABLED);

    /*Step2: Disable PHY Digital Loopback or Not */
    PORT_SEM_LOCK(unit);

    /* set value from CHIP*/
    if ((ret = phy_fiberOAMLoopBackEnable_set(unit, port, enable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "port %d Error Code: 0x%X", port, ret);
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /* end of dal_maple_port_OAMLoopBack_set */

/* Function Name:
 *      dal_maple_port_fiberInternalLoopBack_set
 * Description:
 *      Set Internal Loopback featrue of the specific Fiber-port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - fiber down speed status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      SERDES port is not included.
 */
int32
dal_maple_port_fiberInternalLoopBack_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable)
{
    int32   ret;    
	rtk_enable_t  pollSts;

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d,port=%d,enable=%d",unit, port, enable);
    RT_PARAM_CHK((!HAL_IS_GE_COMBO_PORT(unit, port)), RT_ERR_PORT_ID);

    PORT_SEM_LOCK(unit);

    /* back up MAC polling status*/
    if ((ret = hal_miim_pollingEnable_get(unit, port, &pollSts)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "port %d disable MAC polling fail (0x%x)", port, ret);
        return ret;
    }

	/* disable MAC polling */
    if ((ret = hal_miim_pollingEnable_set(unit, port, DISABLED)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "port %d disable MAC polling fail (0x%x)", port, ret);
        return ret;
    }

    /* set value*/
    if ((ret = phy_fiberInternalLoopBackEnable_set(unit, port, enable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        /* enable MAC polling */
        hal_miim_pollingEnable_set(unit, port, pollSts);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "port %d Error Code: 0x%X", port, ret);
        return ret;
    }

    /* Restore MAC polling status*/
    if ((ret = hal_miim_pollingEnable_set(unit, port, pollSts)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "port %d enable MAC polling fail (0x%x)", port, ret);
       return ret;
    }
	
    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /* end of dal_maple_port_fiberInternalLoopBack_set */

/* Function Name:
 *      dal_maple_port_phyFiberTxDis_set
 * Description:
 *      Set PHY fiber Tx disable signal
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      enable  - ENABLED: Enable Tx disable signal;
 *                DISABLED: Disable Tx disable signal.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      None
 */
int32
dal_maple_port_phyFiberTxDis_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d,port=%d,enable=%d", unit, port, enable);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);

    /* function body */
    PORT_SEM_LOCK(unit);

    if ((ret = phy_fiberTxDis_set(unit, port, enable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL | MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /* end of dal_maple_port_phyFiberTxDis_set */

/* Function Name:
 *      dal_maple_port_phyFiberTxDisPin_set
 * Description:
 *      Set PHY fiber Tx disable signal GPO output
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      data      - GPO pin value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      None
 */
int32
dal_maple_port_phyFiberTxDisPin_set(uint32 unit, rtk_port_t port, uint32 data)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d,port=%d,data=%d", unit, port, data);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);

    /* function body */
    PORT_SEM_LOCK(unit);

    if ((ret = phy_fiberTxDisPin_set(unit, port, data)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL | MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /* end of dal_maple_port_phyFiberTxDisPin_set */


