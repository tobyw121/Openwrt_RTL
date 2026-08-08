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
 * $Revision:  $
 * $Date:  $
 *
 * Purpose : Definition of TIME API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) IEEE 1588
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
#include <hal/common/miim.h>
#include <hal/common/halctrl.h>
#include <dal/ssw/dal_ssw_time.h>
#include <rtk/default.h>
#include <rtk/time.h>

/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */
static uint32               time_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         time_sem[RTK_MAX_NUM_OF_UNIT];

/*
 * Macro Definition
 */
/* time semaphore handling */
#define TIME_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(time_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_DAL|MOD_TIME),"semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)
#define TIME_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(time_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_DAL|MOD_TIME),"semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)
#define RT_IF_ERR_GOTO_HANDLE(op, errHandle, ret) \
do {\
    if ((ret = (op)) != RT_ERR_OK)\
        goto errHandle;\
} while(0)


/*
 * Function Declaration
 */

/* Module Name : TIME */

/* Function Name:
 *      dal_ssw_time_init
 * Description:
 *      Initialize ptp module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Must initialize ptp module before calling any ptp APIs.
 */
int32
dal_ssw_time_init(uint32 unit)
{
    time_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    time_sem[unit] = osal_sem_mutex_create();
    if (0 == time_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_TIME), "semaphore create failed");
        return RT_ERR_FAILED;
    }

    /* set init flag to complete init */
    time_init[unit] = INIT_COMPLETED;

    return RT_ERR_OK;
} /* end of dal_ssw_time_init */


/* Function Name:
 *      _dal_ssw_time_phyReg_get
 * Description:
 *      Get PHY register data of the specific port
 * Input:
 *      unit                - unit id
 *      port                - port id
 *      reg_addr           - reg address
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
_dal_ssw_time_phyReg_get(
    uint32              unit,
    rtk_port_t          port,
    uint32              reg_addr,
    uint32              *pData)
{
    int32 ret;
    uint32 base_addr = 0x1600;
    uint32 reg, page_base;
    rtk_port_t port_base;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, reg_addr=0x%x, reg=0x%x", 
           unit, port, reg_addr);   
    
    /* check Init status */
    RT_INIT_CHK(time_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);

    reg = (reg_addr - base_addr) % 8 + 16;
    page_base = reg_addr - (reg_addr % 8);
    port_base = port / 8;

    if ((ret = hal_miim_write(unit, port_base, 31, 29, 8)) != RT_ERR_OK)
    {
        return ret;
    }
    if ((ret = hal_miim_write(unit, port_base, 31, 31, page_base)) != RT_ERR_OK)
    {
        return ret;
    }
    if ((ret = hal_miim_read(unit, port_base, 31, reg, pData)) != RT_ERR_OK)
    {
        return ret;
    }
    if ((ret = hal_miim_write(unit, port_base, 31, 29, 0)) != RT_ERR_OK)
    {
        return ret;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pData=0x%x", *pData);        

    return RT_ERR_OK;
}

