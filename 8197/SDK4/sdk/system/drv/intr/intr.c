/* Copyright (C) 2009 Realtek Semiconductor Corp.
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
 * Purpose : Definition of Interrupt control API
 *
 * Feature : The file includes the following modules
 *           (1) SWCORE
 *           (2) NIC
 *
 */

 /*
 * Include Files
 */
#include <common/rt_autoconf.h>
#include <drv/intr/intr.h>
#include <drv/swcore/chip.h>
#if defined(CONFIG_SDK_RTL8389)
#include <drv/swcore/rtl8389.h>
#endif
#if defined(CONFIG_SDK_RTL8328)
#include <drv/swcore/rtl8328.h>
#include <drv/nic/r8328.h>
#endif
#if defined(CONFIG_SDK_RTL8390)
#include <drv/swcore/rtl8390.h>
#include <drv/swcore/l2notification.h>
#include <drv/nic/r8390.h>
#endif
#if defined(CONFIG_SDK_RTL8380)
#include <drv/swcore/rtl8380.h>
#include <drv/nic/r8380.h>
#endif
#include <rtcore/rtcore.h>
#include <ioal/mem32.h>
#include <osal/print.h>
/*
 * Symbol Definition
 */
typedef struct drv_intr_hdl_attr_s {
    uint32	    maskbit;
    drv_intr_hdl_f	fIntr_handler;
    uint32	    intr_data[2];
    uint8	    *pIntr_name;
} drv_intr_hdl_attr_t;

#if defined(CONFIG_SDK_RTL8389)
#define NUM_OF_R8389_INTR_HANDLER (sizeof(r8389_intr_handlers)/sizeof(drv_intr_hdl_attr_t))
#endif
#if defined(CONFIG_SDK_RTL8328)
#define NUM_OF_R8328_INTR_HANDLER (sizeof(r8328_intr_handlers)/sizeof(drv_intr_hdl_attr_t))
#endif
#if defined(CONFIG_SDK_RTL8390)
#define NUM_OF_R8390_INTR_HANDLER (sizeof(r8390_intr_handlers)/sizeof(drv_intr_hdl_attr_t))
#endif
#if defined(CONFIG_SDK_RTL8380)
#define NUM_OF_R8380_INTR_HANDLER (sizeof(r8380_intr_handlers)/sizeof(drv_intr_hdl_attr_t))
#endif
#define INTR_DB_SIZE     (sizeof(intr_db)/sizeof(cid_group_t))
/*
 * Data Declaration
 */
static void drv_intr_link_stat_hdl(uint32 unit, void* data);
static drv_intr_hdl_f fLink_stat_cb = NULL;
static uint32 intr_count = 0;
#if defined(CONFIG_SDK_RTL8389)
static drv_intr_hdl_attr_t r8389_intr_handlers[] = {
    {(uint32)0x1, drv_intr_link_stat_hdl, {0}, "link change interrupt"},
};
#endif
#if defined(CONFIG_SDK_RTL8328)
static drv_intr_hdl_attr_t r8328_intr_handlers[] = {
    {(uint32)0x20, drv_intr_link_stat_hdl, {0}, "link change interrupt"},
};
#endif
#if defined(CONFIG_SDK_RTL8390)
static drv_intr_hdl_attr_t r8390_intr_handlers[] = {
    {RTL8390_ISR_GLB_SRC_ISR_GLB_LINK_CHG_MASK, drv_intr_link_stat_hdl, {0}, (uint8 *)"link change interrupt"},
};
#endif
#if defined(CONFIG_SDK_RTL8380)
static drv_intr_hdl_attr_t r8380_intr_handlers[] = {
    {(uint32)0x1, drv_intr_link_stat_hdl, {0}, (uint8 *)"link change interrupt"},
};
#endif

