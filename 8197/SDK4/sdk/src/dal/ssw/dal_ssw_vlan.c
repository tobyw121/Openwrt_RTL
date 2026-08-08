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
 * Purpose : Definition those public vlan APIs and its data type in the SDK.
 * 
 * Feature : The file have include the following module and sub-modules
 *           1) vlan
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
#include <dal/ssw/dal_ssw_vlan.h>
#include <rtk/default.h>
#include <rtk/vlan.h>

/* 
 * Symbol Definition 
 */
/* vlan information structure */
typedef struct dal_ssw_vlan_info_s
{
    uint32 count;           /* count of valid vlan number    */
    uint32 *pValid_lists;   /* valid bit for this table      */
    uint16 *pVid2tblindex;  /* table index of vid, 0:invalid */
} dal_ssw_vlan_info_t;

/* vlan entry*/
typedef struct dal_ssw_vlan_data_s
{
    rtk_vlan_t  vid;
    rtk_fid_t   fid;
    rtk_stg_t   msti;
    rtk_portmask_t  member_portmask;
    rtk_portmask_t  untag_portmask;
} dal_ssw_vlan_data_t;

/* 
 * Data Declaration 
 */
static uint32               vlan_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         vlan_sem[RTK_MAX_NUM_OF_UNIT];
static dal_ssw_vlan_info_t  *pDal_ssw_vlan_info[RTK_MAX_NUM_OF_UNIT];

const static uint16 vlan_accept_tagged_or_untagged_control_regidx[] = 
{  SSW_VLAN_ACCEPT_TAGGED_OR_UNTAGGED_CONTROL0r, SSW_VLAN_ACCEPT_TAGGED_OR_UNTAGGED_CONTROL0r\
 , SSW_VLAN_ACCEPT_TAGGED_OR_UNTAGGED_CONTROL0r, SSW_VLAN_ACCEPT_TAGGED_OR_UNTAGGED_CONTROL0r\
 , SSW_VLAN_ACCEPT_TAGGED_OR_UNTAGGED_CONTROL0r, SSW_VLAN_ACCEPT_TAGGED_OR_UNTAGGED_CONTROL0r\
 , SSW_VLAN_ACCEPT_TAGGED_OR_UNTAGGED_CONTROL0r, SSW_VLAN_ACCEPT_TAGGED_OR_UNTAGGED_CONTROL0r\
 , SSW_VLAN_ACCEPT_TAGGED_OR_UNTAGGED_CONTROL0r, SSW_VLAN_ACCEPT_TAGGED_OR_UNTAGGED_CONTROL0r\
 , SSW_VLAN_ACCEPT_TAGGED_OR_UNTAGGED_CONTROL0r, SSW_VLAN_ACCEPT_TAGGED_OR_UNTAGGED_CONTROL0r\
 , SSW_VLAN_ACCEPT_TAGGED_OR_UNTAGGED_CONTROL0r, SSW_VLAN_ACCEPT_TAGGED_OR_UNTAGGED_CONTROL0r\
 , SSW_VLAN_ACCEPT_TAGGED_OR_UNTAGGED_CONTROL0r, SSW_VLAN_ACCEPT_TAGGED_OR_UNTAGGED_CONTROL0r\
 , SSW_VLAN_ACCEPT_TAGGED_OR_UNTAGGED_CONTROL1r, SSW_VLAN_ACCEPT_TAGGED_OR_UNTAGGED_CONTROL1r\
 , SSW_VLAN_ACCEPT_TAGGED_OR_UNTAGGED_CONTROL1r, SSW_VLAN_ACCEPT_TAGGED_OR_UNTAGGED_CONTROL1r\
 , SSW_VLAN_ACCEPT_TAGGED_OR_UNTAGGED_CONTROL1r, SSW_VLAN_ACCEPT_TAGGED_OR_UNTAGGED_CONTROL1r\
 , SSW_VLAN_ACCEPT_TAGGED_OR_UNTAGGED_CONTROL1r, SSW_VLAN_ACCEPT_TAGGED_OR_UNTAGGED_CONTROL1r\
 , SSW_VLAN_ACCEPT_TAGGED_OR_UNTAGGED_CONTROL1r, SSW_VLAN_ACCEPT_TAGGED_OR_UNTAGGED_CONTROL1r\
 , SSW_VLAN_ACCEPT_TAGGED_OR_UNTAGGED_CONTROL1r, SSW_VLAN_ACCEPT_TAGGED_OR_UNTAGGED_CONTROL1r\
 , SSW_VLAN_ACCEPT_TAGGED_OR_UNTAGGED_CONTROL1r};

const static uint16 tag_based_and_port_based_control_regidx[] = 
{  SSW_TAG_BASED_AND_PORT_BASED_CONTROL0r, SSW_TAG_BASED_AND_PORT_BASED_CONTROL0r\
 , SSW_TAG_BASED_AND_PORT_BASED_CONTROL1r, SSW_TAG_BASED_AND_PORT_BASED_CONTROL1r\
 , SSW_TAG_BASED_AND_PORT_BASED_CONTROL2r, SSW_TAG_BASED_AND_PORT_BASED_CONTROL2r\
 , SSW_TAG_BASED_AND_PORT_BASED_CONTROL3r, SSW_TAG_BASED_AND_PORT_BASED_CONTROL3r\
 , SSW_TAG_BASED_AND_PORT_BASED_CONTROL4r, SSW_TAG_BASED_AND_PORT_BASED_CONTROL4r\
 , SSW_TAG_BASED_AND_PORT_BASED_CONTROL5r, SSW_TAG_BASED_AND_PORT_BASED_CONTROL5r\
 , SSW_TAG_BASED_AND_PORT_BASED_CONTROL6r, SSW_TAG_BASED_AND_PORT_BASED_CONTROL6r\
 , SSW_TAG_BASED_AND_PORT_BASED_CONTROL7r, SSW_TAG_BASED_AND_PORT_BASED_CONTROL7r\
 , SSW_TAG_BASED_AND_PORT_BASED_CONTROL8r, SSW_TAG_BASED_AND_PORT_BASED_CONTROL8r\
 , SSW_TAG_BASED_AND_PORT_BASED_CONTROL9r, SSW_TAG_BASED_AND_PORT_BASED_CONTROL9r\
 , SSW_TAG_BASED_AND_PORT_BASED_CONTROL10r, SSW_TAG_BASED_AND_PORT_BASED_CONTROL10r\
 , SSW_TAG_BASED_AND_PORT_BASED_CONTROL11r, SSW_TAG_BASED_AND_PORT_BASED_CONTROL11r\
 , SSW_TAG_BASED_AND_PORT_BASED_CONTROL12r, SSW_TAG_BASED_AND_PORT_BASED_CONTROL12r\
 , SSW_TAG_BASED_AND_PORT_BASED_CONTROL13r, SSW_TAG_BASED_AND_PORT_BASED_CONTROL13r\
 , SSW_TAG_BASED_AND_PORT_BASED_CONTROL14r};

