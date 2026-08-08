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
 *             1) Configuration of ingress port bandwidth control (ingress rate limit ).
 *             2) Configuration of egress port bandwidth control (egress rate limit).
 *             3) Configuration of storm control
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
#include <dal/ssw/dal_ssw_qos.h>
#include <dal/ssw/dal_ssw_port.h>
#include <rtk/default.h>
#include <rtk/qos.h>

/* 
 * Symbol Definition 
 */
#define QOS_CHK_QUEUE_EMPTY_TIMES           (1200000)
#define QUEUE_EMPTY_VALUE                   (BITMASK_28B)
#define CPU_QUEUE_NUM                       (8)

/* 
 * Data Declaration 
 */
static uint32               qos_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         qos_sem[RTK_MAX_NUM_OF_UNIT];


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

const static uint16 dscp_priority_assignment_control_regidx[] = 
{  SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL0r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL0r\
 , SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL0r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL0r\
 , SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL0r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL0r\
 , SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL0r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL0r\
 , SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL0r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL0r\
 , SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL1r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL1r\
 , SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL1r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL1r\
 , SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL1r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL1r\
 , SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL1r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL1r\
 , SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL1r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL1r\
 , SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL2r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL2r\
 , SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL2r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL2r\
 , SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL2r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL2r\
 , SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL2r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL2r\
 , SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL2r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL2r\
 , SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL3r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL3r\
 , SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL3r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL3r\
 , SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL3r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL3r\
 , SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL3r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL3r\
 , SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL3r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL3r\
 , SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL4r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL4r\
 , SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL4r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL4r\
 , SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL4r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL4r\
 , SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL4r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL4r\
 , SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL4r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL4r\
 , SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL5r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL5r\
 , SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL5r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL5r\
 , SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL5r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL5r\
 , SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL5r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL5r\
 , SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL5r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL5r\
 , SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL6r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL6r\
 , SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL6r, SSW_DSCP_PRIORITY_ASSIGNMENT_CONTROL6r};

const static uint16 internal_priority_to_queue_id_control_regidx[] = 
{  SSW_INTERNAL_PRIORITY_TO_QUEUE_ID_CONTROL0r, SSW_INTERNAL_PRIORITY_TO_QUEUE_ID_CONTROL1r\
 , SSW_INTERNAL_PRIORITY_TO_QUEUE_ID_CONTROL2r, SSW_INTERNAL_PRIORITY_TO_QUEUE_ID_CONTROL3r\
 , SSW_INTERNAL_PRIORITY_TO_QUEUE_ID_CONTROL4r, SSW_INTERNAL_PRIORITY_TO_QUEUE_ID_CONTROL5r\
 , SSW_INTERNAL_PRIORITY_TO_QUEUE_ID_CONTROL6r, SSW_INTERNAL_PRIORITY_TO_QUEUE_ID_CONTROL7r};

const static uint16 internal_priority_to_dscp_control_regidx[] = 
{  SSW_INTERNAL_PRIORITY_TO_DSCP_CONTROL1r, SSW_INTERNAL_PRIORITY_TO_DSCP_CONTROL1r\
 , SSW_INTERNAL_PRIORITY_TO_DSCP_CONTROL1r, SSW_INTERNAL_PRIORITY_TO_DSCP_CONTROL1r\
 , SSW_INTERNAL_PRIORITY_TO_DSCP_CONTROL1r, SSW_INTERNAL_PRIORITY_TO_DSCP_CONTROL2r\
 , SSW_INTERNAL_PRIORITY_TO_DSCP_CONTROL2r, SSW_INTERNAL_PRIORITY_TO_DSCP_CONTROL2r};
 
const static uint16 wfq_wrr_port_parameter0_regidx[] = 
{SSW_WFQ_WRR_PORT_0_PARAMETER0r, SSW_WFQ_WRR_PORT_1_PARAMETER0r, SSW_WFQ_WRR_PORT_2_PARAMETER0r, SSW_WFQ_WRR_PORT_3_PARAMETER0r, SSW_WFQ_WRR_PORT_4_PARAMETER0r, SSW_WFQ_WRR_PORT_5_PARAMETER0r, SSW_WFQ_WRR_PORT_6_PARAMETER0r, SSW_WFQ_WRR_PORT_7_PARAMETER0r, SSW_WFQ_WRR_PORT_8_PARAMETER0r, SSW_WFQ_WRR_PORT_9_PARAMETER0r\
,SSW_WFQ_WRR_PORT_10_PARAMETER0r, SSW_WFQ_WRR_PORT_11_PARAMETER0r, SSW_WFQ_WRR_PORT_12_PARAMETER0r, SSW_WFQ_WRR_PORT_13_PARAMETER0r, SSW_WFQ_WRR_PORT_14_PARAMETER0r, SSW_WFQ_WRR_PORT_15_PARAMETER0r, SSW_WFQ_WRR_PORT_16_PARAMETER0r, SSW_WFQ_WRR_PORT_17_PARAMETER0r, SSW_WFQ_WRR_PORT_18_PARAMETER0r, SSW_WFQ_WRR_PORT_19_PARAMETER0r\
,SSW_WFQ_WRR_PORT_20_PARAMETER0r, SSW_WFQ_WRR_PORT_21_PARAMETER0r, SSW_WFQ_WRR_PORT_22_PARAMETER0r, SSW_WFQ_WRR_PORT_23_PARAMETER0r, SSW_WFQ_WRR_PORT_24_PARAMETER0r, SSW_WFQ_WRR_PORT_25_PARAMETER0r, SSW_WFQ_WRR_PORT_26_PARAMETER0r, SSW_WFQ_WRR_PORT_27_PARAMETER0r, SSW_WFQ_WRR_PORT_28_PARAMETER0r};

const static uint16 queue_number_control_regidx[] = 
{  SSW_QUEUE_NUMBER_CONTROL0r, SSW_QUEUE_NUMBER_CONTROL0r\
 , SSW_QUEUE_NUMBER_CONTROL0r, SSW_QUEUE_NUMBER_CONTROL0r\
 , SSW_QUEUE_NUMBER_CONTROL0r, SSW_QUEUE_NUMBER_CONTROL0r\
 , SSW_QUEUE_NUMBER_CONTROL0r, SSW_QUEUE_NUMBER_CONTROL0r\
 , SSW_QUEUE_NUMBER_CONTROL0r, SSW_QUEUE_NUMBER_CONTROL0r\
 , SSW_QUEUE_NUMBER_CONTROL1r, SSW_QUEUE_NUMBER_CONTROL1r\
 , SSW_QUEUE_NUMBER_CONTROL1r, SSW_QUEUE_NUMBER_CONTROL1r\
 , SSW_QUEUE_NUMBER_CONTROL1r, SSW_QUEUE_NUMBER_CONTROL1r\
 , SSW_QUEUE_NUMBER_CONTROL1r, SSW_QUEUE_NUMBER_CONTROL1r\
 , SSW_QUEUE_NUMBER_CONTROL1r, SSW_QUEUE_NUMBER_CONTROL1r\
 , SSW_QUEUE_NUMBER_CONTROL2r, SSW_QUEUE_NUMBER_CONTROL2r\
 , SSW_QUEUE_NUMBER_CONTROL2r, SSW_QUEUE_NUMBER_CONTROL2r\
 , SSW_QUEUE_NUMBER_CONTROL2r, SSW_QUEUE_NUMBER_CONTROL2r\
 , SSW_QUEUE_NUMBER_CONTROL2r, SSW_QUEUE_NUMBER_CONTROL2r\
 , SSW_QUEUE_NUMBER_CONTROL2r};

 
const static uint16 pPri_fieldidx[] = {SSW_P0PPRIf, SSW_P1PPRIf, SSW_P2PPRIf, SSW_P3PPRIf, SSW_P4PPRIf, SSW_P5PPRIf, SSW_P6PPRIf, SSW_P7PPRIf, SSW_P8PPRIf, SSW_P9PPRIf\
                         ,SSW_P10PPRIf, SSW_P11PPRIf, SSW_P12PPRIf, SSW_P13PPRIf, SSW_P14PPRIf, SSW_P15PPRIf, SSW_P16PPRIf, SSW_P17PPRIf, SSW_P18PPRIf, SSW_P19PPRIf\
                         ,SSW_P20PPRIf, SSW_P21PPRIf, SSW_P22PPRIf, SSW_P23PPRIf, SSW_P24PPRIf, SSW_P25PPRIf, SSW_P26PPRIf, SSW_P27PPRIf, SSW_P28PPRIf};
const static uint16 dscpremap_fieldidx[] = {SSW_PDSCP0f, SSW_PDSCP1f, SSW_PDSCP2f, SSW_PDSCP3f, SSW_PDSCP4f, SSW_PDSCP5f, SSW_PDSCP6f, SSW_PDSCP7f, SSW_PDSCP8f, SSW_PDSCP9f,
                                 SSW_PDSCP10f, SSW_PDSCP11f, SSW_PDSCP12f, SSW_PDSCP13f, SSW_PDSCP14f, SSW_PDSCP15f, SSW_PDSCP16f, SSW_PDSCP17f, SSW_PDSCP18f, SSW_PDSCP19f,
                                 SSW_PDSCP20f, SSW_PDSCP21f, SSW_PDSCP22f, SSW_PDSCP23f, SSW_PDSCP24f, SSW_PDSCP25f, SSW_PDSCP26f, SSW_PDSCP27f, SSW_PDSCP28f, SSW_PDSCP29f,
                                 SSW_PDSCP30f, SSW_PDSCP31f, SSW_PDSCP32f, SSW_PDSCP33f, SSW_PDSCP34f, SSW_PDSCP35f, SSW_PDSCP36f, SSW_PDSCP37f, SSW_PDSCP38f, SSW_PDSCP39f,
                                 SSW_PDSCP40f, SSW_PDSCP41f, SSW_PDSCP42f, SSW_PDSCP43f, SSW_PDSCP44f, SSW_PDSCP45f, SSW_PDSCP46f, SSW_PDSCP47f, SSW_PDSCP48f, SSW_PDSCP49f,
                                 SSW_PDSCP50f, SSW_PDSCP51f, SSW_PDSCP52f, SSW_PDSCP53f, SSW_PDSCP54f, SSW_PDSCP55f, SSW_PDSCP56f, SSW_PDSCP57f, SSW_PDSCP58f, SSW_PDSCP59f,
                                 SSW_PDSCP60f, SSW_PDSCP61f, SSW_PDSCP62f, SSW_PDSCP63f};
