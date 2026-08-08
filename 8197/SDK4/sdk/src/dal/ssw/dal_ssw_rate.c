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
 * Purpose : Definition those public port bandwidth control and storm control APIs and its data type
 *           in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) Configuration of ingress port bandwidth control (ingress rate limit ).
 *           2) Configuration of egress port bandwidth control (egress rate limit).
 *           3) Configuration of storm control
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
#include <osal/time.h>
#include <hal/chipdef/allmem.h>
#include <hal/chipdef/allreg.h>
#include <hal/chipdef/ssw/rtk_ssw_table_struct.h>
#include <hal/chipdef/ssw/rtk_ssw_reg_struct.h>
#include <hal/mac/reg.h>
#include <hal/mac/mem.h>
#include <hal/common/halctrl.h>
#include <dal/ssw/dal_ssw_rate.h>
#include <dal/ssw/dal_ssw_port.h>
#include <rtk/default.h>
#include <rtk/rate.h>

/* 
 * Symbol Definition 
 */
#define DAL_SSW_RATE_DISABLED_EGR_BANDWIDTH_RATE                 0xFFFF

typedef struct dal_ssw_rate_egr_bandwidth_ctrl_s {
    uint32  status;
    uint32  rate;
} dal_ssw_rate_egr_bandwidth_ctrl_t;

typedef enum dal_ssw_rate_igr_bandwidth_ctrl_act_e {
    IGR_ACT_RCV = 0,
    IGR_ACT_DROP,
    IGR_ACT_END
} dal_ssw_rate_igr_bandwidth_ctrl_act_t;

/* 
 * Data Declaration 
 */
static uint32               rate_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         rate_sem[RTK_MAX_NUM_OF_UNIT];

static dal_ssw_rate_egr_bandwidth_ctrl_t   egr_bandwidth_ctrl[RTK_MAX_NUM_OF_UNIT][RTK_MAX_NUM_OF_PORTS];

#if 0
const static uint16 igrBandwidthPortControl_regidx[] = {INPUT_BANDWIDTH_PORT0_CONTROL, INPUT_BANDWIDTH_PORT1_CONTROL, 
                                                        INPUT_BANDWIDTH_PORT2_CONTROL, INPUT_BANDWIDTH_PORT3_CONTROL,
                                                        INPUT_BANDWIDTH_PORT4_CONTROL, INPUT_BANDWIDTH_PORT5_CONTROL,
                                                        INPUT_BANDWIDTH_PORT6_CONTROL, INPUT_BANDWIDTH_PORT7_CONTROL,
                                                        INPUT_BANDWIDTH_PORT8_CONTROL, INPUT_BANDWIDTH_PORT9_CONTROL,
                                                        INPUT_BANDWIDTH_PORT10_CONTROL, INPUT_BANDWIDTH_PORT11_CONTROL, 
                                                        INPUT_BANDWIDTH_PORT12_CONTROL, INPUT_BANDWIDTH_PORT13_CONTROL,
                                                        INPUT_BANDWIDTH_PORT14_CONTROL, INPUT_BANDWIDTH_PORT15_CONTROL,
                                                        INPUT_BANDWIDTH_PORT16_CONTROL, INPUT_BANDWIDTH_PORT17_CONTROL,
                                                        INPUT_BANDWIDTH_PORT18_CONTROL, INPUT_BANDWIDTH_PORT19_CONTROL,
                                                        INPUT_BANDWIDTH_PORT20_CONTROL, INPUT_BANDWIDTH_PORT21_CONTROL, 
                                                        INPUT_BANDWIDTH_PORT22_CONTROL, INPUT_BANDWIDTH_PORT23_CONTROL,
                                                        INPUT_BANDWIDTH_PORT24_CONTROL, INPUT_BANDWIDTH_PORT25_CONTROL,
                                                        INPUT_BANDWIDTH_PORT26_CONTROL, INPUT_BANDWIDTH_PORT27_CONTROL,
                                                        INPUT_BANDWIDTH_PORT28_CONTROL};                                                  
#endif
                                                     
const static uint16 wfqwrrPortParameter0_regidx[] = {SSW_WFQ_WRR_PORT_0_PARAMETER0r, SSW_WFQ_WRR_PORT_1_PARAMETER0r, 
                                                     SSW_WFQ_WRR_PORT_2_PARAMETER0r, SSW_WFQ_WRR_PORT_3_PARAMETER0r,
                                                     SSW_WFQ_WRR_PORT_4_PARAMETER0r, SSW_WFQ_WRR_PORT_5_PARAMETER0r,
                                                     SSW_WFQ_WRR_PORT_6_PARAMETER0r, SSW_WFQ_WRR_PORT_7_PARAMETER0r,
                                                     SSW_WFQ_WRR_PORT_8_PARAMETER0r, SSW_WFQ_WRR_PORT_9_PARAMETER0r,
                                                     SSW_WFQ_WRR_PORT_10_PARAMETER0r, SSW_WFQ_WRR_PORT_11_PARAMETER0r, 
                                                     SSW_WFQ_WRR_PORT_12_PARAMETER0r, SSW_WFQ_WRR_PORT_13_PARAMETER0r,
                                                     SSW_WFQ_WRR_PORT_14_PARAMETER0r, SSW_WFQ_WRR_PORT_15_PARAMETER0r,
                                                     SSW_WFQ_WRR_PORT_16_PARAMETER0r, SSW_WFQ_WRR_PORT_17_PARAMETER0r,
                                                     SSW_WFQ_WRR_PORT_18_PARAMETER0r, SSW_WFQ_WRR_PORT_19_PARAMETER0r,
                                                     SSW_WFQ_WRR_PORT_20_PARAMETER0r, SSW_WFQ_WRR_PORT_21_PARAMETER0r, 
                                                     SSW_WFQ_WRR_PORT_22_PARAMETER0r, SSW_WFQ_WRR_PORT_23_PARAMETER0r,
                                                     SSW_WFQ_WRR_PORT_24_PARAMETER0r, SSW_WFQ_WRR_PORT_25_PARAMETER0r,
                                                     SSW_WFQ_WRR_PORT_26_PARAMETER0r, SSW_WFQ_WRR_PORT_27_PARAMETER0r,
                                                     SSW_WFQ_WRR_PORT_28_PARAMETER0r};   
                                                     
                                                     
