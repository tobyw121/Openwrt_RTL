/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * $Revision: 48300 $
 * $Date: 2014-06-06 10:35:15 +0800 (Fri, 06 Jun 2014) $
 *
 * Purpose : Use to Management each device
 *
 * Feature : The file have include the following module and sub-modules
 *           1) Initialize system
 *           2) Initialize device
 *           3) Mangement Devices
 *
 */

/*
 * Include Files
 */
#include <common/rt_autoconf.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/debug/rt_log.h>
//#include <common/util/rt_bitop.h>
//#include <common/util/rt_util.h>
#include <osal/sem.h>
#include <osal/lib.h>
#include <osal/memory.h>
#include <osal/thread.h>

#include <hal/common/halctrl.h>
#include <dal/esw/dal_esw_l2.h>
#include <dal/esw/dal_esw_port.h>
#include <dal/esw/dal_esw_eee.h>
#include <dal/esw/dal_esw_led.h>
#include <dal/esw/dal_esw_switch.h>
#include <dal/maple/dal_maple_led.h>
#include <dal/dal_common.h>
#include <dal/dal_waMon.h>
#include <rtk/default.h>
#include <dal/maple/dal_maple_port.h>
#include <dal/cypress/dal_cypress_port.h>
#include <hal/common/miim.h>

#if defined(CONFIG_SDK_WA_SERDES_FIBER_LINKDOWN_WATCHDOG)
#include <hal/phy/phy_8390.h>
#endif

/*
 * Symbol Definition
 */
/* workaround monitor control block */
typedef struct dal_waMon_cb_s {
    osal_thread_t       thread_id;
    uint32              scan_interval_us;
} dal_waMon_cb_t;

/*
 * Data Declaration
 */
static uint32   waMon_init;
static uint32   wa_change_sem;
static dal_waMon_cb_t   *pWaMon_cb;
rtk_port_phyReconfig_callback_t pPhyReconfig_cb;

uint32 pktBuf_watchdog_cnt = 0;
uint32 macSerdes_watchdog_cnt = 0;
uint32 phy_watchdog_cnt = 0;
uint32 fiber_rx_watchdog_cnt = 0;

/*
 * Macro Declaration
 */

/*
 * Function Declaration
 */

static void _dal_waMon_thread(void *pInput);


/* Module Name : */

/* Function Name:
 *      dal_waMon_init
 * Description:
 *      Initial Workaround Monitor component
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED - initialize fail
 *      RT_ERR_OK     - initialize success
 * Note:
 *
 */
int32 dal_waMon_init(void)
{

    /* init value */
    waMon_init = INIT_NOT_COMPLETED;

    /* allocate memory for control block */
    pWaMon_cb = osal_alloc(sizeof(dal_waMon_cb_t));

    if (0 == pWaMon_cb){
        RT_LOG(LOG_DEBUG, MOD_DAL, "workaround monitor allocate memory failed");
        return RT_ERR_FAILED;
    }

    /* create semaphore for sync, this semaphore is empty in beginning */
    wa_change_sem = osal_sem_create(0);

    if (0 == wa_change_sem){
        osal_free(pWaMon_cb);
        RT_LOG(LOG_DEBUG, MOD_DAL, "workaround monitor semaphore create failed");
        return RT_ERR_FAILED;
    }

    osal_memset(pWaMon_cb, 0, sizeof(dal_waMon_cb_t));

    waMon_init = INIT_COMPLETED;

    pktBuf_watchdog_cnt = 0;
    macSerdes_watchdog_cnt = 0;
    phy_watchdog_cnt = 0;
    fiber_rx_watchdog_cnt = 0;


    return RT_ERR_OK;

} /* end of dal_waMon_init */

/* Function Name:
 *      dal_waMon_enable
 * Description:
 *      Enable workaround monitor thread
 * Input:
 *      scan_interval_us        - scan interval in us.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - initialize success
 *      RT_ERR_FAILED   - initialize fail
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - scan interval is too small
 * Note:
 *      When enable workaround monitor thread
 *
 */
