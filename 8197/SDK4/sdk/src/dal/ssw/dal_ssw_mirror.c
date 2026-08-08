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
 * $Revision: 21577 $
 * $Date: 2011-08-27 12:02:46 +0800 (Sat, 27 Aug 2011) $
 *
 * Purpose : Definition those public MIRROR APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) port mirroring
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
#include <hal/common/halctrl.h>
#include <dal/ssw/dal_ssw_mirror.h>
#include <rtk/mirror.h>

/* 
 * Symbol Definition 
 */
/* mirror information structure */
typedef struct rtk_mirror_info_s
{
    rtk_enable_t  status;           /* enabled or disabled */
    rtk_port_t  mirroring_port;
    int32   tx_dir_session_id;    /* Store session Id for TX */
    int32   rx_dir_session_id;    /* Store session Id for RX */
} rtk_mirror_info_t;

/* Mirror direction */
typedef enum rtk_mirror_direction_e
{
    MIRROR_TX = 0,
    MIRROR_RX,
    MIRROR_END
} rtk_mirror_direction_t;

/* 
 * Data Declaration 
 */
static uint32               mirror_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         mirror_sem[RTK_MAX_NUM_OF_UNIT];

static rtk_mirror_info_t    mirror_table[RTK_MAX_NUM_OF_UNIT][RTK_MAX_NUM_OF_MIRRORING_PORT];


const static uint16 mirroredPortmask_fieldidx[] = {SSW_SET0_MIRROREDf, SSW_SET1_MIRROREDf};
const static uint16 mirrorDir_fieldidx[] = {SSW_SET0_MIRR_DIRf, SSW_SET1_MIRR_DIRf};
const static uint16 mirrorPort_fieldidx[] = {SSW_SET0_MIRRORINGf, SSW_SET1_MIRRORINGf};

/*
 * Macro Definition
 */
/* vlan semaphore handling */
#define MIRROR_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(mirror_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_DAL|MOD_MIRROR), "semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)
#define MIRROR_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(mirror_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_DAL|MOD_MIRROR), "semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)


/* 
 * Function Declaration 
 */

static int32
_dal_ssw_mirror_singleDirEntry_get(
    uint32                  unit,
    uint32                  mirror_id,
    rtk_mirror_direction_t  *pDirection,
    uint32                  *pMirroring_port,
    rtk_portmask_t          *pMirrored_portmask);
    
static int32
_dal_ssw_mirror_singleDirEntry_set(
    uint32                  unit,
    uint32                  mirror_id,
    rtk_mirror_direction_t  direction,
    uint32                  mirroring_port,
    rtk_portmask_t          *pMirrored_portmask);

/* Module Name : Mirror */

/* Function Name:
 *      dal_ssw_mirror_init
 * Description:
 *      Initialize the mirroring database.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 * Note:
 *      Must initialize Mirror module before calling any Mirror APIs.
 */
int32
dal_ssw_mirror_init(uint32 unit)
{
    uint32  i;

    mirror_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    mirror_sem[unit] = osal_sem_mutex_create();
    if (0 == mirror_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_MIRROR), "semaphore create failed");
        return RT_ERR_FAILED;
    }
    
    MIRROR_SEM_LOCK(unit);
    
    for (i = 0; i < RTK_MAX_NUM_OF_MIRRORING_PORT; i++)
    {
        mirror_table[unit][i].status = DISABLED;
        mirror_table[unit][i].mirroring_port = 0;
        mirror_table[unit][i].tx_dir_session_id = i;
        mirror_table[unit][i].rx_dir_session_id = i + 1;
    }
    
    MIRROR_SEM_UNLOCK(unit);
    
    /* set init flag to complete init */
    mirror_init[unit] = INIT_COMPLETED;
    
    return RT_ERR_OK;
} /* end of dal_ssw_mirror_init */