const static uint16 stormFilteringControl0_regidx[] = {SSW_STORM_FILTERING_CONTROL_PORT_00r, SSW_STORM_FILTERING_CONTROL_PORT_10r, 
                                                       SSW_STORM_FILTERING_CONTROL_PORT_20r, SSW_STORM_FILTERING_CONTROL_PORT_30r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_40r, SSW_STORM_FILTERING_CONTROL_PORT_50r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_60r, SSW_STORM_FILTERING_CONTROL_PORT_70r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_80r, SSW_STORM_FILTERING_CONTROL_PORT_90r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_100r, SSW_STORM_FILTERING_CONTROL_PORT_110r, 
                                                       SSW_STORM_FILTERING_CONTROL_PORT_120r, SSW_STORM_FILTERING_CONTROL_PORT_130r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_140r, SSW_STORM_FILTERING_CONTROL_PORT_150r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_160r, SSW_STORM_FILTERING_CONTROL_PORT_170r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_180r, SSW_STORM_FILTERING_CONTROL_PORT_190r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_200r, SSW_STORM_FILTERING_CONTROL_PORT_210r, 
                                                       SSW_STORM_FILTERING_CONTROL_PORT_220r, SSW_STORM_FILTERING_CONTROL_PORT_230r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_240r, SSW_STORM_FILTERING_CONTROL_PORT_250r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_260r, SSW_STORM_FILTERING_CONTROL_PORT_270r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_280r};
                                                      
const static uint16 stormFilteringControl1_regidx[] = {SSW_STORM_FILTERING_CONTROL_PORT_01r, SSW_STORM_FILTERING_CONTROL_PORT_11r, 
                                                       SSW_STORM_FILTERING_CONTROL_PORT_21r, SSW_STORM_FILTERING_CONTROL_PORT_31r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_41r, SSW_STORM_FILTERING_CONTROL_PORT_51r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_61r, SSW_STORM_FILTERING_CONTROL_PORT_71r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_81r, SSW_STORM_FILTERING_CONTROL_PORT_91r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_101r, SSW_STORM_FILTERING_CONTROL_PORT_111r, 
                                                       SSW_STORM_FILTERING_CONTROL_PORT_121r, SSW_STORM_FILTERING_CONTROL_PORT_131r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_141r, SSW_STORM_FILTERING_CONTROL_PORT_151r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_161r, SSW_STORM_FILTERING_CONTROL_PORT_171r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_181r, SSW_STORM_FILTERING_CONTROL_PORT_191r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_201r, SSW_STORM_FILTERING_CONTROL_PORT_211r, 
                                                       SSW_STORM_FILTERING_CONTROL_PORT_221r, SSW_STORM_FILTERING_CONTROL_PORT_231r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_241r, SSW_STORM_FILTERING_CONTROL_PORT_251r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_261r, SSW_STORM_FILTERING_CONTROL_PORT_271r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_281r};
                                                      
const static uint16 stormFilteringControl2_regidx[] = {SSW_STORM_FILTERING_CONTROL_PORT_02r, SSW_STORM_FILTERING_CONTROL_PORT_12r, 
                                                       SSW_STORM_FILTERING_CONTROL_PORT_22r, SSW_STORM_FILTERING_CONTROL_PORT_32r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_42r, SSW_STORM_FILTERING_CONTROL_PORT_52r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_62r, SSW_STORM_FILTERING_CONTROL_PORT_72r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_82r, SSW_STORM_FILTERING_CONTROL_PORT_92r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_102r, SSW_STORM_FILTERING_CONTROL_PORT_112r, 
                                                       SSW_STORM_FILTERING_CONTROL_PORT_122r, SSW_STORM_FILTERING_CONTROL_PORT_132r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_142r, SSW_STORM_FILTERING_CONTROL_PORT_152r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_162r, SSW_STORM_FILTERING_CONTROL_PORT_172r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_182r, SSW_STORM_FILTERING_CONTROL_PORT_192r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_202r, SSW_STORM_FILTERING_CONTROL_PORT_212r, 
                                                       SSW_STORM_FILTERING_CONTROL_PORT_222r, SSW_STORM_FILTERING_CONTROL_PORT_232r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_242r, SSW_STORM_FILTERING_CONTROL_PORT_252r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_262r, SSW_STORM_FILTERING_CONTROL_PORT_272r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_282r};

const static uint16 stormFilteringControl3_regidx[] = {SSW_STORM_FILTERING_CONTROL_PORT_03r, SSW_STORM_FILTERING_CONTROL_PORT_13r, 
                                                       SSW_STORM_FILTERING_CONTROL_PORT_23r, SSW_STORM_FILTERING_CONTROL_PORT_33r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_43r, SSW_STORM_FILTERING_CONTROL_PORT_53r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_63r, SSW_STORM_FILTERING_CONTROL_PORT_73r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_83r, SSW_STORM_FILTERING_CONTROL_PORT_93r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_103r, SSW_STORM_FILTERING_CONTROL_PORT_113r, 
                                                       SSW_STORM_FILTERING_CONTROL_PORT_123r, SSW_STORM_FILTERING_CONTROL_PORT_133r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_143r, SSW_STORM_FILTERING_CONTROL_PORT_153r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_163r, SSW_STORM_FILTERING_CONTROL_PORT_173r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_183r, SSW_STORM_FILTERING_CONTROL_PORT_193r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_203r, SSW_STORM_FILTERING_CONTROL_PORT_213r, 
                                                       SSW_STORM_FILTERING_CONTROL_PORT_223r, SSW_STORM_FILTERING_CONTROL_PORT_233r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_243r, SSW_STORM_FILTERING_CONTROL_PORT_253r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_263r, SSW_STORM_FILTERING_CONTROL_PORT_273r,
                                                       SSW_STORM_FILTERING_CONTROL_PORT_283r};  
                                                                                                                                                                  
