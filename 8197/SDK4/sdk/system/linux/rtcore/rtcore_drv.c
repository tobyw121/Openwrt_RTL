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
#include <linux/version.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>           
#include <linux/slab.h>             
#include <linux/fs.h>              
#include <linux/errno.h>           
#include <linux/types.h>            
#include <linux/mm.h>
#include <linux/kdev_t.h>
#include <asm/io.h>
#include <asm/page.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <common/rt_autoconf.h>
#include <common/debug/rt_log.h>
#include <common/rtcore/init.h>
#include <osal/sem.h>
#include <osal/cache.h>
#include <drv/swcore/chip.h>
#include <drv/nic/diag.h>
#include <drv/nic/nic.h>
#include <rtcore/rtcore.h>
#include <drv/gpio/gpio.h>
#include <drv/smi/smi.h>
#include <drv/watchdog/watchdog.h>
#if defined(CONFIG_SDK_DRIVER_NIC_USER_MODE)
#include <dev_config.h>
#include <rtcore/user/rtcore_drv_usr.h>
#include <drv/swcore/rtl8389.h>
#include <drv/swcore/rtl8328.h>
#include <drv/swcore/rtl8390.h>
#include <drv/swcore/rtl8380.h>
#include <ioal/mem32.h>
#include <osal/isr.h>
#include <drv/swcore/l2notification.h>
#include <drv/gpio/generalCtrl_gpio.h>
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,19))
#include <linux/sched.h>
#endif
#endif /* defined(CONFIG_SDK_DRIVER_NIC_USER_MODE) */ 
#include <drv/gpio/ext_gpio.h>
#include <drv/rtl8231/rtl8231.h>
#include <drv/smi/ext_smi.h>
#if defined(CONFIG_SDK_SOFTWARE_CONTROL_LED)
#include <drv/swled/swctrl_led_main.h>
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,19))
#include <platform.h>
#endif
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,19))
#include <bspchip.h>
#endif
#include <dev_config.h>
#include <osal/isr.h>
#endif
#if defined(CONFIG_SDK_UART1)
#include <drv/uart/uart.h>
#endif

/*
 * Symbol Definition
 */
#define RTCORE_DRV_MAJOR            200
#define RTCORE_DRV_NAME             "rtcore"
#define MEM_RESERVED_SIZE           4096
#define SYS_DEFAULT_INIT_UNIT_ID    0