const static cid_group_t intr_db[] =
{
#if defined(CONFIG_SDK_RTL8389)
    {RTL8389M_CHIP_ID, INTR_R8389},
    {RTL8329M_CHIP_ID, INTR_R8389},
    {RTL8377M_CHIP_ID, INTR_R8389},
    {RTL8389L_CHIP_ID, INTR_R8389},
#endif
#if defined(CONFIG_SDK_RTL8328)
    {RTL8328M_CHIP_ID, INTR_R8328},
    {RTL8328S_CHIP_ID, INTR_R8328},
    {RTL8328L_CHIP_ID, INTR_R8328},
#endif
#if defined(CONFIG_SDK_RTL8390)
    {RTL8352M_CHIP_ID, INTR_R8390},
    {RTL8353M_CHIP_ID, INTR_R8390},
    {RTL8392M_CHIP_ID, INTR_R8390},
    {RTL8393M_CHIP_ID, INTR_R8390},
    {RTL8396M_CHIP_ID, INTR_R8390},
    {RTL8352MES_CHIP_ID, INTR_R8390},
    {RTL8353MES_CHIP_ID, INTR_R8390},
    {RTL8392MES_CHIP_ID, INTR_R8390},
    {RTL8393MES_CHIP_ID, INTR_R8390},
    {RTL8396MES_CHIP_ID, INTR_R8390},
#endif
#if defined(CONFIG_SDK_RTL8380)
    {RTL8380M_CHIP_ID, INTR_R8380},
    {RTL8330M_CHIP_ID, INTR_R8380},
    {RTL8382M_CHIP_ID, INTR_R8380},
    {RTL8332M_CHIP_ID, INTR_R8380},
    {RTL8380MES_CHIP_ID, INTR_R8380},
    {RTL8330MES_CHIP_ID, INTR_R8380},
    {RTL8382MES_CHIP_ID, INTR_R8380},
    {RTL8332MES_CHIP_ID, INTR_R8380},
#endif
};

#if defined(CONFIG_SDK_RTL8389)
static int32 _r8389_intr_enable_set(uint32 unit, drv_intr_source_t intr_source);
static void  _r8389_intr_swcore_handler(void *pParam);
static void  _r8389_intr_nic_handler(void *pParam);
#endif
#if defined(CONFIG_SDK_RTL8328)
static int32 _r8328_intr_enable_set(uint32 unit, drv_intr_source_t intr_source);
static void  _r8328_intr_swcore_handler(void *pParam);
static void  _r8328_intr_nic_handler(void *pParam);
#endif
#if defined(CONFIG_SDK_RTL8390)
static int32 _r8390_intr_enable_set(uint32 unit, drv_intr_source_t intr_source);
static void  _r8390_intr_swcore_handler(void *pParam);
static void  _r8390_intr_nic_handler(void *pParam);
#endif
#if defined(CONFIG_SDK_RTL8380)
static int32 _r8380_intr_enable_set(uint32 unit, drv_intr_source_t intr_source);
static void  _r8380_intr_swcore_handler(void *pParam);
static void  _r8380_intr_nic_handler(void *pParam);
#endif

drv_intr_mapper_operation_t intr_ops[INTR_CTRL_END] =
{
#if defined(CONFIG_SDK_RTL8389)
    {   /* INTR_R8389 */
        .enable_set = _r8389_intr_enable_set,
        .swcore_handler = _r8389_intr_swcore_handler,
        .nic_handler = _r8389_intr_nic_handler,
    },
#endif
#if defined(CONFIG_SDK_RTL8328)
    {   /* INTR_R8328 */
        .enable_set = _r8328_intr_enable_set,
        .swcore_handler = _r8328_intr_swcore_handler,
        .nic_handler = _r8328_intr_nic_handler,
    },
#endif
#if defined(CONFIG_SDK_RTL8390)
    {   /* INTR_R8390 */
        .enable_set = _r8390_intr_enable_set,
        .swcore_handler = _r8390_intr_swcore_handler,
        .nic_handler = _r8390_intr_nic_handler,
    },
#endif
#if defined(CONFIG_SDK_RTL8380)
    {   /* INTR_R8380 */
        .enable_set = _r8380_intr_enable_set,
        .swcore_handler = _r8380_intr_swcore_handler,
        .nic_handler = _r8380_intr_nic_handler,
    },
#endif
};

uint32 intr_if[RTK_MAX_NUM_OF_UNIT];


/*
 * Function Declaration
 */


/* Function Name:
 *      drv_intr_init
 * Description:
 *      Initialize intr module of the specified device.
 * Input:
 *      unit     - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Must initialize nic module before calling any nic APIs.
 */
