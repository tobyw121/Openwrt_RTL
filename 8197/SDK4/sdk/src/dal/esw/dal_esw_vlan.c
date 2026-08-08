/*
 * Copyright(c) Realtek Semiconductor Corporation, 2009
 * All rights reserved.
 *
 * $Revision: 24800 $
 * $Date: 2009-08-25 15:01:31 +0800 (???¤? 25 ?«䥿* Purpose : Definition those public vlan APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) Vlan table configure and modification
 *           2) Accept frame type
 *           3) Vlan ingress/egress filter
 *           4) Port based and protocol based vlan
 *           5) TPID configuration
 *           6) Ingress tag handling
 *           7) Tag format handling
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
#include <hal/chipdef/esw/rtk_esw_table_struct.h>
#include <hal/chipdef/esw/rtk_esw_reg_struct.h>
#include <hal/mac/reg.h>
#include <hal/mac/mem.h>
#include <hal/common/halctrl.h>
#include <dal/esw/dal_esw_vlan.h>
#include <rtk/default.h>
#include <rtk/vlan.h>


/*
 * Symbol Definition
 */
/* vlan information structure */
typedef struct dal_esw_vlan_info_s
{
    uint32 count;           /* count of valid vlan number    */
    uint32 *pValid_lists;   /* valid bit for this table      */
    uint16 *pVid2tblindex;  /* table index of vid, 0:invalid */
} dal_esw_vlan_info_t;

/* vlan entry*/
typedef struct dal_esw_vlan_data_s
{
    rtk_vlan_t          vid;
    rtk_fid_t           fid;
    rtk_vlan_fwdMode_t  vlan_base_fwd;
    rtk_portmask_t      member_portmask;
    rtk_portmask_t      untag_portmask;
} dal_esw_vlan_data_t;

typedef enum dal_esw_vlan_egrTagStatus_e
{
    EGR_BOTH_TAG = 0,
    EGR_INNER_TAG_ONLY,
    EGR_OUTER_TAG_ONLY,
    EGR_UNTAG,
    DAL_ESW_EGRTAGSTATUS_END
} dal_esw_vlan_egrTagStatus_t;

/*
 * Data Declaration
 */
static uint32               vlan_init[RTK_MAX_NUM_OF_UNIT];
static osal_mutex_t         vlan_sem[RTK_MAX_NUM_OF_UNIT];
static dal_esw_vlan_info_t  *pDal_esw_vlan_info[RTK_MAX_NUM_OF_UNIT];

const static uint16 port_and_protocol_based_vlan_entry0_regidx[] = 
    {ESW_PORT_PORT_AND_PROTOCOL_BASED_VLAN_ENTRY0_0r
   , ESW_PORT_PORT_AND_PROTOCOL_BASED_VLAN_ENTRY1_0r
   , ESW_PORT_PORT_AND_PROTOCOL_BASED_VLAN_ENTRY2_0r
   , ESW_PORT_PORT_AND_PROTOCOL_BASED_VLAN_ENTRY3_0r
   , ESW_PORT_PORT_AND_PROTOCOL_BASED_VLAN_ENTRY4_0r
   , ESW_PORT_PORT_AND_PROTOCOL_BASED_VLAN_ENTRY5_0r
   , ESW_PORT_PORT_AND_PROTOCOL_BASED_VLAN_ENTRY6_0r
   , ESW_PORT_PORT_AND_PROTOCOL_BASED_VLAN_ENTRY7_0r};

const static uint16 port_and_protocol_based_vlan_entry1_regidx[] = 
    {ESW_PORT_PORT_AND_PROTOCOL_BASED_VLAN_ENTRY0_1r
   , ESW_PORT_PORT_AND_PROTOCOL_BASED_VLAN_ENTRY1_1r
   , ESW_PORT_PORT_AND_PROTOCOL_BASED_VLAN_ENTRY2_1r
   , ESW_PORT_PORT_AND_PROTOCOL_BASED_VLAN_ENTRY3_1r
   , ESW_PORT_PORT_AND_PROTOCOL_BASED_VLAN_ENTRY4_1r
   , ESW_PORT_PORT_AND_PROTOCOL_BASED_VLAN_ENTRY5_1r
   , ESW_PORT_PORT_AND_PROTOCOL_BASED_VLAN_ENTRY6_1r
   , ESW_PORT_PORT_AND_PROTOCOL_BASED_VLAN_ENTRY7_1r};

/*
 * Macro Declaration
 */
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

#define VLANINFO_VALID_IS_SET(unit, vid)    BITMAP_IS_SET(pDal_esw_vlan_info[unit]->pValid_lists, vid)
#define VLANINFO_VALID_IS_CLEAR(unit, vid)  BITMAP_IS_CLEAR(pDal_esw_vlan_info[unit]->pValid_lists, vid)
#define VLANINFO_VALID_SET(unit, vid)       BITMAP_SET(pDal_esw_vlan_info[unit]->pValid_lists, vid)
#define VLANINFO_VALID_CLEAR(unit, vid)     BITMAP_CLEAR(pDal_esw_vlan_info[unit]->pValid_lists, vid)

/* ED Temporary define */
#define RTK_DEFAULT_VLAN_FWD_MODE   (VLAN_FWD_ON_ALE)

/*
 * Function Declaration
 */
static int32 _dal_esw_setVlan(uint32 unit, dal_esw_vlan_data_t *pVlan_entry);
static int32 _dal_esw_getVlan(uint32 unit, dal_esw_vlan_data_t *pVlan_entry);
static int32 _dal_esw_vlan_init_config(uint32 unit); 
/* Module Name : vlan */ 
 
/* Function Name:
 *      dal_esw_vlan_init
 * Description:
 *      Initialize vlan module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      Must initialize vlan module before calling any vlan APIs.
 */
int32
dal_esw_vlan_init(uint32 unit)
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
    if ((ret = table_size_get(unit, ESW_VLANt, &vlan_tableSize)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "table size get failed");
        return ret;
    }
    
    num_of_vlan_1bitlist = (vlan_tableSize + 31) >> 5;
    
    VLAN_SEM_LOCK(unit);
    
    /* allocate memory for each database and initilize database */
    pDal_esw_vlan_info[unit] = (dal_esw_vlan_info_t *)osal_alloc(sizeof(dal_esw_vlan_info_t));
    if (0 == pDal_esw_vlan_info[unit])
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(RT_ERR_FAILED, (MOD_VLAN|MOD_DAL), "memory allocate failed");
        return RT_ERR_FAILED;
    }
    osal_memset(pDal_esw_vlan_info[unit], 0, sizeof(dal_esw_vlan_info_t));
    
    pDal_esw_vlan_info[unit]->pValid_lists = (uint32 *)osal_alloc(num_of_vlan_1bitlist * sizeof(uint32));
    if (0 == pDal_esw_vlan_info[unit]->pValid_lists)
    {
        osal_free(pDal_esw_vlan_info[unit]);
        pDal_esw_vlan_info[unit] = 0;
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(RT_ERR_FAILED, (MOD_VLAN|MOD_DAL), "memory allocate failed");
        return RT_ERR_FAILED;
    }
    osal_memset(pDal_esw_vlan_info[unit]->pValid_lists, 0, (num_of_vlan_1bitlist * sizeof(uint32)));
    
    pDal_esw_vlan_info[unit]->pVid2tblindex = (uint16 *)osal_alloc(vlan_tableSize * sizeof(uint16));
    if (0 == pDal_esw_vlan_info[unit]->pVid2tblindex)
    {
        osal_free(pDal_esw_vlan_info[unit]->pValid_lists);
        pDal_esw_vlan_info[unit]->pValid_lists = 0;
        osal_free(pDal_esw_vlan_info[unit]);
        pDal_esw_vlan_info[unit] = 0;
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(RT_ERR_FAILED, (MOD_VLAN|MOD_DAL), "memory allocate failed");
        return RT_ERR_FAILED;
    }
    osal_memset(pDal_esw_vlan_info[unit]->pVid2tblindex, 0, (vlan_tableSize * sizeof(uint16)));
    VLAN_SEM_UNLOCK(unit);
    
    /* set init flag to complete init */
    vlan_init[unit] = INIT_COMPLETED; 
    
    if (( ret = _dal_esw_vlan_init_config(unit)) != RT_ERR_OK)
    {
        vlan_init[unit] = INIT_NOT_COMPLETED;
        osal_free(pDal_esw_vlan_info[unit]->pVid2tblindex);
        pDal_esw_vlan_info[unit]->pVid2tblindex = 0;
        osal_free(pDal_esw_vlan_info[unit]->pValid_lists);
        pDal_esw_vlan_info[unit]->pValid_lists = 0;
        osal_free(pDal_esw_vlan_info[unit]);
        pDal_esw_vlan_info[unit] = 0;
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "Init default vlan config failed");
        return ret;
    }
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_init */

/* Module Name    : Vlan                                  */
/* Sub-module Name: Vlan table configure and modification */

/* Function Name:
 *      dal_esw_vlan_create
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
 *         rtk_vlan_fid_set
 *         rtk_vlan_stg_set
 */