const static uint16 dot1pPri_fieldidx[] = {SSW_INT_PRI0f, SSW_INT_PRI1f, SSW_INT_PRI2f, SSW_INT_PRI3f, SSW_INT_PRI4f, SSW_INT_PRI5f, SSW_ITN_PRI6f, SSW_ITN_PRI7f};

const static uint16 dot1pRemark_fieldidx[] = {SSW_USER_PRI0f, SSW_USER_PRI1f, SSW_USER_PRI2f, SSW_USER_PRI3f, SSW_USER_PRI4f, SSW_USER_PRI5f, SSW_USER_PRI6f, SSW_USER_PRI7f};

const static uint16 dscpRemark_fieldidx[] = {SSW_DSCP_PRI0f, SSW_DSCP_PRI1f, SSW_DSCP_PRI2f, SSW_DSCP_PRI3f, SSW_DSCP_PRI4f, SSW_DSCP_PRI5f, SSW_DSCP_PRI6f, SSW_DSCP_PRI7f};

const static uint16 portWRRWFQSelect_fieldidx[] = {SSW_P0_WFQWRR_SELf, SSW_P1_WFQWRR_SELf, SSW_P2_WFQWRR_SELf, SSW_P3_WFQWRR_SELf, SSW_P4_WFQWRR_SELf, SSW_P5_WFQWRR_SELf, SSW_P6_WFQWRR_SELf, SSW_P7_WFQWRR_SELf, SSW_P8_WFQWRR_SELf, SSW_P9_WFQWRR_SELf\
                         ,SSW_P10_WFQWRR_SELf, SSW_P11_WFQWRR_SELf, SSW_P12_WFQWRR_SELf, SSW_P13_WFQWRR_SELf, SSW_P14_WFQWRR_SELf, SSW_P15_WFQWRR_SELf, SSW_P16_WFQWRR_SELf, SSW_P17_WFQWRR_SELf, SSW_P18_WFQWRR_SELf, SSW_P19_WFQWRR_SELf\
                         ,SSW_P20_WFQWRR_SELf, SSW_P21_WFQWRR_SELf, SSW_P22_WFQWRR_SELf, SSW_P23_WFQWRR_SELf, SSW_P24_WFQWRR_SELf, SSW_P25_WFQWRR_SELf, SSW_P26_WFQWRR_SELf, SSW_P27_WFQWRR_SELf, SSW_P28_WFQWRR_SELf};

const static uint16 queuePriQid_fieldidx[] = {SSW__1QPRI0QIDf, SSW__1QPRI1QIDf, SSW__1QPRI2QIDf, SSW__1QPRI3QIDf, SSW__1QPRI4QIDf, SSW__1QPRI5QIDf, SSW__1QPRI6QIDf, SSW__1QPRI7QIDf};

const static uint16 portQueueNum_fieldidx[] = {SSW_P0QNUMf, SSW_P1QNUMf, SSW_P2QNUMf, SSW_P3QNUMf, SSW_P4QNUMf, SSW_P5QNUMf, SSW_P6QNUMf, SSW_P7QNUMf, SSW_P8QNUMf, SSW_P9QNUMf\
                         ,SSW_P10QNUMf, SSW_P11QNUMf, SSW_P12QNUMf, SSW_P13QNUMf, SSW_P14QNUMf, SSW_P15QNUMf, SSW_P16QNUMf, SSW_P17QNUMf, SSW_P18QNUMf, SSW_P19QNUMf\
                         ,SSW_P20QNUMf, SSW_P21QNUMf, SSW_P22QNUMf, SSW_P23QNUMf, SSW_P24QNUMf, SSW_P25QNUMf, SSW_P26QNUMf, SSW_P27QNUMf, SSW_P28QNUMf};

const static uint16 portQueueWeight_fieldidx[] = {SSW_P0Q0_WEIGHTf, SSW_P0Q1_WEIGHTf, SSW_P0Q2_WEIGHTf, SSW_P0Q3_WEIGHTf, SSW_P0Q4_WEIGHTf, SSW_P0Q5_WEIGHTf, SSW_P0Q6_WEIGHTf, SSW_P0Q7_WEIGHTf};
const static uint16 portStrictEn_fieldidx[] = {SSW_P0Q0_SP_ENf, SSW_P0Q1_SP_ENf, SSW_P0Q2_SP_ENf, SSW_P0Q3_SP_ENf, SSW_P0Q4_SP_ENf, SSW_P0Q5_SP_ENf, SSW_P0Q6_SP_ENf, SSW_P0Q7_SP_ENf};
const static uint16 aprLb_fieldidx[] = {SSW_P0Q0_LB_APRf, SSW_P0Q1_LB_APRf, SSW_P0Q2_LB_APRf, SSW_P0Q3_LB_APRf, SSW_P0Q4_LB_APRf, SSW_P0Q5_LB_APRf, SSW_P0Q6_LB_APRf, SSW_P0Q7_LB_APRf};

const static uint16 qos_scheduling_queueWeight[8][8] = {{0,0,0,0,0,0,0,0}, 
                                                        {0,0,0,0,0,0,0,1}, 
                                                        {0,1,0,0,0,0,0,2}, 
                                                        {0,1,2,0,0,0,0,3}, 
                                                        {0,1,2,3,0,0,0,4}, 
                                                        {0,1,2,3,4,0,0,5},
                                                        {0,1,2,3,4,5,0,6},
                                                        {0,1,2,3,4,5,6,7}};

const static rtk_qos_pri2queue_t qos_scheduling_priorityToQueue[8]= {{{0,0,0,0,0,0,0,0}}, 
                                                                     {{0,0,0,0,1,1,1,1}}, 
                                                                     {{0,0,0,0,1,1,2,2}}, 
                                                                     {{0,0,1,1,2,2,3,3}},
                                                                     {{0,0,1,1,2,3,4,4}}, 
                                                                     {{0,0,1,2,3,4,5,5}},
                                                                     {{0,0,1,2,3,4,5,6}},
                                                                     {{0,1,2,3,4,5,6,7}}};
/*
 * Macro Definition
 */
/* rate semaphore handling */
#define QOS_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(qos_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_DAL|MOD_QOS), "semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)
#define QOS_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(qos_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_DAL|MOD_QOS), "semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)


/* 
 * Function Declaration 
 */
static int32 _dal_ssw_qos_init_config(uint32 unit, uint32 queueNum);
static void _dal_ssw_qos_translateAsicLayerQidToApiLayerQid(uint32 qnum, rtk_qid_t qid, rtk_qid_t *pApiQid);
static void _dal_ssw_qos_translateApiLayerQidToAsicLayerQid(uint32 qnum, rtk_qid_t qid, rtk_qid_t *pAsicQid);
static int32 _dal_ssw_qos_queueWeight_set(
    uint32 unit, 
    rtk_port_t port, 
    rtk_qid_t qid, 
    rtk_qos_queue_type_t queueType, 
    uint32 weight);
static int32 _dal_ssw_qos_queueWeight_get(
    uint32 unit, 
    rtk_port_t port, 
    rtk_qid_t qid, 
    rtk_qos_queue_type_t *pQueueType, 
    uint32 *pWeight);
static int32 _dal_ssw_qos_outputQueueNumber_set(uint32 unit, rtk_port_t port, uint32 qNum);
static int32 _dal_ssw_qos_outputQueueNumber_get(uint32 unit, rtk_port_t port, uint32 *pQNum);
static int32 _dal_ssw_qos_cpuQueueNum_set(uint32 unit, uint32 queue_num);
static int32 _dal_ssw_qos_cleanQosQueue(uint32 unit);
static int32 _dal_ssw_qos_restartRx(uint32 unit);

/* Function Name:
 *      dal_ssw_qos_init
 * Description:
 *      Configure QoS initial settings with queue number assigment to each port
 * Input:
 *      unit     - unit id
 *      queueNum - Queue number of each port, it is available at 1~8
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID   - Invalid unit id
 *      RT_ERR_QUEUE_NUM - Invalid queue number
 * Note:
 *      This API will initialize related QoS setting with queue number assignment.
 *      The initialization does the following actions:
 *      1. set input bandwidth control parameters to default values
 *      2. set priority decision parameters
 *      3. set scheduling parameters
 *      4. disable port remark ability
 *      5. set flow control thresholds
 */
int32
dal_ssw_qos_init(uint32 unit, uint32 queueNum)
{
    int32       ret;
    
    qos_init[unit] = INIT_NOT_COMPLETED;
    
    RT_PARAM_CHK(queueNum > HAL_MAX_NUM_OF_QUEUE(unit), RT_ERR_QUEUE_NUM);
    
    /* create semaphore */
    qos_sem[unit] = osal_sem_mutex_create();
    if (0 == qos_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_QOS), "semaphore create failed");
        return RT_ERR_FAILED;
    }    
    
    /* set init flag to complete init */
    qos_init[unit] = INIT_COMPLETED;    

    if ((ret = _dal_ssw_qos_init_config(unit, queueNum)) != RT_ERR_OK)
    {
        qos_init[unit] = INIT_NOT_COMPLETED;
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    return RT_ERR_OK;
} /* end of dal_ssw_qos_init */

/* Function Name:
 *      dal_ssw_qos_priSel_get
 * Description:
 *      Get the priority among different priority mechanism.
 * Input:
 *      unit       - unit id
 * Output:
 *      pPort_pri  - Priority assign for port based selection (0~3). 3 is the highest
 *                   priority
 *      pClass_pri - Priority assign for classifier selection (0~3). 3 is the highest
 *                   priority
 *      pAcl_pri   - Priority assign for ingress acl selection (0~3). 3 is the highest
 *                   priority
 *      pDscp_pri  - Priority assign for dscp selection (0~3). 3 is the highest priority
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *      None.
 */