void rtcore_vma_open(struct vm_area_struct *vma);
void rtcore_vma_close(struct vm_area_struct *vma);
static int rtcore_open(struct inode *inode, struct file *file);
static int rtcore_release(struct inode *inode, struct file *file);
static int rtcore_ioctl( struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
static int rtcore_mmap(struct file *filp, struct vm_area_struct *vma);

/*
 * Data Declaration
 */
static const struct file_operations rtcore_fops = {
    .owner		= THIS_MODULE,     
    .open		= rtcore_open,
    .release	= rtcore_release,
    .ioctl		= rtcore_ioctl,
    .mmap       = rtcore_mmap,
};

static struct vm_operations_struct rtcore_remap_vm_ops = {
	.open       = rtcore_vma_open,
	.close      = rtcore_vma_close,
};

typedef struct rtcore_dev_s
{
    void *rt_data;
    struct cdev rt_cdev;
} rtcore_dev_t;

rtcore_dev_t *rtcore_devices;
int rtcore_num = RTCORE_DEV_NUM;

#if defined(CONFIG_SDK_DRIVER_NIC_USER_MODE)
static wait_queue_head_t sw_intr_wait_queue;
static wait_queue_head_t nic_intr_wait_queue;
static atomic_t sw_wait_for_intr;
static atomic_t nic_wait_for_intr;
static uint32 _nic_intr_enabled = DISABLED;
static uint32 _sw_intr_enabled = DISABLED;
static uint32 sw_intr_status = 0;
static uint32 nic_intr_status = 0;
static uint32 sw_sisr0 = 0;
#if defined(CONFIG_SDK_RTL8390)
static uint32 sw_sisr1 = 0;
#endif
uint32  l2NotifyEventCnt = 0;
uint32  curPos = 0;

extern uint32 totalEventCount;
#endif /* defined(CONFIG_SDK_DRIVER_NIC_USER_MODE) */ 
/*
 * Macro Declaration
 */

/*
 * Function Declaration
 */
#if defined(CONFIG_SDK_DRIVER_NIC_USER_MODE)
osal_isrret_t
_nic_intr_handler(void *isr_param)
{
#if defined(CONFIG_SDK_RTL8389)
    ioal_mem32_write(0, RTL8389_CPU_INTERFACE_INTERRUPT_MASK_ADDR, (0x00000000U));
    
    ioal_mem32_read(0, RTL8389_CPU_INTERFACE_INTERRUPT_STATUS_ADDR, &nic_intr_status);
    ioal_mem32_write(0, RTL8389_CPU_INTERFACE_INTERRUPT_STATUS_ADDR, nic_intr_status);        
#elif defined(CONFIG_SDK_RTL8328)
    ioal_mem32_write(0, RTL8328_CPUIIMR_ADDR, (0x00000000U));

    ioal_mem32_read(0, RTL8328_CPUIISR_ADDR, &nic_intr_status);
    ioal_mem32_write(0, RTL8328_CPUIISR_ADDR, nic_intr_status);
#endif
    
    if ((drv_chip_family_id ==  RTL8390_FAMILY_ID)|| (drv_chip_family_id ==  RTL8350_FAMILY_ID))
    {
        ioal_mem32_write(0, RTL8390_DMA_IF_INTR_MSK_ADDR, (0x00000000U));        
    ioal_mem32_read(0, RTL8390_DMA_IF_INTR_STS_ADDR, &nic_intr_status);
        ioal_mem32_write(0, RTL8390_DMA_IF_INTR_STS_ADDR, nic_intr_status);
#if defined(CONFIG_SDK_DRIVER_L2NTFY)
    if (nic_intr_status & INT_NTFY_DONE_MASK)
    {
        drv_swcore_l2_notification_isr_handler(0);
    }
#endif
    }
    else if ((drv_chip_family_id ==  RTL8380_FAMILY_ID)|| (drv_chip_family_id ==  RTL8330_FAMILY_ID))
    {
    ioal_mem32_write(0, RTL8380_DMA_IF_INTR_MSK_ADDR, (0x00000000U));
    
    ioal_mem32_read(0, RTL8380_DMA_IF_INTR_STS_ADDR, &nic_intr_status);
        ioal_mem32_write(0, RTL8380_DMA_IF_INTR_STS_ADDR, nic_intr_status);
    }

    if (atomic_dec_return(&nic_wait_for_intr) >= 0) 
    {
        wake_up_interruptible(&nic_intr_wait_queue);
    }
    return OSAL_INT_HANDLED;
} /* end of _nic_isr_handler */ 

osal_isrret_t
_sw_intr_handler(void *isr_param)
{  
#if defined(CONFIG_SDK_RTL8389)
    ioal_mem32_read(0, RTL8389_SWITCH_INTERRUPT_SOURCE_STATUS_ADDR, &sw_intr_status);
    ioal_mem32_write(0, RTL8389_SWITCH_INTERRUPT_CONTROL0_ADDR, (0x00000000U));
    
    ioal_mem32_read(0, RTL8389_SWITCH_INTERRUPT_STATUS0_ADDR, &sw_sisr0);
	ioal_mem32_write(0, RTL8389_SWITCH_INTERRUPT_STATUS0_ADDR, sw_sisr0);
#endif
#if defined(CONFIG_SDK_RTL8328)
    ioal_mem32_read(0, RTL8328_SWITCH_INTERRUPT_GLOBAL_SOURCE_STATUS_ADDR, &sw_intr_status);
    ioal_mem32_write(0, RTL8328_PER_PORT_LINK_CHANGE_INTERRUPT_CONTROL_ADDR, (0x00000000U));

    ioal_mem32_read(0, RTL8328_PER_PORT_LINK_CHANGE_INTERRUPT_STATUS_ADDR, &sw_sisr0);
	ioal_mem32_write(0, RTL8328_PER_PORT_LINK_CHANGE_INTERRUPT_STATUS_ADDR, sw_sisr0);
#endif
#if defined(CONFIG_SDK_RTL8390)
if ((drv_chip_family_id ==  RTL8390_FAMILY_ID)|| (drv_chip_family_id ==  RTL8350_FAMILY_ID))
	{
		ioal_mem32_read(0, RTL8390_ISR_GLB_SRC_ADDR, &sw_intr_status);			   
		ioal_mem32_read(0, RTL8390_ISR_PORT_LINK_STS_CHG_ADDR(0),  &sw_sisr0);		 
		ioal_mem32_read(0, RTL8390_ISR_PORT_LINK_STS_CHG_ADDR(32), &sw_sisr1); 	      
		ioal_mem32_write(0, RTL8390_ISR_PORT_LINK_STS_CHG_ADDR(0), sw_sisr0);		
		ioal_mem32_write(0, RTL8390_ISR_PORT_LINK_STS_CHG_ADDR(32), sw_sisr1);
	}	
#endif
#if defined(CONFIG_SDK_RTL8380)
	if ((drv_chip_family_id ==  RTL8380_FAMILY_ID)|| (drv_chip_family_id ==  RTL8330_FAMILY_ID))
	{
		ioal_mem32_read(0, RTL8380_ISR_GLB_SRC_ADDR, &sw_intr_status);
		ioal_mem32_read(0, RTL8380_ISR_PORT_LINK_STS_CHG_ADDR, &sw_sisr0);
		/* clear status */
		ioal_mem32_write(0, RTL8380_ISR_PORT_LINK_STS_CHG_ADDR, sw_sisr0);
	}
#endif

    if (atomic_dec_return(&sw_wait_for_intr) >= 0) 
    {
        wake_up_interruptible(&sw_intr_wait_queue);
    }

    return OSAL_INT_HANDLED;
} /* end of _nic_isr_handler */ 
#endif /* defined(CONFIG_SDK_DRIVER_NIC_USER_MODE) */ 

 
static int rtcore_open(struct inode *inode, struct file *file)
{   
    rtcore_dev_t *dev;

    dev = container_of(inode->i_cdev, rtcore_dev_t, rt_cdev);
    dev->rt_data = rtcore_devices[SYS_DEFAULT_INIT_UNIT_ID].rt_data;
    file->private_data = dev;

    return RT_ERR_OK;           
}

static int rtcore_release(struct inode *inode, struct file *file)
{ 
    return RT_ERR_OK;       
}

void rtcore_vma_open(struct vm_area_struct *vma)
{
	printk(KERN_DEBUG "VMA Open, Virt %lx, Phys %lx\n",
			vma->vm_start, vma->vm_pgoff << PAGE_SHIFT);
}

void rtcore_vma_close(struct vm_area_struct *vma)
{
	printk(KERN_DEBUG "VMA Close\n");
}

static int rtcore_ioctl( struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
    rtcore_ioctl_t dio;
#if defined(CONFIG_SDK_DRIVER_NIC_USER_MODE)
    unsigned long flags;
#endif
#if defined(CONFIG_SDK_UART1)
    uint8   uart_data;
    drv_uart_baudrate_t uart_baudrate;
#endif

    if (copy_from_user(&dio, (void*)arg, sizeof(dio)))
    {
        return -EFAULT;
    }
	
    switch (cmd)
    {       	
#if defined(CONFIG_SDK_KERNEL_LINUX_USER_MODE)
        case RTCORE_CID_GET:
            dio.ret = drv_swcore_cid_get(dio.data[0], &dio.data[1], &dio.data[2]);
            break;

        case RTCORE_CID_CMP:
            dio.ret = drv_swcore_cid_cmp(dio.data[0], dio.data[1]);
            break;    

        case RTCORE_CACHE_FLUSH:
            dio.ret = osal_cache_memory_flush(dio.data[0], dio.data[1]);
            break;         
#if defined(CONFIG_SDK_OSAL_SEM_KERNEL)
        case RTCORE_SEM_CREATE:
            dio.ret = osal_sem_create(dio.data[0]);
            break;
            
        case RTCORE_SEM_DESTROY:
            osal_sem_destroy(dio.data[0]);
            return RT_ERR_OK;
            
        case RTCORE_SEM_TAKE:
            dio.ret = osal_sem_take(dio.data[0], dio.data[1]);
            break;
            
        case RTCORE_SEM_GIVE:
            dio.ret = osal_sem_give(dio.data[0]);
            break;
#endif /* defined(CONFIG_SDK_OSAL_SEM_KERNEL) */

#if defined(CONFIG_SDK_DRIVER_NIC_KERNEL_MODE)
        case RTCORE_NIC_DBG_GET:
            dio.ret = drv_nic_dbg_get(dio.data[0], &dio.data[1]);
            break;

        case RTCORE_NIC_DBG_SET:
            dio.ret = drv_nic_dbg_set(dio.data[0], dio.data[1]);
            break;

        case RTCORE_NIC_CNTR_CLEAR:
            dio.ret = drv_nic_cntr_clear(dio.data[0]);
            break;

        case RTCORE_NIC_CNTR_DUMP:
            dio.ret = drv_nic_cntr_dump(dio.data[0]);
            break;
            
        case RTCORE_NIC_BUF_DUMP:
            dio.ret = drv_nic_ringbuf_dump(dio.data[0]);
            break;

        case RTCORE_NIC_PHMBUF_DUMP:
            dio.ret = drv_nic_pkthdr_mbuf_dump(dio.data[0], dio.data[1], dio.data[2], dio.data[3], dio.data[4]);
            break;
            
        case RTCORE_NIC_RX_START:
            dio.ret = drv_nic_rx_start(dio.data[0]);
            break;
            
        case RTCORE_NIC_RX_STOP:
            dio.ret = drv_nic_rx_stop(dio.data[0]);
            break;   
            
        case RTCORE_NIC_RX_STATUS_GET:
            dio.ret = drv_nic_rx_status_get(dio.data[0], &dio.data[1]);
            break;

        case RTCORE_NIC_PKT_TX:
            dio.ret = drv_nic_pkt_tx(dio.data[0], (drv_nic_pkt_t *)dio.data[1], (drv_nic_tx_cb_f) dio.data[2], (void *) dio.data[3]);
            break;

        case RTCORE_NIC_RESET:
            dio.ret = drv_nic_reset(dio.data[0]);
            break;

#endif /* defined(CONFIG_SDK_DRIVER_NIC_KERNEL_MODE) */ 
#if defined(CONFIG_SDK_DRIVER_NIC_USER_MODE)
        case RTCORE_INTR_ENABLE_SET:
            {
                local_irq_save(flags);                                
                
                if (INTR_TYPE_NIC == (dio.data[2]))
                {
                    if (ENABLED == dio.data[1])
                    {
                        init_waitqueue_head(&nic_intr_wait_queue);
                        if(DISABLED == _nic_intr_enabled)
                        {                                 
                            dio.ret = osal_isr_register(RTK_DEV_NIC, _nic_intr_handler, NULL);
                            _nic_intr_enabled = ENABLED;
                        }
                    }
                    else
                    {
                        if(ENABLED == _nic_intr_enabled)
                        {                                     
                            dio.ret = osal_isr_unregister(RTK_DEV_NIC);
                            _nic_intr_enabled = DISABLED;
                        }
                    }
                }
                else
                {
                    if (ENABLED == dio.data[1])
                    {
                        init_waitqueue_head(&sw_intr_wait_queue);
                        if(DISABLED == _sw_intr_enabled)
                        {                           
                            dio.ret = osal_isr_register(RTK_DEV_SWCORE, _sw_intr_handler, NULL);
                            _sw_intr_enabled = ENABLED;                            
                        }
                    }
                    else
                    {  
                        if(ENABLED == _sw_intr_enabled)
                        {                                   
                            dio.ret = osal_isr_unregister(RTK_DEV_SWCORE);
                            _sw_intr_enabled = DISABLED;
                        }
                    }
                }                   
                local_irq_restore(flags);
            }       
            break;
        case RTCORE_INTR_WAIT:
            {
                if (INTR_TYPE_NIC == (dio.data[1]))
                {                
            		if (nic_wait_for_intr.counter <= 0)
            	    {
            			atomic_inc(&nic_wait_for_intr);
            		}            
                    wait_event_interruptible(nic_intr_wait_queue, atomic_read(&nic_wait_for_intr) <= 0);
                   /*                      
                    * only run the interrupt handler once.
                    */
                    atomic_set(&nic_wait_for_intr, 0);
                    dio.data[2] = nic_intr_status;              
                    dio.data[3] = l2NotifyEventCnt;
                    dio.data[4] = curPos;
                }
                else
                {                
            		if (sw_wait_for_intr.counter <= 0)
            	    {
            			atomic_inc(&sw_wait_for_intr);
            		}
            
                    wait_event_interruptible(sw_intr_wait_queue, atomic_read(&sw_wait_for_intr) <= 0);
                   /*                      
                    * only run the interrupt handler once.
                    */
                    atomic_set(&sw_wait_for_intr, 0);
                    dio.data[2] = sw_intr_status;
                    dio.data[3] = sw_sisr0;
#if defined(CONFIG_SDK_RTL8390)
                    if ((drv_chip_family_id ==  RTL8390_FAMILY_ID)|| (drv_chip_family_id ==  RTL8350_FAMILY_ID))
	            {
		        dio.data[4] = sw_sisr1;
                    }
#endif
                }                    
            }
            break;
#endif /* defined(CONFIG_SDK_DRIVER_NIC_USER_MODE) */ 
        case RTCORE_GPIO_DATABIT_GET:
            dio.ret = drv_gpio_dataBit_get(dio.data[0], &dio.data[1]);
            break;
        case RTCORE_GPIO_DATABIT_SET:
            dio.ret = drv_gpio_dataBit_set(dio.data[0], dio.data[1]);
            break;
        case RTCORE_GPIO_INIT:
            dio.ret = drv_gpio_init(dio.data[0], dio.data[1], dio.data[2], dio.data[3]);
            break;
        case RTCORE_GPIO_DATABIT_INIT:
            dio.ret = drv_gpio_dataBit_init(dio.data[0], dio.data[1]);
            break;
        case RTCORE_SMI_INIT:
            dio.ret = drv_smi_init(dio.data[1], dio.data[2], dio.data[3], dio.data[4], dio.data[0]);
            break;
        case RTCORE_SMI_GROUP_GET:
            dio.ret = drv_smi_group_get(&dio.data[1], &dio.data[2], &dio.data[3], &dio.data[4], dio.data[0]);
            break;
        case RTCORE_SMI_TYPE_GET:
            dio.ret = drv_smi_type_get(&dio.data[1], &dio.data[2], &dio.data[3], dio.data[0]);
            break;
        case RTCORE_SMI_TYPE_SET:
            dio.ret = drv_smi_type_set(dio.data[1], dio.data[2], dio.data[3], dio.data[0]);
            break;
        case RTCORE_SMI_READ:
            dio.ret = drv_smi_read(dio.data[1], &dio.data[2], dio.data[0]);
            break;
        case RTCORE_SMI_WRITE:
            dio.ret = drv_smi_write(dio.data[1], dio.data[2], dio.data[0]);
            break;
#if defined(CONFIG_SDK_RTL8231)
        /* EXT_SMI */
        case RTCORE_EXTSMI_DEV_INIT:
            dio.ret = drv_extSmi_dev_init(dio.data[0], dio.data[1], dio.data[2], dio.data[3], dio.data[4], dio.data[5]);
            break;
        case RTCORE_EXTSMI_READ:
            dio.ret = drv_extSmi_read(dio.data[0], dio.data[1], dio.data[2], &dio.data[3]);
            break;
        case RTCORE_EXTSMI_WRITE:
            dio.ret = drv_extSmi_write(dio.data[0], dio.data[1], dio.data[2], dio.data[3]);
            break;
        
        /* EXT_GPIO */
        case RTCORE_EXTGPIO_REG_WRITE:
            dio.ret = drv_extGpio_reg_write(dio.data[0], dio.data[1], dio.data[2], dio.data[3]);
            break;
        case RTCORE_EXTGPIO_DEV_INIT:
            {
                drv_extGpio_devConf_t devconf;
                devconf.access_mode = dio.data[2];
                devconf.address = dio.data[3];
                devconf.page = dio.data[4];
                dio.ret = drv_extGpio_dev_init(dio.data[0], dio.data[1], &devconf);
            }
            break;
        case RTCORE_EXTGPIO_DEVENABLE_GET:
            {
                rtk_enable_t    enable;
                dio.ret = drv_extGpio_devEnable_get(dio.data[0], dio.data[1], &enable);
                dio.data[2] = enable;
            }
            break;
        case RTCORE_EXTGPIO_REG_READ:
            dio.ret = drv_extGpio_reg_read(dio.data[0], dio.data[1], dio.data[2], &dio.data[3]);
            break;
        case RTCORE_EXTGPIO_DEVREADY_GET:
            dio.ret = drv_extGpio_devReady_get(dio.data[0], dio.data[1], &dio.data[2]);
            break;
        case RTCORE_EXTGPIO_DEV_GET:
            {
                drv_extGpio_devConf_t devconf;
                dio.ret = drv_extGpio_dev_get(dio.data[0], dio.data[1], &devconf);
                dio.data[2] = devconf.access_mode;
                dio.data[3] = devconf.address;
                dio.data[4] = devconf.page;
            }
            break;
        case RTCORE_EXTGPIO_DEVENABLE_SET:
            dio.ret = drv_extGpio_devEnable_set(dio.data[0], dio.data[1], dio.data[2]);
            break;
        case RTCORE_EXTGPIO_SYNCENABLE_GET:
            {
                rtk_enable_t    enable;
                dio.ret = drv_extGpio_syncEnable_get(dio.data[0], dio.data[1], &enable);
                dio.data[2] = enable;
            }
            break;
        case RTCORE_EXTGPIO_SYNCENABLE_SET:
            dio.ret = drv_extGpio_syncEnable_set(dio.data[0], dio.data[1], dio.data[2]);
            break;
        case RTCORE_EXTGPIO_SYNCSTATUS_GET:
            dio.ret = drv_extGpio_syncStatus_get(dio.data[0], dio.data[1], &dio.data[2]);
            break;
        case RTCORE_EXTGPIO_SYNC_START:
            dio.ret = drv_extGpio_sync_start(dio.data[0], dio.data[1]);
            break;
        case RTCORE_EXTGPIO_PIN_GET:
            {
                drv_extGpio_conf_t  conf;
                dio.ret = drv_extGpio_pin_get(dio.data[0], dio.data[1], dio.data[2], &conf);
                dio.data[3] = conf.direction;
                dio.data[4] = conf.debounce;
                dio.data[5] = conf.inverter;
            }
            break;
        case RTCORE_EXTGPIO_PIN_INIT:
            {
                drv_extGpio_conf_t  conf;
                conf.direction = dio.data[3];
                conf.debounce = dio.data[4];
                conf.inverter = dio.data[5];
                dio.ret = drv_extGpio_pin_init(dio.data[0], dio.data[1], dio.data[2], &conf);
            }
            break;
        case RTCORE_EXTGPIO_DATABIT_GET:
            dio.ret = drv_extGpio_dataBit_get(dio.data[0], dio.data[1], dio.data[2], &dio.data[3]);
            break;
        case RTCORE_EXTGPIO_DATABIT_SET:
            dio.ret = drv_extGpio_dataBit_set(dio.data[0], dio.data[1], dio.data[2], dio.data[3]);
            break;
        case RTCORE_EXTGPIO_DEVRECOVERY_START:
            dio.ret = drv_extGpio_devRecovery_start(dio.data[0], dio.data[1]);
            break;
        /* RTL8231 */
        case RTCORE_RTL8231_I2C_READ:
            dio.ret = drv_rtl8231_i2c_read(dio.data[0], dio.data[1], dio.data[2], &dio.data[3]);
            break;
        case RTCORE_RTL8231_I2C_WRITE:
            dio.ret = drv_rtl8231_i2c_write(dio.data[0], dio.data[1], dio.data[2], dio.data[3]);
            break;
        case RTCORE_RTL8231_MDC_READ:
            dio.ret = drv_rtl8231_mdc_read(dio.data[0], dio.data[1], dio.data[2], dio.data[3], &dio.data[4]);
            break;
        case RTCORE_RTL8231_MDC_WRITE:
            dio.ret = drv_rtl8231_mdc_write(dio.data[0], dio.data[1], dio.data[2], dio.data[3], dio.data[4]);
            break;
        case RTCORE_EXTGPIO_DIRECTION_GET:
            dio.ret = drv_extGpio_direction_get(dio.data[0], dio.data[1], dio.data[2], (drv_gpio_direction_t *)&dio.data[3]);
            break;
        case RTCORE_EXTGPIO_DIRECTION_SET:
            dio.ret = drv_extGpio_direction_set(dio.data[0], dio.data[1], dio.data[2], (drv_gpio_direction_t)dio.data[3]);
            break;
        case RTCORE_EXTGPIO_I2C_INIT:
            dio.ret = drv_extGpio_i2c_init(dio.data[0], dio.data[1], dio.data[2], dio.data[3]);
            break;
        case RTCORE_EXTGPIO_I2C_READ:
            dio.ret = drv_extGpio_i2c_read(dio.data[0], dio.data[1], dio.data[2], &dio.data[3]);
            break;
        case RTCORE_EXTGPIO_I2C_WRITE:
            dio.ret = drv_extGpio_i2c_write(dio.data[0], dio.data[1], dio.data[2], dio.data[3]);
            break;			
#endif /* defined(CONFIG_SDK_RTL8231) */

		 /* General GPIO */  
		 case RTCORE_GENCTRL_GPIO_DEV_INIT:
			 {
				 drv_generalCtrlGpio_devConf_t config_dev;
				 if(dio.data[1] == GEN_GPIO_DEV_ID0_INTERNAL)
				 {
					 config_dev.direction = dio.data[3];
					 config_dev.default_value = dio.data[4];
				 }else{
					 config_dev.direction = dio.data[3];
					 config_dev.default_value = dio.data[4];
					 config_dev.ext_gpio.access_mode = dio.data[5];
					 config_dev.ext_gpio.address = dio.data[6];
					 config_dev.ext_gpio.page= dio.data[7];
				 }
				 dio.ret = drv_generalCtrlGPIO_dev_init(dio.data[0], dio.data[1], dio.data[2], &config_dev);
			 }	 
			 break;
		 case RTCORE_GENCTRL_GPIO_DEV_ENABLE_SET:
			 dio.ret = drv_generalCtrlGPIO_devEnable_set(dio.data[0], dio.data[1], dio.data[2]);
			 break;
		 case RTCORE_GENCTRL_GPIO_PIN_INIT:
			 {
				 drv_generalCtrlGpio_pinConf_t config_pin;
				 if(dio.data[1] == GEN_GPIO_DEV_ID0_INTERNAL)
				 {
					 config_pin.direction = dio.data[3];
					 config_pin.default_value = dio.data[4];
					 config_pin.int_gpio.function = dio.data[5];
					 config_pin.int_gpio.interruptEnable = dio.data[6];
				 }else{
					 config_pin.direction = dio.data[3];
					 config_pin.default_value = dio.data[4];
					 config_pin.ext_gpio.direction = dio.data[5];
					 config_pin.ext_gpio.debounce = dio.data[6];
					 config_pin.ext_gpio.inverter = dio.data[7];
				 }
				 dio.ret = drv_generalCtrlGPIO_pin_init(dio.data[0], dio.data[1], dio.data[2], &config_pin);
			 }	 
			 break;
		 case RTCORE_GENCTRL_GPIO_DATABIT_SET:
			 dio.ret = drv_generalCtrlGPIO_dataBit_set(dio.data[0], dio.data[1], dio.data[2], dio.data[3]);
			 break;
		 case RTCORE_GENCTRL_GPIO_DATABIT_GET:
			 dio.ret = drv_generalCtrlGPIO_dataBit_get(dio.data[0], dio.data[1], dio.data[2], &dio.data[3]);
			 break; 	 

         case RTCORE_WATCHDOG_INIT:
            dio.ret = drv_watchdog_init((uint32)dio.data[0]);
            break;
         case RTCORE_WATCHDOG_MODE_SET:
            dio.ret = drv_watchdog_mode_set((uint32)dio.data[0], (drv_watchdog_mode_t)dio.data[1]);
            break;
         case RTCORE_WATCHDOG_MODE_GET:
            dio.ret = drv_watchdog_mode_get((uint32)dio.data[0], (drv_watchdog_mode_t*)&dio.data[1]);
            break;
         case RTCORE_WATCHDOG_SCALE_SET:
            dio.ret = drv_watchdog_scale_set((uint32)dio.data[0], (drv_watchdog_scale_t)dio.data[1]);
            break;
         case RTCORE_WATCHDOG_SCALE_GET:
            dio.ret = drv_watchdog_scale_get((uint32)dio.data[0], (drv_watchdog_scale_t*)&dio.data[1]);
            break;
         case RTCORE_WATCHDOG_ENABLE_SET:
            dio.ret = drv_watchdog_enable_set((uint32)dio.data[0], (uint32)dio.data[1]);
            break;
         case RTCORE_WATCHDOG_ENABLE_GET:
            dio.ret = drv_watchdog_enable_get((uint32)dio.data[0], (uint32*)&dio.data[1]);
            break;
         case RTCORE_WATCHDOG_KICK:
            dio.ret = drv_watchdog_kick((uint32)dio.data[0]);
            break;
         case RTCORE_WATCHDOG_THRESHOLD_SET:
            {
                drv_watchdog_threshold_t watchdog_threshold;
                watchdog_threshold.phase_1_threshold = dio.data[1];
                watchdog_threshold.phase_2_threshold = dio.data[2];
                dio.ret = drv_watchdog_threshold_set((uint32)dio.data[0], &watchdog_threshold);
                break;
            }
         case RTCORE_WATCHDOG_THRESHOLD_GET:
            {
                drv_watchdog_threshold_t watchdog_threshold;
                dio.ret = drv_watchdog_threshold_get((uint32)dio.data[0], &watchdog_threshold);
                dio.data[1] = watchdog_threshold.phase_1_threshold ;
                dio.data[2] = watchdog_threshold.phase_2_threshold ;
            	break;
            }
         case RTCORE_WATCHDOG_UNAVAIL:
            dio.ret = drv_watchdog_unavail();
            break;
            
#if defined(CONFIG_SDK_UART1)
         case RTCORE_UART1_CHAR_GET:
            dio.ret = drv_uart_getc((uint32)dio.data[0], &uart_data, (uint32) dio.data[2]);
            dio.data[1] = (int32)uart_data;
            break;
         case RTCORE_UART1_CHAR_PUT:
            dio.ret = drv_uart_putc((uint32)dio.data[0], (uint8)dio.data[1]);
            break;
         case RTCORE_UART1_BAUDRATE_GET:
            dio.ret = drv_uart_baudrate_get((uint32)dio.data[0], &uart_baudrate);
            dio.data[1] = (int32)uart_baudrate;
            break;
         case RTCORE_UART1_BAUDRATE_SET:
            dio.ret = drv_uart_baudrate_set((uint32)dio.data[0], (drv_uart_baudrate_t)dio.data[1]);
            break;
#endif /* defined(CONFIG_SDK_UART1) */

#endif /* defined(CONFIG_SDK_KERNEL_LINUX_USER_MODE) */
        case RTCORE_DEBUG_REGISTER_DUMP:
            dio.ret = drv_swcore_register_dump(dio.data[0]);
            break;

        default:
            return -ENOTTY;            
    }

    if (copy_to_user((void*)arg, &dio, sizeof(dio)))
    {
        return -EFAULT;
    }
    
    return RT_ERR_OK;     
}

static int rtcore_mmap(struct file *filp, struct vm_area_struct *vma)
{
    unsigned long pfn;
    unsigned long start = vma->vm_start;    
    size_t size = vma->vm_end - vma->vm_start;
    rtcore_dev_t *dev = filp->private_data;

    /* vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot); */
    pfn = (virt_to_phys((void *)dev->rt_data)) >> PAGE_SHIFT;

    if (remap_pfn_range(vma,
                        start,
                        pfn,
                        size,
                        vma->vm_page_prot))
        return -EAGAIN;         

    vma->vm_private_data = filp->private_data;    
    vma->vm_ops = &rtcore_remap_vm_ops;
    rtcore_vma_open(vma);
    
    return RT_ERR_OK;   
}

/* Function Name:
 *      rtcore_dev_setup
 * Description:
 *      Seteup Core Device 
 * Input:
 *      None 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
static int32 rtcore_dev_setup(uint32 unit)
{
#if defined(CONFIG_SDK_KERNEL_LINUX_USER_MODE)

    int32 ret = RT_ERR_FAILED;    

    rtcore_dev_data_t *data = rtcore_devices[unit].rt_data;

    RT_ERR_HDL(rt_log_format_get(&data->log_format), setup_end, ret);
    RT_ERR_HDL(rt_log_level_get(&data->log_level), setup_end, ret);
    RT_ERR_HDL(rt_log_mask_get(&data->log_mask), setup_end, ret);
    RT_ERR_HDL(rt_log_moduleMask_get(&data->log_module_mask), setup_end, ret);
    RT_ERR_HDL(rt_log_type_get(&data->log_type), setup_end, ret);

    /* save the backup value */
    data->log_level_bak = data->log_level;
    data->log_mask_bak = data->log_mask;

setup_end:
    return ret;     

#else
    return RT_ERR_OK;

#endif 
}