int32 dal_waMon_enable(uint32 scan_interval_us)
{
    uint32  unit = 0;
    hal_control_t *pHal_ctrl;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "scan_interval_us=%u",
           scan_interval_us);

    /* check Init status */
    RT_INIT_CHK(waMon_init);

    /* parameter check */
    RT_PARAM_CHK((scan_interval_us < RTK_WA_SCAN_INTERVAL_MIN), RT_ERR_OUT_OF_RANGE);

    pWaMon_cb->scan_interval_us = scan_interval_us;

    if ((pWaMon_cb->thread_id) != 0)
    {
        RT_ERR(pWaMon_cb->thread_id, (MOD_DAL|MOD_PORT), "");
        return RT_ERR_THREAD_EXIST;
    }

    /* Check whether device is exist in lower layer(HAL) */
    if ((pHal_ctrl = hal_ctrlInfo_get(unit)) == NULL)
    {
        return RT_ERR_FAILED;
    }


    if ((RTL8328M_CHIP_ID == pHal_ctrl->chip_id || RTL8328S_CHIP_ID == pHal_ctrl->chip_id || RTL8328L_CHIP_ID == pHal_ctrl->chip_id) ||
        (HAL_IS_RTL8390_FAMILY_ID(unit) || HAL_IS_RTL8350_FAMILY_ID(unit)) ||
        (RTL8332M_CHIP_ID == pHal_ctrl->chip_id || RTL8330M_CHIP_ID == pHal_ctrl->chip_id) ||
        (RTL8382M_CHIP_ID == pHal_ctrl->chip_id || RTL8380M_CHIP_ID == pHal_ctrl->chip_id))
    {
        /* create thread */
        pWaMon_cb->thread_id = osal_thread_create("WA Monitor Thread", RTK_DEFAULT_WA_MON_STACK_SIZE, RTK_DEFAULT_WA_MON_THREAD_PRI
                                , (void *)_dal_waMon_thread, NULL);

        if (0 == (pWaMon_cb->thread_id))
        {
            RT_ERR(pWaMon_cb->thread_id, (MOD_DAL|MOD_PORT), "");
            return RT_ERR_THREAD_CREATE_FAILED;
        }
    }

    return RT_ERR_OK;
} /* end of dal_waMon_enable */

/* Function Name:
 *      dal_waMon_disable
 * Description:
 *      Disable workaround scan thread
 * Input:
 *      None.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - initialize success
 *      RT_ERR_FAILED   - initialize fail
 *      RT_ERR_NOT_INIT - The module is not initial
 * Note:
 *      When disable workaround monitor thread
 */
int32 dal_waMon_disable(void)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "");

    /* check Init status */
    RT_INIT_CHK(waMon_init);

    /* parameter check */

    /* reset scan_interval_us to 0, thread will suicide after finish all waiting job */
    pWaMon_cb->scan_interval_us = 0;

    /* let thread continue */
    osal_sem_give(wa_change_sem);

    return RT_ERR_OK;

} /* end of dal_waMon_disable */