int32
dal_ssw_qos_priSel_get(
    uint32      unit,
    rtk_pri_t   *pPort_pri,
    rtk_pri_t   *pClass_pri,
    rtk_pri_t   *pAcl_pri,
    rtk_pri_t   *pDscp_pri)
{
    int32   ret;
    uint32  value;
   
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(qos_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pPort_pri), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pClass_pri), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pAcl_pri), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pDscp_pri), RT_ERR_NULL_POINTER);
    
    QOS_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_read(unit, SSW_PRIORITY_SELECTION_TABLE_CONTROLr, &value)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    if ((ret = reg_field_get(unit, SSW_PRIORITY_SELECTION_TABLE_CONTROLr, SSW_DSCP_PRIf, pDscp_pri, &value)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    if ((ret = reg_field_get(unit, SSW_PRIORITY_SELECTION_TABLE_CONTROLr, SSW_IN_ACL_PRIf, pAcl_pri, &value)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    if ((ret = reg_field_get(unit, SSW_PRIORITY_SELECTION_TABLE_CONTROLr, SSW_CLASS_PRIf, pClass_pri, &value)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    if ((ret = reg_field_get(unit, SSW_PRIORITY_SELECTION_TABLE_CONTROLr, SSW_PORT_PRIf, pPort_pri, &value)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    QOS_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "pPort_pri=%d, pClass_pri=%d, pAcl_pri=%d, pDscp_pri=%d", 
           *pPort_pri, *pClass_pri, *pAcl_pri, *pDscp_pri);
    
    return RT_ERR_OK;
} /* end of dal_ssw_qos_priSel_get */

/* Function Name:
 *      dal_ssw_qos_priSel_set
 * Description:
 *      Set the priority among different priority mechanism.
 * Input:
 *      unit      - unit id
 *      port_pri  - Priority assign for port based selection (0~3). 3 is the highest
 *                  priority
 *      class_pri - Priority assign for classifier selection (0~3). 3 is the highest
 *                  priority
 *      acl_pri   - Priority assign for ingress acl selection (0~3). 3 is the highest
 *                  priority
 *      dscp_pri  - Priority assign for dscp selection (0~3). 3 is the highest priority
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID            - Invalid unit id
 *      RT_ERR_QOS_SEL_PORT_PRI   - Invalid port selection priority
 *      RT_ERR_QOS_SEL_CLASS_PRI  - Invalid classifier selection priority
 *      RT_ERR_QOS_SEL_IN_ACL_PRI - Invalid ingress ACL selection priority
 *      RT_ERR_QOS_SEL_DSCP_PRI   - Invalid dscp selection priority
 * Note:
 *      1. ASIC will follow user priority setting of mechanisms to select mapped queue priority for
 *         receiving frame.
 *
 *      2. If more than one priorities of mechanisms are the same, ASIC will chose the highest
 *         priority from mechanisms to assign queue priority to receiving frame.
 *
 *      3. This API can set priority for four mechanisms :
 *         Port based priority, Classifier priority, Ingress ACL priority, and DSCP priority.
 */
int32
dal_ssw_qos_priSel_set(
    uint32      unit,
    rtk_pri_t   port_pri,
    rtk_pri_t   class_pri,
    rtk_pri_t   acl_pri,
    rtk_pri_t   dscp_pri)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "unit=%d, port_pri=%d, class_pri=%d, \
           acl_pri=%d, dscp_pri=%d", unit, port_pri, class_pri, acl_pri, dscp_pri);
    
    /* check Init status */
    RT_INIT_CHK(qos_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((port_pri < HAL_PRI_OF_SELECTION_MIN(unit)) 
               ||(port_pri > HAL_PRI_OF_SELECTION_MAX(unit)), RT_ERR_QOS_SEL_PORT_PRI);
    RT_PARAM_CHK((class_pri < HAL_PRI_OF_SELECTION_MIN(unit)) 
               ||(class_pri > HAL_PRI_OF_SELECTION_MAX(unit)), RT_ERR_QOS_SEL_CLASS_PRI);
    RT_PARAM_CHK((acl_pri < HAL_PRI_OF_SELECTION_MIN(unit)) 
               ||(acl_pri > HAL_PRI_OF_SELECTION_MAX(unit)), RT_ERR_QOS_SEL_IN_ACL_PRI);
    RT_PARAM_CHK((dscp_pri < HAL_PRI_OF_SELECTION_MIN(unit)) 
               ||(dscp_pri > HAL_PRI_OF_SELECTION_MAX(unit)), RT_ERR_QOS_SEL_DSCP_PRI);
    
    QOS_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_read(unit, SSW_PRIORITY_SELECTION_TABLE_CONTROLr, &value)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    if ((ret = reg_field_set(unit, SSW_PRIORITY_SELECTION_TABLE_CONTROLr, SSW_DSCP_PRIf, &dscp_pri, &value)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    if ((ret = reg_field_set(unit, SSW_PRIORITY_SELECTION_TABLE_CONTROLr, SSW_IN_ACL_PRIf, &acl_pri, &value)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    if ((ret = reg_field_set(unit, SSW_PRIORITY_SELECTION_TABLE_CONTROLr, SSW_CLASS_PRIf, &class_pri, &value)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    if ((ret = reg_field_set(unit, SSW_PRIORITY_SELECTION_TABLE_CONTROLr, SSW_PORT_PRIf, &port_pri, &value)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    if ((ret = reg_write(unit, SSW_PRIORITY_SELECTION_TABLE_CONTROLr, &value)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    QOS_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_qos_priSel_set */

/* Function Name:
 *      dal_ssw_qos_portPri_get
 * Description:
 *      Get internal priority of one port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pInt_pri - Priorities assigment for specific port. (range from 0 ~ 7, 7 is
 *                 the highest prioirty)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    None.
 */
int32
dal_ssw_qos_portPri_get(uint32 unit, rtk_port_t port, rtk_pri_t *pInt_pri)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(qos_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pInt_pri), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    QOS_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, tag_based_and_port_based_control_regidx[port], pPri_fieldidx[port], pInt_pri)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    QOS_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "pInt_pri=%d", *pInt_pri);
    
    return RT_ERR_OK;
} /* end of dal_ssw_qos_portPri_get */

/* Function Name:
 *      dal_ssw_qos_portPri_set
 * Description:
 *      Set internal priority of one port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      int_pri - Priorities assigment for specific port. (range from 0 ~ 7, 7 is
 *                the highest prioirty)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID          - Invalid unit id
 *      RT_ERR_PORT_ID          - Invalid port id
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority value
 * Note:
 *    This API can set port to 3 bits internal priority mapping.
 *    When a packet is received from a port, a port based priority will be assigned
 *    by the mapping setting.
 *    By default, the mapping priorities for all ports are 0.
 */
int32
dal_ssw_qos_portPri_set(uint32 unit, rtk_port_t port, rtk_pri_t int_pri)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "unit=%d, port=%d, int_pri=%d", 
           unit, port, int_pri);
    
    /* check Init status */
    RT_INIT_CHK(qos_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((int_pri > HAL_INTERNAL_PRIORITY_MAX(unit)), RT_ERR_QOS_INT_PRIORITY);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    QOS_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_write(unit, tag_based_and_port_based_control_regidx[port], pPri_fieldidx[port], &int_pri)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    QOS_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_qos_portPri_set */

/* Function Name:
 *      dal_ssw_qos_dscpPriRemap_get
 * Description:
 *      Get the internal priority that DSCP value remap.
 * Input:
 *      unit     - unit id
 *      dscp     - DSCP value of receiving frame (0~63)
 * Output:
 *      pInt_pri - Priorities assigment for specific port. (range from 0 ~ 7, 7 is
 *                 the highest prioirty)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - Invalid unit id
 *      RT_ERR_QOS_DSCP_VALUE - Invalid DSCP value
 *      RT_ERR_NULL_POINTER   - NULL pointer
 * Note:
 *      None.
 */
int32
dal_ssw_qos_dscpPriRemap_get(uint32 unit, uint32 dscp, rtk_pri_t *pInt_pri)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "unit=%d, dscp=%d", unit, dscp);
    
    /* check Init status */
    RT_INIT_CHK(qos_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pInt_pri), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(dscp > RTK_VALUE_OF_DSCP_MAX, RT_ERR_QOS_DSCP_VALUE);
    
    QOS_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, dscp_priority_assignment_control_regidx[dscp], dscpremap_fieldidx[dscp], pInt_pri)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    QOS_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "pInt_pri=%d", *pInt_pri);
    
    return RT_ERR_OK;
} /* end of dal_ssw_qos_dscpPriRemap_get */

/* Function Name:
 *      dal_ssw_qos_dscpPriRemap_set
 * Description:
 *      Set the internal priority that DSCP value remap.
 * Input:
 *      unit    - unit id
 *      dscp    - DSCP value of receiving frame (0~63)
 *      int_pri - Priorities assigment for specific port. (range from 0 ~ 7, 7 is
 *                the highest prioirty)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID          - Invalid unit id
 *      RT_ERR_QOS_DSCP_VALUE   - Invalid DSCP value
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority value
 * Note:
 *      The Differentiated Service Code Point is a selector for router's per-hop behaviours.
 *      As a selector, there is no implication that a numerically greater DSCP implies a better
 *      network service. As can be seen, the DSCP totally overlaps the old precedence field of TOS.
 *      So if values of DSCP are carefully chosen then backward compatibility can be achieved.
 */
int32
dal_ssw_qos_dscpPriRemap_set(uint32 unit, uint32 dscp, rtk_pri_t int_pri)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "unit=%d, dscp=%d, int_pri=%d", 
           unit, dscp, int_pri);
    
    /* check Init status */
    RT_INIT_CHK(qos_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(dscp > RTK_VALUE_OF_DSCP_MAX, RT_ERR_QOS_DSCP_VALUE);
    RT_PARAM_CHK(int_pri > HAL_INTERNAL_PRIORITY_MAX(unit), RT_ERR_QOS_INT_PRIORITY);
    
    
    QOS_SEM_LOCK(unit);
    
    /* program value to chip CHIP*/
    if ((ret = reg_field_write(unit, dscp_priority_assignment_control_regidx[dscp], dscpremap_fieldidx[dscp], &int_pri)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    QOS_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_qos_dscpPriRemap_set */

/* Function Name:
 *      dal_ssw_qos_1pPriRemap_get
 * Description:
 *      Get the internal priority that 802.1p priority remap.
 * Input:
 *      unit      - unit id
 *      dot1p_pri - 802.1p priority value (range from 0 ~ 7)
 * Output:
 *      pInt_pri  - internal priority value (range from 0 ~ 7)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - Invalid unit id
 *      RT_ERR_QOS_1P_PRIORITY - Invalid 802.1p priority
 *      RT_ERR_NULL_POINTER    - NULL pointer
 * Note:
 *      None.
 */
int32
dal_ssw_qos_1pPriRemap_get(uint32 unit, rtk_pri_t dot1p_pri, rtk_pri_t *pInt_pri)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "unit=%d, dot1p_pri=%d", 
           unit, dot1p_pri);
    
    /* check Init status */
    RT_INIT_CHK(qos_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pInt_pri), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(dot1p_pri > RTK_DOT1P_PRIORITY_MAX, RT_ERR_QOS_1P_PRIORITY);
    
    
    QOS_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, SSW_USER_PRIORITY_TO_INTERNAL_PRIORITY_CONTROLr, dot1pPri_fieldidx[dot1p_pri], pInt_pri)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    QOS_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "pInt_pri=%d", *pInt_pri);
    
    return RT_ERR_OK;
} /* end of dal_ssw_qos_1pPriRemap_get */

/* Function Name:
 *      dal_ssw_qos_1pPriRemap_set
 * Description:
 *      Set the internal priority that 802.1p priority remap.
 * Input:
 *      unit      - unit id
 *      dot1p_pri - 802.1p priority value (range from 0 ~ 7)
 *      int_pri   - internal priority value (range from 0 ~ 7)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID          - Invalid unit id
 *      RT_ERR_QOS_1P_PRIORITY  - Invalid 802.1p priority
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority value
 * Note:
 *      None.
 */
int32
dal_ssw_qos_1pPriRemap_set(uint32 unit, rtk_pri_t dot1p_pri, rtk_pri_t int_pri)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "unit=%d, dot1p_pri=%d, int_pri=%d", 
           unit, dot1p_pri, int_pri);
    
    /* check Init status */
    RT_INIT_CHK(qos_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(dot1p_pri > RTK_DOT1P_PRIORITY_MAX, RT_ERR_QOS_1P_PRIORITY);
    RT_PARAM_CHK(int_pri > HAL_INTERNAL_PRIORITY_MAX(unit), RT_ERR_QOS_INT_PRIORITY);
    
    
    QOS_SEM_LOCK(unit);
    
    /* program value to CHIP*/
    if ((ret = reg_field_write(unit, SSW_USER_PRIORITY_TO_INTERNAL_PRIORITY_CONTROLr, dot1pPri_fieldidx[dot1p_pri], &int_pri)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    QOS_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_qos_1pPriRemap_set */

/* Function Name:
 *      dal_ssw_qos_queueNum_get
 * Description:
 *      Get the number of queue for the system.
 * Input:
 *      unit       - unit id
 * Output:
 *      pQueue_num - the number of queue (1~8).
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *      None.
 */
int32
dal_ssw_qos_queueNum_get(uint32 unit, uint32 *pQueue_num)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(qos_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pQueue_num), RT_ERR_NULL_POINTER);
    
    QOS_SEM_LOCK(unit);
    
    if ((ret = _dal_ssw_qos_outputQueueNumber_get(unit, 0, pQueue_num)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        return ret;
    }
    QOS_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "pQueue_num=%d", *pQueue_num);
    
    return RT_ERR_OK;
} /* end of dal_ssw_qos_queueNum_get */

/* Function Name:
 *      dal_ssw_qos_queueNum_set
 * Description:
 *      Set the number of queue for the system.
 * Input:
 *      unit      - unit id
 *      queue_num - the number of queue (1~8).
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID   - Invalid unit id
 *      RT_ERR_QUEUE_NUM - Invalid queue number
 * Note:
 *      None.
 */
int32
dal_ssw_qos_queueNum_set(uint32 unit, uint32 queue_num)
{
    int32       ret;
    rtk_qid_t   qid;
    rtk_port_t  port;   
    rtk_port_t  max_port;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "unit=%d, queue_num=%d", unit, queue_num);

    /* check Init status */
    RT_INIT_CHK(qos_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(((queue_num > HAL_MAX_NUM_OF_QUEUE(unit)) || (queue_num < HAL_MIN_NUM_OF_QUEUE(unit))), RT_ERR_QUEUE_NUM);
    
    max_port = HAL_GET_MAX_ETHER_PORT(unit);
    
    QOS_SEM_LOCK(unit);
    
    /* E0005358: clean Queue */
    if ((ret = _dal_ssw_qos_cleanQosQueue(unit)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        return ret;
    }
    
    for (port = HAL_GET_MIN_ETHER_PORT(unit); port <= max_port; port++)
    {
        if (!HAL_IS_ETHER_PORT(unit, port))
        {
            continue;
        }
        
        if ((ret = _dal_ssw_qos_outputQueueNumber_set(unit, port, queue_num)) != RT_ERR_OK)
        {
            /* restart Rx */
            _dal_ssw_qos_restartRx(unit);
            
            QOS_SEM_UNLOCK(unit);            
            return ret;
        }
        
        for (qid = 0; qid <= (HAL_MAX_NUM_OF_QUEUE(unit) - 1); qid++)
        {
            if ((ret = _dal_ssw_qos_queueWeight_set(unit, port, qid
                                , RTK_DEFAULT_QOS_SCHED_QUEUE_TYPE
                                , qos_scheduling_queueWeight[queue_num-1][qid])) != RT_ERR_OK)
            {
                /* restart Rx */
                _dal_ssw_qos_restartRx(unit);
                
                QOS_SEM_UNLOCK(unit);                
                return ret;
            }
        }
    }
    
    /* E0005358: restart Rx */
    if ((ret = _dal_ssw_qos_restartRx(unit)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        return ret;
    }
    
    QOS_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_qos_queueNum_set */

/* Function Name:
 *      dal_ssw_qos_priMap_get
 * Description:
 *      Get the value of internal priority to QID mapping table.
 * Input:
 *      unit      - unit id
 *      queue_num - the number of queue (1~8).
 * Output:
 *      pPri2qid  - array of internal priority on a queue
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_QUEUE_NUM    - Invalid queue number
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *      None.
 */
int32
dal_ssw_qos_priMap_get(uint32 unit, uint32 queue_num, rtk_qos_pri2queue_t *pPri2qid)
{
    int32       ret;
    rtk_pri_t   pri;
    rtk_qid_t   apiQid;
    rtk_ssw_regField_list_t  field_id;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "unit=%d, queue_num=%d", unit, queue_num);

    /* check Init status */
    RT_INIT_CHK(qos_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(queue_num > HAL_MAX_NUM_OF_QUEUE(unit), RT_ERR_QUEUE_NUM);
    RT_PARAM_CHK((NULL == pPri2qid), RT_ERR_NULL_POINTER);
    
    QOS_SEM_LOCK(unit);
    
    for (pri = 0; pri <= HAL_INTERNAL_PRIORITY_MAX(unit); pri++)
    {
        
        field_id = queuePriQid_fieldidx[pri] + (queue_num - 1) * 8;
        
        if ((ret = reg_field_read(unit, internal_priority_to_queue_id_control_regidx[queue_num - 1], field_id, &apiQid)) != RT_ERR_OK)
        {
            QOS_SEM_UNLOCK(unit);
            return ret;
        }
        _dal_ssw_qos_translateAsicLayerQidToApiLayerQid(queue_num, apiQid, &(pPri2qid->pri2queue[pri]));
    }
    
    QOS_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "pPri2qid[0]=%d, pPri2qid[1]=%d, pPri2qid[2]=%d\
           pPri2qid[3]=%d, pPri2qid[4]=%d, pPri2qid[5]=%d, pPri2qid[6]=%d, pPri2qid[7]=%d", 
           pPri2qid->pri2queue[0],
           pPri2qid->pri2queue[1],
           pPri2qid->pri2queue[2],
           pPri2qid->pri2queue[3],
           pPri2qid->pri2queue[4],
           pPri2qid->pri2queue[5],
           pPri2qid->pri2queue[6],
           pPri2qid->pri2queue[7]);                           
    
    return RT_ERR_OK;
} /* end of dal_ssw_qos_priMap_get */

/* Function Name:
 *      dal_ssw_qos_priMap_set
 * Description:
 *      Set the value of internal priority to QID mapping table.
 * Input:
 *      unit      - unit id
 *      queue_num - the number of queue (1~8).
 *      pPri2qid  - array of internal priority on a queue
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID   - Invalid unit id
 *      RT_ERR_QUEUE_NUM - Invalid queue number
 * Note:
 *      Below is an example of internal priority to QID mapping table.
 *      When queue numbers are 8, the pri2qid are pri2qid[0]=0, pri2qid[1]=1, pri2qid[2]=2..., etc.
 *
 *                  Number of Available Output Queue
 *        Priority  1   2   3   4   5   6   7   8
 *              0   0   0   0   0   0   0   0   0
 *              1   0   0   0   0   0   0   0   1
 *              2   0   0   0   1   1   1   1   2
 *              3   0   0   0   1   1   2   2   3
 *              4   0   1   1   2   2   3   3   4
 *              5   0   1   1   2   3   4   4   5
 *              6   0   1   2   3   4   5   5   6
 *              7   0   1   2   3   4   5   6   7
 */
int32
dal_ssw_qos_priMap_set(uint32 unit, uint32 queue_num, rtk_qos_pri2queue_t *pPri2qid)
{
    int32       ret;
    rtk_pri_t   pri;
    rtk_qid_t   asicQid;
    rtk_ssw_regField_list_t  field_id;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "unit=%d, queue_num=%d", unit, queue_num);

    /* check Init status */
    RT_INIT_CHK(qos_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(queue_num > HAL_MAX_NUM_OF_QUEUE(unit), RT_ERR_QUEUE_NUM);
    RT_PARAM_CHK((NULL == pPri2qid), RT_ERR_NULL_POINTER);

    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "Pri2qid[0]=%d, pPri2qid[1]=%d, pPri2qid[2]=%d\
           pPri2qid[3]=%d, pPri2qid[4]=%d, pPri2qid[5]=%d, pPri2qid[6]=%d, pPri2qid[7]=%d",
           pPri2qid->pri2queue[0],
           pPri2qid->pri2queue[1],
           pPri2qid->pri2queue[2],
           pPri2qid->pri2queue[3],
           pPri2qid->pri2queue[4],
           pPri2qid->pri2queue[5],
           pPri2qid->pri2queue[6],
           pPri2qid->pri2queue[7]);  
    
    QOS_SEM_LOCK(unit);
    
    /* E0005358: clean queue before do any queue change */
    if (( ret = _dal_ssw_qos_cleanQosQueue(unit)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        return ret;
    }
    
    for (pri = 0; pri <= HAL_INTERNAL_PRIORITY_MAX(unit); pri++)
    {
        _dal_ssw_qos_translateApiLayerQidToAsicLayerQid(queue_num, pPri2qid->pri2queue[pri], &asicQid);
        field_id = queuePriQid_fieldidx[pri] + (queue_num - 1) * 8;
        
        if ((ret = reg_field_write(unit, internal_priority_to_queue_id_control_regidx[queue_num - 1], field_id, &asicQid)) != RT_ERR_OK)
        {
            _dal_ssw_qos_restartRx(unit);
            QOS_SEM_UNLOCK(unit);
            return ret;
        }
    }
    
    /* E0005358: restart Rx */
    if (( ret = _dal_ssw_qos_restartRx(unit)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        return ret;
    }
    
    QOS_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_qos_priMap_set */

/* Function Name:
 *      dal_ssw_qos_1pRemarkEnable_get
 * Description:
 *      Get 802.1p remark status for a port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - status of 802.1p remark
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    The status of 802.1p remark:
 *    - DISABLED
 *    - ENABLED
 */
int32
dal_ssw_qos_1pRemarkEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    rtk_portmask_t  portmask;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(qos_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    osal_memset(&portmask, 0, sizeof(portmask));
    
    QOS_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, SSW_INTERNAL_PRIORITY_TO_USER_PRIORITY_CONTROL0r, SSW__1PRM_ENf, &portmask.bits[0])) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    QOS_SEM_UNLOCK(unit);
    
    if (RTK_PORTMASK_IS_PORT_SET(portmask, port))
    {
        *pEnable = ENABLED;
    } 
    else 
    {
        *pEnable = DISABLED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "pEnable=%d", *pEnable);
    
    return RT_ERR_OK;
} /* end of dal_ssw_qos_1pRemarkEnable_get */

/* Function Name:
 *      dal_ssw_qos_1pRemarkEnable_set
 * Description:
 *      Set 802.1p remark status for a port
 * Input:
 *      unit   - unit id
 *      port   - port id.
 *      enable - status of 802.1p remark
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - Invalid unit id
 *      RT_ERR_PORT_ID - Invalid port id
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *    The status of 802.1p remark:
 *    - DISABLED
 *    - ENABLED
 */
int32
dal_ssw_qos_1pRemarkEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    rtk_portmask_t  portmask;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "unit=%d, port=%d, enable=%d", 
           unit, port, enable);
    
    /* check Init status */
    RT_INIT_CHK(qos_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(enable >= RTK_ENABLE_END, RT_ERR_INPUT);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    osal_memset(&portmask, 0, sizeof(portmask));
    
    QOS_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, SSW_INTERNAL_PRIORITY_TO_USER_PRIORITY_CONTROL0r, SSW__1PRM_ENf, &portmask.bits[0])) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    /* set or clear bit of this port in DSCP_EN portmask */
    if (ENABLED == enable)
    {
        RTK_PORTMASK_PORT_SET(portmask, port);
    } 
    else 
    {
        RTK_PORTMASK_PORT_CLEAR(portmask, port);
    }
    
    /* program new DSCP_EN portmask to CHIP */
    if ((ret = reg_field_write(unit, SSW_INTERNAL_PRIORITY_TO_USER_PRIORITY_CONTROL0r, SSW__1PRM_ENf, &portmask.bits[0])) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    QOS_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_qos_1pRemarkEnable_set */

/* Function Name:
 *      dal_ssw_qos_1pRemark_get
 * Description:
 *      Get the internal priority (3bits) to remarkd 802.1p priority(3bits) mapping.
 * Input:
 *      unit       - unit id
 *      int_pri    - internal priority value (range from 0 ~ 7)
 * Output:
 *      pDot1p_pri - remarked 802.1p priority value (range from 0 ~ 7)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID          - Invalid unit id
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority
 *      RT_ERR_NULL_POINTER     - NULL pointer
 * Note:
 *      None.
 */
int32
dal_ssw_qos_1pRemark_get(uint32 unit, rtk_pri_t int_pri, rtk_pri_t *pDot1p_pri)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "unit=%d, int_pri=%d", unit, int_pri);
    
    /* check Init status */
    RT_INIT_CHK(qos_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pDot1p_pri), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(int_pri > HAL_INTERNAL_PRIORITY_MAX(unit), RT_ERR_QOS_INT_PRIORITY);
    
    QOS_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, SSW_INTERNAL_PRIORITY_TO_USER_PRIORITY_CONTROL1r, dot1pRemark_fieldidx[int_pri], pDot1p_pri)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    QOS_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "pDot1p_pri=%d", *pDot1p_pri);
    
    return RT_ERR_OK;
} /* end of dal_ssw_qos_1pRemark_get */

/* Function Name:
 *      dal_ssw_qos_1pRemark_set
 * Description:
 *      Set the internal priority(3bits) to remarked 802.1p priority(3bits) mapping.
 * Input:
 *      unit      - unit id
 *      int_pri   - internal priority value (range from 0 ~ 7)
 *      dot1p_pri - remarked 802.1p priority value (range from 0 ~ 7)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID          - Invalid unit id
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority
 * Note:
 *      802.1p remark functionality can map the internal priority to 802.1p priority before a packet
 *      is going to be transmited.
 */
int32
dal_ssw_qos_1pRemark_set(uint32 unit, rtk_pri_t int_pri, rtk_pri_t dot1p_pri)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "unit=%d, int_pri=%d, dot1p_pri=%d", 
           unit, int_pri, dot1p_pri);
    
    /* check Init status */
    RT_INIT_CHK(qos_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(dot1p_pri > RTK_DOT1P_PRIORITY_MAX, RT_ERR_QOS_1P_PRIORITY);
    RT_PARAM_CHK(int_pri > HAL_INTERNAL_PRIORITY_MAX(unit), RT_ERR_QOS_INT_PRIORITY);
    
    QOS_SEM_LOCK(unit);
    
    /* program value to chip*/
    if ((ret = reg_field_write(unit, SSW_INTERNAL_PRIORITY_TO_USER_PRIORITY_CONTROL1r, dot1pRemark_fieldidx[int_pri], &dot1p_pri)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    QOS_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_qos_1pRemark_set */

/* Function Name:
 *      dal_ssw_qos_dscpRemarkEnable_get
 * Description:
 *      Get DSCP remark status for a port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - status of DSCP remark
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    The status of DSCP remark:
 *    - DISABLED
 *    - ENABLED
 */
int32
dal_ssw_qos_dscpRemarkEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    rtk_portmask_t  portmask;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(qos_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    osal_memset(&portmask, 0, sizeof(portmask));
    
    QOS_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, SSW_INTERNAL_PRIORITY_TO_DSCP_CONTROL0r, SSW_DSCPRM_ENf, &portmask.bits[0])) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    QOS_SEM_UNLOCK(unit);
    
    if (RTK_PORTMASK_IS_PORT_SET(portmask, port))
    {
        *pEnable = ENABLED;
    } 
    else 
    {
        *pEnable = DISABLED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "pEnable=%d", *pEnable);
    
    return RT_ERR_OK;
} /* end of dal_ssw_qos_dscpRemarkEnable_get */

/* Function Name:
 *      dal_ssw_qos_dscpRemarkEnable_set
 * Description:
 *      Set DSCP remark status for a port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of DSCP remark
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - Invalid unit id
 *      RT_ERR_PORT_ID - Invalid port id
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *    The status of DSCP remark:
 *    - DISABLED
 *    - ENABLED
 */
int32
dal_ssw_qos_dscpRemarkEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    rtk_portmask_t  portmask;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "unit=%d, port=%d, enable=%d", unit, port, enable);
    
    /* check Init status */
    RT_INIT_CHK(qos_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(enable >= RTK_ENABLE_END, RT_ERR_INPUT);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    osal_memset(&portmask, 0, sizeof(portmask));
    
    QOS_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, SSW_INTERNAL_PRIORITY_TO_DSCP_CONTROL0r, SSW_DSCPRM_ENf, &portmask.bits[0])) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    /* set or clear bit of this port in DSCP_EN portmask */
    if (ENABLED == enable)
    {
        RTK_PORTMASK_PORT_SET(portmask, port);
    } 
    else 
    {
        RTK_PORTMASK_PORT_CLEAR(portmask, port);
    }
    
    /* program new DSCP_EN portmask to CHIP */
    if ((ret = reg_field_write(unit, SSW_INTERNAL_PRIORITY_TO_DSCP_CONTROL0r, SSW_DSCPRM_ENf, &portmask.bits[0])) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    QOS_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_qos_dscpRemarkEnable_set */

/* Function Name:
 *      dal_ssw_qos_dscpRemark_get
 * Description:
 *      Get the internal priority (3bits) to remarked DSCP mapping.
 * Input:
 *      unit    - unit id
 *      int_pri - internal priority value (range from 0 ~ 7)
 * Output:
 *      pDscp   - remarked DSCP value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID          - Invalid unit id
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority
 *      RT_ERR_NULL_POINTER     - NULL pointer
 * Note:
 *      None.
 */
int32
dal_ssw_qos_dscpRemark_get(uint32 unit, rtk_pri_t int_pri, uint32 *pDscp)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "unit=%d, int_pri=%d", unit, int_pri);
    
    /* check Init status */
    RT_INIT_CHK(qos_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pDscp), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(int_pri > HAL_INTERNAL_PRIORITY_MAX(unit), RT_ERR_QOS_INT_PRIORITY);
    
    QOS_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, internal_priority_to_dscp_control_regidx[int_pri], dscpRemark_fieldidx[int_pri], pDscp)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    QOS_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "pDscp=%d", *pDscp);
    
    return RT_ERR_OK;
} /* end of dal_ssw_qos_dscpRemark_get */