const static uint16 uknUniRate_fieldidx[] = {SSW_P0_UKN_UNI_RATEf, SSW_P1_UKN_UNI_RATEf, SSW_P2_UKN_UNI_RATEf, SSW_P3_UKN_UNI_RATEf, SSW_P4_UKN_UNI_RATEf, SSW_P5_UKN_UNI_RATEf, SSW_P6_UKN_UNI_RATEf, SSW_P7_UKN_UNI_RATEf, SSW_P8_UKN_UNI_RATEf, SSW_P9_UKN_UNI_RATEf,
                                             SSW_P10_UKN_UNI_RATEf, SSW_P11_UKN_UNI_RATEf, SSW_P12_UKN_UNI_RATEf, SSW_P13_UKN_UNI_RATEf, SSW_P14_UKN_UNI_RATEf, SSW_P15_UKN_UNI_RATEf, SSW_P16_UKN_UNI_RATEf, SSW_P17_UKN_UNI_RATEf, SSW_P18_UKN_UNI_RATEf, SSW_P19_UKN_UNI_RATEf,
                                             SSW_P20_UKN_UNI_RATEf, SSW_P21_UKN_UNI_RATEf, SSW_P22_UKN_UNI_RATEf, SSW_P23_UKN_UNI_RATEf, SSW_P24_UKN_UNI_RATEf, SSW_P25_UKN_UNI_RATEf, SSW_P26_UKN_UNI_RATEf, SSW_P27_UKN_UNI_RATEf, SSW_P28_UKN_UNI_RATEf};                                                         
                                             
const static uint16 uknMultiRate_fieldidx[] = {SSW_P0_UKN_MULTI_RATEf, SSW_P1_UKN_MULTI_RATEf, SSW_P2_UKN_MULTI_RATEf, SSW_P3_UKN_MULTI_RATEf, SSW_P4_UKN_MULTI_RATEf, SSW_P5_UKN_MULTI_RATEf, SSW_P6_UKN_MULTI_RATEf, SSW_P7_UKN_MULTI_RATEf, SSW_P8_UKN_MULTI_RATEf, SSW_P9_UKN_MULTI_RATEf,
                                               SSW_P10_UKN_MULTI_RATEf, SSW_P11_UKN_MULTI_RATEf, SSW_P12_UKN_MULTI_RATEf, SSW_P13_UKN_MULTI_RATEf, SSW_P14_UKN_MULTI_RATEf, SSW_P15_UKN_MULTI_RATEf, SSW_P16_UKN_MULTI_RATEf, SSW_P17_UKN_MULTI_RATEf, SSW_P18_UKN_MULTI_RATEf, SSW_P19_UKN_MULTI_RATEf,
                                               SSW_P20_UKN_MULTI_RATEf, SSW_P21_UKN_MULTI_RATEf, SSW_P22_UKN_MULTI_RATEf, SSW_P23_UKN_MULTI_RATEf, SSW_P24_UKN_MULTI_RATEf, SSW_P25_UKN_MULTI_RATEf, SSW_P26_UKN_MULTI_RATEf, SSW_P27_UKN_MULTI_RATEf, SSW_P28_UKN_MULTI_RATEf};
                                               
const static uint16 multiRate_fieldidx[] = {SSW_P0_MULTI_RATEf, SSW_P1_MULTI_RATEf, SSW_P2_MULTI_RATEf, SSW_P3_MULTI_RATEf, SSW_P4_MULTI_RATEf, SSW_P5_MULTI_RATEf, SSW_P6_MULTI_RATEf, SSW_P7_MULTI_RATEf, SSW_P8_MULTI_RATEf, SSW_P9_MULTI_RATEf,
                                            SSW_P10_MULTI_RATEf, SSW_P11_MULTI_RATEf, SSW_P12_MULTI_RATEf, SSW_P13_MULTI_RATEf, SSW_P14_MULTI_RATEf, SSW_P15_MULTI_RATEf, SSW_P16_MULTI_RATEf, SSW_P17_MULTI_RATEf, SSW_P18_MULTI_RATEf, SSW_P19_MULTI_RATEf,
                                            SSW_P20_MULTI_RATEf, SSW_P21_MULTI_RATEf, SSW_P22_MULTI_RATEf, SSW_P23_MULTI_RATEf, SSW_P24_MULTI_RATEf, SSW_P25_MULTI_RATEf, SSW_P26_MULTI_RATEf, SSW_P27_MULTI_RATEf, SSW_P28_MULTI_RATEf};

const static uint16 bcstRate_fieldidx[] = {SSW_P0_BCST_RATEf, SSW_P1_BCST_RATEf, SSW_P2_BCST_RATEf, SSW_P3_BCST_RATEf, SSW_P4_BCST_RATEf, SSW_P5_BCST_RATEf, SSW_P6_BCST_RATEf, SSW_P7_BCST_RATEf, SSW_P8_BCST_RATEf, SSW_P9_BCST_RATEf,
                                           SSW_P10_BCST_RATEf, SSW_P11_BCST_RATEf, SSW_P12_BCST_RATEf, SSW_P13_BCST_RATEf, SSW_P14_BCST_RATEf, SSW_P15_BCST_RATEf, SSW_P16_BCST_RATEf, SSW_P17_BCST_RATEf, SSW_P18_BCST_RATEf, SSW_P19_BCST_RATEf,
                                           SSW_P20_BCST_RATEf, SSW_P21_BCST_RATEf, SSW_P22_BCST_RATEf, SSW_P23_BCST_RATEf, SSW_P24_BCST_RATEf, SSW_P25_BCST_RATEf, SSW_P26_BCST_RATEf, SSW_P27_BCST_RATEf, SSW_P28_BCST_RATEf};
                                            
