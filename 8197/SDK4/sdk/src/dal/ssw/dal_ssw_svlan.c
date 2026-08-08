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
 * Purpose : Definition those public svlan APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) svlan
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
#include <dal/ssw/dal_ssw_svlan.h>
#include <rtk/vlan.h>
#include <rtk/svlan.h>

/*
 * Symbol Definition
 */
/* svlan information structure */
typedef struct dal_ssw_svlan_info_s
{
    uint32 count;               /* count of valid svlan number    */
    uint32 *pValid_lists;       /* valid bit for this table       */
    uint16 *pSvid2tblindex;     /* table index of svid, 0:invalid */
} dal_ssw_svlan_info_t;


/*
 * Data Declaration
 */
static uint32               svlan_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         svlan_sem[RTK_MAX_NUM_OF_UNIT];
static dal_ssw_svlan_info_t *pDal_ssw_svlan_info[RTK_MAX_NUM_OF_UNIT];

const static uint16 svlan_port_regidx[] =
{  SSW_SVLAN_PORT0r, SSW_SVLAN_PORT0r\
 , SSW_SVLAN_PORT1r, SSW_SVLAN_PORT1r\
 , SSW_SVLAN_PORT2r, SSW_SVLAN_PORT2r\
 , SSW_SVLAN_PORT3r, SSW_SVLAN_PORT3r\
 , SSW_SVLAN_PORT4r, SSW_SVLAN_PORT4r\
 , SSW_SVLAN_PORT5r, SSW_SVLAN_PORT5r\
 , SSW_SVLAN_PORT6r, SSW_SVLAN_PORT6r\
 , SSW_SVLAN_PORT7r, SSW_SVLAN_PORT7r\
 , SSW_SVLAN_PORT8r, SSW_SVLAN_PORT8r\
 , SSW_SVLAN_PORT9r, SSW_SVLAN_PORT9r\
 , SSW_SVLAN_PORT10r, SSW_SVLAN_PORT10r\
 , SSW_SVLAN_PORT11r, SSW_SVLAN_PORT11r\
 , SSW_SVLAN_PORT12r, SSW_SVLAN_PORT12r\
 , SSW_SVLAN_PORT13r, SSW_SVLAN_PORT13r\
 , SSW_SVLAN_PORT14r};

const static uint16 svlan_id_regidx[] =
{  SSW_SVLAN_ID0r, SSW_SVLAN_ID0r\
 , SSW_SVLAN_ID1r, SSW_SVLAN_ID1r\
 , SSW_SVLAN_ID2r, SSW_SVLAN_ID2r\
 , SSW_SVLAN_ID3r, SSW_SVLAN_ID3r\
 , SSW_SVLAN_ID4r, SSW_SVLAN_ID4r\
 , SSW_SVLAN_ID5r, SSW_SVLAN_ID5r\
 , SSW_SVLAN_ID6r, SSW_SVLAN_ID6r\
 , SSW_SVLAN_ID7r, SSW_SVLAN_ID7r\
 , SSW_SVLAN_ID8r, SSW_SVLAN_ID8r\
 , SSW_SVLAN_ID9r, SSW_SVLAN_ID9r\
 , SSW_SVLAN_ID10r, SSW_SVLAN_ID10r\
 , SSW_SVLAN_ID11r, SSW_SVLAN_ID11r\
 , SSW_SVLAN_ID12r, SSW_SVLAN_ID12r\
 , SSW_SVLAN_ID13r, SSW_SVLAN_ID13r\
 , SSW_SVLAN_ID14r, SSW_SVLAN_ID14r\
 , SSW_SVLAN_ID15r, SSW_SVLAN_ID15r\
 , SSW_SVLAN_ID16r, SSW_SVLAN_ID16r\
 , SSW_SVLAN_ID17r, SSW_SVLAN_ID17r\
 , SSW_SVLAN_ID18r, SSW_SVLAN_ID18r\
 , SSW_SVLAN_ID19r, SSW_SVLAN_ID19r\
 , SSW_SVLAN_ID20r, SSW_SVLAN_ID20r\
 , SSW_SVLAN_ID21r, SSW_SVLAN_ID21r\
 , SSW_SVLAN_ID22r, SSW_SVLAN_ID22r\
 , SSW_SVLAN_ID23r, SSW_SVLAN_ID23r\
 , SSW_SVLAN_ID24r, SSW_SVLAN_ID24r\
 , SSW_SVLAN_ID25r, SSW_SVLAN_ID25r\
 , SSW_SVLAN_ID26r, SSW_SVLAN_ID26r\
 , SSW_SVLAN_ID27r, SSW_SVLAN_ID27r\
 , SSW_SVLAN_ID28r, SSW_SVLAN_ID28r\
 , SSW_SVLAN_ID29r, SSW_SVLAN_ID29r\
 , SSW_SVLAN_ID30r, SSW_SVLAN_ID30r\
 , SSW_SVLAN_ID31r, SSW_SVLAN_ID31r};

const static uint16 spvid_fieldidx[]={SSW_SPVID0f, SSW_SPVID1f, SSW_SPVID2f, SSW_SPVID3f, SSW_SPVID4f, SSW_SPVID5f, SSW_SPVID6f, SSW_SPVID7f, SSW_SPVID8f, SSW_SPVID9f,         \
                                     SSW_SPVID10f, SSW_SPVID11f, SSW_SPVID12f, SSW_SPVID13f, SSW_SPVID14f, SSW_SPVID15f, SSW_SPVID16f, SSW_SPVID17f, SSW_SPVID18f, SSW_SPVID19f,\
                                     SSW_SPVID20f, SSW_SPVID21f, SSW_SPVID22f, SSW_SPVID23f, SSW_SPVID24f, SSW_SPVID25f, SSW_SPVID26f, SSW_SPVID27f, SSW_SPVID28f};