/* Function Name:
 *      _dal_waMon_thread
 * Description:
 *      Unregister callback function for link change notification
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
static void _dal_waMon_thread(void *pInput)
{
    uint32  unit = 0;
    hal_control_t *pHal_ctrl;

    /* Check whether device is exist in lower layer(HAL) */
    if ((pHal_ctrl = hal_ctrlInfo_get(unit)) == NULL)
    {
        return;
    }

    /* forever loop */
    while (pWaMon_cb->scan_interval_us != 0)
    {

        /* wait semaphore for workaround scan interval */
        osal_sem_take(wa_change_sem, pWaMon_cb->scan_interval_us);

#if defined(CONFIG_SDK_RTL8328)
        if ((RTL8328M_CHIP_ID == pHal_ctrl->chip_id || RTL8328S_CHIP_ID == pHal_ctrl->chip_id || RTL8328L_CHIP_ID == pHal_ctrl->chip_id))
        {
            if (CHIP_REV_ID_A == pHal_ctrl->chip_rev_id)
            {
#if defined(CONFIG_SDK_WA_INTRALINK_DELAY)
                dal_esw_port_intraLinkDelay_workaround(unit);
#endif

#if defined(CONFIG_SDK_WA_LINKDOWN_PWR_SAVING)
                /* Scan phy register(port0-15) for link down power saving problem in RTL8328M/RTL8328S
                 * 1) Check signal form PHY page0, register 1, bit2, if TRUE, disable link down power saving mode
                 * 2) delay 800ms (value is provided by HW)
                 * 3) enable the link down power saving mode
                 */
                dal_esw_port_linkdownPowerSaving_workaround(unit);
#endif

#if defined(CONFIG_SDK_WA_BACK_PRESSURE)
                /* Patch for half-duplex [back pressure] problem in RTL8328M/RTL8328S
                 * 1) Check the port have meet the condition or not?
                 *    - if linkup + half-duplex + enbkprs + auto-neg, add into the scan_portmask
                 *    - otherwise remove from the scan_portmask
                 * 2) Enable the MAC force mode, and start software polling PHY mechanism
                 * 3) Depend on PHY polling result and update the MAC port property register
                 * 4) The port link status register will be updated after filling port property register
                 */
                dal_esw_port_backpressure_workaround(unit);
#endif
            }

#if defined(CONFIG_SDK_WA_EEE_COMPATIBLE)
            /* Patch the eee compatible with Marvell chip problem in RTL8208D
             * 1) 100half mode is not supported the eee function: thread to detect linkup 100half case,
             *    then disable eee and re-Nway again.
             * 2) enable eee and re-Nway back if linkup is not 100half.
             */
            dal_esw_eee_compatible_workaround(unit);
#endif

#if defined(CONFIG_SDK_WA_RTL8231_RESET)
            dal_esw_led_8231Reset_workaround(unit);
#endif
#if defined(CONFIG_SDK_RTL8328)
#if defined(CONFIG_SDK_WA_PKTBUF_WATCHDOG)
            if ((CHIP_REV_ID_A == pHal_ctrl->chip_rev_id || CHIP_REV_ID_B == pHal_ctrl->chip_rev_id || CHIP_REV_ID_C == pHal_ctrl->chip_rev_id))
            {
                dal_esw_switch_pktbuf_watchdog(unit);
            }
#endif
#endif
    }
#endif


#if defined(CONFIG_SDK_RTL8380)
        if (RTL8332M_CHIP_ID == pHal_ctrl->chip_id || RTL8330M_CHIP_ID == pHal_ctrl->chip_id)
        {
#if defined(CONFIG_SDK_WA_RTL833X_COMBO_LED)
            if (CHIP_REV_ID_A == pHal_ctrl->chip_rev_id)
            {
                /*Only RTL833X A-Cut needs this section*/
                dal_maple_led_comboPort_workaround(unit);
            }
#endif
        }

        /*For RTL838X & RTL833X*/
        if (RTL8382M_CHIP_ID == pHal_ctrl->chip_id || RTL8380M_CHIP_ID == pHal_ctrl->chip_id || \
            RTL8332M_CHIP_ID == pHal_ctrl->chip_id || RTL8330M_CHIP_ID == pHal_ctrl->chip_id  )
        {
#if defined(CONFIG_SDK_WA_PKTBUF_WATCHDOG)
            dal_maple_port_pktbuf_watchdog(unit);
#endif
#if defined(CONFIG_SDK_WA_SERDES_WATCHDOG)
            dal_maple_port_serdes_watchdog(unit);
#endif
#if defined(CONFIG_SDK_WA_PHY_WATCHDOG)
            dal_maple_port_phy_watchdog(unit);
#endif
#if defined(CONFIG_SDK_WA_FIBER_RX_WATCHDOG)
            dal_maple_port_fiber_rx_watchdog(unit);
#endif

#if defined(CONFIG_SDK_WA_PKTBUF_WATCHDOG) || \
      defined(CONFIG_SDK_WA_SERDES_WATCHDOG) ||\
      defined(CONFIG_SDK_WA_PHY_WATCHDOG)
             {
                    dal_maple_port_watchdog_debug(unit);
              }
#endif

#if defined(CONFIG_SDK_WA_COMBO_FLOWCONTROL)
            /*For RTL83X & RTL833X combo-port*/
            dal_maple_port_comboPort_workaround(unit);
#endif
        }
#endif


#if defined(CONFIG_SDK_RTL8390)
        /*For RTL839X & RTL835X*/
        if (HAL_IS_RTL8390_FAMILY_ID(unit) || HAL_IS_RTL8350_FAMILY_ID(unit))
        {
#if defined(CONFIG_SDK_WA_PKTBUF_WATCHDOG)
            dal_cypress_port_pktbuf_watchdog(unit);
#endif
#if defined(CONFIG_SDK_WA_PHY_WATCHDOG)
            dal_cypress_port_phy_watchdog(unit);
#endif
#if defined(CONFIG_SDK_WA_SERDES_WATCHDOG)
            dal_cypress_port_serdes_watchdog(unit);
#endif
#if defined(CONFIG_SDK_WA_FIBER_RX_WATCHDOG)
            dal_cypress_port_fiber_rx_watchdog(unit);
#endif
#if defined(CONFIG_SDK_WA_SERDES_FIBER_LINKDOWN_WATCHDOG)
            if (HAL_IS_RTL8390_FAMILY_ID(unit) && \
                    HAL_GET_CHIP_REV_ID(unit) < CHIP_REV_ID_C && \
                    HAL_GET_SERDES_PORT_NUM(unit) != 0)
                phy_8390_linkdown_watchdog(unit);
#endif

        }
#endif

    }

    osal_thread_exit(0);

    return;
} /* end of _dal_waMon_thread */