int32
drv_intr_init(uint32 unit)
{
    uint32 i;

    for (i = 0; i < INTR_DB_SIZE; i++)
    {
        if(!drv_swcore_cid_cmp(unit, intr_db[i].cid))
        {
            intr_if[unit] = intr_db[i].gid;
            return RT_ERR_OK;
        }
    }

    return RT_ERR_FAILED;
} /* end of drv_intr_init */

/* Function Name:
 *      drv_intr_enable_set
 * Description:
 *      Set the interrupt enable status of the specified source.
 * Input:
 *      unit     - unit id
 *      intr_source - interrupt source that is going to be enabled.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 * Note:
 *      Must initialize interrupt module before calling any nic APIs.
 */
int32
drv_intr_enable_set(uint32 unit, drv_intr_source_t intr_source)
{
    return INTR_CTRL(unit).enable_set(unit, intr_source);
} /* end of drv_intr_enable_set */

/* Function Name:
 *      drv_intr_swcore_handler
 * Description:
 *      Common swcore interrupt handler function.
 * Input:
 *      pParam - The argument passed to interrupt handler at interrupt time.
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void
drv_intr_swcore_handler(void *pParam)
{
    rtcore_ioctl_t *dio;

    dio = (rtcore_ioctl_t *)pParam;

    return INTR_CTRL(dio->data[0]).swcore_handler(pParam);
} /* end of drv_intr_swcore_handler */

/* Function Name:
 *      drv_intr_nic_handler
 * Description:
 *      NIC interrupt handler function.
 * Input:
 *      pParam - The argument passed to interrupt handler at interrupt time.
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void
drv_intr_nic_handler(void *pParam)
{
    rtcore_ioctl_t *dio;

    dio = (rtcore_ioctl_t *)pParam;

    return INTR_CTRL(dio->data[0]).nic_handler(pParam);
} /* end of drv_intr_nic_handler */

#if defined(CONFIG_SDK_RTL8389)
/* Function Name:
 *      _r8389_intr_enable_set
 * Description:
 *      Enable interrupt with specified interrupt source of RTL8389 series
 * Input:
 *      unit        - unit id
 *      intr_source - interrupt source that is going to be enabled.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
static int32
_r8389_intr_enable_set(uint32 unit, drv_intr_source_t intr_source)
{
    RT_PARAM_CHK(intr_source >= INTR_SOURCE_END, RT_ERR_INPUT);

    switch (intr_source)
    {
        case LINK_CHANGE_INTR:
            ioal_mem32_write(unit, RTL8389_SWITCH_INTERRUPT_CONTROL0_ADDR, RTL8389_SWITCH_INTERRUPT_CONTROL0_LINK_STA_CHANGE_IE_MASK);
            break;
        case MEDIA_CHANGE_INTR:
            break;
        case SPEED_CHANGE_INTR:
            break;
        case DUPLEX_CHANGE_INTR:
            break;
        default:
            break;
    }

    return RT_ERR_OK;
} /* end of _r8389_intr_enable_set */

/* Function Name:
 *      _r8389_intr_swcoreHandler
 * Description:
 *      Common interrupt handler function of 8389 series.
 * Input:
 *      pParam - The argument passed to interrupt handler at interrupt time.
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void
_r8389_intr_swcore_handler(void *pParam)
{
    uint32  intr_status;
    uint32  unit, i;

    intr_count++;
    unit = ((rtcore_ioctl_t *)pParam)->data[0];
    intr_status = ((rtcore_ioctl_t *)pParam)->data[2];

	for (i = 0; i < NUM_OF_R8389_INTR_HANDLER; i++)
    {
	    if (intr_status & r8389_intr_handlers[i].maskbit)
	    {
    		/* Bit found, dispatch interrupt */
    	    /* Assing interrupt status carried from kernel space */
            r8389_intr_handlers[i].intr_data[0] = ((rtcore_ioctl_t *)pParam)->data[3];
    		(r8389_intr_handlers[i].fIntr_handler)(unit, (void *)&r8389_intr_handlers[i].intr_data);
        }
	}

    /*
     * reenable interrupts here.
     */
    ioal_mem32_write(unit, RTL8389_SWITCH_INTERRUPT_CONTROL0_ADDR, RTL8389_SWITCH_INTERRUPT_CONTROL0_LINK_STA_CHANGE_IE_MASK);
    return;
} /* end of _r8389_intr_swcore_handler */