/* Function Name:
 *      dal_ssw_qos_dscpRemark_set
 * Description:
 *      Set the internal priority (3bits) to remarked DSCP mapping.
 * Input:
 *      unit    - unit id
 *      int_pri - internal priority value (range from 0 ~ 7)
 *      dscp    - remarked DSCP value
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID          - Invalid unit id
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority
 *      RT_ERR_QOS_DSCP_VALUE   - Invalid dscp value
 * Note:
 *      DSCP remark functionality can map the internal priority to DSCP before a packet is going
 *      to be transmited.
 */
int32
dal_ssw_qos_dscpRemark_set(uint32 unit, rtk_pri_t int_pri, uint32 dscp)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "unit=%d, int_pri=%d, dscp=%d", 
           unit, int_pri, dscp);
    
    /* check Init status */
    RT_INIT_CHK(qos_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(dscp > RTK_VALUE_OF_DSCP_MAX, RT_ERR_QOS_DSCP_VALUE);
    RT_PARAM_CHK(int_pri > HAL_INTERNAL_PRIORITY_MAX(unit), RT_ERR_QOS_INT_PRIORITY);
    
    QOS_SEM_LOCK(unit);
    
    /* program value to CHIP*/
    if ((ret = reg_field_write(unit, internal_priority_to_dscp_control_regidx[int_pri], dscpRemark_fieldidx[int_pri], &dscp)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    QOS_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_qos_dscpRemark_set */

/* Function Name:
 *      dal_ssw_qos_schedulingAlgorithm_get
 * Description:
 *      Get the scheduling algorithm of the port.
 * Input:
 *      unit             - unit id
 *      port             - port id
 * Output:
 *      pScheduling_type - type of scheduling algorithm.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    The types of scheduling algorithm:
 *    - WFQ
 *    - WRR
 */
int32
dal_ssw_qos_schedulingAlgorithm_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_qos_scheduling_type_t   *pScheduling_type)
{
    int32   ret;
    uint32  value;

    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "unit=%d, port=%d", 
           unit, port);
    
    /* check Init status */
    RT_INIT_CHK(qos_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pScheduling_type), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    QOS_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, wfq_wrr_port_parameter0_regidx[port], portWRRWFQSelect_fieldidx[port], &value)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    QOS_SEM_UNLOCK(unit);

    switch (value)
    {
        case 0:
            *pScheduling_type = WFQ;
            break;
        case 1:
            *pScheduling_type = WRR;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "pScheduling_type=%d", 
           *pScheduling_type);    
    
    return RT_ERR_OK;
} /* end of dal_ssw_qos_schedulingAlgorithm_get */

