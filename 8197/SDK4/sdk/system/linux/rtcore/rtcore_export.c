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
 * $Revision: 48962 $
 * $Date: 2014-07-01 10:17:05 +0800 (Tue, 01 Jul 2014) $
 *
 * Purpose : Realtek Switch SDK Core Module.
 *
 * Feature : Realtek Switch SDK Core Module
 *
 */

/*
 * Include Files
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <dev_config.h>
#include <common/debug/rt_log.h>
#include <common/debug/mem.h>
#include <osal/cache.h>
#include <osal/isr.h>
#include <osal/memory.h>
#include <osal/print.h>
#include <osal/time.h>
#include <osal/sem.h>
#include <osal/thread.h>
#include <osal/spl.h>
#include <osal/atomic.h>
#include <ioal/mem32.h>
#include <drv/watchdog/watchdog.h>
#include <drv/swcore/chip.h>
#if defined(CONFIG_SDK_DRIVER_NIC_KERNEL_MODE)
#include <drv/nic/nic.h>
#include <drv/nic/r8389.h>
#endif
#if defined(CONFIG_SDK_UART1)
#include <drv/uart/uart.h>
#endif
#if defined(CONFIG_SDK_BSP_SMI)
#include <drv/smi/smi.h>
#include <drv/smi/ext_smi.h>
#include <drv/gpio/gpio.h>
#include <drv/gpio/generalCtrl_gpio.h>
#endif            
#if defined(CONFIG_SDK_RTL8231)
#include <drv/rtl8231/rtl8231.h>
#include <drv/gpio/ext_gpio.h>
#endif /* CONFIG_SDK_RTL8231 */

#if defined(CONFIG_SDK_MODEL_MODE)
#include <ioal/ioal_init.h>
#include <virtualmac/vmac_target.h>
#endif /*end of #if defined(CONFIG_SDK_MODEL_MODE)*/

#if defined(CONFIG_SDK_RTL8390)
#include <drv/swcore/l2notification.h>
#endif /* CONFIG_SDK_RTL8390 */

/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */


/*
 * Macro Declaration
 */

/*
 * Function Declaration
 */

/* BSP functions */
#if defined(CONFIG_SDK_BSP_SMI)
EXPORT_SYMBOL(drv_smi_type_get);
EXPORT_SYMBOL(drv_smi_type_set);
EXPORT_SYMBOL(drv_smi_write);
EXPORT_SYMBOL(drv_smi_read);
EXPORT_SYMBOL(drv_smi_init);
EXPORT_SYMBOL(drv_smi_group_get);
EXPORT_SYMBOL(drv_gpio_init);
EXPORT_SYMBOL(drv_gpio_dataBit_init);
EXPORT_SYMBOL(drv_gpio_dataBit_get);
EXPORT_SYMBOL(drv_gpio_dataBit_set);
EXPORT_SYMBOL(drv_gpio_isr_get);
EXPORT_SYMBOL(drv_gpio_isr_clear);
EXPORT_SYMBOL(drv_smi_slavePresent);
EXPORT_SYMBOL(drv_gpio_intrHandler_register);
EXPORT_SYMBOL(drv_gpio_intrHandler_unregister);
EXPORT_SYMBOL(drv_generalCtrlGPIO_dev_init);
EXPORT_SYMBOL(drv_generalCtrlGPIO_devEnable_set);
EXPORT_SYMBOL(drv_generalCtrlGPIO_pin_init);
EXPORT_SYMBOL(drv_generalCtrlGPIO_dataBit_set);
EXPORT_SYMBOL(drv_generalCtrlGPIO_dataBit_get);
#endif /* CONFIG_SDK_BSP_SMI */

EXPORT_SYMBOL(rtk_dev);
EXPORT_SYMBOL(drv_watchdog_mode_set);
EXPORT_SYMBOL(drv_watchdog_mode_get);
EXPORT_SYMBOL(drv_watchdog_scale_set);
EXPORT_SYMBOL(drv_watchdog_scale_get);
EXPORT_SYMBOL(drv_watchdog_enable_set);
EXPORT_SYMBOL(drv_watchdog_enable_get);
EXPORT_SYMBOL(drv_watchdog_kick);
EXPORT_SYMBOL(drv_watchdog_threshold_set);
EXPORT_SYMBOL(drv_watchdog_threshold_get);

EXPORT_SYMBOL(debug_mem_read);
EXPORT_SYMBOL(debug_mem_write);