/* Function Name:
 *      _r8389_intr_nic_handler
 * Description:
 *      NIC interrupt handler function.
 * Input:
 *      pParam - The argument passed to interrupt handler at interrupt time.
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void
_r8389_intr_nic_handler(void *pParam)
{
    uint32  cpu_iisr;
    uint32  cur_tick;
    rtcore_ioctl_t* dio;

    dio = pParam;

    r8389_isr_handler(dio->data[2]);

    /*
     * reenable interrupts here.
     */
    ioal_mem32_write(0, RTL8389_CPU_INTERFACE_INTERRUPT_MASK_ADDR, 0xffffffff);
    return;
} /* end of _r8389_intr_nic_handler */
#endif

/* Function Name:
 *      drv_intr_link_stat_register
 * Description:
 *      Register the callback function for handling link state change interrupt handler.
 * Input:
 *      unit   - unit id
 *      fLinkStatCb - pointer to a handler of link state change
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
drv_intr_link_stat_register(uint32 unit, drv_intr_hdl_f fLinkStatCb)
{
    RT_PARAM_CHK(NULL == fLinkStatCb, RT_ERR_NULL_POINTER);

    if (NULL == fLink_stat_cb)
    {
        fLink_stat_cb = fLinkStatCb;
    }
    else
    {
        /* Handler is already existing */
        return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
} /* end of drv_intr_link_stat_register */

/* Function Name:
 *      drv_intr_link_stat_unregister
 * Description:
 *      Unregister the callback function for handling link state change interrupt handler.
 * Input:
 *      unit   - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 * Note:
 *      None
 */
int32
drv_intr_link_stat_unregister(uint32 unit)
{

    if (NULL != fLink_stat_cb)
    {
        fLink_stat_cb = NULL;
    }
    else
    {
        /* Handler is not existing */
        return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
} /* end of drv_intr_link_stat_unregister */


static void drv_intr_link_stat_hdl(uint32 unit, void* data)
{
    if (NULL != fLink_stat_cb)
        fLink_stat_cb(unit, data);
    return;
} /* end of drv_intr_link_stat_hdl */

#if defined(CONFIG_SDK_RTL8328)
/* Function Name:
 *      _r8328_intr_enable_set
 * Description:
 *      Enable interrupt with specified interrupt source of RTL8328 series
 * Input:
 *      unit        - unit id
 *      intr_source - interrupt source that is going to be enabled.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
static int32
_r8328_intr_enable_set(uint32 unit, drv_intr_source_t intr_source)
{
    uint32  value;

    RT_PARAM_CHK(intr_source >= INTR_SOURCE_END, RT_ERR_INPUT);

    switch (intr_source)
    {
        case LINK_CHANGE_INTR:
            ioal_mem32_read(unit, RTL8328_SWITCH_INTERRUPT_GLOBAL_CONTROL_ADDR, &value);
            value |= (RTL8328_SWITCH_INTERRUPT_GLOBAL_CONTROL_SWITCH_IE_MASK | RTL8328_SWITCH_INTERRUPT_GLOBAL_CONTROL_GLOBAL_LINK_STA_CHANGE_IE_MASK);
            ioal_mem32_write(unit, RTL8328_SWITCH_INTERRUPT_GLOBAL_CONTROL_ADDR, value);
            ioal_mem32_write(unit, RTL8328_PER_PORT_LINK_CHANGE_INTERRUPT_CONTROL_ADDR, 0x1fffffff);
            break;
        case MEDIA_CHANGE_INTR:
            break;
        case SPEED_CHANGE_INTR:
            break;
        case DUPLEX_CHANGE_INTR:
            break;
        default:
            break;
    }

    return RT_ERR_OK;
} /* end of _r8328_intr_enable_set */

/* Function Name:
 *      _r8328_intr_swcoreHandler
 * Description:
 *      Common interrupt handler function of 8328 series.
 * Input:
 *      pParam - The argument passed to interrupt handler at interrupt time.
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void
_r8328_intr_swcore_handler(void *pParam)
{
    uint32  intr_status;
    uint32  unit, i;

    intr_count++;
    unit = ((rtcore_ioctl_t *)pParam)->data[0];
    intr_status = ((rtcore_ioctl_t *)pParam)->data[2];

	for (i = 0; i < NUM_OF_R8328_INTR_HANDLER; i++)
    {
	    if (intr_status & r8328_intr_handlers[i].maskbit)
	    {
    		/* Bit found, dispatch interrupt */
    	    /* Assing interrupt status carried from kernel space */
            r8328_intr_handlers[i].intr_data[0] = ((rtcore_ioctl_t *)pParam)->data[3];
    		(r8328_intr_handlers[i].fIntr_handler)(unit, (void *)&r8328_intr_handlers[i].intr_data);
        }
	}

    /*
     * reenable interrupts here.
     */
    ioal_mem32_write(unit, RTL8328_PER_PORT_LINK_CHANGE_INTERRUPT_CONTROL_ADDR, 0x1fffffff);
    return;
} /* end of _r8328_intr_swcore_handler */