#if defined(CONFIG_SDK_SOFTWARE_CONTROL_LED)
static int32
swCtrl_led_intr_handler(void *isr_param)
{
    uint32 chipId = 0, chipRevId = 0;
	int32  ret;
    static int32 is_probe = 0, flag_es = 0;

    if (is_probe == 0)
    {
        if ((ret = drv_swcore_cid_get(SYS_DEFAULT_INIT_UNIT_ID, &chipId, &chipRevId)) != RT_ERR_OK)
            return RT_ERR_FAILED;
    
        if ((chipId & 0xFFFF)  == 0x6966)
            flag_es = 1;
        else
            flag_es = 0;
        is_probe = 1;
    }

	ret = swCtrl_led_refresh_handler();

    if (flag_es)
    {
        REG32(RTL8390ES_TCIR) |= RTL8390ES_TC1IP; /* Clear the Timer 1 is Interrupt Flag*/
    
    	/* Checking and Double checking the Interrupt Flags are cleared*/
        if (REG32(RTL8390ES_TCIR) & RTL8390ES_TC1IP)
            ret = swCtrl_led_error_handler(3);
        if (REG32(GISR) & TC1_IP)
            ret = swCtrl_led_error_handler(4);
    }
    else
    {
        REG32(RTL8390MP_TC1INT) |= RTL8390MP_TCIP; /* Clear the Timer 1 is Interrupt Flag*/
    
    	/* Checking and Double checking the Interrupt Flags are cleared*/
        if (REG32(RTL8390MP_TC1INT) & RTL8390MP_TCIP)
            ret = swCtrl_led_error_handler(3);
        if (REG32(GISR) & TC1_IP)
            ret = swCtrl_led_error_handler(4);
    }

    return RT_ERR_OK;
} /* end of swCtrl_led_intr_handler */ 
#endif