const static uint16 vlan_mode_control_regidx[] = 
{  SSW_VLAN_MODE_CONTROL0r, SSW_VLAN_MODE_CONTROL0r\
 , SSW_VLAN_MODE_CONTROL0r, SSW_VLAN_MODE_CONTROL0r\
 , SSW_VLAN_MODE_CONTROL0r, SSW_VLAN_MODE_CONTROL0r\
 , SSW_VLAN_MODE_CONTROL0r, SSW_VLAN_MODE_CONTROL0r\
 , SSW_VLAN_MODE_CONTROL0r, SSW_VLAN_MODE_CONTROL0r\
 , SSW_VLAN_MODE_CONTROL0r, SSW_VLAN_MODE_CONTROL0r\
 , SSW_VLAN_MODE_CONTROL0r, SSW_VLAN_MODE_CONTROL0r\
 , SSW_VLAN_MODE_CONTROL0r, SSW_VLAN_MODE_CONTROL0r\
 , SSW_VLAN_MODE_CONTROL1r, SSW_VLAN_MODE_CONTROL1r\
 , SSW_VLAN_MODE_CONTROL1r, SSW_VLAN_MODE_CONTROL1r\
 , SSW_VLAN_MODE_CONTROL1r, SSW_VLAN_MODE_CONTROL1r\
 , SSW_VLAN_MODE_CONTROL1r, SSW_VLAN_MODE_CONTROL1r\
 , SSW_VLAN_MODE_CONTROL1r, SSW_VLAN_MODE_CONTROL1r\
 , SSW_VLAN_MODE_CONTROL1r, SSW_VLAN_MODE_CONTROL1r\
 , SSW_VLAN_MODE_CONTROL1r};
 
static uint16 pvid_fieldidx[] = {SSW_P0PVIDf, SSW_P1PVIDf, SSW_P2PVIDf, SSW_P3PVIDf, SSW_P4PVIDf, SSW_P5PVIDf, SSW_P6PVIDf, SSW_P7PVIDf, SSW_P8PVIDf, SSW_P9PVIDf\
                         ,SSW_P10PVIDf, SSW_P11PVIDf, SSW_P12PVIDf, SSW_P13PVIDf, SSW_P14PVIDf, SSW_P15PVIDf, SSW_P16PVIDf, SSW_P17PVIDf, SSW_P18PVIDf, SSW_P19PVIDf\
                         ,SSW_P20PVIDf, SSW_P21PVIDf, SSW_P22PVIDf, SSW_P23PVIDf, SSW_P24PVIDf, SSW_P25PVIDf, SSW_P26PVIDf, SSW_P27PVIDf, SSW_P28PVIDf};
static uint16 tag_accept_field[] = {SSW_P0_TAG_ACCEPTf, SSW_P1_TAG_ACCEPTf, SSW_P2_TAG_ACCEPTf, SSW_P3_TAG_ACCEPTf, SSW_P4_TAG_ACCEPTf, SSW_P5_TAG_ACCEPTf, SSW_P6_TAG_ACCEPTf, SSW_P7_TAG_ACCEPTf, SSW_P8_TAG_ACCEPTf, SSW_P9_TAG_ACCEPTf\
                         ,SSW_P10_TAG_ACCEPTf, SSW_P11_TAG_ACCEPTf, SSW_P12_TAG_ACCEPTf, SSW_P13_TAG_ACCEPTf, SSW_P14_TAG_ACCEPTf, SSW_P15_TAG_ACCEPTf, SSW_P16_TAG_ACCEPTf, SSW_P17_TAG_ACCEPTf, SSW_P18_TAG_ACCEPTf, SSW_P19_TAG_ACCEPTf\
                         ,SSW_P20_TAG_ACCEPTf, SSW_P21_TAG_ACCEPTf, SSW_P22_TAG_ACCEPTf, SSW_P23_TAG_ACCEPTf, SSW_P24_TAG_ACCEPTf, SSW_P25_TAG_ACCEPTf, SSW_P26_TAG_ACCEPTf, SSW_P27_TAG_ACCEPTf, SSW_P28_TAG_ACCEPTf};
static uint16 vlanmode_fieldidx[] = {SSW_P0_VLAN_MODEf, SSW_P1_VLAN_MODEf, SSW_P2_VLAN_MODEf, SSW_P3_VLAN_MODEf, SSW_P4_VLAN_MODEf, SSW_P5_VLAN_MODEf, SSW_P6_VLAN_MODEf, SSW_P7_VLAN_MODEf, SSW_P8_VLAN_MODEf, SSW_P9_VLAN_MODEf\
                         ,SSW_P10_VLAN_MODEf, SSW_P11_VLAN_MODEf, SSW_P12_VLAN_MODEf, SSW_P13_VLAN_MODEf, SSW_P14_VLAN_MODEf, SSW_P15_VLAN_MODEf, SSW_P16_VLAN_MODEf, SSW_P17_VLAN_MODEf, SSW_P18_VLAN_MODEf, SSW_P19_VLAN_MODEf\
                         ,SSW_P20_VLAN_MODEf, SSW_P21_VLAN_MODEf, SSW_P22_VLAN_MODEf, SSW_P23_VLAN_MODEf, SSW_P24_VLAN_MODEf, SSW_P25_VLAN_MODEf, SSW_P26_VLAN_MODEf, SSW_P27_VLAN_MODEf, SSW_P28_VLAN_MODEf};

/*
 * Macro Definition
 */
/* vlan semaphore handling */
#define VLAN_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(vlan_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_VLAN|MOD_DAL), "semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)

#define VLAN_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(vlan_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_VLAN|MOD_DAL), "semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)


#define VLANINFO_VALID_IS_SET(unit, vid)    BITMAP_IS_SET(pDal_ssw_vlan_info[unit]->pValid_lists, vid)
#define VLANINFO_VALID_IS_CLEAR(unit, vid)  BITMAP_IS_CLEAR(pDal_ssw_vlan_info[unit]->pValid_lists, vid)
#define VLANINFO_VALID_SET(unit, vid)       BITMAP_SET(pDal_ssw_vlan_info[unit]->pValid_lists, vid)
#define VLANINFO_VALID_CLEAR(unit, vid)     BITMAP_CLEAR(pDal_ssw_vlan_info[unit]->pValid_lists, vid)

/* 
 * Function Declaration 
 */
static int32 _dal_ssw_setVlan(uint32 unit, dal_ssw_vlan_data_t *pVlan_entry);
static int32 _dal_ssw_getVlan(uint32 unit, dal_ssw_vlan_data_t *pVlan_entry);
static int32 _dal_ssw_vlan_init_config(uint32 unit); 
/* Module Name : vlan */


/* Function Name:
 *      dal_ssw_vlan_init
 * Description:
 *      Initialize vlan module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Must initialize vlan module before calling any vlan APIs.
 */