const static uint16 svid_fieldidx[]={SSW_SVID0f, SSW_SVID1f, SSW_SVID2f, SSW_SVID3f, SSW_SVID4f, SSW_SVID5f, SSW_SVID6f, SSW_SVID7f, SSW_SVID8f, SSW_SVID9f,         \
                                     SSW_SVID10f, SSW_SVID11f, SSW_SVID12f, SSW_SVID13f, SSW_SVID14f, SSW_SVID15f, SSW_SVID16f, SSW_SVID17f, SSW_SVID18f, SSW_SVID19f,\
                                     SSW_SVID20f, SSW_SVID21f, SSW_SVID22f, SSW_SVID23f, SSW_SVID24f, SSW_SVID25f, SSW_SVID26f, SSW_SVID27f, SSW_SVID28f, SSW_SVID29f,\
                                     SSW_SVID30f, SSW_SVID31f, SSW_SVID32f, SSW_SVID33f, SSW_SVID34f, SSW_SVID35f, SSW_SVID36f, SSW_SVID37f, SSW_SVID38f, SSW_SVID39f,\
                                     SSW_SVID40f, SSW_SVID41f, SSW_SVID42f, SSW_SVID43f, SSW_SVID44f, SSW_SVID45f, SSW_SVID46f, SSW_SVID47f, SSW_SVID48f, SSW_SVID49f,\
                                     SSW_SVID50f, SSW_SVID51f, SSW_SVID52f, SSW_SVID53f, SSW_SVID54f, SSW_SVID55f, SSW_SVID56f, SSW_SVID57f, SSW_SVID58f, SSW_SVID59f,\
                                     SSW_SVID60f, SSW_SVID61f, SSW_SVID62f, SSW_SVID63f};
/*
 * Macro Declaration
 */
#define SVLAN_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(svlan_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_DAL|MOD_SVLAN), "semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)
#define SVLAN_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(svlan_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_DAL|MOD_SVLAN), "semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)

#define SVLAN_ENTRY_NOT_USED    0xffff




/*
 * Function Declaration
 */
static int32 _dal_ssw_svlan_addTblValidLists(uint32 unit, uint32 *pIdx);
static int32 _dal_ssw_setSvlan(uint32 unit, rtk_svlan_data_t *pSvlan_entry);
static int32 _dal_ssw_getSvlan(uint32 unit, rtk_svlan_data_t *pSvlan_entry);
/* Module Name : SVLAN */

/* Function Name:
 *      dal_ssw_svlan_init
 * Description:
 *      Initialize svlan module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      Must initialize svlan module before calling any svlan APIs.
 */
int32
dal_ssw_svlan_init(uint32 unit)
{
    int32   ret;
    uint32  svlan_tableSize;
    uint32  num_of_svlan_1bitlist;


    svlan_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    svlan_sem[unit] = osal_sem_mutex_create();
    if (0 == svlan_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_SVLAN), "semaphore create failed");
        return RT_ERR_FAILED;
    }

    if ((ret = table_size_get(unit, SSW_SVLAN_TABLEt, &svlan_tableSize)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }
    num_of_svlan_1bitlist = (svlan_tableSize + 31) >> 5;

    SVLAN_SEM_LOCK(unit);

    /* allocate memory for each database and initilize database */
    pDal_ssw_svlan_info[unit] = (dal_ssw_svlan_info_t *)osal_alloc(sizeof(dal_ssw_svlan_info_t));
    if (NULL == pDal_ssw_svlan_info[unit])
    {
        SVLAN_SEM_UNLOCK(unit);
        RT_ERR(RT_ERR_FAILED, (MOD_SVLAN|MOD_DAL), "memory allocate failed");
        return RT_ERR_FAILED;
    }
    osal_memset(pDal_ssw_svlan_info[unit], 0, sizeof(dal_ssw_svlan_info_t));

    pDal_ssw_svlan_info[unit]->pValid_lists = (uint32 *)osal_alloc(num_of_svlan_1bitlist * sizeof(uint32));
    if (NULL == pDal_ssw_svlan_info[unit]->pValid_lists)
    {
        osal_free(pDal_ssw_svlan_info[unit]);
        pDal_ssw_svlan_info[unit] = 0;
        SVLAN_SEM_UNLOCK(unit);
        RT_ERR(RT_ERR_FAILED, (MOD_SVLAN|MOD_DAL), "memory allocate failed");
        return RT_ERR_FAILED;
    }

    pDal_ssw_svlan_info[unit]->pSvid2tblindex = (uint16 *)osal_alloc(RTK_MAX_NUM_OF_SVLAN_ID * sizeof(uint16));

    if (NULL == pDal_ssw_svlan_info[unit]->pSvid2tblindex)
    {
        osal_free(pDal_ssw_svlan_info[unit]->pValid_lists);
        pDal_ssw_svlan_info[unit]->pValid_lists = 0;
        osal_free(pDal_ssw_svlan_info[unit]);
        pDal_ssw_svlan_info[unit] = 0;
        SVLAN_SEM_UNLOCK(unit);
        RT_ERR(RT_ERR_FAILED, (MOD_SVLAN|MOD_DAL), "memory allocate failed");
        return RT_ERR_FAILED;
    }

    osal_memset(pDal_ssw_svlan_info[unit]->pValid_lists, 0, (num_of_svlan_1bitlist * sizeof(uint32)));
    osal_memset(pDal_ssw_svlan_info[unit]->pSvid2tblindex, 0xff, (RTK_MAX_NUM_OF_SVLAN_ID * sizeof(uint16)));

    SVLAN_SEM_UNLOCK(unit);

    /* set init flag to complete init */
    svlan_init[unit] = INIT_COMPLETED;

    return RT_ERR_OK;
} /* end of dal_ssw_svlan_init */


