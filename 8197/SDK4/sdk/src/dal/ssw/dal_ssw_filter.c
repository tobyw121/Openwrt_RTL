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
 * Purpose : Definition those public filter APIs and its data type in the SDK.
 * 
 * Feature : The file have include the following module and sub-modules
 *           1) Flow table
 *           2) Ingress ACL
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
#include <dal/ssw/dal_ssw_filter.h>
#include <rtk/default.h>
#include <rtk/filter.h>

/*
 * Symbol Definition
 */
typedef struct rtk_filter_info_s
{
    rtk_filter_owner_t owner[RTK_MAX_NUM_OF_FILTER_ENTRY];
} rtk_filter_info_t;

/*
 * Data Declaration
 */
static rtk_filter_info_t    *pFilter_tbl_mapping[RTK_MAX_NUM_OF_UNIT];
 
static uint32               filter_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         filter_sem[RTK_MAX_NUM_OF_UNIT];

const static uint16 acl_rate_limit_leaky_bucket_control_regidx[] = 
{  SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL0r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL0r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL1r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL1r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL2r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL2r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL3r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL3r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL4r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL4r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL5r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL5r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL6r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL6r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL7r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL7r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL8r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL8r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL9r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL9r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL10r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL10r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL11r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL11r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL12r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL12r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL13r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL13r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL14r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL14r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL15r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL15r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL16r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL16r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL17r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL17r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL18r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL18r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL19r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL19r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL20r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL20r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL21r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL21r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL22r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL22r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL23r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL23r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL24r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL24r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL25r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL25r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL26r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL26r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL27r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL27r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL28r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL28r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL29r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL29r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL30r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL30r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL31r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL31r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL32r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL32r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL33r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL33r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL34r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL34r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL35r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL35r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL36r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL36r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL37r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL37r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL38r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL38r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL39r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL39r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL40r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL40r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL41r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL41r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL42r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL42r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL43r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL43r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL44r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL44r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL45r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL45r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL46r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL46r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL47r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL47r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL48r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL48r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL49r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL49r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL50r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL50r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL51r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL51r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL52r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL52r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL53r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL53r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL54r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL54r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL55r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL55r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL56r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL56r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL57r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL57r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL58r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL58r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL59r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL59r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL60r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL60r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL61r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL61r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL62r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL62r\
 , SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL63r, SSW_ACL_RATE_LIMIT_LEAKY_BUCKET_CONTROL63r};


const static uint16 aclRateLimit_fieldIdx[] = 
{  SSW_ACLRL0_RATEf, SSW_ACLRL1_RATEf, SSW_ACLRL2_RATEf, SSW_ACLRL3_RATEf, SSW_ACLRL4_RATEf\
 , SSW_ACLRL5_RATEf, SSW_ACLRL6_RATEf, SSW_ACLRL7_RATEf, SSW_ACLRL8_RATEf, SSW_ACLRL9_RATEf\
 , SSW_ACLRL10_RATEf, SSW_ACLRL11_RATEf, SSW_ACLRL12_RATEf, SSW_ACLRL13_RATEf, SSW_ACLRL14_RATEf\
 , SSW_ACLRL15_RATEf, SSW_ACLRL16_RATEf, SSW_ACLRL17_RATEf, SSW_ACLRL18_RATEf, SSW_ACLRL19_RATEf\
 , SSW_ACLRL20_RATEf, SSW_ACLRL21_RATEf, SSW_ACLRL22_RATEf, SSW_ACLRL23_RATEf, SSW_ACLRL24_RATEf\
 , SSW_ACLRL25_RATEf, SSW_ACLRL26_RATEf, SSW_ACLRL27_RATEf, SSW_ACLRL28_RATEf, SSW_ACLRL29_RATEf\
 , SSW_ACLRL30_RATEf, SSW_ACLRL31_RATEf, SSW_ACLRL32_RATEf, SSW_ACLRL33_RATEf, SSW_ACLRL34_RATEf\
 , SSW_ACLRL35_RATEf, SSW_ACLRL36_RATEf, SSW_ACLRL37_RATEf, SSW_ACLRL38_RATEf, SSW_ACLRL39_RATEf\
 , SSW_ACLRL40_RATEf, SSW_ACLRL41_RATEf, SSW_ACLRL42_RATEf, SSW_ACLRL43_RATEf, SSW_ACLRL44_RATEf\
 , SSW_ACLRL45_RATEf, SSW_ACLRL46_RATEf, SSW_ACLRL47_RATEf, SSW_ACLRL48_RATEf, SSW_ACLRL49_RATEf\
 , SSW_ACLRL50_RATEf, SSW_ACLRL51_RATEf, SSW_ACLRL52_RATEf, SSW_ACLRL53_RATEf, SSW_ACLRL54_RATEf\
 , SSW_ACLRL55_RATEf, SSW_ACLRL56_RATEf, SSW_ACLRL57_RATEf, SSW_ACLRL58_RATEf, SSW_ACLRL59_RATEf\
 , SSW_ACLRL60_RATEf, SSW_ACLRL61_RATEf, SSW_ACLRL62_RATEf, SSW_ACLRL63_RATEf, SSW_ACLRL64_RATEf\
 , SSW_ACLRL65_RATEf, SSW_ACLRL66_RATEf, SSW_ACLRL67_RATEf, SSW_ACLRL68_RATEf, SSW_ACLRL69_RATEf\
 , SSW_ACLRL70_RATEf, SSW_ACLRL71_RATEf, SSW_ACLRL72_RATEf, SSW_ACLRL73_RATEf, SSW_ACLRL74_RATEf\
 , SSW_ACLRL75_RATEf, SSW_ACLRL76_RATEf, SSW_ACLRL77_RATEf, SSW_ACLRL78_RATEf, SSW_ACLRL79_RATEf\
 , SSW_ACLRL80_RATEf, SSW_ACLRL81_RATEf, SSW_ACLRL82_RATEf, SSW_ACLRL83_RATEf, SSW_ACLRL84_RATEf\
 , SSW_ACLRL85_RATEf, SSW_ACLRL86_RATEf, SSW_ACLRL87_RATEf, SSW_ACLRL88_RATEf, SSW_ACLRL89_RATEf\
 , SSW_ACLRL90_RATEf, SSW_ACLRL91_RATEf, SSW_ACLRL92_RATEf, SSW_ACLRL93_RATEf, SSW_ACLRL94_RATEf\
 , SSW_ACLRL95_RATEf, SSW_ACLRL96_RATEf, SSW_ACLRL97_RATEf, SSW_ACLRL98_RATEf, SSW_ACLRL99_RATEf\
 , SSW_ACLRL100_RATEf, SSW_ACLRL101_RATEf, SSW_ACLRL102_RATEf, SSW_ACLRL103_RATEf, SSW_ACLRL104_RATEf\
 , SSW_ACLRL105_RATEf, SSW_ACLRL106_RATEf, SSW_ACLRL107_RATEf, SSW_ACLRL108_RATEf, SSW_ACLRL109_RATEf\
 , SSW_ACLRL110_RATEf, SSW_ACLRL111_RATEf, SSW_ACLRL112_RATEf, SSW_ACLRL113_RATEf, SSW_ACLRL114_RATEf\
 , SSW_ACLRL115_RATEf, SSW_ACLRL116_RATEf, SSW_ACLRL117_RATEf, SSW_ACLRL118_RATEf, SSW_ACLRL119_RATEf\
 , SSW_ACLRL120_RATEf, SSW_ACLRL121_RATEf, SSW_ACLRL122_RATEf, SSW_ACLRL123_RATEf, SSW_ACLRL124_RATEf\
 , SSW_ACLRL125_RATEf, SSW_ACLRL126_RATEf, SSW_ACLRL127_RATEf};


/*
 * Macro Declaration
 */
#define FILTERINFO_OWNER_CHK(unit, fid, type)     (pFilter_tbl_mapping[unit]->owner[fid] == type)
#define FILTERINFO_OWNER_AND(unit, fid, type)     (pFilter_tbl_mapping[unit]->owner[fid] & type)
#define FILTERINFO_OWNER_SET(unit, fid, type)     (pFilter_tbl_mapping[unit]->owner[fid] = type)
 
/* semaphore handling */
#define FILTER_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(filter_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_DAL|MOD_FILTER), "semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)
#define FILTER_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(filter_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_DAL|MOD_FILTER), "semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)

/*
 * Function Declaration
 */
static int32 _dal_ssw_filter_init_config(uint32 unit);
static int32 _dal_ssw_flowTblEntry_set(uint32 unit, rtk_filter_id_t filter_id, rtk_filter_flowTbl_t *pFilter_cfg, rtk_filter_action_t *pAction);
static int32 _dal_ssw_flowTblEntry_get(uint32 unit, rtk_filter_id_t filter_id, rtk_filter_flowTbl_t *pFilter_cfg, rtk_filter_action_t *pAction);
static int32 _dal_ssw_filterEntry_valid_set(uint32 unit, rtk_filter_id_t filter_id, uint32 valid);
static int32 _dal_ssw_aclEntry_set(uint32 unit, rtk_filter_id_t filter_id, rtk_filter_aclCfg_t *pFilter_cfg, rtk_filter_action_t *pAction);
static int32 _dal_ssw_aclEntry_get(uint32 unit, rtk_filter_id_t filter_id, rtk_filter_aclCfg_t *pFilter_cfg, rtk_filter_action_t *pAction);
static int32 _dal_ssw_logEntry_set(uint32 unit, rtk_log_id_t log_id, uint32 pkt_cnt, uint64 byte_cnt);
static int32 _dal_ssw_logEntry_get(uint32 unit, rtk_log_id_t log_id, uint32 *pPkt_cnt, uint64 *pByte_cnt);
static int32 _dal_ssw_filterEntry_del(uint32 unit, rtk_filter_id_t filter_id, rtk_filter_owner_t owner_type);
static int32 _dal_ssw_filterEntry_init(uint32 unit);

/* Module Name     : filter */
/* Sub-module Name : global */

/* Function Name:
 *      dal_ssw_filter_blkCutline_get
 * Description:
 *      Get the cutline value from the specified device.
 * Input:
 *      unit     - unit id
 * Output:
 *      pCutline - pointer buffer of cutline value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. RTL8329/RTL8389: default cutline value is 4, mean block 0-3 is used by flow table
 *         and 4-7 is used by ingress acl table.
 *      2. cutline value is configurable and 0~(cutline-1) is used by flow table
 *         and cutline~maximum block is used by ingress acl table.
 */
int32
dal_ssw_filter_blkCutline_get(uint32 unit, uint32 *pCutline)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d", unit); 
    
    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((NULL == pCutline), RT_ERR_NULL_POINTER);
    
    FILTER_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, SSW_FLOW_TABLE_CONTROLr, SSW_CUTLINEf, pCutline)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    FILTER_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "pCutline=%d", *pCutline);
    
    return RT_ERR_OK;
} /* end of dal_ssw_filter_blkCutline_get */

/* Function Name:
 *      dal_ssw_filter_blkCutline_set
 * Description:
 *      Set the cutline value to the specified device.
 * Input:
 *      unit    - unit id
 *      cutline - cutline value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_FILTER_CUTLINE - invalid cutline value
 * Note:
 *      1. RTL8329/RTL8389: default cutline value is 4, mean block 0-3 is used by flow table
 *         and 4-7 is used by ingress acl table.
 *      2. cutline value is configurable and 0~(cutline-1) is used by flow table
 *         and cutline~maximum block is used by ingress acl table.
 */
int32
dal_ssw_filter_blkCutline_set(uint32 unit, uint32 cutline)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, cutline=%d", unit, cutline);
    
    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(cutline > HAL_MAX_NUM_OF_PIE_BLOCK(unit), RT_ERR_FILTER_CUTLINE);
    
    FILTER_SEM_LOCK(unit);
    
    /* set value to CHIP*/
    if ((ret = reg_field_write(unit, SSW_FLOW_TABLE_CONTROLr, SSW_CUTLINEf, &cutline)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    FILTER_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_filter_blkCutline_set */

/* Function Name:
 *      dal_ssw_filter_pieEnable_get
 * Description:
 *      Get the PIE enable status from the specified device.
 * Input:
 *      unit     - unit id
 * Output:
 *      pEnable - pointer of enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None.
 */
int32
dal_ssw_filter_pieEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d", unit);
    
    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    FILTER_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, SSW_FLOW_TABLE_CONTROLr, SSW_EN_PIE89f, &value)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    FILTER_SEM_UNLOCK(unit);
    
    if (1 == value)
    {
        *pEnable = ENABLED;
    }
    else
    {
        *pEnable = DISABLED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "pEnable=%d", *pEnable);
    
    return RT_ERR_OK;
} /* end of dal_ssw_filter_pieEnable_get*/

/* Function Name:
 *      dal_ssw_filter_pieEnable_set
 * Description:
 *      Set the PIE enable status to the specified device.
 * Input:
 *      unit   - unit id
 *      enable - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 * Note:
 *      None.
 */