/* Function Name:
 *      _r8328_intr_nic_handler
 * Description:
 *      NIC interrupt handler function.
 * Input:
 *      pParam - The argument passed to interrupt handler at interrupt time.
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void
_r8328_intr_nic_handler(void *pParam)
{
    rtcore_ioctl_t* dio;

    dio = pParam;

    r8328_isr_handler(dio->data[2]);

    /*
     * reenable interrupts here.
     */
    ioal_mem32_write(0, RTL8328_CPUIIMR_ADDR, 0xffffffff);
    return;
} /* end of _r8328_intr_nic_handler */
#endif

#if defined(CONFIG_SDK_RTL8390)
/* Function Name:
 *      _r8390_intr_enable_set
 * Description:
 *      Enable interrupt with specified interrupt source of 8390 series
 * Input:
 *      unit        - unit id
 *      intr_source - interrupt source that is going to be enabled.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
static int32
_r8390_intr_enable_set(uint32 unit, drv_intr_source_t intr_source)
{
    RT_PARAM_CHK(intr_source >= INTR_SOURCE_END, RT_ERR_INPUT);

    switch (intr_source)
    {
        case LINK_CHANGE_INTR:
            //ioal_mem32_write(unit, RTL8390_IMR_PORT_LINK_STS_CHG_ADDR(0), 0xFFFFFFFF);
            //ioal_mem32_write(unit, RTL8390_IMR_PORT_LINK_STS_CHG_ADDR(32), 0xFFFFF);
            break;
        case MEDIA_CHANGE_INTR:
            break;
        case SPEED_CHANGE_INTR:
            break;
        case DUPLEX_CHANGE_INTR:
            break;
        default:
            break;
    }

    return RT_ERR_OK;
} /* end of _r8390_intr_enable_set */

/* Function Name:
 *      _r8390_intr_swcoreHandler
 * Description:
 *      Common interrupt handler function of 8390 series.
 * Input:
 *      pParam - The argument passed to interrupt handler at interrupt time.
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void
_r8390_intr_swcore_handler(void *pParam)
{
    uint32  intr_status;
    uint32  unit, i;

    intr_count++;
    unit = ((rtcore_ioctl_t *)pParam)->data[0];
    intr_status = ((rtcore_ioctl_t *)pParam)->data[2];

	for (i = 0; i < NUM_OF_R8390_INTR_HANDLER; i++)
    {
	    if (intr_status & r8390_intr_handlers[i].maskbit)
	    {
    		/* Bit found, dispatch interrupt */
    	    /* Assing interrupt status carried from kernel space */
            r8390_intr_handlers[i].intr_data[0] = ((rtcore_ioctl_t *)pParam)->data[3];
            r8390_intr_handlers[i].intr_data[1] = ((rtcore_ioctl_t *)pParam)->data[4];
    		(r8390_intr_handlers[i].fIntr_handler)(unit, (void *)&r8390_intr_handlers[i].intr_data);
        }
	}

    /*
     * re-enable interrupts here.
     */
    /* link change interrupt */
    ioal_mem32_write(unit, RTL8390_IMR_PORT_LINK_STS_CHG_ADDR(0), 0xFFFFFFFF);
    ioal_mem32_write(unit, RTL8390_IMR_PORT_LINK_STS_CHG_ADDR(32), 0xFFFFF);
    return;
} /* end of _r8390_intr_swcore_handler */

