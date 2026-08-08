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
 * Purpose : Definition those public global APIs and its data type in the SDK.
 *
 * Feature : (1) Include chip-supported conditions for flow control on/off
 *           (2) Get/set the threshold parameters for the flow control on/off
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
#include <dal/ssw/dal_ssw_flowctrl.h>
#include <rtk/default.h>
#include <rtk/flowctrl.h>

/* 
 * Symbol Definition 
 */


/* 
 * Data Declaration 
 */
static uint32               flowctrl_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         flowctrl_sem[RTK_MAX_NUM_OF_UNIT];


const static uint16 flow_control_threshold_for_control0_regidx[] = 
{SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_0_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_1_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_2_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_3_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_4_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_5_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_6_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_7_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_8_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_9_CONTROL0r\
,SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_10_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_11_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_12_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_13_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_14_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_15_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_16_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_17_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_18_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_19_CONTROL0r\
,SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_20_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_21_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_22_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_23_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_24_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_25_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_26_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_27_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_28_CONTROL0r};

const static uint16 flow_control_threshold_for_control1_regidx[] = 
{SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_0_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_1_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_2_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_3_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_4_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_5_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_6_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_7_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_8_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_9_CONTROL1r\
,SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_10_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_11_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_12_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_13_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_14_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_15_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_16_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_17_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_18_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_19_CONTROL1r\
,SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_20_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_21_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_22_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_23_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_24_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_25_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_26_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_27_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_28_CONTROL1r};

const static uint16 flow_control_threshold_for_control2_regidx[] = 
{SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_0_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_1_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_2_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_3_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_4_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_5_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_6_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_7_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_8_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_9_CONTROL2r\
,SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_10_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_11_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_12_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_13_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_14_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_15_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_16_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_17_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_18_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_19_CONTROL2r\
,SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_20_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_21_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_22_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_23_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_24_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_25_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_26_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_27_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_28_CONTROL2r};

const static uint16 per_port_egress_drop_threshold_control_regidx[] = 
{  SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL0r, SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL0r\
 , SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL1r, SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL1r\
 , SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL2r, SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL2r\
 , SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL3r, SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL3r\
 , SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL4r, SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL4r\
 , SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL5r, SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL5r\
 , SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL6r, SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL6r\
 , SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL7r, SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL7r\
 , SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL8r, SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL8r\
 , SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL9r, SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL9r\
 , SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL10r, SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL10r\
 , SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL11r, SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL11r\
 , SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL12r, SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL12r\
 , SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL13r, SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL13r\
 , SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL14r};

const static uint16 per_queue_egress_drop_threshold_control_regidx[] = 
{  SSW_PER_QUEUE_EGRESS_DROP_THRESHOLD_CONTROL0r, SSW_PER_QUEUE_EGRESS_DROP_THRESHOLD_CONTROL0r\
 , SSW_PER_QUEUE_EGRESS_DROP_THRESHOLD_CONTROL1r, SSW_PER_QUEUE_EGRESS_DROP_THRESHOLD_CONTROL1r\
 , SSW_PER_QUEUE_EGRESS_DROP_THRESHOLD_CONTROL2r, SSW_PER_QUEUE_EGRESS_DROP_THRESHOLD_CONTROL2r\
 , SSW_PER_QUEUE_EGRESS_DROP_THRESHOLD_CONTROL3r, SSW_PER_QUEUE_EGRESS_DROP_THRESHOLD_CONTROL3r};

const static uint16 th_port_high_on_fieldidx[] = {SSW_TH_PORT0_HIGH_ONf, SSW_TH_PORT1_HIGH_ONf, SSW_TH_PORT2_HIGH_ONf, SSW_TH_PORT3_HIGH_ONf, SSW_TH_PORT4_HIGH_ONf, SSW_TH_PORT5_HIGH_ONf, SSW_TH_PORT6_HIGH_ONf, SSW_TH_PORT7_HIGH_ONf, SSW_TH_PORT8_HIGH_ONf, SSW_TH_PORT9_HIGH_ONf\
                         ,SSW_TH_PORT10_HIGH_ONf, SSW_TH_PORT11_HIGH_ONf, SSW_TH_PORT12_HIGH_ONf, SSW_TH_PORT13_HIGH_ONf, SSW_TH_PORT14_HIGH_ONf, SSW_TH_PORT15_HIGH_ONf, SSW_TH_PORT16_HIGH_ONf, SSW_TH_PORT17_HIGH_ONf, SSW_TH_PORT18_HIGH_ONf, SSW_TH_PORT19_HIGH_ONf\
                         ,SSW_TH_PORT20_HIGH_ONf, SSW_TH_PORT21_HIGH_ONf, SSW_TH_PORT22_HIGH_ONf, SSW_TH_PORT23_HIGH_ONf, SSW_TH_PORT24_HIGH_ONf, SSW_TH_PORT25_HIGH_ONf, SSW_TH_PORT26_HIGH_ONf, SSW_TH_PORT27_HIGH_ONf, SSW_TH_PORT28_HIGH_ONf};
