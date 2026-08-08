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
 * $Revision: 57334 $
 * $Date: 2015-03-30 14:13:45 +0800 (Mon, 30 Mar 2015) $
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
#include <hal/chipdef/cypress/rtk_cypress_table_struct.h>
#include <hal/mac/reg.h>
#include <hal/mac/mem.h>
#include <hal/mac/drv.h>
#include <hal/chipdef/allreg.h>
#include <hal/chipdef/cypress/rtk_cypress_reg_struct.h>
#include <hal/common/miim.h>
#include <hal/common/halctrl.h>
#include <dal/dal_common.h>
#include <dal/cypress/dal_cypress_port.h>
#include <dal/cypress/dal_cypress_vlan.h>
#include <rtk/port.h>
#include <rtk/default.h>
#include <drv/intr/intr.h>
#include <hal/phy/phy_8390.h>
#include <ioal/mem32.h>

/*
 * Symbol Definition
 */
typedef struct dal_cypress_mac_info_s {
    uint8   admin_enable[RTK_MAX_NUM_OF_PORTS];
    uint8   green_enable[RTK_MAX_NUM_OF_PORTS];
} dal_cypress_mac_info_t;

typedef struct dal_cypress_phy_info_s {
    uint8   force_mode_speed[RTK_MAX_NUM_OF_PORTS];
    uint8   force_fiber_mode_speed[RTK_MAX_NUM_OF_PORTS];
    uint8   force_mode_duplex[RTK_MAX_NUM_OF_PORTS];
    uint8   force_mode_flowControl[RTK_MAX_NUM_OF_PORTS];
    uint8   auto_mode_pause[RTK_MAX_NUM_OF_PORTS];
    uint8   auto_mode_asy_pause[RTK_MAX_NUM_OF_PORTS];
    uint8   cross_over_mode[RTK_MAX_NUM_OF_PORTS];
} dal_cypress_phy_info_t;


/*
 * Data Declaration
 */
static uint32               port_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         port_sem[RTK_MAX_NUM_OF_UNIT];

static dal_cypress_mac_info_t   *pMac_info[RTK_MAX_NUM_OF_UNIT];
static dal_cypress_phy_info_t   *pPhy_info[RTK_MAX_NUM_OF_UNIT];
static dal_link_change_callback_f   link_change_callback_f[RTK_MAX_NUM_OF_UNIT];

extern uint32 phy_watchdog_cnt;
extern uint32 macSerdes_watchdog_cnt;
extern uint32 fiber_rx_watchdog_cnt;

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
static int32 _dal_cypress_port_init_config(uint32 unit);

#ifdef CONFIG_SDK_DRIVER_NIC_USER_MODE
static void _dal_cypress_port_linkChange_isr(uint32 unit, void *isr_param);
#endif /* CONFIG_SDK_DRIVER_NIC_USER_MODE */

extern int32 dal_waMon_phyReconfig_portMaskSet(uint32 unit, rtk_port_t port);

/* Module Name    : port     */
/* Sub-module Name: Global */

/* Function Name:
 *      dal_cypress_port_init
 * Description:
 *      Initialize port module of the specified device.
 * Input:
 *      unit          - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      1. Module must be initialized before using all of APIs in this module
 */
int32
dal_cypress_port_init(uint32 unit)
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

    pMac_info[unit] = (dal_cypress_mac_info_t *)osal_alloc(sizeof(dal_cypress_mac_info_t));
    if (NULL == pMac_info[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PORT), "memory allocate failed");
        return RT_ERR_FAILED;
    }
    osal_memset(pMac_info[unit], 0, sizeof(dal_cypress_mac_info_t));

    pPhy_info[unit] = (dal_cypress_phy_info_t *)osal_alloc(sizeof(dal_cypress_phy_info_t));
    if (NULL == pPhy_info[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PORT), "memory allocate failed");
        osal_free(pMac_info[unit]);
        pMac_info[unit] = NULL;
        return RT_ERR_FAILED;
    }

    osal_memset(pPhy_info[unit], 0, sizeof(dal_cypress_phy_info_t));

    /* init callback function for link change */
    link_change_callback_f[unit] = 0;

    /* set init flag to complete init */
    port_init[unit] = INIT_COMPLETED;

    if (( ret = _dal_cypress_port_init_config(unit)) != RT_ERR_OK)
    {
        port_init[unit] = INIT_NOT_COMPLETED;
        osal_free(pMac_info[unit]);
        pMac_info[unit] = NULL;
        osal_free(pPhy_info[unit]);
        pPhy_info[unit] = NULL;
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "Port default configuration init failed");
        return ret;
    }

#ifdef CONFIG_SDK_DRIVER_NIC_USER_MODE
    /* linkscan callback handler */
    {
        /* enable interrupt */
        if (( ret = drv_intr_enable_set(unit, LINK_CHANGE_INTR)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "LinkScan interrupt enable failed");
            return ret;
        }

        /* register callback */
        if (( ret = drv_intr_link_stat_register(unit, _dal_cypress_port_linkChange_isr)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "LinkScan interrupt handler installed failed");
            return ret;
        }
    }
#endif

    return RT_ERR_OK;
}/* end of dal_cypress_port_init */

/* Function Name:
 *      dal_cypress_port_link_get
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
 *      RT_ERR_PORT_ID         - invalid port id
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 * Note:
 *      The link status of the port is as following:
 *      - LINKDOWN
 *      - LINKUP
 */