/* Function Name:
 *      _dal_ssw_time_phyReg_set
 * Description:
 *      Set PHY register data of the specific port
 * Input:
 *      unit               - unit id
 *      port               - port id
 *      reg_addr           - reg address
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
_dal_ssw_time_phyReg_set(uint32 unit, uint32 port, uint32 reg_addr, uint32 reg_val)
{
    int32 ret;
    uint32 base_addr = 0x1600;
    uint32 reg, page_base;
    rtk_port_t port_base;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TIME), "unit=%d, port=%d, reg_addr=0x%x, reg_val=0x%x", 
            unit, port, reg_addr, reg_val);
    
    /* check Init status */
    RT_INIT_CHK(time_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    reg = (reg_addr - base_addr) % 8 + 16;
    page_base = reg_addr - (reg_addr % 8);
    port_base = port / 8;

    if ((ret = hal_miim_write(unit, port_base, 31, 29, 8)) != RT_ERR_OK)
    {
        return ret;
    }
    if ((ret = hal_miim_write(unit, port_base, 31, 31, page_base)) != RT_ERR_OK)
    {
        return ret;
    }
    if ((ret = hal_miim_write(unit, port_base, 31, reg, reg_val)) != RT_ERR_OK)
    {
        return ret;
    }
    if ((ret = hal_miim_write(unit, port_base, 31, 29, 0)) != RT_ERR_OK)
    {
        return ret;
    }

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_ssw_time_portPtpEnable_get
 * Description:
 *      Get TIME status of the specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_PORT          - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_ssw_time_portPtpEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32 ret;
    uint32 data, reg_addr, base_addr = 0x161c;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TIME), "unit=%d, port=%d", unit, port);

    /* check Init status */
    RT_INIT_CHK(time_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    reg_addr = base_addr + (port * 16);

    TIME_SEM_LOCK(unit);

    /* get value */
    if ((ret = _dal_ssw_time_phyReg_get(unit, port, reg_addr, &data)) != RT_ERR_OK)
    {
        TIME_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TIME), "");
        return ret;
    }

    TIME_SEM_UNLOCK(unit);

    if (data & 0x0100) {
        *pEnable = ENABLED;
    } else {
        *pEnable = DISABLED;
    }

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_ssw_time_portPtpEnable_set
 * Description:
 *      Set TIME status of the specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      enable  - status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT     - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
dal_ssw_time_portPtpEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32 ret;
    uint32 data, reg_addr, base_addr = 0x161c;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TIME), "unit=%d, port=%d, enable=%d", unit, port, enable);

    /* check Init status */
    RT_INIT_CHK(time_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);

    reg_addr = base_addr + (port * 16);

    TIME_SEM_LOCK(unit);

    /* get value */
    if ((ret = _dal_ssw_time_phyReg_get(unit, port, reg_addr, &data)) != RT_ERR_OK)
    {
        TIME_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TIME), "");
        return ret;
    }

    if (ENABLED == enable)
        data |= 0x0100;
    else
        data &= ~(0x0100);

    /* set value */
    if ((ret = _dal_ssw_time_phyReg_set(unit, port, reg_addr, data)) != RT_ERR_OK)
    {
        TIME_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TIME), "");
        return ret;
    }

    TIME_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_ssw_time_portPtpRxTimestamp_get
 * Description:
 *      Get TIME timstamp of the TIME identifier of the dedicated port from the specified device.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      identifier - indentifier of TIME packet
 * Output:
 *      pTimeStamp - pointer buffer of TIME timestamp
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      None
 */