/* Function Name:
 *      dal_ssw_svlan_create
 * Description:
 *      Create the svlan in the specified device.
 * Input:
 *      unit - unit id
 *      svid - svlan id to be created
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_SVLAN_EXIST - SVLAN entry is exist
 * Note:
 *      None
 */
int32
dal_ssw_svlan_create(uint32 unit, rtk_vlan_t svid)
{
    int32   ret;
    uint32  idx;
    rtk_svlan_data_t svlan_data_entry;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SVLAN), "unit=%d, svid=%d", unit, svid);

    /* check Init status */
    RT_INIT_CHK(svlan_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((svid < RTK_VLAN_ID_MIN) || (svid > RTK_VLAN_ID_MAX), RT_ERR_SVLAN_VID);

    SVLAN_SEM_LOCK(unit);

    /* check available svlan entry */
    if ((idx = pDal_ssw_svlan_info[unit]->pSvid2tblindex[svid]) != SVLAN_ENTRY_NOT_USED)
    {
        SVLAN_SEM_UNLOCK(unit);
        RT_ERR(RT_ERR_SVLAN_EXIST, (MOD_DAL|MOD_SVLAN), "svlan entry already exist");
        return RT_ERR_SVLAN_EXIST;
    }

    if ((ret = _dal_ssw_svlan_addTblValidLists(unit, &idx)) != RT_ERR_OK)
    {
        SVLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    /* create svlan in CHIP*/
    osal_memset(&svlan_data_entry, 0, sizeof(svlan_data_entry));

    /* configure svlan data entry */
    svlan_data_entry.idx      = idx;
    svlan_data_entry.svid     = svid;


    /* write entry to CHIP*/
    if ((ret = _dal_ssw_setSvlan(unit, &svlan_data_entry)) != RT_ERR_OK)
    {
        BITMAP_CLEAR(pDal_ssw_svlan_info[unit]->pValid_lists, idx);
        SVLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    /* update svlan software database */
    pDal_ssw_svlan_info[unit]->pSvid2tblindex[svid] = idx;
    pDal_ssw_svlan_info[unit]->count++;

    SVLAN_SEM_UNLOCK(unit);

    return ret;
} /* end of dal_ssw_svlan_create */


/* Function Name:
 *      dal_ssw_svlan_destroy
 * Description:
 *      Destroy the svlan in the specified device.
 * Input:
 *      unit - unit id
 *      svid - svlan id to be destroyed
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - invalid unit id
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND - specified svlan entry not found
 * Note:
 *      None
 */
int32
dal_ssw_svlan_destroy(uint32 unit, rtk_vlan_t svid)
{
    int32   ret;
    uint32  idx;
    rtk_svlan_data_t svlan_data_entry;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SVLAN), "unit=%d, svid=%d", unit, svid);

    /* check Init status */
    RT_INIT_CHK(svlan_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((svid < RTK_VLAN_ID_MIN) || (svid > RTK_VLAN_ID_MAX), RT_ERR_SVLAN_VID);

    SVLAN_SEM_LOCK(unit);

    if ((idx = pDal_ssw_svlan_info[unit]->pSvid2tblindex[svid]) == SVLAN_ENTRY_NOT_USED)
    {
        SVLAN_SEM_UNLOCK(unit);
        RT_ERR(RT_ERR_SVLAN_ENTRY_NOT_FOUND, (MOD_DAL|MOD_SVLAN), "svlan entry not found");
        return RT_ERR_SVLAN_ENTRY_NOT_FOUND;
    }

    osal_memset(&svlan_data_entry, 0, sizeof(svlan_data_entry));

    /* configure svlan data entry */
    svlan_data_entry.idx      = idx;

    /* write entry to CHIP*/
    if ((ret = _dal_ssw_setSvlan(unit, &svlan_data_entry)) != RT_ERR_OK)
    {
        SVLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    /* update svlan software database */
    BITMAP_CLEAR(pDal_ssw_svlan_info[unit]->pValid_lists, idx);
    pDal_ssw_svlan_info[unit]->pSvid2tblindex[svid] = SVLAN_ENTRY_NOT_USED;
    pDal_ssw_svlan_info[unit]->count--;

    SVLAN_SEM_UNLOCK(unit);

    return ret;
} /* end of dal_ssw_svlan_destroy */


/* Function Name:
 *      dal_ssw_svlan_portSvid_get
 * Description:
 *      Get port default svlan id from the specified device.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pSvid - pointer buffer of port default svlan id
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
dal_ssw_svlan_portSvid_get(uint32 unit, rtk_port_t port, rtk_vlan_t *pSvid)
{
    int32  ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SVLAN), "unit=%d, port=%d", unit, port);

    /* check Init status */
    RT_INIT_CHK(svlan_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(NULL == pSvid, RT_ERR_NULL_POINTER);

    SVLAN_SEM_LOCK(unit);
    /* get value from the CHIP*/
    if ((ret = reg_field_read(unit, svlan_port_regidx[port], spvid_fieldidx[port], pSvid)) != RT_ERR_OK)
    {
        SVLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }
    SVLAN_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SVLAN), "pSvid=%x", *pSvid);

    return ret;
} /* end of dal_ssw_svlan_portSvid_get */


/* Function Name:
 *      dal_ssw_svlan_portSvid_set
 * Description:
 *      Set port default svlan id to the specified device.
 * Input:
 *      unit - unit id
 *      port - port id
 *      svid - port default svlan id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      None
 */
int32
dal_ssw_svlan_portSvid_set(uint32 unit, rtk_port_t port, rtk_vlan_t svid)
{
    int32  ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SVLAN), "unit=%d, port=%d, svid=%d", unit, port, svid);

    /* check Init status */
    RT_INIT_CHK(svlan_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((svid < RTK_VLAN_ID_MIN) || (svid > RTK_VLAN_ID_MAX), RT_ERR_SVLAN_VID);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);

    SVLAN_SEM_LOCK(unit);
    /* write value to the CHIP*/
    if ((ret = reg_field_write(unit, svlan_port_regidx[port], spvid_fieldidx[port], &svid)) != RT_ERR_OK)
    {
        SVLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }
    SVLAN_SEM_UNLOCK(unit);

    return ret;
} /* end of dal_ssw_svlan_portSvid_set */