int32
dal_ssw_filter_pieEnable_set(uint32 unit, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, enable=%d", unit, enable);
    
    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((enable != DISABLED && enable != ENABLED), RT_ERR_INPUT);
    
    
    if (ENABLED == enable)
    {
        value = 1;
    }
    else
    {
        value = 0;
    }
    
    FILTER_SEM_LOCK(unit);
    
    /* set value to CHIP*/
    if ((ret = reg_field_write(unit, SSW_FLOW_TABLE_CONTROLr, SSW_EN_PIE89f, &value)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    FILTER_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_filter_pieEnable_set */

/* Function Name:
 *      dal_ssw_filter_init
 * Description:
 *      Initialize filter module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      Must initialize filter module before calling any filter APIs.
 */
int32
dal_ssw_filter_init(uint32 unit)
{
    int32   ret;
    
    filter_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    filter_sem[unit] = osal_sem_mutex_create();
    if (0 == filter_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_FILTER), "semaphore create failed");
        return RT_ERR_FAILED;
    }
    
    FILTER_SEM_LOCK(unit);
    
    /*init database*/
    /* allocate memory for each database and initilize database */
    pFilter_tbl_mapping[unit] = (rtk_filter_info_t *)osal_alloc(sizeof(rtk_filter_info_t));
    if (0 == pFilter_tbl_mapping[unit])
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(RT_ERR_FAILED, (MOD_FILTER|MOD_DAL), "memory allocate failed");
        return RT_ERR_FAILED;
    }
    osal_memset(pFilter_tbl_mapping[unit], 0, sizeof(rtk_filter_info_t));
    
    FILTER_SEM_UNLOCK(unit);
    
    filter_init[unit] = INIT_COMPLETED;
    
    if (( ret = _dal_ssw_filter_init_config(unit)) != RT_ERR_OK)
    {
        filter_init[unit] = INIT_NOT_COMPLETED;
        osal_free(pFilter_tbl_mapping[unit]);
        pFilter_tbl_mapping[unit] = 0;
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "Filter default config initialize failed");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_ssw_filter_init */

/* Module Name     : filter        */
/* Sub-module Name : pattern match */

/* Function Name:
 *      dal_ssw_filter_patternMatch_get
 * Description:
 *      Get per port pattern match from the specified device.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pMode    - pointer buffer of pattern match mode
 *      pPattern - pointer buffer of pattern to be matched.
 *      pMask    - pointer buffer of pattrn character care mask.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The pattern match mode as following:
 *          - PM_4BYTE_MODE       (two 4-bytes mode)
 *          - PM_8BYTE_MODE       (one 8-bytes mode)
 */
int32
dal_ssw_filter_patternMatch_get(
    uint32                          unit,
    rtk_port_t                      port,
    rtk_filter_patternMatch_mode_t  *pMode,
    uint8                           *pPattern,
    uint32                          *pMask)
{
    int32   ret;
//    uint32  reg_idx;
    uint32  mode;
    uint32  maskBuf;
    uint32  data;
    uint32  value;    
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, port=%d", unit, port);    
    
    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pMode), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pPattern), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pMask), RT_ERR_NULL_POINTER);    
    
    FILTER_SEM_LOCK(unit);

    /*Get mode*/
    //reg_idx = PATTERN_MATCH_PORT0_CONTROL + port;
    if ((ret = reg_array_field_read(unit, SSW_PATTERN_MATCH_PORT_CONTROLr, port, 0, SSW_PM_MODEf, &mode)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    if (0 == mode)
    {
        *pMode = PM_4BYTE_MODE;
    }
    else
    {
        *pMode = PM_8BYTE_MODE;
    } 

    /*Get set0 data*/
    //reg_idx = PATTERN_MATCH_PORT0_SET_0_4_BYTE_DATA + port;
    /* get value from CHIP*/
    if ((ret = reg_array_read(unit, SSW_PATTERN_MATCH_PORT_SET_0_4_BYTE_DATAr, port, 0, &value)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
   
    if ((ret = reg_field_get(unit, SSW_PATTERN_MATCH_PORT_SET_0_4_BYTE_DATAr, SSW_BYTE3f, &data, &value)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pPattern[0] = data;
    if ((ret = reg_field_get(unit, SSW_PATTERN_MATCH_PORT_SET_0_4_BYTE_DATAr, SSW_BYTE2f, &data, &value)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pPattern[1] = data;
    if ((ret = reg_field_get(unit, SSW_PATTERN_MATCH_PORT_SET_0_4_BYTE_DATAr, SSW_BYTE1f, &data, &value)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pPattern[2] = data;
    if ((ret = reg_field_get(unit, SSW_PATTERN_MATCH_PORT_SET_0_4_BYTE_DATAr, SSW_BYTE0f, &data, &value)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pPattern[3] = data;
    
    /*Get set0 mask*/
    //reg_idx = PATTERN_MATCH_PORT0_CONTROL + port;
    if ((ret = reg_array_field_read(unit, SSW_PATTERN_MATCH_PORT_CONTROLr, port, 0, SSW_MASK_SET_0f, &maskBuf)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    *pMask = maskBuf << 4;
    
    /*Get set1 data*/
    //reg_idx = PATTERN_MATCH_PORT0_SET_1_4_BYTE_DATA + port;
    /* get value from CHIP*/
    if ((ret = reg_array_read(unit, SSW_PATTERN_MATCH_PORT_SET_1_4_BYTE_DATAr, port, 0, &value)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        return ret;
    }
    if ((ret = reg_field_get(unit, SSW_PATTERN_MATCH_PORT_SET_1_4_BYTE_DATAr, SSW_BYTE3f, &data, &value)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        return ret;
    }
    pPattern[4] = data;
    if ((ret = reg_field_get(unit, SSW_PATTERN_MATCH_PORT_SET_1_4_BYTE_DATAr, SSW_BYTE2f, &data, &value)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pPattern[5] = data;
    if ((ret = reg_field_get(unit, SSW_PATTERN_MATCH_PORT_SET_1_4_BYTE_DATAr, SSW_BYTE1f, &data, &value)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pPattern[6] = data;
    if ((ret = reg_field_get(unit, SSW_PATTERN_MATCH_PORT_SET_1_4_BYTE_DATAr, SSW_BYTE0f, &data, &value)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pPattern[7] = data;
    
    /*Get set1 mask*/
    //reg_idx = PATTERN_MATCH_PORT0_CONTROL + port;
    if ((ret = reg_array_field_read(unit, SSW_PATTERN_MATCH_PORT_CONTROLr, port, 0, SSW_MASK_SET_1f, &maskBuf)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    FILTER_SEM_UNLOCK(unit);
    
    *pMask = *pMask | maskBuf;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "pMode=%d, pPattern=%x, pMask=%x", 
           *pMode, *pPattern, *pMask);

    return RT_ERR_OK;
} /* end of dal_ssw_filter_patternMatch_get */


/* Function Name:
 *      dal_ssw_filter_patternMatch_set
 * Description:
 *      Set per port pattern match to the specified device.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      mode     - pattern match mode
 *      pPattern - pointer buffer of pattern to be matched.
 *      mask     - pattrn character care mask.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_PM_LENGTH    - invalid pattern length (the pattern string is not 8 bytes)
 *      RT_ERR_PM_MASK      - invalid pattern match mask (the mask value is more than 8 bits)
 *      RT_ERR_PM_MODE      - invalid pattern match mode
 * Note:
 *      1. The pattern match mode as following:
 *          - PM_4BYTE_MODE       (two 4-bytes mode)
 *          - PM_8BYTE_MODE       (one 8-bytes mode)
 *
 *      2. The API can set ether in 4-Byte mode or 8-Byte mode.
 *         In 4-Byte mode, pattern of set0 should place in pattern[0~3] and
 *         that of set1 should place in pattern[4~7].
 *         In 4-Byte mode, the first byte of set0 and set1 must care even
 *         if you don't speciy set1.
 *         This is due to ASIC limitation.
 *         If you really want not set1 to be compared, just set the care-bit 
 *         of "PatternMatch" field in ACL to 2 ('10' in binary).
 *
 *         In 8-Byte mode, the first byte must care.
 *         
 *         Here are some 4-Byte mode examples:
 *             Example 1. To compare "abcd" and "efgh"
 *                 input parameter:
 *                 mode        = 0
 *                 pattern[]   = "abcdefgh"
 *                 mask        = 0xff
 *             Example 2. To compare "abcd" and "efg"
 *                 input parameter:
 *                 mode        = 0
 *                 pattern[]   = "abcdefg " <---   ### Leave 1 blank
 *                 mask        = 0xfe
 *             Example 3. To compare "abc" and "efgh"
 *                 input parameter:
 *                 mode        = 0
 *                 pattern[]   = "abc efgh"
 *                 mask        = 0xef
 *             Example 4. To compare "abc" and "efg"
 *                 input parameter:
 *                 mode        = 0
 *                 pattern[]   = "abc efg "
 *                 mask        = 0xee          
 *             Example 5. To compare "abcd"
 *                 input parameter:
 *                 mode        = 0
 *                 pattern[]   = "abcd    " <---   ### Leave 4 blank
 *                 mask        = 0xf1
 *             Example 6. To compare "abc"
 *                 input parameter:
 *                 mode        = 0
 *                 pattern[]   = "abc     " <---   ### Leave 5 blank
 *                 mask        = 0xe1
 *             Example 7. To compare "a*cd"
 *                 input parameter:
 *                 mode        = 0
 *                 pattern[]   = "a cd    " <---   ### There are 5 blank
 *                 mask        = 0xb1
 *         
 *         In above case 5,6 and 7, although set1 is not specified, we still
 *         care its first byte.
 *         
 *         
 *         Here are some 8-Byte mode examples:
 *             Example 1. To compare "abcdefgh"
 *                 input parameter:
 *                 mode        = 1
 *                 pattern[]   = "abcdefgh"
 *                 mask        = 0xff
 *             Example 2. To compare "abcde"
 *                 input parameter:
 *                 mode        = 1
 *                 pattern[]   = "abcde   "
 *                 mask        = 0xf8
 *             Example 3. To compare "abc*e"
 *                 input parameter:
 *                 mode        = 1
 *                 pattern[]   = "abcde   "
 *                 mask        = 0xe8
 */
int32
dal_ssw_filter_patternMatch_set(
    uint32                          unit,
    rtk_port_t                      port,
    rtk_filter_patternMatch_mode_t  mode,
    uint8                           *pPattern,
    uint32                          mask)
{
    int32   ret;
//    uint32  reg_idx;
    uint32  value, temp;

    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, port=%d, mode=%d, \
           pPattern=%x, mask=%x", unit, port, mode, pPattern, mask);
        
    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pPattern), RT_ERR_NULL_POINTER);
    /*RT_PARAM_CHK(strlen(pPattern) != 8, RT_ERR_PM_LENGTH);*/
    RT_PARAM_CHK(mode >= PM_MODE_END, RT_ERR_PM_MODE);
       
    if (PM_4BYTE_MODE == mode)/*4-byte mode*/
    {
        RT_PARAM_CHK((mask & 0x88) != 0x88, RT_ERR_PM_MASK);
    }
    else/*8-byte mode*/
    {
        RT_PARAM_CHK((mask & 0x80) != 0x80, RT_ERR_PM_MASK);
    }

    if (PM_4BYTE_MODE == mode)
    {
        mode = 0;
    }
    else
    {
        mode = 1;
    }
    /* Set pattern match mode*/
    //reg_idx = PATTERN_MATCH_PORT0_CONTROL + port;
    
    FILTER_SEM_LOCK(unit);
    
    if ((ret = reg_array_field_write(unit, SSW_PATTERN_MATCH_PORT_CONTROLr, port, 0, SSW_PM_MODEf, &mode)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /*Set set0 data*/
    //reg_idx = PATTERN_MATCH_PORT0_SET_0_4_BYTE_DATA + port;
    /* get value from CHIP*/
    if ((reg_array_read(unit, SSW_PATTERN_MATCH_PORT_SET_0_4_BYTE_DATAr, port, 0, &value)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    temp = pPattern[0];
    if ((ret = reg_field_set(unit, SSW_PATTERN_MATCH_PORT_SET_0_4_BYTE_DATAr, SSW_BYTE3f, &temp, &value)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    temp = pPattern[1];
    if ((ret = reg_field_set(unit, SSW_PATTERN_MATCH_PORT_SET_0_4_BYTE_DATAr, SSW_BYTE2f, &temp, &value)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    temp = pPattern[2];
    if ((ret = reg_field_set(unit, SSW_PATTERN_MATCH_PORT_SET_0_4_BYTE_DATAr, SSW_BYTE1f, &temp, &value)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    temp = pPattern[3];
    if ((ret = reg_field_set(unit, SSW_PATTERN_MATCH_PORT_SET_0_4_BYTE_DATAr, SSW_BYTE0f, &temp, &value)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    /* program value to CHIP*/
    if ((ret = reg_array_write(unit, SSW_PATTERN_MATCH_PORT_SET_0_4_BYTE_DATAr, port, 0, &value)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }


    /*Set set0 mask*/
    //reg_idx = PATTERN_MATCH_PORT0_CONTROL + port;
    temp = (mask>>4);
    if ((ret = reg_array_field_write(unit, SSW_PATTERN_MATCH_PORT_CONTROLr, port, 0, SSW_MASK_SET_0f, &temp)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /*Set set1 data*/
    //reg_idx = PATTERN_MATCH_PORT0_SET_1_4_BYTE_DATA + port;
    /* get value from CHIP*/
    if ((reg_array_read(unit, SSW_PATTERN_MATCH_PORT_SET_1_4_BYTE_DATAr, port, 0, &value)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    temp = pPattern[4];
    if ((ret = reg_field_set(unit, SSW_PATTERN_MATCH_PORT_SET_1_4_BYTE_DATAr, SSW_BYTE3f, &temp, &value)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    temp = pPattern[5];
    if ((ret = reg_field_set(unit, SSW_PATTERN_MATCH_PORT_SET_1_4_BYTE_DATAr, SSW_BYTE2f, &temp, &value)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    temp = pPattern[6];
    if ((ret = reg_field_set(unit, SSW_PATTERN_MATCH_PORT_SET_1_4_BYTE_DATAr, SSW_BYTE1f, &temp, &value)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    temp = pPattern[7];
    if ((ret = reg_field_set(unit, SSW_PATTERN_MATCH_PORT_SET_1_4_BYTE_DATAr, SSW_BYTE0f, &temp, &value)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    /* program value to CHIP*/
    if ((ret = reg_array_write(unit, SSW_PATTERN_MATCH_PORT_SET_1_4_BYTE_DATAr, port, 0, &value)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /*Set set1 mask*/
    //reg_idx = PATTERN_MATCH_PORT0_CONTROL + port;
    temp = (mask&0xff);
    if ((ret = reg_array_field_write(unit, SSW_PATTERN_MATCH_PORT_CONTROLr, port, 0, SSW_MASK_SET_1f, &temp)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    FILTER_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_filter_patternMatch_set */


/* Module Name     : filter     */
/* Sub-module Name : flow table */

/* Function Name:
 *      dal_ssw_filter_flowTbl_del
 * Description:
 *      Delete one flow table entry from the specified device.
 * Input:
 *      unit      - unit id
 *      filter_id - filter id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_FILTER_ENTRYIDX        - invalid entry index
 *      RT_ERR_FILTER_FLOWTBL_RULENUM - invalid flow table rulenum
 *      RT_ERR_FILTER_FLOWTBL_TYPE    - entry type isn't a flow table rule
 *      RT_ERR_FILTER_FLOWTBL_EMPTY   - flow table entry is empty
 * Note:
 *      Valid range of filter id of flow table is 0 .. HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline -1.
 */
int32
dal_ssw_filter_flowTbl_del(uint32 unit, rtk_filter_id_t filter_id)
{
    int32   ret;
    uint32  cutline;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, filter_id=%d", 
           unit, filter_id);
    
    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    /* Check filter id range */
    RT_PARAM_CHK(filter_id > HAL_PIE_FILTER_ID_MAX(unit), RT_ERR_FILTER_ENTRYIDX);
    
    /*Check if filter id is in the range of flow table*/
    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
        return ret;
    RT_PARAM_CHK(filter_id >= cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit), RT_ERR_FILTER_FLOWTBL_RULENUM);        

    FILTER_SEM_LOCK(unit);
    
    if ((ret = _dal_ssw_filterEntry_del(unit, filter_id, FLOW_CLASSIFICATION)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
    }
    
    FILTER_SEM_UNLOCK(unit);
    
    return ret;
} /* end of dal_ssw_filter_flowTbl_del */


/* Function Name:
 *      dal_ssw_filter_flowTbl_delAll
 * Description:
 *      Delete all flow table entries from the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 * Note:
 *      None
 */
int32
dal_ssw_filter_flowTbl_delAll(uint32 unit)
{
    int32   ret;
    uint32  index, min_idx, max_idx;
    uint32  cutline;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d", unit);

    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    
    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    max_idx = (cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) - 1;
    min_idx = 0;

    /* Fixup the owner for all flow-table entry */
    for (index = min_idx; index <= max_idx; index++)
    {
        if (FILTERINFO_OWNER_CHK(unit, index, FLOW_CLASSIFICATION))
        {
            /* Delete this entry */
            if ((ret = dal_ssw_filter_flowTbl_del(unit, index)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
            }
        }
    }
    
    return RT_ERR_OK;
} /* end of dal_ssw_filter_flowTbl_delAll */


/* Function Name:
 *      dal_ssw_filter_flowTbl_get
 * Description:
 *      Get one flow table entry from the specified device.
 * Input:
 *      unit        - unit id
 *      filter_id   - filter id
 * Output:
 *      pFilter_cfg - pointer buffer of flow table data
 *      pAction     - pointer buffer of flow table action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_FILTER_ENTRYIDX        - invalid entry index
 *      RT_ERR_FILTER_FLOWTBL_RULENUM - invalid flow table rulenum
 *      RT_ERR_NULL_POINTER           - input parameter may be null pointer
 * Note:
 *      Valid range of filter id of flow table is 0 .. HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline -1.
 */
int32
dal_ssw_filter_flowTbl_get(
    uint32                  unit,
    rtk_filter_id_t         filter_id,
    rtk_filter_flowTbl_t    *pFilter_cfg,
    rtk_filter_action_t     *pAction)
{
    int32   ret;
    uint32  cutline;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, filter_id=%d", unit, filter_id);

    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);
     
    /* Check arguments */
    /* Check filter id range */
    RT_PARAM_CHK(filter_id > HAL_PIE_FILTER_ID_MAX(unit), RT_ERR_FILTER_ENTRYIDX);
      
    /*Check if filter id is in the range of flow table*/
    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
        return ret;
    RT_PARAM_CHK(filter_id >= cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit), RT_ERR_FILTER_FLOWTBL_RULENUM);    
    RT_PARAM_CHK((NULL == pFilter_cfg), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);

    /* Check the owner of this entry */
    /*RT_PARAM_CHK(!FILTERINFO_OWNER_CHK(unit, filter_id, FLOW_CLASSIFICATION), RT_ERR_FAILED);*/
    
    FILTER_SEM_LOCK(unit);
    
    if ((ret = _dal_ssw_flowTblEntry_get(unit, filter_id, pFilter_cfg, pAction)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    FILTER_SEM_UNLOCK(unit);
          
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "pFilter_cfg=%x, pAction=%x", 
           pFilter_cfg, pAction);
          
    return RT_ERR_OK;
} /* end of dal_ssw_filter_flowTbl_get */

/* Function Name:
 *      dal_ssw_filter_flowTbl_set
 * Description:
 *      Set one flow table entry to the specified device.
 * Input:
 *      unit        - unit id
 *      filter_id   - filter id
 * Output:
 *      pFilter_cfg - pointer buffer of flow table data
 *      pAction     - pointer buffer of flow table action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_FILTER_ENTRYIDX        - invalid entry index
 *      RT_ERR_FILTER_FLOWTBL_RULENUM - invalid flow table rulenum
 *      RT_ERR_NULL_POINTER           - input parameter may be null pointer
 * Note:
 *      Valid range of filter id of flow table is 0 .. HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline -1.
 */
int32
dal_ssw_filter_flowTbl_set(
    uint32                  unit,
    rtk_filter_id_t         filter_id,
    rtk_filter_flowTbl_t    *pFilter_cfg,
    rtk_filter_action_t     *pAction)
{
    int32   ret;
    uint32  cutline;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, filter_id=%d, pFilter_cfg=%x, pAction=%x", 
           unit, filter_id, pFilter_cfg, pAction);
        
    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    /* Check arguments */
    /* Check filter id range */
    RT_PARAM_CHK(filter_id > HAL_PIE_FILTER_ID_MAX(unit), RT_ERR_FILTER_ENTRYIDX);
    /*Check if filter id is in the range of flow table*/
    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    RT_PARAM_CHK(filter_id >= cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit), RT_ERR_FILTER_FLOWTBL_RULENUM);    
    RT_PARAM_CHK((NULL == pFilter_cfg), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pAction->actGroup != FLOW_LOG_ACTGROUP) && (pAction->actGroup != FLOW_ASSIGN_VLAN_ACTGROUP), RT_ERR_FILTER_ACTION);
     
    /* Check the owner of this entry */
    RT_PARAM_CHK(!FILTERINFO_OWNER_CHK(unit, filter_id, FILTER_NO_OWNER) \
              && !FILTERINFO_OWNER_CHK(unit, filter_id, FLOW_CLASSIFICATION), RT_ERR_FAILED);
   
    FILTER_SEM_LOCK(unit);
    
    /*Spec change*/
    /*Caller have to set the valid bit by himself, if who want to write a valid entry into PIE.*/
    /*pFilter_cfg->valid = TRUE;*/
    if ((ret = _dal_ssw_flowTblEntry_set(unit, filter_id, pFilter_cfg, pAction)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* Fixup the owner of this entry */
    FILTERINFO_OWNER_SET(unit, filter_id, FLOW_CLASSIFICATION);
    
    FILTER_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_ssw_filter_flowTbl_set */


/* Function Name:
 *      dal_ssw_filter_flowTbl_add
 * Description:
 *      Add one flow table entry to the specified device.
 * Input:
 *      unit        - unit id
 *      filter_id   - filter id
 *      pFilter_cfg - pointer buffer of flow table data
 *      pAction     - pointer buffer of flow table action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_FILTER_ENTRYIDX        - invalid entry index
 *      RT_ERR_FILTER_FLOWTBL_RULENUM - invalid flow table rulenum
 *      RT_ERR_FILTER_FLOWTBL_TYPE    - entry type isn't an flow table rule
 *      RT_ERR_FILTER_ACTION          - action doesn't consist to entry type
 * Note:
 *      Valid range of filter id of flow table is 0 .. HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline -1.
 *
 *      Caller have to set the valid bit by himself, if who want to write 
 *      a valid entry into PIE. 
 */
int32
dal_ssw_filter_flowTbl_add(
    uint32                  unit,
    rtk_filter_id_t         filter_id,
    rtk_filter_flowTbl_t    *pFilter_cfg,
    rtk_filter_action_t     *pAction)
{
    int32   ret;
    uint32  cutline;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, filter_id=%d, pFilter_cfg=%x, pAction=%x", 
           unit, filter_id, pFilter_cfg, pAction);
        
    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    /* Check arguments */
    /* Check filter id range */
    RT_PARAM_CHK(filter_id > HAL_PIE_FILTER_ID_MAX(unit), RT_ERR_FILTER_ENTRYIDX);
    /*Check if filter id is in the range of flow table*/
    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    RT_PARAM_CHK(filter_id >= cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit), RT_ERR_FILTER_FLOWTBL_RULENUM);    
    RT_PARAM_CHK((NULL == pFilter_cfg), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pAction->actGroup != FLOW_LOG_ACTGROUP) && (pAction->actGroup != FLOW_ASSIGN_VLAN_ACTGROUP), RT_ERR_FILTER_ACTION);
     
    /* Check the owner of this entry */
    RT_PARAM_CHK(!FILTERINFO_OWNER_CHK(unit, filter_id, FILTER_NO_OWNER), RT_ERR_FAILED);
   
    FILTER_SEM_LOCK(unit);
    
    /*Spec change*/
    /*Caller have to set the valid bit by himself, if who want to write a valid entry into PIE.*/
    /*pFilter_cfg->valid = TRUE;*/
    if ((ret = _dal_ssw_flowTblEntry_set(unit, filter_id, pFilter_cfg, pAction)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* Fixup the owner of this entry */
    FILTERINFO_OWNER_SET(unit, filter_id, FLOW_CLASSIFICATION);
    
    FILTER_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_ssw_filter_flowTbl_add*/

/* Function Name:
 *      dal_ssw_filter_flowTbl_validate
 * Description:
 *      Validate entry without modifying other field of flow entry.
 * Input:
 *      unit        - unit id
 *      filter_id   - filter id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_FILTER_ENTRYIDX        - invalid entry index
 *      RT_ERR_FILTER_FLOWTBL_RULENUM - invalid flow table rulenum
 *      RT_ERR_FILTER_FLOWTBL_TYPE    - entry type isn't an flow table rule
 * Note:
 */
int32
dal_ssw_filter_flowTbl_validate(
    uint32                  unit,
    rtk_filter_id_t         filter_id)
{
    int32   ret;
    uint32  cutline;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, filter_id=%d", 
           unit, filter_id);
        
    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    /* Check arguments */
    /* Check filter id range */
    RT_PARAM_CHK(filter_id > HAL_PIE_FILTER_ID_MAX(unit), RT_ERR_FILTER_ENTRYIDX);
    /*Check if filter id is in the range of flow table*/
    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    RT_PARAM_CHK(filter_id >= cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit), RT_ERR_FILTER_FLOWTBL_RULENUM);    
     
    /* Check the owner of this entry */
    RT_PARAM_CHK(!FILTERINFO_OWNER_CHK(unit, filter_id, FLOW_CLASSIFICATION), RT_ERR_FAILED);
   
    FILTER_SEM_LOCK(unit);
    
    ret = _dal_ssw_filterEntry_valid_set(unit, filter_id, 1/* valid*/);
    
    FILTER_SEM_UNLOCK(unit);
    
    if (ret != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_ssw_filter_flowTbl_validate*/
    
/* Function Name:
 *      dal_ssw_filter_flowTbl_invalidate
 * Description:
 *      Invalidate entry without modifying other field of flow entry.
 * Input:
 *      unit        - unit id
 *      filter_id   - filter id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_FILTER_ENTRYIDX        - invalid entry index
 *      RT_ERR_FILTER_FLOWTBL_RULENUM - invalid flow table rulenum
 *      RT_ERR_FILTER_FLOWTBL_TYPE    - entry type isn't an flow table rule
 * Note:
 */
int32
dal_ssw_filter_flowTbl_invalidate(
    uint32                  unit,
    rtk_filter_id_t         filter_id)
{
    int32   ret;
    uint32  cutline;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, filter_id=%d", 
           unit, filter_id);
        
    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    /* Check arguments */
    /* Check filter id range */
    RT_PARAM_CHK(filter_id > HAL_PIE_FILTER_ID_MAX(unit), RT_ERR_FILTER_ENTRYIDX);
    /*Check if filter id is in the range of flow table*/
    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    RT_PARAM_CHK(filter_id >= cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit), RT_ERR_FILTER_FLOWTBL_RULENUM);    
     
    /* Check the owner of this entry */
    RT_PARAM_CHK(!FILTERINFO_OWNER_CHK(unit, filter_id, FLOW_CLASSIFICATION), RT_ERR_FAILED);
   
    FILTER_SEM_LOCK(unit);
    
    ret = _dal_ssw_filterEntry_valid_set(unit, filter_id, 0/* invalid*/);
    
    FILTER_SEM_UNLOCK(unit);
    
    if (ret != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_ssw_filter_flowTbl_invalidate*/


/* Module Name     : filter      */
/* Sub-module Name : ingress acl */

/* Function Name:
 *      dal_ssw_filter_igrAcl_del
 * Description:
 *      Delete one ingress acl entry from the specified device.
 * Input:
 *      unit      - unit id
 *      filter_id - filter id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_FILTER_ENTRYIDX      - invalid entry index
 *      RT_ERR_FILTER_INACL_RULENUM - invalid ACL rulenum
 *      RT_ERR_FILTER_INACL_TYPE    - entry type isn't an ingress ACL rule
 *      RT_ERR_FILTER_INACL_EMPTY   - entry is empty
 * Note:
 *      Valid range of filter id of ingress acl table is 
 *      HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline ~ HAL_PIE_FILTER_ID_MAX.
 */
int32
dal_ssw_filter_igrAcl_del(uint32 unit, rtk_filter_id_t filter_id)
{
    int32   ret;
    uint32  cutline;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, filter_id=%d", 
           unit, filter_id);
    
    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);
    
    /* Check filter id range */
    RT_PARAM_CHK(filter_id > HAL_PIE_FILTER_ID_MAX(unit), RT_ERR_FILTER_ENTRYIDX);
    
    /*Check if filter id is in the range of ACL*/
    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    RT_PARAM_CHK(filter_id < cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit), RT_ERR_FILTER_INACL_RULENUM);        
    
    /* Check the owner of this entry */
    RT_PARAM_CHK(FILTERINFO_OWNER_CHK(unit, filter_id, FILTER_NO_OWNER), RT_ERR_FILTER_INACL_EMPTY);
    
    FILTER_SEM_LOCK(unit);
    
    if ((ret = _dal_ssw_filterEntry_del(unit, filter_id, IGR_ACL)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    FILTER_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_ssw_filter_igrAcl_del */


/* Function Name:
 *      dal_ssw_filter_igrAcl_delAll
 * Description:
 *      Delete all ingress acl entries from the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 * Note:
 *      None
 */
int32
dal_ssw_filter_igrAcl_delAll(uint32 unit)
{
    int32   ret;
    uint32  index, min_idx, max_idx;
    uint32  cutline;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d", unit);

    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    max_idx = HAL_PIE_FILTER_ID_MAX(unit);
    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    min_idx = cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit);        

    /* Fixup the owner for all acl-table entry */
    for (index = min_idx; index <= max_idx; index++)
    {
        if (FILTERINFO_OWNER_CHK(unit, index, IGR_ACL))
        {
            /* Delete this entry */
            if ((ret = dal_ssw_filter_igrAcl_del(unit, index)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
            }
        }
    }
   
    return RT_ERR_OK;
} /* end of dal_ssw_filter_igrAcl_delAll */


/* Function Name:
 *      dal_ssw_filter_igrAcl_get
 * Description:
 *      Get one ingress acl entry from the specified device.
 * Input:
 *      unit        - unit id
 *      filter_id   - filter id
 * Output:
 *      pFilter_cfg - pointer buffer of ingress acl data
 *      pAction     - pointer buffer of ingress acl action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_FILTER_ENTRYIDX      - invalid entry index
 *      RT_ERR_FILTER_INACL_RULENUM - invalid ACL rulenum
 *      RT_ERR_FILTER_INACL_TYPE    - entry type isn't an ingress ACL rule
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      Valid range of filter id of ingress acl table is 
 *      HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline ~ HAL_PIE_FILTER_ID_MAX.
 */
int32
dal_ssw_filter_igrAcl_get(
    uint32              unit,
    rtk_filter_id_t     filter_id,
    rtk_filter_aclCfg_t *pFilter_cfg,
    rtk_filter_action_t *pAction)
{
    int32   ret;
    uint32  cutline;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, filter_id=%d", unit, filter_id);

    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    /* Check arguments */
    /* Check filter id range */
    RT_PARAM_CHK(filter_id > HAL_PIE_FILTER_ID_MAX(unit), RT_ERR_FILTER_ENTRYIDX);

    /*Check if filter id is in the range of flow table*/
    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    RT_PARAM_CHK(filter_id < cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit), RT_ERR_FILTER_INACL_RULENUM);    
    RT_PARAM_CHK((NULL == pFilter_cfg), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);

    FILTER_SEM_LOCK(unit);
    
    if ((ret = _dal_ssw_aclEntry_get(unit, filter_id, pFilter_cfg, pAction)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    FILTER_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "pFilter_cfg=%x, pAction=%x", pFilter_cfg, pAction);

    return RT_ERR_OK;
} /* end of dal_ssw_filter_igrAcl_get */

/* Function Name:
 *      dal_ssw_filter_igrAcl_set
 * Description:
 *      Set one ingress acl entry to the specified device.
 * Input:
 *      unit        - unit id
 *      filter_id   - filter id
 * Output:
 *      pFilter_cfg - pointer buffer of ingress acl data
 *      pAction     - pointer buffer of ingress acl action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_FILTER_ENTRYIDX      - invalid entry index
 *      RT_ERR_FILTER_INACL_RULENUM - invalid ACL rulenum
 *      RT_ERR_FILTER_INACL_TYPE    - entry type isn't an ingress ACL rule
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      Valid range of filter id of ingress acl table is 
 *      HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline ~ HAL_PIE_FILTER_ID_MAX.
 */
int32
dal_ssw_filter_igrAcl_set(
    uint32              unit,
    rtk_filter_id_t     filter_id,
    rtk_filter_aclCfg_t *pFilter_cfg,
    rtk_filter_action_t *pAction)
{
    int32   ret;
    uint32  cutline;    

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, filter_id=%d, pFilter_cfg=%x, pAction=%x", 
           unit, filter_id, pFilter_cfg, pAction);

    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    /* Check arguments */
    /* Check filter id range */
    RT_PARAM_CHK(filter_id > HAL_PIE_FILTER_ID_MAX(unit), RT_ERR_FILTER_ENTRYIDX);
    /*Check if filter id is in the range of ACL*/
    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    RT_PARAM_CHK(filter_id < cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit), RT_ERR_FILTER_INACL_RULENUM);    
    RT_PARAM_CHK((NULL == pFilter_cfg), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pAction->actGroup >= FILTER_ACTGROUP_END) && (pAction->actGroup != FILTER_RESERVED_ACTGROUP) , RT_ERR_FILTER_ACTION);

    /* Check the owner of this entry */
    RT_PARAM_CHK(!FILTERINFO_OWNER_CHK(unit, filter_id, FILTER_NO_OWNER) \
                && !FILTERINFO_OWNER_CHK(unit, filter_id, IGR_ACL) , RT_ERR_FAILED);

    FILTER_SEM_LOCK(unit);
    
    /*Caller have to set the valid bit by himself, if who want to write a valid entry into PIE.*/
    if ((ret = _dal_ssw_aclEntry_set(unit, filter_id, pFilter_cfg, pAction)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* Fixup the owner of this entry */
    FILTERINFO_OWNER_SET(unit, filter_id, IGR_ACL);
    
    FILTER_SEM_UNLOCK(unit);    

    return RT_ERR_OK;
} /* end of dal_ssw_filter_igrAcl_set */

/* Function Name:
 *      dal_ssw_filter_igrAcl_add
 * Description:
 *      Add one ingress acl entry to the specified device.
 * Input:
 *      unit        - unit id
 *      filter_id   - filter id
 *      pFilter_cfg - pointer buffer of ingress acl data
 *      pAction     - pointer buffer of ingress acl action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_FILTER_ENTRYIDX      - invalid entry index
 *      RT_ERR_FILTER_INACL_RULENUM - invalid ACL rulenum
 *      RT_ERR_FILTER_INACL_TYPE    - entry type isn't an ingress ACL rule
 *      RT_ERR_FILTER_INACL_EXIST   - ingress ACL entry is already exist
 *      RT_ERR_FILTER_ACTION        - action doesn't consist to entry type
 * Note:
 *      Valid range of filter id of ingress acl table is 
 *      HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline ~ HAL_PIE_FILTER_ID_MAX.
 *
 *      Caller have to set the valid bit by himself, if who want to write 
 *      a valid entry into PIE.
 */
int32
dal_ssw_filter_igrAcl_add(
    uint32              unit,
    rtk_filter_id_t     filter_id,
    rtk_filter_aclCfg_t *pFilter_cfg,
    rtk_filter_action_t *pAction)
{
    int32   ret;
    uint32  cutline;    

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, filter_id=%d, pFilter_cfg=%x, pAction=%x", 
           unit, filter_id, pFilter_cfg, pAction);

    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    /* Check arguments */
    /* Check filter id range */
    RT_PARAM_CHK(filter_id > HAL_PIE_FILTER_ID_MAX(unit), RT_ERR_FILTER_ENTRYIDX);
    /*Check if filter id is in the range of ACL*/
    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    RT_PARAM_CHK(filter_id < cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit), RT_ERR_FILTER_INACL_RULENUM);    
    RT_PARAM_CHK((NULL == pFilter_cfg), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pAction->actGroup >= FILTER_ACTGROUP_END) && (pAction->actGroup != FILTER_RESERVED_ACTGROUP) , RT_ERR_FILTER_ACTION);

    /* Check the owner of this entry */
    RT_PARAM_CHK(!FILTERINFO_OWNER_CHK(unit, filter_id, FILTER_NO_OWNER), RT_ERR_FAILED);

    FILTER_SEM_LOCK(unit);
    
    /*Spec change*/
    /*Caller have to set the valid bit by himself, if who want to write a valid entry into PIE.*/
    /* pFilter_cfg->valid = TRUE; */
    if ((ret = _dal_ssw_aclEntry_set(unit, filter_id, pFilter_cfg, pAction)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* Fixup the owner of this entry */
    FILTERINFO_OWNER_SET(unit, filter_id, IGR_ACL);
    
    FILTER_SEM_UNLOCK(unit);    

    return RT_ERR_OK;
} /* end of dal_ssw_filter_igrAcl_add */

/* Function Name:
 *      dal_ssw_filter_igrAcl_validate
 * Description:
 *      Validate acl entry without modifying other content of acl
 * Input:
 *      unit        - unit id
 *      filter_id   - filter id
 * 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_FILTER_ENTRYIDX      - invalid entry index
 *      RT_ERR_FILTER_INACL_RULENUM - invalid ACL rulenum
 *      RT_ERR_FILTER_INACL_TYPE    - entry type isn't an ingress ACL rule
 * Note:
 */
int32
dal_ssw_filter_igrAcl_validate(
    uint32                  unit,
    rtk_filter_id_t         filter_id)
{
    int32   ret;
    uint32  cutline;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, filter_id=%d", unit, filter_id);

    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    /* Check arguments */
    /* Check filter id range */
    RT_PARAM_CHK(filter_id > HAL_PIE_FILTER_ID_MAX(unit), RT_ERR_FILTER_ENTRYIDX);

    /*Check if filter id is in the range of flow table*/
    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    RT_PARAM_CHK(filter_id < cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit), RT_ERR_FILTER_INACL_RULENUM);    
     
    /* Check the owner of this entry */
    RT_PARAM_CHK(!FILTERINFO_OWNER_CHK(unit, filter_id, IGR_ACL), RT_ERR_FAILED);
   
    FILTER_SEM_LOCK(unit);
    
    ret = _dal_ssw_filterEntry_valid_set(unit, filter_id, 1/* valid*/);
    
    FILTER_SEM_UNLOCK(unit);
    
    if (ret != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    return RT_ERR_OK;
}
    
/* Function Name:
 *      dal_ssw_filter_igrAcl_invalidate
 * Description:
 *      Invalidate acl entry without modifying other content of acl
 * Input:
 *      unit        - unit id
 *      filter_id   - filter id
 * 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_FILTER_ENTRYIDX      - invalid entry index
 *      RT_ERR_FILTER_INACL_RULENUM - invalid ACL rulenum
 *      RT_ERR_FILTER_INACL_TYPE    - entry type isn't an ingress ACL rule
 * Note:
 */
int32
dal_ssw_filter_igrAcl_invalidate(
    uint32                  unit,
    rtk_filter_id_t         filter_id)
{
    int32   ret;
    uint32  cutline;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, filter_id=%d", unit, filter_id);

    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    /* Check arguments */
    /* Check filter id range */
    RT_PARAM_CHK(filter_id > HAL_PIE_FILTER_ID_MAX(unit), RT_ERR_FILTER_ENTRYIDX);

    /*Check if filter id is in the range of flow table*/
    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    RT_PARAM_CHK(filter_id < cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit), RT_ERR_FILTER_INACL_RULENUM);    
     
    /* Check the owner of this entry */
    RT_PARAM_CHK(!FILTERINFO_OWNER_CHK(unit, filter_id, IGR_ACL), RT_ERR_FAILED);
   
    FILTER_SEM_LOCK(unit);
    
    ret = _dal_ssw_filterEntry_valid_set(unit, filter_id, 0/* invalid*/);
    
    FILTER_SEM_UNLOCK(unit);
    
    if (ret != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_ssw_filter_igrAclRateLimit_get
 * Description:
 *      Get ratelimit value of the metering entry from the specified device.
 * Input:
 *      unit     - unit id
 *      meter_id - meter id
 * Output:
 *      pRate    - pointer buffer of ingress acl ratelimit
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_FILTER_METER_ID - invalid metering id
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 * Note:
 *      Valid range of meter id is 0 .. HAL_MAX_NUM_OF_METERING-1
 */
int32
dal_ssw_filter_igrAclRateLimit_get(uint32 unit, rtk_meter_id_t meter_id, uint32 *pRate)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, meter_id=%d", unit, meter_id);
    
    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(meter_id >= HAL_MAX_NUM_OF_METERING(unit), RT_ERR_FILTER_METER_ID);
    RT_PARAM_CHK((NULL == pRate), RT_ERR_NULL_POINTER);
    
    FILTER_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, acl_rate_limit_leaky_bucket_control_regidx[meter_id], aclRateLimit_fieldIdx[meter_id], pRate)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    FILTER_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "pRate=%d", *pRate);
    
    return RT_ERR_OK;
} /* end of dal_ssw_filter_igrAclRateLimit_get */


/* Function Name:
 *      dal_ssw_filter_igrAclRateLimit_set
 * Description:
 *      Set ratelimit value of the metering entry to the specified device.
 * Input:
 *      unit     - unit id
 *      meter_id - meter id
 *      rate     - ingress acl ratelimit
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_FILTER_METER_ID - invalid metering id
 *      RT_ERR_RATE            - invalid rate
 * Note:
 *      Valid range of meter id is 0 .. HAL_MAX_NUM_OF_METERING-1
 */
int32
dal_ssw_filter_igrAclRateLimit_set(uint32 unit, rtk_meter_id_t meter_id, uint32 rate)
{
    int32   ret;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, meter_id=%d, \
           rate=%d", unit, meter_id, rate);
        
    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(meter_id >= HAL_MAX_NUM_OF_METERING(unit), RT_ERR_FILTER_METER_ID);
    RT_PARAM_CHK(rate > HAL_ACL_RATE_MAX(unit), RT_ERR_RATE);

    FILTER_SEM_LOCK(unit);
    
    /* set value to CHIP*/
    if ((ret = reg_field_write(unit, acl_rate_limit_leaky_bucket_control_regidx[meter_id], aclRateLimit_fieldIdx[meter_id], &rate)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    FILTER_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_filter_igrAclRateLimit_set */


/* Module Name     : filter                          */
/* Sub-module Name : metering and statistic counters */

/* Function Name:
 *      dal_ssw_filter_stat_get
 * Description:
 *      Get statistic counter of the log id from the specified device.
 * Input:
 *      unit      - unit id
 *      log_id    - log id
 * Output:
 *      pPkt_cnt  - pointer buffer of packet count
 *      pByte_cnt - pointer buffer of byte count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_FILTER_LOG_ID - invalid log id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      Valid range of log id is 0 .. max number of log entry -1
 */
int32
dal_ssw_filter_stat_get(uint32 unit, rtk_log_id_t log_id, uint32 *pPkt_cnt, uint64 *pByte_cnt)
{
    int32   ret;
    uint32  log_tableSize;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, log_id=%d", unit, log_id);
    
    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    /* Check arguments */
    if ((ret = table_size_get(unit, SSW_LOG_TABLEt, &log_tableSize)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    RT_PARAM_CHK(log_id >= log_tableSize, RT_ERR_FILTER_LOG_ID);
    RT_PARAM_CHK((NULL == pPkt_cnt), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pByte_cnt), RT_ERR_NULL_POINTER);
    
    FILTER_SEM_LOCK(unit);
    
    if ((ret = _dal_ssw_logEntry_get(unit, log_id, pPkt_cnt, pByte_cnt)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    FILTER_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "pPkt_cnt=%d, pByte_cnt=%llu", 
           *pPkt_cnt, *pByte_cnt);
    
    return RT_ERR_OK;
} /* end of dal_ssw_filter_stat_get */


/* Function Name:
 *      dal_ssw_filter_stat_set
 * Description:
 *      Set statistic counter of the log id to the specified device.
 * Input:
 *      unit     - unit id
 *      log_id   - log id
 *      pkt_cnt  - packet count
 *      byte_cnt - byte count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_FILTER_LOG_ID - invalid log id
 * Note:
 *      Valid range of log id is 0 .. max number of log entry -1
 */
int32
dal_ssw_filter_stat_set(uint32 unit, rtk_log_id_t log_id, uint32 pkt_cnt, uint64 byte_cnt)
{
    int32   ret;
    uint32  log_tableSize;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, log_id=%d, pkt_cnt=%d, \
           byte_cnt=%llu", unit, log_id, pkt_cnt, byte_cnt);
    
    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    /* Check arguments */
    if ((ret = table_size_get(unit, SSW_LOG_TABLEt, &log_tableSize)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    RT_PARAM_CHK(log_id >= log_tableSize, RT_ERR_FILTER_LOG_ID);

    FILTER_SEM_LOCK(unit);
    
    if ((ret = _dal_ssw_logEntry_set(unit, log_id, pkt_cnt, byte_cnt)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    FILTER_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_filter_stat_set */

/* Module Name     : filter                         */
/* Sub-module Name : mac-based vlan (by flow table) */

/* Function Name:
 *      dal_ssw_filter_macBasedVlan_add
 * Description:
 *      Add a source mac which associate to the vlan id and priority to the specified device.
 * Input:
 *      unit      - unit id
 *      filter_id - filter id
 *      sa_mac    - source mac address
 *      vid       - vlan id
 *      pri       - priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_MAC                    - invalid mac address
 *      RT_ERR_FILTER_FLOWTBL_EXIST   - flow table entry is already exist
 *      RT_ERR_FILTER_ENTRYIDX        - invalid entry index
 *      RT_ERR_FILTER_FLOWTBL_RULENUM - invalid flow table rulenum
 *      RT_ERR_VLAN_VID               - invalid vid
 *      RT_ERR_OUT_OF_RANGE           - input parameter out of range
 * Note:
 *      1. The incoming packet which match the source mac address will use the
 *         configure vid and priority for ingress pipeline
 *      2. For destination mac or both, please use the flow table API to configure.
 */
int32
dal_ssw_filter_macBasedVlan_add(
    uint32          unit,
    rtk_filter_id_t filter_id,
    rtk_mac_t       sa_mac,
    rtk_vlan_t      vid,
    rtk_pri_t       pri)
{
    int32   ret;
    rtk_filter_flowTbl_t    filter_cfg;
    rtk_filter_action_t     action;    
    uint32  cutline;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, filter_id=%d, sa_mac=%x-%x-%x-%x-%x-%x, \
           vid=%d, pri=%d", unit, filter_id, sa_mac.octet[0], sa_mac.octet[1], sa_mac.octet[2], sa_mac.octet[3],sa_mac.octet[4],
           sa_mac.octet[5], vid, pri);
        
    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);
    
    /* Check arguments */
    RT_PARAM_CHK(filter_id > HAL_PIE_FILTER_ID_MAX(unit), RT_ERR_FILTER_ENTRYIDX);
    /*Check if filter id is in the range of flow table*/
    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    RT_PARAM_CHK(filter_id >= cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit), RT_ERR_FILTER_FLOWTBL_RULENUM);    
    /* [FIXME] check the source-mac */
    RT_PARAM_CHK((vid < RTK_VLAN_ID_MIN) || (vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    RT_PARAM_CHK(pri >= RTK_MAX_NUM_OF_PRIORITY, RT_ERR_OUT_OF_RANGE);

    /* Check the owner of this entry */
    RT_PARAM_CHK(!FILTERINFO_OWNER_CHK(unit, filter_id, FILTER_NO_OWNER), RT_ERR_FILTER_FLOWTBL_EXIST);
    
    /* Setting the flow-table parameters */
    osal_memset(&filter_cfg, 0, sizeof(rtk_filter_flowTbl_t));
    osal_memset(&action, 0, sizeof(rtk_filter_action_t));
    osal_memcpy(filter_cfg.smac, sa_mac.octet, ETHER_ADDR_LEN);
    osal_memset(filter_cfg.care_smac, 0xff, ETHER_ADDR_LEN);
    action.actGroup = 0x4;   /* Assign VLAN */
    action.un.assignVlan.actvid = vid;
    action.un.assignVlan.actpri = pri;
    filter_cfg.valid = TRUE;
    
    FILTER_SEM_LOCK(unit);
    
    if ((ret = _dal_ssw_flowTblEntry_set(unit, filter_id, &filter_cfg, &action)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* Fixup the owner of this entry */
    FILTERINFO_OWNER_SET(unit, filter_id, MAC_BASED_VLAN);
    
    FILTER_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_filter_macBasedVlan_add */


/* Function Name:
 *      dal_ssw_filter_macBasedVlan_del
 * Description:
 *      Delete a source mac which associate to vlan id and priority from the specified device.
 * Input:
 *      unit      - unit id
 *      filter_id - filter id
 *      sa_mac    - source mac address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_MAC                    - invalid mac address
 *      RT_ERR_FILTER_FLOWTBL_EMPTY   - flow table entry is empty
 *      RT_ERR_FILTER_ENTRYIDX        - invalid entry index
 *      RT_ERR_FILTER_FLOWTBL_RULENUM - invalid flow table rulenum
 *      RT_ERR_FILTER_FLOWTBL_TYPE    - entry type isn't a flow table rule
 * Note:
 *      None
 */
int32
dal_ssw_filter_macBasedVlan_del(uint32 unit, rtk_filter_id_t filter_id, rtk_mac_t sa_mac)
{
    int32   ret;
    rtk_filter_flowTbl_t    filter_cfg;
    rtk_filter_action_t     action;    
    uint32  cutline;
    
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, filter_id=%d, sa_mac=%x-%x-%x-%x-%x-%x", 
           unit, filter_id, sa_mac.octet[0], sa_mac.octet[1], sa_mac.octet[2], sa_mac.octet[3],sa_mac.octet[4], sa_mac.octet[5]);
        
    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(filter_id > HAL_PIE_FILTER_ID_MAX(unit), RT_ERR_FILTER_ENTRYIDX);
    /*Check if filter id is in the range of flow table*/
    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    RT_PARAM_CHK(filter_id >= cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit), RT_ERR_FILTER_FLOWTBL_RULENUM);    

    /* Check the owner of this entry */
    RT_PARAM_CHK(FILTERINFO_OWNER_CHK(unit, filter_id, FILTER_NO_OWNER), RT_ERR_FILTER_FLOWTBL_EMPTY);
    RT_PARAM_CHK(!FILTERINFO_OWNER_CHK(unit, filter_id, MAC_BASED_VLAN), RT_ERR_FAILED);

    FILTER_SEM_LOCK(unit);
    /* Get this entry */
    if ((ret = _dal_ssw_flowTblEntry_get(unit, filter_id, &filter_cfg, &action)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    if ((ret = rt_util_macCmp(&filter_cfg.smac[0], &sa_mac.octet[0])) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* Delete this entry */
    if ((ret = _dal_ssw_filterEntry_del(unit, filter_id, MAC_BASED_VLAN)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    FILTER_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_filter_macBasedVlan_del */


/* Function Name:
 *      dal_ssw_filter_macBasedVlan_delAll
 * Description:
 *      Delete all source mac addresses that associate to vlan id and priority from the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 * Note:
 *      None
 */
int32
dal_ssw_filter_macBasedVlan_delAll(uint32 unit)
{
    int32  ret;
    uint32 index, min_idx, max_idx;
    uint32 cutline;
    

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d", unit);

    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);
    
    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    min_idx = 0;
    max_idx = cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit) - 1;
    
    FILTER_SEM_LOCK(unit);
    /* Delete all mac-based vlan entry */
    for (index = min_idx; index <= max_idx; index++)
    {
        if (FILTERINFO_OWNER_CHK(unit, index, MAC_BASED_VLAN))
        {
            /* Delete this entry */
            if ((ret = _dal_ssw_filterEntry_del(unit, index, MAC_BASED_VLAN)) != RT_ERR_OK)
            {
                FILTER_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
                return ret;
            }
        }
    }
    
    FILTER_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_filter_macBasedVlan_delAll */

/* Module Name     : filter                                 */
/* Sub-module Name : ingress vlan translate (by flow table) */

/* Function Name:
 *      dal_ssw_filter_igrVlanXlate_add
 * Description:
 *      Add the ingress vlan translate mapping of the port to the specified device.
 * Input:
 *      unit      - unit id
 *      filter_id - filter id
 *      port      - port id
 *      old_vid   - old vlan id
 *      new_vid   - new vlan id
 *      new_pri   - new priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_PORT_ID                - invalid port id
 *      RT_ERR_FILTER_FLOWTBL_EXIST   - flow table entry is already exist
 *      RT_ERR_FILTER_ENTRYIDX        - invalid entry index
 *      RT_ERR_FILTER_FLOWTBL_RULENUM - invalid flow table rulenum
 *      RT_ERR_VLAN_VID               - invalid vid
 *      RT_ERR_OUT_OF_RANGE           - input parameter out of range
 * Note:
 *      1. The incoming packet in the port which match the old_vid will use the
 *         configure new_vid and new_pri for ingress pipeline
 */
int32
dal_ssw_filter_igrVlanXlate_add(
    uint32          unit,
    rtk_filter_id_t filter_id,
    rtk_port_t      port,
    rtk_vlan_t      old_vid,
    rtk_vlan_t      new_vid,
    rtk_pri_t       new_pri)
{
    int32   ret;
    rtk_filter_flowTbl_t    filter_cfg;
    rtk_filter_action_t     action;  
    uint32  cutline;
    

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, filter_id=%d, port=%d\
           old_vid=%d, new_vid=%d, new_pri=%d", unit, filter_id, port, old_vid, new_vid, new_pri);  

    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(filter_id > HAL_PIE_FILTER_ID_MAX(unit), RT_ERR_FILTER_ENTRYIDX);
    /*Check if filter id is in the range of flow table*/
    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    RT_PARAM_CHK(filter_id >= cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit), RT_ERR_FILTER_FLOWTBL_RULENUM);    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((old_vid < RTK_VLAN_ID_MIN) || (old_vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    RT_PARAM_CHK((new_vid < RTK_VLAN_ID_MIN) || (new_vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    RT_PARAM_CHK(new_pri >= RTK_MAX_NUM_OF_PRIORITY, RT_ERR_OUT_OF_RANGE);

    /* Check the owner of this entry */
    RT_PARAM_CHK(!FILTERINFO_OWNER_CHK(unit, filter_id, FILTER_NO_OWNER), RT_ERR_FILTER_FLOWTBL_EXIST);

    /* Setting the flow-table parameters */
    osal_memset(&filter_cfg, 0, sizeof(rtk_filter_flowTbl_t));
    osal_memset(&action, 0, sizeof(rtk_filter_action_t));
    filter_cfg.valid = TRUE;
    filter_cfg.slp = port;
    filter_cfg.care_slp = 0x1f;
    filter_cfg.pktcvid = old_vid;
    filter_cfg.care_pktcvid = 0xfff;
    action.actGroup = 0x4;   /* Assign VLAN */
    action.un.assignVlan.actvid = new_vid;
    action.un.assignVlan.actpri = new_pri;

    /* Set this entry */
    FILTER_SEM_LOCK(unit);
    
    if ((ret = _dal_ssw_flowTblEntry_set(unit, filter_id, &filter_cfg, &action)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* Fixup the owner of this entry */
    FILTERINFO_OWNER_SET(unit, filter_id, IGR_VLAN_XLATE);
    
    FILTER_SEM_UNLOCK(unit);
        
    return RT_ERR_OK;
} /* end of dal_ssw_filter_igrVlanXlate_add*/


/* Function Name:
 *      dal_ssw_filter_igrVlanXlate_del
 * Description:
 *      Delete the ingress vlan translate mapping of the port from the specified device.
 * Input:
 *      unit      - unit id
 *      filter_id - filter id
 *      port      - port id
 *      old_vid   - old vlan id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_PORT_ID                - invalid port id
 *      RT_ERR_FILTER_FLOWTBL_EMPTY   - flow table entry is empty
 *      RT_ERR_FILTER_ENTRYIDX        - invalid entry index
 *      RT_ERR_FILTER_FLOWTBL_RULENUM - invalid flow table rulenum
 *      RT_ERR_FILTER_FLOWTBL_TYPE    - entry type isn't a flow table rule
 *      RT_ERR_VLAN_VID               - invalid vid
 * Note:
 *      None
 */
int32
dal_ssw_filter_igrVlanXlate_del(
    uint32          unit,
    rtk_filter_id_t filter_id,
    rtk_port_t      port,
    rtk_vlan_t      old_vid)
{
    int32   ret;
    rtk_filter_flowTbl_t    filter_cfg;
    rtk_filter_action_t     action;    
    uint32  cutline;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, filter_id=%d, port=%d\
           old_vid=%d", unit, filter_id, port, old_vid);  

    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(filter_id > HAL_PIE_FILTER_ID_MAX(unit), RT_ERR_FILTER_ENTRYIDX);
    /*Check if filter id is in the range of flow table*/
    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    RT_PARAM_CHK(filter_id >= cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit), RT_ERR_FILTER_FLOWTBL_RULENUM);    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((old_vid < RTK_VLAN_ID_MIN) || (old_vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    /* Check the owner of this entry */
    RT_PARAM_CHK(FILTERINFO_OWNER_CHK(unit, filter_id, FILTER_NO_OWNER), RT_ERR_FILTER_FLOWTBL_EMPTY);
    RT_PARAM_CHK(!FILTERINFO_OWNER_CHK(unit, filter_id, IGR_VLAN_XLATE), RT_ERR_FAILED);

    FILTER_SEM_LOCK(unit);
    /* Get this entry */
    if ((ret = _dal_ssw_flowTblEntry_get(unit, filter_id, &filter_cfg, &action)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    if ((filter_cfg.slp != port) || (filter_cfg.pktcvid != old_vid))
    {
        FILTER_SEM_UNLOCK(unit);
        return RT_ERR_FAILED;
    }        

    /* Delete this entry */
    if ((ret = _dal_ssw_filterEntry_del(unit, filter_id, IGR_VLAN_XLATE)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    FILTER_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_filter_igrVlanXlate_del */


/* Function Name:
 *      dal_ssw_filter_igrVlanXlate_delAll
 * Description:
 *      Delete all ingress vlan translate from the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 * Note:
 *      None
 */
int32
dal_ssw_filter_igrVlanXlate_delAll(uint32 unit)
{
    int32  ret;
    uint32 index, min_idx, max_idx;
    uint32 cutline;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d", unit);  

    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    min_idx = 0;
    max_idx = cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit) - 1;
    
    FILTER_SEM_LOCK(unit);
    /* Delete all igr-vlan-xlate entry */
    for (index = min_idx; index <= max_idx; index++)
    {
        if (FILTERINFO_OWNER_CHK(unit, index, IGR_VLAN_XLATE))
        {
            /* Delete this entry */
            if ((ret = _dal_ssw_filterEntry_del(unit, index, IGR_VLAN_XLATE)) != RT_ERR_OK)
            {
                FILTER_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
                return ret;
            }
        }
    }

    FILTER_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_filter_igrVlanXlate_delAll */



/* Module Name     : filter                             */
/* Sub-module Name : egress vlan translate by acl table */

/* Function Name:
 *      dal_ssw_filter_egrVlanXlate_add
 * Description:
 *      Add the egress vlan translate mapping of the port to the specified device.
 * Input:
 *      unit      - unit id
 *      filter_id - filter id
 *      port      - port id
 *      old_vid   - old vlan id
 *      new_vid   - new vlan id
 *      new_pri   - new priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT           - The module is not initial
 *      RT_ERR_UNIT_ID            - invalid unit id
 *      RT_ERR_PORT_ID            - invalid port id
 *      RT_ERR_FILTER_INACL_EXIST - ACL entry is already exit
 *      RT_ERR_FILTER_INACL_TYPE  - entry type isn't an ingress ACL rule
 *      RT_ERR_FILTER_ENTRYIDX    - invalid entry index
 *      RT_ERR_VLAN_VID           - invalid vid
 *      RT_ERR_OUT_OF_RANGE       - input parameter out of range
 * Note:
 *      1. The incoming packet in the port which match the old_vid will use the
 *         configure new_vid and new_pri for egress pipeline
 */
int32
dal_ssw_filter_egrVlanXlate_add(
    uint32          unit,
    rtk_filter_id_t filter_id,
    rtk_port_t      port,
    rtk_vlan_t      old_vid,
    rtk_vlan_t      new_vid,
    rtk_pri_t       new_pri)
{
    int32   ret;
    rtk_filter_aclCfg_t     filter_cfg;
    rtk_filter_action_t     action;    
    uint32  cutline;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, filter_id=%d, port=%d\
           old_vid=%d, new_vid=%d, new_pri=%d", unit, filter_id, port, old_vid, new_vid, new_pri);

    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(filter_id > HAL_PIE_FILTER_ID_MAX(unit), RT_ERR_FILTER_ENTRYIDX);
    /*Check if filter id is in the range of acl table*/
    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    RT_PARAM_CHK(filter_id < cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit), RT_ERR_FILTER_INACL_RULENUM);    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((old_vid < RTK_VLAN_ID_MIN) || (old_vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    RT_PARAM_CHK((new_vid < RTK_VLAN_ID_MIN) || (new_vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    RT_PARAM_CHK(new_pri >= RTK_MAX_NUM_OF_PRIORITY, RT_ERR_OUT_OF_RANGE);

    /* Check the owner of this entry */
    RT_PARAM_CHK(!FILTERINFO_OWNER_CHK(unit, filter_id, FILTER_NO_OWNER), RT_ERR_FILTER_INACL_EXIST);

    /* Setting the acl-table parameters */
    osal_memset(&filter_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    osal_memset(&action, 0, sizeof(rtk_filter_action_t));
    filter_cfg.valid = TRUE;
    filter_cfg.slp = port;
    filter_cfg.care_slp = 0x1f;
    filter_cfg.rvid = old_vid;
    filter_cfg.care_rvid = 0xfff;
    action.actGroup = 0x6;   /* New CVID */
    action.un.newcvid.replacecvid = 1;
    action.un.newcvid.replacecpri = 1;
    action.un.newcvid.cvid = new_vid;
    action.un.newcvid.cpri = new_pri;

    /* Set this entry */
    FILTER_SEM_LOCK(unit);
    
    if ((ret = _dal_ssw_aclEntry_set(unit, filter_id, &filter_cfg, &action)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* Fixup the owner of this entry */
    FILTERINFO_OWNER_SET(unit, filter_id, EGR_VLAN_XLATE);

    FILTER_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_filter_egrVlanXlate_add */


/* Function Name:
 *      dal_ssw_filter_egrVlanXlate_del
 * Description:
 *      Delete the egress vlan translate mapping of the port from the specified device.
 * Input:
 *      unit      - unit id
 *      filter_id - filter id
 *      port      - port id
 *      old_vid   - old vlan id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_FILTER_INACL_EMPTY   - ACL entry is empty
 *      RT_ERR_FILTER_ENTRYIDX      - invalid entry index
 *      RT_ERR_FILTER_INACL_RULENUM - invalid ACL rulenum
 *      RT_ERR_FILTER_INACL_TYPE    - entry type isn't an ingress ACL rule
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_OUT_OF_RANGE         - input parameter out of range
 * Note:
 *      None
 */
int32
dal_ssw_filter_egrVlanXlate_del(
    uint32          unit,
    rtk_filter_id_t filter_id,
    rtk_port_t      port,
    rtk_vlan_t      old_vid)
{
    int32   ret;
    rtk_filter_aclCfg_t     filter_cfg;
    rtk_filter_action_t     action;   
    uint32  cutline;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, filter_id=%d, port=%d\
           old_vid=%d", unit, filter_id, port, old_vid);  

    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(filter_id > HAL_PIE_FILTER_ID_MAX(unit), RT_ERR_FILTER_ENTRYIDX);
    /*Check if filter id is in the range of acl table*/
    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    RT_PARAM_CHK(filter_id < cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit), RT_ERR_FILTER_INACL_RULENUM);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((old_vid < RTK_VLAN_ID_MIN) || (old_vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    /* Check the owner of this entry */
    RT_PARAM_CHK(FILTERINFO_OWNER_CHK(unit, filter_id, FILTER_NO_OWNER), RT_ERR_FILTER_INACL_EMPTY);
    RT_PARAM_CHK(!FILTERINFO_OWNER_CHK(unit, filter_id, EGR_VLAN_XLATE), RT_ERR_FAILED);

    FILTER_SEM_LOCK(unit);
    
    /* Get this entry */
    if ((ret = _dal_ssw_aclEntry_get(unit, filter_id, &filter_cfg, &action)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    if ((filter_cfg.slp != port) || (filter_cfg.rvid != old_vid))
    {
        FILTER_SEM_UNLOCK(unit);
        return RT_ERR_FAILED;
    }

    /* Delete this entry */
    if ((ret = _dal_ssw_filterEntry_del(unit, filter_id, EGR_VLAN_XLATE)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    FILTER_SEM_UNLOCK(unit);
    return RT_ERR_OK;
} /* end of dal_ssw_filter_egrVlanXlate_del */


/* Function Name:
 *      dal_ssw_filter_egrVlanXlate_delAll
 * Description:
 *      Delete all egress vlan translate from the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 * Note:
 *      None
 */
int32
dal_ssw_filter_egrVlanXlate_delAll(uint32 unit)
{
    int32  ret;
    uint32 index, min_idx, max_idx;
    uint32 cutline;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d", unit);  

    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    min_idx = cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit);
    max_idx = HAL_PIE_FILTER_ID_MAX(unit);
    
    FILTER_SEM_LOCK(unit);
    /* Delete all igr-vlan-xlate entry */
    for (index = min_idx; index <= max_idx; index++)
    {
        if (FILTERINFO_OWNER_CHK(unit, index, EGR_VLAN_XLATE))
        {
            if ((ret = _dal_ssw_filterEntry_del(unit, index, EGR_VLAN_XLATE)) != RT_ERR_OK)
            {
                FILTER_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
                return ret;
            }
        }
    }    
    
    FILTER_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_filter_egrVlanXlate_delAll */



/* Module Name     : filter                          */
/* Sub-module Name : vlan double tagged by acl table */

/* Function Name:
 *      dal_ssw_filter_stagVlan_add
 * Description:
 *      Add the service-TAG with service vid and priority when match customer
 *      vid to the specified device.
 * Input:
 *      unit         - unit id
 *      filter_id    - filter id
 *      port         - port id
 *      customer_vid - customer (inner) vlan id
 *      service_vid  - service (outer) vlan id
 *      service_pri  - service (outer) priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_FILTER_INACL_EXIST   - ACL entry is already exit
 *      RT_ERR_FILTER_ENTRYIDX      - invalid entry index
 *      RT_ERR_FILTER_INACL_RULENUM - invalid ACL rulenum
 *      RT_ERR_FILTER_INACL_TYPE    - entry type isn't an ingress ACL rule
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_OUT_OF_RANGE         - input parameter out of range
 * Note:
 *      1. The incoming packet in the port which match the customer_vid will add
 *         one service-TAG with service_vid and service_pri for egress pipeline
 */
int32
dal_ssw_filter_stagVlan_add(
    uint32          unit,
    rtk_filter_id_t filter_id,
    rtk_port_t      port,
    rtk_vlan_t      customer_vid,
    rtk_vlan_t      service_vid,
    rtk_pri_t       service_pri)
{
    int32   ret;
    rtk_filter_aclCfg_t     filter_cfg;
    rtk_filter_action_t     action;    
    uint32  cutline;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, filter_id=%d, port=%d\
           customer_vid=%d, service_vid=%d, service_pri=%d", unit, filter_id, port, customer_vid, service_vid, service_pri);

    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(filter_id > HAL_PIE_FILTER_ID_MAX(unit), RT_ERR_FILTER_ENTRYIDX);
    /*Check if filter id is in the range of acl table*/
    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    RT_PARAM_CHK(filter_id < cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit), RT_ERR_FILTER_INACL_RULENUM);   
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((customer_vid < RTK_VLAN_ID_MIN) || (customer_vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    RT_PARAM_CHK((service_vid < RTK_VLAN_ID_MIN) || (service_vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    RT_PARAM_CHK(service_pri >= RTK_MAX_NUM_OF_PRIORITY, RT_ERR_OUT_OF_RANGE);

    /* Check the owner of this entry */
    RT_PARAM_CHK(!FILTERINFO_OWNER_CHK(unit, filter_id, FILTER_NO_OWNER), RT_ERR_FILTER_INACL_EXIST);

    /* Setting the flow-table parameters */
    osal_memset(&filter_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    osal_memset(&action, 0, sizeof(rtk_filter_action_t));
    filter_cfg.valid = TRUE;
    filter_cfg.slp = port;
    filter_cfg.care_slp = 0x1f;
    filter_cfg.rvid = customer_vid;
    filter_cfg.care_rvid = 0xfff;
    action.actGroup = 0x5;   /* New SVID */
    action.un.newsvid.svid = service_vid;

    /* Set this entry */
    FILTER_SEM_LOCK(unit);
    
    if ((ret = _dal_ssw_aclEntry_set(unit, filter_id, &filter_cfg, &action)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* Fixup the owner of this entry */
    FILTERINFO_OWNER_SET(unit, filter_id, STAG_VLAN);

    FILTER_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_filter_stagVlan_add*/


/* Function Name:
 *      dal_ssw_filter_stagVlan_del
 * Description:
 *      Delete the double tagged configure of the port from the specified device.
 * Input:
 *      unit      - unit id
 *      filter_id - filter id
 *      port      - port id
 *      vid       - vlan id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_FILTER_INACL_EMPTY   - ACL entry is empty
 *      RT_ERR_FILTER_ENTRYIDX      - invalid entry index
 *      RT_ERR_FILTER_INACL_RULENUM - invalid ACL rulenum
 *      RT_ERR_VLAN_VID             - invalid vid
 * Note:
 *      None
 */
int32
dal_ssw_filter_stagVlan_del(uint32 unit, rtk_filter_id_t filter_id, rtk_port_t port, rtk_vlan_t vid)
{
    int32   ret;
    rtk_filter_aclCfg_t     filter_cfg;
    rtk_filter_action_t     action;   
    uint32  cutline;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, filter_id=%d, port=%d, vid=%d", 
           unit, filter_id, port, vid);

    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(filter_id > HAL_PIE_FILTER_ID_MAX(unit), RT_ERR_FILTER_ENTRYIDX);
    /*Check if filter id is in the range of acl table*/
    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    RT_PARAM_CHK(filter_id < cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit), RT_ERR_FILTER_INACL_RULENUM);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((vid < RTK_VLAN_ID_MIN) || (vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    /* Check the owner of this entry */
    RT_PARAM_CHK(FILTERINFO_OWNER_CHK(unit, filter_id, FILTER_NO_OWNER), RT_ERR_FILTER_INACL_EMPTY);
    RT_PARAM_CHK(!FILTERINFO_OWNER_CHK(unit, filter_id, STAG_VLAN), RT_ERR_FAILED);

    FILTER_SEM_LOCK(unit);
    
    /* Get this entry */
    if ((ret = _dal_ssw_aclEntry_get(unit, filter_id, &filter_cfg, &action)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    if ((filter_cfg.slp != port) || (filter_cfg.rvid != vid))
    {
        FILTER_SEM_UNLOCK(unit);
        return RT_ERR_FAILED;
    }

    /* Delete this entry */

    if ((ret = _dal_ssw_filterEntry_del(unit, filter_id, STAG_VLAN)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    FILTER_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_filter_stagVlan_del */


/* Function Name:
 *      dal_ssw_filter_stagVlan_delAll
 * Description:
 *      Delete all the service-TAG configures from the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 * Note:
 *      None
 */
int32
dal_ssw_filter_stagVlan_delAll(uint32 unit)
{
    int32  ret;
    uint32 index, min_idx, max_idx;
    uint32 cutline;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d", unit);

    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    min_idx = cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit);
    max_idx = HAL_PIE_FILTER_ID_MAX(unit);
    
    FILTER_SEM_LOCK(unit);
    
    /* Delete all igr-vlan-xlate entry */
    for (index = min_idx; index <= max_idx; index++)
    {
        if (FILTERINFO_OWNER_CHK(unit, index, STAG_VLAN))
        {
            if ((ret = _dal_ssw_filterEntry_del(unit, index, STAG_VLAN)) != RT_ERR_OK)
            {
                FILTER_SEM_UNLOCK(unit);
                return ret;
            }
        }
    }
    
    FILTER_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_filter_stagVlan_delAll */



/* Module Name     : filter                             */
/* Sub-module Name : ip-subnet-based vlan by flow table */

/* Function Name:
 *      dal_ssw_filter_ipSubnetBasedVlan_add
 * Description:
 *      Add the source ip subnet-based vlan to the specified device.
 * Input:
 *      unit      - unit id
 *      filter_id - filter id
 *      ipaddr    - ip address
 *      netmask   - netmask
 *      vid       - vlan id
 *      pri       - priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_FILTER_FLOWTBL_EXIST   - flow table entry is already exist
 *      RT_ERR_FILTER_ENTRYIDX        - invalid entry index
 *      RT_ERR_FILTER_FLOWTBL_RULENUM - invalid flow table rulenum
 *      RT_ERR_VLAN_VID               - invalid vid
 *      RT_ERR_OUT_OF_RANGE           - input parameter out of range
 * Note:
 *      The incoming packet which match the source ip subnet will use the
 *      configure vid and priority for ingress pipeline
 */
int32
dal_ssw_filter_ipSubnetBasedVlan_add(
    uint32          unit,
    rtk_filter_id_t filter_id,
    ipaddr_t        ipaddr,
    ipaddr_t        netmask,
    rtk_vlan_t      vid,
    rtk_pri_t       pri)
{
    int32   ret;
    rtk_filter_flowTbl_t    filter_cfg;
    rtk_filter_action_t     action;    
    uint32  cutline;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, filter_id=%d, \
           ipaddr=%d, netmask=%x, vid=%d, pri=%d", unit, filter_id, ipaddr, netmask, vid, pri);

    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(filter_id > HAL_PIE_FILTER_ID_MAX(unit), RT_ERR_FILTER_ENTRYIDX);
    /*Check if filter id is in the range of flow table*/
    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    RT_PARAM_CHK(filter_id >= cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit), RT_ERR_FILTER_FLOWTBL_RULENUM);    
    /* [FIXME] check the source-mac */
    RT_PARAM_CHK((vid < RTK_VLAN_ID_MIN) || (vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    RT_PARAM_CHK(pri >= RTK_MAX_NUM_OF_PRIORITY, RT_ERR_OUT_OF_RANGE);

    /* Check the owner of this entry */
    RT_PARAM_CHK(!FILTERINFO_OWNER_CHK(unit, filter_id, FILTER_NO_OWNER), RT_ERR_FILTER_FLOWTBL_EXIST);
 
    /* Setting the flow-table parameters */
    osal_memset(&filter_cfg, 0, sizeof(rtk_filter_flowTbl_t));
    osal_memset(&action, 0, sizeof(rtk_filter_action_t));
    filter_cfg.valid = TRUE;
    filter_cfg.sip = ipaddr;
    filter_cfg.care_sip = netmask;
    action.actGroup = 0x4;   /* Assign VLAN */
    action.un.assignVlan.actvid = vid;
    action.un.assignVlan.actpri = pri;

    FILTER_SEM_LOCK(unit);
    
    if ((ret = _dal_ssw_flowTblEntry_set(unit, filter_id, &filter_cfg, &action)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* Fixup the owner of this entry */
    FILTERINFO_OWNER_SET(unit, filter_id, IP_SUBNET_BASED_VLAN);

    FILTER_SEM_UNLOCK(unit);
        
    return RT_ERR_OK;
} /* end of dal_ssw_filter_ipSubnetBasedVlan_add*/


/* Function Name:
 *      dal_ssw_filter_ipSubnetBasedVlan_del
 * Description:
 *      Delete the source ip subnet-based vlan from the specified device.
 * Input:
 *      unit      - unit id
 *      filter_id - filter id
 *      ipaddr    - ip address
 *      netmask   - netmask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_FILTER_FLOWTBL_EMPTY   - flow table entry is empty
 *      RT_ERR_FILTER_ENTRYIDX        - invalid entry index
 *      RT_ERR_FILTER_FLOWTBL_RULENUM - invalid flow table rulenum
 *      RT_ERR_FILTER_FLOWTBL_TYPE    - entry type isn't a flow table rule
 * Note:
 *      None
 */
int32
dal_ssw_filter_ipSubnetBasedVlan_del(
    uint32          unit,
    rtk_filter_id_t filter_id,
    ipaddr_t        ipaddr,
    ipaddr_t        netmask)
{
    int32   ret;
    rtk_filter_flowTbl_t    filter_cfg;
    rtk_filter_action_t     action;    
    uint32  cutline;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, filter_id=%d, \
           ipaddr=%d, netmask=%x", unit, filter_id, ipaddr, netmask);

    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(filter_id > HAL_PIE_FILTER_ID_MAX(unit), RT_ERR_FILTER_ENTRYIDX);

    /*Check if filter id is in the range of flow table*/
    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    RT_PARAM_CHK(filter_id >= cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit), RT_ERR_FILTER_FLOWTBL_RULENUM);    

    /* Check the owner of this entry */
    RT_PARAM_CHK(FILTERINFO_OWNER_CHK(unit, filter_id, FILTER_NO_OWNER), RT_ERR_FILTER_FLOWTBL_EMPTY);
    RT_PARAM_CHK(!FILTERINFO_OWNER_CHK(unit, filter_id, IP_SUBNET_BASED_VLAN), RT_ERR_FAILED);

    FILTER_SEM_LOCK(unit);
    
    /* Get this entry */
    if ((ret = _dal_ssw_flowTblEntry_get(unit, filter_id, &filter_cfg, &action)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    if (((filter_cfg.sip & filter_cfg.care_sip) != (ipaddr & netmask)) || (filter_cfg.care_sip != netmask))
    {
        FILTER_SEM_UNLOCK(unit);
        return RT_ERR_FAILED;
    }

    /* Delete this entry */
    if ((ret = _dal_ssw_filterEntry_del(unit, filter_id, IP_SUBNET_BASED_VLAN)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    FILTER_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_ssw_filter_ipSubnetBasedVlan_del */


/* Function Name:
 *      dal_ssw_filter_ipSubnetBasedVlan_delAll
 * Description:
 *      Delete all source ip subnet-based vlans from the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 * Note:
 *      None
 */
int32
dal_ssw_filter_ipSubnetBasedVlan_delAll(uint32 unit)
{
    int32  ret;
    uint32 index, min_idx, max_idx;
    uint32 cutline;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d", unit);

    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    min_idx = 0;
    max_idx = cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit) - 1;
    
    FILTER_SEM_LOCK(unit);
    
    /* Delete all ip-subnet-based vlan entry */
    for (index = min_idx; index <= max_idx; index++)
    {
        if (FILTERINFO_OWNER_CHK(unit, index, IP_SUBNET_BASED_VLAN))
        {
            /* Delete this entry */
            if ((ret = _dal_ssw_filterEntry_del(unit, index, IP_SUBNET_BASED_VLAN)) != RT_ERR_OK)
            {
                FILTER_SEM_UNLOCK(unit);
                return ret;
            }
        }
    }
    
    FILTER_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_filter_ipSubnetBasedVlan_delAll */



/* Module Name     : filter                                     */
/* Sub-module Name : protocol-and-port-based vlan by flow table */

/* Function Name:
 *      dal_ssw_filter_protoAndPortBasedVlan_add
 * Description:
 *      Add the protocol-and-port-based vlan to the specified port of device.
 * Input:
 *      unit            - unit id
 *      filter_id       - filter id
 *      port            - port id
 *      info.proto_type - protocol type
 *      info.frame_type - frame type
 *      info.cvid       - cvlan id
 *      info.cpri       - cvlan priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_PORT_ID                - invalid port id
 *      RT_ERR_FILTER_FLOWTBL_EXIST   - flow table entry is already exist
 *      RT_ERR_FILTER_ENTRYIDX        - invalid entry index
 *      RT_ERR_FILTER_FLOWTBL_RULENUM - invalid flow table rulenum
 *      RT_ERR_INPUT                  - invalid input parameter
 *      RT_ERR_CHIP_NOT_SUPPORTED     - functions not supported by this chip model
 *      RT_ERR_VLAN_VID               - invalid vid
 *      RT_ERR_OUT_OF_RANGE           - input parameter out of range
 * Note:
 *      1. The incoming packet which match the protocol-and-port-based vlan will use 
 *         the configure vid for ingress pipeline
 *      2. The frame type as following:
 *          - FRAME_TYPE_ETHERNET
 *          - FRAME_TYPE_RFC1042
 *          - FRAME_TYPE_SNAP8021H      (not support now)
 *          - FRAME_TYPE_SNAPOTHER      (not support now)
 *          - FRAME_TYPE_LLCOTHER
 */
int32
dal_ssw_filter_protoAndPortBasedVlan_add(
    uint32                      unit,
    rtk_filter_id_t             filter_id,
    rtk_port_t                  port,
    rtk_vlan_protoAndPortInfo_t info)
{
    int32   ret;
    rtk_filter_flowTbl_t    filter_cfg;
    rtk_filter_action_t     action;   
    uint32  cutline;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, filter_id=%d, \
           port=%d, info=%x", unit, filter_id, port, info);

    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);
    
    /* Check arguments */
    RT_PARAM_CHK(filter_id > HAL_PIE_FILTER_ID_MAX(unit), RT_ERR_FILTER_ENTRYIDX);
    /*Check if filter id is in the range of flow table*/
    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    RT_PARAM_CHK(filter_id >= cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit), RT_ERR_FILTER_FLOWTBL_RULENUM);    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(info.proto_type > 0xffff, RT_ERR_FAILED);
    RT_PARAM_CHK(info.frame_type >= FRAME_TYPE_END, RT_ERR_INPUT);
    RT_PARAM_CHK(info.frame_type == FRAME_TYPE_SNAP8021H, RT_ERR_CHIP_NOT_SUPPORTED);
    RT_PARAM_CHK(info.frame_type == FRAME_TYPE_SNAPOTHER, RT_ERR_CHIP_NOT_SUPPORTED);
    RT_PARAM_CHK((info.cvid < RTK_VLAN_ID_MIN) || (info.cvid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    RT_PARAM_CHK(info.cpri >= RTK_MAX_NUM_OF_PRIORITY, RT_ERR_OUT_OF_RANGE);

    /* Check the owner of this entry */
    RT_PARAM_CHK(!FILTERINFO_OWNER_CHK(unit, filter_id, FILTER_NO_OWNER), RT_ERR_FILTER_FLOWTBL_EXIST);

    /* Setting the flow-table parameters */
    osal_memset(&filter_cfg, 0, sizeof(rtk_filter_flowTbl_t));
    osal_memset(&action, 0, sizeof(rtk_filter_action_t));
    filter_cfg.valid = TRUE;
    filter_cfg.slp = port;
    filter_cfg.care_slp = 0x1f;
    filter_cfg.frametype = (info.frame_type == FRAME_TYPE_ETHERNET)? 0x0 : 
                         ((info.frame_type == FRAME_TYPE_RFC1042)? 0x1 : 0x2);
    filter_cfg.care_frametype = 0x3;
    filter_cfg.ethertype= info.proto_type & 0xffff;
    filter_cfg.care_ethertype = 0xffff;
    action.actGroup = 0x4;   /* Assign VLAN */
    action.un.assignVlan.actvid = info.cvid & 0xfff;
    action.un.assignVlan.actpri = info.cpri & 0x7;

    /* Set this entry */
    FILTER_SEM_LOCK(unit);
    
    if ((ret = _dal_ssw_flowTblEntry_set(unit, filter_id, &filter_cfg, &action)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* Fixup the owner of this entry */
    FILTERINFO_OWNER_SET(unit, filter_id, PROTO_AND_PORT_BASED_VLAN);
    
    FILTER_SEM_UNLOCK(unit);    
        
    return RT_ERR_OK;
} /* end of dal_ssw_filter_protoAndPortBasedVlan_add*/


/* Function Name:
 *      dal_ssw_filter_protoAndPortBasedVlan_del
 * Description:
 *      Delete the protocol-and-port-based vlan from the specified port of device.
 * Input:
 *      unit       - unit id
 *      filter_id  - filter id
 *      port       - port id
 *      proto_type - protocol type
 *      frame_type - frame type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_PORT_ID                - invalid port id
 *      RT_ERR_FILTER_FLOWTBL_EMPTY   - flow table entry is empty
 *      RT_ERR_FILTER_ENTRYIDX        - invalid entry index
 *      RT_ERR_FILTER_FLOWTBL_RULENUM - invalid flow table rulenum
 *      RT_ERR_FILTER_FLOWTBL_TYPE    - entry type isn't a flow table rule
 *      RT_ERR_INPUT                  - invalid input parameter
 *      RT_ERR_CHIP_NOT_SUPPORTED     - functions not supported by this chip model
 *      RT_ERR_VLAN_VID               - invalid vid
 * Note:
 *      1. The incoming packet which match the protocol-and-port-based vlan will use
 *         the configure vid for ingress pipeline
 *      2. The frame type as following:
 *          - FRAME_TYPE_ETHERNET
 *          - FRAME_TYPE_RFC1042
 *          - FRAME_TYPE_SNAP8021H      (not support now)
 *          - FRAME_TYPE_SNAPOTHER      (not support now)
 *          - FRAME_TYPE_LLCOTHER
 */
int32
dal_ssw_filter_protoAndPortBasedVlan_del(
    uint32                         unit,
    rtk_filter_id_t                filter_id,
    rtk_port_t                     port,
    uint32                         proto_type,
    rtk_vlan_protoVlan_frameType_t frame_type)
{
    int32   ret;
    rtk_filter_flowTbl_t    filter_cfg;
    rtk_filter_action_t     action;    
    uint32  cutline;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, filter_id=%d, \
           port=%d, proto_type=%d, frame_type=%d", unit, filter_id, port, proto_type, frame_type);

    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(filter_id > HAL_PIE_FILTER_ID_MAX(unit), RT_ERR_FILTER_ENTRYIDX);
    /*Check if filter id is in the range of flow table*/
    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    RT_PARAM_CHK(filter_id >= cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit), RT_ERR_FILTER_FLOWTBL_RULENUM);    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(proto_type > 0xffff, RT_ERR_FAILED);
    RT_PARAM_CHK(frame_type >= FRAME_TYPE_END, RT_ERR_INPUT);
    RT_PARAM_CHK(frame_type == FRAME_TYPE_SNAP8021H, RT_ERR_CHIP_NOT_SUPPORTED);
    RT_PARAM_CHK(frame_type == FRAME_TYPE_SNAPOTHER, RT_ERR_CHIP_NOT_SUPPORTED);

    /* Check the owner of this entry */
    RT_PARAM_CHK(FILTERINFO_OWNER_CHK(unit, filter_id, FILTER_NO_OWNER), RT_ERR_FILTER_FLOWTBL_EMPTY);
    RT_PARAM_CHK(!FILTERINFO_OWNER_CHK(unit, filter_id, PROTO_AND_PORT_BASED_VLAN), RT_ERR_FAILED);

    FILTER_SEM_LOCK(unit);
    
    /* Get this entry */
    if ((ret = _dal_ssw_flowTblEntry_get(unit, filter_id, &filter_cfg, &action)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    if ((filter_cfg.slp != port) || (filter_cfg.ethertype != proto_type))
    {
        FILTER_SEM_UNLOCK(unit);
        return RT_ERR_FAILED;
    }

    if (((frame_type == FRAME_TYPE_ETHERNET) && (filter_cfg.frametype != 0x0)) ||
        ((frame_type == FRAME_TYPE_RFC1042) && (filter_cfg.frametype != 0x1)) ||
        ((frame_type == FRAME_TYPE_LLCOTHER) && (filter_cfg.frametype != 0x2)))
    {
        FILTER_SEM_UNLOCK(unit);
        return RT_ERR_FAILED;
    }

    /* Delete this entry */
    if ((ret = _dal_ssw_filterEntry_del(unit, filter_id, PROTO_AND_PORT_BASED_VLAN)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    FILTER_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_filter_protoAndPortBasedVlan_del */


/* Function Name:
 *      dal_ssw_filter_protoAndPortBasedVlan_delAll
 * Description:
 *      Delete all protocol-and-port-based vlans from the specified port of device.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 * Note:
 *      1. The incoming packet which match the protocol-and-port-based vlan will use
 *         the configure vid for ingress pipeline
 *      2. Delete all flow table protocol-and-port-based vlan entries.
 */
int32
dal_ssw_filter_protoAndPortBasedVlan_delAll(uint32 unit, rtk_port_t port)
{
    int32  ret;
    rtk_filter_flowTbl_t    filter_cfg;
    rtk_filter_action_t     action;    
    uint32 index, min_idx, max_idx;
    uint32 cutline;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FILTER), "unit=%d, port=%d", 
           unit, port);

    /* Check init state */
    RT_INIT_CHK(filter_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);

    if ((ret = dal_ssw_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    min_idx = 0;
    max_idx = cutline * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit) - 1;
    
    FILTER_SEM_LOCK(unit);
    
    /* Delete all ip-subnet-based vlan entry */
    for (index = min_idx; index <= max_idx; index++)
    {
        if (FILTERINFO_OWNER_CHK(unit, index, PROTO_AND_PORT_BASED_VLAN))
        {
            /* Get this entry */
            if ((ret = _dal_ssw_flowTblEntry_get(unit, index, &filter_cfg, &action)) != RT_ERR_OK)
            {
                FILTER_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
                return ret;
            }

            if (filter_cfg.slp != port)
                continue;

            if ((ret = _dal_ssw_filterEntry_del(unit, index, PROTO_AND_PORT_BASED_VLAN)) != RT_ERR_OK)
            {
                FILTER_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
                return ret;
            }
        }
    }
    
    FILTER_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_ssw_filter_protoAndPortBasedVlan_delAll */



/* Internal Function Body */

/* Function Name:
 *      _dal_ssw_flowTblEntry_set
 * Description:
 *      Set flow table entry to chip.
 * Input:
 *      unit        - unit id
 *      filter_id   - filter id
 *      pFilter_cfg - content of filter entry
 *      pAction     - content of action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 */
static int32
_dal_ssw_flowTblEntry_set(uint32 unit, rtk_filter_id_t filter_id, rtk_filter_flowTbl_t *pFilter_cfg, rtk_filter_action_t *pAction)
{
    int32   ret;
    pie89_entry_t    pie89_entry;
    uint32  temp_var;
    uint32  table_index;
    uint8   temp_mac[ETHER_ADDR_LEN];
    
    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_FILTER), "unit=%d, filter_id=%d, pFilter_cfg=%x, pAction=%x", 
           unit, filter_id, pFilter_cfg, pAction);    
    
    osal_memset(&pie89_entry, 0, sizeof(pie89_entry_t));
    
    /*translate filter-id to table index*/
    table_index = ((filter_id / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 7) + (filter_id % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));
    
    /* set DMAC */
    osal_memcpy(temp_mac, pFilter_cfg->dmac, ETHER_ADDR_LEN);
    if ((ret = table_field_mac_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_DMACf, temp_mac, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* set SMAC */
    osal_memcpy(temp_mac, pFilter_cfg->smac, ETHER_ADDR_LEN);
    if ((ret = table_field_mac_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_SMACf, temp_mac, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* set DIP */
    temp_var = pFilter_cfg->dip;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_DIPf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* set SIP */
    temp_var = pFilter_cfg->sip;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_SIPf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* set FLOWLABEL */
    temp_var = pFilter_cfg->flowlabel;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_FLOWLABELf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* set PKTCVID */
    temp_var = pFilter_cfg->pktcvid;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_PKTCVIDf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }    
    /* set SSW_PIE89_TABLE_RVID ???*/
    
    /* set DSTPORT */
    temp_var = pFilter_cfg->dstport;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_DSTPORTf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* set SRCPORT */
    temp_var = pFilter_cfg->srcport;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_SRCPORTf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* set ETHTYPE */
    temp_var = pFilter_cfg->ethertype;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_ETHTYPEf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* set IPPROTO */
    temp_var = pFilter_cfg->ipproto;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_IPPROTOf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* set TOS */
    temp_var = pFilter_cfg->tos;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_TOSf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* set PKTCPRI */
    temp_var = pFilter_cfg->pktcpri;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_PKTCPRIf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* set SLP */
    temp_var = pFilter_cfg->slp;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_FLOWTABLE_SLPf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* set FRAMETYPE */
    temp_var = pFilter_cfg->frametype;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_FLOWTABLE_FRAMETYPEf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* set PKTTAGIF */
    temp_var = pFilter_cfg->pktctagif;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_PKTTAGIFf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    } 

    /* set IPV6MLD */
    temp_var = pFilter_cfg->ipv6mld;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_FLOWTABLE_IPV6MLDf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* set IPV6 */
    temp_var = pFilter_cfg->ipv6;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_FLOWTABLE_IPV6f, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }   
    
    /* set IPV4 */
    temp_var = pFilter_cfg->ipv4;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_FLOWTABLE_IPV4f, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }   

    /* set PKTSTAGIF */
    temp_var = pFilter_cfg->pktstagif;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_PKTSTAGIFf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }  

    /* set PKTSPRI */
    temp_var = pFilter_cfg->pktspri;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_PKTSPRIf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }  

    /* set PKTSVID */
    temp_var = pFilter_cfg->pktsvid;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_FLOWTABLE_PKTSVIDf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }  

    /* set ACTGROUP */
    temp_var = pAction->actGroup;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_ACTGROUPf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }  
    
    if (/*0b100*/ 4 == pAction->actGroup)/*Assign VLAN*/
    {
        /* set SSW_PIE89_TABLE_SRAM_17B */
        temp_var = 0;
        temp_var = (pAction->un.assignVlan.actpri & 0x7) ;/*bit0~bit2*/
        temp_var = temp_var | ((pAction->un.assignVlan.actvid & 0xfff) << 3);/*bit3~bit14*/
        temp_var = temp_var | ((pAction->un.assignVlan.usepktctag & 0x1) << 15);/*bit15*/
        temp_var = temp_var | ((pAction->un.assignVlan.usepktspri & 0x1) << 16);/*bit16*/
        if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_SRAM_17Bf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
            return ret;
        }
    }
    
    if (/*0b000*/ 0 == pAction->actGroup)/*Log*/
    {  
        /* set SSW_PIE89_TABLE_SRAM_7B */
        temp_var = pAction->un.log.logindex;
        if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_SRAM_7Bf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
            return ret;
        }
    }
    
    /* set VALID */
    temp_var = pFilter_cfg->valid;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_VALIDf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* set CARE_DMAC */
    osal_memcpy(temp_mac, pFilter_cfg->care_dmac, ETHER_ADDR_LEN);
    if ((ret = table_field_mac_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_DMACf, temp_mac, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* set CARE_SMAC */
    osal_memcpy(temp_mac, pFilter_cfg->care_smac, ETHER_ADDR_LEN);
    if ((ret = table_field_mac_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_SMACf, temp_mac, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* set CARE_DIP */
    temp_var = pFilter_cfg->care_dip;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_DIPf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* set CARE_SIP */
    temp_var = pFilter_cfg->care_sip;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_SIPf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* set CARE_FLOWLABE */
    temp_var = pFilter_cfg->care_flowlabel;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_FLOWLABELf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* set CARE_PKTCVID */
    temp_var = pFilter_cfg->care_pktcvid;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_PKTCVIDf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }    
    /* set SSW_PIE89_TABLE_CARE_RVID ???*/
    
    /* set CARE_DSTPORT */
    temp_var = pFilter_cfg->care_dstport;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_DSTPORTf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* set CARE_SRCPORT */
    temp_var = pFilter_cfg->care_srcport;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_SRCPORTf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* set CARE_ETHTYPE */
    temp_var = pFilter_cfg->care_ethertype;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_ETHTYPEf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* set CARE_IPPROTO */
    temp_var = pFilter_cfg->care_ipproto;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_IPPROTOf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* set CARE_TOS */
    temp_var = pFilter_cfg->care_tos;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_TOSf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* set CARE_PKTCPRI */
    temp_var = pFilter_cfg->care_pktcpri;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_PKTCPRIf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* set CARE_FLOWTABLE_SLP */
    temp_var = pFilter_cfg->care_slp;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_FLOWTABLE_SLPf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* set CARE_FLOWTABLE_FRAMETYPE */
    temp_var = pFilter_cfg->care_frametype;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_FLOWTABLE_FRAMETYPEf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* set CARE_PKTTAGIF */
    temp_var = pFilter_cfg->care_pktctagif;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_PKTTAGIFf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    } 

    /* set CARE_FLOWTABLE_IPV6MLD */
    temp_var = pFilter_cfg->care_ipv6mld;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_FLOWTABLE_IPV6MLDf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* set CARE_FLOWTABLE_IPV6 */
    temp_var = pFilter_cfg->care_ipv6;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_FLOWTABLE_IPV6f, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }   
    
    /* set CARE_FLOWTABLE_IPV4 */
    temp_var = pFilter_cfg->care_ipv4;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_FLOWTABLE_IPV4f, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }   

    /* set CARE_PKTSTAGIF */
    temp_var = pFilter_cfg->care_pktstagif;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_PKTSTAGIFf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }  

    /* set CARE_PKTSPRI */
    temp_var = pFilter_cfg->care_pktspri;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_PKTSPRIf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }  

    /* set CARE_FLOWTABLE_PKTSVID */
    temp_var = pFilter_cfg->care_pktsvid;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_FLOWTABLE_PKTSVIDf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }  

    /* programming flow table entry in chip */
    if ((ret = table_write(unit, SSW_PIE89_TABLEt, table_index, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of _dal_ssw_flowTblEntry_set */

/* Function Name:
 *      _dal_ssw_flowTblEntry_get
 * Description:
 *      Get flow table entry from chip.
 * Input:
 *      unit        - unit id
 *      filter_id   - filter id
 * Output:
 *      pFilter_cfg - content of flow table entry
 *      pAction     - content of action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 */
static int32
_dal_ssw_flowTblEntry_get(uint32 unit, rtk_filter_id_t filter_id, rtk_filter_flowTbl_t *pFilter_cfg, rtk_filter_action_t *pAction)
{
    int32   ret;
    pie89_entry_t    pie89_entry;
    uint32  temp_var;
    uint32  table_index;
    uint8   temp_mac[ETHER_ADDR_LEN];

    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_FILTER), "unit=%d, filter_id=%d, pFilter_cfg=%x, pAction=%x", 
           unit, filter_id, pFilter_cfg, pAction);  

    osal_memset(&pie89_entry, 0, sizeof(pie89_entry_t));

    /*translate filter-id to table index*/
    table_index = ((filter_id / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 7) + (filter_id % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));
    
    /* get entry from chip */
    if ((ret = table_read(unit, SSW_PIE89_TABLEt, table_index, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
        
    /* get DMAC */
    if ((ret = table_field_mac_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_DMACf, temp_mac, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    osal_memcpy(pFilter_cfg->dmac, temp_mac, ETHER_ADDR_LEN);
    
    /* get SMAC */
    if ((ret = table_field_mac_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_SMACf, temp_mac, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    osal_memcpy(pFilter_cfg->smac, temp_mac, ETHER_ADDR_LEN);
    
    /* get DIP */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_DIPf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->dip = temp_var;
    
    /* get SIP */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_SIPf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->sip = temp_var;
    
    /* get FLOWLABEL */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_FLOWLABELf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->flowlabel = temp_var;
    
    /* get PKTCVID */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_PKTCVIDf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }    
    pFilter_cfg->pktcvid = temp_var;
    /* get SSW_PIE89_TABLE_RVID ???*/
    
    /* get DSTPORT */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_DSTPORTf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->dstport = temp_var;

    /* get SRCPORT */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_SRCPORTf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->srcport = temp_var;

    /* get ETHTYPE */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_ETHTYPEf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->ethertype = temp_var;

    /* get IPPROTO */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_IPPROTOf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->ipproto = temp_var;

    /* get TOS */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_TOSf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->tos = temp_var;

    /* get PKTCPRI */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_PKTCPRIf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->pktcpri = temp_var;
    
    /* get SLP */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_FLOWTABLE_SLPf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->slp = temp_var;
    
    /* get FRAMETYPE */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_FLOWTABLE_FRAMETYPEf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->frametype = temp_var;

    /* get PKTTAGIF */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_PKTTAGIFf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    } 
    pFilter_cfg->pktctagif = temp_var;

    /* get IPV6MLD */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_FLOWTABLE_IPV6MLDf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->ipv6mld = temp_var;
    
    /* get IPV6 */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_FLOWTABLE_IPV6f, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->ipv6 = temp_var;
    
    /* get IPV4 */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_FLOWTABLE_IPV4f, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->ipv4 = temp_var;

    /* get PKTSTAGIF */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_PKTSTAGIFf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }  
    pFilter_cfg->pktstagif = temp_var;

    /* get PKTSPRI */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_PKTSPRIf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }  
    pFilter_cfg->pktspri = temp_var;

    /* get PKTSVID */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_FLOWTABLE_PKTSVIDf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }  
    pFilter_cfg->pktsvid = temp_var;

    /* get ACTGROUP */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_ACTGROUPf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }  
    pAction->actGroup = temp_var;
    
    if (/*0b100*/ 4 == pAction->actGroup)/*Assign VLAN*/
    {
        /* get SSW_PIE89_TABLE_SRAM_17B */
        temp_var = 0;
        if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_SRAM_17Bf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
            return ret;
        }
        pAction->un.assignVlan.actpri = (temp_var & 0x7);/*bit0~bit2*/
        pAction->un.assignVlan.actvid = ((temp_var >> 3) & 0xfff);/*bit3~bit14*/
        pAction->un.assignVlan.usepktctag = ((temp_var >> 15) & 0x1);/*bit15*/
        pAction->un.assignVlan.usepktspri = ((temp_var >> 16) & 0x1);/*bit16*/
    }
    
    if (/*0b000*/ 0 == pAction->actGroup)/*Log*/
    {  
        /* get SSW_PIE89_TABLE_SRAM_7B */
        if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_SRAM_7Bf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
            return ret;
        }
        pAction->un.log.logindex = temp_var;
    }
    
    /* get VALID */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_VALIDf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->valid = temp_var;

    /* get CARE_DMAC */
    if ((ret = table_field_mac_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_DMACf, temp_mac, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    osal_memcpy(pFilter_cfg->care_dmac, temp_mac, ETHER_ADDR_LEN);
    
    /* get CARE_SMAC */
    if ((ret = table_field_mac_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_SMACf, temp_mac, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    osal_memcpy(pFilter_cfg->care_smac, temp_mac, ETHER_ADDR_LEN);
    
    /* get CARE_DIP */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_DIPf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->care_dip = temp_var;
    
    /* get CARE_SIP */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_SIPf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        return ret;
    }
    pFilter_cfg->care_sip = temp_var;
    
    /* get CARE_FLOWLABE */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_FLOWLABELf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->care_flowlabel = temp_var;
    
    /* get CARE_PKTCVID */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_PKTCVIDf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }    
    pFilter_cfg->care_pktcvid = temp_var;
    
    /* get SSW_PIE89_TABLE_CARE_RVID ???*/
    
    /* get CARE_DSTPORT */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_DSTPORTf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->care_dstport = temp_var;

    /* get CARE_SRCPORT */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_SRCPORTf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->care_srcport = temp_var;

    /* get CARE_ETHTYPE */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_ETHTYPEf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->care_ethertype = temp_var;

    /* get CARE_IPPROTO */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_IPPROTOf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->care_ipproto = temp_var;

    /* get CARE_TOS */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_TOSf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->care_tos = temp_var;

    /* get CARE_PKTCPRI */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_PKTCPRIf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->care_pktcpri = temp_var;
    
    /* get CARE_FLOWTABLE_SLP */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_FLOWTABLE_SLPf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->care_slp = temp_var;
    
    /* get CARE_FLOWTABLE_FRAMETYPE */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_FLOWTABLE_FRAMETYPEf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->care_frametype = temp_var;

    /* get CARE_PKTTAGIF */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_PKTTAGIFf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    } 
    pFilter_cfg->care_pktctagif = temp_var;

    /* get CARE_FLOWTABLE_IPV6MLD */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_FLOWTABLE_IPV6MLDf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->care_ipv6mld = temp_var;
    
    /* get CARE_FLOWTABLE_IPV6 */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_FLOWTABLE_IPV6f, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->care_ipv6 = temp_var;
    
    /* get CARE_FLOWTABLE_IPV4 */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_FLOWTABLE_IPV4f, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->care_ipv4 = temp_var;

    /* get CARE_PKTSTAGIF */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_PKTSTAGIFf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }  
    pFilter_cfg->care_pktstagif = temp_var;

    /* get CARE_PKTSPRI */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_PKTSPRIf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->care_pktspri = temp_var;

    /* get CARE_FLOWTABLE_PKTSVID */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_FLOWTABLE_PKTSVIDf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }  
    pFilter_cfg->care_pktsvid = temp_var;
 
    return RT_ERR_OK;
} /* end of _dal_ssw_flowTblEntry_get */


/* Function Name:
 *      _dal_ssw_aclEntry_get
 * Description:
 *      Get ACL entry from chip.
 * Input:
 *      unit        - unit id
 *      filter_id   - filter id
 * Output:
 *      pFilter_cfg - content of ACL entry
 *      pAction     - content of action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 */
static int32
_dal_ssw_aclEntry_get(uint32 unit, rtk_filter_id_t filter_id, rtk_filter_aclCfg_t *pFilter_cfg, rtk_filter_action_t *pAction)
{
    int32   ret;
    pie89_entry_t    pie89_entry;
    uint32  temp_var;
    uint32  table_index;
    uint8   temp_mac[ETHER_ADDR_LEN];

    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_FILTER), "unit=%d, filter_id=%d", unit, filter_id);

    osal_memset(&pie89_entry, 0, sizeof(pie89_entry_t));

    /*translate filter-id to table index*/
    table_index = ((filter_id / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 7) + (filter_id % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));
    
    /* get entry from chip */
    if ((ret = table_read(unit, SSW_PIE89_TABLEt, table_index, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
        
    /* get DMAC */
    if ((ret = table_field_mac_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_DMACf, temp_mac, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    osal_memcpy(pFilter_cfg->dmac, temp_mac, ETHER_ADDR_LEN);
    
    /* get SMAC */
    if ((ret = table_field_mac_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_SMACf, temp_mac, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    osal_memcpy(pFilter_cfg->smac, temp_mac, ETHER_ADDR_LEN);
    
    /* get DIP */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_DIPf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->dip = temp_var;
    
    /* get SIP */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_SIPf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->sip = temp_var;
    
    /* get FLOWLABEL */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_FLOWLABELf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->flowlabel = temp_var;
    
    /* get RVID */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_RVIDf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }    
    pFilter_cfg->rvid = temp_var;
       
    /* get DSTPORT */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_DSTPORTf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->dstport = temp_var;

    /* get SRCPORT */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_SRCPORTf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->srcport = temp_var;

    /* get ETHTYPE */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_ETHTYPEf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->ethertype = temp_var;

    /* get IPPROTO */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_IPPROTOf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->ipproto = temp_var;

    /* get TOS */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_TOSf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->tos = temp_var;

    /* get TCPFLAG */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_TCPFLAGf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->tcpflag = temp_var;
    
    /* get SLP */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_INACL_SLPf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->slp = temp_var;
    
    /* get FRAMETYPE */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_INACL_FRAMETYPEf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->frametype = temp_var;

    /* get IPV6MLD */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_INACL_IPV6MLDf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->ipv6mld = temp_var;
    
    /* get IPV6 */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_INACL_IPV6f, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->ipv6 = temp_var;
    
    /* get IPV4 */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_INACL_IPV4f, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->ipv4 = temp_var;

    /* get PATTERNMATCH */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_PATTERNMATCHf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        return ret;
    }  
    pFilter_cfg->patternmatch = temp_var;

    /* get PKTSVID */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_INACL_PKTSVIDf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }  
    pFilter_cfg->pktsvid = temp_var;

    /* get ACTGROUP */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_ACTGROUPf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }  
    pAction->actGroup = temp_var;

    temp_var = 0;
    if (/*0b000*/ 0 == pAction->actGroup)/*permit, drop, redirect, copy to cpu*/
    {  
        /* get SSW_PIE89_TABLE_SRAM_7B */
        if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_SRAM_7Bf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
            return ret;
        }
        
        pAction->un.permit_drop_redirect.acttype = ((temp_var>>5) & 0x3);
        if (0 == (pAction->un.permit_drop_redirect.acttype))
        {/*permit*/
            
        }
        else if (1 == (pAction->un.permit_drop_redirect.acttype))
        {/*drop*/
        
        } 
        else if (2 == (pAction->un.permit_drop_redirect.acttype))
        {/*redirect*/
            pAction->un.permit_drop_redirect.portid = (temp_var & 0x1f);
        } 
        else if (3 == (pAction->un.permit_drop_redirect.acttype))
        {/*copy to cpu*/
        
        }
        else
        {}
    }
    
    if (/*0b001*/ 1 == pAction->actGroup)/*Mirror*/
    {
        /* get SSW_PIE89_TABLE_SRAM_7B */
        if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_SRAM_7Bf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
            return ret;
        }
        pAction->un.mirror.mirrorsetid = temp_var;
    }
    
    if (/*0b010*/ 2 == pAction->actGroup)/*Log*/
    {
        /* get SSW_PIE89_TABLE_SRAM_7B */
        if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_SRAM_7Bf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
            return ret;
        }
        pAction->un.log.logindex = temp_var;
    }    
    
    if (/*0b100*/ 4 == pAction->actGroup)/*Ratelimit*/
    {
        /* get SSW_PIE89_TABLE_SRAM_17B */
        if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_SRAM_17Bf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
            return ret;
        }
        pAction->un.ratelimit.rateindex = (temp_var& 0x7f);
    }

    if (/*0b101*/ 5 == pAction->actGroup)/*New SVID*/
    {
        /* get SSW_PIE89_TABLE_SRAM_17B */
        if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_SRAM_17Bf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
            return ret;
        }
        pAction->un.newsvid.svid = (temp_var& 0xfff);
    }
    
    if (/*0b110*/ 6 == pAction->actGroup)/*New CVID*/
    {
        /* get SSW_PIE89_TABLE_SRAM_17B */
        if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_SRAM_17Bf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
            return ret;
        }
        pAction->un.newcvid.replacecvid = ((temp_var >> 16) & 0x1);
        pAction->un.newcvid.replacecpri = ((temp_var >> 15) & 0x1);
        pAction->un.newcvid.cvid = ((temp_var >> 3) & 0xfff);
        pAction->un.newcvid.cpri = (temp_var & 0x7);
    }
   
    /* get VALID */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_VALIDf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->valid = temp_var;

    /* get CARE_DMAC */
    if ((ret = table_field_mac_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_DMACf, temp_mac, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    osal_memcpy(pFilter_cfg->care_dmac, temp_mac, ETHER_ADDR_LEN);
    
    /* get CARE_SMAC */
    if ((ret = table_field_mac_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_SMACf, temp_mac, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    osal_memcpy(pFilter_cfg->care_smac, temp_mac, ETHER_ADDR_LEN);
    
    /* get CARE_DIP */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_DIPf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->care_dip = temp_var;
    
    /* get CARE_SIP */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_SIPf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->care_sip = temp_var;
    
    /* get CARE_FLOWLABE */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_FLOWLABELf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->care_flowlabel = temp_var;
    
    /* get CARE_RVID */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_RVIDf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }    
    pFilter_cfg->care_rvid = temp_var;
    
    /* get CARE_DSTPORT */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_DSTPORTf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->care_dstport = temp_var;

    /* get CARE_SRCPORT */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_SRCPORTf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->care_srcport = temp_var;

    /* get CARE_ETHTYPE */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_ETHTYPEf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->care_ethertype = temp_var;

    /* get CARE_IPPROTO */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_IPPROTOf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->care_ipproto = temp_var;

    /* get CARE_TOS */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_TOSf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->care_tos = temp_var;

    /* get CARE_TCPFLAG */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_TCPFLAGf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->care_tcpflag = temp_var;
    
    /* get CARE_FLOWTABLE_SLP */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_INACL_SLPf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->care_slp = temp_var;
    
    /* get CARE_FLOWTABLE_FRAMETYPE */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_INACL_FRAMETYPEf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->care_frametype = temp_var;

    /* get CARE_FLOWTABLE_IPV6MLD */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_INACL_IPV6MLDf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->care_ipv6mld = temp_var;
    
    /* get CARE_FLOWTABLE_IPV6 */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_INACL_IPV6f, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->care_ipv6 = temp_var;
    
    /* get CARE_FLOWTABLE_IPV4 */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_INACL_IPV4f, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    pFilter_cfg->care_ipv4 = temp_var;

    /* get CARE_PATTERNMATCH */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_PATTERNMATCHf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }  
    pFilter_cfg->care_patternmatch = temp_var;

    /* get CARE_FLOWTABLE_PKTSVID */
    if ((ret = table_field_get( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_INACL_PKTSVIDf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }  
    pFilter_cfg->care_pktsvid = temp_var;

    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_FILTER), "pFilter_cfg=%x, pAction=%x", pFilter_cfg, pAction);
 
    return RT_ERR_OK;
} /* end of _dal_ssw_aclEntry_get */

/* Function Name:
 *      _dal_ssw_aclEntry_set
 * Description:
 *      Set flow ACL to chip.
 * Input:
 *      unit        - unit id
 *      filter_id   - filter id
 *      pFilter_cfg - content of filter entry
 *      pAction     - content of action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 */
static int32
_dal_ssw_aclEntry_set(uint32 unit, rtk_filter_id_t filter_id, rtk_filter_aclCfg_t *pFilter_cfg, rtk_filter_action_t *pAction)
{
    int32   ret;
    pie89_entry_t    pie89_entry;
    uint32  temp_var;
    uint32  table_index;
    uint8   temp_mac[ETHER_ADDR_LEN];
    
    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_FILTER), "unit=%d, filter_id=%d, pFilter_cfg=%x, pAction=%x", 
           unit, filter_id, pFilter_cfg, pAction);  
    
    osal_memset(&pie89_entry, 0, sizeof(pie89_entry_t));
    
    /*translate filter-id to table index*/
    table_index = ((filter_id / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 7) + (filter_id % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));
    
    /* set DMAC */
    osal_memcpy(temp_mac, pFilter_cfg->dmac, ETHER_ADDR_LEN);
    if ((ret = table_field_mac_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_DMACf, temp_mac, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* set SMAC */
    osal_memcpy(temp_mac, pFilter_cfg->smac, ETHER_ADDR_LEN);
    if ((ret = table_field_mac_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_SMACf, temp_mac, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* set DIP */
    temp_var = pFilter_cfg->dip;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_DIPf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* set SIP */
    temp_var = pFilter_cfg->sip;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_SIPf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* set FLOWLABEL */
    temp_var = pFilter_cfg->flowlabel;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_FLOWLABELf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* set RVID */
    temp_var = pFilter_cfg->rvid;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_RVIDf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
       
    /* set DSTPORT */
    temp_var = pFilter_cfg->dstport;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_DSTPORTf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* set SRCPORT */
    temp_var = pFilter_cfg->srcport;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_SRCPORTf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* set ETHTYPE */
    temp_var = pFilter_cfg->ethertype;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_ETHTYPEf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* set IPPROTO */
    temp_var = pFilter_cfg->ipproto;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_IPPROTOf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* set TOS */
    temp_var = pFilter_cfg->tos;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_TOSf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* set TCPFLAG */
    temp_var = pFilter_cfg->tcpflag;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_TCPFLAGf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* set SLP */
    temp_var = pFilter_cfg->slp;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_INACL_SLPf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* set FRAMETYPE */
    temp_var = pFilter_cfg->frametype;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_INACL_FRAMETYPEf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* set IPV6MLD */
    temp_var = pFilter_cfg->ipv6mld;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_INACL_IPV6MLDf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* set IPV6 */
    temp_var = pFilter_cfg->ipv6;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_INACL_IPV6f, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* set IPV4 */
    temp_var = pFilter_cfg->ipv4;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_INACL_IPV4f, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* set PATTERNMATCH */
    temp_var = pFilter_cfg->patternmatch;    
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_PATTERNMATCHf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }  

    /* set PKTSVID */
    temp_var = pFilter_cfg->pktsvid;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_INACL_PKTSVIDf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }  

    /* set ACTGROUP */
    temp_var = pAction->actGroup;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_ACTGROUPf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }  

    temp_var = 0;
    if (/*0b000*/ 0 == pAction->actGroup)/*permit, drop, redirect, copy to cpu*/
    {  
        //temp_var = (pAction->un.permit_drop_redirect.acttype & 0x3);
        if (0 == (pAction->un.permit_drop_redirect.acttype))
        {/*permit*/
            temp_var = (0x0 << 5);    
        }
        else if (1 == (pAction->un.permit_drop_redirect.acttype))
        {/*drop*/
            temp_var = (0x1 << 5);
        } 
        else if (2 == (pAction->un.permit_drop_redirect.acttype))
        {/*redirect*/
            temp_var = (0x2 << 5) | (pAction->un.permit_drop_redirect.portid & 0x1f);
        } 
        else if (3 == (pAction->un.permit_drop_redirect.acttype))
        {/*copy to cpu*/
            temp_var = (0x3 << 5);
        }
        else
        {}
        
        /* set SSW_PIE89_TABLE_SRAM_7B */
        if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_SRAM_7Bf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
            return ret;
        }        
    }
    
    if (/*0b001*/ 1 == pAction->actGroup)/*Mirror*/
    {
        /* set SSW_PIE89_TABLE_SRAM_7B */
        temp_var = pAction->un.mirror.mirrorsetid;
        if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_SRAM_7Bf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
            return ret;
        }
    }
    
    if (/*0b010*/ 2 == pAction->actGroup)/*Log*/
    {
        /* set SSW_PIE89_TABLE_SRAM_7B */
        temp_var = pAction->un.log.logindex;        
        if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_SRAM_7Bf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
            return ret;
        }
    }    
    
    if (/*0b100*/ 4 == pAction->actGroup)/*Ratelimit*/
    {
        /* set SSW_PIE89_TABLE_SRAM_17B */
        temp_var = (pAction->un.ratelimit.rateindex & 0x7f);        
        if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_SRAM_17Bf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
            return ret;
        }
    }

    if (/*0b101*/ 5 == pAction->actGroup)/*New SVID*/
    {
        /* set SSW_PIE89_TABLE_SRAM_17B */
        temp_var = (pAction->un.newsvid.svid & 0xfff);        
        if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_SRAM_17Bf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
            return ret;
        }
    }
    
    if (/*0b110*/ 6 == pAction->actGroup)/*New CVID*/
    {
        temp_var = 0;
        /* set SSW_PIE89_TABLE_SRAM_17B */
        temp_var = ((pAction->un.newcvid.replacecvid & 0x1) << 16);
        temp_var = temp_var | ((pAction->un.newcvid.replacecpri & 0x1) << 15);
        temp_var = temp_var | ((pAction->un.newcvid.cvid & 0xfff) << 3);
        temp_var = temp_var | (pAction->un.newcvid.cpri & 0x7);
        if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_SRAM_17Bf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
            return ret;
        }
    }
   
    /* set VALID */
    temp_var = pFilter_cfg->valid;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_VALIDf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* set CARE_DMAC */
    osal_memcpy(temp_mac, pFilter_cfg->care_dmac, ETHER_ADDR_LEN);
    if ((ret = table_field_mac_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_DMACf, temp_mac, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* set CARE_SMAC */
    osal_memcpy(temp_mac, pFilter_cfg->care_smac, ETHER_ADDR_LEN);
    if ((ret = table_field_mac_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_SMACf, temp_mac, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* set CARE_DIP */
    temp_var = pFilter_cfg->care_dip;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_DIPf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* set CARE_SIP */
    temp_var = pFilter_cfg->care_sip;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_SIPf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
   
    /* set CARE_FLOWLABE */
    temp_var = pFilter_cfg->care_flowlabel;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_FLOWLABELf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
   
    /* set CARE_RVID */
    temp_var = pFilter_cfg->care_rvid;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_RVIDf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }    
    
    /* set CARE_DSTPORT */
    temp_var = pFilter_cfg->care_dstport;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_DSTPORTf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* set CARE_SRCPORT */
    temp_var = pFilter_cfg->care_srcport;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_SRCPORTf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* set CARE_ETHTYPE */
    temp_var = pFilter_cfg->care_ethertype;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_ETHTYPEf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* set CARE_IPPROTO */
    temp_var = pFilter_cfg->care_ipproto;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_IPPROTOf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* set CARE_TOS */
    temp_var = pFilter_cfg->care_tos;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_TOSf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* set CARE_TCPFLAG */
    temp_var = pFilter_cfg->care_tcpflag;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_TCPFLAGf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* set CARE_FLOWTABLE_SLP */
    temp_var = pFilter_cfg->care_slp;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_INACL_SLPf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* set CARE_FLOWTABLE_FRAMETYPE */
    temp_var = pFilter_cfg->care_frametype;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_INACL_FRAMETYPEf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* set CARE_FLOWTABLE_IPV6MLD */
    temp_var = pFilter_cfg->care_ipv6mld;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_INACL_IPV6MLDf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* set CARE_FLOWTABLE_IPV6 */
    temp_var = pFilter_cfg->care_ipv6;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_INACL_IPV6f, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* set CARE_FLOWTABLE_IPV4 */
    temp_var = pFilter_cfg->care_ipv4;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_INACL_IPV4f, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* set CARE_PATTERNMATCH */
    temp_var = pFilter_cfg->care_patternmatch;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_PATTERNMATCHf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }  

    /* set CARE_FLOWTABLE_PKTSVID */
    temp_var = pFilter_cfg->care_pktsvid;
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_CARE_INACL_PKTSVIDf, &temp_var, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }  

    /* programming flow table entry in chip */
    if ((ret = table_write(unit, SSW_PIE89_TABLEt, table_index, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
   
    return RT_ERR_OK;
} /* end of _dal_ssw_aclEntry_set */

/* Function Name:
 *      _dal_ssw_logEntry_get
 * Description:
 *      Get log entry from chip.
 * Input:
 *      unit        - unit id
 *      log_id      - log id
 * Output:
 *      pPkt_cnt    - counter by packet
 *      pByte_cnt   - counter by byte
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 */
static int32
_dal_ssw_logEntry_get(uint32 unit, rtk_log_id_t log_id, uint32 *pPkt_cnt, uint64 *pByte_cnt)
{
    int32   ret;
    uint32  value[2];
    log_entry_t    log_entry;

    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_FILTER), "unit=%d, log_id=%d", unit, log_id);  

    osal_memset(&log_entry, 0, sizeof(log_entry_t));

    /* get entry from chip */
    if ((ret = table_read(unit, SSW_LOG_TABLEt, log_id, (uint32 *) &log_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
        
    /* get PKTCNT */
    if ((ret = table_field_get( unit, SSW_LOG_TABLEt, SSW_LOG_TABLE_PKTCNTf, pPkt_cnt, (uint32 *) &log_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /* get BYTECNT */
    if ((ret = table_field_get( unit, SSW_LOG_TABLEt, SSW_LOG_TABLE_BYTECNTf, value, (uint32 *) &log_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }    

    *pByte_cnt = (uint64)value[0] + (((uint64)value[1]) << 32);
    
    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_FILTER), "pPkt_cnt=%d, pByte_cnt=%llu", *pPkt_cnt, *pByte_cnt);  

    return RT_ERR_OK;
} /* end of _dal_ssw_logEntry_get */

/* Function Name:
 *      _dal_ssw_logEntry_set
 * Description:
 *      Set log entry to chip.
 * Input:
 *      unit        - unit id
 *      log_id      - log id
 *      pkt_cnt     - counter by packet
 *      byte_cnt    - counter by byte
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 */
static int32
_dal_ssw_logEntry_set(uint32 unit, rtk_log_id_t log_id, uint32 pkt_cnt, uint64 byte_cnt)
{
    int32   ret;
    uint32  value[2];
    log_entry_t    log_entry;
    
    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_FILTER), "unit=%d, log_id=%d, pkt_cnt=%d, byte_cnt=%llu", 
           unit, log_id, pkt_cnt, byte_cnt);      
    
    osal_memset(&log_entry, 0, sizeof(log_entry_t));
   
    /* set PKTCNT */
    if ((ret = table_field_set( unit, SSW_LOG_TABLEt, SSW_LOG_TABLE_PKTCNTf, &pkt_cnt, (uint32 *) &log_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }  

    value[0] = (uint32)(byte_cnt & 0xFFFFFFFF);
    value[1] = (uint32)((byte_cnt >> 32) & 0xFFFFFFFF);
    
    /* set BYTECNT */
    if ((ret = table_field_set( unit, SSW_LOG_TABLEt, SSW_LOG_TABLE_BYTECNTf, value, (uint32 *) &log_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    } 
    
    /* programming flow table entry in chip */
    if ((ret = table_write(unit, SSW_LOG_TABLEt, log_id, (uint32 *) &log_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    return RT_ERR_OK;
} /* end of _dal_ssw_logEntry_set */

static int32
_dal_ssw_filterEntry_del(uint32 unit, rtk_filter_id_t filter_id, rtk_filter_owner_t owner_type)
{
    int32   ret;
    uint32  table_index;
    pie89_entry_t    pie89_entry;
    
    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_FILTER), "unit=%d, filter_id=%d, owner_type=%d", 
           unit, filter_id, owner_type);   
    
    /* Check the owner of this entry */
    RT_PARAM_CHK(FILTERINFO_OWNER_CHK(unit, filter_id, FILTER_NO_OWNER), RT_ERR_FILTER_FLOWTBL_EMPTY);
    RT_PARAM_CHK(!FILTERINFO_OWNER_CHK(unit, filter_id, owner_type), RT_ERR_FAILED);
    
    /*translate filter-id to table index*/
    table_index = ((filter_id / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 7) + (filter_id % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));
    
    osal_memset(&pie89_entry, 0, sizeof(pie89_entry_t));
    
    /* programming flow table entry in chip */
    if ((ret = table_write(unit, SSW_PIE89_TABLEt, table_index, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* Fixup the owner of this entry */
    FILTERINFO_OWNER_SET(unit, filter_id, FILTER_NO_OWNER);
    
    return RT_ERR_OK; 
} /*end of _dal_ssw_filterEntry_del */


/* Function Name:
 *      _dal_ssw_filter_init_config
 * Description:
 *      Initialize config of filter module for the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Must initialize filter module before calling this API.
 */
static int32
_dal_ssw_filter_init_config(uint32 unit)
{
    int32   ret;
    uint32  value, temp;
    
    
    FILTER_SEM_LOCK(unit);
    /* get value from CHIP*/
    if ((ret = reg_read(unit, SSW_FLOW_TABLE_CONTROLr, &value)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    /*init cutline*/
    temp = RTK_DEFAULT_CUTLINE;
    if ((ret = reg_field_set(unit, SSW_FLOW_TABLE_CONTROLr, SSW_CUTLINEf, &temp, &value)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "cutline init error!");
        return ret;
    }
    
    /*init PIE mode*/
    temp = RTK_DEFAULT_PIE_MODE;
    if ((ret = reg_field_set(unit, SSW_FLOW_TABLE_CONTROLr, SSW_PIE89_MODEf, &temp, &value)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "PIE mode init error!");
        return ret;
    }
    
    /*init PIE status*/
    temp = RTK_DEFAULT_PIE_STATUS;
    if ((ret = reg_field_set(unit, SSW_FLOW_TABLE_CONTROLr, SSW_EN_PIE89f, &temp, &value)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "PIE status init error!");
        return ret;
    }

    /* program value to CHIP*/
    if ((ret = reg_write(unit, SSW_FLOW_TABLE_CONTROLr, &value)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, MOD_DAL|MOD_FILTER, "");
        return ret;
    }
    
    /*init entry*/
    if ((ret = _dal_ssw_filterEntry_init(unit)) != RT_ERR_OK)
    {
        FILTER_SEM_UNLOCK(unit);
        RT_ERR(ret, MOD_DAL|MOD_FILTER, "filter entry init error!");
        return ret;
    }
    
    FILTER_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of _dal_ssw_filter_init_config */

static int32
_dal_ssw_filterEntry_init(uint32 unit)
{    
    int32           ret;
    pie89_entry_t   pie89_entry;
    uint32          table_index;
    uint32          filter_id;
    
    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_FILTER), "unit=%d", unit); 
    
    osal_memset(&pie89_entry, 0, sizeof(pie89_entry_t));
        
    for (filter_id=HAL_PIE_FILTER_ID_MIN(unit); filter_id<=HAL_PIE_FILTER_ID_MAX(unit); filter_id++)
    {
        /*translate filter-id to table index*/
        table_index = ((filter_id / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 7) + (filter_id % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));
        
        /* programming flow table entry in chip */
        if ((ret = table_write(unit, SSW_PIE89_TABLEt, table_index, (uint32 *) &pie89_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
            return ret;
        }
    }
    
    return RT_ERR_OK;
} /* end of _dal_ssw_filterEntry_init */

/* Function Name:
 *      _dal_ssw_filterEntry_valid_set
 * Description:
 *      Set valid field of filter entry to chip.(apply to all filter entry(flow and acl)
 * Input:
 *      unit        - unit id
 *      filter_id   - filter id
 *      valid       - value of valid field
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 */
static int32
_dal_ssw_filterEntry_valid_set(uint32 unit, rtk_filter_id_t filter_id, uint32 valid)
{
    int32   ret;
    pie89_entry_t    pie89_entry;
    uint32  table_index;
    
    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_FILTER), "unit=%d, filter_id=%d, valid=%d", 
           unit, filter_id, valid);    
    
    /*translate filter-id to table index*/
    table_index = ((filter_id / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 7) 
                    + (filter_id % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));

    /* get entry from chip */
    if ((ret = table_read(unit, SSW_PIE89_TABLEt, table_index, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    if ((ret = table_field_set( unit, SSW_PIE89_TABLEt, SSW_PIE89_TABLE_VALIDf, &valid, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }
    
    /* programming flow table entry in chip */
    if ((ret = table_write(unit, SSW_PIE89_TABLEt, table_index, (uint32 *) &pie89_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FILTER), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of _dal_ssw_filterEntry_valid_set */