const static uint16 th_port_high_off_fieldidx[] = {SSW_TH_PORT0_HIGH_OFFf, SSW_TH_PORT1_HIGH_OFFf, SSW_TH_PORT2_HIGH_OFFf, SSW_TH_PORT3_HIGH_OFFf, SSW_TH_PORT4_HIGH_OFFf, SSW_TH_PORT5_HIGH_OFFf, SSW_TH_PORT6_HIGH_OFFf, SSW_TH_PORT7_HIGH_OFFf, SSW_TH_PORT8_HIGH_OFFf, SSW_TH_PORT9_HIGH_OFFf\
                         ,SSW_TH_PORT10_HIGH_OFFf, SSW_TH_PORT11_HIGH_OFFf, SSW_TH_PORT12_HIGH_OFFf, SSW_TH_PORT13_HIGH_OFFf, SSW_TH_PORT14_HIGH_OFFf, SSW_TH_PORT15_HIGH_OFFf, SSW_TH_PORT16_HIGH_OFFf, SSW_TH_PORT17_HIGH_OFFf, SSW_TH_PORT18_HIGH_OFFf, SSW_TH_PORT19_HIGH_OFFf\
                         ,SSW_TH_PORT20_HIGH_OFFf, SSW_TH_PORT21_HIGH_OFFf, SSW_TH_PORT22_HIGH_OFFf, SSW_TH_PORT23_HIGH_OFFf, SSW_TH_PORT24_HIGH_OFFf, SSW_TH_PORT25_HIGH_OFFf, SSW_TH_PORT26_HIGH_OFFf, SSW_TH_PORT27_HIGH_OFFf, SSW_TH_PORT28_HIGH_OFFf};
const static uint16 th_port_low_fieldidx[] = {SSW_PORT0_LOWf, SSW_PORT1_LOWf, SSW_PORT2_LOWf, SSW_PORT3_LOWf, SSW_PORT4_LOWf, SSW_PORT5_LOWf, SSW_PORT6_LOWf, SSW_PORT7_LOWf, SSW_PORT8_LOWf, SSW_PORT9_LOWf\
                         ,SSW_PORT10_LOWf, SSW_PORT11_LOWf, SSW_PORT12_LOWf, SSW_PORT13_LOWf, SSW_PORT14_LOWf, SSW_PORT15_LOWf, SSW_PORT16_LOWf, SSW_PORT17_LOWf, SSW_PORT18_LOWf, SSW_PORT19_LOWf\
                         ,SSW_PORT20_LOWf, SSW_PORT21_LOWf, SSW_PORT22_LOWf, SSW_PORT23_LOWf, SSW_PORT24_LOWf, SSW_PORT25_LOWf, SSW_PORT26_LOWf, SSW_PORT27_LOWf, SSW_PORT28_LOWf};