/* Function Name:
 *      dal_ssw_svlan_servicePort_add
 * Description:
 *      Enable one service port in the specified device.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      None
 */
int32
dal_ssw_svlan_servicePort_add(uint32 unit, rtk_port_t port)
{
    int32  ret;
    rtk_portmask_t stag_portmask;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SVLAN), "unit=%d, port=%d", unit, port);

    /* check Init status */
    RT_INIT_CHK(svlan_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);


    SVLAN_SEM_LOCK(unit);
    /* get value from CHIP */
    if ((ret = reg_field_read(unit, SSW_SVLAN_CONFIGURATION0r, SSW_STAG_PORTf, &stag_portmask.bits[0])) != RT_ERR_OK)
    {
        SVLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }
    /* add port to member_portmask */
    RTK_PORTMASK_PORT_SET(stag_portmask, port);

    /* write value to CHIP*/
    if ((ret = reg_field_write(unit, SSW_SVLAN_CONFIGURATION0r, SSW_STAG_PORTf, &stag_portmask.bits[0])) != RT_ERR_OK)
    {        
        SVLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }
    SVLAN_SEM_UNLOCK(unit);

    return ret;
} /* end of dal_ssw_svlan_servicePort_add */


/* Function Name:
 *      dal_ssw_svlan_servicePort_del
 * Description:
 *      Disable one service port in the specified device.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      None
 */
int32
dal_ssw_svlan_servicePort_del(uint32 unit, rtk_port_t port)
{
    int32  ret;
    rtk_portmask_t stag_portmask;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SVLAN), "unit=%d, port=%d", unit, port);

    /* check Init status */
    RT_INIT_CHK(svlan_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);


    SVLAN_SEM_LOCK(unit);
    /* get value from CHIP */
    if ((ret = reg_field_read(unit, SSW_SVLAN_CONFIGURATION0r, SSW_STAG_PORTf, &stag_portmask.bits[0])) != RT_ERR_OK)
    {
        SVLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }
    /* add port to member_portmask */
    RTK_PORTMASK_PORT_CLEAR(stag_portmask, port);

    /* write value to CHIP*/
    if ((ret = reg_field_write(unit, SSW_SVLAN_CONFIGURATION0r, SSW_STAG_PORTf, &stag_portmask.bits[0])) != RT_ERR_OK)
    {
        SVLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }
    SVLAN_SEM_UNLOCK(unit);

    return ret;
} /* end of dal_ssw_svlan_servicePort_del */


/* Function Name:
 *      dal_ssw_svlan_servicePort_get
 * Description:
 *      Get service ports from the specified device.
 * Input:
 *      unit            - unit id
 * Output:
 *      pSvlan_portmask - pointer buffer of svlan ports
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_ssw_svlan_servicePort_get(uint32 unit, rtk_portmask_t *pSvlan_portmask)
{
    int32  ret;
    rtk_portmask_t stag_portmask;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SVLAN), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(svlan_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(NULL == pSvlan_portmask, RT_ERR_NULL_POINTER);


    SVLAN_SEM_LOCK(unit);
    /* get value from CHIP */
    if ((ret = reg_field_read(unit, SSW_SVLAN_CONFIGURATION0r, SSW_STAG_PORTf, &stag_portmask.bits[0])) != RT_ERR_OK)
    {
        SVLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    /* copy new portmask to vlan data entry */
    RTK_PORTMASK_ASSIGN(*pSvlan_portmask, stag_portmask);

    SVLAN_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SVLAN), "pSvlan_portmask=%x", pSvlan_portmask->bits[0]);

    return ret;
} /* end of dal_ssw_svlan_servicePort_get */


/* Function Name:
 *      dal_ssw_svlan_servicePort_set
 * Description:
 *      Set service ports to the specified device.
 * Input:
 *      unit            - unit id
 *      pSvlan_portmask - svlan ports
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      None
 */
int32
dal_ssw_svlan_servicePort_set(uint32 unit, rtk_portmask_t *pSvlan_portmask)
{
    int32  ret;
    rtk_portmask_t stag_portmask;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SVLAN), "pSvlan_portmask=%x", pSvlan_portmask->bits[0]);

    /* check Init status */
    RT_INIT_CHK(svlan_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(NULL == pSvlan_portmask, RT_ERR_NULL_POINTER);

    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SVLAN), "unit=%d, pSvlan_portmask=%x",
           unit, pSvlan_portmask->bits[0]);

    SVLAN_SEM_LOCK(unit);
    /* get value from CHIP */
    if ((ret = reg_field_read(unit, SSW_SVLAN_CONFIGURATION0r, SSW_STAG_PORTf, &stag_portmask.bits[0])) != RT_ERR_OK)
    {
        SVLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    /* copy new portmask to vlan data entry */
    RTK_PORTMASK_ASSIGN(stag_portmask, *pSvlan_portmask);

    /* write value to CHIP*/
    if ((ret = reg_field_write(unit, SSW_SVLAN_CONFIGURATION0r, SSW_STAG_PORTf, &stag_portmask.bits[0])) != RT_ERR_OK)
    {
        SVLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }
    SVLAN_SEM_UNLOCK(unit);

    return ret;

} /* end of dal_ssw_svlan_servicePort_set */


/* Function Name:
 *      dal_ssw_svlan_memberPort_add
 * Description:
 *      Add one member of the svlan to the specified device.
 * Input:
 *      unit - unit id
 *      svid - svlan id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      1. svlan portmask only for svlan ingress filter checking
 */