int32
dal_ssw_time_portPtpRxTimestamp_get(
    uint32 unit,
    rtk_port_t port,
    rtk_time_ptpIdentifier_t identifier,
    rtk_time_timeStamp_t *pTimeStamp)
{
    int32 ret;
    uint32 data, reg_addr, base_addr; 
    uint32 sec_h, sec_l, nsec_h, nsec_l;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TIME), "unit=%d, port=%d, msgType=%d, sequenceId=0x%x", 
            unit, port, identifier.msgType, identifier.sequenceId);

    /* check Init status */
    RT_INIT_CHK(time_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);

    base_addr = 0x1614;
    base_addr += identifier.msgType;
    reg_addr = base_addr + (port * 16);

    TIME_SEM_LOCK(unit);
    
    /* get sequence ID */
    if ((ret = _dal_ssw_time_phyReg_get(unit, port, reg_addr, &data)) != RT_ERR_OK)
    {
        TIME_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TIME), "");
        return ret;
    }
    TIME_SEM_UNLOCK(unit);

    /* Input sequence ID NEED match currently sequence ID of chip */
    if (data != identifier.sequenceId)
    {
        return RT_ERR_INPUT;
    }

    /* prepare address value */
    base_addr = 0x1618;
    reg_addr = base_addr + (port * 16);

    TIME_SEM_LOCK(unit);

    /* Get value */
    if ((ret = _dal_ssw_time_phyReg_get(unit, port, (reg_addr + 3), &sec_h)) != RT_ERR_OK)
    {
        TIME_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TIME), "");
        return ret;
    }
    if ((ret = _dal_ssw_time_phyReg_get(unit, port, (reg_addr + 2), &sec_l)) != RT_ERR_OK)
    {
        TIME_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TIME), "");
        return ret;
    }
    if ((ret = _dal_ssw_time_phyReg_get(unit, port, (reg_addr + 1), &nsec_h)) != RT_ERR_OK)
    {
        TIME_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TIME), "");
        return ret;
    }
    if ((ret = _dal_ssw_time_phyReg_get(unit, port, (reg_addr + 0), &nsec_l)) != RT_ERR_OK)
    {
        TIME_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TIME), "");
        return ret;
    }

    TIME_SEM_UNLOCK(unit);

    pTimeStamp->sec = (sec_h << 8) | (sec_l & 0xFFFF);
    pTimeStamp->nsec = (((nsec_h & 0x7FF) << 8) | (nsec_l & 0xFFFF)) << 3; /* convert 8nsec to nsec */

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_ssw_time_portPtpTxTimestampCallback_register
 * Description:
 *      Register TIME transmission callback function of the TIME identifier of the dedicated port to the specified device.
 * Input:
 *      unit              - unit id
 *      port              - port id
 *      identifier        - indentifier of TIME packet
 *      rtk_ptp_time_cb_f - callback function
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
dal_ssw_time_portPtpTxTimestampCallback_register(
    uint32 unit,
    rtk_port_t port,
    rtk_time_ptpIdentifier_t identifier,
    rtk_time_ptpTime_cb_f *fCb)
{
    /* need to implement after ISR handler ready */

    /* lowlevel must create a thread to notify which has registered callback function */

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_ssw_time_refTime_get
 * Description:
 *      Get the reference time of TIME of the specified device.
 * Input:
 *      unit  - unit id
 * Output:
 *      pTimeStamp - pointer buffer of TIME reference time
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_ssw_time_refTime_get(uint32 unit, rtk_time_timeStamp_t *pTimeStamp)
{
    int32 ret;
    rtk_port_t max_port, port;
    uint32 data = 0;
    uint32 sec_h, sec_l, nsec_h = 0, nsec_l;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TIME), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(time_init[unit]);

    /* execute writing command, [15] = 0x1 executing, [13:12] = 0x0 read */
    data = 0x8000;

    TIME_SEM_LOCK(unit);
    
    max_port = HAL_GET_MAX_PORT(unit);
    for (port = 0; port < max_port; port = port + 8)
    {
        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }

        if ((ret = _dal_ssw_time_phyReg_set(unit, port, 0x1601, data)) != RT_ERR_OK)
        {
            TIME_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TIME), "");
            return ret;
        }

        /* check status */
        do {
            if ((ret = _dal_ssw_time_phyReg_get(unit, port, 0x1601, &data)) != RT_ERR_OK)
            {
                TIME_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_TIME), "");
                return ret;
            }
        } while ((data >> 15) & 0x1); /* busy watting */

        if ((ret = _dal_ssw_time_phyReg_get(unit, port, 0x160d, &sec_h)) != RT_ERR_OK)
        {
            TIME_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TIME), "");
            return ret;
        }
        if ((ret = _dal_ssw_time_phyReg_get(unit, port, 0x160c, &sec_l)) != RT_ERR_OK)
        {
            TIME_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TIME), "");
            return ret;
        }
        if ((ret = _dal_ssw_time_phyReg_get(unit, port, 0x160b, &nsec_h)) != RT_ERR_OK)
        {
            TIME_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TIME), "");
            return ret;
        }
        if ((ret = _dal_ssw_time_phyReg_get(unit, port, 0x160a, &nsec_l)) != RT_ERR_OK)
        {
            TIME_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TIME), "");
            return ret;
        }

        break; /* Get first phy chip value ONLY */
    }

    TIME_SEM_UNLOCK(unit);

    pTimeStamp->sec = (sec_h << 8) | (sec_l & 0xFFFF);
    pTimeStamp->nsec = (((nsec_h & 0x7FF) << 8) | (nsec_l & 0xFFFF)) << 3; /* convert 8nsec to nsec */

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_ssw_time_refTime_set
 * Description:
 *      Set the reference time of TIME of the specified device.
 * Input:
 *      unit      - unit id
 *      timeStamp - reference timestamp value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 * Note:
 *      None
 */