const static uint16 th_port_high_fcoff_on_fieldidx[] = {SSW_TH_PORT0_FCOFF_HIGH_ONf, SSW_TH_PORT1_FCOFF_HIGH_ONf, SSW_TH_PORT2_FCOFF_HIGH_ONf, SSW_TH_PORT3_FCOFF_HIGH_ONf, SSW_TH_PORT4_FCOFF_HIGH_ONf, SSW_TH_PORT5_FCOFF_HIGH_ONf, SSW_TH_PORT6_FCOFF_HIGH_ONf, SSW_TH_PORT7_FCOFF_HIGH_ONf, SSW_TH_PORT8_FCOFF_HIGH_ONf, SSW_TH_PORT9_FCOFF_HIGH_ONf\
                         ,SSW_TH_PORT10_FCOFF_HIGH_ONf, SSW_TH_PORT11_FCOFF_HIGH_ONf, SSW_TH_PORT12_FCOFF_HIGH_ONf, SSW_TH_PORT13_FCOFF_HIGH_ONf, SSW_TH_PORT14_FCOFF_HIGH_ONf, SSW_TH_PORT15_FCOFF_HIGH_ONf, SSW_TH_PORT16_FCOFF_HIGH_ONf, SSW_TH_PORT17_FCOFF_HIGH_ONf, SSW_TH_PORT18_FCOFF_HIGH_ONf, SSW_TH_PORT19_FCOFF_HIGH_ONf\
                         ,SSW_TH_PORT20_FCOFF_HIGH_ONf, SSW_TH_PORT21_FCOFF_HIGH_ONf, SSW_TH_PORT22_FCOFF_HIGH_ONf, SSW_TH_PORT23_FCOFF_HIGH_ONf, SSW_TH_PORT24_FCOFF_HIGH_ONf, SSW_TH_PORT25_FCOFF_HIGH_ONf, SSW_TH_PORT26_FCOFF_HIGH_ONf, SSW_TH_PORT27_FCOFF_HIGH_ONf, SSW_TH_PORT28_FCOFF_HIGH_ONf};
const static uint16 th_port_high_fcoff_off_fieldidx[] = {SSW_TH_PORT0_FCOFF_HIGH_OFFf, SSW_TH_PORT1_FCOFF_HIGH_OFFf, SSW_TH_PORT2_FCOFF_HIGH_OFFf, SSW_TH_PORT3_FCOFF_HIGH_OFFf, SSW_TH_PORT4_FCOFF_HIGH_OFFf, SSW_TH_PORT5_FCOFF_HIGH_OFFf, SSW_TH_PORT6_FCOFF_HIGH_OFFf, SSW_TH_PORT7_FCOFF_HIGH_OFFf, SSW_TH_PORT8_FCOFF_HIGH_OFFf, SSW_TH_PORT9_FCOFF_HIGH_OFFf\
                         ,SSW_TH_PORT10_FCOFF_HIGH_OFFf, SSW_TH_PORT11_FCOFF_HIGH_OFFf, SSW_TH_PORT12_FCOFF_HIGH_OFFf, SSW_TH_PORT13_FCOFF_HIGH_OFFf, SSW_TH_PORT14_FCOFF_HIGH_OFFf, SSW_TH_PORT15_FCOFF_HIGH_OFFf, SSW_TH_PORT16_FCOFF_HIGH_OFFf, SSW_TH_PORT17_FCOFF_HIGH_OFFf, SSW_TH_PORT18_FCOFF_HIGH_OFFf, SSW_TH_PORT19_FCOFF_HIGH_OFFf\
                         ,SSW_TH_PORT20_FCOFF_HIGH_OFFf, SSW_TH_PORT21_FCOFF_HIGH_OFFf, SSW_TH_PORT22_FCOFF_HIGH_OFFf, SSW_TH_PORT23_FCOFF_HIGH_OFFf, SSW_TH_PORT24_FCOFF_HIGH_OFFf, SSW_TH_PORT25_FCOFF_HIGH_OFFf, SSW_TH_PORT26_FCOFF_HIGH_OFFf, SSW_TH_PORT27_FCOFF_HIGH_OFFf, SSW_TH_PORT28_FCOFF_HIGH_OFFf};
const static uint16 th_port_fcoff_low_fieldidx[] = {SSW_PORT0_FCOFF_LOWf, SSW_PORT1_FCOFF_LOWf, SSW_PORT2_FCOFF_LOWf, SSW_PORT3_FCOFF_LOWf, SSW_PORT4_FCOFF_LOWf, SSW_PORT5_FCOFF_LOWf, SSW_PORT6_FCOFF_LOWf, SSW_PORT7_FCOFF_LOWf, SSW_PORT8_FCOFF_LOWf, SSW_PORT9_FCOFF_LOWf\
                         ,SSW_PORT10_FCOFF_LOWf, SSW_PORT11_FCOFF_LOWf, SSW_PORT12_FCOFF_LOWf, SSW_PORT13_FCOFF_LOWf, SSW_PORT14_FCOFF_LOWf, SSW_PORT15_FCOFF_LOWf, SSW_PORT16_FCOFF_LOWf, SSW_PORT17_FCOFF_LOWf, SSW_PORT18_FCOFF_LOWf, SSW_PORT19_FCOFF_LOWf\
                         ,SSW_PORT20_FCOFF_LOWf, SSW_PORT21_FCOFF_LOWf, SSW_PORT22_FCOFF_LOWf, SSW_PORT23_FCOFF_LOWf, SSW_PORT24_FCOFF_LOWf, SSW_PORT25_FCOFF_LOWf, SSW_PORT26_FCOFF_LOWf, SSW_PORT27_FCOFF_LOWf, SSW_PORT28_FCOFF_LOWf};