/* Function Name:
 *      dal_ssw_qos_schedulingAlgorithm_set
 * Description:
 *      Set the scheduling algorithm of the port.
 * Input:
 *      unit            - unit id
 *      port            - port id
 *      scheduling_type - type of scheduling algorithm.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_QOS_SCHE_TYPE - Error scheduling algorithm type
 * Note:
 *    The types of scheduling algorithm:
 *    - WFQ
 *    - WRR
 */
int32
dal_ssw_qos_schedulingAlgorithm_set(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_qos_scheduling_type_t   scheduling_type)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "unit=%d, port=%d, scheduling_type=%d", 
           unit, port, scheduling_type);
    
    /* check Init status */
    RT_INIT_CHK(qos_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(scheduling_type >= SCHEDULING_TYPE_END, RT_ERR_QOS_SCHE_TYPE);
    
    switch (scheduling_type)
    {
        case WFQ:
            value = 0;
            break;
        case WRR:
            value = 1;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    QOS_SEM_LOCK(unit);
    
    /* program value to CHIP*/
    if ((ret = reg_field_write(unit, wfq_wrr_port_parameter0_regidx[port], portWRRWFQSelect_fieldidx[port], &value)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    QOS_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_ssw_qos_schedulingAlgorithm_set */

/* Function Name:
 *      dal_ssw_qos_schedulingQueue_get
 * Description:
 *      Get the scheduling types and weights of queues on specific port in egress scheduling.
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pQweights - the array of weights for WRR/WFQ queue (valid:1~128, 0 for STRICT_PRIORITY queue)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    The types of queue are: WFQ_WRR_PRIORITY or STRICT_PRIORITY.
 *    If the weight is 0 then the type is STRICT_PRIORITY, else the type is WFQ_WRR_PRIORITY.
 */
int32
dal_ssw_qos_schedulingQueue_get(uint32 unit, rtk_port_t port, rtk_qos_queue_weights_t *pQweights)
{
    int32       ret;
    uint32      weight;
    uint32      queueNum;
    rtk_qid_t   qid;
    rtk_qid_t   asicQid;
    rtk_qos_queue_type_t    queueType;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "unit=%d, port=%d", 
           unit, port);    
        
    /* check Init status */
    RT_INIT_CHK(qos_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pQweights), RT_ERR_NULL_POINTER);
    
    QOS_SEM_LOCK(unit);
    
    if ((ret = _dal_ssw_qos_outputQueueNumber_get(unit, port, &queueNum)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    for (qid = 0; qid <= (HAL_MAX_NUM_OF_QUEUE(unit) - 1); qid++)
    {
        _dal_ssw_qos_translateApiLayerQidToAsicLayerQid(queueNum, qid, &asicQid);
        
        if ((ret = _dal_ssw_qos_queueWeight_get(unit, port, asicQid
                , &queueType, &weight)) != RT_ERR_OK)
        {
            QOS_SEM_UNLOCK(unit);
            return ret;
        }
        
        if (STRICT_PRIORITY == queueType )
        {
            pQweights->weights[qid] = 0;
        } 
        else 
        {
            pQweights->weights[qid] = weight + 1;
        }
    }
    QOS_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "pQweights[0]=%d, pQweights[1]=%d, pQweights[2]=%d\
           pQweights[3]=%d, pQweights[4]=%d, pQweights[5]=%d, pQweights[6]=%d, pQweights[7]=%d", 
           pQweights->weights[0],
           pQweights->weights[1],
           pQweights->weights[2],
           pQweights->weights[3],
           pQweights->weights[4],
           pQweights->weights[5],
           pQweights->weights[6],
           pQweights->weights[7]); 
    
    return RT_ERR_OK;
} /* end of dal_ssw_qos_schedulingQueue_get */

/* Function Name:
 *      dal_ssw_qos_schedulingQueue_set
 * Description:
 *      Set the scheduling types and weights of queues on specific port in egress scheduling.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      pQweights - the array of weights for WRR/WFQ queue (valid:1~128, 0 for STRICT_PRIORITY queue)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID          - Invalid unit id
 *      RT_ERR_PORT_ID          - Invalid port id
 *      RT_ERR_NULL_POINTER     - Null pointer
 *      RT_ERR_QOS_QUEUE_WEIGHT - Invalid queue weight
 * Note:
 *    The types of queue are: WFQ_WRR_PRIORITY or STRICT_PRIORITY.
 *    If the weight is 0 then the type is STRICT_PRIORITY, else the type is WFQ_WRR_PRIORITY.
 */
int32
dal_ssw_qos_schedulingQueue_set(uint32 unit, rtk_port_t port, rtk_qos_queue_weights_t *pQweights)
{
    int32       ret;     
    uint32      weight;
    uint32      queueNum;
    rtk_qid_t   qid;
    rtk_qid_t   asicQid;
    rtk_qos_queue_type_t    queueType;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "unit=%d, port=%d", 
           unit, port);
        
    /* check Init status */
    RT_INIT_CHK(qos_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pQweights), RT_ERR_NULL_POINTER);

    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "pQweights[0]=%d, \
           pQweights[1]=%d, pQweights[2]=%d, pQweights[3]=%d, pQweights[4]=%d, pQweights[5]=%d, pQweights[6]=%d, \
           pQweights[7]=%d",
           pQweights->weights[0],
           pQweights->weights[1],
           pQweights->weights[2],
           pQweights->weights[3],
           pQweights->weights[4],
           pQweights->weights[5],
           pQweights->weights[6],
           pQweights->weights[7]);       
    
    QOS_SEM_LOCK(unit);
    
    if ((ret = _dal_ssw_qos_outputQueueNumber_get(unit, port, &queueNum)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    for (qid = 0; qid <= (HAL_MAX_NUM_OF_QUEUE(unit) - 1); qid++)
    {
        _dal_ssw_qos_translateApiLayerQidToAsicLayerQid(queueNum, qid, &asicQid);
        
        if (0 == pQweights->weights[qid])
        {
            queueType = STRICT_PRIORITY;
            weight = 0;
        } 
        else 
        {
            queueType = WFQ_WRR_PRIORITY;
            weight = pQweights->weights[qid];
        }
        
        if ((ret = _dal_ssw_qos_queueWeight_set(unit, port, asicQid
                , queueType, weight)) != RT_ERR_OK)
        {
            QOS_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_QOS), "");
            return ret;
        }
    }
    
    QOS_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_qos_schedulingQueue_set */

/* Function Name:
 *      _dal_ssw_qos_init_config
 * Description:
 *      Configure QoS initial settings with queue number assigment to each port
 * Input:
 *      unit     - unit id
 *      queueNum - Queue number of each port, it is available at 1~8
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID   - Invalid unit id
 *      RT_ERR_QUEUE_NUM - Invalid queue number
 * Note:
 *      This API will initialize related QoS setting with queue number assignment.
 *      The initialization does the following actions:
 *      1. set input bandwidth control parameters to default values
 *      2. set priority decision parameters
 *      3. set scheduling parameters
 *      4. disable port remark ability
 *      5. set flow control thresholds
 */
static int32
_dal_ssw_qos_init_config(uint32 unit, uint32 queueNum)
{
    uint32      val;
    int32       ret;
    rtk_port_t  port, max_port;
    rtk_pri_t   dot1p_pri;
    rtk_pri_t   int_pri;
    rtk_qid_t   qid;
    rtk_qos_pri2queue_t pri2Queue;
    rtk_ssw_reg_list_t      reg_id;
    rtk_ssw_regField_list_t field_id;
    rtk_pri_t   dot1pPriRemap[RTK_DOT1P_PRIORITY_MAX + 1] = {RTK_DEFAULT_QOS_1P_PRIORITY0_REMAP, RTK_DEFAULT_QOS_1P_PRIORITY1_REMAP,
                                                   RTK_DEFAULT_QOS_1P_PRIORITY2_REMAP, RTK_DEFAULT_QOS_1P_PRIORITY3_REMAP,
                                                   RTK_DEFAULT_QOS_1P_PRIORITY4_REMAP, RTK_DEFAULT_QOS_1P_PRIORITY5_REMAP,
                                                   RTK_DEFAULT_QOS_1P_PRIORITY6_REMAP, RTK_DEFAULT_QOS_1P_PRIORITY7_REMAP};

    rtk_qos_queue_weights_t queue_weights = {{RTK_DEFAULT_QOS_SCHED_QUEUE0_WEIGHT, RTK_DEFAULT_QOS_SCHED_QUEUE1_WEIGHT,
                                             RTK_DEFAULT_QOS_SCHED_QUEUE2_WEIGHT, RTK_DEFAULT_QOS_SCHED_QUEUE3_WEIGHT,
                                             RTK_DEFAULT_QOS_SCHED_QUEUE4_WEIGHT, RTK_DEFAULT_QOS_SCHED_QUEUE5_WEIGHT,
                                             RTK_DEFAULT_QOS_SCHED_QUEUE6_WEIGHT, RTK_DEFAULT_QOS_SCHED_QUEUE7_WEIGHT}};
    
    if ((ret = dal_ssw_qos_queueNum_set(unit, queueNum)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    if ((ret = dal_ssw_qos_priSel_set(unit, RTK_DEFAULT_QOS_SELECTION_PRIORITY_OF_PORT, RTK_DEFAULT_QOS_SELECTION_PRIORITY_OF_CLASS
                          , RTK_DEFAULT_QOS_SELECTION_PRIORITY_OF_ACL, RTK_DEFAULT_QOS_SELECTION_PRIORITY_OF_DSCP)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }

    max_port = HAL_GET_MAX_PORT(unit);
    for (port = 0; port < max_port; port++)
    { 
        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }
        if ((ret = dal_ssw_qos_portPri_set(unit, port, RTK_DEFAULT_QOS_PORT_PRIORITY)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
            return ret;
        }
        
        if ((ret = dal_ssw_qos_schedulingAlgorithm_set(unit, port, RTK_DEFAULT_QOS_SCHED_ALGORITHM)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
            return ret;
        }
        
        if ((ret = dal_ssw_qos_schedulingQueue_set(unit, port, &queue_weights)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
            return ret;
        }
    }
 
    for (dot1p_pri = 0; dot1p_pri <= RTK_DOT1P_PRIORITY_MAX; dot1p_pri++)
    {
        if ((ret = dal_ssw_qos_1pPriRemap_set(unit, dot1p_pri, dot1pPriRemap[dot1p_pri])) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
            return ret;
        }
    }
    
    for (int_pri = 0; int_pri <= HAL_INTERNAL_PRIORITY_MAX(unit); int_pri++)
    {
        osal_memcpy(&pri2Queue, &(qos_scheduling_priorityToQueue[int_pri]), sizeof(pri2Queue));
        if ((ret = dal_ssw_qos_priMap_set(unit, queueNum, &pri2Queue)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
            return ret;
        }
    }
    
    QOS_SEM_LOCK(unit);
    
    if ((ret = _dal_ssw_qos_cpuQueueNum_set(unit, RTK_DEFAULT_QOS_QUEUE_NUMBER_IN_CPU_PORT)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    val = RTK_DEFAULT_QOS_SCHED_LB_BYTE_PER_TOKEN;
    if ((ret = reg_field_write(unit, SSW_PACKET_SCHEDULING_GLOBAL_CONTROLr, SSW_BYTE_PER_TKNf, &val)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }

    val = RTK_DEFAULT_QOS_SCHED_LB_TICK_PERIOD;
    if ((ret = reg_field_write(unit, SSW_PACKET_SCHEDULING_GLOBAL_CONTROLr, SSW_TICK_PERIODf, &val)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    val = RTK_DEFAULT_QOS_SCHED_LB_PREIFP;
    if ((ret = reg_field_write(unit, SSW_PACKET_SCHEDULING_GLOBAL_CONTROLr, SSW_INC_IFGf, &val)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    val = RTK_DEFAULT_QOS_SCHED_LB_WFQ_HIGH_THRESHOLD;
    if ((ret = reg_field_write(unit, SSW_WFQ_GLOBAL_CONTROLr, SSW_WFQ_LB_SIZEf, &val)) != RT_ERR_OK)
    {
        QOS_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    for (port = 0; port < max_port; port++)
    {
        
        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }
        
        reg_id = SSW_AVERAGE_PACKET_RATE_PORT_0_LEAKY_BUCKET_PARAMETER0r + port * 5;
        field_id = SSW_P0_LB_SIZEf + port * 9;
        val = RTK_DEFAULT_QOS_SCHED_LB_APR_BURST_SIZE;
        if ((ret = reg_field_write(unit, reg_id, field_id, &val)) != RT_ERR_OK)
        {
            QOS_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
            return ret;
        }
    
        for (qid = 0; qid < queueNum; qid++)
        {
            reg_id = (SSW_AVERAGE_PACKET_RATE_PORT_0_LEAKY_BUCKET_PARAMETER1r + qid/2) + port * 5;
            field_id = aprLb_fieldidx[qid] + port * 9;
            val = RTK_DEFAULT_QOS_SCHED_LB_APR_RATE;
            if ((ret = reg_field_write(unit, reg_id, field_id, &val)) != RT_ERR_OK)
            {
                QOS_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
                return ret;
            }
        }
    }
    QOS_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of _dal_ssw_qos_init_config */

/* Function Name:
 *      _dal_ssw_qos_translateApiLayerQidToAsicLayerQid
 * Description:
 *      Set the scheduling types and weights of queues on specific port in egress scheduling.
 * Input:
 *      qnum        - queue number
 *      qid         - original qid
 *      pAsicQid    - asic qid
 * Output:
 *      None.
 * Return:
 *      None
 * Note:
 */
static void
_dal_ssw_qos_translateApiLayerQidToAsicLayerQid(uint32 qnum, rtk_qid_t qid, rtk_qid_t *pAsicQid)
{
    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_QOS), "qnum=%d, qid=%d", qnum, qid); 

    /* Assign the same value to asic queue id */
    *pAsicQid = qid;

    /* Translate */
    switch (qnum)
    {
        case 1:  /* Valid Queue ID in ASIC: 0 */
            break;
        case 2:  /* Valid Queue ID in ASIC: 0, 7 */
            if (1 == qid)
                *pAsicQid = 7;
            break;
        case 3:  /* Valid Queue ID in ASIC: 0, 1, 7 */
            if (2 == qid)
                *pAsicQid = 7;
            break;
        case 4:  /* Valid Queue ID in ASIC: 0, 1, 2, 7 */
            if (3 == qid)
                *pAsicQid = 7;
            break;
        case 5:  /* Valid Queue ID in ASIC: 0, 1, 2, 3, 7 */
            if (4 == qid)
                *pAsicQid = 7;
            break;
        case 6:  /* Valid Queue ID in ASIC: 0, 1, 2, 3, 4, 7 */
            if (5 == qid)
                *pAsicQid = 7;
            break;
        case 7:  /* Valid Queue ID in ASIC: 0, 1, 2, 3, 4, 5, 7 */
            if (6 == qid)
                *pAsicQid = 7;
            break;
        case 8:  /* Valid Queue ID in ASIC: 0, 1, 2, 3, 4, 5, 6, 7 */
        default:
            break;
    }

    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_QOS), "*pAsicQid=%d", 
           *pAsicQid);

    return;
} /* end of _dal_ssw_qos_translateApiLayerQidToAsicLayerQid */

/* Function Name:
 *      _dal_ssw_qos_translateAsicLayerQidToApiLayerQid
 * Description:
 *      Set the scheduling types and weights of queues on specific port in egress scheduling.
 * Input:
 *      qnum    - queue number
 *      qid     - original qid
 *      pApiQid - api qid
 * Output:
 *      None.
 * Return:
 *      None
 * Note:
 */
static void
_dal_ssw_qos_translateAsicLayerQidToApiLayerQid(uint32 qnum, rtk_qid_t qid, rtk_qid_t *pApiQid)
{
    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_QOS), "qnum=%d, qid=%d", qnum, qid); 

    /* Assign the same value to asic queue id */
    *pApiQid = qid;

    /* Translate */
    switch (qnum)
    {
        case 1:  /* ASIC Qid: 0;    API Qid: 0 */
            break;
        case 2:  /* ASIC Qid: 0,7;  API Qid: 0,1 */
            if (7 == qid)
                *pApiQid = 1;
            break;
        case 3:  /* ASIC Qid: 0,1,7; API Qid: 0,1,2 */
            if (7 == qid)
                *pApiQid = 2;
            break;
        case 4:  /* ASIC Qid: 0,1,2,7; API Qid: 0,1,2,3 */
            if (7 == qid)
                *pApiQid = 3;
            break;
        case 5:  /* ASIC Qid: 0,1,2,3,7; API Qid: 0,1,2,3,4 */
            if (7 == qid)
                *pApiQid = 4;
            break;
        case 6:  /* ASIC Qid: 0,1,2,3,4,7; API Qid: 0,1,2,3,4,5 */
            if (7 == qid)
                *pApiQid = 5;
            break;
        case 7:  /* ASIC Qid: 0,1,2,3,4,5,7; API Qid: 0,1,2,3,4,5,6 */
            if (7 == qid)
                *pApiQid = 6;
            break;
        case 8:  /* ASIC Qid: 0,1,2,3,4,5,6,7; API Qid: 0,1,2,3,4,5,6,7 */
        default:
            break;
    }

    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_QOS), "*pApiQid=%d", 
           *pApiQid);

    return;
} /* end of _dal_ssw_qos_translateAsicLayerQidToApiLayerQid */


/* Function Name:
 *      _dal_ssw_qos_outputQueueNumber_get
 * Description:
 *      get queue number for port
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      pQnum - queue number
 * Output:
 *      pQnum - queue number
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_QUEUE_NUM
 *      RT_ERR_PORT_ID
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_FAILED
 * Note:
 */
static int32
_dal_ssw_qos_outputQueueNumber_get(uint32 unit, rtk_port_t port, uint32 *pQNum)
{
    int32   ret;
    
    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_QOS), "unit=%d, port=%d", 
           unit, port);
       
    if ((ret = reg_field_read(unit, queue_number_control_regidx[port], portQueueNum_fieldidx[port], pQNum)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    if (0 == *pQNum)
    {
        *pQNum = HAL_MAX_NUM_OF_QUEUE(unit);
    }
    
    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_QOS), "pQNum=%d", *pQNum);
    
    return RT_ERR_OK;
} /* end of _dal_ssw_qos_outputQueueNumber_get */


/* Function Name:
 *      _dal_ssw_qos_outputQueueNumber_set
 * Description:
 *      Set queue number for port
 * Input:
 *      unit - unit id
 *      port - port id
 *      qnum - queue number
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 */
static int32
_dal_ssw_qos_outputQueueNumber_set(uint32 unit, rtk_port_t port, uint32 qNum)
{
    int32   ret;
    
    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_QOS), "unit=%d, port=%d, qNum=%d", 
           unit, port, qNum);
       
    if (HAL_MAX_NUM_OF_QUEUE(unit) == qNum )
    {
        qNum = 0;
    }
    
    if ((ret = reg_field_write(unit, queue_number_control_regidx[port], portQueueNum_fieldidx[port], &qNum)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    return RT_ERR_OK;
} /* end of _dal_ssw_qos_outputQueueNumber_set */


/* Function Name:
 *      _dal_ssw_qos_queueWeight_get
 * Description:
 *      Set queue number for port
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      qid        - queue id
 * Output:
 *      pQueueType - queue type
 *                     - WFQ_WRR_PRIORITY
 *                     - STRICT_PRIORITY
 *      pWeight    - weight of queue
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_PORT_ID
 *      RT_ERR_QUEUE_ID
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_FAILED
 * Note:
 */
static 
int32 _dal_ssw_qos_queueWeight_get(
    uint32                  unit, 
    rtk_port_t              port, 
    rtk_qid_t               qid, 
    rtk_qos_queue_type_t    *pQueueType, 
    uint32                  *pWeight)
{
    int32   ret;
    uint32  sp_en;
    uint32  value;
    rtk_ssw_reg_list_t  reg_id;

    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_QOS), "unit=%d, port=%d, qid=%d", 
           unit, port, qid);     
    
    reg_id = (SSW_WFQ_WRR_PORT_0_PARAMETER1r + qid/4) + port*3;
    
    /* get value from CHIP*/
    if ((ret = reg_read(unit, reg_id, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    if ((ret = reg_field_get(unit, reg_id, (portQueueWeight_fieldidx[qid] + port * 18), pWeight, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    if ((ret = reg_field_get(unit, reg_id, (portStrictEn_fieldidx[qid] + port * 18), &sp_en, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    
    if (1 == sp_en)
    {
        *pQueueType = STRICT_PRIORITY;
    } 
    else 
    {
        *pQueueType = WFQ_WRR_PRIORITY;
    }
    
    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_QOS), "pQueueType=%d, pWeight=%d", 
           *pQueueType, *pWeight);
    
    return RT_ERR_OK;
} /* end of _dal_ssw_qos_queueWeight_get */

/* Function Name:
 *      _dal_ssw_qos_queueWeight_set
 * Description:
 *      Set queue number for port
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      qid       - queue id
 *      queueType - queue type
 *                    - WFQ_WRR_PRIORITY
 *                    - STRICT_PRIORITY
 *      weight    - weight of queue
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_PORT_ID
 *      RT_ERR_QUEUE_ID
 *      RT_ERR_QOS_SCHE_TYPE
 *      RT_ERR_QOS_QUEUE_WEIGHT
 *      RT_ERR_FAILED
 * Note:
 */
static 
int32 _dal_ssw_qos_queueWeight_set(
    uint32                  unit, 
    rtk_port_t              port, 
    rtk_qid_t               qid, 
    rtk_qos_queue_type_t    queueType, 
    uint32                  weight)
{
    uint32  sp_en;
    uint32  value;
    int32   ret;
    rtk_ssw_reg_list_t  reg_id;

    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_QOS), "unit=%d, port=%d, qid=%d, queueType=%d\
           weight=%d", 
           unit, port, qid, queueType, weight);
    
    /* parameter check */
    RT_PARAM_CHK(queueType >= QUEUE_TYPE_END, RT_ERR_QOS_SCHE_TYPE);
    RT_PARAM_CHK(weight > HAL_QUEUE_WEIGHT_MAX(unit), RT_ERR_QOS_QUEUE_WEIGHT);
    
    if (STRICT_PRIORITY == queueType)
    {
        sp_en = 1;
    } 
    else 
    {
        sp_en = 0;
    }
    
    reg_id = (SSW_WFQ_WRR_PORT_0_PARAMETER1r + qid/4) + port*3;
    
    /* get value from CHIP*/
    if ((ret = reg_read(unit, reg_id, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    /* weight (1~128) -> chip value (0~127) */
    weight = weight - 1;
    if ((ret = reg_field_set(unit, reg_id, (portQueueWeight_fieldidx[qid] + port * 18), &weight, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    if ((ret = reg_field_set(unit, reg_id, (portStrictEn_fieldidx[qid] + port * 18), &sp_en, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    if ((ret = reg_write(unit, reg_id, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    
    return RT_ERR_OK;
} /* end of _dal_ssw_qos_queueWeight_set */

static int32
_dal_ssw_qos_cpuQueueNum_set(uint32 unit, uint32 queue_num)
{
    int32       ret;
    rtk_qid_t   qid;
    rtk_port_t  cpuPort;   

    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_QOS), "unit=%d, queue_num=%d", unit, queue_num);

    /* parameter check */
    RT_PARAM_CHK(queue_num > HAL_MAX_NUM_OF_QUEUE(unit), RT_ERR_QUEUE_NUM);
    
    cpuPort = HAL_GET_CPU_PORT(unit);
    
    /* E0005358: clean Queue */
    if ((ret = _dal_ssw_qos_cleanQosQueue(unit)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    if ((ret = _dal_ssw_qos_outputQueueNumber_set(unit, cpuPort, queue_num)) != RT_ERR_OK)
    {
        /* E0005358: restart Rx */
        _dal_ssw_qos_restartRx(unit);
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    for (qid = 0; qid <= (HAL_MAX_NUM_OF_QUEUE(unit) - 1); qid++)
    {
        if ((ret = _dal_ssw_qos_queueWeight_set(unit, cpuPort, qid
                            , RTK_DEFAULT_QOS_SCHED_QUEUE_TYPE
                            , qos_scheduling_queueWeight[queue_num-1][qid])) != RT_ERR_OK)
        {
            /* E0005358: restart Rx */
            _dal_ssw_qos_restartRx(unit);
            RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
            return ret;
        }
    }
    
    /* E0005358: restart Rx */
    if ((ret = _dal_ssw_qos_restartRx(unit)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_QOS), "");
        return ret;
    }
    
    return RT_ERR_OK;
} /* end of _dal_ssw_qos_cpuQueueNum_set */


/* Function Name:
 *      _dal_ssw_qos_cleanQosQueue
 * Description:
 *      E0005358: Clean all packet in Queue
 * Input:
 *      unit - unit id
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      In 8389, it need to clean Queue before change some queue setting.
 *      After execute this function, must re-enable rx again
 */
static 
int32 _dal_ssw_qos_cleanQosQueue(uint32 unit)
{
    uint32  check_times;
    uint32  qEmpty;
    int32   ret;
    rtk_port_t  port, max_port;
    rtk_enable_t    port_admin_status;
    
    max_port = HAL_GET_MAX_PORT(unit);
    
    /* disable rx in all port */
    for (port = 0; port < max_port; port++)
    {
        
        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }
        
        if ((ret = dal_ssw_port_rxEnable_set(unit, port, DISABLED)) != RT_ERR_OK)
        {
            return ret;
        }
    }

    qEmpty = 0;
    /* Wait a while and check whether queue is clean */
    check_times = 0;

    do 
    {
        check_times++;
        if ((ret = reg_field_read(unit, SSW_LINK_AGGREGATION_CONTROL1r, SSW_QEMPTYf, &qEmpty)) != RT_ERR_OK)
        {
            continue;
        }
    } 
    while((qEmpty != QUEUE_EMPTY_VALUE) && (check_times < QOS_CHK_QUEUE_EMPTY_TIMES));

    if (check_times < QOS_CHK_QUEUE_EMPTY_TIMES)
    {
        return RT_ERR_OK;
    }
    
    /* if queue is not clean, disable tx, too */
    for (port = 0; port < max_port; port++)
    {
        
        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }
        
        if ((ret = dal_ssw_port_txEnable_set(unit, port, DISABLED)) != RT_ERR_OK)
        {
            return ret;
        }
    }
    
    /* Wait a while and check whether queue is clean */
    qEmpty = 0;
    check_times = 0;
    do 
    {
        check_times++;
        if ((ret = reg_field_read(unit, SSW_LINK_AGGREGATION_CONTROL1r, SSW_QEMPTYf, &qEmpty)) != RT_ERR_OK)
        {
            continue;
        }
    } 
    while ((qEmpty != QUEUE_EMPTY_VALUE) && (check_times < QOS_CHK_QUEUE_EMPTY_TIMES));
    
    /* re-enable Tx first, Rx should be enabled after completing Queue configure */
    for (port = 0; port < max_port; port++)
    {
        
        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }
        
        if (( ret = dal_ssw_port_adminEnable_get(unit, port, &port_admin_status)) != RT_ERR_OK)
        {
            return ret;
        }
        
        /* if original port status is not enable, don't need to enable Tx on this port */
        if (port_admin_status != ENABLED)
        {
            continue;
        }
        
        if (( ret = dal_ssw_port_txEnable_set(unit, port, ENABLED)) != RT_ERR_OK)
        {
            return ret;
        }
    }

    return RT_ERR_OK;
} /* end of _dal_ssw_qos_cleanQosQueue */

/* Function Name:
 *      _dal_ssw_qos_restartRx
 * Description:
 *      E0005358: Used to restart RX after Clean all packet in Queue
 * Input:
 *      unit - unit id
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      
 */
static 
int32 _dal_ssw_qos_restartRx(uint32 unit)
{
    rtk_port_t  port, max_port;
    int32       ret;
    rtk_enable_t    port_admin_status;
    
    max_port = HAL_GET_MAX_PORT(unit);
    
    /* enable rx in all port */
    for (port = 0; port < max_port; port++)
    {   
        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }
        
        if (( ret = dal_ssw_port_adminEnable_get(unit, port, &port_admin_status)) != RT_ERR_OK)
        {
            return ret;
        }
        
        /* if original port status is not enable, don't need to enable rx on this port */
        if (port_admin_status != ENABLED)
        {
            continue;
        }
        
        if (( ret = dal_ssw_port_rxEnable_set(unit, port, ENABLED)) != RT_ERR_OK)
        {
            return ret;
        }
    }
    
    return RT_ERR_OK;
} /* end of _dal_ssw_qos_restartRx */