int32
dal_ssw_time_refTime_set(uint32 unit, rtk_time_timeStamp_t timeStamp)
{
    int32 ret;
    rtk_port_t max_port, port;
    uint32 data = 0, val;
    uint32 sec_h, sec_l, nsec_h, nsec_l;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TIME), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(time_init[unit]);

    sec_h = (timeStamp.sec) >> 8;
    sec_l = (timeStamp.sec) & 0xFFFF;
    /* convert nsec to 8nsec */
    nsec_h = (timeStamp.nsec >> 3) >> 8; 
    nsec_l = (timeStamp.nsec >> 3) & 0xFFFF;

    /* execute writing command, [15] = 0x1 executing, [13:12] = 0x1 write */
    data = nsec_h | 0x9000;

    TIME_SEM_LOCK(unit);
    
    max_port = HAL_GET_MAX_PORT(unit);
    for (port = 0; port < max_port; port = port + 8)
    {
        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }

        if ((ret = _dal_ssw_time_phyReg_set(unit, port, 0x1603, sec_h)) != RT_ERR_OK)
        {
            TIME_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TIME), "");
            return ret;
        }
        if ((ret = _dal_ssw_time_phyReg_set(unit, port, 0x1602, sec_l)) != RT_ERR_OK)
        {
            TIME_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TIME), "");
            return ret;
        }
        if ((ret = _dal_ssw_time_phyReg_set(unit, port, 0x1600, nsec_l)) != RT_ERR_OK)
        {
            TIME_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TIME), "");
            return ret;
        }

        if ((ret = _dal_ssw_time_phyReg_set(unit, port, 0x1601, data)) != RT_ERR_OK)
        {
            TIME_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TIME), "");
            return ret;
        }

        do {
            if ((ret = _dal_ssw_time_phyReg_get(unit, port, 0x1601, &val)) != RT_ERR_OK)
            {
                TIME_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_TIME), "");
                return ret;
            }
        } while ((val >> 15) & 0x1); /* busy watting */
    }

    TIME_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_ssw_ptp_timeAdjust_set
 * Description:
 *      Adjust TIME reference time.
 * Input:
 *      unit      - unit id
 *      sign      - significant
 *      timeStamp - reference timestamp value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      None
 */