/* Function Name:
 *      _r8390_intr_nic_handler
 * Description:
 *      NIC interrupt handler function.
 * Input:
 *      pParam - The argument passed to interrupt handler at interrupt time.
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void
_r8390_intr_nic_handler(void *pParam)
{
    rtcore_ioctl_t* dio;

    dio = pParam;

    r8390_isr_handler(dio->data[2]);
    
#if defined(CONFIG_SDK_DRIVER_L2NTFY)
    if (dio->data[2] & INT_NTFY_DONE_MASK)
    {
        drv_swcore_l2_notification_usr_handler(dio->data[3], dio->data[4]);
    }
    if (dio->data[2] & INT_NTFY_BUF_RUN_OUT_MASK)
    {
        drv_swcore_l2_notification_buf_runout_handler(NULL);
    }
    if (dio->data[2] & INT_LOCAL_NTFY_BUF_RUN_OUT_MASK)
    {
        drv_swcore_l2_notification_localBuf_runout_handler(NULL);
    }
#endif

    /*
     * reenable interrupts here.
     */
    ioal_mem32_write(0, RTL8390_DMA_IF_INTR_MSK_ADDR, 0xffffffff);

    return;
} /* end of _r8390_intr_nic_handler */
#endif

#if defined(CONFIG_SDK_RTL8380)
/* Function Name:
 *      _r8380_intr_enable_set
 * Description:
 *      Enable interrupt with specified interrupt source of RTL8380 series
 * Input:
 *      unit        - unit id
 *      intr_source - interrupt source that is going to be enabled.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
static int32
_r8380_intr_enable_set(uint32 unit, drv_intr_source_t intr_source)
{
    RT_PARAM_CHK(intr_source >= INTR_SOURCE_END, RT_ERR_INPUT);

    switch (intr_source)
    {
        case LINK_CHANGE_INTR:
           //by cw ioal_mem32_write(unit, RTL8380_SWITCH_INTERRUPT_CONTROL0_ADDR, RTL8380_SWITCH_INTERRUPT_CONTROL0_LINK_STA_CHANGE_IE_MASK);
            break;
        case MEDIA_CHANGE_INTR:
            break;
        case SPEED_CHANGE_INTR:
            break;
        case DUPLEX_CHANGE_INTR:
            break;
        default:
            break;
    }

    return RT_ERR_OK;
} /* end of _r8380_intr_enable_set */

/* Function Name:
 *      _r8380_intr_swcoreHandler
 * Description:
 *      Common interrupt handler function of 8380 series.
 * Input:
 *      pParam - The argument passed to interrupt handler at interrupt time.
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void
_r8380_intr_swcore_handler(void *pParam)
{
    uint32  intr_status;
    uint32  unit, i;

    intr_count++;
    unit = ((rtcore_ioctl_t *)pParam)->data[0];
    intr_status = ((rtcore_ioctl_t *)pParam)->data[2];

	for (i = 0; i < NUM_OF_R8380_INTR_HANDLER; i++)
    {
	    if (intr_status & r8380_intr_handlers[i].maskbit)
	    {
    		/* Bit found, dispatch interrupt */
    	    /* Assing interrupt status carried from kernel space */
            r8380_intr_handlers[i].intr_data[0] = ((rtcore_ioctl_t *)pParam)->data[3];
    		(r8380_intr_handlers[i].fIntr_handler)(unit, (void *)&r8380_intr_handlers[i].intr_data);
        }
	}

    /*
     * reenable interrupts here.
     */
   //by cw ioal_mem32_write(unit, RTL8380_SWITCH_INTERRUPT_CONTROL0_ADDR, RTL8380_SWITCH_INTERRUPT_CONTROL0_LINK_STA_CHANGE_IE_MASK);
    return;
} /* end of _r8389_intr_swcore_handler */

/* Function Name:
 *      _r8380_intr_nic_handler
 * Description:
 *      NIC interrupt handler function.
 * Input:
 *      pParam - The argument passed to interrupt handler at interrupt time.
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void
_r8380_intr_nic_handler(void *pParam)
{
    rtcore_ioctl_t* dio;

    dio = pParam;

    r8380_isr_handler(dio->data[2]);

    /*
     * reenable interrupts here.
     */
    ioal_mem32_write(0, RTL8380_DMA_IF_INTR_MSK_ADDR, 0xffffffff);

    return;
} /* end of _r8380_intr_nic_handler */
#endif