EXPORT_SYMBOL(rt_log_init);
EXPORT_SYMBOL(rt_log);
EXPORT_SYMBOL(rt_log_reset);
EXPORT_SYMBOL(rt_log_enable_get);
EXPORT_SYMBOL(rt_log_enable_set);
EXPORT_SYMBOL(rt_log_level_get);
EXPORT_SYMBOL(rt_log_level_set);
EXPORT_SYMBOL(rt_log_level_reset);
EXPORT_SYMBOL(rt_log_mask_get);
EXPORT_SYMBOL(rt_log_mask_set);
EXPORT_SYMBOL(rt_log_mask_reset);
EXPORT_SYMBOL(rt_log_type_get);
EXPORT_SYMBOL(rt_log_type_set);
EXPORT_SYMBOL(rt_log_type_reset);
EXPORT_SYMBOL(rt_log_format_get);
EXPORT_SYMBOL(rt_log_format_set);
EXPORT_SYMBOL(rt_log_format_reset);
EXPORT_SYMBOL(rt_log_moduleMask_get);
EXPORT_SYMBOL(rt_log_moduleMask_set);
EXPORT_SYMBOL(rt_log_moduleMask_reset);
EXPORT_SYMBOL(rt_log_moduleName_get);
EXPORT_SYMBOL(rt_log_config_get);

#if defined(CONFIG_SDK_UART1)
EXPORT_SYMBOL(drv_uart_clearfifo);
EXPORT_SYMBOL(drv_uart_getc);
EXPORT_SYMBOL(drv_uart_gets);
EXPORT_SYMBOL(drv_uart_putc);
EXPORT_SYMBOL(drv_uart_puts);
EXPORT_SYMBOL(drv_uart_baudrate_get);
EXPORT_SYMBOL(drv_uart_baudrate_set);
#endif

EXPORT_SYMBOL(osal_sem_create);
EXPORT_SYMBOL(osal_sem_destroy);
EXPORT_SYMBOL(osal_sem_take);
EXPORT_SYMBOL(osal_sem_give);
EXPORT_SYMBOL(osal_sem_mutex_create);
EXPORT_SYMBOL(osal_sem_mutex_destroy);
EXPORT_SYMBOL(osal_sem_mutex_take);
EXPORT_SYMBOL(osal_sem_mutex_give);

EXPORT_SYMBOL(osal_alloc);
EXPORT_SYMBOL(osal_free);

EXPORT_SYMBOL(osal_time_usec2Ticks_get);
EXPORT_SYMBOL(osal_time_seconds_get);
EXPORT_SYMBOL(osal_time_usecs_get);
EXPORT_SYMBOL(osal_time_udelay);
EXPORT_SYMBOL(osal_time_usleep);
EXPORT_SYMBOL(osal_time_sleep);
EXPORT_SYMBOL(osal_time_mdelay);

EXPORT_SYMBOL(osal_cache_memory_flush);
EXPORT_SYMBOL(osal_isr_register);
EXPORT_SYMBOL(osal_isr_unregister);

EXPORT_SYMBOL(osal_thread_destroy);
EXPORT_SYMBOL(osal_thread_name);
EXPORT_SYMBOL(osal_thread_exit);
EXPORT_SYMBOL(osal_thread_create);
EXPORT_SYMBOL(osal_thread_self);

EXPORT_SYMBOL(ioal_mem32_check);
EXPORT_SYMBOL(ioal_mem32_read);
EXPORT_SYMBOL(ioal_mem32_write);
EXPORT_SYMBOL(ioal_mem32_field_read);
EXPORT_SYMBOL(ioal_mem32_field_write);

EXPORT_SYMBOL(osal_spl_spin_unlock);
EXPORT_SYMBOL(osal_spl_spin_lock);

EXPORT_SYMBOL(osal_atomic_inc);
EXPORT_SYMBOL(osal_atomic_set);
EXPORT_SYMBOL(osal_atomic_dec_return);
EXPORT_SYMBOL(osal_atomic_read);

EXPORT_SYMBOL(drv_swcore_cid_get);
EXPORT_SYMBOL(drv_swcore_cid_cmp);
EXPORT_SYMBOL(drv_swcore_sysMac_get);
EXPORT_SYMBOL(drv_swcore_sysMac_set);
EXPORT_SYMBOL(drv_swcore_CPU_freq_get);
EXPORT_SYMBOL(drv_swcore_register_dump);


#if defined(CONFIG_SDK_RTL8390)
#if !defined(__MODEL_USER__) && defined(CONFIG_SDK_KERNEL_LINUX_KERNEL_MODE)
EXPORT_SYMBOL(drv_swcore_l2_notification_register);
#endif
EXPORT_SYMBOL(drv_swcore_l2_notification_debug_set);
EXPORT_SYMBOL(drv_swcore_l2_notification_dump_counter);
EXPORT_SYMBOL(drv_swcore_l2_notification_dump);
#endif