/* Function Name:
 *      dal_ssw_mirror_portBased_create
 * Description:
 *      Create one mirroring session in the specified device.
 * Input:
 *      unit           - unit id
 *      mirroring_port - mirroring port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_PORT_ID           - invalid port id
 *      RT_ERR_MIRROR_PORT_FULL  - Exceeds maximum number of supported mirroring port
 *      RT_ERR_MIRROR_PORT_EXIST - mirroring port already exists
 * Note:
 */
int32
dal_ssw_mirror_portBased_create(uint32 unit, rtk_port_t mirroring_port)
{
    uint32  found;
    uint32  isExist;
    uint32  mirror_id;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, mirroring_port=%d", unit, mirroring_port);    
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, mirroring_port), RT_ERR_PORT_ID);
    
    found = FALSE;
    isExist = FALSE;
    MIRROR_SEM_LOCK(unit);
    for (mirror_id = 0; mirror_id < RTK_MAX_NUM_OF_MIRRORING_PORT; mirror_id++)
    {
        if (mirror_table[unit][mirror_id].status != ENABLED)
        {
            mirror_table[unit][mirror_id].status = ENABLED;
            mirror_table[unit][mirror_id].mirroring_port = mirroring_port;
            found = TRUE;
            break;
        }
        
        if (mirror_table[unit][mirror_id].mirroring_port == mirroring_port)
        {
            isExist = TRUE;
            break;
        }
        
    }
    
    MIRROR_SEM_UNLOCK(unit);
    
    if (TRUE == isExist)
    {
        return RT_ERR_MIRROR_PORT_EXIST;
    }
    
    if (FALSE == found)
    {
        return RT_ERR_MIRROR_PORT_FULL;
    }
    
    return RT_ERR_OK;
} /* end of dal_ssw_mirror_portBased_create */


/* Function Name:
 *      dal_ssw_mirror_portBased_destroy
 * Description:
 *      Destroy one mirroring session from the specified device.
 * Input:
 *      unit           - unit id
 *      mirroring_port - mirroring port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - invalid unit id
 *      RT_ERR_PORT_ID               - invalid port id
 *      RT_ERR_MIRROR_PORT_NOT_EXIST - mirroring port does not exists
 * Note:
 *      1. In RTL8329/RTL8389, if you have sess-0 (mirroring_port A with TX) and
 *         sess-1 (mirroring_port A with RX), remove mirroring_port A will
 *         remove those 2 sessions at the same time.
 */