int32
dal_esw_vlan_create(uint32 unit, rtk_vlan_t vid)
{
    int32   ret;
    dal_esw_vlan_data_t vlan_data_entry;

    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, vid=%d", unit, vid);

    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* VID check */
    RT_PARAM_CHK((vid < RTK_VLAN_ID_MIN) || (vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    RT_PARAM_CHK(VLANINFO_VALID_IS_SET(unit, vid), RT_ERR_VLAN_EXIST);
    
    /* create vlan in CHIP*/
    osal_memset(&vlan_data_entry, 0, sizeof(vlan_data_entry));
    
    /* configure vlan data entry */
    vlan_data_entry.vid = vid;
    vlan_data_entry.fid = RTK_DEFAULT_MSTI;
    vlan_data_entry.vlan_base_fwd = RTK_DEFAULT_VLAN_FWD_MODE;   
    
    VLAN_SEM_LOCK(unit);
    
    /* write entry to CHIP*/
    if ((ret = _dal_esw_setVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    /* Set vlan valid bit in software */
    VLANINFO_VALID_SET(unit, vid);
    pDal_esw_vlan_info[unit]->count++;
    
    VLAN_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_create */

/* Function Name:
 *      dal_esw_vlan_destroy
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
dal_esw_vlan_destroy(uint32 unit, rtk_vlan_t vid)
{
    
    int32 ret;
    dal_esw_vlan_data_t vlan_data_entry;
    
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
    if ((ret = _dal_esw_setVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    /* Unset vlan valid bit in software */
    VLANINFO_VALID_CLEAR(unit, vid);
    pDal_esw_vlan_info[unit]->count--;
    
    VLAN_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_destroy */

/* Function Name:
 *      dal_esw_vlan_destroyAll
 * Description:
 *      Destroy all vlans except default vlan in the specified device.
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
dal_esw_vlan_destroyAll(uint32 unit, uint32 restore_default_vlan)
{
    int32       ret;
    dal_esw_vlan_data_t vlan_data_entry;
    rtk_vlan_t  vid;
    uint32      vlan_tableSize;
    
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, restore_default_vlan=%d",
           unit, restore_default_vlan);
    
    RT_INIT_CHK(vlan_init[unit]);
    
    if ((ret = table_size_get(unit, ESW_VLANt, &vlan_tableSize)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "table size get failed");
        return ret;
    }
    
    for (vid = 0; vid < vlan_tableSize; vid++)
    {
        if (VLANINFO_VALID_IS_SET(unit, vid))
        {
            if ((ret = dal_esw_vlan_destroy(unit, vid)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
                return ret;
            }
        }
    }
    
    VLAN_SEM_LOCK(unit);
    if (1 == restore_default_vlan)
    {
        /* configure vlan data entry */
        vlan_data_entry.vid     = RTK_DEFAULT_VLAN_ID;
        vlan_data_entry.fid     = RTK_DEFAULT_VLAN_ID;
        vlan_data_entry.vlan_base_fwd = RTK_DEFAULT_VLAN_FWD_MODE;  
        HAL_GET_ALL_PORTMASK(unit, vlan_data_entry.member_portmask);
        HAL_GET_ALL_PORTMASK(unit, vlan_data_entry.untag_portmask);
        
        if ((ret = _dal_esw_setVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
        {
            VLAN_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
            return ret;
        }
        
        /* Set vlan valid bit in software */
        VLANINFO_VALID_SET(unit, RTK_DEFAULT_VLAN_ID);
        pDal_esw_vlan_info[unit]->count++;
    }
    
    VLAN_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_destroyAll */

/* Function Name:
 *      dal_esw_vlan_fid_get
 * Description:
 *      Get the filter id of the vlan from the specified device.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
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
 *      1. fid is equal 0 after vlan create.
 *      2. You don't need to care fid when you use the IVL mode.
 *      3. The API should be used for SVL mode.
 */
int32
dal_esw_vlan_fid_get(uint32 unit, rtk_vlan_t vid, rtk_fid_t *pFid)
{
    int32   ret;
    dal_esw_vlan_data_t vlan_data_entry;
    
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
    if ((ret = _dal_esw_getVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    VLAN_SEM_UNLOCK(unit);
    
    *pFid = vlan_data_entry.fid;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pFid=%x", *pFid); 

    return RT_ERR_OK;
} /* end of dal_esw_vlan_fid_get */

/* Function Name:
 *      dal_esw_vlan_fid_set
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
 *      1. fid is equal 0 after vlan create.
 *      2. You don't need to care fid when you use the IVL mode.
 *      3. The API should be used for SVL mode.
 */
int32
dal_esw_vlan_fid_set(uint32 unit, rtk_vlan_t vid, rtk_fid_t fid)
{
    int32 ret;
    dal_esw_vlan_data_t vlan_data_entry;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, vid=%d, fid=%d", unit, vid, fid);
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
    
    /* VID check */
    RT_PARAM_CHK((vid < RTK_VLAN_ID_MIN) || (vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    /* TBD, should define FID. How to handle chip-dependent max value */
    RT_PARAM_CHK((fid > HAL_VLAN_FID_MAX(unit)), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(VLANINFO_VALID_IS_CLEAR(unit, vid), RT_ERR_VLAN_ENTRY_NOT_FOUND);
    
    osal_memset(&vlan_data_entry, 0, sizeof(vlan_data_entry));
    
    /* configure vlan data entry */
    vlan_data_entry.vid     = vid;
    
    VLAN_SEM_LOCK(unit);
    
    /* get entry from CHIP*/
    if ((ret = _dal_esw_getVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    /* set fid to new fid */
    vlan_data_entry.fid = fid;
    
    /* programming to chip */
    if ((ret = _dal_esw_setVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    VLAN_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_fid_set */

/* Function Name:
 *      dal_esw_vlan_port_add
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
dal_esw_vlan_port_add(uint32 unit, rtk_vlan_t vid, rtk_port_t port, uint32 is_untag)
{
    int32           ret;
    dal_esw_vlan_data_t vlan_data_entry;
    
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
    if ((ret = _dal_esw_getVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
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
    if ((ret = _dal_esw_setVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    VLAN_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_port_add */

/* Function Name:
 *      dal_esw_vlan_port_del
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
dal_esw_vlan_port_del(uint32 unit, rtk_vlan_t vid, rtk_port_t port)
{
    int32           ret;
    dal_esw_vlan_data_t vlan_data_entry;
    
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
    if ((ret = _dal_esw_getVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    /* clear member_portmask and untag_portmask */
    RTK_PORTMASK_PORT_CLEAR(vlan_data_entry.member_portmask, port);
    RTK_PORTMASK_PORT_CLEAR(vlan_data_entry.untag_portmask, port);
    
    /* programming to chip */
    if ((ret = _dal_esw_setVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    VLAN_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_port_del */

/* Function Name:
 *      dal_esw_vlan_port_get
 * Description:
 *      Get the vlan members from the specified device.
 * Input:
 *      unit             - unit id
 *      vid              - vlan id
 * Output:
 *      pMember_portmask - pointer buffer of member ports
 *      pUntag_portmask  - pointer buffer of untagged member ports
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
dal_esw_vlan_port_get(
    uint32         unit,
    rtk_vlan_t     vid,
    rtk_portmask_t *pMember_portmask,
    rtk_portmask_t *pUntag_portmask)
{
    int32   ret;
    dal_esw_vlan_data_t vlan_data_entry;
    
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
    if ((ret = _dal_esw_getVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);

    RTK_PORTMASK_ASSIGN(*pMember_portmask, vlan_data_entry.member_portmask);
    RTK_PORTMASK_ASSIGN(*pUntag_portmask, vlan_data_entry.untag_portmask);
       
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pMember_portmask=%x, pUntag_portmask=%x", 
           unit, vid, pMember_portmask->bits[0], pUntag_portmask->bits[0]);    
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_port_get */

/* Function Name:
 *      dal_esw_vlan_port_set
 * Description:
 *      Replace the vlan members in the specified device.
 * Input:
 *      unit             - unit id
 *      vid              - vlan id
 *      pMember_portmask - member ports
 *      pUntag_portmask  - untagged member ports
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
dal_esw_vlan_port_set(
    uint32         unit,
    rtk_vlan_t     vid,
    rtk_portmask_t *pMember_portmask,
    rtk_portmask_t *pUntag_portmask)
{
    int32           ret;
    dal_esw_vlan_data_t vlan_data_entry;
                  
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
    if ((ret = _dal_esw_getVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    /* copy new portmask to vlan data entry */
    RTK_PORTMASK_ASSIGN(vlan_data_entry.member_portmask, *pMember_portmask);
    RTK_PORTMASK_ASSIGN(vlan_data_entry.untag_portmask, *pUntag_portmask);
    
    /* programming to chip */
    if ((ret = _dal_esw_setVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    VLAN_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_port_set */


/* Module Name    : Vlan              */
/* Sub-module Name: Accept frame type */

/* Function Name:
 *      dal_esw_vlan_portAcceptFrameType_get
 * Description:
 *      Get vlan accept frame type of the port from the specified device.
 * Input:
 *      unit               - unit id
 *      port               - port id
 * Output:
 *      pAccept_frame_type - pointer buffer of accept frame type
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
 *         accept frame type, please use rtk_svlan_portAcceptFrameType_get
 */
int32
dal_esw_vlan_portAcceptFrameType_get(
    uint32                     unit,
    rtk_port_t                 port,
    rtk_vlan_acceptFrameType_t *pAccept_frame_type)
{
    int32   ret;
    uint32  accept_tag, accept_untag, type;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pAccept_frame_type), RT_ERR_NULL_POINTER);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_ACCEPTED_FRAME_TYPE_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_ACPTITAGf, &accept_tag)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    if ((ret = reg_array_field_read(unit, ESW_PORT_ACCEPTED_FRAME_TYPE_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_ACPTIUTAGf, &accept_untag)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }    
    VLAN_SEM_UNLOCK(unit);
    
    type = ((accept_tag << 1) | accept_untag);
    
    /* translate chip's value to definition */
    switch (type) 
    {
        case 1:
            *pAccept_frame_type = ACCEPT_FRAME_TYPE_UNTAG_ONLY;
            break;
        case 2:
            *pAccept_frame_type = ACCEPT_FRAME_TYPE_TAG_ONLY;
            break;
        case 3:
            *pAccept_frame_type = ACCEPT_FRAME_TYPE_ALL;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pAccept_frame_type=%x", *pAccept_frame_type);  
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portAcceptFrameType_get */

/* Function Name:
 *      dal_esw_vlan_portAcceptFrameType_set
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
 *      RT_ERR_CHIP_NOT_SUPPORTED     - functions not supported by this chip model
 * Note:
 *      1. The accept frame type as following:
 *         - ACCEPT_FRAME_TYPE_ALL
 *         - ACCEPT_FRAME_TYPE_TAG_ONLY
 *         - ACCEPT_FRAME_TYPE_UNTAG_ONLY
 *      2. The API is used for 802.1Q tagged  and if you want to set the 802.1ad
 *         accept frame type, please use rtk_svlan_portAcceptFrameType_set
 */
int32
dal_esw_vlan_portAcceptFrameType_set(
    uint32                     unit,
    rtk_port_t                 port,
    rtk_vlan_acceptFrameType_t accept_frame_type)
{
    uint32  accept_tag, accept_untag;
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, accept_frame_type=%d", 
           unit, port, accept_frame_type);
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((accept_frame_type > ACCEPT_FRAME_TYPE_UNTAG_ONLY), RT_ERR_INPUT);
    
    /* translate chip's value to definition */
    switch (accept_frame_type) 
    {
        case ACCEPT_FRAME_TYPE_ALL:
            accept_tag = 1;
            accept_untag = 1;
            break;
        case ACCEPT_FRAME_TYPE_TAG_ONLY:
            accept_tag = 1;
            accept_untag = 0;
            break;
        case ACCEPT_FRAME_TYPE_UNTAG_ONLY:
            accept_tag = 0;
            accept_untag = 1;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    VLAN_SEM_LOCK(unit);
    /* program value to CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_ACCEPTED_FRAME_TYPE_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_ACPTITAGf, &accept_tag)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    if ((ret = reg_array_field_write(unit, ESW_PORT_ACCEPTED_FRAME_TYPE_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_ACPTIUTAGf, &accept_untag)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }   
    VLAN_SEM_UNLOCK(unit);
    
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portAcceptFrameType_set */

/* Function Name:
 *      dal_esw_vlan_portOuterAcceptFrameType_get
 * Description:
 *      Get accept frame type of outer tag on specified port.
 * Input:
 *      unit                - unit id
 *      port                - port id
 * Output:
 *      pAccept_frame_type  - pointer to accept frame type
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Accept frame type is as following:
 *      - ACCEPT_FRAME_TYPE_ALL
 *      - ACCEPT_FRAME_TYPE_TAG_ONLY
 *      - ACCEPT_FRAME_TYPE_UNTAG_ONLY
 */
int32
dal_esw_vlan_portOuterAcceptFrameType_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_vlan_acceptFrameType_t  *pAccept_frame_type)
{
    uint32  accept_tag, accept_untag, type;
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pAccept_frame_type), RT_ERR_NULL_POINTER);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_ACCEPTED_FRAME_TYPE_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_ACPTOTAGf, &accept_tag)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    if ((ret = reg_array_field_read(unit, ESW_PORT_ACCEPTED_FRAME_TYPE_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_ACPTOUTAGf, &accept_untag)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }    
    VLAN_SEM_UNLOCK(unit);
    
    type = ((accept_tag << 1) | accept_untag);
    
    /* translate chip's value to definition */
    switch (type) 
    {
        case 1:
            *pAccept_frame_type = ACCEPT_FRAME_TYPE_UNTAG_ONLY;
            break;
        case 2:
            *pAccept_frame_type = ACCEPT_FRAME_TYPE_TAG_ONLY;
            break;
        case 3:
            *pAccept_frame_type = ACCEPT_FRAME_TYPE_ALL;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pAccept_frame_type=%x", *pAccept_frame_type);  
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portOuterAcceptFrameType_get */

/* Function Name:
 *      dal_esw_vlan_portOuterAcceptFrameType_set
 * Description:
 *      Set accept frame type of outer tag on specified port.
 * Input:
 *      unit                - unit id
 *      port                - port id
 *      accept_frame_type   - accept frame type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT                 - The module is not initial
 *      RT_ERR_UNIT_ID                  - invalid unit id
 *      RT_ERR_PORT_ID                  - invalid port id
 *      RT_ERR_VLAN_ACCEPT_FRAME_TYPE   - invalid accept frame type
 * Note:
 *  Accept frame type is as following:
 *      - ACCEPT_FRAME_TYPE_ALL
 *      - ACCEPT_FRAME_TYPE_TAG_ONLY
 *      - ACCEPT_FRAME_TYPE_UNTAG_ONLY
 */
int32
dal_esw_vlan_portOuterAcceptFrameType_set(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_vlan_acceptFrameType_t  accept_frame_type)
{
    int32   ret;
    uint32  accept_tag, accept_untag;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, accept_frame_type=%d", 
           unit, port, accept_frame_type);
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((accept_frame_type > ACCEPT_FRAME_TYPE_UNTAG_ONLY), RT_ERR_INPUT);
    
    /* translate chip's value to definition */
    switch (accept_frame_type) 
    {
        case ACCEPT_FRAME_TYPE_ALL:
            accept_tag = 1;
            accept_untag = 1;
            break;
        case ACCEPT_FRAME_TYPE_TAG_ONLY:
            accept_tag = 1;
            accept_untag = 0;
            break;
        case ACCEPT_FRAME_TYPE_UNTAG_ONLY:
            accept_tag = 0;
            accept_untag = 1;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    VLAN_SEM_LOCK(unit);
    /* program value to CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_ACCEPTED_FRAME_TYPE_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_ACPTOTAGf, &accept_tag)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    if ((ret = reg_array_field_write(unit, ESW_PORT_ACCEPTED_FRAME_TYPE_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_ACPTOUTAGf, &accept_untag)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }   
    VLAN_SEM_UNLOCK(unit);
    
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portOuterAcceptFrameType_set */

/* Module Name    : Vlan                       */
/* Sub-module Name: Vlan ingress/egress filter */

/* Function Name:
 *      dal_esw_vlan_portIgrFilterEnable_get
 * Description:
 *      Get vlan ingress filter status of the port from the specified device.
 * Input:
 *      unit        - unit id
 *      port        - port id
 * Output:
 *      pIgr_filter - pointer buffer of ingress filter status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. The status of vlan function is as following:
 *      - DISABLED
 *      - ENABLED
 */
int32
dal_esw_vlan_portIgrFilterEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pIgr_filter)
{
    uint32  value;
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pIgr_filter), RT_ERR_NULL_POINTER);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_VLAN_INGRESS_FILTER_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_IGFILTER_ENf, &value)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
       
    /* translate chip's value to definition */
    switch (value) 
    {
        case 0:
            *pIgr_filter = DISABLED;
            break;
        case 1:
            *pIgr_filter = ENABLED;            
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pIgr_filter=%x", *pIgr_filter);  
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portIgrFilterEnable_get */


/* Function Name:
 *      dal_esw_vlan_portIgrFilterEnable_set
 * Description:
 *      Set vlan ingress filter status of the port to the specified device.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      igr_filter - ingress filter configure
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
 *      1. If the chip is per system configuration, set any port will apply to
 *      whole device. like RTL8329/RTL8389 chip.
 *      2. The status of vlan ingress filter is as following:
 *      - DISABLED
 *      - ENABLED
 */
int32
dal_esw_vlan_portIgrFilterEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t igr_filter)
{
    uint32  value;
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, igr_filter=%d", unit, port, igr_filter); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((igr_filter >= RTK_ENABLE_END), RT_ERR_INPUT);

    /* translate chip's value to definition */
    switch (igr_filter) 
    {
        case DISABLED:
            value = 0;
            break;
        case ENABLED:
            value = 1;            
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_VLAN_INGRESS_FILTER_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_IGFILTER_ENf, &value)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
           
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portIgrFilterEnable_set */

/* Function Name:
 *      dal_esw_vlan_portEgrFilterEnable_get
 * Description:
 *      Get enable status of egress filtering on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of egress filtering
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_vlan_portEgrFilterEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_VLAN_EGRESS_FILTER_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_EGFILTERf, &value)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
       
    /* translate chip's value to definition */
    switch (value) 
    {
        case 0:
            *pEnable = DISABLED;
            break;
        case 1:
            *pEnable = ENABLED;            
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pEnable=%x", *pEnable);  
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portEgrFilterEnable_get */

/* Function Name:
 *      dal_esw_vlan_portEgrFilterEnable_set
 * Description:
 *      Set enable status of egress filtering on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of egress filtering
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_vlan_portEgrFilterEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, enable=%d", unit, port, enable); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);

    /* translate chip's value to definition */
    switch (enable) 
    {
        case DISABLED:
            value = 0;
            break;
        case ENABLED:
            value = 1;            
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_VLAN_EGRESS_FILTER_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_EGFILTERf, &value)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
           
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portEgrFilterEnable_set */

/* Module Name    : Vlan                               */
/* Sub-module Name: Port based and protocol based vlan */

/* Function Name:
 *      dal_esw_vlan_portPvid_get
 * Description:
 *      Get port default vlan id from the specified device.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pPvid - pointer buffer of port default vlan id
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
dal_esw_vlan_portPvid_get(uint32 unit, rtk_port_t port, uint32 *pPvid)
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
    if ((ret = reg_array_field_read(unit, ESW_PORT_BASED_VLAN_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_PIVIDf, pPvid)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pPvid=%d", *pPvid);     
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portPvid_get */

/* Function Name:
 *      dal_esw_vlan_portPvid_set
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
dal_esw_vlan_portPvid_set(uint32 unit, rtk_port_t port, uint32 pvid)
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
    if ((ret = reg_array_field_write(unit, ESW_PORT_BASED_VLAN_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_PIVIDf, &pvid)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    VLAN_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portPvid_set */

/* Function Name:
 *      dal_esw_vlan_portOuterPvid_get
 * Description:
 *      Get outer port based vlan on specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pPvid - pointer to outer port based vlan id
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_vlan_portOuterPvid_get(uint32 unit, rtk_port_t port, rtk_vlan_t *pPvid)
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
    if ((ret = reg_array_field_read(unit, ESW_PORT_BASED_VLAN_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_POVIDf, pPvid)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pPvid=%d", *pPvid);     
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portOuterPvid_get */

/* Function Name:
 *      dal_esw_vlan_portOuterPvid_set
 * Description:
 *      Set outer port based vlan on specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 *      vid  - outer port based vlan id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_VLAN_VID         - invalid vlan id
 * Note:
 *      None
 */
int32
dal_esw_vlan_portOuterPvid_set(uint32 unit, rtk_port_t port, rtk_vlan_t pvid)
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
    if ((ret = reg_array_field_write(unit, ESW_PORT_BASED_VLAN_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_POVIDf, &pvid)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    VLAN_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portOuterPvid_set */

/* Function Name:
 *      dal_esw_vlan_protoGroup_get
 * Description:
 *      Get protocol group for protocol based vlan.
 * Input:
 *      unit            - unit id
 *      protoGroup_idx  - protocol group index
 * Output:
 *      pProtoGroup     - pointer to protocol group
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - protocol group index is out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_vlan_protoGroup_get(
    uint32                  unit,
    uint32                  protoGroup_idx,
    rtk_vlan_protoGroup_t   *pProtoGroup)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, protoGroup_idx=%d", unit, protoGroup_idx); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK((protoGroup_idx > 7), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pProtoGroup), RT_ERR_NULL_POINTER);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PROTOCOL_GROUP_DATABASE_CONTROLr, protoGroup_idx, REG_ARRAY_INDEX_NONE, ESW_FRAMETYPEf, &value)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    if ((ret = reg_array_field_read(unit, ESW_PROTOCOL_GROUP_DATABASE_CONTROLr, protoGroup_idx, REG_ARRAY_INDEX_NONE, ESW_FRAMEVALUEf, &pProtoGroup->framevalue)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }    
    VLAN_SEM_UNLOCK(unit);
       
    /* translate chip's value to definition */
    switch (value) 
    {
        case 0:
            pProtoGroup->frametype = FRAME_TYPE_ETHERNET;
            break;
        case 1:
            pProtoGroup->frametype = FRAME_TYPE_LLCOTHER;
            break;
        case 2:
            pProtoGroup->frametype = FRAME_TYPE_RFC1042;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "frametype=%d, framevalue=%x", pProtoGroup->frametype, pProtoGroup->framevalue);  
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_protoGroup_get */

/* Function Name:
 *      dal_esw_vlan_protoGroup_set
 * Description:
 *      Set protocol group for protocol based vlan.
 * Input:
 *      unit            - unit id
 *      protoGroup_idx  - protocol group index
 *      protoGroup      - protocol group
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_VLAN_FRAME_TYPE  - invalid frame type
 *      RT_ERR_OUT_OF_RANGE     - protocol group index is out of range
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Frame type is as following:
 *      - FRAME_TYPE_ETHERNET
 *      - FRAME_TYPE_RFC1042 (SNAP)
 *      - FRAME_TYPE_LLCOTHER
 */
int32
dal_esw_vlan_protoGroup_set(
    uint32                  unit,
    uint32                  protoGroup_idx,
    rtk_vlan_protoGroup_t   *pProtoGroup)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, protoGroup_idx=%d, frametype=%d, framevalue=%x", unit, protoGroup_idx,
           pProtoGroup->frametype, pProtoGroup->framevalue);   
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK((protoGroup_idx > 7), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pProtoGroup), RT_ERR_NULL_POINTER);
    
    /* translate chip's value to definition */
    switch (pProtoGroup->frametype) 
    {
        case FRAME_TYPE_ETHERNET:
            value = 0;
            break;
        case FRAME_TYPE_LLCOTHER:
            value = 1;
            break;
        case FRAME_TYPE_RFC1042:
            value = 2;
            break;
        default:
            return RT_ERR_FAILED;
    }

    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PROTOCOL_GROUP_DATABASE_CONTROLr, protoGroup_idx, REG_ARRAY_INDEX_NONE, ESW_FRAMETYPEf, &value)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    if ((ret = reg_array_field_write(unit, ESW_PROTOCOL_GROUP_DATABASE_CONTROLr, protoGroup_idx, REG_ARRAY_INDEX_NONE, ESW_FRAMEVALUEf, &pProtoGroup->framevalue)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }    
    VLAN_SEM_UNLOCK(unit);      
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_protoGroup_set */

/* Function Name:
 *      dal_esw_vlan_portProtoVlan_get
 * Description:
 *      Get vlan of specificed protocol group on specified port.
 * Input:
 *      unit            - unit id
 *      port            - port id
 *      protoGroup_idx  - protocol group index
 * Output:
 *      pVlan_cfg       - pointer to vlan configuration of protocol group
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_OUT_OF_RANGE     - protocol group index is out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_vlan_portProtoVlan_get(
    uint32                  unit,
    rtk_port_t              port,
    uint32                  protoGroup_idx,
    rtk_vlan_protoVlanCfg_t *pVlan_cfg)
{
    int32   ret;
    uint32  reg_idx;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, protoGroup_idx=%d", unit, port, protoGroup_idx); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK((protoGroup_idx > 7), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pVlan_cfg), RT_ERR_NULL_POINTER);
    
    reg_idx = ESW_PORT_PORT_AND_PROTOCOL_BASED_VLAN_ENTRY0_0r + 2 * protoGroup_idx;
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_read(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, ESW_PPDFIf, &pVlan_cfg->valid)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }            
    if ((ret = reg_array_field_read(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, ESW_PPIVIDf, &pVlan_cfg->vid)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }   
    if ((ret = reg_array_field_read(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, ESW_PPIPRIf, &pVlan_cfg->pri)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }       
    VLAN_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pVlan_cfg->valid=%d, pVlan_cfg->vid=%d, pVlan_cfg->pri=%d", pVlan_cfg->valid, pVlan_cfg->vid, pVlan_cfg->pri);  
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portProtoVlan_get */

/* Function Name:
 *      dal_esw_vlan_portProtoVlan_set
 * Description:
 *      Set vlan of specificed protocol group on specified port.
 * Input:
 *      unit            - unit id
 *      port            - port id
 *      protoGroup_idx  - protocol group index
 *      pVlan_cfg       - vlan configuration of protocol group
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_OUT_OF_RANGE     - protocol group index is out of range
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_vlan_portProtoVlan_set(
    uint32                  unit,
    rtk_port_t              port,
    uint32                  protoGroup_idx,
    rtk_vlan_protoVlanCfg_t *pVlan_cfg)
{
    int32   ret;
    uint32  reg_idx;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, protoGroup_idx=%d, pVlan_cfg->vid=%d, pVlan_cfg->pri=%d"
    , unit, port, protoGroup_idx, pVlan_cfg->vid, pVlan_cfg->pri);     
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK((protoGroup_idx > HAL_PROTOCOL_VLAN_IDX_MAX(unit)), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pVlan_cfg), RT_ERR_NULL_POINTER);
    
    reg_idx = ESW_PORT_PORT_AND_PROTOCOL_BASED_VLAN_ENTRY0_0r + 2 * protoGroup_idx;
    
    VLAN_SEM_LOCK(unit);
    /* program entry to CHIP*/
    if ((ret = reg_array_field_write(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, ESW_PPDFIf, &pVlan_cfg->valid)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }            
    if ((ret = reg_array_field_write(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, ESW_PPIVIDf, &pVlan_cfg->vid)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }   
    if ((ret = reg_array_field_write(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, ESW_PPIPRIf, &pVlan_cfg->pri)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }    
    VLAN_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_esw_vlan_portProtoVlan_set */

/* Function Name:
 *      dal_esw_vlan_portOuterProtoVlan_get
 * Description:
 *      Get outer vlan of specificed protocol group on specified port.
 * Input:
 *      unit            - unit id
 *      port            - port id
 *      protoGroup_idx  - protocol group index
 * Output:
 *      pVlan_cfg       - pointer to vlan configuration of protocol group
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_OUT_OF_RANGE     - protocol group index is out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_vlan_portOuterProtoVlan_get(
    uint32                  unit,
    rtk_port_t              port,
    uint32                  protoGroup_idx,
    rtk_vlan_protoVlanCfg_t *pVlan_cfg)
{
    int32   ret;
    uint32  reg_idx;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, protoGroup_idx=%d", unit, port, protoGroup_idx); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK((protoGroup_idx > 7), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pVlan_cfg), RT_ERR_NULL_POINTER);
    
    reg_idx = ESW_PORT_PORT_AND_PROTOCOL_BASED_VLAN_ENTRY0_1r + (2 * protoGroup_idx);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_read(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, ESW_PPDFOf, &pVlan_cfg->valid)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }          
    if ((ret = reg_array_field_read(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, ESW_PPOVIDf, &pVlan_cfg->vid)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }   
    if ((ret = reg_array_field_read(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, ESW_PPOPRIf, &pVlan_cfg->pri)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    if ((ret = reg_array_field_read(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, ESW_PPODEIf, &pVlan_cfg->dei)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }            
    VLAN_SEM_UNLOCK(unit);
       
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pVlan_cfg->valid=%d, pVlan_cfg->vid=%d, pVlan_cfg->pri=%d, pVlan_cfg->dei=%d", 
          pVlan_cfg->valid, pVlan_cfg->vid, pVlan_cfg->pri, pVlan_cfg->dei);  
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portOuterProtoVlan_get */

/* Function Name:
 *      dal_esw_vlan_portOuterProtoVlan_set
 * Description:
 *      Set outer vlan of specificed protocol group on specified port.
 * Input:
 *      unit            - unit id
 *      port            - port id
 *      protoGroup_idx  - protocol group index
 *      pVlan_cfg       - vlan configuration of protocol group
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_OUT_OF_RANGE     - protocol group index is out of range
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_vlan_portOuterProtoVlan_set(
    uint32                  unit,
    rtk_port_t              port,
    uint32                  protoGroup_idx,
    rtk_vlan_protoVlanCfg_t *pVlan_cfg)
{
    int32   ret;
    uint32  reg_idx;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, protoGroup_idx=%d, pVlan_cfg->vid=%d, pVlan_cfg->pri=%d, pVlan_cfg->dei=%d"
    , unit, port, protoGroup_idx, pVlan_cfg->vid, pVlan_cfg->pri, pVlan_cfg->dei);     
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK((protoGroup_idx > 7), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pVlan_cfg), RT_ERR_NULL_POINTER);
    
    reg_idx = ESW_PORT_PORT_AND_PROTOCOL_BASED_VLAN_ENTRY0_1r + (2 * protoGroup_idx);
    
    VLAN_SEM_LOCK(unit);
    /* program entry to CHIP*/
    if ((ret = reg_array_field_write(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, ESW_PPDFOf, &pVlan_cfg->valid)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }          
    if ((ret = reg_array_field_write(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, ESW_PPOVIDf, &pVlan_cfg->vid)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }   
    if ((ret = reg_array_field_write(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, ESW_PPOPRIf, &pVlan_cfg->pri)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    if ((ret = reg_array_field_write(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, ESW_PPODEIf, &pVlan_cfg->dei)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }            
    VLAN_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_esw_vlan_portOuterProtoVlan_set */

/* Module Name    : Vlan               */
/* Sub-module Name: TPID configuration */

/* Function Name:
 *      dal_esw_vlan_portTpidEntry_get
 * Description:
 *      Get TPID of TPID entry on specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      tpid_idx - index of TPID entry
 * Output:
 *      pTpid    - pointer to TPID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_vlan_portTpidEntry_get(uint32 unit, rtk_port_t port, uint32 tpid_idx, uint32 *pTpid)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, tpid_idx=%d", unit, port, tpid_idx); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    /* ED: RT_PARAM_CHK(tpid_idx > 0x3, RT_ERR_OUT_OF_RANGE); */
    RT_PARAM_CHK((NULL == pTpid), RT_ERR_NULL_POINTER);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_PACKET_PARSER_TPID_CONTROLr, port, tpid_idx, ESW_TPIDf, pTpid)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
          
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pTpid_idx=%x", *pTpid);  
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portTpidEntry_get */

/* Function Name:
 *      dal_esw_vlan_portTpidEntry_set
 * Description:
 *      Set tpid of TPID entry on specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      tpid_idx - index of TPID entry
 *      tpid     - TPID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 * Note:
 *
 */
int32
dal_esw_vlan_portTpidEntry_set(uint32 unit, rtk_port_t port, uint32 tpid_idx, uint32 tpid)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, tpid_idx=%d, tpid=%d", unit, port, tpid_idx, tpid); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(tpid_idx > 3, RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((tpid > RTK_ETHERTYPE_MAX), RT_ERR_OUT_OF_RANGE);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_PACKET_PARSER_TPID_CONTROLr, port, tpid_idx, ESW_TPIDf, &tpid)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portTpidEntry_set */

/* Function Name:
 *      dal_esw_vlan_portEgrInnerTpidMode_get
 * Description:
 *      Get TPID mode of inner tag at egress.
 *      TPID mode will decide what kind of packet should be replaced inner TPID with configured one.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pTpid_mode - pointer to inner TPID mode at egress
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Inner TPID mode is as following:
 *      - EGR_TPID_MODE_IGR_UNTAG_AND_EGR_TAG
 *      - EGR_TPID_MODE_ALL_PACKETS
 */
int32
dal_esw_vlan_portEgrInnerTpidMode_get(uint32 unit, rtk_port_t port, rtk_vlan_egrTpidMode_t *pTpid_mode)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pTpid_mode), RT_ERR_NULL_POINTER);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_INNER_TAG_TPID_INDEX_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_TPID_SELf, &value)) != RT_ERR_OK)        
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);

    switch(value)
    {
        case 0:
            *pTpid_mode = EGR_TPID_MODE_IGR_UNTAG_AND_EGR_TAG;
            break;
        case 1:
            *pTpid_mode = EGR_TPID_MODE_ALL_PACKETS;
            break;
        default:
            return RT_ERR_FAILED;
    }      
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pTpid_mode=%d", *pTpid_mode);  
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portEgrInnerTpidMode_get */

/* Function Name:
 *      dal_esw_vlan_portEgrInnerTpidMode_set
 * Description:
 *      Set TPID mode of inner tag at egress.
 *      TPID mode will decide what kind of packet should be replaced inner TPID with configured one.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      tpid_mode - inner TPID mode at egress
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Inner TPID mode is as following:
 *      - EGR_TPID_MODE_IGR_UNTAG_AND_EGR_TAG
 *      - EGR_TPID_MODE_ALL_PACKETS
 */
int32
dal_esw_vlan_portEgrInnerTpidMode_set(uint32 unit, rtk_port_t port, rtk_vlan_egrTpidMode_t tpid_mode)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, tpid_mode=%d", unit, port, tpid_mode); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(tpid_mode >= EGR_TPID_MODE_END, RT_ERR_INPUT);

    switch(tpid_mode)
    {
        case EGR_TPID_MODE_IGR_UNTAG_AND_EGR_TAG:
            value = 0;
            break;
        case EGR_TPID_MODE_ALL_PACKETS:
            value = 1;
            break;
        default:
            return RT_ERR_FAILED;
    }          
       
    VLAN_SEM_LOCK(unit);
    /* program value to CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_INNER_TAG_TPID_INDEX_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_TPID_SELf, &value)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
           
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portEgrInnerTpidMode_set */

/* Function Name:
 *      dal_esw_vlan_portIgrInnerTpid_get
 * Description:
 *      Get TPIDs of inner tag at ingress.
 * Input:
 *      unit             - unit id
 *      port             - port id
 * Output:
 *      pTpid_idx_mask - pointer to mask for index of tpid entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_vlan_portIgrInnerTpid_get(uint32 unit, rtk_port_t port, uint32 *pTpid_idx_mask)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pTpid_idx_mask), RT_ERR_NULL_POINTER);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_PACKET_PARSER_VLAN_CONTROL0r, port, REG_ARRAY_INDEX_NONE, ESW_PITPIDMASKf, pTpid_idx_mask)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
          
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pTpid_idx_mask=%x", *pTpid_idx_mask);  
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portIgrInnerTpid_get */

/* Function Name:
 *      dal_esw_vlan_portIgrInnerTpid_set
 * Description:
 *      Set TPIDs of inner tag at ingress.
 * Input:
 *      unit            - unit id
 *      port            - port id
 *      tpid_idx_mask - mask for index of tpid entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_vlan_portIgrInnerTpid_set(uint32 unit, rtk_port_t port, uint32 tpid_idx_mask)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, tpid_entry_mask=%d", unit, port, tpid_idx_mask); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(tpid_idx_mask > HAL_TPID_ENTRY_MASK_MAX(unit), RT_ERR_OUT_OF_RANGE);
       
    VLAN_SEM_LOCK(unit);
    /* program value to CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_PACKET_PARSER_VLAN_CONTROL0r, port, REG_ARRAY_INDEX_NONE, ESW_PITPIDMASKf, &tpid_idx_mask)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
           
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portIgrInnerTpid_set */

/* Function Name:
 *      dal_esw_vlan_portEgrInnerTpid_get
 * Description:
 *      Get TPID of inner tag at egress.
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pTpid_idx - pointer to index of inner TPID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_vlan_portEgrInnerTpid_get(uint32 unit, rtk_port_t port, uint32 *pTpid_idx)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pTpid_idx), RT_ERR_NULL_POINTER);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_INNER_TAG_TPID_INDEX_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_TPID_IDXf, pTpid_idx)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
          
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pTpid_idx=%x", *pTpid_idx);  
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portEgrInnerTpid_get */

/* Function Name:
 *      dal_esw_vlan_portEgrInnerTpid_set
 * Description:
 *      Set TPID of inner tag at egress.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      tpid_idx - index of inner TPID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 * Note:
 *      None
 */
int32
dal_esw_vlan_portEgrInnerTpid_set(uint32 unit, rtk_port_t port, uint32 tpid_idx)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, tpid_idx=%d", unit, port, tpid_idx); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(tpid_idx > HAL_TPID_ENTRY_IDX_MAX(unit), RT_ERR_OUT_OF_RANGE);
       
    VLAN_SEM_LOCK(unit);
    /* program value to CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_INNER_TAG_TPID_INDEX_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_TPID_IDXf, &tpid_idx)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
           
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portEgrInnerTpid_set */

/* Function Name:
 *      dal_esw_vlan_portEgrOuterTpidMode_get
 * Description:
 *      Get TPID mode of outer tag at egress.
 *      TPID mode will decide what kind of packet should be replaced outer TPID with configured one.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pTpid_mode - pointer to outer TPID mode at egress
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Inner TPID mode is as following:
 *      - EGR_TPID_MODE_IGR_UNTAG_AND_EGR_TAG
 *      - EGR_TPID_MODE_ALL_PACKETS
 */
int32
dal_esw_vlan_portEgrOuterTpidMode_get(uint32 unit, rtk_port_t port, rtk_vlan_egrTpidMode_t *pTpid_mode)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pTpid_mode), RT_ERR_NULL_POINTER);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_OUTER_TAG_SPID_INDEX_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_SPID_SELf, &value)) != RT_ERR_OK)        
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);

    switch(value)
    {
        case 0:
            *pTpid_mode = EGR_TPID_MODE_IGR_UNTAG_AND_EGR_TAG;
            break;
        case 1:
            *pTpid_mode = EGR_TPID_MODE_ALL_PACKETS;
            break;
        default:
            return RT_ERR_FAILED;
    }      
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pTpid_mode=%d", *pTpid_mode);  
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portEgrOuterTpidMode_get */

/* Function Name:
 *      dal_esw_vlan_portEgrOuterTpidMode_set
 * Description:
 *      Set TPID mode of outer tag at egress.
 *      TPID mode will decide what kind of packet should be replaced outer TPID with configured one.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      tpid_mode - outer TPID mode at egress
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Inner TPID mode is as following:
 *      - EGR_TPID_MODE_IGR_UNTAG_AND_EGR_TAG
 *      - EGR_TPID_MODE_ALL_PACKETS
 */
int32
dal_esw_vlan_portEgrOuterTpidMode_set(uint32 unit, rtk_port_t port, rtk_vlan_egrTpidMode_t tpid_mode)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, tpid_mode=%d", unit, port, tpid_mode); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(tpid_mode >= EGR_TPID_MODE_END, RT_ERR_INPUT);

    switch(tpid_mode)
    {
        case EGR_TPID_MODE_IGR_UNTAG_AND_EGR_TAG:
            value = 0;
            break;
        case EGR_TPID_MODE_ALL_PACKETS:
            value = 1;
            break;
        default:
            return RT_ERR_FAILED;
    }          
       
    VLAN_SEM_LOCK(unit);
    /* program value to CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_OUTER_TAG_SPID_INDEX_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_SPID_SELf, &value)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
           
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portEgrOuterTpidMode_set */

/* Function Name:
 *      dal_esw_vlan_portIgrOuterTpid_get
 * Description:
 *      Get TPIDs of outer tag at ingress.
 * Input:
 *      unit            - unit id
 *      port            - port id
 * Output:
 *      pTpid_idx_mask  - pointer to mask for index of tpid entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_vlan_portIgrOuterTpid_get(uint32 unit, rtk_port_t port, uint32 *pTpid_idx_mask)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pTpid_idx_mask), RT_ERR_NULL_POINTER);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_PACKET_PARSER_VLAN_CONTROL0r, port, REG_ARRAY_INDEX_NONE, ESW_POTPIDMASKf, pTpid_idx_mask)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
          
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pTpid_idx_mask=%x", *pTpid_idx_mask);  
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portIgrOuterTpid_get */

/* Function Name:
 *      dal_esw_vlan_portIgrOuterTpid_set
 * Description:
 *      Set TPIDs of outer tag at ingress.
 * Input:
 *      unit            - unit id
 *      port            - port id
 *      tpid_idx_mask   - mask for index of tpid entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_vlan_portIgrOuterTpid_set(uint32 unit, rtk_port_t port, uint32 tpid_idx_mask)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, tpid_idx_mask=%d", unit, port, tpid_idx_mask); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(tpid_idx_mask > HAL_TPID_ENTRY_MASK_MAX(unit), RT_ERR_OUT_OF_RANGE);
       
    VLAN_SEM_LOCK(unit);
    /* program value to CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_PACKET_PARSER_VLAN_CONTROL0r, port, REG_ARRAY_INDEX_NONE, ESW_POTPIDMASKf, &tpid_idx_mask)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
           
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portIgrOuterTpid_set */

/* Function Name:
 *      dal_esw_vlan_portEgrOuterTpid_get
 * Description:
 *      Get TPID of outer tag at egress.
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pTpid_idx - pointer to index of outer TPID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_vlan_portEgrOuterTpid_get(uint32 unit, rtk_port_t port, uint32 *pTpid_idx)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pTpid_idx), RT_ERR_NULL_POINTER);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_OUTER_TAG_SPID_INDEX_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_SPID_IDXf, pTpid_idx)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
          
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pTpid_idx=%x", *pTpid_idx);  
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portEgrOuterTpid_get */

/* Function Name:
 *      dal_esw_vlan_portEgrOuterTpid_set
 * Description:
 *      Set TPID of outer tag at egress.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      tpid_idx - index of outer TPID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 * Note:
 *      None
 */
int32
dal_esw_vlan_portEgrOuterTpid_set(uint32 unit, rtk_port_t port, uint32 tpid_idx)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, tpid_idx=%d", unit, port, tpid_idx); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(tpid_idx > HAL_TPID_ENTRY_IDX_MAX(unit), RT_ERR_OUT_OF_RANGE);
       
    VLAN_SEM_LOCK(unit);
    /* program value to CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_OUTER_TAG_SPID_INDEX_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_SPID_IDXf, &tpid_idx)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
           
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portEgrOuterTpid_set */

/* Function Name:
 *      dal_esw_vlan_portIgrExtraTpid_get
 * Description:
 *      Get TPIDs of extra tag at ingress.
 * Input:
 *      unit            - unit id
 *      port            - port id
 * Output:
 *      pTpid_idx_mask  - pointer to mask for index of tpid entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_vlan_portIgrExtraTpid_get(uint32 unit, rtk_port_t port, uint32 *pTpid_idx_mask)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pTpid_idx_mask), RT_ERR_NULL_POINTER);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_PACKET_PARSER_VLAN_CONTROL0r, port, REG_ARRAY_INDEX_NONE, ESW_PETPIDMASKf, pTpid_idx_mask)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
          
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pTpid_idx_mask=%x", *pTpid_idx_mask);  
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portIgrExtraTpid_get */

/* Function Name:
 *      dal_esw_vlan_portIgrExtraTpid_set
 * Description:
 *      Set TPIDs of extra tag at ingress.
 * Input:
 *      unit            - unit id
 *      port            - port id
 *      tpid_idx_mask   - mask for index of tpid entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_vlan_portIgrExtraTpid_set(uint32 unit, rtk_port_t port, uint32 tpid_idx_mask)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, tpid_idx_mask=%d", unit, port, tpid_idx_mask); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(tpid_idx_mask > HAL_TPID_ENTRY_MASK_MAX(unit), RT_ERR_OUT_OF_RANGE);
       
    VLAN_SEM_LOCK(unit);
    /* program value to CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_PACKET_PARSER_VLAN_CONTROL0r, port, REG_ARRAY_INDEX_NONE, ESW_PETPIDMASKf, &tpid_idx_mask)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
           
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portIgrExtraTpid_set */

/* Module Name    : Vlan                 */
/* Sub-module Name: Ingress tag handling */

/* Function Name:
 *      dal_esw_vlan_portIgrIgnoreInnerTagEnable_get
 * Description:
 *      Get enable status of ignore inner tag at ingress.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      enable - enable status of ignore inner tag
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_vlan_portIgrIgnoreInnerTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_IGNORE_VLAN_TAG_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_IGNOREITAGVIDf, &value)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
       
    /* translate chip's value to definition */
    switch (value) 
    {
        case 0:
            *pEnable = DISABLED;
            break;
        case 1:
            *pEnable = ENABLED;            
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pEnable=%x", *pEnable);  
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portIgrIgnoreInnerTagEnable_get */

/* Function Name:
 *      dal_esw_vlan_portIgrIgnoreInnerTagEnable_set
 * Description:
 *      Set enable status of ignore inner tag at ingress.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of ignore inner tag
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_vlan_portIgrIgnoreInnerTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, enable=%d", unit, port, enable); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);

    /* translate chip's value to definition */
    switch (enable) 
    {
        case DISABLED:
            value = 0;
            break;
        case ENABLED:
            value = 1;            
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_IGNORE_VLAN_TAG_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_IGNOREITAGVIDf, &value)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
           
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portIgrIgnoreInnerTagEnable_set */

/* Function Name:
 *      dal_esw_vlan_portIgrIgnoreOuterTagEnable_get
 * Description:
 *      Get enable status of ignore outer tag at ingress.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of ignore outer tag
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_vlan_portIgrIgnoreOuterTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_IGNORE_VLAN_TAG_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_IGNOREOTAGVIDf, &value)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
       
    /* translate chip's value to definition */
    switch (value) 
    {
        case 0:
            *pEnable = DISABLED;
            break;
        case 1:
            *pEnable = ENABLED;            
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pEnable=%x", *pEnable);  
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portIgrIgnoreOuterTagEnable_get */

/* Function Name:
 *      dal_esw_vlan_portIgrIgnoreOuterTagEnable_set
 * Description:
 *      Set enable status of ignore outer tag at ingress.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of ignore outer tag
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_vlan_portIgrIgnoreOuterTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, enable=%d", unit, port, enable); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);

    /* translate chip's value to definition */
    switch (enable) 
    {
        case DISABLED:
            value = 0;
            break;
        case ENABLED:
            value = 1;            
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_IGNORE_VLAN_TAG_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_IGNOREOTAGVIDf, &value)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
           
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portIgrIgnoreOuterTagEnable_set */

/* Module Name    : Vlan                */
/* Sub-module Name: Egress tag handling */

/* Function Name:
 *      dal_esw_vlan_portEgrInnerTagEnable_get
 * Description:
 *      Get enable status of inner tag capability at egress.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of inner tag capability
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Egress packet will send out with inner tag when following is true.
 *      - inner tag capability is enable
 *      - this port is tag member of specified vlan
 */
int32
dal_esw_vlan_portEgrInnerTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_EGRESS_TAG_STATUS_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_EGRESS_TAG_STATUSf, &value)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
       
    /* translate chip's value to definition */     
    switch (value) 
    {
        case EGR_BOTH_TAG:
            *pEnable = ENABLED;
            break;
        case EGR_INNER_TAG_ONLY:
            *pEnable = ENABLED;
            break;
        case EGR_OUTER_TAG_ONLY:
            *pEnable = DISABLED;            
            break;
        case EGR_UNTAG:
            *pEnable = DISABLED;            
            break;            
        default:
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pEnable=%x", *pEnable);  
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portEgrInnerTagEnable_get */

/* Function Name:
 *      dal_esw_vlan_portEgrInnerTagEnable_set
 * Description:
 *      Set enable status of inner tag capability at egress.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of inner tag capability
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Egress packet will send out with inner tag when following is true.
 *      - inner tag capability is enable
 *      - this port is tag member of specified vlan
 */
int32
dal_esw_vlan_portEgrInnerTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, enable=%d", unit, port, enable); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);
       
    VLAN_SEM_LOCK(unit);
    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_EGRESS_TAG_STATUS_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_EGRESS_TAG_STATUSf, &value)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    /* translate chip's value to definition */
    if (enable == ENABLED)
    {
        if (value == EGR_UNTAG)
            value = EGR_INNER_TAG_ONLY;
        else if (value == EGR_OUTER_TAG_ONLY)
            value = EGR_BOTH_TAG;
    }
    else
    {
        if (value == EGR_INNER_TAG_ONLY)
            value = EGR_UNTAG;
        else if (value == EGR_BOTH_TAG)
            value = EGR_OUTER_TAG_ONLY;        
    }            

    /* program value to CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_EGRESS_TAG_STATUS_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_EGRESS_TAG_STATUSf, &value)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
           
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portEgrInnerTagEnable_set */

/* Function Name:
 *      dal_esw_vlan_portEgrOuterTagEnable_get
 * Description:
 *      Get enable status of outer tag capability at egress.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of outer tag capability
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Egress packet will send out with outer tag when following is true.
 *      - outer tag capability is enable
 *      - this port is tag member of specified vlan
 */
int32
dal_esw_vlan_portEgrOuterTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_EGRESS_TAG_STATUS_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_EGRESS_TAG_STATUSf, &value)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
       
    /* translate chip's value to definition */
    switch (value) 
    {
        case EGR_BOTH_TAG:
            *pEnable = ENABLED;
            break;
        case EGR_INNER_TAG_ONLY:
            *pEnable = DISABLED;
            break;
        case EGR_OUTER_TAG_ONLY:
            *pEnable = ENABLED;            
            break;
        case EGR_UNTAG:
            *pEnable = DISABLED;            
            break;            
        default:               
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pEnable=%x", *pEnable);  
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portEgrOuterTagEnable_get */

/* Function Name:
 *      dal_esw_vlan_portEgrOuterTagEnable_set
 * Description:
 *      Set enable status of outer tag capability at egress.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of outer tag capability
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Egress packet will send out with outer tag when following is true.
 *      - outer tag capability is enable
 *      - this port is tag member of specified vlan
 */
int32
dal_esw_vlan_portEgrOuterTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, enable=%d", unit, port, enable); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);
       
    VLAN_SEM_LOCK(unit);
    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_EGRESS_TAG_STATUS_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_EGRESS_TAG_STATUSf, &value)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    /* translate chip's value to definition */
    if (enable == ENABLED)
    {
        if (value == EGR_UNTAG)
            value = EGR_OUTER_TAG_ONLY;
        else if (value == EGR_INNER_TAG_ONLY)
            value = EGR_BOTH_TAG;
    }
    else
    {
        if (value == EGR_OUTER_TAG_ONLY)
            value = EGR_UNTAG;
        else if (value == EGR_BOTH_TAG)
            value = EGR_INNER_TAG_ONLY;        
    }            

    /* program value to CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_EGRESS_TAG_STATUS_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_EGRESS_TAG_STATUSf, &value)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
           
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portEgrOuterTagEnable_set */

/* Function Name:
 *      dal_esw_vlan_portIgrExtraTagEnable_get
 * Description:
 *      Get enable status of recognizing extra tag at ingress.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of recognizing extra tag
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_vlan_portIgrExtraTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_INSERT_EXTRA_VLAN_TAG_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_RX_ISEXTRATAGf, &value)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
       
    /* translate chip's value to definition */
    switch (value) 
    {
        case 0:
            *pEnable = DISABLED;
            break;
        case 1:
            *pEnable = ENABLED;            
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pEnable=%x", *pEnable);  
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portIgrExtraTagEnable_get */

/* Function Name:
 *      dal_esw_vlan_portIgrExtraTagEnable_set
 * Description:
 *      Set enable status of recognizing extra tag at ingress.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of recognizing extra tag
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_vlan_portIgrExtraTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, enable=%d", unit, port, enable); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);

    /* translate chip's value to definition */
    switch (enable) 
    {
        case DISABLED:
            value = 0;
            break;
        case ENABLED:
            value = 1;            
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_INSERT_EXTRA_VLAN_TAG_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_RX_ISEXTRATAGf, &value)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
           
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portIgrExtraTagEnable_set */

/* Function Name:
 *      dal_esw_vlan_portEgrExtraTagEnable_get
 * Description:
 *      Get enable status of recognizing extra tag at egress.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of recognizing extra tag
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_vlan_portEgrExtraTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_INSERT_EXTRA_VLAN_TAG_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_TX_ISEXTRATAGf, &value)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
       
    /* translate chip's value to definition */
    switch (value) 
    {
        case 0:
            *pEnable = DISABLED;
            break;
        case 1:
            *pEnable = ENABLED;            
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pEnable=%x", *pEnable);  
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portEgrExtraTagEnable_get */

/* Function Name:
 *      dal_esw_vlan_portEgrExtraTagEnable_set
 * Description:
 *      Set enable status of recognizing extra tag at egress.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of recognizing extra tag
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_vlan_portEgrExtraTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, enable=%d", unit, port, enable); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);

    /* translate chip's value to definition */
    switch (enable) 
    {
        case DISABLED:
            value = 0;
            break;
        case ENABLED:
            value = 1;            
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_INSERT_EXTRA_VLAN_TAG_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_TX_ISEXTRATAGf, &value)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    VLAN_SEM_UNLOCK(unit);
           
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portEgrExtraTagEnable_set */

/* Function Name:
 *      dal_esw_vlan_portEgrInnerVidSource_get
 * Description:
 *      Get source of vid field in inner tag.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pVidSource - pointer to source of vid field in inner tag
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Vid source is as following
 *      - TAG_SOURCE_FROM_ALE
 *      - TAG_SOURCE_FROM_ORIG_INNER_TAG
 *      - TAG_SOURCE_FROM_ORIG_OUTER_TAG
 */
int32
dal_esw_vlan_portEgrInnerVidSource_get(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t *pVidSource)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pVidSource), RT_ERR_NULL_POINTER);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/    
    if ((ret = reg_array_field_read(unit, ESW_PORT_VLAN_TAG_SOURCE_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_IVID_SRCf, &value)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }    
    VLAN_SEM_UNLOCK(unit);
        
    /* translate chip's value to definition */
    switch (value) 
    {
        case 0:
            *pVidSource = TAG_SOURCE_FROM_ALE;
            break;
        case 1:
            *pVidSource = TAG_SOURCE_FROM_ORIG_INNER_TAG;
            break;
        case 2:
            *pVidSource = TAG_SOURCE_FROM_ORIG_OUTER_TAG;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pVidSource=%x", *pVidSource);  
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portEgrInnerVidSource_get */

/* Function Name:
 *      dal_esw_vlan_portEgrInnerVidSource_set
 * Description:
 *      Set source of vid field in inner tag.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      vidSource - source of vid field in inner tag
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Vid source is as following
 *      - TAG_SOURCE_FROM_ALE
 *      - TAG_SOURCE_FROM_ORIG_INNER_TAG
 *      - TAG_SOURCE_FROM_ORIG_OUTER_TAG
 */
int32
dal_esw_vlan_portEgrInnerVidSource_set(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t vidSource)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, vidSource=%d", unit, port, vidSource); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((vidSource >= TAG_SOURCE_END || vidSource == TAG_SOURCE_NULL), RT_ERR_OUT_OF_RANGE);

    /* vidSource chip's value to definition */
    switch (vidSource) 
    {
        case TAG_SOURCE_FROM_ALE:
            value = 0;
            break;
        case TAG_SOURCE_FROM_ORIG_INNER_TAG:
            value = 1;
            break;
        case TAG_SOURCE_FROM_ORIG_OUTER_TAG:
            value = 2;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/    
    if ((ret = reg_array_field_write(unit, ESW_PORT_VLAN_TAG_SOURCE_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_IVID_SRCf, &value)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }    
    VLAN_SEM_UNLOCK(unit);
      
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portEgrInnerVidSource_set */

/* Function Name:
 *      dal_esw_vlan_portEgrInnerPriSource_get
 * Description:
 *      Get source of priority field in inner tag.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pPriSource - pointer to source of priority field in inner tag
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Vid source is as following
 *      - TAG_SOURCE_FROM_ORIG_INNER_TAG
 *      - TAG_SOURCE_FROM_ORIG_OUTER_TAG
 *      - TAG_SOURCE_NULL
 */
int32
dal_esw_vlan_portEgrInnerPriSource_get(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t *pPriSource)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pPriSource), RT_ERR_NULL_POINTER);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/    
    if ((ret = reg_array_field_read(unit, ESW_PORT_VLAN_TAG_SOURCE_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_IPRI_SRCf, &value)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }    
    VLAN_SEM_UNLOCK(unit);
        
    /* translate chip's value to definition */
    switch (value) 
    {
        case 0:
            *pPriSource = TAG_SOURCE_NULL;
            break;
        case 1:
            *pPriSource = TAG_SOURCE_FROM_ORIG_INNER_TAG;
            break;
        case 2:
            *pPriSource = TAG_SOURCE_FROM_ORIG_OUTER_TAG;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pPriSource=%x", *pPriSource);  
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portEgrInnerPriSource_get */

/* Function Name:
 *      dal_esw_vlan_portEgrInnerPriSource_set
 * Description:
 *      Set source of priority field in inner tag.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      priSource - source of priority field in inner tag
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Vid source is as following
 *      - TAG_SOURCE_FROM_ORIG_INNER_TAG
 *      - TAG_SOURCE_FROM_ORIG_OUTER_TAG
 *      - TAG_SOURCE_NULL
 */
int32
dal_esw_vlan_portEgrInnerPriSource_set(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t priSource)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, priSource=%d", unit, port, priSource); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((priSource >= TAG_SOURCE_END || priSource == TAG_SOURCE_FROM_ALE), RT_ERR_OUT_OF_RANGE);

    /* translate chip's value to definition */
    switch (priSource) 
    {
        case TAG_SOURCE_NULL:
            value = 0;
            break;
        case TAG_SOURCE_FROM_ORIG_INNER_TAG:
            value = 1;
            break;
        case TAG_SOURCE_FROM_ORIG_OUTER_TAG:
            value = 2;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/    
    if ((ret = reg_array_field_write(unit, ESW_PORT_VLAN_TAG_SOURCE_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_IPRI_SRCf, &value)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }    
    VLAN_SEM_UNLOCK(unit);
      
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portEgrInnerPriSource_set */

/* Function Name:
 *      dal_esw_vlan_portEgrOuterVidSource_get
 * Description:
 *      Get source of vid field in outer tag.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pVidSource - pointer to source of vid field in outer tag
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Vid source is as following
 *      - TAG_SOURCE_FROM_ALE
 *      - TAG_SOURCE_FROM_ORIG_INNER_TAG
 *      - TAG_SOURCE_FROM_ORIG_OUTER_TAG
 */
int32
dal_esw_vlan_portEgrOuterVidSource_get(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t *pVidSource)
{

    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pVidSource), RT_ERR_NULL_POINTER);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/    
    if ((ret = reg_array_field_read(unit, ESW_PORT_VLAN_TAG_SOURCE_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_OVID_SRCf, &value)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }    
    VLAN_SEM_UNLOCK(unit);
        
    /* translate chip's value to definition */
    switch (value) 
    {
        case 0:
            *pVidSource = TAG_SOURCE_FROM_ALE;
            break;
        case 1:
            *pVidSource = TAG_SOURCE_FROM_ORIG_INNER_TAG;
            break;
        case 2:
            *pVidSource = TAG_SOURCE_FROM_ORIG_OUTER_TAG;
            break;
        default:
            RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pVidSource=%x", *pVidSource);  

    return RT_ERR_OK;
} /* end of dal_esw_vlan_portEgrOuterVidSource_get */

/* Function Name:
 *      dal_esw_vlan_portEgrOuterVidSource_set
 * Description:
 *      Set source of vid field in outer tag.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      vidSource - source of vid field in outer tag
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Vid source is as following
 *      - TAG_SOURCE_FROM_ALE
 *      - TAG_SOURCE_FROM_ORIG_INNER_TAG
 *      - TAG_SOURCE_FROM_ORIG_OUTER_TAG
 */
int32
dal_esw_vlan_portEgrOuterVidSource_set(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t vidSource)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, vidSource=%d", unit, port, vidSource); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((vidSource >= TAG_SOURCE_END || vidSource == TAG_SOURCE_NULL), RT_ERR_OUT_OF_RANGE);

    /* vidSource chip's value to definition */
    switch (vidSource) 
    {
        case TAG_SOURCE_FROM_ALE:
            value = 0;
            break;
        case TAG_SOURCE_FROM_ORIG_INNER_TAG:
            value = 1;
            break;
        case TAG_SOURCE_FROM_ORIG_OUTER_TAG:
            value = 2;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/    
    if ((ret = reg_array_field_write(unit, ESW_PORT_VLAN_TAG_SOURCE_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_OVID_SRCf, &value)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }    
    VLAN_SEM_UNLOCK(unit);
      
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portEgrOuterVidSource_set */

/* Function Name:
 *      dal_esw_vlan_portEgrOuterPriSource_get
 * Description:
 *      Get source of priority field in outer tag.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pPriSource - pointer to source of priority field in outer tag
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Vid source is as following
 *      - TAG_SOURCE_FROM_ORIG_INNER_TAG
 *      - TAG_SOURCE_FROM_ORIG_OUTER_TAG
 *      - TAG_SOURCE_NULL
 */
int32
dal_esw_vlan_portEgrOuterPriSource_get(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t *pPriSource)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pPriSource), RT_ERR_NULL_POINTER);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/    
    if ((ret = reg_array_field_read(unit, ESW_PORT_VLAN_TAG_SOURCE_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_OPRI_SRCf, &value)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }    
    VLAN_SEM_UNLOCK(unit);
        
    /* translate chip's value to definition */
    switch (value) 
    {
        case 0:
            *pPriSource = TAG_SOURCE_NULL;
            break;
        case 1:
            *pPriSource = TAG_SOURCE_FROM_ORIG_INNER_TAG;
            break;
        case 2:
            *pPriSource = TAG_SOURCE_FROM_ORIG_OUTER_TAG;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pPriSource=%x", *pPriSource);  
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portEgrOuterPriSource_get */

/* Function Name:
 *      dal_esw_vlan_portEgrOuterPriSource_set
 * Description:
 *      Set source of priority field in outer tag.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      priSource - source of priority field in outer tag
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Vid source is as following
 *      - TAG_SOURCE_FROM_ORIG_INNER_TAG
 *      - TAG_SOURCE_FROM_ORIG_OUTER_TAG
 *      - TAG_SOURCE_NULL
 */
int32
dal_esw_vlan_portEgrOuterPriSource_set(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t priSource)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, priSource=%d", unit, port, priSource); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((priSource >= TAG_SOURCE_END || priSource == TAG_SOURCE_FROM_ALE), RT_ERR_OUT_OF_RANGE);

    /* translate chip's value to definition */
    switch (priSource) 
    {
        case TAG_SOURCE_NULL:
            value = 0;
            break;
        case TAG_SOURCE_FROM_ORIG_INNER_TAG:
            value = 1;
            break;
        case TAG_SOURCE_FROM_ORIG_OUTER_TAG:
            value = 2;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/    
    if ((ret = reg_array_field_write(unit, ESW_PORT_VLAN_TAG_SOURCE_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_OPRI_SRCf, &value)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }    
    VLAN_SEM_UNLOCK(unit);
      
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portEgrOuterPriSource_set */

/* Module Name    : Vlan                */
/* Sub-module Name: Tag format handling */

/* Function Name:
 *      dal_esw_vlan_portIgrTagKeepEnable_get
 * Description:
 *      Get enable status of keep tag format at ingress.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pKeepOuter - enable status of keep outer tag format
 *      pKeepInner - enable status of keep inner tag format
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Packet will be transmitted with original tag format when following condition are true.
 *      - Enable keep tag format at ingress.
 *      - Enable keep tag format at egress
 */
int32
dal_esw_vlan_portIgrTagKeepEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pKeepOuter, rtk_enable_t *pKeepInner)
{
    uint32  value1, value2;
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pKeepOuter) || (NULL == pKeepInner), RT_ERR_NULL_POINTER);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_VLAN_TAG_ORIGINAL_FORMAT_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_RX_OTAGFMT_KEEPf, &value1)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    /* get entry from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_VLAN_TAG_ORIGINAL_FORMAT_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_RX_ITAGFMT_KEEPf, &value2)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }    
    VLAN_SEM_UNLOCK(unit);
       
    /* translate chip's value to definition */
    switch (value1) 
    {
        case 0:
            *pKeepOuter = DISABLED;
            break;
        case 1:
            *pKeepOuter = ENABLED;            
            break;
        default:
            return RT_ERR_FAILED;
    }
    switch (value2) 
    {
        case 0:
            *pKeepInner = DISABLED;
            break;
        case 1:
            *pKeepInner = ENABLED;            
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pKeepOuter=%d, pKeepInner=%d", *pKeepOuter, *pKeepInner);  
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portIgrTagKeepEnable_get */

/* Function Name:
 *      dal_esw_vlan_portIgrTagKeepEnable_set
 * Description:
 *      Set enable status of keep tag format at ingress.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      keepOuter - enable status of keep outer tag format
 *      keepInner - enable status of keep inner tag format
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Packet will be transmitted with original tag format when following condition are true.
 *      - Enable keep tag format at ingress.
 *      - Enable keep tag format at egress
 */
int32
dal_esw_vlan_portIgrTagKeepEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t keepOuter, rtk_enable_t keepInner)
{
    uint32  value1, value2;
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, keepOuter=%d, keepInner=%d", 
          unit, port, keepOuter, keepInner); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((keepOuter >= RTK_ENABLE_END) || (keepInner >= RTK_ENABLE_END), RT_ERR_INPUT);

    /* translate chip's value to definition */
    switch (keepOuter) 
    {
        case DISABLED:
            value1 = 0;
            break;
        case ENABLED:
            value1 = 1;            
            break;
        default:
            return RT_ERR_FAILED;
    }
    switch (keepInner) 
    {
        case DISABLED:
            value2 = 0;
            break;
        case ENABLED:
            value2 = 1;            
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_VLAN_TAG_ORIGINAL_FORMAT_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_RX_OTAGFMT_KEEPf, &value1)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    if ((ret = reg_array_field_write(unit, ESW_PORT_VLAN_TAG_ORIGINAL_FORMAT_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_RX_ITAGFMT_KEEPf, &value2)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }    
    VLAN_SEM_UNLOCK(unit);
           
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portIgrTagKeepEnable_set */

/* Function Name:
 *      dal_esw_vlan_portEgrTagKeepEnable_get
 * Description:
 *      Get enable status of keep tag format at egress.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pKeepOuter - pointer to enable status of keep outer tag format
 *      pKeepInner - pointer to enable status of keep inner tag format
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Packet will be transmitted with original tag format when following condition are true.
 *      - Enable keep tag format at ingress.
 *      - Enable keep tag format at egress
 */
int32
dal_esw_vlan_portEgrTagKeepEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pKeepOuter, rtk_enable_t *pKeepInner)
{
    int32   ret;
    uint32  value1, value2;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pKeepOuter) || (NULL == pKeepInner), RT_ERR_NULL_POINTER);
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_VLAN_TAG_ORIGINAL_FORMAT_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_TX_OTAGFMT_KEEPf, &value1)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    /* get entry from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_VLAN_TAG_ORIGINAL_FORMAT_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_TX_ITAGFMT_KEEPf, &value2)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }    
    VLAN_SEM_UNLOCK(unit);
       
    /* translate chip's value to definition */
    switch (value1) 
    {
        case 0:
            *pKeepOuter = DISABLED;
            break;
        case 1:
            *pKeepOuter = ENABLED;            
            break;
        default:
            return RT_ERR_FAILED;
    }
    switch (value2) 
    {
        case 0:
            *pKeepInner = DISABLED;
            break;
        case 1:
            *pKeepInner = ENABLED;            
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pKeepOuter=%d, pKeepInner=%d", *pKeepOuter, *pKeepInner);  
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portEgrTagKeepEnable_get */

/* Function Name:
 *      dal_esw_vlan_portEgrTagKeepEnable_set
 * Description:
 *      Set enable status of keep tag format at egress.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      keepOuter - enable status of keep outer tag format
 *      keepInner - enable status of keep inner tag format
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Packet will be transmitted with original tag format when following condition are true.
 *      - Enable keep tag format at ingress.
 *      - Enable keep tag format at egress
 */
int32
dal_esw_vlan_portEgrTagKeepEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t keepOuter, rtk_enable_t keepInner)
{
    int32   ret;
    uint32  value1, value2;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, port=%d, keepOuter=%d, keepInner=%d", 
          unit, port, keepOuter, keepInner); 
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
        
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((keepOuter >= RTK_ENABLE_END) || (keepInner >= RTK_ENABLE_END), RT_ERR_INPUT);

    /* translate chip's value to definition */
    switch (keepOuter) 
    {
        case DISABLED:
            value1 = 0;
            break;
        case ENABLED:
            value1 = 1;            
            break;
        default:
            return RT_ERR_FAILED;
    }
    switch (keepInner) 
    {
        case DISABLED:
            value2 = 0;
            break;
        case ENABLED:
            value2 = 1;            
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    VLAN_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_VLAN_TAG_ORIGINAL_FORMAT_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_TX_OTAGFMT_KEEPf, &value1)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    if ((ret = reg_array_field_write(unit, ESW_PORT_VLAN_TAG_ORIGINAL_FORMAT_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_TX_ITAGFMT_KEEPf, &value2)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }    
    VLAN_SEM_UNLOCK(unit);
           
    return RT_ERR_OK;
} /* end of dal_esw_vlan_portEgrTagKeepEnable_set */

/*
 * MISC
 */

/* Function Name:
 *      dal_esw_vlan_fwdMode_get
 * Description:
 *      Get packet forwarding mode on specified vlan.
 *      Packet can be forwarded by ALE decision or forward to members of vlan.
 * Input:
 *      unit  - unit id
 *      vid   - vlan id
 * Output:
 *      pMode - pointer to packet forwarding mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Packet forwarding mode is as following:
 *      - VLAN_FWD_ON_ALE
 *      - VLAN_FWD_ON_VLAN_MEMBER
 */
int32
dal_esw_vlan_fwdMode_get(uint32 unit, rtk_vlan_t vid, rtk_vlan_fwdMode_t *pMode)
{
   int32           ret;
    dal_esw_vlan_data_t vlan_data_entry;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, vid=%d", unit, vid);    
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
    
    /* VID check */
    RT_PARAM_CHK((vid < RTK_VLAN_ID_MIN) || (vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    RT_PARAM_CHK(VLANINFO_VALID_IS_CLEAR(unit, vid), RT_ERR_VLAN_ENTRY_NOT_FOUND);
    RT_PARAM_CHK((NULL == pMode), RT_ERR_NULL_POINTER);
    
    osal_memset(&vlan_data_entry, 0, sizeof(vlan_data_entry));
    
    /* configure vlan data entry */
    vlan_data_entry.vid     = vid;
    
    VLAN_SEM_LOCK(unit);
    
    /* get entry from CHIP*/
    if ((ret = _dal_esw_getVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    VLAN_SEM_UNLOCK(unit);
    
    *pMode = vlan_data_entry.vlan_base_fwd;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pMode=%x", *pMode); 

    return RT_ERR_OK;
} /* end of dal_esw_vlan_fwdMode_get */

/* Function Name:
 *      dal_esw_vlan_fwdMode_set
 * Description:
 *      Set packet forwarding mode on specified vlan.
 *      Packet can be forwarded by ALE decision or forward to members of vlan.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 *      mode - packet forwarding mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Packet forwarding mode is as following:
 *      - VLAN_FWD_ON_ALE
 *      - VLAN_FWD_ON_VLAN_MEMBER
 *
 */
int32
dal_esw_vlan_fwdMode_set(uint32 unit, rtk_vlan_t vid, rtk_vlan_fwdMode_t mode)
{
    int32 ret;
    dal_esw_vlan_data_t vlan_data_entry;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "unit=%d, vid=%d, mode=%d", unit, vid, mode);
    
    /* check Init status */
    RT_INIT_CHK(vlan_init[unit]);
    
    /* VID check */
    RT_PARAM_CHK((vid < RTK_VLAN_ID_MIN) || (vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    RT_PARAM_CHK((mode >= VLAN_FWD_END), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(VLANINFO_VALID_IS_CLEAR(unit, vid), RT_ERR_VLAN_ENTRY_NOT_FOUND);
    
    osal_memset(&vlan_data_entry, 0, sizeof(vlan_data_entry));
    
    /* configure vlan data entry */
    vlan_data_entry.vid     = vid;
    
    VLAN_SEM_LOCK(unit);
    
    /* get entry from CHIP*/
    if ((ret = _dal_esw_getVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    /* set vlan_base_fwd to new mode */
    vlan_data_entry.vlan_base_fwd = mode;
    
    /* programming to chip */
    if ((ret = _dal_esw_setVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    VLAN_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_fwdMode_set */

/* Internal Function Body */

/* Function Name:
 *      _dal_esw_vlan_init_config
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
_dal_esw_vlan_init_config(uint32 unit) 
{
    int32       ret;
    dal_esw_vlan_data_t vlan_data_entry;
    rtk_port_t  port, max_port;
    
    /* configure vlan data entry */
    vlan_data_entry.vid     = RTK_DEFAULT_VLAN_ID;
    vlan_data_entry.fid     = RTK_DEFAULT_MSTI;
    vlan_data_entry.vlan_base_fwd = RTK_DEFAULT_VLAN_FWD_MODE;  
    HAL_GET_ALL_PORTMASK(unit, vlan_data_entry.member_portmask);
    HAL_GET_ALL_PORTMASK(unit, vlan_data_entry.untag_portmask);
    
    if ((ret = _dal_esw_setVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "create default vlan entry failed");
        return ret;
    }
    
     /* Set vlan valid bit in software */
    VLANINFO_VALID_SET(unit, RTK_DEFAULT_VLAN_ID);
    pDal_esw_vlan_info[unit]->count++;
    
    max_port = HAL_GET_MAX_PORT(unit);
    for (port = 0; port < max_port; port++)
    {
        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }
        
        if ((ret = dal_esw_vlan_portPvid_set(unit, port, RTK_DEFAULT_PORT_VID)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_VLAN|MOD_DAL), "set default port pvid failed");
            return ret; 
        }
        
        if ((ret = dal_esw_vlan_portEgrInnerTagEnable_set(unit, port, ENABLED)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_VLAN|MOD_DAL), "set default port status of egress inner tag failed");
            return ret; 
        } 
    }
    
    for (port = 0; port <= max_port; port++)
    {
        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }
        
        if ((ret = dal_esw_vlan_portIgrIgnoreInnerTagEnable_set(unit, port, RTK_DEFAULT_VLAN_IGNORE_INNER_TAG)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_VLAN|MOD_DAL), "set default port ignore inner tag of ingress failed");
            return ret; 
        }

        if ((ret = dal_esw_vlan_portIgrIgnoreOuterTagEnable_set(unit, port, RTK_DEFAULT_VLAN_IGNORE_OUTER_TAG)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_VLAN|MOD_DAL), "set default port ignore outer tag of ingress failed");
            return ret; 
        }
        
        if ((ret = dal_esw_vlan_portEgrFilterEnable_set(unit, port, RTK_DEFAULT_VLAN_EGRESS_FILTER_ENABLE)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_VLAN|MOD_DAL), "set default port vlan filter status of egress failed");
            return ret; 
        }
    }

    return RT_ERR_OK;
} /* end of _dal_esw_vlan_init_config */

/* Function Name:
 *      _dal_esw_setVlan
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
static int32 _dal_esw_setVlan(uint32 unit, dal_esw_vlan_data_t *pVlan_entry)
{
    int32   ret;
    vlan_entry_t    vlan_entry;
    uint32  temp_var;
    
    RT_DBG(LOG_TRACE, (MOD_VLAN|MOD_DAL), "unit=%d, vid=%d, fid=%d, vlan_base_fwd=%d, member_portmask=%x, untag_portmask=%x", 
           unit, pVlan_entry->vid, pVlan_entry->fid, pVlan_entry->vlan_base_fwd, pVlan_entry->member_portmask, pVlan_entry->untag_portmask);
    
    osal_memset(&vlan_entry, 0, sizeof(vlan_entry));
    
    /* set member set */
    if ((ret = table_field_set(unit, ESW_VLANt, ESW_VLAN_MBRf, pVlan_entry->member_portmask.bits, (uint32 *) &vlan_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    /* set untagged member set */
    if ((ret = table_field_set(unit, ESW_VLANt, ESW_VLAN_UTAGf, pVlan_entry->untag_portmask.bits, (uint32 *) &vlan_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    /* set fid */
    temp_var = pVlan_entry->fid;
    if ((ret = table_field_set(unit, ESW_VLANt, ESW_VLAN_FIDf, &temp_var, (uint32 *) &vlan_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }    
    
    /* set vlan_base_fwd */
    temp_var = pVlan_entry->vlan_base_fwd;
    if ((ret = table_field_set(unit, ESW_VLANt, ESW_VLAN_VLAN_BASE_FWDf, &temp_var, (uint32 *) &vlan_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    /* programming vlan entry in chip */
    if ((ret = table_write(unit, ESW_VLANt, pVlan_entry->vid, (uint32 *) &vlan_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    return RT_ERR_OK;
} /* end of _dal_esw_setVlan */

/* Function Name:
 *      _dal_esw_getVlan
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
static int32 _dal_esw_getVlan(uint32 unit, dal_esw_vlan_data_t *pVlan_entry)
{
    int32   ret;
    vlan_entry_t    vlan_entry;
    uint32  temp_var;
    
    RT_DBG(LOG_TRACE, (MOD_VLAN|MOD_DAL), "unit=%d, vid=%d", unit, pVlan_entry->vid);
    
    osal_memset(&vlan_entry, 0, sizeof(vlan_entry));
 
    /* get entry from chip */
    if ((ret = table_read(unit, ESW_VLANt, pVlan_entry->vid, (uint32 *) &vlan_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }

    /* get member set from vlan_entry */
    if ((ret = table_field_get(unit, ESW_VLANt, ESW_VLAN_MBRf, pVlan_entry->member_portmask.bits, (uint32 *) &vlan_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }

    /* get untagged member set */
    if ((ret = table_field_get(unit, ESW_VLANt, ESW_VLAN_UTAGf, pVlan_entry->untag_portmask.bits, (uint32 *) &vlan_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }

    /* get fid */
    if ((ret = table_field_get(unit, ESW_VLANt, ESW_VLAN_FIDf, &temp_var, (uint32 *) &vlan_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }    
    pVlan_entry->fid = temp_var;

    /* get vlan_base_fwd */
    if ((ret = table_field_get(unit, ESW_VLANt, ESW_VLAN_VLAN_BASE_FWDf, &temp_var, (uint32 *) &vlan_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    pVlan_entry->vlan_base_fwd = temp_var;

    RT_DBG(LOG_TRACE, (MOD_VLAN|MOD_DAL), "member_portmask=%x, untag_portmask=%x\
           fid=%d, vlan_base_fwd=%d", pVlan_entry->member_portmask.bits[0], pVlan_entry->untag_portmask.bits[0], 
           pVlan_entry->fid, pVlan_entry->vlan_base_fwd);
    
    return RT_ERR_OK;
} /* end of _dal_esw_getVlan */

/* Function Name:
 *      dal_esw_vlan_stg_get
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
dal_esw_vlan_stg_get(uint32 unit, rtk_vlan_t vid, rtk_stg_t *pStg)
{
    int32           ret;
    dal_esw_vlan_data_t vlan_data_entry;
    
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
    if ((ret = _dal_esw_getVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    VLAN_SEM_UNLOCK(unit);
    
    /* set fid to new fid */
    *pStg = vlan_data_entry.fid;
    
    RT_DBG(LOG_DEBUG, (MOD_VLAN|MOD_DAL), "pStg=%x", *pStg); 
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_stg_get */


/* Function Name:
 *      dal_esw_vlan_stg_set
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
dal_esw_vlan_stg_set(uint32 unit, rtk_vlan_t vid, rtk_stg_t stg)
{
    int32           ret;
    dal_esw_vlan_data_t vlan_data_entry;
    
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
    if ((ret = _dal_esw_getVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    /* set fid to new fid */
    vlan_data_entry.fid = stg;
    
    /* programming to chip */
    if ((ret = _dal_esw_setVlan(unit, &vlan_data_entry)) != RT_ERR_OK)
    {
        VLAN_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_VLAN|MOD_DAL), "");
        return ret;
    }
    
    VLAN_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_vlan_stg_set */