#if defined(CONFIG_SDK_DRIVER_NIC_KERNEL_MODE)
EXPORT_SYMBOL(drv_nic_init);
EXPORT_SYMBOL(drv_nic_pkt_tx);
EXPORT_SYMBOL(drv_nic_rx_start);
EXPORT_SYMBOL(drv_nic_rx_stop);
EXPORT_SYMBOL(drv_nic_rx_register);
EXPORT_SYMBOL(drv_nic_rx_unregister);
EXPORT_SYMBOL(drv_nic_dbg_get);
EXPORT_SYMBOL(drv_nic_dbg_set);
EXPORT_SYMBOL(drv_nic_cntr_clear);
EXPORT_SYMBOL(drv_nic_cntr_dump);
EXPORT_SYMBOL(drv_nic_ringbuf_dump);
EXPORT_SYMBOL(drv_nic_pkthdr_mbuf_dump);
EXPORT_SYMBOL(drv_nic_rx_status_get);
EXPORT_SYMBOL(drv_nic_pkt_alloc);
EXPORT_SYMBOL(drv_nic_pkt_free);
EXPORT_SYMBOL(drv_nic_reset);
#if defined(CONFIG_SDK_SOFTWARE_RX_CPU_TAG)
EXPORT_SYMBOL(drv_nic_pieCpuEntry_add);
EXPORT_SYMBOL(drv_nic_pieCpuEntry_del);
EXPORT_SYMBOL(drv_nic_pieCpuEntry_set);
EXPORT_SYMBOL(drv_nic_pieCpuEntry_get);
#endif
#endif /* CONFIG_SDK_DRIVER_NIC_KERNEL_MODE */

#if defined(CONFIG_SDK_RTL8231)
EXPORT_SYMBOL(drv_rtl8231_i2c_read);
EXPORT_SYMBOL(drv_rtl8231_i2c_write);
EXPORT_SYMBOL(drv_rtl8231_mdc_read);
EXPORT_SYMBOL(drv_rtl8231_mdc_write);
EXPORT_SYMBOL(drv_rtl8231_mdcSem_register);
EXPORT_SYMBOL(drv_rtl8231_extra_devReady_get);
EXPORT_SYMBOL(drv_rtl8231_extra_devEnable_get);
EXPORT_SYMBOL(drv_rtl8231_extra_devEnable_set);
EXPORT_SYMBOL(drv_rtl8231_extra_dataBit_get);
EXPORT_SYMBOL(drv_rtl8231_extra_dataBit_set);
EXPORT_SYMBOL(drv_rtl8231_extra_direction_get);
EXPORT_SYMBOL(drv_rtl8231_extra_direction_set);
EXPORT_SYMBOL(drv_extGpio_dataBit_get);
EXPORT_SYMBOL(drv_extGpio_dataBit_set);
EXPORT_SYMBOL(drv_extGpio_sync_start);
EXPORT_SYMBOL(drv_extGpio_syncEnable_get);
EXPORT_SYMBOL(drv_extGpio_devEnable_set);
EXPORT_SYMBOL(drv_extGpio_pin_get);
EXPORT_SYMBOL(drv_extGpio_pin_init);
EXPORT_SYMBOL(drv_extGpio_devReady_get);
EXPORT_SYMBOL(drv_extGpio_devEnable_get);
EXPORT_SYMBOL(drv_extGpio_syncStatus_get);
EXPORT_SYMBOL(drv_extGpio_syncEnable_set);
EXPORT_SYMBOL(drv_extGpio_dev_get);
EXPORT_SYMBOL(drv_extGpio_dev_init);
EXPORT_SYMBOL(drv_extGpio_devRecovery_start);
EXPORT_SYMBOL(drv_extGpio_reg_read);
EXPORT_SYMBOL(drv_extGpio_reg_write);
EXPORT_SYMBOL(drv_extGpio_direction_get);
EXPORT_SYMBOL(drv_extGpio_direction_set);
EXPORT_SYMBOL(drv_extGpio_i2c_init);
EXPORT_SYMBOL(drv_extGpio_i2c_read);
EXPORT_SYMBOL(drv_extGpio_i2c_write);
EXPORT_SYMBOL(drv_extSmi_dev_init);
EXPORT_SYMBOL(drv_extSmi_write);
EXPORT_SYMBOL(drv_extSmi_read);
#endif /* CONFIG_SDK_RTL8231 */

#if defined(CONFIG_SDK_MODEL_MODE)
EXPORT_SYMBOL(ioal_init);
EXPORT_SYMBOL(pVirtualSWTable);
EXPORT_SYMBOL(vmac_setCaredICType);
EXPORT_SYMBOL(vmac_getTarget);
EXPORT_SYMBOL(vmac_setTarget);
EXPORT_SYMBOL(vmac_setRegAccessType);
EXPORT_SYMBOL(vmac_getRegAccessType);
#endif /*end of #if defined(CONFIG_SDK_MODEL_MODE) */