const static uint16 th_ppe_drop_on_fieldidx[] = {SSW_TH_PP0E_DROPf, SSW_TH_PP1E_DROPf, SSW_TH_PP2E_DROPf, SSW_TH_PP3E_DROPf, SSW_TH_PP4E_DROPf, SSW_TH_PP5E_DROPf, SSW_TH_PP6E_DROPf, SSW_TH_PP7E_DROPf, SSW_TH_PP8E_DROPf, SSW_TH_PP9E_DROPf\
                         ,SSW_TH_PP10E_DROPf, SSW_TH_PP11E_DROPf, SSW_TH_PP12E_DROPf, SSW_TH_PP13E_DROPf, SSW_TH_PP14E_DROPf, SSW_TH_PP15E_DROPf, SSW_TH_PP16E_DROPf, SSW_TH_PP17E_DROPf, SSW_TH_PP18E_DROPf, SSW_TH_PP19E_DROPf\
                         ,SSW_TH_PP20E_DROPf, SSW_TH_PP21E_DROPf, SSW_TH_PP22E_DROPf, SSW_TH_PP23E_DROPf, SSW_TH_PP24E_DROPf, SSW_TH_PP25E_DROPf, SSW_TH_PP26E_DROPf, SSW_TH_PP27E_DROPf, SSW_TH_PP28E_DROPf};

const static uint16 th_pqe_drop_on_fieldidx[] = {SSW_TH_PQ0E_DROPf, SSW_TH_PQ1E_DROPf, SSW_TH_PQ2E_DROPf, SSW_TH_PQ3E_DROPf, SSW_TH_PQ4E_DROPf, SSW_TH_PQ5E_DROPf, SSW_TH_PQ6E_DROPf, SSW_TH_PQ7E_DROPf};
/*
 * Macro Definition
 */
/* flowctrl semaphore handling */
#define FLOWCTRL_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(flowctrl_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_DAL|MOD_FLOWCTRL), "semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)
#define FLOWCTRL_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(flowctrl_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_DAL|MOD_FLOWCTRL), "semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)


/* 
 * Function Declaration 
 */

/* Function Name:
 *      dal_ssw_flowctrl_init
 * Description:
 *      Initialize flowctrl module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      1. Module must be initialized before using all of APIs in this module
 */
int32
dal_ssw_flowctrl_init(uint32 unit)
{
    flowctrl_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    flowctrl_sem[unit] = osal_sem_mutex_create();
    if (0 == flowctrl_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_FLOWCTRL), "semaphore create failed");
        return RT_ERR_FAILED;
    }
    
    /* set init flag to complete init */
    flowctrl_init[unit] = INIT_COMPLETED;
    
    /* Initialized configuration */

    return RT_ERR_OK;
} /* end of dal_ssw_flowctrl_init */

/*
 * Flow Control ON
 */