int32
dal_ssw_vlan_init(uint32 unit) 
{
    int32       ret;
    uint32      vlan_tableSize, num_of_vlan_1bitlist;
    
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d", unit); 
    
    vlan_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    vlan_sem[unit] = osal_sem_mutex_create();
    if (0 == vlan_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_VLAN|MOD_DAL), "semaphore create failed");
        return RT_ERR_FAILED;
    }
    
    vlan_tableSize = 0;
    if ((ret = table_size_get(unit, SSW_VLAN_TABLEt, &vlan_tableSize)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "table size get failed");
        return ret;
    }
    
    num_of_vlan_1bitlist = (vlan_tableSize + 31) >> 5;
    
    VLAN_SEM_LOCK(unit);
    
    /* allocate memory for each database and initilize database */
    pDal_ssw_vlan_info[unit] = (dal_ssw_vlan_info_t *)osal_alloc(sizeof(dal_ssw_vlan_info_t));
    if (0 == pDal_ssw_vlan_info[unit])
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(RT_ERR_FAILED, (MOD_VLAN|MOD_DAL), "memory allocate failed");
        return RT_ERR_FAILED;
    }
    osal_memset(pDal_ssw_vlan_info[unit], 0, sizeof(dal_ssw_vlan_info_t));
    
    pDal_ssw_vlan_info[unit]->pValid_lists = (uint32 *)osal_alloc(num_of_vlan_1bitlist * sizeof(uint32));
    if (0 == pDal_ssw_vlan_info[unit]->pValid_lists)
    {
        osal_free(pDal_ssw_vlan_info[unit]);
        pDal_ssw_vlan_info[unit] = 0;
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(RT_ERR_FAILED, (MOD_VLAN|MOD_DAL), "memory allocate failed");
        return RT_ERR_FAILED;
    }
    osal_memset(pDal_ssw_vlan_info[unit]->pValid_lists, 0, (num_of_vlan_1bitlist * sizeof(uint32)));
    
    pDal_ssw_vlan_info[unit]->pVid2tblindex = (uint16 *)osal_alloc(vlan_tableSize * sizeof(uint16));
    if (0 == pDal_ssw_vlan_info[unit]->pVid2tblindex)
    {
        osal_free(pDal_ssw_vlan_info[unit]->pValid_lists);
        pDal_ssw_vlan_info[unit]->pValid_lists = 0;
        osal_free(pDal_ssw_vlan_info[unit]);
        pDal_ssw_vlan_info[unit] = 0;
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(RT_ERR_FAILED, (MOD_VLAN|MOD_DAL), "memory allocate failed");
        return RT_ERR_FAILED;
    }
    osal_memset(pDal_ssw_vlan_info[unit]->pVid2tblindex, 0, (vlan_tableSize * sizeof(uint16)));
    VLAN_SEM_UNLOCK(unit);
    
    /* set init flag to complete init */
    vlan_init[unit] = INIT_COMPLETED; 
    
    if (( ret = _dal_ssw_vlan_init_config(unit)) != RT_ERR_OK)
    {
        vlan_init[unit] = INIT_NOT_COMPLETED;
        osal_free(pDal_ssw_vlan_info[unit]->pVid2tblindex);
        pDal_ssw_vlan_info[unit]->pVid2tblindex = 0;
        osal_free(pDal_ssw_vlan_info[unit]->pValid_lists);
        pDal_ssw_vlan_info[unit]->pValid_lists = 0;
        osal_free(pDal_ssw_vlan_info[unit]);
        pDal_ssw_vlan_info[unit] = 0;
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "Init default vlan config failed");
        return ret;
    }
    
    return RT_ERR_OK;
} /* end of dal_ssw_vlan_init */


/* Function Name:
 *      dal_ssw_vlan_create
 * Description:
 *      Create the vlan in the specified device.
 * Input:
 *      unit - unit id
 *      vid  - vlan id to be created
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_UNIT_ID    - invalid unit id
 *      RT_ERR_VLAN_VID   - invalid vid
 *      RT_ERR_VLAN_EXIST - vlan is exist
 * Note:
 *      1. Default FID and STG be assigned after vlan create.
 *         Default FID is equal with VID (IVL mode) and default STG is CIST.
 *      2. FID and STG can be reassigned later by following APIs.
 *         dal_ssw_vlan_fid_set
 *         dal_ssw_vlan_stg_set
 */