/* Function Name:
 *      dal_waMon_phyReconfig_register
 * Description:
 *      Register callback function for PHY need to reconfigure notification
 * Input:
 *      phyReconfig_callback    - callback function for reconfigure notification
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED   - initialize fail
 *      RT_ERR_OK       - initialize success
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *
 */
int32 dal_waMon_phyReconfig_register(rtk_port_phyReconfig_callback_t phyReconfig_callback)
{

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "");

    /* check Init status */
    RT_INIT_CHK(waMon_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == phyReconfig_callback), RT_ERR_NULL_POINTER);

    pPhyReconfig_cb = phyReconfig_callback;

    return RT_ERR_OK;
} /* end of dal_waMon_phyReconfig_register */

/* Function Name:
 *      dal_waMon_phyReconfig_unregister
 * Description:
 *      UnRegister callback function for PHY need to reconfigure notification
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED   - initialize fail
 *      RT_ERR_OK       - initialize success
 *      RT_ERR_NOT_INIT - The module is not initial
 * Note:
 *
 */
int32 dal_waMon_phyReconfig_unregister(void)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "");

    /* check Init status */
    RT_INIT_CHK(waMon_init);

    pPhyReconfig_cb = NULL;

    return RT_ERR_OK;
} /* end of dal_waMon_phyReconfig_unregister */

/* Function Name:
 *      dal_waMon_phyReconfig_portMaskSet
 * Description:
 *      Send the need ReConfig port mask back to user.
 * Input:
 *      phyReconfig_callback    - callback function for link change
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED   - initialize fail
 *      RT_ERR_OK       - initialize success
 *      RT_ERR_NOT_INIT - The module is not initial
 * Note:
 *
 */
int32 dal_waMon_phyReconfig_portMaskSet(uint32 unit, rtk_port_t port)
{
    hal_control_t *pHal_ctrl;
    rtk_portmask_t portMask;
    int loop;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "");

    osal_memset(&portMask, 0, sizeof(rtk_portmask_t));

    /* Check whether device is exist in lower layer(HAL) */
    if ((pHal_ctrl = hal_ctrlInfo_get(unit)) == NULL)
    {
        return RT_ERR_FAILED;
    }

    /* check Init status */
    RT_INIT_CHK(waMon_init);

    /*For RTL839X & RTL835X*/
    if (HAL_IS_RTL8390_FAMILY_ID(unit) || HAL_IS_RTL8350_FAMILY_ID(unit))
    {
        if(port != 48){
            for(loop = port; loop < (port + 8); loop++)
                RTK_PORTMASK_PORT_SET(portMask,loop);
        }else{
            for(loop = port; loop < (port + 4); loop++)
                RTK_PORTMASK_PORT_SET(portMask,loop);
        }
    }

    /*For RTL838X & RTL833X*/
    if (RTL8332M_CHIP_ID == pHal_ctrl->chip_id || RTL8330M_CHIP_ID == pHal_ctrl->chip_id || \
        RTL8382M_CHIP_ID == pHal_ctrl->chip_id || RTL8380M_CHIP_ID == pHal_ctrl->chip_id)
    {
              if((0==(port%8)) && (port < 28))
              {
                if(port != 24)
                    {
                    for(loop = port; loop < (port + 8); loop++)
                    {
                        RTK_PORTMASK_PORT_SET(portMask,loop);
                    }
                }
                     else
                {
                    for(loop = port; loop < (port + 4); loop++)
                    {
                        RTK_PORTMASK_PORT_SET(portMask,loop);
                    }
                }
              }
    }


    if(pPhyReconfig_cb != NULL)
        (pPhyReconfig_cb)(0, &portMask);
    else
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "No CallBack for port %d\n",port);

    return RT_ERR_OK;
} /* end of dal_waMon_phyReconfig_portMaskSet */