int32
dal_ssw_svlan_memberPort_add(uint32 unit, rtk_vlan_t svid, rtk_port_t port)
{
    int32   ret;
    uint32  idx;
    rtk_svlan_data_t svlan_data_entry;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SVLAN), "unit=%d, svid=%d, port=%d", unit, svid, port);

    /* check Init status */
    RT_INIT_CHK(svlan_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((svid < RTK_VLAN_ID_MIN) || (svid > RTK_VLAN_ID_MAX), RT_ERR_SVLAN_VID);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);

    osal_memset(&svlan_data_entry, 0, sizeof(svlan_data_entry));

    SVLAN_SEM_LOCK(unit);

    if ((idx = pDal_ssw_svlan_info[unit]->pSvid2tblindex[svid]) == SVLAN_ENTRY_NOT_USED)
    {
        SVLAN_SEM_UNLOCK(unit);
        return RT_ERR_SVLAN_ENTRY_NOT_FOUND;
    }

    /* configure svlan data entry */
    svlan_data_entry.idx = idx;

    /* get entry from CHIP*/
    if ((ret = _dal_ssw_getSvlan(unit, &svlan_data_entry)) != RT_ERR_OK)
    {
        SVLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }
    /* add port to member_portmask */
    RTK_PORTMASK_PORT_SET(svlan_data_entry.member_portmask, port);

    /* programming to chip */
    if ((ret = _dal_ssw_setSvlan(unit, &svlan_data_entry)) != RT_ERR_OK)
    {
        SVLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }
    SVLAN_SEM_UNLOCK(unit);

    return ret;
} /* end of dal_ssw_svlan_memberPort_add */


/* Function Name:
 *      dal_ssw_svlan_memberPort_del
 * Description:
 *      Delete one member of the svlan from the specified device.
 * Input:
 *      unit - unit id
 *      svid - svlan id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - invalid unit id
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND - specified svlan entry not found
 * Note:
 *      1. svlan portmask only for svlan ingress filter checking
 */
int32
dal_ssw_svlan_memberPort_del(uint32 unit, rtk_vlan_t svid, rtk_port_t port)
{
    int32   ret;
    uint32  idx;
    rtk_svlan_data_t svlan_data_entry;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SVLAN), "unit=%d, svid=%d, port=%d", unit, svid, port);

    /* check Init status */
    RT_INIT_CHK(svlan_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((svid < RTK_VLAN_ID_MIN) || (svid > RTK_VLAN_ID_MAX), RT_ERR_SVLAN_VID);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);

    osal_memset(&svlan_data_entry, 0, sizeof(svlan_data_entry));

    SVLAN_SEM_LOCK(unit);

    if ((idx = pDal_ssw_svlan_info[unit]->pSvid2tblindex[svid]) == SVLAN_ENTRY_NOT_USED)
    {
        SVLAN_SEM_UNLOCK(unit);
        return RT_ERR_SVLAN_ENTRY_NOT_FOUND;
    }

    /* configure svlan data entry */
    svlan_data_entry.idx = idx;

    /* get entry from CHIP*/
    if ((ret = _dal_ssw_getSvlan(unit, &svlan_data_entry)) != RT_ERR_OK)
    {
        SVLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }
    /* set member_portmask and untag_portmask */
    RTK_PORTMASK_PORT_CLEAR(svlan_data_entry.member_portmask, port);

    /* programming to chip */
    if ((ret = _dal_ssw_setSvlan(unit, &svlan_data_entry)) != RT_ERR_OK)
    {
        SVLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }
    SVLAN_SEM_UNLOCK(unit);

    return ret;
} /* end of dal_ssw_svlan_memberPort_del */


/* Function Name:
 *      dal_ssw_svlan_memberPort_get
 * Description:
 *      Get the svlan members from the specified device.
 * Input:
 *      unit            - unit id
 *      svid            - svlan id
 * Output:
 *      pSvlan_portmask - pointer buffer of svlan member ports
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - invalid unit id
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND - specified svlan entry not found
 *      RT_ERR_NULL_POINTER          - input parameter may be null pointer
 * Note:
 *      1. svlan portmask only for svlan ingress filter checking
 */
int32
dal_ssw_svlan_memberPort_get(uint32 unit, rtk_vlan_t svid, rtk_portmask_t *pSvlan_portmask)
{
    int32   ret;
    uint32  idx;
    rtk_svlan_data_t svlan_data_entry;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SVLAN), "unit=%d, svid=%d", unit, svid);

    /* check Init status */
    RT_INIT_CHK(svlan_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((svid < RTK_VLAN_ID_MIN) || (svid > RTK_VLAN_ID_MAX), RT_ERR_SVLAN_VID);
    RT_PARAM_CHK(NULL == pSvlan_portmask, RT_ERR_NULL_POINTER);

    osal_memset(&svlan_data_entry, 0, sizeof(svlan_data_entry));

    SVLAN_SEM_LOCK(unit);
    if ((idx = pDal_ssw_svlan_info[unit]->pSvid2tblindex[svid]) == SVLAN_ENTRY_NOT_USED)
    {
        SVLAN_SEM_UNLOCK(unit);
        return RT_ERR_SVLAN_ENTRY_NOT_FOUND;
    }

    /* configure svlan data entry */
    svlan_data_entry.idx = idx;

    /* get entry from CHIP*/
    if ((ret = _dal_ssw_getSvlan(unit, &svlan_data_entry)) != RT_ERR_OK)
    {
        SVLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    RTK_PORTMASK_ASSIGN(*pSvlan_portmask, svlan_data_entry.member_portmask);

    SVLAN_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SVLAN), "pSvlan_portmask=%x", pSvlan_portmask->bits[0]);

    return ret;
} /* end of dal_ssw_svlan_memberPort_get */