const static uint16 wfqwrr_rate_fieldidx[] = {SSW_P0_WFQWRR_RATEf, SSW_P1_WFQWRR_RATEf, SSW_P2_WFQWRR_RATEf, SSW_P3_WFQWRR_RATEf, SSW_P4_WFQWRR_RATEf, SSW_P5_WFQWRR_RATEf, SSW_P6_WFQWRR_RATEf, SSW_P7_WFQWRR_RATEf, SSW_P8_WFQWRR_RATEf, SSW_P9_WFQWRR_RATEf\
                         ,SSW_P10_WFQWRR_RATEf, SSW_P11_WFQWRR_RATEf, SSW_P12_WFQWRR_RATEf, SSW_P13_WFQWRR_RATEf, SSW_P14_WFQWRR_RATEf, SSW_P15_WFQWRR_RATEf, SSW_P16_WFQWRR_RATEf, SSW_P17_WFQWRR_RATEf, SSW_P18_WFQWRR_RATEf, SSW_P19_WFQWRR_RATEf\
                         ,SSW_P20_WFQWRR_RATEf, SSW_P21_WFQWRR_RATEf, SSW_P22_WFQWRR_RATEf, SSW_P23_WFQWRR_RATEf, SSW_P24_WFQWRR_RATEf, SSW_P25_WFQWRR_RATEf, SSW_P26_WFQWRR_RATEf, SSW_P27_WFQWRR_RATEf, SSW_P28_WFQWRR_RATEf};
/*                                                  
 * Macro Definition                                 
 */                                                 
/* rate semaphore handling */                       
#define RATE_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(rate_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_DAL|MOD_RATE), "semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)
#define RATE_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(rate_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_DAL|MOD_RATE), "semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)


/* 
 * Function Declaration 
 */
static int32 _dal_ssw_rate_init_config(uint32 unit);
static int32 _dal_ssw_rate_igrBandwidthCtrlFCCnt_set(uint32 unit, rtk_port_t port, uint32 fccnt);
static int32 _dal_ssw_rate_igrBandwidthCtrlAct_set(uint32 unit, rtk_port_t port, dal_ssw_rate_igr_bandwidth_ctrl_act_t act);


/* Function Name:
 *      dal_ssw_rate_init
 * Description:
 *      Initial the rate module of the specified device..
 * Input:
 *      unit                - unit id
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
int32
dal_ssw_rate_init(uint32 unit)
{
    int32   ret;
    
    rate_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    rate_sem[unit] = osal_sem_mutex_create();
    if (0 == rate_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_RATE), "semaphore create failed");
        return RT_ERR_FAILED;
    }
    
    osal_memset(&egr_bandwidth_ctrl, 0, sizeof(egr_bandwidth_ctrl));
    
    /* set init flag to complete init */
    rate_init[unit] = INIT_COMPLETED;
    
    if (( ret = _dal_ssw_rate_init_config(unit)) != RT_ERR_OK)
    {
        rate_init[unit] = INIT_NOT_COMPLETED;
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    return RT_ERR_OK;
} /* end of dal_ssw_rate_init */

/* Function Name:
 *      dal_ssw_rate_igrBandwidthCtrlEnable_get
 * Description:
 *      Get the ingress bandwidth control status.
 * Input:
 *      unit                - unit id
 *      port                - port id
 * Output:
 *      pEnable            - status of ingress bandwidth control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_NULL_POINTER  - NULL pointer
 * Note:
 *    The status of ingress bandwidth control is as following:
 *    - DISABLED
 *    - ENABLED
 */
