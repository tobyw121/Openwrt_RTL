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
 * $Revision: 55230 $
 * $Date: 2015-01-26 21:56:17 +0800 (Mon, 26 Jan 2015) $
 *
 * Purpose : Realtek Switch SDK Core Module.
 *
 * Feature : Realtek Switch SDK Core Module
 *
 */

/*
 * Include Files
 */
#include <common/rt_autoconf.h>
#include <common/debug/rt_log.h>
#include <ioal/ioal_init.h>
#include <drv/nic/probe.h>
#include <drv/watchdog/probe.h>
#include <drv/swcore/chip.h>
#if defined(CONFIG_SDK_RTL8231)
#include <drv/rtl8231/probe.h>
#endif
#include <osal/isr.h>
#include <dev_config.h>
#include <drv/intr/intr.h>
#if defined(CONFIG_SDK_RTL8389)
#include <drv/swcore/rtl8389.h>
#endif
#if defined(CONFIG_SDK_RTL8328)
#include <drv/swcore/rtl8328.h>
#endif
#if defined(CONFIG_SDK_RTL8390)
#include <drv/swcore/rtl8390.h>
#include <drv/swcore/l2notification.h>
#endif
#include <rtcore/rtcore.h>
#include <ioal/mem32.h>
#if defined(CONFIG_SDK_UART1)
#include <drv/uart/probe.h>
#endif
#include <drv/smi/smi.h>

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */

extern uint32 chip_id_updated_flag;

/*
 * Macro Declaration
 */

/*
 * Function Declaration
 */

#if 0
static uint32 sw_intr_status = 0;
static uint32 sw_sisr0 = 0;
static uint32 sw_sisr1 = 0;

osal_isrret_t
_sw_intr_handler(void *isr_param)
{  
    rtcore_ioctl_t  dio;

#if defined(CONFIG_SDK_RTL8389)
    ioal_mem32_read(0, RTL8389_SWITCH_INTERRUPT_SOURCE_STATUS_ADDR, &sw_intr_status);
    ioal_mem32_write(0, RTL8389_SWITCH_INTERRUPT_CONTROL0_ADDR, (0x00000000U));
    
    ioal_mem32_read(0, RTL8389_SWITCH_INTERRUPT_STATUS0_ADDR, &sw_sisr0);
	ioal_mem32_write(0, RTL8389_SWITCH_INTERRUPT_STATUS0_ADDR, sw_sisr0);
#elif defined(CONFIG_SDK_RTL8328)
    ioal_mem32_read(0, RTL8328_SWITCH_INTERRUPT_GLOBAL_SOURCE_STATUS_ADDR, &sw_intr_status);
    ioal_mem32_write(0, RTL8328_PER_PORT_LINK_CHANGE_INTERRUPT_CONTROL_ADDR, (0x00000000U));

    ioal_mem32_read(0, RTL8328_PER_PORT_LINK_CHANGE_INTERRUPT_STATUS_ADDR, &sw_sisr0);
	ioal_mem32_write(0, RTL8328_PER_PORT_LINK_CHANGE_INTERRUPT_STATUS_ADDR, sw_sisr0);
#elif defined(CONFIG_SDK_RTL8390)
    /* Get the interrupt source */
    ioal_mem32_read(0, RTL8390_ISR_GLB_SRC_ADDR, &sw_intr_status);
    /* Mask off the link status change interrupt */
	ioal_mem32_write(0, RTL8390_IMR_PORT_LINK_STS_CHG_ADDR(0), (0x00000000U));
	ioal_mem32_write(0, RTL8390_IMR_PORT_LINK_STS_CHG_ADDR(32), (0x00000000U));

    /* Get the interrupt pending status */
	ioal_mem32_read(0, RTL8390_ISR_PORT_LINK_STS_CHG_ADDR(0), &sw_sisr0);
	ioal_mem32_read(0, RTL8390_ISR_PORT_LINK_STS_CHG_ADDR(32), &sw_sisr1);
    /* Clear the interrupt pending status */
	ioal_mem32_write(0, RTL8390_ISR_PORT_LINK_STS_CHG_ADDR(0), sw_sisr0);
	ioal_mem32_write(0, RTL8390_ISR_PORT_LINK_STS_CHG_ADDR(32), sw_sisr1);
#endif

    dio.data[0] = 0;
    dio.data[1] = INTR_TYPE_SWCORE;
    dio.data[2] = sw_intr_status;
    dio.data[3] = sw_sisr0;
    dio.data[4] = sw_sisr1;

    drv_intr_swcore_handler(&dio);

    return OSAL_INT_HANDLED;
} /* end of _nic_isr_handler */ 
#endif

/* Function Name:
 *      rtcore_init
 * Description:
 *      Initialize RTCORE module with the specified device
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      1. INIT must be initialized before using all of APIs in each modules
 */
int32
rtcore_init(uint32 unit)
{
    int32 ret = RT_ERR_FAILED;
#if defined(CONFIG_SDK_DRIVER_L2NTFY)
    uint32 chip_family_id;
#endif

    /* Check whether unit id is valid(not out of range) */
    RT_PARAM_CHK((RTK_MAX_UNIT_ID < unit), RT_ERR_UNIT_ID);

    /* Will be used by all layers, Initialize first */
    RT_ERR_CHK(rt_log_init(), ret);

    /* Initialize the ioal layer */
    RT_ERR_CHK(ioal_init(unit), ret);

    chip_id_updated_flag = 0;	

#if !defined(CONFIG_SDK_DRIVER_NIC_USER_MODE)
    /* Probe the nic */
    RT_ERR_CHK(nic_probe(unit), ret);
#endif

    /* Probe the watchdog */
    RT_ERR_CHK(watchdog_probe(unit), ret);
    RT_ERR_CHK(drv_watchdog_init(unit), ret);

#if defined(CONFIG_SDK_RTL8231)
    RT_ERR_CHK(rtl8231_probe(unit), ret);
    RT_ERR_CHK(drv_rtl8231_init(unit), ret);
#endif

#if defined(CONFIG_SDK_DRIVER_L2NTFY)
    drv_swcore_family_cid_get(0, &chip_family_id);
    if ((chip_family_id ==  RTL8390_FAMILY_ID)|| (chip_family_id ==  RTL8350_FAMILY_ID))
    {
    RT_ERR_CHK(drv_swcore_l2_notification_init(unit, RTL8390_L2NOTIFY_RING_SIZE), ret);
    }
#endif
#if 0
    /* Add the kernel mode swcore link change interrupt hook */
    RT_ERR_CHK(drv_intr_init(unit), ret);
    RT_ERR_CHK(osal_isr_register(RTK_DEV_SWCORE, _sw_intr_handler, NULL), ret);
    /* The following line is for testing, and user need to call by itself */
    //RT_ERR_CHK(drv_intr_enable_set(unit, LINK_CHANGE_INTR), ret);
#endif
#if defined(CONFIG_SDK_UART1)
    RT_ERR_CHK(uart_probe(unit), ret);
    RT_ERR_CHK(drv_uart_init(unit), ret);
#endif
	RT_ERR_CHK(drv_smi_module_init(unit), ret);
	
    return ret;
} /* end of rtcore_init */