/* Function Name:
 *      dal_ssw_svlan_memberPort_set
 * Description:
 *      Replace the svlan members in the specified device.
 * Input:
 *      unit            - unit id
 *      svid            - svlan id
 *      pSvlan_portmask - svlan member ports
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - invalid unit id
 *      RT_ERR_SVLAN_ENTRY_INDEX     - invalid svid entry no
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND - specified svlan entry not found
 * Note:
 *      1. Don't care the original svlan members and replace with new configure
 *         directly.
 *      2. svlan portmask only for svlan ingress filter checking
 */
int32
dal_ssw_svlan_memberPort_set(uint32 unit, rtk_vlan_t svid, rtk_portmask_t *pSvlan_portmask)
{
    int32   ret;
    uint32  idx;
    rtk_svlan_data_t svlan_data_entry;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SVLAN), "unit=%d, svid=%d", unit, svid);

    /* check Init status */
    RT_INIT_CHK(svlan_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((svid < RTK_VLAN_ID_MIN) || (svid > RTK_VLAN_ID_MAX), RT_ERR_SVLAN_VID);
    RT_PARAM_CHK(NULL == pSvlan_portmask, RT_ERR_NULL_POINTER);

    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SVLAN), "pSvlan_portmask=%x", pSvlan_portmask->bits[0]);

    osal_memset(&svlan_data_entry, 0, sizeof(svlan_data_entry));

    SVLAN_SEM_LOCK(unit);

    if ((idx = pDal_ssw_svlan_info[unit]->pSvid2tblindex[svid]) == SVLAN_ENTRY_NOT_USED)
    {
        SVLAN_SEM_UNLOCK(unit);
        RT_ERR(RT_ERR_SVLAN_ENTRY_NOT_FOUND, (MOD_DAL|MOD_SVLAN), "");
        return RT_ERR_SVLAN_ENTRY_NOT_FOUND;
    }

    /* configure svlan data entry */
    svlan_data_entry.idx = idx;

    /* get entry from CHIP*/
    if ((ret = _dal_ssw_getSvlan(unit, &svlan_data_entry)) != RT_ERR_OK)
    {
        SVLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    RTK_PORTMASK_ASSIGN(svlan_data_entry.member_portmask, *pSvlan_portmask);

    /* write entry to CHIP*/
    if ((ret = _dal_ssw_setSvlan(unit, &svlan_data_entry)) != RT_ERR_OK)
    {
        SVLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }
    SVLAN_SEM_UNLOCK(unit);

    return ret;
} /* end of dal_ssw_svlan_memberPort_set */


/* Function Name:
 *      dal_ssw_svlan_memberPortEntry_get
 * Description:
 *      Get the svlan id and members by svlan table index from the specified device.
 * Input:
 *      unit            - unit id
 *      svid_idx        - svlan table index(0~63)
 * Output:
 *      pSvid           - pointer buffer of svlan id
 *      pSvlan_portmask - pointer buffer of svlan member ports
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_SVLAN_ENTRY_INDEX - invalid svid entry no
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 * Note:
 *      1. svlan portmask only for svlan ingress filter checking
 */
int32
dal_ssw_svlan_memberPortEntry_get(
    uint32         unit,
    uint32         svid_idx,
    rtk_vlan_t     *pSvid,
    rtk_portmask_t *pSvlan_portmask)
{
    int32  ret;
    uint32 svlan_tableSize;
    rtk_svlan_data_t svlan_data_entry;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SVLAN), "unit=%d, svid_idx=%d",
           unit, svid_idx);

    /* check Init status */
    RT_INIT_CHK(svlan_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(NULL == pSvid, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(NULL == pSvlan_portmask, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(BITMAP_IS_CLEAR(pDal_ssw_svlan_info[unit]->pValid_lists, svid_idx), RT_ERR_SVLAN_ENTRY_NOT_FOUND);
    if ((ret = table_size_get(unit, SSW_SVLAN_TABLEt, &svlan_tableSize)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }
    RT_PARAM_CHK(svid_idx >= svlan_tableSize, RT_ERR_SVLAN_ENTRY_INDEX);

    osal_memset(&svlan_data_entry, 0, sizeof(svlan_data_entry));

    /* configure svlan data entry */
    svlan_data_entry.idx = svid_idx;

    SVLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = _dal_ssw_getSvlan(unit, &svlan_data_entry)) != RT_ERR_OK)
    {
        SVLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }
    *pSvid = svlan_data_entry.svid;
    RTK_PORTMASK_ASSIGN(*pSvlan_portmask, svlan_data_entry.member_portmask);

    SVLAN_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SVLAN), "pSvid=%d, pSvlan_portmask=%x",
           *pSvid, pSvlan_portmask->bits[0]);

    return ret;
} /* end of dal_ssw_svlan_memberPortEntry_get */


/* Function Name:
 *      dal_ssw_svlan_memberPortEntry_set
 * Description:
 *      Set the svlan id and members by svlan table index to the specified device.
 * Input:
 *      unit            - unit id
 *      svid_idx        - svlan table index
 *      svid            - svlan id
 *      pSvlan_portmask - svlan member ports
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_SVLAN_ENTRY_INDEX - invalid svid entry no
 * Note:
 *      1. svlan portmask only for svlan ingress filter checking
 */