int32
dal_ssw_mirror_portBased_destroy(uint32 unit, rtk_port_t mirroring_port)
{
    int32   ret;
    rtk_portmask_t  portmask;
    uint32  found;
    uint32  isExist;
    uint32  mirror_id;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, mirroring_port=%d", unit, mirroring_port); 
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, mirroring_port), RT_ERR_PORT_ID);
    
    found = FALSE;
    isExist = FALSE;
    MIRROR_SEM_LOCK(unit);
    for (mirror_id = 0; mirror_id < RTK_MAX_NUM_OF_MIRRORING_PORT; mirror_id++)
    {
        if (mirror_table[unit][mirror_id].mirroring_port == mirroring_port)
        {
            isExist = TRUE;
            break;
        }
    }
    
    if (TRUE != isExist)
    {
        MIRROR_SEM_UNLOCK(unit);
        return RT_ERR_MIRROR_PORT_NOT_EXIST;
    }
    
    osal_memset(&portmask, 0, sizeof(rtk_portmask_t));
    if ((ret = _dal_ssw_mirror_singleDirEntry_set(unit, mirror_table[unit][mirror_id].tx_dir_session_id, MIRROR_TX, 0, &portmask))
         != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    
    if ((ret = _dal_ssw_mirror_singleDirEntry_set(unit, mirror_table[unit][mirror_id].rx_dir_session_id, MIRROR_RX, 0, &portmask))
         != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    mirror_table[unit][mirror_id].mirroring_port = 0;
    mirror_table[unit][mirror_id].status = DISABLED;
    
    MIRROR_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_mirror_portBased_destroy */


/* Function Name:
 *      dal_ssw_mirror_portBased_destroyAll
 * Description:
 *      Destroy all mirroring sessions from the specified device.
 * Input:
 *      unit           - unit id
 *      mirroring_port - mirroring port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 */
int32
dal_ssw_mirror_portBased_destroyAll(uint32 unit)
{
    int32   ret;
    rtk_portmask_t  portmask;
    uint32  mirror_id;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d", unit); 
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    /* parameter check */
    
    MIRROR_SEM_LOCK(unit);
    
    for (mirror_id = 0; mirror_id < RTK_MAX_NUM_OF_MIRRORING_PORT; mirror_id++)
    {
        if (mirror_table[unit][mirror_id].status != ENABLED)
        {
            continue;
        }
        
        osal_memset(&portmask, 0, sizeof(rtk_portmask_t));
        if ((ret = _dal_ssw_mirror_singleDirEntry_set(unit, mirror_table[unit][mirror_id].tx_dir_session_id, MIRROR_TX, 0, &portmask))
             != RT_ERR_OK)
        {
            MIRROR_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
            return ret;
        }
        
        
        if ((ret = _dal_ssw_mirror_singleDirEntry_set(unit, mirror_table[unit][mirror_id].rx_dir_session_id, MIRROR_RX, 0, &portmask))
             != RT_ERR_OK)
        {
            MIRROR_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
            return ret;
        }
        
        mirror_table[unit][mirror_id].mirroring_port = 0;
        mirror_table[unit][mirror_id].status = DISABLED;
    }
    
    MIRROR_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_mirror_portBased_destroyAll */


/* Function Name:
 *      dal_ssw_mirror_portBased_get
 * Description:
 *      Get the mirroring session information by mirroring port from
 *      the specified device.
 * Input:
 *      unit                  - unit id
 *      mirroring_port        - mirroring port
 * Output:
 *      pMirrored_rx_portmask - pointer buffer of rx of mirrored ports
 *      pMirrored_tx_portmask - pointer buffer of tx of mirrored ports
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_PORT_ID                - invalid port id
 *      RT_ERR_NULL_POINTER           - input parameter may be null pointer
 *      RT_ERR_MIRROR_SESSION_NOEXIST - mirroring session not exist
 *      RT_ERR_MIRROR_PORT_NOT_EXIST  - mirroring port does not exists
 * Note:
 *      1. In RTL8329/RTL8389, if you have sess-0 (mirroring_port A with TX) and
 *         sess-1 (mirroring_port A with RX), get mirroring_port A will
 *         reply rx_portmask and tx_portmask at the same time.
 */
int32
dal_ssw_mirror_portBased_get(
    uint32          unit,
    rtk_port_t      mirroring_port,
    rtk_portmask_t  *pMirrored_rx_portmask,
    rtk_portmask_t  *pMirrored_tx_portmask)
{
    int32   ret;
    rtk_mirror_direction_t  direction;
    rtk_port_t  port;
    uint32  found;
    uint32  isExist;
    uint32  mirror_id;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, mirroring_port=%d",
           unit, mirroring_port); 
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, mirroring_port), RT_ERR_PORT_ID);
    
    found = FALSE;
    isExist = FALSE;
    MIRROR_SEM_LOCK(unit);
    for (mirror_id = 0; mirror_id < RTK_MAX_NUM_OF_MIRRORING_PORT; mirror_id++)
    {
        if (mirror_table[unit][mirror_id].mirroring_port == mirroring_port)
        {
            isExist = TRUE;
            break;
        }
    }
    
    if (TRUE != isExist)
    {
        MIRROR_SEM_UNLOCK(unit);
        return RT_ERR_MIRROR_PORT_NOT_EXIST;
    }
    
    osal_memset(pMirrored_tx_portmask, 0, sizeof(rtk_portmask_t));
    direction = 0;
    port = 0;
    if ((ret = _dal_ssw_mirror_singleDirEntry_get(unit, mirror_table[unit][mirror_id].tx_dir_session_id, &direction, &port, pMirrored_tx_portmask))
         != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    osal_memset(pMirrored_rx_portmask, 0, sizeof(rtk_portmask_t));
    direction = 0;
    port = 0;
    if ((ret = _dal_ssw_mirror_singleDirEntry_get(unit, mirror_table[unit][mirror_id].rx_dir_session_id, &direction, &port, pMirrored_rx_portmask))
         != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    MIRROR_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "pMirrored_rx_portmask[0]=%x, pMirrored_tx_portmask[0]=%x", 
           pMirrored_rx_portmask->bits[0], pMirrored_tx_portmask->bits[0]); 
    
    return RT_ERR_OK;
} /* end of dal_ssw_mirror_portBased_get */


/* Function Name:
 *      dal_ssw_mirror_portBased_set
 * Description:
 *      Set the mirroring session information by mirroring port to
 *      the specified device.
 * Input:
 *      unit                  - unit id
 *      mirroring_port        - mirroring port
 *      pMirrored_rx_portmask - rx of mirrored ports
 *      pMirrored_tx_portmask - tx of mirrored ports
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_PORT_ID                - invalid port id
 *      RT_ERR_MIRROR_SESSION_NOEXIST - mirroring session not exist
 *      RT_ERR_MIRROR_PORT_NOT_EXIST  - mirroring port does not exists
 * Note:
 */
int32
dal_ssw_mirror_portBased_set(
    uint32          unit,
    rtk_port_t      mirroring_port,
    rtk_portmask_t  *pMirrored_rx_portmask,
    rtk_portmask_t  *pMirrored_tx_portmask)
{
    int32   ret;
    uint32  found;
    uint32  isExist;
    uint32  mirror_id;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, mirroring_port=%d", unit, mirroring_port);

    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, mirroring_port), RT_ERR_PORT_ID);

    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR)," pMirrored_rx_portmask[0]=%x, pMirrored_tx_portmask[0]=%x", 
           pMirrored_rx_portmask->bits[0], pMirrored_tx_portmask->bits[0]);
    
    found = FALSE;
    isExist = FALSE;
    MIRROR_SEM_LOCK(unit);
    for (mirror_id = 0; mirror_id < RTK_MAX_NUM_OF_MIRRORING_PORT; mirror_id++)
    {
        if (mirror_table[unit][mirror_id].mirroring_port == mirroring_port)
        {
            isExist = TRUE;
            break;
        }
    }
    
    if (TRUE != isExist)
    {
        MIRROR_SEM_UNLOCK(unit);
        return RT_ERR_MIRROR_PORT_NOT_EXIST;
    }
    
    if ((ret = _dal_ssw_mirror_singleDirEntry_set(unit, mirror_table[unit][mirror_id].tx_dir_session_id, MIRROR_TX, mirroring_port, pMirrored_tx_portmask))
         != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    
    if ((ret = _dal_ssw_mirror_singleDirEntry_set(unit, mirror_table[unit][mirror_id].rx_dir_session_id, MIRROR_RX, mirroring_port, pMirrored_rx_portmask))
         != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    
    MIRROR_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_mirror_portBased_set */


/* Function Name:
 *      _dal_ssw_mirror_singleDirEntry_set
 * Description:
 *      Set the mirroring session information by mirror id to
 *      the specified device.
 * Input:
 *      unit               - unit id
 *      mirror_id          - mirror id
 *      direction          - direction for mirror
 *      mirroring_port     - mirroring port
 *      pMirrored_portmask - of mirrored ports
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 */
static int32
_dal_ssw_mirror_singleDirEntry_set(
    uint32                  unit,
    uint32                  mirror_id,
    rtk_mirror_direction_t  direction,
    rtk_port_t              mirroring_port,
    rtk_portmask_t          *pMirrored_portmask)
{
    int32   ret;
    rtk_ssw_reg_list_t  reg_idx;
    uint32  value;

    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_MIRROR), "unit=%d, mirror_id=%d \
           direction=%d, mirroring_port=%d, pMirrored_portmask[0]=%x", 
           unit, mirror_id, direction, mirroring_port, 
           pMirrored_portmask->bits[0]); 
    
    RT_PARAM_CHK((mirror_id >= HAL_MAX_NUM_OF_MIRROR(unit)), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pMirrored_portmask), RT_ERR_NULL_POINTER);
    
    /* get value from CHIP*/
    if ((ret = reg_read(unit, SSW_PORT_MIRROR_CONTROLr, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    /* set direction */
    if ((ret = reg_field_set(unit, SSW_PORT_MIRROR_CONTROLr, mirrorDir_fieldidx[mirror_id], &direction, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    /* set mirroring port*/
    if ((ret = reg_field_set(unit, SSW_PORT_MIRROR_CONTROLr, mirrorPort_fieldidx[mirror_id], &mirroring_port, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    /* program value to chip */
    if ((ret = reg_write(unit, SSW_PORT_MIRROR_CONTROLr, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    reg_idx = SSW_PORT_MIRROR_SET_0_MIRRORED_BITMAP_CONTROLr + mirror_id;
    
    /* program mirrored portmask to CHIP*/
    if ((ret = reg_field_write(unit, reg_idx, mirroredPortmask_fieldidx[mirror_id], &(pMirrored_portmask->bits[0]))) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    return RT_ERR_OK;
} /* end of _dal_ssw_mirror_singleDirEntry_set */

/* Function Name:
 *      _dal_ssw_mirror_singleDirEntry_get
 * Description:
 *      Get the mirroring session information by mirror id from
 *      the specified device.
 * Input:
 *      unit               - unit id
 *      mirror_id          - mirror id
 *      pDirection         - buffer for store direction for mirror
 *      pMirroring_port    - buffer for store mirroring port
 *      pMirrored_portmask - buffer for store mirrored ports
 * Output:                     
 *      pDirection         - direction for mirror
 *      pMirroring_port    - mirroring port
 *      pMirrored_portmask - mirrored ports
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 */
static int32
_dal_ssw_mirror_singleDirEntry_get(
    uint32                  unit,
    uint32                  mirror_id,
    rtk_mirror_direction_t  *pDirection,
    rtk_port_t              *pMirroring_port,
    rtk_portmask_t          *pMirrored_portmask)
{
    int32   ret;
    rtk_ssw_reg_list_t  reg_idx;
    uint32  value;
    
    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_MIRROR), "unit=%d, mirror_id=%d",
           unit, mirror_id);     
    
    RT_PARAM_CHK((mirror_id >= HAL_MAX_NUM_OF_MIRROR(unit)), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pMirrored_portmask), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pDirection), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pDirection), RT_ERR_NULL_POINTER);
    
    /* get value from CHIP*/
    if ((ret = reg_read(unit, SSW_PORT_MIRROR_CONTROLr, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    /* program direction to CHIP*/
    if ((ret = reg_field_get(unit, SSW_PORT_MIRROR_CONTROLr, mirrorDir_fieldidx[mirror_id], pDirection, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    /* program mirroring port to CHIP*/
    if ((ret = reg_field_get(unit, SSW_PORT_MIRROR_CONTROLr, mirrorPort_fieldidx[mirror_id], pMirroring_port, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    reg_idx = SSW_PORT_MIRROR_SET_0_MIRRORED_BITMAP_CONTROLr + mirror_id;
    
    /* program mirrored portmask to CHIP*/
    if ((ret = reg_field_read(unit, reg_idx, mirroredPortmask_fieldidx[mirror_id], &(pMirrored_portmask->bits[0]))) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_MIRROR), "direction=%d, mirroring_port=%d, \
           pMirrored_portmask[0]=%x", *pDirection, *pMirroring_port, 
           pMirrored_portmask->bits[0]); 
    
    return RT_ERR_OK;
} /* end of _dal_ssw_mirror_singleDirEntry_get */