/* Function Name:
 *      rtcore_dev_init
 * Description:
 *      Core Driver Init 
 * Input:
 *      None 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
static int32 __init rtcore_dev_init(void)
{  
    int i, ret = RT_ERR_FAILED;    
    dev_t devno = MKDEV(RTCORE_DRV_MAJOR, 0);
#if defined(CONFIG_SDK_SOFTWARE_CONTROL_LED)	
    uint32  temp_reg;
    uint32  chipId = 0, chipRevId = 0;
    uint32  flag_es;
#endif    
#if defined(CONFIG_SDK_DRIVER_RTCORE_PAGE_RESERVED)
    struct page *page;     
#endif
	uint32  chip_family_Id;

    if (register_chrdev_region(devno, rtcore_num, RTCORE_DRV_NAME) < 0)
    {
		printk("unable to get major %d for %s dev\n", RTCORE_DRV_MAJOR, RTCORE_DRV_NAME);
		return RT_ERR_FAILED;
	}

    rtcore_devices = kmalloc(rtcore_num * sizeof(rtcore_dev_t), GFP_KERNEL);
    if (!rtcore_devices)
        return -ENOMEM;
    
    memset(rtcore_devices, 0, rtcore_num * sizeof(rtcore_dev_t));

    for (i = 0; i < rtcore_num; i++)
    {       
        devno = MKDEV(RTCORE_DRV_MAJOR, 0 + i);
        cdev_init(&rtcore_devices[i].rt_cdev, &rtcore_fops);
	    rtcore_devices[i].rt_cdev.owner = THIS_MODULE;
	    rtcore_devices[i].rt_cdev.ops = &rtcore_fops;

        if(cdev_add(&rtcore_devices[i].rt_cdev, devno, 1))
	    {
	        printk (KERN_NOTICE "**Error- dev %s(%d) adding error", RTCORE_DRV_NAME, RTCORE_DRV_MAJOR);
	        unregister_chrdev_region(devno, 1);
            return RT_ERR_FAILED;
        }

#if defined(CONFIG_SDK_KERNEL_LINUX_USER_MODE)
        rtcore_devices[i].rt_data = (rtcore_dev_data_t *)kmalloc(MEM_RESERVED_SIZE, GFP_KERNEL);
        if(!rtcore_devices[i].rt_data)
        {
   	        unregister_chrdev_region(devno, 1);
            return RT_ERR_FAILED;
        }

        memset(rtcore_devices[i].rt_data, 0, MEM_RESERVED_SIZE);

#if defined(CONFIG_SDK_DRIVER_RTCORE_PAGE_RESERVED)
        for(page = virt_to_page(rtcore_devices[i].rt_data); page < virt_to_page(rtcore_devices[i].rt_data + MEM_RESERVED_SIZE); page++)
            SetPageReserved(page);
#endif /* defined(CONFIG_SDK_DRIVER_RTCORE_PAGE_RESERVED) */