int32
dal_ssw_svlan_memberPortEntry_set(
    uint32         unit,
    uint32         svid_idx,
    rtk_vlan_t     svid,
    rtk_portmask_t *pSvlan_portmask)
{
    int32   ret;
    uint32  old_svid = 0;
    uint32  svlan_tableSize;
    rtk_svlan_data_t svlan_data_entry;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SVLAN), "unit=%d, svid_idx=%d, svid=%d", unit, svid_idx, svid);

    /* check Init status */
    RT_INIT_CHK(svlan_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((svid < RTK_VLAN_ID_MIN) || (svid > RTK_VLAN_ID_MAX), RT_ERR_SVLAN_VID);
    RT_PARAM_CHK(NULL == pSvlan_portmask, RT_ERR_NULL_POINTER);
    if ((ret = table_size_get(unit, SSW_SVLAN_TABLEt, &svlan_tableSize)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }
    RT_PARAM_CHK(svid_idx >= svlan_tableSize, RT_ERR_SVLAN_ENTRY_INDEX);

    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SVLAN), "pSvlan_portmask=%x", pSvlan_portmask->bits[0]);

    osal_memset(&svlan_data_entry, 0, sizeof(svlan_data_entry));

    /* configure svlan data entry */
    svlan_data_entry.idx = svid_idx;

    SVLAN_SEM_LOCK(unit);

    /* get entry from CHIP*/
    if ((ret = _dal_ssw_getSvlan(unit, &svlan_data_entry)) != RT_ERR_OK)
    {
        SVLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    /* store old svid information */
    old_svid = svlan_data_entry.svid;

    svlan_data_entry.svid = svid;
    RTK_PORTMASK_ASSIGN(svlan_data_entry.member_portmask, *pSvlan_portmask);

    /* write entry to CHIP*/
    if ((ret = _dal_ssw_setSvlan(unit, &svlan_data_entry)) != RT_ERR_OK)
    {
        SVLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    /* update svlan software database */
    if (old_svid != svid)
    {
        pDal_ssw_svlan_info[unit]->pSvid2tblindex[old_svid] = SVLAN_ENTRY_NOT_USED;
    }
    pDal_ssw_svlan_info[unit]->pSvid2tblindex[svid] = svid_idx;

    if (BITMAP_IS_CLEAR(pDal_ssw_svlan_info[unit]->pValid_lists, svid_idx))
    {
        BITMAP_SET(pDal_ssw_svlan_info[unit]->pValid_lists, svid_idx);
        pDal_ssw_svlan_info[unit]->count++;
    }

    SVLAN_SEM_UNLOCK(unit);

    return ret;
} /* end of dal_ssw_svlan_memberPortEntry_set */


/* Function Name:
 *      dal_ssw_svlan_nextValidMemberPortEntry_get
 * Description:
 *      Get next valid svlan table entry from the specified device.
 * Input:
 *      unit            - unit id
 *      pSvid_idx       - input svlan table index for get next
 * Output:
 *      pSvid           - pointer buffer of svlan id
 *      pSvlan_portmask - pointer buffer of svlan member ports
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_SVLAN_ENTRY_INDEX - invalid svid entry no
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 * Note:
 *      1. svlan portmask only for svlan ingress filter checking
 *      2. Please input -1 for get the first entry of svlan table.
 *      3. The *pSvid_idx is the input and output key BOTH.
 */
int32
dal_ssw_svlan_nextValidMemberPortEntry_get(
    uint32         unit,
    int32          *pSvid_idx,
    rtk_vlan_t     *pSvid,
    rtk_portmask_t *pSvlan_portmask)
{
    int32  ret;
    uint32 idx;
    uint32 svlan_tableSize;
    rtk_svlan_data_t svlan_data_entry;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SVLAN), "unit=%d, pSvid_idx=%x",
           pSvid, pSvlan_portmask);

    /* check Init status */
    RT_INIT_CHK(svlan_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(NULL == pSvid_idx, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(NULL == pSvid, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(NULL == pSvlan_portmask, RT_ERR_NULL_POINTER);
    if ((ret = table_size_get(unit, SSW_SVLAN_TABLEt, &svlan_tableSize)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }
    RT_PARAM_CHK(*pSvid_idx >= (int32)svlan_tableSize, RT_ERR_SVLAN_ENTRY_INDEX);
    osal_memset(&svlan_data_entry, 0, sizeof(svlan_data_entry));


    SVLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    for (idx = *pSvid_idx+1; idx < svlan_tableSize; idx++)
    {
        /* configure svlan data entry */
        svlan_data_entry.idx = idx;

        if ((ret = _dal_ssw_getSvlan(unit, &svlan_data_entry)) == RT_ERR_OK)
        {
            if (svlan_data_entry.svid != 0)
            {
                *pSvid_idx = svlan_data_entry.idx;
                *pSvid = svlan_data_entry.svid;
                RTK_PORTMASK_ASSIGN(*pSvlan_portmask, svlan_data_entry.member_portmask);
                
                SVLAN_SEM_UNLOCK(unit);
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SVLAN), "pSvid=%x, \
                pSvlan_portmask=%x", *pSvid, pSvlan_portmask->bits[0]);
                return RT_ERR_OK;
            }
        }
        else
        {
            SVLAN_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
            return ret;
        }
    }

    SVLAN_SEM_UNLOCK(unit);

    /* if achieve here means no entry was found */
    *pSvid = -1;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SVLAN), "vlan entry not found");

    return RT_ERR_SVLAN_ENTRY_NOT_FOUND;
} /* end of dal_ssw_svlan_nextValidMemberPortEntry_get */


/* Function Name:
 *      dal_ssw_svlan_tpidEntry_get
 * Description:
 *      Get the svlan TPID from the specified device.
 * Input:
 *      unit          - unit id
 *      svlan_index   - index of svlan table
 * Output:
 *      pSvlan_tag_id - pointer buffer of svlan TPID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Only svlan_index 0 is valid and default svlan TPID is 0x88A8 in RTL8329/RTL8389.
 */
int32
dal_ssw_svlan_tpidEntry_get(uint32 unit, uint32 svlan_index, uint32 *pSvlan_tag_id)
{
    int32  ret;
    uint32 reg_idx;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SVLAN), "unit=%d, svlan_index=%d",
           unit, svlan_index);

    /* check Init status */
    RT_INIT_CHK(svlan_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(NULL == pSvlan_tag_id, RT_ERR_NULL_POINTER);

    reg_idx = SSW_SVLAN_CONFIGURATION1r;

    SVLAN_SEM_LOCK(unit);
    /* read value from the CHIP*/
    if ((ret = reg_field_read(unit, reg_idx, SSW_STAG_TPIf, pSvlan_tag_id)) != RT_ERR_OK)
    {
        SVLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }
    SVLAN_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SVLAN), "pSvlan_tag_id=%x", *pSvlan_tag_id);

    return ret;
} /* end of dal_ssw_svlan_tpidEntry_get */