int32
dal_ssw_rate_igrBandwidthCtrlEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value;  
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d",
           unit, port);    
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
       
    RATE_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    //if ((ret = reg_field_read(unit, (uint32)igrBandwidthPortControl_regidx[port], SSW_INBW_EN, &value)) != RT_ERR_OK)
    if ((ret = reg_array_field_read(unit, SSW_INPUT_BANDWIDTH_PORT_CONTROLr, port, 0, SSW_INBW_ENf, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    RATE_SEM_UNLOCK(unit);
    
    /* chip's value translate */
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
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "pEnable=%d", *pEnable); 
    
    return RT_ERR_OK;
} /* end of dal_ssw_rate_igrBandwidthCtrlEnable_get */

/* Function Name:
 *      dal_ssw_rate_igrBandwidthCtrlEnable_set
 * Description:
 *      Set the ingress bandwidth control status.
 * Input:
 *      unit                - unit id
 *      port                - port id
 *      enable              - status of ingress bandwidth control
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_INPUT         - Invalid input parameter
 * Note:
 *    The status of ingress bandwidth control is as following:
 *    - DISABLED
 *    - ENABLED
 */
int32
dal_ssw_rate_igrBandwidthCtrlEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;
    uint32  old_value;
    rtk_enable_t  port_admin_status;

    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, enable",
           unit, port, enable);
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(enable >= RTK_ENABLE_END, RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    /* translate to chip value */
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
        
    RATE_SEM_LOCK(unit);
 
    /* E0005361 */
    //if ((ret = reg_field_read(unit, (uint32)igrBandwidthPortControl_regidx[port], SSW_INBW_EN, &old_value)) != RT_ERR_OK)
    if ((ret = reg_array_field_read(unit, SSW_INPUT_BANDWIDTH_PORT_CONTROLr, port, 0, SSW_INBW_ENf, &old_value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    if ((ret = dal_ssw_port_adminEnable_get(unit, port, &port_admin_status)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }            
    
    if ((old_value != value) && (DISABLED != port_admin_status))
    {
        if ((ret = dal_ssw_port_rxEnable_set(unit, port, DISABLED)) != RT_ERR_OK)
        {
            RATE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
            return ret;
        } 
        osal_time_usleep(1200);        
    }
    /* End of E0005361 */
   
    /* program value into CHIP*/
    //if ((ret = reg_field_write(unit, (uint32)igrBandwidthPortControl_regidx[port], SSW_INBW_EN, value)) != RT_ERR_OK)
    if ((ret = reg_array_field_write(unit, SSW_INPUT_BANDWIDTH_PORT_CONTROLr, port, 0, SSW_INBW_ENf, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }

    /* E0005361 */
    if ((old_value != value) && (DISABLED != port_admin_status))
    {
        if ((ret = dal_ssw_port_rxEnable_set(unit, port, ENABLED)) != RT_ERR_OK)
        {
            RATE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
            return ret;
        } 
    }
    /* End of E0005361 */
    
    RATE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_rate_igrBandwidthCtrlEnable_set */

/* Function Name:
 *      dal_ssw_rate_igrBandwidthCtrlRate_get
 * Description:
 *      Get the ingress bandwidth control rate.
 * Input:
 *      unit        - unit id
 *      port        - port id
 * Output:
 *      pRate      - ingress bandwidth control rate
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_NULL_POINTER  - NULL pointer
 * Note:
 *    The actual rate is "rate * chip granularity".
 *    The unit of granularity in RTL8389/RTL8329 is 16Kbps.
 */
int32
dal_ssw_rate_igrBandwidthCtrlRate_get(uint32 unit, rtk_port_t port, uint32 *pRate)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d",
           unit, port);  
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pRate), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);    
    
    RATE_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    //if ((ret = reg_field_read(unit, (uint32)igrBandwidthPortControl_regidx[port], SSW_INBW_RATE, pRate)) != RT_ERR_OK)
    if ((ret = reg_array_field_read(unit, SSW_INPUT_BANDWIDTH_PORT_CONTROLr, port, 0, SSW_INBW_RATEf, pRate)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    RATE_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "pRate=%d",
           *pRate);  
    
    return RT_ERR_OK;
} /* end of dal_ssw_rate_igrBandwidthCtrlRate_get */

/* Function Name:
 *      dal_ssw_rate_igrBandwidthCtrlRate_set
 * Description:
 *      Set the ingress bandwidth control rate.
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      rate        - ingress bandwidth control rate
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_RATE          - Invalid input rate
 * Note:
 *    The actual rate is "rate * chip granularity".
 *    The unit of granularity in RTL8389/RTL8329 is 16Kbps.
 */
int32
dal_ssw_rate_igrBandwidthCtrlRate_set(uint32 unit, rtk_port_t port, uint32 rate)
{
    int32   ret;
    uint32  old_rate;
    rtk_enable_t  port_admin_status;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, rate=%d",
           unit, port, rate);  
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((rate > HAL_RATE_OF_BANDWIDTH_MAX(unit)), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
       
    RATE_SEM_LOCK(unit);
    
    /* E0005361 */
    //if ((ret = reg_field_read(unit, (uint32)igrBandwidthPortControl_regidx[port], SSW_INBW_RATE, &old_rate)) != RT_ERR_OK)
    if ((ret = reg_array_field_read(unit, SSW_INPUT_BANDWIDTH_PORT_CONTROLr, port, 0, SSW_INBW_RATEf, &old_rate)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    if ((ret = dal_ssw_port_adminEnable_get(unit, port, &port_admin_status)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }            
    
    if ((old_rate != rate) && (DISABLED != port_admin_status))
    {        
        if ((ret = dal_ssw_port_rxEnable_set(unit, port, DISABLED)) != RT_ERR_OK)
        {
            RATE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
            return ret;
        } 
        osal_time_usleep(1200);
    }
    /* End of E0005361 */
    
    /* program value to CHIP*/
    //if ((ret = reg_field_write(unit, (uint32)igrBandwidthPortControl_regidx[port], SSW_INBW_RATE, rate)) != RT_ERR_OK)
    if ((ret = reg_array_field_write(unit, SSW_INPUT_BANDWIDTH_PORT_CONTROLr, port, 0, SSW_INBW_RATEf, &rate)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
 
    /* E0005361 */
    if ((old_rate != rate) && (DISABLED != port_admin_status))
    {
        if ((ret = dal_ssw_port_rxEnable_set(unit, port, ENABLED)) != RT_ERR_OK)
        {
            RATE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
            return ret;
        }
    }
    /* End of E0005361 */  

    RATE_SEM_UNLOCK(unit);
   
    return RT_ERR_OK;
} /* end of dal_ssw_rate_igrBandwidthCtrlRate_set */

/* Function Name:
 *      dal_ssw_rate_igrBandwidthCtrlIncludeIfg_get
 * Description:
 *      Get the status of ingress bandwidth control includes IFG or not.
 * Input:
 *      unit                    - unit id
 * Output:
 *      pIfg_include           - include IFG or not
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID          - Invalid unit id
 *      RT_ERR_NULL_POINTER     - NULL pointer
 * Note:
 *      1. Ingress bandwidth control includes/excludes the Preamble & IFG (20 Bytes).
 *
 *      2. The status of ifg_include:
 *         - DISABLED
 *         - ENABLED
 */
int32
dal_ssw_rate_igrBandwidthCtrlIncludeIfg_get(uint32 unit, rtk_enable_t *pIfg_include)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d", unit);  
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pIfg_include), RT_ERR_NULL_POINTER);
    
    RATE_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, SSW_INPUT_BANDWIDTH_CONTROL_GLOBAL_CONTROL0r, SSW_INBW_INC_IFGf, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    RATE_SEM_UNLOCK(unit);
    
    /* chip's value translate */
    switch (value)
    {
        case 0:
            *pIfg_include = DISABLED;
            break;
        case 1:
            *pIfg_include = ENABLED;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "pIfg_include=%d", *pIfg_include);  
    
    return RT_ERR_OK;
} /* end of dal_ssw_rate_igrBandwidthCtrlIncludeIfg_get */

/* Function Name:
 *      dal_ssw_rate_igrBandwidthCtrlIncludeIfg_set
 * Description:
 *      Set the status of ingress bandwidth control includes IFG or not.
 * Input:
 *      unit                    - unit id
 *      ifg_include             - include IFG or not
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - Invalid unit id
 *      RT_ERR_INPUT                 - Invalid input parameter
 * Note:
 *      1. Ingress bandwidth control includes/excludes the Preamble & IFG (20 Bytes).
 *
 *      2. The status of ifg_include:
 *         - DISABLED
 *         - ENABLED
 */
int32
dal_ssw_rate_igrBandwidthCtrlIncludeIfg_set(uint32 unit, rtk_enable_t ifg_include)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, ifg_include=%d", 
           unit, ifg_include);  
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(ifg_include >= RTK_ENABLE_END, RT_ERR_INPUT);
    
    /* translate to chip value */
    switch (ifg_include)
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
        
    RATE_SEM_LOCK(unit);
    
    /* program value into CHIP*/
    if ((ret = reg_field_write(unit, SSW_INPUT_BANDWIDTH_CONTROL_GLOBAL_CONTROL0r, SSW_INBW_INC_IFGf, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    RATE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_rate_igrBandwidthCtrlIncludeIfg_set */

/* Function Name:
 *      dal_ssw_rate_egrBandwidthCtrlEnable_get
 * Description:
 *      Get the egress bandwidth control status.
 * Input:
 *      unit                - unit id
 *      port                - port id
 * Output:
 *      pEnable            - status of egress bandwidth control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_NULL_POINTER  - NULL pointer
 * Note:
 *    The status of egress bandwidth control is as following:
 *    - DISABLED
 *    - ENABLED
 */
int32
dal_ssw_rate_egrBandwidthCtrlEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d", 
           unit, port);      
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    RATE_SEM_LOCK(unit);
    
    *pEnable = egr_bandwidth_ctrl[unit][port].status;
    
    RATE_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "pEnable=%d", 
           *pEnable);   
    
    return RT_ERR_OK;
} /* end of dal_ssw_rate_egrBandwidthCtrlEnable_get */

/* Function Name:
 *      dal_ssw_rate_egrBandwidthCtrlEnable_set
 * Description:
 *      Set the egress bandwidth control status.
 * Input:
 *      unit                - unit id
 *      port                - port id
 *      enable              - status of egress bandwidth control
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_INPUT     - Invalid input parameter
 * Note:
 *    The status of egress bandwidth control is as following:
 *    - DISABLED
 *    - ENABLED
 */
int32
dal_ssw_rate_egrBandwidthCtrlEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  rate;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, \
           enable=%d", unit, port, enable);
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(enable >= RTK_ENABLE_END, RT_ERR_INPUT);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    if (enable == egr_bandwidth_ctrl[unit][port].status)
    {
        /* same status as original, no need to process */
        return RT_ERR_OK;
    }
    
    /* translate to chip value */
    switch (enable)
    {
        case DISABLED:
            rate = DAL_SSW_RATE_DISABLED_EGR_BANDWIDTH_RATE;
            break;
        case ENABLED:
            rate = egr_bandwidth_ctrl[unit][port].rate;
            break;
        default:
            return RT_ERR_FAILED;
    }    
    
    RATE_SEM_LOCK(unit);
    
    /* program value into CHIP*/
    if ((ret = reg_field_write(unit, (uint32)wfqwrrPortParameter0_regidx[port], (uint32)wfqwrr_rate_fieldidx[port], &rate)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    egr_bandwidth_ctrl[unit][port].status = enable;
    
    RATE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_rate_egrBandwidthCtrlEnable_set */

/* Function Name:
 *      dal_ssw_rate_egrBandwidthCtrlRate_get
 * Description:
 *      Get the egress bandwidth control rate.
 * Input:
 *      unit            - unit id
 *      port            - port id
 * Output:
 *      pRate          - egress bandwidth control rate
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_NULL_POINTER  - NULL pointer
 * Note:
 *    The actual rate is "rate * chip granularity".
 *    The unit of granularity in RTL8389/RTL8329 is 16Kbps.
 */
int32
dal_ssw_rate_egrBandwidthCtrlRate_get(uint32 unit, rtk_port_t port, uint32 *pRate)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d", 
           unit, port);

    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pRate), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    RATE_SEM_LOCK(unit);
    
    *pRate = egr_bandwidth_ctrl[unit][port].rate;
    
    RATE_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "pRate=%d", *pRate);
    
    return RT_ERR_OK;
} /* end of dal_ssw_rate_egrBandwidthCtrlRate_get */

/* Function Name:
 *      dal_ssw_rate_egrBandwidthCtrlRate_set
 * Description:
 *      Set the egress bandwidth control rate.
 * Input:
 *      unit                - unit id
 *      port                - port id
 *      rate                - egress bandwidth control rate
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_RATE          - Invalid input rate
 * Note:
 *    The actual rate is "rate * chip granularity".
 *    The unit of granularity in RTL8389/RTL8329 is 16Kbps.
 */
int32
dal_ssw_rate_egrBandwidthCtrlRate_set(uint32 unit, rtk_port_t port, uint32 rate)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, rate=%d",
           unit, port, rate);
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(rate > HAL_RATE_OF_BANDWIDTH_MAX(unit), RT_ERR_RATE);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    if (DISABLED == egr_bandwidth_ctrl[unit][port].status)
    {
        /* if disabled, only config it in database. */
        egr_bandwidth_ctrl[unit][port].rate = rate;
        return RT_ERR_OK;
    }
       
    RATE_SEM_LOCK(unit);
    
    /* program value into CHIP*/
    if ((ret = reg_field_write(unit, (uint32)wfqwrrPortParameter0_regidx[port], (uint32)wfqwrr_rate_fieldidx[port], &rate)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    egr_bandwidth_ctrl[unit][port].rate = rate;
    
    RATE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_rate_egrBandwidthCtrlRate_set */

/* Function Name:
 *      dal_ssw_rate_egrBandwidthCtrlIncludeIfg_get
 * Description:
 *      Get the status of egress bandwidth control includes IFG or not.
 * Input:
 *      unit                    - unit id
 * Output:
 *      pIfg_include           - include IFG or not
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID          - Invalid unit id
 *      RT_ERR_INPUT            - Invalid input parameter
 *      RT_ERR_NULL_POINTER     - NULL pointer
 * Note:
 *      1. Egress bandwidth control includes/excludes the Preamble & IFG (20 Bytes).
 *
 *      2. The status of ifg_include:
 *         - DISABLED
 *         - ENABLED
 */
int32
dal_ssw_rate_egrBandwidthCtrlIncludeIfg_get(uint32 unit, rtk_enable_t *pIfg_include)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pIfg_include), RT_ERR_NULL_POINTER);
    
    RATE_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, SSW_PACKET_SCHEDULING_GLOBAL_CONTROLr, SSW_INC_IFGf, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    RATE_SEM_UNLOCK(unit);
    
    /* chip's value translate */
    switch (value)
    {
        case 0:
            *pIfg_include = DISABLED;
            break;
        case 1:
            *pIfg_include = ENABLED;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "pIfg_include=%d", *pIfg_include);
    
    return RT_ERR_OK;
} /* end of dal_ssw_rate_egrBandwidthCtrlIncludeIfg_get */

/* Function Name:
 *      dal_ssw_rate_egrBandwidthCtrlIncludeIfg_set
 * Description:
 *      Set the status of egress bandwidth control includes IFG or not.
 * Input:
 *      unit                    - unit id
 *      ifg_include             - include IFG or not
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - Invalid unit id
 *      RT_ERR_INPUT                 - Invalid input parameter
 * Note:
 *      1. Egress bandwidth control includes/excludes the Preamble & IFG (20 Bytes).
 *
 *      2. The status of ifg_include:
 *         - DISABLED
 *         - ENABLED
 */
int32
dal_ssw_rate_egrBandwidthCtrlIncludeIfg_set(uint32 unit, rtk_enable_t ifg_include)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, ifg_include=%d", unit,
           ifg_include);
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(ifg_include >= RTK_ENABLE_END, RT_ERR_INPUT);
    
    /* translate to chip value */
    switch (ifg_include)
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
    
    RATE_SEM_LOCK(unit);
    
    /* program value into CHIP*/
    if ((ret = reg_field_write(unit, SSW_PACKET_SCHEDULING_GLOBAL_CONTROLr, SSW_INC_IFGf, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    RATE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_rate_egrBandwidthCtrlIncludeIfg_set */

/* Function Name:
 *      dal_ssw_rate_stormControlRate_get
 * Description:
 *      Get the storm control rate.
 * Input:
 *      unit            - unit id
 *      port            - port id
 *      storm_type      - storm group type
 * Output:
 *      pRate          - storm control rate (packet-per-second).
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_INPUT         - Invalid input parameter
 *      RT_ERR_NULL_POINTER  - NULL pointer
 * Note:
 *    The storm group types are as following:
 *    - STORM_GROUP_UNKNOWN_UNICAST
 *    - STORM_GROUP_UNKNOWN_MULTICAST
 *    - STORM_GROUP_MULTICAST
 *    - STORM_GROUP_BROADCAST
 */
int32
dal_ssw_rate_stormControlRate_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    uint32                  *pRate)
{
    int32   ret;
    uint32  reg_idx;
    uint32  field_idx;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, storm_type=%d", 
           unit, port, storm_type);
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pRate), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((storm_type > STORM_GROUP_END), RT_ERR_INPUT);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    switch (storm_type)
    {
        case STORM_GROUP_UNKNOWN_UNICAST:
            reg_idx = stormFilteringControl0_regidx[port];
            field_idx = uknUniRate_fieldidx[port];
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            reg_idx = stormFilteringControl1_regidx[port];
            field_idx = uknMultiRate_fieldidx[port];
            break;
        case STORM_GROUP_MULTICAST:
            reg_idx = stormFilteringControl2_regidx[port];
            field_idx = multiRate_fieldidx[port];
            break;
        case STORM_GROUP_BROADCAST:
            reg_idx = stormFilteringControl3_regidx[port];
            field_idx = bcstRate_fieldidx[port];
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RATE_SEM_LOCK(unit);
    
    /* read value from CHIP*/
    if ((ret = reg_field_read(unit, reg_idx, field_idx, pRate)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    RATE_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "pRate=%d", *pRate);
    
    return RT_ERR_OK;
} /* end of dal_ssw_rate_stormControlRate_get */

/* Function Name:
 *      dal_ssw_rate_stormControlRate_set
 * Description:
 *      Set the storm control rate.
 * Input:
 *      unit            - unit id
 *      port            - port id
 *      storm_type      - storm group type
 *      rate            - storm control rate (packet-per-second).
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_INPUT         - Invalid input parameter
 *      RT_ERR_RATE          - Invalid input bandwidth
 * Note:
 *    The storm group types are as following:
 *    - STORM_GROUP_UNKNOWN_UNICAST
 *    - STORM_GROUP_UNKNOWN_MULTICAST
 *    - STORM_GROUP_MULTICAST
 *    - STORM_GROUP_BROADCAST
 */
int32
dal_ssw_rate_stormControlRate_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    uint32                  rate)
{
    int32   ret;
    uint32  reg_idx;
    uint32  field_idx;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, storm_type=%d\
           rate=%d", unit, port, storm_type, rate); 
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((rate > HAL_RATE_OF_STORM_CONTROL_MAX(unit)), RT_ERR_RATE);
    RT_PARAM_CHK((storm_type > STORM_GROUP_END), RT_ERR_INPUT);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    switch (storm_type)
    {
        case STORM_GROUP_UNKNOWN_UNICAST:
            reg_idx = stormFilteringControl0_regidx[port];
            field_idx = uknUniRate_fieldidx[port];
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            reg_idx = stormFilteringControl1_regidx[port];
            field_idx = uknMultiRate_fieldidx[port];
            break;
        case STORM_GROUP_MULTICAST:
            reg_idx = stormFilteringControl2_regidx[port];
            field_idx = multiRate_fieldidx[port];
            break;
        case STORM_GROUP_BROADCAST:
            reg_idx = stormFilteringControl3_regidx[port];
            field_idx = bcstRate_fieldidx[port];
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RATE_SEM_LOCK(unit);
    
    /* program value to CHIP*/
    if ((ret = reg_field_write(unit, reg_idx, field_idx, &rate)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    RATE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_rate_stormControlRate_set */


/* Function Name:
 *      _dal_ssw_rate_init_config
 * Description:
 *      Initialize default configuration for  the rate module of the specified device..
 * Input:
 *      unit                - unit id
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
static int32
_dal_ssw_rate_init_config(uint32 unit)
{
    int32   ret;
    rtk_port_t  port, max_port;
    
    max_port = HAL_GET_MAX_PORT(unit);
    
    for(port = 0; port < max_port; port++)
    {
        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }
        if ((ret = dal_ssw_rate_egrBandwidthCtrlEnable_set(unit, port, RTK_DEFAULT_EGR_BANDWIDTH_CTRL_STATUS)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
            return ret;
        }
            
        if ((ret = dal_ssw_rate_egrBandwidthCtrlRate_set(unit, port, RTK_DEFAULT_EGR_BANDWIDTH_CTRL_RATE)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
            return ret;
        }
        
        /* E0005366 */
        if ((ret = _dal_ssw_rate_igrBandwidthCtrlAct_set(unit, port, IGR_ACT_DROP)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
            return ret;
        }
        /* end of E0005366 */
        
        /* E0005367 */
        if ((ret = _dal_ssw_rate_igrBandwidthCtrlFCCnt_set(unit, port, 8)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
            return ret;
        }
        /* End of E0005367 */
    }
    
    return RT_ERR_OK;
} /* end of _dal_ssw_rate_init_config */


/* Function Name:
 *      _dal_ssw_rate_igrBandwidthCtrlAct_set
 * Description:
 *      Set the action of ingress bandwidth control.
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      act         - ingress bandwidth control action after PAUSE ON frame is sent
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_INPUT         - Invalid input parameter
 * Note:
 *    The unit of granularity in RTL8389/RTL8329 is 16Kbps.
 */
static int32
_dal_ssw_rate_igrBandwidthCtrlAct_set(uint32 unit, rtk_port_t port, dal_ssw_rate_igr_bandwidth_ctrl_act_t act)
{
    int32   ret;
    uint32  value;
    
    RT_LOG(LOG_TRACE, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, act=%d", 
           unit, port, act); 
        
    if (IGR_ACT_RCV == act)
    {
        value = 0;
    } 
    else 
    {
        value = 1;
    }
    
    RATE_SEM_LOCK(unit);
    
    /* program value to CHIP*/
    //if ((ret = reg_field_write(unit, (uint32)igrBandwidthPortControl_regidx[port], SSW_INBW_ACT, value)) != RT_ERR_OK)
    if ((ret = reg_array_field_write(unit, SSW_INPUT_BANDWIDTH_PORT_CONTROLr, port, 0, SSW_INBW_ACTf, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    RATE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* _dal_ssw_rate_igrBandwidthCtrlAct_set */

/* Function Name:
 *      _dal_ssw_rate_igrBandwidthCtrlFCCnt_set
 * Description:
 *      Set the action of ingress bandwidth control.
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      fccnt       - when ingress bandwidth control set to drop, the packet count allowed to be received 
 *                    after PAUSE ON frame is sent.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_RATE          - Invalid input rate
 * Note:
 *    The unit of granularity in RTL8389/RTL8329 is 16Kbps.
 */
static int32
_dal_ssw_rate_igrBandwidthCtrlFCCnt_set(uint32 unit, rtk_port_t port, uint32 fccnt)
{
    int32   ret;
    
    RT_LOG(LOG_TRACE, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, fccnt=%d", 
           unit, port, fccnt); 
           
    RATE_SEM_LOCK(unit);
    
    /* program value to CHIP*/
    //if ((ret = reg_field_write(unit, (uint32)igrBandwidthPortControl_regidx[port], SSW_INBW_FCCNT, fccnt)) != RT_ERR_OK)
    if ((ret = reg_array_field_write(unit, SSW_INPUT_BANDWIDTH_PORT_CONTROLr, port, 0, SSW_INBW_FCCNTf, &fccnt)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    RATE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* _dal_ssw_rate_igrBandwidthCtrlAct_set */


/* Function Name:
 *      dal_ssw_rate_igrBandwidthFCOffRate_set
 * Description:
 *      Set the ingress bandwidth control flow control off rate.
 * Input:
 *      unit - unit id
 *      rate - ingress bandwidth control flow control off rate
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - Invalid unit id
 *      RT_ERR_RATE    - Invalid input rate
 * Note:
 *      None
 */
int32
dal_ssw_rate_igrBandwidthFCOffRate_set(uint32 unit, uint32 rate)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, rate=%d",
           unit, rate);
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(rate > 0x3FFF, RT_ERR_RATE);
    
    RATE_SEM_LOCK(unit);
    
    /* program value into CHIP*/
    if ((ret = reg_field_write(unit, SSW_INPUT_BANDWIDTH_CONTROL_GLOBAL_CONTROL1r, SSW_INBW_FC_OFFf, &rate)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    RATE_SEM_UNLOCK(unit);
    return RT_ERR_OK;
} /* end of dal_ssw_rate_igrBandwidthFCOffRate_set */