#endif /* defined(CONFIG_SDK_KERNEL_LINUX_USER_MODE) */
    }

    /* Used by all layers, Initialize first */
    RT_ERR_CHK(rtcore_init(SYS_DEFAULT_INIT_UNIT_ID), ret);

    /* Setup the device */
    RT_ERR_CHK(rtcore_dev_setup(SYS_DEFAULT_INIT_UNIT_ID), ret);

#if defined(CONFIG_SDK_DRIVER_RTCORE_MODULE)	
    printk(KERN_INFO"Init RTCORE Driver Module....OK\n");
#endif
#if defined(CONFIG_SDK_SOFTWARE_CONTROL_LED)
    if ((ret = drv_swcore_cid_get(SYS_DEFAULT_INIT_UNIT_ID, &chipId, &chipRevId)) != RT_ERR_OK)
        return RT_ERR_FAILED;

    flag_es = 0;
    if ((chipId & 0xFFFF)  == 0x6966)
        flag_es = 1;

    if (flag_es)
    {
        /* Clear Timer IP status */
        if (REG32(RTL8390ES_TCIR) & RTL8390ES_TC1IP)
            REG32(RTL8390ES_TCIR) |= RTL8390ES_TC1IP;
    
        /* The Timer 1 counter value is 10 mS * SWLED_REFLASH_FREQ */
        REG32(RTL8390ES_TC1DATA) = (((MHZ * 1000000) / (DIVISOR * HZ)) << (RTL8390ES_TCD_OFFSET))*SWLED_REFLASH_FREQ;
    	
        /*Configure the Timer 1 mode and Enable Timer 1*/
        temp_reg = REG32(RTL8390ES_TCCNR);
        REG32(RTL8390ES_TCCNR) = temp_reg | RTL8390ES_TC1EN | RTL8390ES_TC1MODE_TIMER;
        REG32(RTL8390ES_TCIR) |= RTL8390ES_TC1IE;	
    }
    else
    {
    	if (REG32(RTL8390MP_TC1INT) & RTL8390MP_TCIP)
    		REG32(RTL8390MP_TC1INT) |= RTL8390MP_TCIP;

        REG32(RTL8390MP_TC1DATA)= ((MHZ * 1000000)/(DIVISOR * HZ))*SWLED_REFLASH_FREQ;
        REG32(RTL8390MP_TC1CTL) = RTL8390MP_TCEN | RTL8390MP_TCMODE_TIMER | DIVISOR;
        REG32(RTL8390MP_TC1INT) = RTL8390MP_TCIE;
    }
	
    /*Setup the board LED information to swCtrl_led module*/
    ret = swCtrl_led_init();
    if (ret == RT_ERR_OK)
    {   /*Register the Timer 1 interrupt callback function*/
        osal_isr_register(RTK_DEV_TC1, swCtrl_led_intr_handler, NULL);
    }
    else
    {
        return RT_ERR_FAILED;
    }
    printk("Setup SW LED Control function....OK\n");