/* Function Name:
 *      dal_ssw_flowctrl_igrSystemPauseThresh_get
 * Description:
 *      Get ingress system used page high/low threshold paramters of the specific unit
 * Input:
 *      unit    - unit id
 * Output:
 *      pThresh - pointer to the threshold structure in the system used page count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_ssw_flowctrl_igrSystemPauseThresh_get(uint32 unit, rtk_flowctrl_thresh_t *pThresh)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pThresh), RT_ERR_NULL_POINTER);
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_read(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL1r, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_get(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL1r, SSW_TH_GLOBAL_HIGH_ONf, &(pThresh->highOn), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_get(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL1r, SSW_TH_GLOBAL_HIGH_OFFf, &(pThresh->highOff), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    /* get value from CHIP*/
    if ((ret = reg_read(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL2r, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_get(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL2r, SSW_TH_GLOBAL_LOW_ONf, &(pThresh->lowOn), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_get(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL2r, SSW_TH_GLOBAL_LOW_OFFf, &(pThresh->lowOff), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    FLOWCTRL_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "highOn=0x%x, highOff=0x%x\
           lowOn=0x%x, lowOff=0x%x", pThresh->highOn, pThresh->highOff, pThresh->lowOn, pThresh->lowOff);

    return RT_ERR_OK;
} /* end of dal_ssw_flowctrl_igrSystemPauseThresh_get */

/* Function Name:
 *      dal_ssw_flowctrl_igrSystemPauseThresh_set
 * Description:
 *      Set ingress system used page high/low threshold paramters of the specific unit
 * Input:
 *      unit    - unit id
 *      pThresh - pointer to the threshold structure in the system used page count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
int32
dal_ssw_flowctrl_igrSystemPauseThresh_set(uint32 unit, rtk_flowctrl_thresh_t *pThresh)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pThresh), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(pThresh->highOn > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pThresh->highOff > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pThresh->lowOn > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pThresh->lowOff > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    
    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d \
           highOn=0x%x, highOff=0x%x, lowOn=0x%x, lowOff=0x%x", unit, pThresh->highOn, pThresh->highOff, 
           pThresh->lowOn, pThresh->lowOff);    
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_read(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL1r, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_set(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL1r, SSW_TH_GLOBAL_HIGH_ONf, &(pThresh->highOn), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_set(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL1r, SSW_TH_GLOBAL_HIGH_OFFf, &(pThresh->highOff), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    /* program value to CHIP*/
    if ((ret = reg_write(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL1r, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    /* get value from CHIP*/
    if ((ret = reg_read(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL2r, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_set(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL2r, SSW_TH_GLOBAL_LOW_ONf, &(pThresh->lowOn), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_set(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL2r, SSW_TH_GLOBAL_LOW_OFFf, &(pThresh->lowOff), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
     /* program value to CHIP*/
    if ((ret = reg_write(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL2r, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    FLOWCTRL_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_flowctrl_igrSystemPauseThresh_set */


/* Function Name:
 *      dal_ssw_flowctrl_igrPortPauseThresh_get
 * Description:
 *      Get used page high/low threshold for the specified ingress port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pThresh - pointer to the threshold structure for the port used page count
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
dal_ssw_flowctrl_igrPortPauseThresh_get(uint32 unit, rtk_port_t port, rtk_flowctrl_thresh_t *pThresh)
{
    int32   ret;
    rtk_ssw_reg_list_t  reg_idx;
    uint32  value;
    

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pThresh), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    reg_idx = flow_control_threshold_for_control0_regidx[port];
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_read(unit, reg_idx, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_get(unit, reg_idx, th_port_high_on_fieldidx[port], &(pThresh->highOn), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_get(unit, reg_idx, th_port_high_off_fieldidx[port], &(pThresh->highOff), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    reg_idx = flow_control_threshold_for_control1_regidx[port];
    
    if ((ret = reg_field_read(unit, reg_idx, th_port_low_fieldidx[port], &(pThresh->lowOn))) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    pThresh->lowOff = pThresh->lowOn;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "highOn=0x%x, highOff=0x%x\
           lowOn=0x%x, lowOff=0x%x", pThresh->highOn, pThresh->highOff, pThresh->lowOn, pThresh->lowOff);
    
    FLOWCTRL_SEM_UNLOCK(unit);
    return RT_ERR_OK;
} /* end of dal_ssw_flowctrl_igrPortPauseThresh_get */

/* Function Name:
 *      dal_ssw_flowctrl_igrPortPauseThresh_set
 * Description:
 *      Set used page high/low threshold for the specified ingress port
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      pThresh - pointer to the threshold structure for the port used page count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      1. For RTL8389/RTL8329 lowOn and lowOff threshold should be set to the same value,
 *         otherwise RT_ERR_INPUT will be returned
 */
int32
dal_ssw_flowctrl_igrPortPauseThresh_set(uint32 unit, rtk_port_t port, rtk_flowctrl_thresh_t *pThresh)
{
    int32   ret;
    rtk_ssw_reg_list_t  reg_idx;
    uint32  value;
    
    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, port=%d", unit, port);  
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pThresh), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(pThresh->highOn > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pThresh->highOff > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pThresh->lowOn > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);

    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "highOn=0x%x, highOff=0x%x, lowOn=0x%x, lowOff=0x%x",
           pThresh->highOn, pThresh->highOff, pThresh->lowOn, pThresh->lowOff);  
    
    reg_idx = flow_control_threshold_for_control0_regidx[port];
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_read(unit, reg_idx, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_set(unit, reg_idx, th_port_high_on_fieldidx[port], &(pThresh->highOn), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_set(unit, reg_idx, th_port_high_off_fieldidx[port], &(pThresh->highOff), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
     /* program value to CHIP*/
    if ((ret = reg_write(unit, reg_idx, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    reg_idx = flow_control_threshold_for_control1_regidx[port];
    
    if ((ret = reg_field_write(unit, reg_idx, th_port_low_fieldidx[port], &(pThresh->lowOn))) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    FLOWCTRL_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_flowctrl_igrPortPauseThresh_set */


/*
 * Flow Control OFF
 */

/* Function Name:
 *      dal_ssw_flowctrl_igrSystemCongestThresh_get
 * Description:
 *      Get system used page high/low drop threshold paramters of the specific unit
 * Input:
 *      unit    - unit id
 * Output:
 *      pThresh - pointer to the threshold structure in the public used page count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_ssw_flowctrl_igrSystemCongestThresh_get(uint32 unit, rtk_flowctrl_thresh_t *pThresh)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pThresh), RT_ERR_NULL_POINTER);
    
    osal_memset(pThresh, 0, sizeof(rtk_flowctrl_thresh_t));
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_read(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL3r, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_get(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL3r, SSW_TH_GLOBAL_FCOFF_HIGH_ONf, &(pThresh->highOn), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_get(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL3r, SSW_TH_GLOBAL_FCOFF_HIGH_OFFf, &(pThresh->highOff), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    /* get value from CHIP*/
    if ((ret = reg_read(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL4r, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_get(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL4r, SSW_TH_GLOBAL_FCOFF_LOW_ONf, &(pThresh->lowOn), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_get(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL4r, SSW_TH_GLOBAL_FCOFF_LOW_OFFf, &(pThresh->lowOff), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    FLOWCTRL_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "highOn=0x%x, highOff=0x%x\
           lowOn=0x%x, lowOff=0x%x", pThresh->highOn, pThresh->highOff, pThresh->lowOn, pThresh->lowOff);
    
    return RT_ERR_OK;
} /* end of dal_ssw_flowctrl_igrSystemCongestThresh_get */

/* Function Name:
 *      dal_ssw_flowctrl_igrSystemCongestThresh_set
 * Description:
 *      Set system used page high/low threshold paramters of the specific unit
 * Input:
 *      unit    - unit id
 *      pThresh - pointer to the threshold structure in the public used page count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
int32
dal_ssw_flowctrl_igrSystemCongestThresh_set(uint32 unit, rtk_flowctrl_thresh_t *pThresh)
{
    int32   ret;
    uint32  value;
       
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pThresh), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(pThresh->highOn > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pThresh->highOff > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pThresh->lowOn > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pThresh->lowOff > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);

    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d \
           highOn=0x%x, highOff=0x%x, lowOn=0x%x, lowOff=0x%x", unit, pThresh->highOn, pThresh->highOff, 
           pThresh->lowOn, pThresh->lowOff);
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_read(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL3r, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_set(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL3r, SSW_TH_GLOBAL_FCOFF_HIGH_ONf, &(pThresh->highOn), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_set(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL3r, SSW_TH_GLOBAL_FCOFF_HIGH_OFFf, &(pThresh->highOff), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
     /* program value to CHIP*/
    if ((ret = reg_write(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL3r, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    /* get value from CHIP*/
    if ((ret = reg_read(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL4r, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_set(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL4r, SSW_TH_GLOBAL_FCOFF_LOW_ONf, &(pThresh->lowOn), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_set(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL4r, SSW_TH_GLOBAL_FCOFF_LOW_OFFf, &(pThresh->lowOff), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
     /* program value to CHIP*/
    if ((ret = reg_write(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL4r, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    FLOWCTRL_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_flowctrl_igrSystemCongestThresh_set */

/* Function Name:
 *      dal_ssw_flowctrl_igrPortCongestThresh_get
 * Description:
 *      Get used page high drop threshold for the specified ingress port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pThresh - pointer to the threshold structure for the port used page count
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
dal_ssw_flowctrl_igrPortCongestThresh_get(uint32 unit, rtk_port_t port, rtk_flowctrl_thresh_t *pThresh)
{
    int32 ret;
    rtk_ssw_reg_list_t  reg_idx;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pThresh), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    reg_idx = flow_control_threshold_for_control2_regidx[port];
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, reg_idx, th_port_high_fcoff_on_fieldidx[port], &(pThresh->highOn))) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_read(unit, reg_idx, th_port_high_fcoff_off_fieldidx[port], &(pThresh->highOff))) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    reg_idx = flow_control_threshold_for_control1_regidx[port];
    
    if ((ret = reg_field_read(unit, reg_idx, th_port_fcoff_low_fieldidx[port], &(pThresh->lowOn))) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    FLOWCTRL_SEM_UNLOCK(unit);
    
    pThresh->lowOff = pThresh->lowOn;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "highOn=0x%x, highOff=0x%x\
           lowOn=0x%x, lowOff=0x%x", pThresh->highOn, pThresh->highOff, pThresh->lowOn, pThresh->lowOff);
    
    return RT_ERR_OK;
} /* end of dal_ssw_flowctrl_igrPortCongestThresh_get */

/* Function Name:
 *      dal_ssw_flowctrl_igrPortCongestThresh_set
 * Description:
 *      Set used page high drop threshold for the specified ingress port
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      pThresh - pointer to the threshold structure for the port used page count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      1. For RTL8389/RTL8329 lowOn and lowOff threshold should be set to the same value,
 *         otherwise RT_ERR_INPUT will be returned
 */
int32
dal_ssw_flowctrl_igrPortCongestThresh_set(uint32 unit, rtk_port_t port, rtk_flowctrl_thresh_t *pThresh)
{
    int32 ret;
    rtk_ssw_reg_list_t  reg_idx;
    
    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, port=%d", unit, port);   
        
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pThresh), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(pThresh->highOn > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pThresh->highOff > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pThresh->lowOn > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "highOn=0x%x, highOff=0x%x, lowOn=0x%x, lowOff=0x%x", 
           pThresh->highOn, pThresh->highOff, pThresh->lowOn, pThresh->lowOff);    
    
    reg_idx = flow_control_threshold_for_control2_regidx[port];
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* program value to chip*/
    if ((ret = reg_field_write(unit, reg_idx, th_port_high_fcoff_on_fieldidx[port], &(pThresh->highOn))) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_write(unit, reg_idx, th_port_high_fcoff_off_fieldidx[port], &(pThresh->highOff))) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    reg_idx = flow_control_threshold_for_control1_regidx[port];
    
    if ((ret = reg_field_write(unit, reg_idx, th_port_fcoff_low_fieldidx[port], &(pThresh->lowOn))) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    FLOWCTRL_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_flowctrl_igrPortCongestThresh_set */

/* Function Name:
 *      dal_ssw_flowctrl_egrSystemDropThresh_get
 * Description:
 *      Get egress system drop threshold for the specified unit
 * Input:
 *      unit    - unit id
 * Output:
 *      pThresh - pointer to the drop threshold
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID            - invalid unit id
 *      RT_ERR_NULL_POINTER       - input parameter may be null pointer
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 * Note:
 *      None
 */
int32
dal_ssw_flowctrl_egrSystemDropThresh_get(uint32 unit, rtk_flowctrl_drop_thresh_t *pThresh)
{
    int32   ret;
    
    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pThresh), RT_ERR_NULL_POINTER);
    
    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "high=0x%x, low=0x%x", pThresh->high, pThresh->low);
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* program value to CHIP*/
    if ((ret = reg_field_read(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL0r, SSW_DROP_ALLf, &(pThresh->high))) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }

    pThresh->low = pThresh->high;
    FLOWCTRL_SEM_UNLOCK(unit);    
    
    return RT_ERR_OK;
} /* end of dal_ssw_flowctrl_egrSystemDropThresh_get */

/* Function Name:
 *      dal_ssw_flowctrl_egrSystemDropThresh_set
 * Description:
 *      Set egress drop threshold for the specified egress port
 * Input:
 *      unit    - unit id
 *      pThresh - pointer to the drop threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID            - invalid unit id
 *      RT_ERR_NULL_POINTER       - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE       - input parameter out of range
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 * Note:
 *      None
 */
int32
dal_ssw_flowctrl_egrSystemDropThresh_set(uint32 unit, rtk_flowctrl_drop_thresh_t *pThresh)
{
    int32 ret;
    
    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pThresh), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(pThresh->high > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pThresh->low > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    
    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "high=0x%x, low=0x%x", pThresh->high, pThresh->low);
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* program value to CHIP*/
    if ((ret = reg_field_write(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL0r, SSW_DROP_ALLf, &(pThresh->high))) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }

    FLOWCTRL_SEM_UNLOCK(unit);    
    
    return RT_ERR_OK;
} /* end of dal_ssw_flowctrl_egrSystemDropThresh_set */

/* Function Name:
 *      dal_ssw_flowctrl_egrPortDropThresh_get
 * Description:
 *      Get egress drop threshold for the specified egress port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pThresh - pointer to the drop threshold
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
dal_ssw_flowctrl_egrPortDropThresh_get(uint32 unit, rtk_port_t port, rtk_flowctrl_drop_thresh_t *pThresh)
{
    int32 ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pThresh), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, per_port_egress_drop_threshold_control_regidx[port], th_ppe_drop_on_fieldidx[port], &(pThresh->high))) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }

    FLOWCTRL_SEM_UNLOCK(unit);
    
    pThresh->low = 0;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "high=0x%x, low=0x%x", pThresh->high, pThresh->low);
    
    return RT_ERR_OK;
} /* end of dal_ssw_flowctrl_egrPortDropThresh_get */

/* Function Name:
 *      dal_ssw_flowctrl_egrPortDropThresh_set
 * Description:
 *      Set egress drop threshold for the specified egress port
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      pThresh - pointer to the drop threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      1. For RTL8389/RTL8329 high and low drop threshold should be set to the same value,
 *         otherwise RT_ERR_INPUT will be returned.
 */
int32
dal_ssw_flowctrl_egrPortDropThresh_set(uint32 unit, rtk_port_t port, rtk_flowctrl_drop_thresh_t *pThresh)
{
    int32 ret;
    
    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, port=%d", unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pThresh), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(pThresh->high > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "high=0x%x, low=0x%x", pThresh->high, pThresh->low);    
    
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* program value to CHIP*/
    if ((ret = reg_field_write(unit, per_port_egress_drop_threshold_control_regidx[port], th_ppe_drop_on_fieldidx[port], &(pThresh->high))) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }

    FLOWCTRL_SEM_UNLOCK(unit);    
    
    return RT_ERR_OK;
} /* end of dal_ssw_flowctrl_egrPortDropThresh_set */

/* Function Name:
 *      dal_ssw_flowctrl_egrQueueDropThresh_get
 * Description:
 *      Get egress global drop threshold for the egress queue
 * Input:
 *      unit    - unit id
 *      queue   - queue id
 * Output:
 *      pThresh - pointer to the global drop threshold
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_ssw_flowctrl_egrQueueDropThresh_get(uint32 unit, rtk_qid_t queue, rtk_flowctrl_drop_thresh_t *pThresh)
{
    int32 ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, queue=%d",
           unit, queue);        
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pThresh), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(queue >= HAL_MAX_NUM_OF_QUEUE(unit), RT_ERR_QUEUE_ID);
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, per_queue_egress_drop_threshold_control_regidx[queue], th_pqe_drop_on_fieldidx[queue], &(pThresh->high))) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }

    FLOWCTRL_SEM_UNLOCK(unit);
    
    pThresh->low = pThresh->high;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "high=0x%x, low=0x%x", 
           pThresh->high, pThresh->low);     
    
    return RT_ERR_OK;
} /* end of dal_ssw_flowctrl_egrQueueDropThresh_get */

/* Function Name:
 *      dal_ssw_flowctrl_egrQueueDropThresh_set
 * Description:
 *      Set egress gloabl drop threshold for the egress queue
 * Input:
 *      unit    - unit id
 *      queue   - queue id
 *      pThresh - pointer to the drop threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      1. For RTL8389/RTL8329 high and low threshold should be set to the same value,
 *         otherwise RT_ERR_INPUT will be returned.
 */
int32
dal_ssw_flowctrl_egrQueueDropThresh_set(uint32 unit, rtk_qid_t queue, rtk_flowctrl_drop_thresh_t *pThresh)
{
    int32 ret;
    
    /* Display debug message */    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, queue=%d", unit, queue); 
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pThresh), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(pThresh->high > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(queue >= HAL_MAX_NUM_OF_QUEUE(unit), RT_ERR_QUEUE_ID);

    /* Display debug message */    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "high=0x%x, low=0x%x", pThresh->high, pThresh->low); 
    
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* program value to CHIP*/
    if ((ret = reg_field_write(unit, per_queue_egress_drop_threshold_control_regidx[queue], th_pqe_drop_on_fieldidx[queue], &(pThresh->high))) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }

    FLOWCTRL_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_flowctrl_egrQueueDropThresh_set */