/* Function Name:
 *      dal_ssw_svlan_tpidEntry_set
 * Description:
 *      Set the svlan TPID to the specified device.
 * Input:
 *      unit         - unit id
 *      svlan_index  - index of svlan table
 *      svlan_tag_id - svlan TPID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      Only svlan_index 0 is valid and default svlan TPID is 0x88A8 in RTL8329/RTL8389.
 */
int32
dal_ssw_svlan_tpidEntry_set(uint32 unit, uint32 svlan_index, uint32 svlan_tag_id)
{
    int32  ret;
    uint32 reg_idx;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SVLAN), "unit=%d, svlan_index=%d, svlan_tag_id=%x",
           unit, svlan_index, svlan_tag_id);

    /* check Init status */
    RT_INIT_CHK(svlan_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(svlan_tag_id > RTK_ETHERTYPE_MAX, RT_ERR_SVLAN_ETHER_TYPE);

    reg_idx = SSW_SVLAN_CONFIGURATION1r;

    SVLAN_SEM_LOCK(unit);
    /* write value to the CHIP*/
    if ((ret = reg_field_write(unit, reg_idx, SSW_STAG_TPIf, &svlan_tag_id)) != RT_ERR_OK)
    {
        SVLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }
    SVLAN_SEM_UNLOCK(unit);

    return ret;
} /* end of dal_ssw_svlan_tpidEntry_set */

/* Internal Function Body */

/* Function Name:
 *      _dal_ssw_svlan_addTblValidLists
 * Description:
 *      Get a free entry index from the SVLAN table
 * Input:
 *      unit - unit id
 * Output:
 *      pIdx - index to the available entry in SVLAN table
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_SVLAN_TABLE_FULL
 * Note:
 */
static int32
_dal_ssw_svlan_addTblValidLists(uint32 unit, uint32 *pIdx)
{
    int32   ret;
    uint32  svlan_idx;
    uint32  svlan_tableSize;

    if ((ret = table_size_get(unit, SSW_SVLAN_TABLEt, &svlan_tableSize)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_SVLAN), "unit=%d", unit);

    for (svlan_idx = 0; svlan_idx < svlan_tableSize; svlan_idx++)
    {
        if (BITMAP_IS_CLEAR(pDal_ssw_svlan_info[unit]->pValid_lists, svlan_idx))
        {
            *pIdx = svlan_idx;
            BITMAP_SET(pDal_ssw_svlan_info[unit]->pValid_lists, svlan_idx);
            RT_DBG(LOG_TRACE, (MOD_DAL|MOD_SVLAN), "idx=%d", *pIdx);
            return RT_ERR_OK;
        }
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SVLAN), "svlan table full");

    return RT_ERR_SVLAN_TABLE_FULL;
} /* end of _dal_ssw_svlan_addTblValidLists */

/* Function Name:
 *      _dal_ssw_setSvlan
 * Description:
 *      Set svlan entry to chip
 * Input:
 *      unit         - unit id
 *      pSvlan_entry - content of svlan entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 */
static int32
_dal_ssw_setSvlan(uint32 unit, rtk_svlan_data_t *pSvlan_entry)
{
    int32   ret;
    svlan_entry_t    svlan_entry;

    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_SVLAN), "unit=%d, idx=%d, svid=%d, member_portmask=%x)",
           unit, pSvlan_entry->idx, pSvlan_entry->svid, pSvlan_entry->member_portmask.bits[0]);

    osal_memset(&svlan_entry, 0, sizeof(svlan_entry));

    /* set svlan id to chip */
    if ((ret = reg_field_write(unit, svlan_id_regidx[pSvlan_entry->idx], svid_fieldidx[pSvlan_entry->idx], &(pSvlan_entry->svid))) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    /* programming svlan member port entry to chip */
    if ((ret = table_field_set(unit, SSW_SVLAN_TABLEt, SSW_SVLAN_TABLE_SMBRf, pSvlan_entry->member_portmask.bits, (uint32 *) &svlan_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = table_write(unit, SSW_SVLAN_TABLEt, pSvlan_entry->idx, (uint32 *) &svlan_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    return ret;
} /* end of _dal_ssw_setSvlan */

/* Function Name:
 *      _dal_ssw_getSvlan
 * Description:
 *      Get svlan entry to chip
 * Input:
 *      unit         - unit id
 *      pSvlan_entry - content of svlan entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 */
static int32
_dal_ssw_getSvlan(uint32 unit, rtk_svlan_data_t *pSvlan_entry)
{
    int32   ret;
    svlan_entry_t    svlan_entry;

    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_SVLAN), "unit=%d, idx=%d)", unit, pSvlan_entry->idx);

    osal_memset(&svlan_entry, 0, sizeof(svlan_entry));

    /* get svlan id from chip */
    if ((ret = reg_field_read(unit, svlan_id_regidx[pSvlan_entry->idx], svid_fieldidx[pSvlan_entry->idx], &pSvlan_entry->svid)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    /* get svlan member port entry from chip */
    if ((ret = table_read(unit, SSW_SVLAN_TABLEt, pSvlan_entry->idx, (uint32 *) &svlan_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = table_field_get(unit, SSW_SVLAN_TABLEt, SSW_SVLAN_TABLE_SMBRf, pSvlan_entry->member_portmask.bits, (uint32 *) &svlan_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SVLAN), "svid=%d, member_portmask=%x)",
           pSvlan_entry->svid, pSvlan_entry->member_portmask.bits[0]);

    return ret;
} /* end of _dal_ssw_getSvlan */