#endif
	drv_swcore_family_cid_get(SYS_DEFAULT_INIT_UNIT_ID, &chip_family_Id);
    return ret;
}

/* Function Name:
 *      rtcore_dev_exit
 * Description:
 *      Core Driver Exit
 * Input:
 *      None 
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void __exit rtcore_dev_exit(void)
{ 
    int i;

#if defined(CONFIG_SDK_DRIVER_RTCORE_PAGE_RESERVED)
    struct page *page;  

    for (i = 0; i < rtcore_num; i++)
    {              
        for(page = virt_to_page(rtcore_devices[i].rt_data); page < virt_to_page(rtcore_devices[i].rt_data + MEM_RESERVED_SIZE); page++)
            ClearPageReserved(page);
    }
#endif /* defined(CONFIG_SDK_DRIVER_RTCORE_PAGE_RESERVED) */

    for (i = 0; i < rtcore_num; i++)
        cdev_del(&rtcore_devices[i].rt_cdev);

    kfree(rtcore_devices);
    unregister_chrdev_region(MKDEV(RTCORE_DRV_MAJOR, 0), rtcore_num);   
    
#if defined(CONFIG_SDK_DRIVER_RTCORE_MODULE)	
    printk(KERN_INFO"Exit RTCORE Driver Module....OK\n");
#endif
}

module_init(rtcore_dev_init);
module_exit(rtcore_dev_exit);
module_param(rtcore_num, int, S_IRUGO);

MODULE_DESCRIPTION ("Switch SDK Core Module");

MODULE_LICENSE("GPL");

#if 0
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,19))	
MODULE_LICENSE("GPL");
#endif
#endif