int32
dal_cypress_port_link_get(uint32 unit, rtk_port_t port, rtk_port_linkStatus_t *pStatus)
{
    int32   ret;
    uint32  val;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", unit, port);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pStatus), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);

    PORT_SEM_LOCK(unit);

    /* get value from CHIP*/
    /* read twice for latency value */
    reg_array_field_read(unit, CYPRESS_MAC_LINK_STSr, port, REG_ARRAY_INDEX_NONE, CYPRESS_LINK_STSf, &val);
    if ((ret = reg_array_field_read(unit,
                          CYPRESS_MAC_LINK_STSr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          CYPRESS_LINK_STSf,
                          &val)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    /* translate chip's value to definition */
    if (TRUE == val)
    {
        *pStatus = PORT_LINKUP;
    }
    else
    {
        *pStatus = PORT_LINKDOWN;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pStatus=%d", *pStatus);

    return RT_ERR_OK;
}/* end of dal_cypress_port_link_get */

/* Function Name:
 *      dal_cypress_port_txEnable_set
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
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      The TX enable status of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
int32
dal_cypress_port_txEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  val;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, enable=%d", unit, port, enable);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable != DISABLED && enable != ENABLED), RT_ERR_INPUT);

    if (ENABLED == enable)
    {
        val = 1;
    }
    else
    {
        val = 0;
    }

    PORT_SEM_LOCK(unit);

    /* programming value on CHIP*/
    if ((ret = reg_array_field_write(unit,
                          CYPRESS_MAC_PORT_CTRLr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          CYPRESS_TX_ENf,
                          &val)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}/* end of dal_cypress_port_txEnable_set */

/* Function Name:
 *      dal_cypress_port_rxEnable_set
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
 *      RT_ERR_PORT_ID   - invalid port id
 *      RT_ERR_INPUT     - invalid input parameter
 * Note:
 *      The RX enable status of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
int32
dal_cypress_port_rxEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  val;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, enable=%d", unit, port, enable);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable != DISABLED && enable != ENABLED), RT_ERR_INPUT);

    if (ENABLED == enable)
    {
        val = 1;
    }
    else
    {
        val = 0;
    }

    PORT_SEM_LOCK(unit);

    /* programming value on CHIP*/
    if ((ret = reg_array_field_write(unit,
                          CYPRESS_MAC_PORT_CTRLr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          CYPRESS_RX_ENf,
                          &val)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}/* end of dal_cypress_port_rxEnable_set */


/* Function Name:
 *      dal_cypress_port_txEnable_get
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
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The TX enable status of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
int32
dal_cypress_port_txEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
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
                      CYPRESS_MAC_PORT_CTRLr,
                      port,
                      REG_ARRAY_INDEX_NONE,
                      CYPRESS_TX_ENf,
                      &value)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    if (1 == value)
        *pEnable = ENABLED;
    else
        *pEnable = DISABLED;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pEnable=%d", *pEnable);

    return RT_ERR_OK;
}/* end of dal_cypress_port_txEnable_get */

/* Function Name:
 *      dal_cypress_port_rxEnable_get
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
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The RX enable status of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
int32
dal_cypress_port_rxEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
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
                      CYPRESS_MAC_PORT_CTRLr,
                      port,
                      REG_ARRAY_INDEX_NONE,
                      CYPRESS_RX_ENf,
                      &value)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    if (1 == value)
        *pEnable = ENABLED;
    else
        *pEnable = DISABLED;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pEnable=%d", *pEnable);

    return RT_ERR_OK;
}/* end of dal_cypress_port_rxEnable_get */

/* Function Name:
 *      dal_cypress_port_specialCongest_set
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
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - invalid input parameter
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
int32
dal_cypress_port_specialCongest_set(uint32 unit, rtk_port_t port, uint32 second)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, second=%d", unit, port, second);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((second > RTK_PORT_SPEC_CONGEST_TIME_MAX), RT_ERR_OUT_OF_RANGE);

    PORT_SEM_LOCK(unit);

    /* set value from CHIP*/
    if ((ret = reg_array_field_write(unit, CYPRESS_SC_P_ENr, port, REG_ARRAY_INDEX_NONE, CYPRESS_CNGST_SUST_TMR_LMTf, &second)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_cypress_port_specialCongest_set */

/* Function Name:
 *      dal_cypress_port_speedDuplex_get
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
dal_cypress_port_speedDuplex_get(
    uint32            unit,
    rtk_port_t        port,
    rtk_port_speed_t  *pSpeed,
    rtk_port_duplex_t *pDuplex)
{
    int32   ret;
    uint32  speed;
    uint32  duplex;
    rtk_port_linkStatus_t  link_status;
    uint32  sts_500M;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", unit, port);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pSpeed), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pDuplex), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);

    /* Check Link status */
    if ((ret = dal_cypress_port_link_get(unit, port, &link_status)) != RT_ERR_OK)
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
    if ((ret = reg_array_field_read(unit,
                          CYPRESS_MAC_LINK_SPD_STSr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          CYPRESS_SPD_STSf,
                          &speed)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    /* get duplex value from CHIP*/
    if ((ret = reg_array_field_read(unit,
                          CYPRESS_MAC_LINK_DUP_STSr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          CYPRESS_DUP_STSf,
                          &duplex)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    if ((ret = reg_array_field_read(unit,
                          CYPRESS_MAC_LINK_500M_STSr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          CYPRESS_LINK_500M_STSf,
                          &sts_500M)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    /* translate chip's value to definition */
    if (1 == duplex)
    {
        *pDuplex = PORT_FULL_DUPLEX;
    }
    else
    {
        *pDuplex = PORT_HALF_DUPLEX;
    }

    if (1 == sts_500M)
    {
        *pSpeed = PORT_SPEED_500M;
    }
    else
    {
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
                *pSpeed = PORT_SPEED_10G;
                break;
            default:
                return RT_ERR_FAILED;
        }
    }
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pSpeed=%d, pDuplex=%d",
           *pSpeed, *pDuplex);

    return RT_ERR_OK;
}/* end of dal_cypress_port_speedDuplex_get */

/* Function Name:
 *      dal_cypress_port_flowctrl_get
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
 *      RT_ERR_PORT_ID         - invalid port id
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 *      RT_ERR_PORT_LINKDOWN   - link down port status
 * Note:
 *      None
 */
int32
dal_cypress_port_flowctrl_get(
    uint32            unit,
    rtk_port_t        port,
    uint32            *pTxStatus,
    uint32            *pRxStatus)
{
    int32   ret;
    uint32  rxPause, txPause;
    rtk_port_linkStatus_t  link_status;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", unit, port);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pTxStatus), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pRxStatus), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);

    /* Check Link status */
    if ((ret = dal_cypress_port_link_get(unit, port, &link_status)) != RT_ERR_OK)
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

    /* get tx pause value from CHIP*/
    if ((ret = reg_array_field_read(unit,
                          CYPRESS_MAC_TX_PAUSE_STSr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          CYPRESS_TX_PAUSE_STSf,
                          &txPause)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    /* get rx pause value from CHIP*/
    if ((ret = reg_array_field_read(unit,
                          CYPRESS_MAC_RX_PAUSE_STSr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          CYPRESS_RX_PAUSE_STSf,
                          &rxPause)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    /* translate chip's value to definition */
    if(1 == txPause)
    {
        *pTxStatus = ENABLED;
    }
    else
    {
        *pTxStatus = DISABLED;
    }

    if(1 == rxPause)
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
}/* end of dal_cypress_port_flowctrl_get */

/* Function Name:
 *      dal_cypress_port_phyAutoNegoEnable_get
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
 *      RT_ERR_PORT_ID        - invalid port id
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_cypress_port_phyAutoNegoEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
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
}/* end of dal_cypress_port_phyAutoNegoEnable_get */

/* Function Name:
 *      dal_cypress_port_phyAutoNegoEnablePortmask_set
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
 *      RT_ERR_PORT_ID        - invalid port id
 *      RT_ERR_INPUT          - input parameter out of range
 * Note:
 *      1. ENABLED : switch to PHY auto negotiation mode
 *         DISABLED: switch to PHY force mode
 *      2. Once the abilities of both auto-nego and force mode are set,
 *         you can freely swtich the mode without calling ability setting API again
 */
int32
dal_cypress_port_phyAutoNegoEnablePortmask_set(uint32 unit, rtk_portmask_t portMask, rtk_enable_t enable)
{
    int32   ret,port;
    rtk_port_phy_ability_t ability;
    rtk_port_media_t port_mode_sts;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, enable=%d",unit, enable);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    for(port=0;port<RTK_MAX_NUM_OF_PORTS;port++){
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

        if (HAL_IS_SERDES_10GE_PORT(unit, port))
        {
            uint32  speed;

            if ((ret = phy_8390_speed_get(unit, port, &speed)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }

            if (PORT_SPEED_10G == speed)
                continue;
        }

        if ((ret = phy_autoNegoEnable_set(unit, port, enable)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

    osal_time_usleep(100000);

    if (ENABLED == enable)
    {
        for(port=0;port<RTK_MAX_NUM_OF_PORTS;port++)
        {
            if(!RTK_PORTMASK_IS_PORT_SET(portMask,port))
                continue;

            if ((ret = phy_autoNegoAbility_get(unit, port, &ability)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }

            ability.FC = pPhy_info[unit]->auto_mode_pause[port];
            ability.AsyFC = pPhy_info[unit]->auto_mode_asy_pause[port];

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

            if ((ret = phy_duplex_set(unit, port, pPhy_info[unit]->force_mode_duplex[port])) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }

            if ((ret = phy_media_get(unit, port, &port_mode_sts)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL | MOD_PORT), "");
                return ret;
            }

            if((port_mode_sts == PORT_MEDIA_FIBER) || (port_mode_sts == PORT_MEDIA_FIBER_AUTO))
            {
                if (!HAL_IS_SERDES_10GE_PORT(unit, port))
                {
                    if ((ret = phy_speed_set(unit, port, pPhy_info[unit]->force_fiber_mode_speed[port])) != RT_ERR_OK)
                    {
                        PORT_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                        return ret;
                    }
                }
            }

            if((port_mode_sts == PORT_MEDIA_COPPER) || (port_mode_sts == PORT_MEDIA_COPPER_AUTO))
            {
                if ((ret = phy_speed_set(unit, port, pPhy_info[unit]->force_mode_speed[port])) != RT_ERR_OK)
                {
                    PORT_SEM_UNLOCK(unit);
                    RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                    return ret;
                }
            }

            if ((ret = phy_autoNegoAbility_get(unit, port, &ability)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }

            ability.FC = pPhy_info[unit]->force_mode_flowControl[port]; /* ENABLED */
            ability.AsyFC = pPhy_info[unit]->force_mode_flowControl[port]; /* ENABLED */

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
}/* end of dal_cypress_port_phyAutoNegoEnablePortmask_set */

/* Function Name:
 *      dal_cypress_port_phyAutoNegoEnable_set
 * Description:
 *      Set PHY ability of the specific port(s)
 * Input:
 *      unit                 - unit id
 *      port                 - port id
 *      enable               - enable PHY auto negotiation
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID        - invalid port id
 *      RT_ERR_INPUT          - input parameter out of range
 * Note:
 *      1. ENABLED : switch to PHY auto negotiation mode
 *         DISABLED: switch to PHY force mode
 *      2. Once the abilities of both auto-nego and force mode are set,
 *         you can freely swtich the mode without calling ability setting API again
 */
int32
dal_cypress_port_phyAutoNegoEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtk_portmask_t myPortMask;

    RTK_PORTMASK_RESET(myPortMask);
    RTK_PORTMASK_PORT_SET(myPortMask,port);

    return dal_cypress_port_phyAutoNegoEnablePortmask_set(unit, myPortMask, enable);

}/* end of dal_cypress_port_phyAutoNegoEnable_set */



/* Function Name:
 *      dal_cypress_port_phyAutoNegoAbility_get
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
 *      RT_ERR_PORT_ID        - invalid port id
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_cypress_port_phyAutoNegoAbility_get(
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

    if ((ret = dal_cypress_port_phyAutoNegoEnable_get(unit, port, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL | MOD_PORT), "");
        return ret;
    }

    osal_memset(pAbility, 0, sizeof(rtk_port_phy_ability_t));

    PORT_SEM_LOCK(unit);

    if ((ret = phy_autoNegoAbility_get(unit, port, pAbility)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL | MOD_PORT), "");
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
}/* end of dal_cypress_port_phyAutoNegoAbility_get */

/* Function Name:
 *      dal_cypress_port_phyAutoNegoAbility_set
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
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. You can set these abilities no matter which mode PHY currently stays on
 */
int32
dal_cypress_port_phyAutoNegoAbility_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_port_phy_ability_t  *pAbility)
{
    int32   ret;
    rtk_enable_t    enable;

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

    if ((ret = dal_cypress_port_phyAutoNegoEnable_get(unit, port, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL | MOD_PORT), "dal_cypress_port_phyAutoNegoEnable_get(unit=%d, port=%d) failed!!",\
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

    if ((ret = phy_autoNegoAbility_set(unit, port, pAbility)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL | MOD_PORT), "");
        return ret;
    }

    if (ENABLED == enable)
    {
        pPhy_info[unit]->auto_mode_pause[port] = pAbility->FC;
        pPhy_info[unit]->auto_mode_asy_pause[port] = pAbility->AsyFC;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}/* end of dal_cypress_port_phyAutoNegoAbility_set */

/* Function Name:
 *      dal_cypress_port_phyForceModeAbility_get
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
 *      RT_ERR_PORT_ID        - invalid port id
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_cypress_port_phyForceModeAbility_get(
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

    if ((ret = dal_cypress_port_phyAutoNegoEnable_get(unit, port, &enable)) != RT_ERR_OK)
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
        if ((ret = phy_speed_get(unit, port, pSpeed)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }

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
}/* end of dal_cypress_port_phyForceModeAbility_get */

/* Function Name:
 *      dal_cypress_port_phyForceModeAbility_set
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
dal_cypress_port_phyForceModeAbility_set(
    uint32              unit,
    rtk_port_t          port,
    rtk_port_speed_t    speed,
    rtk_port_duplex_t   duplex,
    rtk_enable_t        flowControl)
{
    int32   ret;
    rtk_enable_t    enable;
    rtk_port_phy_ability_t ability;
    rtk_port_media_t port_mode_sts;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, speed=%d, duplex=%d \
           flowControl=%d", unit, port, speed, duplex, flowControl);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(speed >= PORT_SPEED_END, RT_ERR_PHY_SPEED);
    RT_PARAM_CHK((HAL_IS_FE_PORT(unit, port)) && speed == PORT_SPEED_1000M, RT_ERR_CHIP_NOT_SUPPORTED);
    RT_PARAM_CHK(duplex >= PORT_DUPLEX_END, RT_ERR_PHY_DUPLEX);
    RT_PARAM_CHK(flowControl >= RTK_ENABLE_END, RT_ERR_INPUT);

    if ((ret = dal_cypress_port_phyAutoNegoEnable_get(unit, port, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL | MOD_PORT), "");
        return ret;
    }

    PORT_SEM_LOCK(unit);

    if ((ret = phy_media_get(unit, port, &port_mode_sts)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL | MOD_PORT), "");
        return ret;
    }

    if((port_mode_sts == PORT_MEDIA_FIBER) || (port_mode_sts == PORT_MEDIA_FIBER_AUTO))
        pPhy_info[unit]->force_fiber_mode_speed[port] = speed;
    if((port_mode_sts == PORT_MEDIA_COPPER) || (port_mode_sts == PORT_MEDIA_COPPER_AUTO))
        pPhy_info[unit]->force_mode_speed[port] = speed;
    pPhy_info[unit]->force_mode_duplex[port] = duplex;
    pPhy_info[unit]->force_mode_flowControl[port] = flowControl;
    if (DISABLED == enable)
    {
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
}/* end of dal_cypress_port_phyForceModeAbility_set */

/* Function Name:
 *      dal_cypress_port_phyMasterSlave_get
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
 *      This function only works on giga/ 10g port to get its master/slave mode configuration.
 */
int32
dal_cypress_port_phyMasterSlave_get(
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
}/* end of dal_cypress_port_phyMasterSlave_get */

/* Function Name:
 *      dal_cypress_port_phyMasterSlave_set
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
dal_cypress_port_phyMasterSlave_set(
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
}/* end of dal_cypress_port_phyMasterSlave_set */

/* Function Name:
 *      dal_cypress_port_gigaLiteEnable_get
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
dal_cypress_port_gigaLiteEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
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
 *      dal_cypress_port_gigaLiteEnable_set
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
dal_cypress_port_gigaLiteEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  poll_500M;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, enable=%d",
           unit, port, enable);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable != DISABLED && enable != ENABLED), RT_ERR_INPUT);

    PORT_SEM_LOCK(unit);
    if ((ret = phy_gigaLiteEnable_set(unit, port, enable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    poll_500M = (uint32)enable;
    if ((ret = reg_array_field_write(unit,
                          CYPRESS_SMI_PORT_500M_POLLING_CTRLr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          CYPRESS_SMI_POLLING_500M_PMSKf,
                          &poll_500M)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_ssw_port_gigaLiteEnable_set */

/* Function Name:
 *      dal_cypress_port_phyExtParkPageReg_get
 * Description:
 *      Get PHY register data of the specific port with extension page and parking page parameters
 * Input:
 *      unit                - unit id
 *      port                - port id
 *      mainPage            - main page id
 *      extPage             - extension page id
 *      parkPage            - parking page id
 *      reg                 - reg id
 * Output:
 *      pData              - pointer to the PHY reg data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_PHY_PAGE_ID   - invalid page id
 *      RT_ERR_PHY_REG_ID    - invalid reg id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_cypress_port_phyExtParkPageReg_get(
    uint32              unit,
    rtk_port_t          port,
    uint32              mainPage,
    uint32              extPage,
    uint32              parkPage,
    rtk_port_phy_reg_t  reg,
    uint32              *pData)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, mainPage=0x%x, extPage=0x%x, parkPage=0x%x, reg=0x%x",
           unit, port, mainPage, extPage, parkPage, reg);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);

    /* get phy register */
    if ((ret = hal_miim_extParkPage_read(unit, port, mainPage, extPage, parkPage, reg, pData)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    PORT_SEM_UNLOCK(unit);
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pData=0x%x", *pData);

    return ret;
}    /* end of dal_cypress_port_phyExtParkPageReg_get */

/* Function Name:
 *      dal_cypress_port_phyExtParkPageReg_set
 * Description:
 *      Set PHY register data of the specific port with extension page and parking page parameters
 * Input:
 *      unit               - unit id
 *      port               - port id
 *      mainPage           - main page id
 *      extPage            - extension page id
 *      parkPage           - parking page id
 *      reg                - reg id
 *      data               - reg data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid page id
 *      RT_ERR_PHY_REG_ID   - invalid reg id
 * Note:
 *      None
 */
int32
dal_cypress_port_phyExtParkPageReg_set(
    uint32              unit,
    rtk_port_t          port,
    uint32              mainPage,
    uint32              extPage,
    uint32              parkPage,
    rtk_port_phy_reg_t  reg,
    uint32              data)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, mainPage=0x%x, extPage=0x%x, parkPage=0x%x, reg=0x%x \
           data=0x%x", unit, port, mainPage, extPage, parkPage, reg, data);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);

    PORT_SEM_LOCK(unit);

    if ((ret = hal_miim_extParkPage_write(unit, port, mainPage, extPage, parkPage, reg, data)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    PORT_SEM_UNLOCK(unit);


    return ret;
}    /* end of dal_cypress_port_phyExtParkPageReg_set */

/* Function Name:
 *      dal_cypress_port_phymaskExtParkPageReg_set
 * Description:
 *      Set PHY register data of the specific portmask with extension page and parking page parameters
 * Input:
 *      unit               - unit id
 *      pPortmask          - pointer to the portmask
 *      mainPage           - main page id
 *      extPage            - extension page id
 *      parkPage           - parking page id
 *      reg                - reg id
 *      data               - reg data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid page id
 *      RT_ERR_PHY_REG_ID   - invalid reg id
 * Note:
 *      None
 */
int32
dal_cypress_port_phymaskExtParkPageReg_set(
    uint32              unit,
    rtk_portmask_t      *pPortmask,
    uint32              mainPage,
    uint32              extPage,
    uint32              parkPage,
    rtk_port_phy_reg_t  reg,
    uint32              data)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, portmask=0x%8x 0x%8x, mainPage=0x%x, extPage=0x%x, parkPage=0x%x, reg=0x%x \
           data=0x%x", unit, pPortmask->bits[1], pPortmask->bits[0], mainPage, extPage, parkPage, reg, data);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pPortmask), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);

    if ((ret = hal_miim_extParkPage_portmask_write(unit, *pPortmask, mainPage, extPage, parkPage, reg, data)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return ret;
}    /* end of dal_cypress_port_phymaskExtParkPageReg_set */

/* Function Name:
 *      dal_cypress_port_phyMmdReg_get
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
dal_cypress_port_phyMmdReg_get(
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
}    /* end of dal_cypress_port_phyMmdReg_get */

/* Function Name:
 *      dal_cypress_port_phyMmdReg_set
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
dal_cypress_port_phyMmdReg_set(
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
}    /* end of dal_cypress_port_phyMmdReg_set */

/* Function Name:
 *      dal_cypress_port_phymaskMmdReg_set
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
dal_cypress_port_phymaskMmdReg_set(
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
}    /* end of dal_cypress_port_phymaskMmdReg_set */

/* Function Name:
 *      dal_cypress_port_phyReg_get
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
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_PHY_PAGE_ID   - invalid page id
 *      RT_ERR_PHY_REG_ID    - invalid reg id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_cypress_port_phyReg_get(
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
}/* end of dal_cypress_port_phyReg_get */

/* Function Name:
 *      dal_cypress_port_phyReg_set
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
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid page id
 *      RT_ERR_PHY_REG_ID   - invalid reg id
 * Note:
 *      None
 */
int32
dal_cypress_port_phyReg_set(
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

}/* end of dal_cypress_port_phyReg_set */

/* Function Name:
 *      dal_cypress_port_phyReg_broadcast_set
 * Description:
 *      Set PHY register data with broadcast mechanism
 * Input:
 *      unit - unit id
 *      page - page id
 *      reg  - reg id
 *      data - reg data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PHY_PAGE_ID - invalid page id
 *      RT_ERR_PHY_REG_ID  - invalid reg id
 * Note:
 *      None
 */
int32
dal_cypress_port_phyReg_broadcast_set(
    uint32              unit,
    uint32              page,
    rtk_port_phy_reg_t  reg,
    uint32              data)
{
    int32   ret;
    rtk_port_t  port, max_port;
    hal_control_t   *pHalCtrl;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, page=%d, reg=%d, data=%d", unit, page, reg, data);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    PORT_SEM_LOCK(unit);

    max_port = HAL_GET_MAX_PORT(unit);

    for (port = 0; port < max_port; port = port + 8)
    {
        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }

        if (HAL_IS_PHY_EXIST(unit, port) && (PHY_MODEL_ID_RTL8218 == pHalCtrl->pPhy_ctrl[port]->phy_model_id))
        {
            if((ret = phy_broadcastEnable_set(unit, port, ENABLED)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                return ret;
            }
        }
    }

    if((ret = hal_miim_broadcast_write(unit, page, reg, data)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        return ret;
    }

    for (port = 0; port < max_port; port = port + 8)
    {
        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }

        if (HAL_IS_PHY_EXIST(unit, port) && (PHY_MODEL_ID_RTL8218 == pHalCtrl->pPhy_ctrl[port]->phy_model_id))
        {
            if((ret = phy_broadcastEnable_set(unit, port, DISABLED)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                return ret;
            }
        }
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /* end of rtk_port_phyReg_broadcast_set */

/* Function Name:
 *      dal_cypress_port_phyReg_broadcast_set
 * Description:
 *      Set PHY broadcast ID
 * Input:
 *      unit - unit id
 *      broadcastID - broadcast id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
dal_cypress_port_phyReg_broadcastID_set(
    uint32              unit,
    uint32              broadcastID)
{
    int32   ret;
    rtk_port_t  port, max_port;
    hal_control_t   *pHalCtrl;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, broadcastID=%d", unit, broadcastID);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    PORT_SEM_LOCK(unit);

    max_port = HAL_GET_MAX_PORT(unit);
    for (port = 0; port < max_port; port = port + 8)
    {
        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }

        if (HAL_IS_PHY_EXIST(unit, port) && (PHY_MODEL_ID_RTL8218 == pHalCtrl->pPhy_ctrl[port]->phy_model_id))
        {
            if((ret = phy_broadcastID_set(unit, port, broadcastID)) != RT_ERR_OK)
            {
                PORT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
                return ret;
            }
        }
    }

    if ((ret = reg_field_write(unit,
                          CYPRESS_BROADCAST_PHYID_CTRLr,
                          CYPRESS_BROADCAST_PHYIDf,
                          &broadcastID)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);
    return RT_ERR_OK;
}   /* end of rtk_port_phyReg_broadcastID_set */


/* Function Name:
 *      dal_cypress_port_cpuPortId_get
 * Description:
 *      Get CPU port id of the specific unit
 * Input:
 *      unit                 - unit id
 * Output:
 *      pPort               - pointer to CPU port id
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_cypress_port_cpuPortId_get(uint32 unit, rtk_port_t *pPort)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pPort), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);

    *pPort = HAL_GET_CPU_PORT(unit);

    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pPort=%d", *pPort);

    return RT_ERR_OK;
}/* end of dal_cypress_port_cpuPortId_get */

/* Function Name:
 *      dal_cypress_port_isolation_get
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
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      1. Default value of each port is 1
 *      2. Enable port isolation in the certain ports if relative portmask bits are set to 1
 */
int32
dal_cypress_port_isolation_get(uint32 unit, rtk_port_t port, rtk_portmask_t *pPortmask)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d", unit, port);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pPortmask), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);

    PORT_SEM_LOCK(unit);

    /* get value from CHIP */
    if ((ret = reg_array_field_read(unit, CYPRESS_PORT_ISO_CTRLr, port, REG_ARRAY_INDEX_NONE, CYPRESS_P_ISO_MBR_0f
                        , &pPortmask->bits[0])) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    if ((ret = reg_array_field_read(unit, CYPRESS_PORT_ISO_CTRLr, port, REG_ARRAY_INDEX_NONE, CYPRESS_P_ISO_MBR_1f
                        , &pPortmask->bits[1])) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pPortmask=0x%x", pPortmask->bits[0]);

    return RT_ERR_OK;
}/* end of dal_cypress_port_isolation_get */

/* Function Name:
 *      dal_cypress_port_isolation_set
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
 *      RT_ERR_PORT_ID   - invalid port id
 *      RT_ERR_PORT_MASK - invalid port mask
 * Note:
 *      1. Default value of each port is 1
 *      2. Enable port isolation in the certain ports if relative portmask bits are set to 1
 */
int32
dal_cypress_port_isolation_set(uint32 unit, rtk_port_t port, rtk_portmask_t portmask)
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
    if ((ret = reg_array_field_write(unit, CYPRESS_PORT_ISO_CTRLr, port, REG_ARRAY_INDEX_NONE, CYPRESS_P_ISO_MBR_0f
                        , &portmask.bits[0])) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    if ((ret = reg_array_field_write(unit, CYPRESS_PORT_ISO_CTRLr, port, REG_ARRAY_INDEX_NONE, CYPRESS_P_ISO_MBR_1f
                        , &portmask.bits[1])) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}/* end of dal_cypress_port_isolation_set */

/* Function Name:
 *      dal_cypress_port_isolation_add
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
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      1. Default value of each port is 1
 *      2. Port and iso_port will be isolated when this API is called
 *      3. The iso_port to the relative portmask bit will be set to 1
 *      4. This API can not use sem lock since it takes use of dal layer API
 */
int32
dal_cypress_port_isolation_add(uint32 unit, rtk_port_t port, rtk_port_t iso_port)
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

    ret = dal_cypress_port_isolation_get(unit, port, &portmask);
    if (RT_ERR_OK != ret)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    RTK_PORTMASK_PORT_SET(portmask, iso_port);

    ret = dal_cypress_port_isolation_set(unit, port, portmask);
    if (RT_ERR_OK != ret)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    return RT_ERR_OK;
}/* end of dal_cypress_port_isolation_add */

/* Function Name:
 *      dal_cypress_port_isolation_del
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
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      1. Default value of each port is 1
 *      2. Isolated status between the port and the iso_port is removed when this API is called
 *      3. The iso_port to the relative portmask bit will be set to 0
 *      4. This API can not use sem lock since it takes use of dal layer API
 */
int32
dal_cypress_port_isolation_del(uint32 unit, rtk_port_t port, rtk_port_t iso_port)
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

    ret = dal_cypress_port_isolation_get(unit, port, &portmask);
    if (RT_ERR_OK != ret)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    RTK_PORTMASK_PORT_CLEAR(portmask, iso_port);

    ret = dal_cypress_port_isolation_set(unit, port, portmask);
    if (RT_ERR_OK != ret)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    return RT_ERR_OK;
}/* end of dal_cypress_port_isolation_del */

/* Function Name:
 *      dal_cypress_port_phyComboPortMedia_get
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
 *      RT_ERR_PORT_ID        - invalid port id
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_cypress_port_phyComboPortMedia_get(uint32 unit, rtk_port_t port, rtk_port_media_t *pMedia)
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
} /* end of dal_cypress_port_phyComboPortMedia_get */

/* Function Name:
 *      dal_cypress_port_phyComboPortMedia_set
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
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. You can set these port media which mode PHY currently stays on
 */
int32
dal_cypress_port_phyComboPortMedia_set(uint32 unit, rtk_port_t port, rtk_port_media_t media)
{
    uint32  pollSts;
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, media=%d",
           unit, port, media);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((media >= PORT_MEDIA_END), RT_ERR_INPUT);

    PORT_SEM_LOCK(unit);

    /* disable MAC polling */
    pollSts = 0;
    if ((ret = reg_array_field_write(unit,
            CYPRESS_SMI_PORT_POLLING_CTRLr, port, REG_ARRAY_INDEX_NONE,
            CYPRESS_SMI_POLLING_PMSKf, &pollSts)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "port %d disable MAC polling fail (0x%x)", port, ret);
        return ret;
    }

    pollSts = 1;
    if ((ret = phy_media_set(unit, port, media)) != RT_ERR_OK)
    {
        /* enable MAC polling */
        reg_array_field_write(unit, CYPRESS_SMI_PORT_POLLING_CTRLr, port,
                REG_ARRAY_INDEX_NONE, CYPRESS_SMI_POLLING_PMSKf, &pollSts);

        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    /* enable MAC polling */
    if ((ret = reg_array_field_write(unit,
            CYPRESS_SMI_PORT_POLLING_CTRLr, port, REG_ARRAY_INDEX_NONE,
            CYPRESS_SMI_POLLING_PMSKf, &pollSts)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "port %d enable MAC polling fail (0x%x)", port, ret);
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_cypress_port_phyComboPortMedia_set */

/* Function Name:
 *      dal_cypress_port_adminEnable_get
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
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_cypress_port_adminEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
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
} /* end of dal_cypress_port_adminEnable_get */

/* Function Name:
 *      dal_cypress_port_adminEnable_set
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
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_cypress_port_adminEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
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
        value = 0x1;
        /* programming value on CHIP*/
        if ((ret = reg_array_field_write(unit,
                          CYPRESS_MAC_PORT_CTRLr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          CYPRESS_TX_ENf,
                          &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "port %d", port);
            return ret;
        }
        if ((ret = reg_array_field_write(unit,
                          CYPRESS_MAC_PORT_CTRLr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          CYPRESS_RX_ENf,
                          &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "port %d", port);
            return ret;
        }
    }

  #if !defined (__MODEL_USER__)
    if (HAL_IS_PHY_EXIST(unit, port))
    {
        if ((ret = phy_enable_set(unit, port, enable)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "port %d", port);
            return ret;
        }
    }
  #endif

    if (DISABLED == enable)
    {
        /* programming value on CHIP*/
        value = 0x0;
        if ((ret = reg_array_field_write(unit,
                          CYPRESS_MAC_PORT_CTRLr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          CYPRESS_TX_ENf,
                          &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "port %d", port);
            return ret;
        }
        if ((ret = reg_array_field_write(unit,
                          CYPRESS_MAC_PORT_CTRLr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          CYPRESS_RX_ENf,
                          &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "port %d", port);
            return ret;
        }
    }

    pMac_info[unit]->admin_enable[port] = enable;
    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_cypress_port_adminEnable_set */

/* Function Name:
 *      dal_cypress_port_macRemoteLoopbackEnable_get
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
dal_cypress_port_macRemoteLoopbackEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
#if 0
    int32 ret;
    uint32 value, serdesIdx = 0;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d",
           unit, port);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);

    /* get value from CHIP*/
    if(HAL_IS_10GE_PORT(unit, port))
    {
        if(24 == port)
            serdesIdx = 0;
        else if (36 == port)
            serdesIdx = 2;

        if ((ret = reg_array_field_read(unit,
                          CYPRESS_LPB_10GBASE_R_CTRLr,
                          REG_ARRAY_INDEX_NONE,
                          serdesIdx,
                          CYPRESS_XGMII_REMO_LPB_ENf,
                          &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }
    else
    {
        serdesIdx = port/4;
        if ((ret = reg_array_field_read(unit,
                          CYPRESS_LPB_5G_CTRLr,
                          REG_ARRAY_INDEX_NONE,
                          serdesIdx,
                          CYPRESS_DIGT_REMO_LPB_ENf,
                          &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

    *pEnable = value;

    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pEnable=%d", *pEnable);
#endif
    return RT_ERR_OK;
} /* end of dal_cypress_port_macRemoteLoopbackEnable_get */

/* Function Name:
 *      dal_cypress_port_macRemoteLoopbackEnable_set
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
dal_cypress_port_macRemoteLoopbackEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
#if 0
    int32 ret;
    uint32 value, serdesIdx = 0;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, enable=%d",
           unit, port, enable);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable != DISABLED && enable != ENABLED), RT_ERR_INPUT);

    PORT_SEM_LOCK(unit);

    value = enable;

    /* program value to CHIP*/
    if(HAL_IS_10GE_PORT(unit, port))
    {
        if(24 == port)
            serdesIdx = 0;
        else if (36 == port)
            serdesIdx = 2;

        if ((ret = reg_array_field_write(unit,
                          CYPRESS_LPB_10GBASE_R_CTRLr,
                          REG_ARRAY_INDEX_NONE,
                          serdesIdx,
                          CYPRESS_XGMII_REMO_LPB_ENf,
                          &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
        if ((ret = reg_array_field_write(unit,
                          CYPRESS_LPB_10GBASE_R_CTRLr,
                          REG_ARRAY_INDEX_NONE,
                          serdesIdx + 1,
                          CYPRESS_XGMII_REMO_LPB_ENf,
                          &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }
    else
    {
        serdesIdx = port/4;
        if ((ret = reg_array_field_write(unit,
                          CYPRESS_LPB_5G_CTRLr,
                          REG_ARRAY_INDEX_NONE,
                          serdesIdx,
                          CYPRESS_DIGT_REMO_LPB_ENf,
                          &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

    PORT_SEM_UNLOCK(unit);
#endif
    return RT_ERR_OK;
} /* end of dal_cypress_port_macRemoteLoopbackEnable_set */

/* Function Name:
 *      dal_cypress_port_macLocalLoopbackEnable_get
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
dal_cypress_port_macLocalLoopbackEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
#if 0
    int32 ret;
    uint32 value, serdesIdx = 0;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d",
           unit, port);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);

    /* get value from CHIP*/
    if(HAL_IS_10GE_PORT(unit, port))
    {
        if(24 == port)
            serdesIdx = 0;
        else if (36 == port)
            serdesIdx = 2;

        if ((ret = reg_array_field_read(unit,
                          CYPRESS_LPB_10GBASE_R_CTRLr,
                          REG_ARRAY_INDEX_NONE,
                          serdesIdx,
                          CYPRESS_AFE_LOCAL_LPB_ENf,
                          &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }
    else
    {
        serdesIdx = port/4;
        if ((ret = reg_array_field_read(unit,
                          CYPRESS_LPB_5G_CTRLr,
                          REG_ARRAY_INDEX_NONE,
                          serdesIdx,
                          CYPRESS_DIGT_LOCAL_LPB_ENf,
                          &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

    *pEnable = value;

    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pEnable=%d", *pEnable);
#endif
    return RT_ERR_OK;
} /* end of dal_cypress_port_macLocalLoopbackEnable_get */

/* Function Name:
 *      dal_cypress_port_macLocalLoopbackEnable_set
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
dal_cypress_port_macLocalLoopbackEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
#if 0
    int32 ret;
    uint32 value, serdesIdx = 0;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, enable=%d",
           unit, port, enable);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable != DISABLED && enable != ENABLED), RT_ERR_INPUT);

    PORT_SEM_LOCK(unit);

    /* program value to CHIP*/
    value = enable;
    if(HAL_IS_10GE_PORT(unit, port))
    {
        if(24 == port)
            serdesIdx = 0;
        else if (36 == port)
            serdesIdx = 2;

        if ((ret = reg_array_field_write(unit,
                          CYPRESS_LPB_10GBASE_R_CTRLr,
                          REG_ARRAY_INDEX_NONE,
                          serdesIdx,
                          CYPRESS_AFE_LOCAL_LPB_ENf,
                          &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
        if ((ret = reg_array_field_write(unit,
                          CYPRESS_LPB_10GBASE_R_CTRLr,
                          REG_ARRAY_INDEX_NONE,
                          serdesIdx + 1,
                          CYPRESS_AFE_LOCAL_LPB_ENf,
                          &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }
    else
    {
        serdesIdx = port/4;
        if ((ret = reg_array_field_write(unit,
                          CYPRESS_LPB_5G_CTRLr,
                          REG_ARRAY_INDEX_NONE,
                          serdesIdx,
                          CYPRESS_DIGT_LOCAL_LPB_ENf,
                          &value)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

    PORT_SEM_UNLOCK(unit);
#endif
    return RT_ERR_OK;
} /* end of dal_cypress_port_macLocalLoopbackEnable_set */

/* Function Name:
 *      dal_cypress_port_backpressureEnable_get
 * Description:
 *      Get the half duplex backpressure enable status of the specific port
 * Input:
 *      unit           - unit id
 *      port           - port id
 * Output:
 *      pEnable        - pointer to the enable status of backpressure in half duplex mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
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
dal_cypress_port_backpressureEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
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
                          CYPRESS_MAC_PORT_CTRLr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          CYPRESS_BKPRES_ENf,
                          pEnable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pEnable=%d", *pEnable);

    return RT_ERR_OK;
} /* end of dal_cypress_port_backpressureEnable_get */

/* Function Name:
 *      dal_cypress_port_backpressureEnable_set
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
dal_cypress_port_backpressureEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
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
                          CYPRESS_MAC_PORT_CTRLr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          CYPRESS_BKPRES_ENf,
                          &enable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_cypress_port_backpressureEnable_set */

/* Function Name:
 *      dal_cypress_port_linkChange_register
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
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 */
int32
dal_cypress_port_linkChange_register(uint32 unit, dal_link_change_callback_f link_change_callback)
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
} /* End of dal_cypress_port_linkChange_register */

/* Function Name:
 *      dal_cypress_port_linkChange_unregister
 * Description:
 *      Unregister callback function for notification of link change
 * Input:
 *      unit           - unit id
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 * Note:
 */
int32
dal_cypress_port_linkChange_unregister(uint32 unit)
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
} /* End of dal_cypress_port_linkChange_unregister */

/* Function Name:
 *      _dal_cypress_port_init_config
 * Description:
 *      Initialize default configuration for port module of the specified device.
 * Input:
 *      unit          - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 * Note:
 *      1. Module must be initialized before using all of APIs in this module
 */
static int32
_dal_cypress_port_init_config(uint32 unit)
{
    int32   ret;
    rtk_port_t  port, max_port;
    rtk_portmask_t  portmask;
    rtk_port_phy_ability_t phy_ability;
    hal_control_t   *pHalCtrl;
  #if !defined (__MODEL_USER__)
    rtk_portmask_t  portmask2;
  #endif

    /* Some serdes control register value seting for RTL8218 + RTL8389M/RTL8389L */
    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    phy_ability.Half_10 = RTK_DEFAULT_PORT_10HALF_CAPABLE;
    phy_ability.Full_10 = RTK_DEFAULT_PORT_10FULL_CAPABLE;
    phy_ability.Half_100 = RTK_DEFAULT_PORT_100HALF_CAPABLE;
    phy_ability.Full_100 = RTK_DEFAULT_PORT_100FULL_CAPABLE;
    phy_ability.Half_1000 = RTK_DEFAULT_PORT_1000HALF_CAPABLE;
    phy_ability.Full_1000 = RTK_DEFAULT_PORT_1000FULL_CAPABLE;
    phy_ability.FC = RTK_DEFAULT_PORT_PAUSE_CAPABILITY;
    phy_ability.AsyFC = RTK_DEFAULT_PORT_ASYPAUSE_CAPABILITY;
    max_port = HAL_GET_MAX_PORT(unit);
    HAL_GET_ALL_PORTMASK(unit, portmask);
  #if !defined (__MODEL_USER__)
    RTK_PORTMASK_RESET(portmask2);
  #endif

    ret = 0;

    for (port = 0; port < max_port; port++)
    {

        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }

        /* Config MAC */
        if (!HAL_IS_CPU_PORT(unit, port))
        {
            if ((ret = dal_cypress_port_isolation_set(unit, port, portmask)) != RT_ERR_OK)
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

            pPhy_info[unit]->force_mode_duplex[port] = PORT_FULL_DUPLEX;
            if(HAL_IS_FE_PORT(unit,port))
                pPhy_info[unit]->force_mode_speed[port] = PORT_SPEED_100M;
            else
                pPhy_info[unit]->force_mode_speed[port] = PORT_SPEED_1000M;

            if((HAL_IS_GE_COMBO_PORT(unit,port)) || (HAL_IS_SERDES_PORT(unit,port)))
                pPhy_info[unit]->force_fiber_mode_speed[port] = PORT_SPEED_1000M;
            else
                pPhy_info[unit]->force_fiber_mode_speed[port] = PORT_SPEED_100M;

            if ((ret = dal_cypress_port_adminEnable_set(unit, port, RTK_DEFAULT_PORT_ADMIN_ENABLE)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_PORT), "Port init enable port failed");
                return ret;
            }

          #if !defined (__MODEL_USER__)
            if (!HAL_IS_CPU_PORT(unit, port))
            {
                if ((ret = dal_cypress_port_phyAutoNegoAbility_set(unit, port, &phy_ability)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_PORT), "Port init set autonegotiation ability failed");
                    return ret;
                }

                RTK_PORTMASK_PORT_SET(portmask2,port);
            }
          #endif
        }

    }//for(port)

  #if !defined (__MODEL_USER__)
    if ((ret = dal_cypress_port_phyAutoNegoEnablePortmask_set(unit, portmask2, RTK_DEFAULT_PORT_AUTONEGO_ENABLE)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "Port init enable PHY autonegotiation failed");
        return ret;
    }
  #endif

    return RT_ERR_OK;
}/* end of _dal_cypress_port_init_config */

/* Function Name:
 *      dal_cypress_port_greenEnable_get
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
dal_cypress_port_greenEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
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
} /* end of dal_cypress_port_greenEnable_get */

/* Function Name:
 *      dal_cypress_port_greenEnable_set
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
dal_cypress_port_greenEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
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
} /* end of dal_cypress_port_greenEnable_set */


#ifdef CONFIG_SDK_DRIVER_NIC_USER_MODE
/* Function Name:
 *      _dal_cypress_port_linkChange_isr
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
_dal_cypress_port_linkChange_isr(uint32 unit, void *isr_param)
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
} /* end of _dal_cypress_port_linkChange_isr */
#endif /* CONFIG_SDK_DRIVER_NIC_USER_MODE */

/* Function Name:
 *      dal_cypress_port_vlanBasedIsolationEntry_get
 * Description:
 *      Get VLAN-based port isolation entry
 * Input:
 *      unit   - unit id
 *      index  - index id
 * Output:
 *      pEntry - pointer to vlan-based port isolation entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *      None
 */
int32
dal_cypress_port_vlanBasedIsolationEntry_get(uint32 unit, uint32 index, rtk_port_vlanIsolationEntry_t* pEntry)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, index=%d", unit, index);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(index >= HAL_MAX_NUM_OF_VLAN_PORT_ISO_ENTRY(unit), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pEntry), RT_ERR_NULL_POINTER);

    PORT_SEM_LOCK(unit);

    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, CYPRESS_PORT_ISO_VB_ISO_PM_CTRLr,
                        REG_ARRAY_INDEX_NONE, index, CYPRESS_VALIDf, &pEntry->enable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    if ((ret = reg_array_field_read(unit, CYPRESS_PORT_ISO_VB_ISO_PM_CTRLr,
                        REG_ARRAY_INDEX_NONE, index, CYPRESS_VID_LOWERf, &pEntry->vid)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    if ((ret = reg_array_field_read(unit, CYPRESS_PORT_ISO_VB_ISO_PM_CTRLr,
                        REG_ARRAY_INDEX_NONE, index, CYPRESS_VID_UPPERf, &pEntry->vid_high)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    if ((ret = reg_array_field_read(unit, CYPRESS_PORT_ISO_VB_ISO_PM_CTRLr,
                        REG_ARRAY_INDEX_NONE, index, CYPRESS_VB_ISO_MBR_0f, &pEntry->portmask.bits[0])) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    if ((ret = reg_array_field_read(unit, CYPRESS_PORT_ISO_VB_ISO_PM_CTRLr,
                        REG_ARRAY_INDEX_NONE, index, CYPRESS_VB_ISO_MBR_1f, &pEntry->portmask.bits[1])) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "enable=%d, vid=%d, pPortmask[1]=0x%x, pPortmask[0]=0x%x",
        pEntry->enable, pEntry->vid, pEntry->portmask.bits[1], pEntry->portmask.bits[0]);

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_cypress_port_vlanBasedIsolationEntry_set
 * Description:
 *      Set VLAN-based port isolation entry
 * Input:
 *      unit   - unit id
 *      index  - index id
 *      pEntry - pointer to vlan-based port isolation entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - NULL pointer
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_VLAN_VID     - invalid vid
 *      RT_ERR_PORT_VLAN_ISO_VID_EXIST_IN_OTHER_IDX - vid exists in other entry
 * Note:
 *      None
 */
int32
dal_cypress_port_vlanBasedIsolationEntry_set(uint32 unit, uint32 index, rtk_port_vlanIsolationEntry_t* pEntry)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, index=%d", unit, index);
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "enable=%d, vid=%d, pPortmask[1]=0x%x, pPortmask[0]=0x%x",
        pEntry->enable, pEntry->vid, pEntry->portmask.bits[1], pEntry->portmask.bits[0]);


    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(index >= HAL_MAX_NUM_OF_VLAN_PORT_ISO_ENTRY(unit), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pEntry), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pEntry->enable >= RTK_ENABLE_END), RT_ERR_INPUT);
    RT_PARAM_CHK((pEntry->vid > RTK_VLAN_ID_MAX) || (pEntry->vid_high > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    PORT_SEM_LOCK(unit);

    /* set value to CHIP*/
    if ((ret = reg_array_field_write(unit, CYPRESS_PORT_ISO_VB_ISO_PM_CTRLr,
                        REG_ARRAY_INDEX_NONE, index, CYPRESS_VALIDf, &pEntry->enable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    if ((ret = reg_array_field_write(unit, CYPRESS_PORT_ISO_VB_ISO_PM_CTRLr,
                        REG_ARRAY_INDEX_NONE, index, CYPRESS_VID_LOWERf, &pEntry->vid)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    if ((ret = reg_array_field_write(unit, CYPRESS_PORT_ISO_VB_ISO_PM_CTRLr,
                        REG_ARRAY_INDEX_NONE, index, CYPRESS_VID_UPPERf, &pEntry->vid_high)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    if ((ret = reg_array_field_write(unit, CYPRESS_PORT_ISO_VB_ISO_PM_CTRLr,
                        REG_ARRAY_INDEX_NONE, index, CYPRESS_VB_ISO_MBR_0f, &pEntry->portmask.bits[0])) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    if ((ret = reg_array_field_write(unit, CYPRESS_PORT_ISO_VB_ISO_PM_CTRLr,
                        REG_ARRAY_INDEX_NONE, index, CYPRESS_VB_ISO_MBR_1f, &pEntry->portmask.bits[1])) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_cypress_port_vlanBasedIsolation_vlanSource_get
 * Description:
 *      Get comparing VID type of VLAN-based port isolation
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of linkdown green feature
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *      None
 */
int32
dal_cypress_port_vlanBasedIsolation_vlanSource_get(uint32 unit, rtk_port_vlanIsolationSrc_t *pVlanSrc)
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
    if ((ret = reg_field_read(unit, CYPRESS_PORT_ISO_VB_CTRLr, CYPRESS_VLAN_TYPEf, &value)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    /* translate chip's value to definition */
    switch (value)
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
}

/* Function Name:
 *      dal_cypress_port_vlanBasedIsolation_vlanSource_set
 * Description:
 *      Set comparing VID type of VLAN-based port isolation
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of linkdown green feature
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      None
 */
int32
dal_cypress_port_vlanBasedIsolation_vlanSource_set(uint32 unit, rtk_port_vlanIsolationSrc_t vlanSrc)
{
    int32   ret;
    uint32  value;

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
    if ((ret = reg_field_write(unit, CYPRESS_PORT_ISO_VB_CTRLr, CYPRESS_VLAN_TYPEf, &value)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_cypress_port_phyCrossOverMode_get
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
dal_cypress_port_phyCrossOverMode_get(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t *pMode)
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
}/* end of dal_cypress_port_phyCrossOverMode_get */

/* Function Name:
 *      dal_cypress_port_phyCrossOverMode_set
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
dal_cypress_port_phyCrossOverMode_set(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t mode)
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
}/* end of dal_cypress_port_phyCrossOverMode_set */

/* Function Name:
 *      dal_cypress_port_phyCrossOverStatus_get
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
dal_cypress_port_phyCrossOverStatus_get(uint32 unit, rtk_port_t port, rtk_port_crossOver_status_t *pStatus)
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
}/* end of dal_cypress_port_phyCrossOverMode_get */


/* Function Name:
 *      dal_cypress_port_flowCtrlEnable_get
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
 *      This API can not use sem lock since it takes use of dal layer API
 */
int32
dal_cypress_port_flowCtrlEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
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

    if ((ret = dal_cypress_port_phyAutoNegoEnable_get(unit, port, &nway_enable)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    if (ENABLED == nway_enable)
    {
        if ((ret = dal_cypress_port_phyAutoNegoAbility_get(unit, port, &ability)) != RT_ERR_OK)
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
        if ((ret = dal_cypress_port_phyForceModeAbility_get(unit, port, &speed, &duplex, &flowctrl_enable)) != RT_ERR_OK)
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }
        (*pEnable) = flowctrl_enable;
    }

    return RT_ERR_OK;
} /* end of dal_cypress_port_flowCtrlEnable_get */

/* Function Name:
 *      dal_cypress_port_flowCtrlEnable_set
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
 *      This API can not use sem lock since it takes use of dal layer API
 */
int32
dal_cypress_port_flowCtrlEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
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

    if ((ret = dal_cypress_port_phyAutoNegoAbility_get(unit, port, &ability)) != RT_ERR_OK)
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
    if ((ret = dal_cypress_port_phyAutoNegoAbility_set(unit, port, &ability)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    if ((ret = dal_cypress_port_phyForceModeAbility_get(unit, port, &speed, &duplex, &flowctrl_enable)) != RT_ERR_OK)
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
    if ((ret = dal_cypress_port_phyForceModeAbility_set(unit, port, speed, duplex, flowctrl_enable)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_cypress_port_flowCtrlEnable_set */

/* Function Name:
 *      dal_cypress_port_phyComboPortFiberMedia_get
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
dal_cypress_port_phyComboPortFiberMedia_get(uint32 unit, rtk_port_t port, rtk_port_fiber_media_t *pMedia)
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
} /* end of dal_cypress_port_phyComboPortFiberMedia_get */

/* Function Name:
 *      dal_cypress_port_phyComboPortFiberMedia_set
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
dal_cypress_port_phyComboPortFiberMedia_set(uint32 unit, rtk_port_t port, rtk_port_fiber_media_t media)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, media=%d",
           unit, port, media);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((media >= PORT_MEDIA_END), RT_ERR_INPUT);

    if(media == PORT_FIBER_MEDIA_1000)
        pPhy_info[unit]->force_fiber_mode_speed[port] = PORT_SPEED_1000M;

    if(media == PORT_FIBER_MEDIA_100)
        pPhy_info[unit]->force_fiber_mode_speed[port] = PORT_SPEED_100M;

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
} /* end of dal_cypress_port_phyComboPortFiberMedia_set */

/* Function Name:
 *      dal_cypress_port_linkDownPowerSavingEnable_get
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
dal_cypress_port_linkDownPowerSavingEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
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
} /* end of dal_cypress_port_linkDownPowerSavingEnable_get */

/* Function Name:
 *      dal_cypress_port_linkDownPowerSavingEnable_set
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
dal_cypress_port_linkDownPowerSavingEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
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
} /* end of dal_cypress_port_linkDownPowerSavingEnable_set */

/* Function Name:
 *      dal_cypress_port_linkMedia_get
 * Description:
 *      Get link status and media
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
dal_cypress_port_linkMedia_get(uint32 unit, rtk_port_t port,
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

    /* link status need to get twice */
    if ((ret = reg_array_field_read(unit, CYPRESS_MAC_LINK_STSr, port,
                          REG_ARRAY_INDEX_NONE, CYPRESS_LINK_STSf, &val)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    if ((ret = reg_array_field_read(unit, CYPRESS_MAC_LINK_STSr, port,
                          REG_ARRAY_INDEX_NONE, CYPRESS_LINK_STSf, &val)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    if (0 == val)
    {
        *pStatus = PORT_LINKDOWN;
        /* media should be ignored when link down */
        /*pMedia = PORT_MEDIA_COPPER;*/
    }
    else
    {
        *pStatus = PORT_LINKUP;
        if ((ret = reg_array_field_read(unit, CYPRESS_MAC_LINK_MEDIA_STSr, port,
                          REG_ARRAY_INDEX_NONE, CYPRESS_MEDIA_STSf, &val)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }

        if (0 == val)
            *pMedia = PORT_MEDIA_COPPER;
        else
            *pMedia = PORT_MEDIA_FIBER;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_cypress_port_linkMedia_get */

/************************************PHY WATCH DOG****************************************/
static uint32   esdPortStart = 0;
static uint8    esdPhyNumPerScan = 1;

/* Function Name:
 *      _dal_cypress_switch_phy_cable_esd
 * Description:
 *      Cable ESD problem.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is monitor for Cable ESD probem and patch it.
 *      Protect PHY page by port semaphore.
 */
static int32 _dal_cypress_switch_phy_cable_esd(uint32 unit)
{
    int32       ret;
    rtk_port_t  port;
    uint32      phy_data;
    uint32      phy_access_flag;
    uint32      phy_rst_flag;
    uint32      regData, regOriData;
    uint32      portNum, num = 0;

    /*Right now Cable ESD problem only for RTL839X*/
    if (HAL_IS_RTL8390_FAMILY_ID(unit))
    {
        if (HAL_GET_MAX_ETHER_PORT(unit) < esdPortStart)
            esdPortStart = HAL_GET_MIN_ETHER_PORT(unit);

        for (port = esdPortStart; num < esdPhyNumPerScan; ++port)
        {
            if (HAL_GET_MAX_ETHER_PORT(unit) < port)
            {
                port = HAL_GET_MIN_ETHER_PORT(unit);
            }

            /* Port check */
            if (!HAL_IS_PORT_EXIST(unit, port) ||
                    !HAL_IS_PHY_EXIST(unit, port) ||
                    !HAL_IS_PHY_INFO_EXIST(unit, port))
            {
                continue;
            }

            portNum = HAL_GET_PHY_NUM(unit, port);
            if (0 != (port % portNum))
            {
                continue;
            }

            phy_rst_flag = 0;

            PORT_SEM_LOCK(unit);

            /* disable MAC polling */
            if ((ret = reg_field_read(unit, CYPRESS_SMI_GLB_CTRLr,
                    CYPRESS_MDX_POLLING_ENf, &regOriData)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Port:%d failed!\n", port);
            }

            regData = 0;
            if ((ret = reg_field_write(unit, CYPRESS_SMI_GLB_CTRLr,
                    CYPRESS_MDX_POLLING_ENf, &regData)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Port:%d failed!\n", port);
            }

            /*Check PHY can be accessed or not yet?*/
            phy_access_flag = 0;
            ret = hal_miim_read(unit, port, 0, 2, &phy_data);
            if (ret != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Port:%d failed!\n", port);
                PORT_SEM_UNLOCK(unit);
                continue;
            }

            if(phy_data == 0x1c)
                phy_access_flag = 0x1;

            phy_rst_flag = 0x0;
            /*PHY can be accessed*/
            if (0x1 == phy_access_flag)
            {
                /*Check whether External PHY has been reseted or not*/
                /*First check*/

                if (phy_chk_rst_status(unit, port, &phy_rst_flag) == RT_ERR_OK)
                {
                    /*This Phy really has been reseted*/
                    if (0x1 == phy_rst_flag)
                    {
                        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "[PHY-WATCHDOG] --->Cable ESD--->Port:%d found Reset!\n", port);

                        /*Reset PHY again*/
                        ret = hal_miim_write(unit, port, 0x0, 30, 0x8);
                        if(ret != RT_ERR_OK)
                        {
                            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Port:%d failed!\n", port);
                            /* enable MAC polling */
                            if ((ret = reg_field_write(unit, CYPRESS_SMI_GLB_CTRLr,
                                    CYPRESS_MDX_POLLING_ENf, &regOriData)) != RT_ERR_OK)
                            {
                                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Port:%d failed!\n", port);
                            }
                            PORT_SEM_UNLOCK(unit);
                            continue;
                        }

                        ret = hal_miim_write(unit, port, 0x262, 16, 0x1);
                        if(ret != RT_ERR_OK)
                        {
                            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Port:%d failed!\n", port);
                            /* enable MAC polling */
                            if ((ret = reg_field_write(unit, CYPRESS_SMI_GLB_CTRLr,
                                    CYPRESS_MDX_POLLING_ENf, &regOriData)) != RT_ERR_OK)
                            {
                                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Port:%d failed!\n", port);
                            }
                            PORT_SEM_UNLOCK(unit);
                            continue;
                        }

                        ret = hal_miim_write(unit, port, 0x0, 30, 0);
                        if(ret != RT_ERR_OK)
                        {
                            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Port:%d failed!\n", port);
                            /* enable MAC polling */
                            if ((ret = reg_field_write(unit, CYPRESS_SMI_GLB_CTRLr,
                                    CYPRESS_MDX_POLLING_ENf, &regOriData)) != RT_ERR_OK)
                            {
                                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Port:%d failed!\n", port);
                            }
                            PORT_SEM_UNLOCK(unit);
                            continue;
                        }

                        /*Delay until PHY reset done*/
                        osal_time_usleep(100 * 1000); /* delay 100mS */

                        /*Do the PHY Re-Patch*/
                        ret = phy_patch_set(unit,  port);
                        if ((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
                        {
                            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "[PHY-WATCHDOG] --->Port:%d PHY re-Patch failed!\n", port);
                            /* enable MAC polling */
                            if ((ret = reg_field_write(unit, CYPRESS_SMI_GLB_CTRLr,
                                    CYPRESS_MDX_POLLING_ENf, &regOriData)) != RT_ERR_OK)
                            {
                                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Port:%d failed!\n", port);
                            }
                            PORT_SEM_UNLOCK(unit);
                            continue;
                        }
                    }
                }
            }

            /* enable MAC polling */
            if ((ret = reg_field_write(unit, CYPRESS_SMI_GLB_CTRLr,
                    CYPRESS_MDX_POLLING_ENf, &regOriData)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Port:%d failed!\n", port);
            }

            PORT_SEM_UNLOCK(unit);

            esdPortStart = port + portNum;
            ++num;

            if (0x1 == phy_rst_flag)
            {
                phy_watchdog_cnt++;
                osal_time_udelay(1000 * 1000);
                ret = hal_mac_serdes_rst(unit, (port/4));
                if (HAL_IS_GE_PORT(unit, port) && portNum > 4)
                {
                    ret = hal_mac_serdes_rst(unit, ((port/4)+1));
                }
                dal_waMon_phyReconfig_portMaskSet(unit, port);
            }
        }
    }

    return RT_ERR_OK;
}   /* end of _dal_cypress_switch_phy_cable_esd */

/* Function Name:
 *      dal_cypress_port_phy_watchdog
 * Description:
 *      Monitor for phy problem.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is monitor for phy problem and patch it.
 *      Protect PHY page by port semaphore.
 */
int32
dal_cypress_port_phy_watchdog(uint32 unit)
{
    int32 ret;

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /***************************1: Cable ESD problem Start*********************/
    if ((ret = _dal_cypress_switch_phy_cable_esd(unit)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    /***************************1: Cable ESD problem End**********************/


    return RT_ERR_OK;
}   /* end of dal_cypress_port_phy_watchdog */

#if defined(CONFIG_SDK_WA_SERDES_WATCHDOG)
/* Function Name:
 *      dal_cypress_port_serdes_watchdog
 * Description:
 *      Monitor for serdes link statuse.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is monitor serdes link down and patch it.
 *      Protect PHY page by port semaphore.
 */
int32
dal_cypress_port_serdes_watchdog(uint32 unit)
{
    uint32  sdsReg[] = {0xA328, 0xA728, 0xAB28, 0xAF28, 0xB320, 0xB728, 0xBB20};
    uint32  sdsIdx, sdsMode, sdsAddr, val;
    uint32  ofst, bit;
    int32   ret;

    if (HAL_IS_RTL8390_FAMILY_ID(unit))
    {
        /* Check serdes interface mode */
        for (sdsIdx = 0; sdsIdx < (2 * (sizeof(sdsReg)/sizeof(uint32))); ++sdsIdx)
        {
            if ((ret = reg_array_field_read(unit, CYPRESS_MAC_SERDES_IF_CTRLr,
                    REG_ARRAY_INDEX_NONE, sdsIdx, CYPRESS_SERDES_SPD_SELf,
                    &sdsMode)) != RT_ERR_OK)
                return ret;

            /* QSGMII */
            if (6 == sdsMode)
            {
                /* check serdes link */
                ofst = (0x400 * (sdsIdx / 2)) + (0x100 * (sdsIdx % 2));
                sdsAddr = 0xA078 + ofst;

                ioal_mem32_read(unit, sdsAddr, &val);
                ioal_mem32_read(unit, sdsAddr, &val);

                if (0x1ff0000 != val)
                {
                    ++macSerdes_watchdog_cnt;

                    sdsAddr = sdsReg[sdsIdx / 2] + (0x80 * (sdsIdx % 2));
                    ioal_mem32_read(unit, sdsAddr, &val);
                    switch (sdsIdx)
                    {
                        case 8 ... 9:
                        case 12 ... 13:
                            val &= ~(1 << 3);
                            break;
                        default:
                            val &= ~1;
                    }
                    ioal_mem32_write(unit, sdsAddr, val);

                    switch (sdsIdx)
                    {
                        case 8 ... 9:
                        case 12 ... 13:
                            sdsAddr += 0x20;
                            bit = 15;
                            break;
                        default:
                            bit = 9;
                    }
                    ioal_mem32_read(unit, sdsAddr, &val);
                    val |= (1 << bit);
                    ioal_mem32_write(unit, sdsAddr, val);

                    ioal_mem32_read(unit, sdsAddr, &val);
                    val &= ~(1 << bit);
                    ioal_mem32_write(unit, sdsAddr, val);

                    /* digital */
                    sdsAddr = 0xa004 + ofst;
                    ioal_mem32_read(unit, sdsAddr, &val);
                    val &= (~(0x1 << 22));
                    val |= (0x1 << 22);
                    ioal_mem32_write(unit, sdsAddr, val);

                    ioal_mem32_read(unit, sdsAddr, &val);
                    val &= (~(0x1 << 22));
                    ioal_mem32_write(unit, sdsAddr, val);
                }
            }
        }
    }

    return RT_ERR_OK;
}   /* end of dal_cypress_port_serdes_watchdog */
#endif  /* CONFIG_SDK_WA_SERDES_WATCHDOG */

#if defined(CONFIG_SDK_WA_FIBER_RX_WATCHDOG)
#define FIBER_RX_WATCHDOG_CHK_PORT_NUM  1

static uint32 fiberRxOrder = 0;
static uint8 fiberRxWDSts[52];

/* Function Name:
 *      _dal_cypress_port_serdesFiber10g_handeler
 * Description:
 *      Handle serdes fiber 10G
 * Input:
 *      unit - unit id
 *      port - port which is to be handle
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED - initialize fail
 *      RT_ERR_OK     - initialize success
 * Note:
 *
 */
void
_dal_cypress_port_serdesFiber10g_handeler(uint32 unit, uint32 port)
{
    rtk_port_linkStatus_t   linkSts;
    uint32                  val, ofst = 0, sdsMode;

    reg_array_field_read(unit, CYPRESS_MAC_LINK_STSr,
            port, REG_ARRAY_INDEX_NONE, CYPRESS_LINK_STSf, &linkSts);
    reg_array_field_read(unit, CYPRESS_MAC_LINK_STSr,
            port, REG_ARRAY_INDEX_NONE, CYPRESS_LINK_STSf, &linkSts);
    if (PORT_LINKDOWN != linkSts)
    {
        return;
    }

    if (24 == port)
    {
        val = 0x9203;
        ofst = 8;
    }
    else if (36 == port)
    {
        val = 0x9303;
        ofst = 12;
    }
    else
    {
        return;
    }

    reg_field_write(unit, CYPRESS_DBG_CTRLr, CYPRESS_DBG_SELf, &val);
    osal_time_usleep(10 * 1000);
    reg_field_read(unit, CYPRESS_DBG_DATA_CTRLr, CYPRESS_DBG_DATAf, &val);

    if (((val >> 3) & 0x3) != 0x3)
    {
        fiber_rx_watchdog_cnt++;

        sdsMode = 7;
        reg_array_field_write(unit, CYPRESS_MAC_SERDES_IF_CTRLr,
                REG_ARRAY_INDEX_NONE, ofst, CYPRESS_SERDES_SPD_SELf, &sdsMode);

        osal_time_usleep(500 * 1000);
        sdsMode = 1;
        reg_array_field_write(unit, CYPRESS_MAC_SERDES_IF_CTRLr,
                REG_ARRAY_INDEX_NONE, ofst, CYPRESS_SERDES_SPD_SELf, &sdsMode);
    }

    return;
}   /* end of _dal_cypress_port_serdesFiber10g_handeler */

/* Function Name:
 *      _dal_cypress_port_serdesFiber1g_handeler
 * Description:
 *      Handle serdes fiber 1G
 * Input:
 *      unit - unit id
 *      port - port which is to be handle
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED - initialize fail
 *      RT_ERR_OK     - initialize success
 * Note:
 *
 */
void
_dal_cypress_port_serdesFiber1g_handeler(uint32 unit, uint32 port)
{
    uint32  reg, regData;
    uint32  loop, errCnt = 0;

    if (HAL_IS_10GE_PORT(unit, port))
    {
        if (24 == port)
        {
            reg = CYPRESS_SDS8_9_XSG0r;
        }
        else if (36 == port)
        {
            reg = CYPRESS_SDS12_13_XSG0r;
        }
        else
        {
            return;
        }
    }   /* if (HAL_IS_10GE_PORT(unit, port)) */
    else
    {
        if (48 == port)
        {
            reg = CYPRESS_SDS12_13_XSG0r;
        }
        else if (49 == port)
        {
            reg = CYPRESS_SDS12_13_XSG1r;
        }
        else
        {
            return;
        }
    }   /* else of if (HAL_IS_10GE_PORT(unit, port)) */

    reg_field_read(unit, reg, CYPRESS_SRE24_CFG_TMR_ALIf, &regData);
    regData &= ~(0x1);
    reg_field_write(unit, reg, CYPRESS_SRE24_CFG_TMR_ALIf, &regData);

    regData = 0;
    reg_field_write(unit, reg, CYPRESS_SRE24_CFG_SYMBOLERR_CNTf, &regData);

    /* read & clear */
    reg_field_read(unit, reg, CYPRESS_SRE25_MUX_SYMBOLERR_CNTf, &regData);

    for(loop = 0; loop < 3; ++loop)
    {
        reg_field_read(unit, reg, CYPRESS_SRE25_MUX_SYMBOLERR_CNTf, &regData);
        if (0xFF == regData)
        {
            ++errCnt;
        }
        osal_time_usleep(200);
    }

    if (errCnt >= 2)
    {
        fiber_rx_watchdog_cnt++;
        RT_DBG(LOG_EVENT, (MOD_DAL|MOD_PORT), "port %d\n", port);

        if (HAL_IS_10GE_PORT(unit, port))
        {
            if (24 == port)
            {
                regData = 0;
                reg_field_write(unit, CYPRESS_SDS8_9_ANA_TGr,
                        CYPRESS_S0_REG_RX_FORCERUNf, &regData);

                regData = 1;
                reg_field_write(unit, CYPRESS_SDS8_9_ANA_TGr,
                        CYPRESS_S0_REG_RX_EN_SELF_XSGf, &regData);

                osal_time_usleep(200);
                regData = 0;
                reg_field_write(unit, CYPRESS_SDS8_9_ANA_TGr,
                        CYPRESS_S0_REG_RX_EN_SELF_XSGf, &regData);

                regData = 1;
                reg_field_write(unit, CYPRESS_SDS8_9_XSG0r,
                        CYPRESS_SR3_SOFT_RSTf, &regData);

                osal_time_usleep(200);
                regData = 0;
                reg_field_write(unit, CYPRESS_SDS8_9_XSG0r,
                        CYPRESS_SR3_SOFT_RSTf, &regData);

            }
            else if (36 == port)
            {
                regData = 0;
                reg_field_write(unit, CYPRESS_SDS12_13_ANA_TGr,
                        CYPRESS_S0_REG_RX_FORCERUNf, &regData);

                regData = 1;
                reg_field_write(unit, CYPRESS_SDS12_13_ANA_TGr,
                        CYPRESS_S0_REG_RX_EN_SELF_XSGf, &regData);

                osal_time_usleep(200);
                regData = 0;
                reg_field_write(unit, CYPRESS_SDS12_13_ANA_TGr,
                        CYPRESS_S0_REG_RX_EN_SELF_XSGf, &regData);

                regData = 1;
                reg_field_write(unit, CYPRESS_SDS12_13_XSG0r,
                        CYPRESS_SR3_SOFT_RSTf, &regData);

                osal_time_usleep(200);
                regData = 0;
                reg_field_write(unit, CYPRESS_SDS12_13_XSG0r,
                        CYPRESS_SR3_SOFT_RSTf, &regData);
            }
        }   /* if (HAL_IS_10GE_PORT(unit, port)) */
        else
        {
            if (48 == port)
            {
                regData = 0;
                reg_field_write(unit, CYPRESS_SDS12_13_ANA_TGr,
                        CYPRESS_S0_REG_RX_FORCERUNf, &regData);

                regData = 1;
                reg_field_write(unit, CYPRESS_SDS12_13_ANA_TGr,
                        CYPRESS_S0_REG_RX_EN_SELF_XSGf, &regData);

                osal_time_usleep(200);
                regData = 0;
                reg_field_write(unit, CYPRESS_SDS12_13_ANA_TGr,
                        CYPRESS_S0_REG_RX_EN_SELF_XSGf, &regData);

                regData = 1;
                reg_field_write(unit, CYPRESS_SDS12_13_XSG0r,
                        CYPRESS_SR3_SOFT_RSTf, &regData);

                osal_time_usleep(200);
                regData = 0;
                reg_field_write(unit, CYPRESS_SDS12_13_XSG0r,
                        CYPRESS_SR3_SOFT_RSTf, &regData);
            }
            else if (49 == port)
            {
                regData = 0;
                reg_field_write(unit, CYPRESS_SDS12_13_ANA_TGr,
                        CYPRESS_S1_REG_RX_FORCERUNf, &regData);

                regData = 1;
                reg_field_write(unit, CYPRESS_SDS12_13_ANA_TGr,
                        CYPRESS_S1_REG_RX_EN_SELF_XSGf, &regData);

                osal_time_usleep(200);
                regData = 0;
                reg_field_write(unit, CYPRESS_SDS12_13_ANA_TGr,
                        CYPRESS_S1_REG_RX_EN_SELF_XSGf, &regData);

                regData = 1;
                reg_field_write(unit, CYPRESS_SDS12_13_XSG1r,
                        CYPRESS_SR3_SOFT_RSTf, &regData);

                osal_time_usleep(200);
                regData = 0;
                reg_field_write(unit, CYPRESS_SDS12_13_XSG1r,
                        CYPRESS_SR3_SOFT_RSTf, &regData);
            }
        }   /* else of if (HAL_IS_10GE_PORT(unit, port)) */
    }   /* if (errCnt >= 2) */

    return;
}   /* end of _dal_cypress_port_serdesFiber1g_handeler */

/* Function Name:
 *      dal_cypress_port_fiber_rx_watchdog
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
int32
dal_cypress_port_fiber_rx_watchdog(uint32 unit)
{
    uint32  maxPortNum = HAL_GET_MAX_ETHER_PORT(unit);
    uint32  port_index;
    int32   phy_id, ret, chkFlag = 0;
    uint32  backup_reg30_data;
    uint32  error_count, chk_error, chk_loop, reg_data, regOriData;
    rtk_port_media_t portMedia;
    uint32  clearFlag, rstFlag;

    if (fiberRxOrder > maxPortNum)
        fiberRxOrder = 0;

    for(port_index = fiberRxOrder; port_index <= maxPortNum && chkFlag < FIBER_RX_WATCHDOG_CHK_PORT_NUM; port_index++)
    {
        chk_error = 0;
        clearFlag = rstFlag = 0;

        if (HAL_IS_GE_COMBO_PORT(unit, port_index))
        {
            ++chkFlag;
            PORT_SEM_LOCK(unit);

            /* disable MAC polling */
            if ((ret = reg_field_read(unit, CYPRESS_SMI_GLB_CTRLr,
                    CYPRESS_MDX_POLLING_ENf, &regOriData)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Port:%d failed!\n", port_index);
            }

            reg_data = 0;
            if ((ret = reg_field_write(unit, CYPRESS_SMI_GLB_CTRLr,
                    CYPRESS_MDX_POLLING_ENf, &reg_data)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Port:%d failed!\n", port_index);
            }

            phy_drv_chk(unit, port_index, &phy_id);
            if((phy_id == RT_PHYDRV_RTL8218FB_MP) || (phy_id == RT_PHYDRV_RTL8214FC_MP))
            {
                phy_media_get(unit, port_index, &portMedia);
                if(portMedia == PORT_MEDIA_FIBER) /*Checking Media is Fiber*/
                {
                    /* only work for giga */
                    hal_miim_read(unit, port_index, 0, 0, &reg_data);
                    if (0 == ((reg_data >> 13) & 0x1) && 1 == ((reg_data >> 6) & 0x1))
                    {
                        hal_miim_read(unit, port_index, 0, 1, &reg_data);
                        hal_miim_read(unit, port_index, 0, 1, &reg_data);

                        /*Checking Port is Link Down*/
                        if ((reg_data & 0x4) == 0)
                        {
                            hal_miim_read(unit, port_index, 0, 30, &backup_reg30_data);
                            hal_miim_write(unit, port_index, 0, 30, 0x3); /*Write page 0 reg 30 = 0x3*/
                            hal_miim_write(unit, port_index, 0xf, 0x10, 0x10); /*Write page 0xf reg 0x10 = 0x10*/

                            /*Check Error counter three times*/
                            for(chk_loop = 0; chk_loop < 3; chk_loop++)
                            {
                                hal_miim_read(unit, port_index, 0xf, 0x11, &error_count); /*Read Error counter*/
                                if(error_count == 0xffff)
                                    chk_error++;
                            }

                            /*Error happen*/
                            if(chk_error > 2)
                            {
                                rstFlag = 1;
                            }
                            /* other situation */
                            else
                            {
                                chk_error = 0;

                                hal_miim_write(unit, port_index, 0, 30, 0x3);

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
                                        rstFlag = 1;
                                    }
                                    else
                                    {
                                        fiberRxWDSts[port_index] = 1;
                                        clearFlag = 0;
                                    }
                                }
                                else
                                {
                                    clearFlag = 1;
                                }
                            }
                            /*Restore Page 0 Reg 30*/
                            hal_miim_write(unit, port_index, 0, 30, backup_reg30_data);
                        }
                        else
                        {
                            clearFlag = 1;
                        }
                    }
                    else
                    {
                        clearFlag = 1;
                    }
                }
                else
                {
                    clearFlag = 1;
                }
            }

            if (1 == rstFlag)
            {
                fiber_rx_watchdog_cnt++;
                /*Reset RX*/
                hal_miim_read(unit, port_index, 0, 30, &backup_reg30_data);
                hal_miim_write(unit, port_index, 0, 30, 0x3); /*Write page 0 reg 30 = 0x3*/

                hal_miim_read(unit, port_index, 0xc, 0x16, &reg_data);
                reg_data |= (1 << 15);
                hal_miim_write(unit, port_index, 0xc, 0x16, reg_data);
                osal_time_usleep(100 * 1000);
                hal_miim_read(unit, port_index, 0xc, 0x16, &reg_data);
                reg_data &= ~(1 << 15);
                hal_miim_write(unit, port_index, 0xc, 0x16, reg_data);

                hal_miim_write(unit, port_index, 0, 30, backup_reg30_data); /*Write page 0 reg 30 = 0x0*/
            }

            /* enable MAC polling */
            if ((ret = reg_field_write(unit, CYPRESS_SMI_GLB_CTRLr,
                    CYPRESS_MDX_POLLING_ENf, &regOriData)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Port:%d failed!\n", port_index);
            }

            if (1 == clearFlag || 1 == rstFlag)
            {
                fiberRxWDSts[port_index] = 0;
            }

            PORT_SEM_UNLOCK(unit);
        }
        /*Check SerDes RX for Direct Fiber module*/
        else if(HAL_IS_SERDES_PORT(unit, port_index))
        {
            ++chkFlag;

            /*For RTL839X & RTL835X*/
            if (HAL_IS_RTL8390_FAMILY_ID(unit) || HAL_IS_RTL8350_FAMILY_ID(unit))
            {
                uint32  speed;

                PORT_SEM_LOCK(unit);

                if (HAL_IS_10GE_PORT(unit, port_index))
                {
                    if (24 != port_index && 36 != port_index)
                    {
                        PORT_SEM_UNLOCK(unit);
                        continue;
                    }
                }
                else
                {
                    if (48 != port_index && 49 != port_index)
                    {
                        PORT_SEM_UNLOCK(unit);
                        continue;
                    }
                }

                /* only work for giga */
                phy_speed_get(unit, port_index, &speed);
                if (PORT_SPEED_10G == speed)
                {
                    _dal_cypress_port_serdesFiber10g_handeler(unit, port_index);
                }
                else if (PORT_SPEED_1000M == speed)
                {
                    _dal_cypress_port_serdesFiber1g_handeler(unit, port_index);
                }

                PORT_SEM_UNLOCK(unit);
            }
        }
    }

    fiberRxOrder = port_index;

    return RT_ERR_OK;
}
#endif  /* CONFIG_SDK_WA_FIBER_RX_WATCHDOG */


/* Function Name:
 *      _dal_cypress_port_auto_swqrst_check
 * Description:
 *
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is monitor for packet buffer run out and patch it.
 *      Protect PHY page by port semaphore.
 */
static int32 _dal_cypress_port_auto_swqrst_check(uint32 unit)
{
    return RT_ERR_OK;
}   /* end of _dal_cypress_port_auto_swqrst_check */

/* Function Name:
 *      dal_cypress_port_pktbuf_watchdog
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
 *      The API is monitor for detect packet buffer problem and patch it.
 *      Protect PHY page by port semaphore.
 */
int32
dal_cypress_port_pktbuf_watchdog(uint32 unit)
{
    int32 ret;

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    PORT_SEM_LOCK(unit);

    /***************************1: Auto Software Queue Reset Start**************/
    if ((ret = _dal_cypress_port_auto_swqrst_check(unit)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    /***************************1: Crash  Prevention Start*******************/

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /* end of dal_cypress_port_pktbuf_watchdog */

/* Function Name:
 *      dal_cypress_port_fiberDownSpeedEnable_get
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
dal_cypress_port_fiberDownSpeedEnable_get(uint32 unit, rtk_port_t port,
    rtk_enable_t *pEnable)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d,port=%d",unit, port);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    /* function body */
    PORT_SEM_LOCK(unit);

    /* get value from CHIP*/
    if ((ret = phy_fiberDownSpeedEnable_get(unit, port, pEnable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "port %d", port);
        return ret;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pEnable=%d", *pEnable);

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /* end of dal_cypress_port_fiberDownSpeedEnable_get */

/* Function Name:
 *      dal_cypress_port_fiberDownSpeedEnable_set
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
dal_cypress_port_fiberDownSpeedEnable_set(uint32 unit, rtk_port_t port,
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
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "port %d", port);
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /* end of dal_cypress_port_fiberDownSpeedEnable_set */

/* Function Name:
 *      dal_cypress_port_downSpeedEnable_get
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
dal_cypress_port_downSpeedEnable_get(uint32 unit, rtk_port_t port,
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
    if ((ret = hal_miim_globalPollingEnable_get(unit, &pollSts)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "port %d disable MAC polling fail (0x%x)", port, ret);
        return ret;
    }

    /* disable MAC polling */
    if ((ret = hal_miim_globalPollingEnable_set(unit, DISABLED)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "port %d disable MAC polling fail (0x%x)", port, ret);
        return ret;
    }

    /* get value from CHIP*/
    if ((ret = phy_downSpeedEnable_get(unit, port, pEnable)) != RT_ERR_OK)
    {
        /* Restore MAC polling status*/
        if ((retError = hal_miim_globalPollingEnable_set(unit, pollSts)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(retError, (MOD_DAL|MOD_PORT), "port %d enable MAC polling fail (0x%x)", port, retError);
            return retError;
        }
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "port %d", port);
        return ret;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pEnable=%d", *pEnable);

    /* Restore MAC polling status*/
    if ((ret = hal_miim_globalPollingEnable_set(unit, pollSts)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "port %d enable MAC polling fail (0x%x)", port, ret);
       return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /* end of dal_cypress_port_downSpeedEnable_get */

/* Function Name:
 *      dal_cypress_port_downSpeedEnable_set
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
dal_cypress_port_downSpeedEnable_set(uint32 unit, rtk_port_t port,
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
    if ((ret = hal_miim_globalPollingEnable_get(unit, &pollSts)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "port %d disable MAC polling fail (0x%x)", port, ret);
        return ret;
    }

    /* disable MAC polling */
    if ((ret = hal_miim_globalPollingEnable_set(unit, DISABLED)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "port %d disable MAC polling fail (0x%x)", port, ret);
        return ret;
    }

    /* set value from CHIP*/
    if ((ret = phy_downSpeedEnable_set(unit, port, enable)) != RT_ERR_OK)
    {
        /* Restore MAC polling status*/
        if ((retError = hal_miim_globalPollingEnable_set(unit, pollSts)) != RT_ERR_OK)
        {
            PORT_SEM_UNLOCK(unit);
            RT_ERR(retError, (MOD_DAL|MOD_PORT), "port %d enable MAC polling fail (0x%x)", port, retError);
            return retError;
        }
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "port %d", port);
        return ret;
    }

    /* Restore MAC polling status*/
    if ((ret = hal_miim_globalPollingEnable_set(unit, pollSts)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "port %d enable MAC polling fail (0x%x)", port, ret);
       return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /* end of dal_cypress_port_downSpeedEnable_set */


/* Function Name:
 *      dal_cypress_port_fiberNwayForceLinkEnable_get
 * Description:
 *      When fiber port is configured N-way,
 *      which can link with link partner is configured force mode.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pEnable - fiber Nway force links status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      None
 */
int32
dal_cypress_port_fiberNwayForceLinkEnable_get(uint32 unit, rtk_port_t port,
    rtk_enable_t *pEnable)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d,port=%d",unit, port);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    /* function body */
    PORT_SEM_LOCK(unit);

    if ((ret = phy_fiberNwayForceLinkEnable_get(unit, port, pEnable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "port %d", port);
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /* end of dal_cypress_port_fiberNwayForceLinkEnable_get */

/* Function Name:
 *      dal_cypress_port_fiberMedia_set
 * Description:
 *      When fiber port is configured N-way,
 *      which can link with link partner is configured force mode.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - fiber Nway force links status
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
dal_cypress_port_fiberNwayForceLinkEnable_set(uint32 unit, rtk_port_t port,
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

    if ((ret = phy_fiberNwayForceLinkEnable_set(unit, port, enable)) != RT_ERR_OK)
    {
        PORT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "port %d", port);
        return ret;
    }

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /* end of dal_cypress_port_fiberNwayForceLinkEnable_set */

/* Function Name:
 *      dal_cypress_port_fiberOAMLoopBack_set
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
dal_cypress_port_fiberOAMLoopBack_set(uint32 unit, rtk_port_t port,
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
        dal_cypress_port_rxEnable_set(unit,  port,  ENABLED);
    else
        dal_cypress_port_rxEnable_set(unit,  port,  DISABLED);

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
}   /* end of dal_cypress_port_OAMLoopBack_set */

/* Function Name:
 *      dal_cypress_port_fiberInternalLoopBack_set
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
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      SERDES port is not included.
 */
int32
dal_cypress_port_fiberInternalLoopBack_set(uint32 unit, rtk_port_t port,
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
}   /* end of dal_cypress_port_fiberInternalLoopBack_set */

/* Function Name:
 *      dal_cypress_port_10gMedia_set
 * Description:
 *      Set 10G port media of the specific port
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      media - port media
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
 *      (1) The media value is as following:
 *          - PORT_10GMEDIA_FIBER_10G,
 *          - PORT_10GMEDIA_FIBER_1G,
 *          - PORT_10GMEDIA_DAC_50CM,
 *          - PORT_10GMEDIA_DAC_100CM,
 *          - PORT_10GMEDIA_DAC_300CM,
 */
int32
dal_cypress_port_10gMedia_set(uint32 unit, rtk_port_t port, rtk_port_10gMedia_t media)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d,port=%d,media=%d",unit, port, media);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_SERDES_10GE_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((PORT_10GMEDIA_END <= media), RT_ERR_INPUT);

    PORT_SEM_LOCK(unit);

    ret = phy_8390_10gMedia_set(unit, port, media);

    PORT_SEM_UNLOCK(unit);

    return ret;
}   /* end of dal_cypress_port_10gMedia_set */

/* Function Name:
 *      dal_cypress_port_10gMedia_get
 * Description:
 *      Get 10G port media of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      media   - pointer to the media type
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      (1) The media type of the port is as following:
 *          - PORT_10GMEDIA_FIBER_10G,
 *          - PORT_10GMEDIA_FIBER_1G,
 *          - PORT_10GMEDIA_DAC_50CM,
 *          - PORT_10GMEDIA_DAC_100CM,
 *          - PORT_10GMEDIA_DAC_300CM,
 */
int32
dal_cypress_port_10gMedia_get(uint32 unit, rtk_port_t port, rtk_port_10gMedia_t *media)
{
    uint32  speed;
    int32   ret = RT_ERR_OK;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d,port=%d",unit, port);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_SERDES_10GE_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == media), RT_ERR_NULL_POINTER);

    /* function body */
    PORT_SEM_LOCK(unit);
    phy_8390_speed_get(unit, port, &speed);
    PORT_SEM_UNLOCK(unit);

    if (PORT_SPEED_10G == speed)
        *media = PORT_10GMEDIA_FIBER_10G;
    else
        *media = PORT_10GMEDIA_FIBER_1G;

    return ret;
}   /* end of dal_cypress_port_10gMedia_get */

/* Function Name:
 *      dal_cypress_port_10gSds_restart
 * Description:
 *      Restart 10G port serdes
 * Input:
 *      unit    - unit id
 *      port    - port id
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
dal_cypress_port_10gSds_restart(uint32 unit, rtk_port_t port)
{
    uint32  speed;
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d,port=%d",unit, port);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(!HAL_IS_SERDES_10GE_PORT(unit, port), RT_ERR_PORT_ID);

    if (HAL_GET_CHIP_REV_ID(unit) < CHIP_REV_ID_C)
        return RT_ERR_OK;

    if ((ret = phy_8390_speed_get(unit, port, &speed)) != RT_ERR_OK)
        return ret;

    if (PORT_SPEED_10G != speed)
        return RT_ERR_OK;

    PORT_SEM_LOCK(unit);

    phy_8390_10gSds_restart(unit, port);

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /* end of dal_cypress_port_10gSds_restart */

/* Function Name:
 *      dal_cypress_port_10g_init
 * Description:
 *      Init 10G port
 * Input:
 *      unit    - unit id
 *      port    - port id
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
dal_cypress_port_10g_init(uint32 unit, rtk_port_t port)
    {
    uint32  speed;
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d,port=%d", unit, port);

    /* check Init status */
    RT_INIT_CHK(port_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(!HAL_IS_SERDES_10GE_PORT(unit, port), RT_ERR_PORT_ID);

    if (HAL_GET_CHIP_REV_ID(unit) <= CHIP_REV_ID_C)
        return RT_ERR_OK;

    if ((ret = phy_8390_speed_get(unit, port, &speed)) != RT_ERR_OK)
        return ret;

    if (PORT_SPEED_10G != speed)
        return RT_ERR_OK;

    /* function body */
    PORT_SEM_LOCK(unit);

    phy_8390_serdes_10g_leq_init(unit, port);

    PORT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /* end of dal_cypress_port_10g_init */