int32
dal_ssw_vlan_create(uint32 unit, rtk_vlan_t vid)
{
    int32   ret;
    dal_ssw_vlan_data_t vlan_data_entry;

    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, vid=%d", unit, vid);

    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* VID check */
    RT_PARAM_CHK((vid < RTK_VLAN_ID_MIN) || (vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    RT_PARAM_CHK(VLANINFO_VALID_IS_SET(unit, vid), RT_ERR_VLAN_EXIST);
    
    /* create vlan in CHIP*/
    osal_memset(&vlan_data_entry, 0, sizeof(vlan_data_entry));
    
    /* configure vlan data entry */
    vlan_data_entry.vid     = vid;
    vlan_data_entry.fid     = vid;
    vlan_data_entry.msti    = RTK_DEFAULT_MSTI;
    
    VLAN_SEM_LOCK(unit);
    
    /* write entry to CHIP*/
    if ((ret = _dal_ssw_setVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    /* Set vlan valid bit in software */
    VLANINFO_VALID_SET(unit, vid);
    pDal_ssw_vlan_info[unit]->count++;
    
    VLAN_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_vlan_create */


/* Function Name:
 *      dal_ssw_vlan_destroy
 * Description:
 *      Destroy the vlan in the specified device.
 * Input:
 *      unit - unit id
 *      vid  - vlan id to be destroyed
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 * Note:
 *      None
 */
int32
dal_ssw_vlan_destroy(uint32 unit, rtk_vlan_t vid)
{
    int32 ret;
    dal_ssw_vlan_data_t vlan_data_entry;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, vid=%d", unit, vid);
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
    
    /* VID check */
    RT_PARAM_CHK((vid < RTK_VLAN_ID_MIN) || (vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    RT_PARAM_CHK(VLANINFO_VALID_IS_CLEAR(unit, vid), RT_ERR_VLAN_ENTRY_NOT_FOUND);
    
    /* create vlan in CHIP*/
    osal_memset(&vlan_data_entry, 0, sizeof(vlan_data_entry));
    
    /* configure vlan data entry */
    vlan_data_entry.vid     = vid;
    
    VLAN_SEM_LOCK(unit);
    /* write entry to CHIP*/
    if ((ret = _dal_ssw_setVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    /* Unset vlan valid bit in software */
    VLANINFO_VALID_CLEAR(unit, vid);
    pDal_ssw_vlan_info[unit]->count--;
    
    VLAN_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_vlan_destroy */


/* Function Name:
 *      dal_ssw_vlan_destroyAll
 * Description:
 *      Destroy all vlans except default vlan in the specified device. 
 *      If restore_default_vlan is enable, default vlan will be restored.
 * Input:
 *      unit                 - unit id
 *      restore_default_vlan - keep and restore default vlan id or not?
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 * Note:
 *      The restore argument is permit following value:
 *      - 0: remove default vlan
 *      - 1: restore default vlan
 */
int32
dal_ssw_vlan_destroyAll(uint32 unit, uint32 keep_and_restore_default_vlan)
{
    int32       ret;
    dal_ssw_vlan_data_t vlan_data_entry;
    rtk_vlan_t  vid;
    uint32      vlan_tableSize;
    
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, keep_and_restore_default_vlan=%d",
           unit, keep_and_restore_default_vlan);
    
    RT_INIT_CHK(vlan_init[unit]);
    
    if ((ret = table_size_get(unit, SSW_VLAN_TABLEt, &vlan_tableSize)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "table size get failed");
        return ret;
    }
    
    for (vid = 0; vid < vlan_tableSize; vid++)
    {
        if (VLANINFO_VALID_IS_SET(unit, vid))
        {
            if ((ret = dal_ssw_vlan_destroy(unit, vid)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
                return ret;
            }
        }
    }
    
    VLAN_SEM_LOCK(unit);
    if (1 == keep_and_restore_default_vlan)
    {
        /* configure vlan data entry */
        vlan_data_entry.vid     = RTK_DEFAULT_VLAN_ID;
        vlan_data_entry.fid     = RTK_DEFAULT_VLAN_ID;
        vlan_data_entry.msti    = RTK_DEFAULT_MSTI;
        HAL_GET_ALL_PORTMASK(unit, vlan_data_entry.member_portmask);
        HAL_GET_ALL_PORTMASK(unit, vlan_data_entry.untag_portmask);
        
        if ((ret = _dal_ssw_setVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
        {
            VLAN_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
            return ret;
        }
        
        /* Set vlan valid bit in software */
        VLANINFO_VALID_SET(unit, RTK_DEFAULT_VLAN_ID);
        pDal_ssw_vlan_info[unit]->count++;
    }
    
    VLAN_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_vlan_destroyAll */


/* Function Name:
 *      dal_ssw_vlan_fid_get
 * Description:
 *      Get the filter id of the vlan from the specified device.
 * Input: 
 *      unit  - unit id
 *      vid   - vlan id
 * Output:
 *      pFid - pointer buffer of filter id
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note: 
 *      1. In IVL mode, fid is equal with vid after vlan create.
 *      2. You don't need to care fid when you use the IVL mode.
 *      3. The API should be used for SVL mode.
 */
int32
dal_ssw_vlan_fid_get(uint32 unit, rtk_vlan_t vid, rtk_fid_t *pFid)
{
    int32           ret;
    dal_ssw_vlan_data_t vlan_data_entry;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, vid=%d", unit, vid);    
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
    
    /* VID check */
    RT_PARAM_CHK((vid < RTK_VLAN_ID_MIN) || (vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    RT_PARAM_CHK(VLANINFO_VALID_IS_CLEAR(unit, vid), RT_ERR_VLAN_ENTRY_NOT_FOUND);
    RT_PARAM_CHK((NULL == pFid), RT_ERR_NULL_POINTER);
    
    osal_memset(&vlan_data_entry, 0, sizeof(vlan_data_entry));
    
    /* configure vlan data entry */
    vlan_data_entry.vid     = vid;
    
    VLAN_SEM_LOCK(unit);
    
    /* get entry from CHIP*/
    if ((ret = _dal_ssw_getVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    VLAN_SEM_UNLOCK(unit);
    
    *pFid = vlan_data_entry.fid;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pFid=%x", *pFid); 

    return RT_ERR_OK;
} /* end of dal_ssw_vlan_fid_get */


/* Function Name:
 *      dal_ssw_vlan_fid_set
 * Description:
 *      Set the filter id of the vlan to the specified device.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 *      fid  - filter id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_OUT_OF_RANGE         - input parameter out of range
 * Note:
 *      1. In IVL mode, fid is equal with vid after vlan create.
 *      2. You don't need to care fid when you use the IVL mode.
 *      3. The API should be used for SVL mode.
 */
int32
dal_ssw_vlan_fid_set(uint32 unit, rtk_vlan_t vid, rtk_fid_t fid)
{
    int32 ret;
    dal_ssw_vlan_data_t vlan_data_entry;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, vid=%d, fid=%d", unit, vid, fid);
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
    
    /* VID check */
    RT_PARAM_CHK((vid < RTK_VLAN_ID_MIN) || (vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    /* TBD, should define FID. How to handle chip-dependent max value */
    RT_PARAM_CHK((fid > RTK_VLAN_ID_MAX), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(VLANINFO_VALID_IS_CLEAR(unit, vid), RT_ERR_VLAN_ENTRY_NOT_FOUND);
    
    osal_memset(&vlan_data_entry, 0, sizeof(vlan_data_entry));
    
    /* configure vlan data entry */
    vlan_data_entry.vid     = vid;
    
    VLAN_SEM_LOCK(unit);
    
    /* get entry from CHIP*/
    if ((ret = _dal_ssw_getVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    /* set fid to new fid */
    vlan_data_entry.fid = fid;
    
    /* programming to chip */
    if ((ret = _dal_ssw_setVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    VLAN_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_vlan_fid_set */


/* Function Name:
 *      dal_ssw_vlan_port_add
 * Description:
 *      Add one vlan member to the specified device.
 * Input:
 *      unit     - unit id
 *      vid      - vlan id
 *      port     - port id for add
 *      is_untag - untagged or tagged member
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_VLAN_PORT_MBR_EXIST  - member port exist in the specified vlan
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_OUT_OF_RANGE         - input parameter out of range
 * Note:
 *      1. The valid value of is_untag are {0: tagged, 1: untagged}
 */
int32
dal_ssw_vlan_port_add(uint32 unit, rtk_vlan_t vid, rtk_port_t port, uint32 is_untag)
{
    int32           ret;
    dal_ssw_vlan_data_t vlan_data_entry;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, vid=%d, port=%d, is_untag=%d",
           unit, vid, port, is_untag);
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
    
    /* VID check */
    RT_PARAM_CHK((vid < RTK_VLAN_ID_MIN) || (vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(is_untag > 1, RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(VLANINFO_VALID_IS_CLEAR(unit, vid), RT_ERR_VLAN_ENTRY_NOT_FOUND);
    
    osal_memset(&vlan_data_entry, 0, sizeof(vlan_data_entry));
    
    /* configure vlan data entry */
    vlan_data_entry.vid     = vid;
    
    VLAN_SEM_LOCK(unit);
    
    /* get entry from CHIP*/
    if ((ret = _dal_ssw_getVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    if (RTK_PORTMASK_IS_PORT_SET(vlan_data_entry.member_portmask, port))
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return RT_ERR_VLAN_PORT_MBR_EXIST;
    }
    
    /* set member_portmask and untag_portmask */
    RTK_PORTMASK_PORT_SET(vlan_data_entry.member_portmask, port);
    if (1 == is_untag)
    {
        RTK_PORTMASK_PORT_SET(vlan_data_entry.untag_portmask, port);
    }
    
    /* programming to chip */
    if ((ret = _dal_ssw_setVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    VLAN_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_vlan_port_add */


/* Function Name:
 *      dal_ssw_vlan_port_del
 * Description:
 *      Delete one vlan member from the specified device.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 *      port - port id for delete
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 * Note:
 *      None
 */
int32
dal_ssw_vlan_port_del(uint32 unit, rtk_vlan_t vid, rtk_port_t port)
{
    int32           ret;
    dal_ssw_vlan_data_t vlan_data_entry;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, vid=%d, port=%d",
           unit, vid, port);

    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
    
    /* VID check */
    RT_PARAM_CHK((vid < RTK_VLAN_ID_MIN) || (vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(VLANINFO_VALID_IS_CLEAR(unit, vid), RT_ERR_VLAN_ENTRY_NOT_FOUND);
    
    osal_memset(&vlan_data_entry, 0, sizeof(vlan_data_entry));
    
    /* configure vlan data entry */
    vlan_data_entry.vid     = vid;

    VLAN_SEM_LOCK(unit);
    
    /* get entry from CHIP*/
    if ((ret = _dal_ssw_getVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    /* clear member_portmask and untag_portmask */
    RTK_PORTMASK_PORT_CLEAR(vlan_data_entry.member_portmask, port);
    RTK_PORTMASK_PORT_CLEAR(vlan_data_entry.untag_portmask, port);
    
    /* programming to chip */
    if ((ret = _dal_ssw_setVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    VLAN_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_vlan_port_del */


/* Function Name:
 *      dal_ssw_vlan_port_get
 * Description:
 *      Get the vlan members from the specified device.
 * Input:
 *      unit              - unit id
 *      vid               - vlan id
 * Output:
 *      pMember_portmask  - pointer buffer of member ports
 *      pUntag_portmask   - pointer buffer of untagged member ports
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_ssw_vlan_port_get(
    uint32         unit,
    rtk_vlan_t     vid,
    rtk_portmask_t *pMember_portmask,
    rtk_portmask_t *pUntag_portmask)
{
    int32   ret;
    dal_ssw_vlan_data_t vlan_data_entry;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, vid=%d", unit, vid);
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
    
    /* VID check */
    RT_PARAM_CHK((vid < RTK_VLAN_ID_MIN) || (vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    RT_PARAM_CHK(((NULL == pMember_portmask) || (NULL == pUntag_portmask)), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(VLANINFO_VALID_IS_CLEAR(unit, vid), RT_ERR_VLAN_ENTRY_NOT_FOUND);
    
    
    osal_memset(&vlan_data_entry, 0, sizeof(vlan_data_entry));
    
    /* configure vlan data entry */
    vlan_data_entry.vid     = vid;
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/  
    if ((ret = _dal_ssw_getVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);

    RTK_PORTMASK_ASSIGN(*pMember_portmask, vlan_data_entry.member_portmask);
    RTK_PORTMASK_ASSIGN(*pUntag_portmask, vlan_data_entry.untag_portmask);
       
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%u, vid=%u, pMember_portmask=%x, pUntag_portmask=%x", 
           unit, vid, pMember_portmask->bits[0], pUntag_portmask->bits[0]);    
    
    return RT_ERR_OK;
} /* end of dal_ssw_vlan_port_get */


/* Function Name:
 *      dal_ssw_vlan_port_set
 * Description:
 *      Replace the vlan members in the specified device.
 * Input:
 *      unit              - unit id
 *      vid               - vlan id
 *      pMember_portmask  - member ports
 *      pUntag_portmask   - untagged member ports
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      1. Don't care the original vlan members and replace with new configure
 *         directly.
 */
int32
dal_ssw_vlan_port_set(
    uint32         unit,
    rtk_vlan_t     vid,
    rtk_portmask_t *pMember_portmask,
    rtk_portmask_t *pUntag_portmask)
{
    int32           ret;
    dal_ssw_vlan_data_t vlan_data_entry;
                  
    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, vid=%d", 
           unit, vid);    

    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
    
    /* VID check */
    RT_PARAM_CHK((vid < RTK_VLAN_ID_MIN) || (vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    RT_PARAM_CHK(((NULL == pMember_portmask) || (NULL == pUntag_portmask)), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(VLANINFO_VALID_IS_CLEAR(unit, vid), RT_ERR_VLAN_ENTRY_NOT_FOUND);
        
    osal_memset(&vlan_data_entry, 0, sizeof(vlan_data_entry));
    
    /* configure vlan data entry */
    vlan_data_entry.vid     = vid;
    
    VLAN_SEM_LOCK(unit);
    
    /* get entry from CHIP*/
    if ((ret = _dal_ssw_getVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    /* copy new portmask to vlan data entry */
    RTK_PORTMASK_ASSIGN(vlan_data_entry.member_portmask, *pMember_portmask);
    RTK_PORTMASK_ASSIGN(vlan_data_entry.untag_portmask, *pUntag_portmask);
    
    /* programming to chip */
    if ((ret = _dal_ssw_setVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    VLAN_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_vlan_port_set */


/* Function Name:
 *      dal_ssw_vlan_stg_get
 * Description:
 *      Get spanning tree group instance of the vlan from the specified device.
 * Input:
 *      unit  - unit id
 *      vid   - vlan id
 * Output:
 *      pStg  - pointer buffer of spanning tree group instance
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 */
int32
dal_ssw_vlan_stg_get(uint32 unit, rtk_vlan_t vid, rtk_stg_t *pStg)
{
    int32           ret;
    dal_ssw_vlan_data_t vlan_data_entry;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, vid=%d", unit, vid); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
    
    /* VID check */
    RT_PARAM_CHK((vid < RTK_VLAN_ID_MIN) || (vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    RT_PARAM_CHK((NULL == pStg), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(VLANINFO_VALID_IS_CLEAR(unit, vid), RT_ERR_VLAN_ENTRY_NOT_FOUND);
    
    osal_memset(&vlan_data_entry, 0, sizeof(vlan_data_entry));
    vlan_data_entry.vid = vid;
    
    VLAN_SEM_LOCK(unit);
    
    /* get entry from CHIP*/
    if ((ret = _dal_ssw_getVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    VLAN_SEM_UNLOCK(unit);
    
    /* set fid to new fid */
    *pStg = vlan_data_entry.msti;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pStg=%x", *pStg); 
    
    return RT_ERR_OK;
} /* end of dal_ssw_vlan_stg_get */


/* Function Name:
 *      dal_ssw_vlan_stg_set
 * Description:
 *      Set spanning tree group instance of the vlan to the specified device.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 *      stg  - spanning tree group instance
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_MSTI                 - invalid msti
 * Note:
 */
int32
dal_ssw_vlan_stg_set(uint32 unit, rtk_vlan_t vid, rtk_stg_t stg)
{
    int32           ret;
    dal_ssw_vlan_data_t vlan_data_entry;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, vid=%d, stg=%d", unit, vid, stg); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
    
    /* VID check */
    RT_PARAM_CHK((vid < RTK_VLAN_ID_MIN) || (vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    RT_PARAM_CHK((stg >= HAL_MAX_NUM_OF_MSTI(unit)), RT_ERR_MSTI);    
    RT_PARAM_CHK(VLANINFO_VALID_IS_CLEAR(unit, vid), RT_ERR_VLAN_ENTRY_NOT_FOUND); 
    
    osal_memset(&vlan_data_entry, 0, sizeof(vlan_data_entry));
    
    /* configure vlan data entry */
    vlan_data_entry.vid     = vid;
    
    VLAN_SEM_LOCK(unit);
    
    /* get entry from CHIP*/
    if ((ret = _dal_ssw_getVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    /* set fid to new fid */
    vlan_data_entry.msti = stg;
    
    /* programming to chip */
    if ((ret = _dal_ssw_setVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    VLAN_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_vlan_stg_set */


/* Module Name     : vlan                */
/* Sub-module Name : vlan port attribute */

/* Function Name:
 *      dal_ssw_vlan_portAcceptFrameType_get
 * Description:
 *      Get vlan accept frame type of the port from the specified device.
 * Input:
 *      unit                - unit id
 *      port                - port id
 * Output:
 *      pAccept_frame_type  - pointer buffer of accept frame type
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. The accept frame type as following:
 *          - ACCEPT_FRAME_TYPE_ALL
 *          - ACCEPT_FRAME_TYPE_TAG_ONLY
 *          - ACCEPT_FRAME_TYPE_UNTAG_ONLY
 *      2. The API is used for 802.1Q tagged  and if you want to get the 802.1ad
 *         accept frame type, please use dal_ssw_svlan_portAcceptFrameType_get
 */
int32
dal_ssw_vlan_portAcceptFrameType_get(
    uint32                     unit,
    rtk_port_t                 port,
    rtk_vlan_acceptFrameType_t *pAccept_frame_type)
{
    int32   ret;
    uint32  type;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pAccept_frame_type), RT_ERR_NULL_POINTER);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_field_read(unit, vlan_accept_tagged_or_untagged_control_regidx[port], (uint32)tag_accept_field[port], &type)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
    
    /* translate chip's value to definition */
    switch (type) 
    {
        case 0:
            *pAccept_frame_type = ACCEPT_FRAME_TYPE_ALL;
            break;
        case 1:
            *pAccept_frame_type = ACCEPT_FRAME_TYPE_TAG_ONLY;
            break;
        case 2:
            *pAccept_frame_type = ACCEPT_FRAME_TYPE_UNTAG_ONLY;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pAccept_frame_type=%x", *pAccept_frame_type);  
    
    return RT_ERR_OK;
} /* end of dal_ssw_vlan_portAcceptFrameType_get */


/* Function Name: 
 *      dal_ssw_vlan_portAcceptFrameType_set
 * Description:
 *      Set vlan accept frame type of the port to the specified device.
 * Input:
 *      unit              - unit id
 *      port              - port id
 *      accept_frame_type - accept frame type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_PORT_ID                - invalid port id
 *      RT_ERR_VLAN_ACCEPT_FRAME_TYPE - invalid accept frame type
 *      RT_ERR_INPUT                  - invalid input parameter
 *      RT_ERR_CHIP_NOT_SUPPORTED     - functions not supported by this chip model
 * Note:
 *      The accept frame type as following:
 *          - ACCEPT_FRAME_TYPE_ALL
 *          - ACCEPT_FRAME_TYPE_TAG_ONLY
 *          - ACCEPT_FRAME_TYPE_UNTAG_ONLY
 *      2. The API is used for 802.1Q tagged  and if you want to set the 802.1ad
 *         accept frame type, please use dal_ssw_svlan_portAcceptFrameType_set
 */
int32
dal_ssw_vlan_portAcceptFrameType_set(
    uint32                     unit,
    rtk_port_t                 port,
    rtk_vlan_acceptFrameType_t accept_frame_type)
{
    int32   ret;
    uint32  type;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, accept_frame_type=%d", 
           unit, port, accept_frame_type);
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((accept_frame_type >= ACCEPT_FRAME_TYPE_END), RT_ERR_INPUT);
    
    /* translate chip's value to definition */
    switch (accept_frame_type) 
    {
        case ACCEPT_FRAME_TYPE_ALL:
            type = 0;
            break;
        case ACCEPT_FRAME_TYPE_TAG_ONLY:
            type = 1;
            break;
        case ACCEPT_FRAME_TYPE_UNTAG_ONLY:
            type = 2;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_field_write(unit, vlan_accept_tagged_or_untagged_control_regidx[port], (uint32)tag_accept_field[port], &type)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
    
    
    return RT_ERR_OK;
} /* end of dal_ssw_vlan_portAcceptFrameType_set */

/* Function Name:
 *      dal_ssw_vlan_vlanFunctionEnable_get
 * Description:
 *      Get the VLAN enable status of the specified device.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_NULL_POINTER   - null pointer
 *      RT_ERR_UNIT_ID        - invalid unit id
 * Note:
 *      1. The status of vlan function is as following:
 *        - DISABLED
 *        - ENABLED
 */
int32
dal_ssw_vlan_vlanFunctionEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_field_read(unit, SSW_VLAN_CONTROLr, SSW_EN_VLANf, pEnable)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pEnable=%x", *pEnable);
    
    return RT_ERR_OK;
} /* end of dal_ssw_vlan_vlanFunctionEnable_get */

/* Function Name:
 *      dal_ssw_vlan_vlanFunctionEnable_set
 * Description:
 *      Set the VLAN enable status of the specified device.
 * Input:
 *      unit    - unit id
 *      enable  - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_INPUT          - invalid input parameter
 * Note:
 *      1. The status of vlan function is as following:
 *        - DISABLED
 *        - ENABLED
 */
int32
dal_ssw_vlan_vlanFunctionEnable_set(uint32 unit, rtk_enable_t enable)
{
    int32   ret;
    uint32  en_vlan;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, enable=%d", unit, enable);
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
    
    RT_PARAM_CHK(((DISABLED != enable) && (ENABLED != enable)) , RT_ERR_INPUT);
    
    if (ENABLED == enable)
    {
        en_vlan = 1;
    } 
    else 
    {
        en_vlan = 0;
    }
        
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_field_write(unit, SSW_VLAN_CONTROLr, SSW_EN_VLANf, &en_vlan)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_vlan_vlanFunctionEnable_set */

/* Function Name:
 *      dal_ssw_vlan_igrFilterEnable_get
 * Description:
 *      Get vlan ingress filter status from the specified device.
 * Input:
 *      unit         - unit id
 * Output:
 *      pIgr_filter  - pointer buffer of ingress filter status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. The status of vlan ingress filter is as following:
 *        - DISABLED
 *        - ENABLED
 */
int32
dal_ssw_vlan_igrFilterEnable_get(uint32 unit, rtk_enable_t *pIgr_filter)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pIgr_filter), RT_ERR_NULL_POINTER);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_field_read(unit, SSW_VLAN_CONTROLr, SSW_EN_IGR_FLTf, pIgr_filter)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pIgr_filter=%d", *pIgr_filter);
    
    return RT_ERR_OK;
} /* end of dal_ssw_vlan_igrFilterEnable_get */

/* Function Name:
 *      dal_ssw_vlan_igrFilterEnable_set
 * Description:
 *      Set vlan ingress filter status of the port to the specified device.
 * Input:
 *      unit       - unit id
 *      igr_filter - ingress filter configure
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_INPUT    - invalid input parameter
 *      RT_ERR_OUT_OF_RANGE - input value is out of range
 * Note:
 *      1. If the chip is per system configuration, set any port will apply to 
 *      whole device. like RTL8329/RTL8389 chip.
 *      2. The status of vlan ingress filter is as following:
 *        - DISABLED
 *        - ENABLED
 */
int32
dal_ssw_vlan_igrFilterEnable_set(uint32 unit, rtk_enable_t igr_filter)
{
    int32   ret;
    uint32  en_igr_filter;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, igr_filter=%d", unit, igr_filter);    
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(((DISABLED != igr_filter) && (ENABLED != igr_filter)) , RT_ERR_INPUT); 
    
    if (ENABLED == igr_filter)
    {
        en_igr_filter = 1;
    } 
    else 
    {
        en_igr_filter = 0;
    }
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_field_write(unit, SSW_VLAN_CONTROLr, SSW_EN_IGR_FLTf, &en_igr_filter)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_vlan_igrFilterEnable_set */

/* Function Name:
 *      dal_ssw_vlan_mcastLeakyEnable_get
 * Description:
 *      Get vlan egress leaky status of the system from the specified device.
 * Input:
 *      unit    - unit id
 * Output:
 *      pLeaky  - pointer buffer of vlan leaky of egress
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. The status of vlan egress multicast leaky is as following:
 *        - DISABLED
 *        - ENABLED
 */
int32
dal_ssw_vlan_mcastLeakyEnable_get(uint32 unit, rtk_enable_t *pLeaky)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d", unit); 

    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pLeaky), RT_ERR_NULL_POINTER);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_field_read(unit, SSW_VLAN_CONTROLr, SSW_EN_VLEAKYf, pLeaky)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pLeaky=%d", *pLeaky); 
    
    return RT_ERR_OK;
} /* end of dal_ssw_vlan_mcastLeakyEnable_get */


/* Function Name:
 *      dal_ssw_vlan_mcastLeakyEnable_set
 * Description:
 *      Set vlan egress leaky configure of the system to the specified device.
 * Input:
 *      unit  - unit id
 *      leaky - vlan leaky of egress
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_INPUT    - invalid input parameter
 *      RT_ERR_OUT_OF_RANGE - input value is out of range
 * Note:
 *      1. egress vlan leaky configuration is apply to L2/IP multicast packet only,
 *         no apply to unicast.
 *      2. The status of vlan egress multicast leaky is as following:
 *        - DISABLED
 *        - ENABLED
 */
int32
dal_ssw_vlan_mcastLeakyEnable_set(uint32 unit, rtk_enable_t leaky)
{
    int32   ret;
    uint32  en_leaky;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, leaky=%d", unit, leaky); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(((DISABLED != leaky) && (ENABLED != leaky)) , RT_ERR_INPUT); 
    
    if (ENABLED == leaky)
    {
        en_leaky = 1;
    } 
    else 
    {
        en_leaky = 0;
    }
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_field_write(unit, SSW_VLAN_CONTROLr, SSW_EN_VLEAKYf, &en_leaky)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_vlan_mcastLeakyEnable_set */


/* Function Name:
 *      dal_ssw_vlan_mcastLeakyPortEnable_get
 * Description:
 *      Get vlan egress leaky status of the port from the specified device.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pLeaky  - pointer buffer of vlan leaky of egress
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT           - The module is not initial
 *      RT_ERR_UNIT_ID            - invalid unit id
 *      RT_ERR_PORT_ID            - invalid port id
 *      RT_ERR_NULL_POINTER       - input parameter may be null pointer
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model  
 * Note:
 *      1. The status of vlan egress multicast leaky is as following:
 *        - DISABLED
 *        - ENABLED
 */
int32
dal_ssw_vlan_mcastLeakyPortEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pLeaky)
{
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d",
           unit, port); 

    return RT_ERR_CHIP_NOT_SUPPORTED;
} /* end of dal_ssw_vlan_mcastLeakyPortEnable_get */


/* Function Name:
 *      dal_ssw_vlan_mcastLeakyPortEnable_set
 * Description:
 *      Set vlan egress leaky configure of the port to the specified device.
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      leaky - vlan leaky of egress
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT           - The module is not initial
 *      RT_ERR_UNIT_ID            - invalid unit id
 *      RT_ERR_PORT_ID            - invalid port id
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model  
 * Note:
 *      1. egress vlan leaky configuration is apply to L2/IP multicast packet only,
 *         no apply to unicast.
 *      2. If the chip is per system configuration, set any port will apply
 *         to whole device.
 *      3. The status of vlan egress multicast leaky is as following:
 *        - DISABLED
 *        - ENABLED
 */
int32
dal_ssw_vlan_mcastLeakyPortEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t leaky)
{
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, leaky=%d", 
           unit, port, leaky); 

    return RT_ERR_CHIP_NOT_SUPPORTED;
} /* end of dal_ssw_vlan_mcastLeakyPortEnable_set */


/* Function Name:
 *      dal_ssw_vlan_portPvid_get
 * Description:
 *      Get port default vlan id from the specified device.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pPvid  - pointer buffer of port default vlan id
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_ssw_vlan_portPvid_get(uint32 unit, rtk_port_t port, uint32 *pPvid)
{
    int32 ret;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pPvid), RT_ERR_NULL_POINTER);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_field_read(unit, tag_based_and_port_based_control_regidx[port], (uint32)pvid_fieldidx[port], pPvid)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pPvid=%d", *pPvid);     
    
    return RT_ERR_OK;
} /* end of dal_ssw_vlan_portPvid_get */


/* Function Name:
 *      dal_ssw_vlan_portPvid_set
 * Description:
 *      Set port default vlan id to the specified device.
 * Input:
 *      unit - unit id
 *      port - port id
 *      pvid - port default vlan id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_VLAN_VID - invalid vid
 * Note:
 *      None
 */
int32
dal_ssw_vlan_portPvid_set(uint32 unit, rtk_port_t port, uint32 pvid)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, pvid=%d", 
           unit, port, pvid); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((pvid < RTK_VLAN_ID_MIN) || (pvid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    VLAN_SEM_LOCK(unit);
    /* write entry entry from CHIP*/
    if ((ret = reg_field_write(unit, tag_based_and_port_based_control_regidx[port], (uint32)pvid_fieldidx[port], &pvid)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    VLAN_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_vlan_portPvid_set */


/* Function Name:
 *      dal_ssw_vlan_tagMode_get
 * Description:
 *      Get vlan tagged mode of the port from the specified device.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pTag_mode  - pointer buffer of vlan tagged mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. The vlan tagged mode as following:
 *          - VLAN_TAG_MODE_ORIGINAL        (depend on chip normal decision)
 *          - VLAN_TAG_MODE_KEEP_FORMAT     (keep ingress format to egress)
 *          - VLAN_TAG_MODE_PRI             (always priority tag out)
 */
int32
dal_ssw_vlan_tagMode_get(uint32 unit, rtk_port_t port, rtk_vlan_tagMode_t *pTag_mode)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pTag_mode), RT_ERR_NULL_POINTER);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_field_read(unit, vlan_mode_control_regidx[port], (uint32)vlanmode_fieldidx[port], pTag_mode)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pTag_mode=%d", *pTag_mode);    
    
    return RT_ERR_OK;
} /* end of dal_ssw_vlan_tagMode_get */

/* Function Name:
 *      dal_ssw_vlan_tagMode_set
 * Description:
 *      Set vlan tagged mode of the port to the specified device.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      tag_mode - vlan tagged mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      1. The vlan tagged mode as following:
 *          - VLAN_TAG_MODE_ORIGINAL        (depend on chip normal decision)
 *          - VLAN_TAG_MODE_KEEP_FORMAT     (keep ingress format to egress)
 *          - VLAN_TAG_MODE_PRI             (always priority tag out)
 */
int32
dal_ssw_vlan_tagMode_set(uint32 unit, rtk_port_t port, rtk_vlan_tagMode_t tag_mode)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, tag_mode=%x", 
           unit, port, tag_mode); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((tag_mode >= VLAN_TAG_MODE_END), RT_ERR_INPUT); 
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_field_write(unit, vlan_mode_control_regidx[port], (uint32)vlanmode_fieldidx[port], &tag_mode)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_vlan_tagMode_set */


/* Internal Function Body */

/* Function Name:
 *      _dal_ssw_vlan_init_config
 * Description:
 *      Initialize default config of vlan for the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Must initialize vlan module before calling this API.
 */
static int32
_dal_ssw_vlan_init_config(uint32 unit) 
{
    int32       ret;
    dal_ssw_vlan_data_t vlan_data_entry;
    rtk_port_t  port, max_port;
    
    
    if ((ret = dal_ssw_vlan_vlanFunctionEnable_set(unit, RTK_DEFAULT_VLAN_FUNCTION)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "enable vlan function failed");
        return ret;
    }
    
    /* configure vlan data entry */
    vlan_data_entry.vid     = RTK_DEFAULT_VLAN_ID;
    vlan_data_entry.fid     = RTK_DEFAULT_VLAN_ID;
    vlan_data_entry.msti    = RTK_DEFAULT_MSTI;
    HAL_GET_ALL_PORTMASK(unit, vlan_data_entry.member_portmask);
    HAL_GET_ALL_PORTMASK(unit, vlan_data_entry.untag_portmask);
    
    if ((ret = _dal_ssw_setVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "create default vlan entry failed");
        return ret;
    }
    
     /* Set vlan valid bit in software */
    VLANINFO_VALID_SET(unit, RTK_DEFAULT_VLAN_ID);
    pDal_ssw_vlan_info[unit]->count++;
    
    max_port = HAL_GET_MAX_PORT(unit);
    for (port = 0; port < max_port; port++)
    {
        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }
        
        if ((ret = dal_ssw_vlan_portPvid_set(unit, port, RTK_DEFAULT_PORT_VID)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_VLAN|MOD_DAL), "set default port pvid failed");
            return ret; 
        }
        
        if (HAL_IS_CPU_PORT(unit, port))
        {
            ret = dal_ssw_vlan_tagMode_set(unit, port, RTK_DEFAULT_VLAN_TAGMODE_IN_CPU);
        } 
        else 
        {
            ret = dal_ssw_vlan_tagMode_set(unit, port, RTK_DEFAULT_VLAN_TAGMODE);
        }
        
        if (RT_ERR_OK != ret)
        {
            RT_ERR(ret, (MOD_VLAN|MOD_DAL), "set default port vlan tag mode failed");
            return ret;
        }
    }
    
    return RT_ERR_OK;
} /* end of _dal_ssw_vlan_init_config */

/* Function Name:
 *      _dal_ssw_setVlan
 * Description:
 *      Set vlan entry to chip.
 * Input:
 *      unit        - unit id
 *      pVlan_entry - content of vlan entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 */
static int32 _dal_ssw_setVlan(uint32 unit, dal_ssw_vlan_data_t *pVlan_entry)
{
    int32   ret;
    vlan_entry_t    vlan_entry;
    uint32  temp_var;
    
    RT_DBG(LOG_TRACE, (MOD_VLAN|MOD_DAL), "unit=%d, vid=%d, fid=%d, msti=%d, member_portmask=%x, untag_portmask=%x", 
           unit, pVlan_entry->vid, pVlan_entry->fid, pVlan_entry->msti, pVlan_entry->member_portmask, pVlan_entry->untag_portmask);
    
    osal_memset(&vlan_entry, 0, sizeof(vlan_entry));
    
    /* set member set */
    if ((ret = table_field_set(unit, SSW_VLAN_TABLEt, SSW_VLAN_TABLE_MEMBER_SETf, pVlan_entry->member_portmask.bits, (uint32 *) &vlan_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    /* set untagged member set */
    if ((ret = table_field_set(unit, SSW_VLAN_TABLEt, SSW_VLAN_TABLE_UNTAG_SETf, pVlan_entry->untag_portmask.bits, (uint32 *) &vlan_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    /* set fid */
    temp_var = pVlan_entry->fid;
    if ((ret = table_field_set(unit, SSW_VLAN_TABLEt, SSW_VLAN_TABLE_FIDf, &temp_var, (uint32 *) &vlan_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }    
    
    /* set msti */
    temp_var = pVlan_entry->msti;
    if ((ret = table_field_set(unit, SSW_VLAN_TABLEt, SSW_VLAN_TABLE_MSTIf, &temp_var, (uint32 *) &vlan_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    /* programming vlan entry in chip */
    if ((ret = table_write(unit, SSW_VLAN_TABLEt, pVlan_entry->vid, (uint32 *) &vlan_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    return RT_ERR_OK;
} /* end of _dal_ssw_setVlan */

/* Function Name:
 *      _dal_ssw_getVlan
 * Description:
 *      Get vlan entry from chip.
 * Input:
 *      unit        - unit id
 *      pVlan_entry - buffer of vlan entry
 * Output:
 *      pVlan_entry - content of vlan entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 */
static int32 _dal_ssw_getVlan(uint32 unit, dal_ssw_vlan_data_t *pVlan_entry)
{
    int32   ret;
    vlan_entry_t    vlan_entry;
    uint32  temp_var;
    
    RT_DBG(LOG_TRACE, (MOD_VLAN|MOD_DAL), "unit=%d, vid=%d", unit, pVlan_entry->vid);
    
    osal_memset(&vlan_entry, 0, sizeof(vlan_entry));
 
    /* get entry from chip */
    if ((ret = table_read(unit, SSW_VLAN_TABLEt, pVlan_entry->vid, (uint32 *) &vlan_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }

    /* get member set from vlan_entry */
    if ((ret = table_field_get(unit, SSW_VLAN_TABLEt, SSW_VLAN_TABLE_MEMBER_SETf, pVlan_entry->member_portmask.bits, (uint32 *) &vlan_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }

    /* get untagged member set */
    if ((ret = table_field_get(unit, SSW_VLAN_TABLEt, SSW_VLAN_TABLE_UNTAG_SETf, pVlan_entry->untag_portmask.bits, (uint32 *) &vlan_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }

    /* get fid */
    if ((ret = table_field_get(unit, SSW_VLAN_TABLEt, SSW_VLAN_TABLE_FIDf, &temp_var, (uint32 *) &vlan_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }    
    pVlan_entry->fid = temp_var;

    /* get msti */
    if ((ret = table_field_get(unit, SSW_VLAN_TABLEt, SSW_VLAN_TABLE_MSTIf, &temp_var, (uint32 *) &vlan_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    pVlan_entry->msti = temp_var;

    RT_DBG(LOG_TRACE, (MOD_VLAN|MOD_DAL), "member_portmask=%x, untag_portmask=%x\
           fid=%d, msti=%d", pVlan_entry->member_portmask.bits[0], pVlan_entry->untag_portmask.bits[0], 
           pVlan_entry->fid, pVlan_entry->msti);
    
    return RT_ERR_OK;
} /* end of _dal_ssw_getVlan */