int32
dal_ssw_time_refTimeAdjust_set(uint32 unit, uint32 sign, rtk_time_timeStamp_t timeStamp)
{
    int32 ret;
    rtk_port_t max_port, port;
    uint32 data = 0, val;
    uint32 sec_h, sec_l, nsec_h, nsec_l;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TIME), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(time_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(sign >= 2, RT_ERR_INPUT);

    sec_h = (timeStamp.sec) >> 8;
    sec_l = (timeStamp.sec) & 0xFFFF;
    /* convert nsec to 8nsec */
    nsec_h = (timeStamp.nsec >> 3) >> 8; 
    nsec_l = (timeStamp.nsec >> 3) & 0xFFFF;

    /* execute writing command, CMD[15] = 0x1:executing, CMD[13:12] = 0x2:Increase, 0x3:Decrease */
    data = (sign == 0) ? (nsec_h | 0xA000) : (nsec_h | 0xB000);   

    TIME_SEM_LOCK(unit);
    
    max_port = HAL_GET_MAX_PORT(unit);
    for (port = 0; port < max_port; port = port + 8)
    {
        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }

            if ((ret = _dal_ssw_time_phyReg_set(unit, port, 0x1602, sec_l)) != RT_ERR_OK)
            {
                TIME_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_TIME), "");
                return ret;
            }
            if ((ret = _dal_ssw_time_phyReg_set(unit, port, 0x1603, sec_h)) != RT_ERR_OK)
            {
                TIME_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_TIME), "");
                return ret;
            }
            if ((ret = _dal_ssw_time_phyReg_set(unit, port, 0x1600, nsec_l)) != RT_ERR_OK)
            {
                TIME_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_TIME), "");
                return ret;
            }

            if ((ret = _dal_ssw_time_phyReg_set(unit, port, 0x1601, data)) != RT_ERR_OK)
            {
                TIME_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_TIME), "");
                return ret;
            }

        do {
            if ((ret = _dal_ssw_time_phyReg_get(unit, port, 0x1601, &val)) != RT_ERR_OK)
            {
                TIME_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_TIME), "");
                return ret;
            }
        } while ((val >> 15) & 0x1); /* busy watting */
    }

    TIME_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_ssw_time_refTimeEnable_get
 * Description:
 *      Get the enable state of reference time of the specified device.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_ssw_time_refTimeEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32 ret;
    rtk_port_t max_port, port;
    uint32 data;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TIME), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(time_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    TIME_SEM_LOCK(unit);
    
    max_port = HAL_GET_MAX_PORT(unit);
    for (port = 0; port < max_port; port = port + 8)
    {
        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }

/*        if (HAL_IS_PHY_EXIST(unit, port) && (PHY_MODEL_ID_RTL8218 == pHalCtrl->pPhy_ctrl[port]->phy_model_id)) */
        if (HAL_IS_PHY_EXIST(unit, port))
        {
            if ((ret = _dal_ssw_time_phyReg_get(unit, port, 0x1604, &data)) != RT_ERR_OK)
            {
                TIME_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_TIME), "");
                return ret;
            }

            break;
        }
    }

    TIME_SEM_UNLOCK(unit);

    if (data & 0x0006){
        *pEnable = ENABLED;
    } else {
        *pEnable = DISABLED;
    }

    return RT_ERR_OK;
} 

/* Function Name:
 *      dal_ssw_time_refTimeEnable_set
 * Description:
 *      Set the enable state of reference time of the specified device.
 * Input:
 *      unit   - unit id
 *      enable - status
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
dal_ssw_time_refTimeEnable_set(uint32 unit, rtk_enable_t enable)
{
    int32 ret;
    rtk_port_t max_port, port;
    uint32 data;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TIME), "unit=%d, enable=%d", unit, enable);

    /* check Init status */
    RT_INIT_CHK(time_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);

    TIME_SEM_LOCK(unit);
    
    max_port = HAL_GET_MAX_PORT(unit);
    for (port = 0; port < max_port; port = port + 8)
    {
        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }

/*        if (HAL_IS_PHY_EXIST(unit, port) && (PHY_MODEL_ID_RTL8218 == pHalCtrl->pPhy_ctrl[port]->phy_model_id)) */
        if (HAL_IS_PHY_EXIST(unit, port))
        {
            if ((ret = _dal_ssw_time_phyReg_get(unit, port, 0x1604, &data)) != RT_ERR_OK)
            {
                TIME_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_TIME), "");
                return ret;
            }

            if (ENABLED == enable)
                data |= 0x0006;
            else
                data &= ~(0x0006);

            if ((ret = _dal_ssw_time_phyReg_set(unit, port, 0x1604, data)) != RT_ERR_OK)
            {
                TIME_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_TIME), "");
                return ret;
            }
        }
    }

    TIME_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}


