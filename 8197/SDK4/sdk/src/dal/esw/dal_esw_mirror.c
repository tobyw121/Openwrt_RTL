/*
 * Copyright(c) Realtek Semiconductor Corporation, 2009
 * All rights reserved.
 *
 * $Revision: 32754 $
 * $Date: 2012-09-17 16:29:08 +0800 (Mon, 17 Sep 2012) $
 *
 * Purpose : Definition those public MIRROR APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) Port-based mirror
 *           2) Group-based mirror
 *           3) RSPAN
 *           4) Mirror-based SFLOW
 *           5) Port-based SFLOW
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
#include <dal/esw/dal_esw_mirror.h>
#include <rtk/default.h>
#include <rtk/mirror.h>


/*
 * Symbol Definition
 */

typedef struct sflow_sample_conf_s
{
    rtk_enable_t    status;
    uint32          rate;
} sflow_sample_conf_t;

typedef struct sflow_port_sample_conf_s
{
    sflow_sample_conf_t igr[RTK_MAX_NUM_OF_PORTS];
    sflow_sample_conf_t egr[RTK_MAX_NUM_OF_PORTS];
} sflow_port_sample_conf_t;

/*
 * Data Declaration
 */
static uint32               mirror_init[RTK_MAX_NUM_OF_UNIT];
static osal_mutex_t         mirror_sem[RTK_MAX_NUM_OF_UNIT];
static sflow_port_sample_conf_t     *pSflow_port_sample_conf[RTK_MAX_NUM_OF_UNIT];
static sflow_sample_conf_t          *pSflow_mirror_sample_conf[RTK_MAX_NUM_OF_UNIT];

const static uint16 traffic_mirror_table_entry_0_regidx[] = 
        {ESW_TRAFFIC_MIRROR_TABLE_ENTRY0_0r, ESW_TRAFFIC_MIRROR_TABLE_ENTRY1_0r, ESW_TRAFFIC_MIRROR_TABLE_ENTRY2_0r, ESW_TRAFFIC_MIRROR_TABLE_ENTRY3_0r};
const static uint16 traffic_mirror_table_entry_1_regidx[] = 
        {ESW_TRAFFIC_MIRROR_TABLE_ENTRY0_1r, ESW_TRAFFIC_MIRROR_TABLE_ENTRY1_1r, ESW_TRAFFIC_MIRROR_TABLE_ENTRY2_1r, ESW_TRAFFIC_MIRROR_TABLE_ENTRY3_1r};
const static uint16 traffic_mirror_table_entry_2_regidx[] = 
        {ESW_TRAFFIC_MIRROR_TABLE_ENTRY0_2r, ESW_TRAFFIC_MIRROR_TABLE_ENTRY1_2r, ESW_TRAFFIC_MIRROR_TABLE_ENTRY2_2r, ESW_TRAFFIC_MIRROR_TABLE_ENTRY3_2r};
const static uint16 total_counter_for_traffic_mirror_table_entry_regidx[] = 
        {ESW_TOTAL_COUNTER_FOR_TRAFFIC_MIRROR_TABLE_ENTRY_0r, ESW_TOTAL_COUNTER_FOR_TRAFFIC_MIRROR_TABLE_ENTRY_1r, ESW_TOTAL_COUNTER_FOR_TRAFFIC_MIRROR_TABLE_ENTRY_2r, ESW_TOTAL_COUNTER_FOR_TRAFFIC_MIRROR_TABLE_ENTRY_3r};
const static uint16 counter_for_traffic_mirror_table_entry_regidx[] = 
        {ESW_COUNTER_FOR_TRAFFIC_MIRROR_TABLE_ENTRY_0r, ESW_COUNTER_FOR_TRAFFIC_MIRROR_TABLE_ENTRY_1r, ESW_COUNTER_FOR_TRAFFIC_MIRROR_TABLE_ENTRY_2r, ESW_COUNTER_FOR_TRAFFIC_MIRROR_TABLE_ENTRY_3r};
const static uint16 sample_rate_for_traffic_mirror_table_entry_regidx[] = 
        {ESW_SAMPLE_RATE_FOR_TRAFFIC_MIRROR_TABLE_ENTRY_0r, ESW_SAMPLE_RATE_FOR_TRAFFIC_MIRROR_TABLE_ENTRY_1r, ESW_SAMPLE_RATE_FOR_TRAFFIC_MIRROR_TABLE_ENTRY_2r, ESW_SAMPLE_RATE_FOR_TRAFFIC_MIRROR_TABLE_ENTRY_3r};



/*
 * Macro Declaration
 */
#define MIRROR_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(mirror_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_MIRROR|MOD_DAL), "semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)

#define MIRROR_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(mirror_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_MIRROR|MOD_DAL), "semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)


/*
 * Function Declaration
 */

/* Module Name : Mirror */

/* Function Name:
 *      dal_esw_mirror_init
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
dal_esw_mirror_init(uint32 unit)
{
    rtk_port_t  port;
    uint32      mirror_id;
    
    RT_DBG(LOG_DEBUG, (MOD_MIRROR|MOD_DAL), "unit=%d", unit); 
    
    mirror_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    mirror_sem[unit] = osal_sem_mutex_create();
    if (0 == mirror_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_MIRROR|MOD_DAL), "semaphore create failed");
        return RT_ERR_FAILED;
    }
    
    pSflow_port_sample_conf[unit] = (sflow_port_sample_conf_t *) osal_alloc(sizeof(sflow_port_sample_conf_t));
    
    if (0 == pSflow_port_sample_conf[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_MIRROR|MOD_DAL), "sflow sampling configure allocate failed");
        return RT_ERR_FAILED;
    }
    
    pSflow_mirror_sample_conf[unit] = (sflow_sample_conf_t *) osal_alloc(HAL_MAX_NUM_OF_MIRROR(unit)*sizeof(sflow_sample_conf_t));
    
    if (0 == pSflow_mirror_sample_conf[unit])
    {
        osal_free(pSflow_port_sample_conf[unit]);
        pSflow_port_sample_conf[unit] = 0;
        RT_ERR(RT_ERR_FAILED, (MOD_MIRROR|MOD_DAL), "sflow sampling configure allocate failed");
        return RT_ERR_FAILED;
    }
    
    for (port = 0; port <= HAL_GET_MAX_ETHER_PORT(unit); port++)
    {
        pSflow_port_sample_conf[unit]->igr[port].rate = 0;
        pSflow_port_sample_conf[unit]->igr[port].status = DISABLED;
        pSflow_port_sample_conf[unit]->egr[port].rate = 0;
        pSflow_port_sample_conf[unit]->egr[port].status = DISABLED;
    }
    
    for (mirror_id = 0; mirror_id <= HAL_MIRROR_ID_MAX(unit); mirror_id++)
    {
        pSflow_mirror_sample_conf[unit][mirror_id].rate = 0;
        pSflow_mirror_sample_conf[unit][mirror_id].status = DISABLED;
    }
    
    mirror_init[unit] = INIT_COMPLETED;
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_init */

/* Module Name    : Mirror            */
/* Sub-module Name: Port-based mirror */

/* Function Name:
 *      dal_esw_mirror_portBased_create
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
 *      None
 */
int32
dal_esw_mirror_portBased_create(uint32 unit, rtk_port_t mirroring_port)
{
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_portBased_create */


/* Function Name:
 *      dal_esw_mirror_portBased_destroy
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
dal_esw_mirror_portBased_destroy(uint32 unit, rtk_port_t mirroring_port)
{
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_portBased_destroy */


/* Function Name:
 *      dal_esw_mirror_portBased_destroyAll
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
 *      None
 */
int32
dal_esw_mirror_portBased_destroyAll(uint32 unit)
{
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_portBased_destroyAll */


/* Function Name:
 *      dal_esw_mirror_portBased_get
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
dal_esw_mirror_portBased_get(
    uint32          unit,
    rtk_port_t      mirroring_port,
    rtk_portmask_t  *pMirrored_rx_portmask,
    rtk_portmask_t  *pMirrored_tx_portmask)
{
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_portBased_get */


/* Function Name:
 *      dal_esw_mirror_portBased_set
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
 *      None
 */
int32
dal_esw_mirror_portBased_set(
    uint32          unit,
    rtk_port_t      mirroring_port,
    rtk_portmask_t  *pMirrored_rx_portmask,
    rtk_portmask_t  *pMirrored_tx_portmask)
{
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_portBased_set */

/* Module Name    : Mirror             */
/* Sub-module Name: Group-based mirror */

/* Function Name:
 *      dal_esw_mirror_group_init
 * Description:
 *      Initialization mirror group entry.
 * Input:
 *      unit         - unit id
 *      pMirrorEntry - mirror entry
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_MIRROR_ID        - invalid mirror id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_mirror_group_init(
    uint32              unit,
    rtk_mirror_entry_t  *pMirrorEntry)
{
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_PARAM_CHK((NULL == pMirrorEntry), RT_ERR_NULL_POINTER);
    
    pMirrorEntry->oper_of_igr_and_egr_ports = 0;
    pMirrorEntry->cross_vlan = ENABLED;
    pMirrorEntry->mirror_ucast = ENABLED;
    pMirrorEntry->mirror_mcast = ENABLED;
    pMirrorEntry->mirror_bcast = ENABLED;
    pMirrorEntry->mirror_goodPkt = ENABLED;
    pMirrorEntry->mirror_badPkt = ENABLED;
    pMirrorEntry->mirror_orginalPkt = ENABLED;
    pMirrorEntry->flowBasedOnly = DISABLED;
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_group_init */

/* Function Name:
 *      dal_esw_mirror_group_get
 * Description:
 *      Get mirror group entry.
 * Input:
 *      unit         - unit id
 *      mirror_id    - mirror id
 * Output:
 *      pMirrorEntry - mirror entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_MIRROR_ID        - invalid mirror id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_mirror_group_get(
    uint32              unit,
    uint32              mirror_id,
    rtk_mirror_entry_t  *pMirrorEntry)
{
    int32   ret;
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, pMirrorEntry=%x"
            , unit, pMirrorEntry);
    
    RT_PARAM_CHK(mirror_id > HAL_MIRROR_ID_MAX(unit), RT_ERR_MIRROR_ID);
    RT_PARAM_CHK((NULL == pMirrorEntry), RT_ERR_NULL_POINTER);
    
    MIRROR_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    osal_memset(pMirrorEntry, 0, sizeof(rtk_mirror_entry_t));
    if ((ret = reg_field_read(unit, traffic_mirror_table_entry_0_regidx[mirror_id]
                        , ESW_SPMf, &(pMirrorEntry->mirrored_igrPorts.bits[0]))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_field_read(unit, traffic_mirror_table_entry_0_regidx[mirror_id]
                        , ESW_SPMDPMOPf, &(pMirrorEntry->oper_of_igr_and_egr_ports))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_field_read(unit, traffic_mirror_table_entry_0_regidx[mirror_id]
                        , ESW_CROSSVLANf, &(pMirrorEntry->cross_vlan))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_field_read(unit, traffic_mirror_table_entry_1_regidx[mirror_id]
                        , ESW_DPMf, &(pMirrorEntry->mirrored_egrPorts.bits[0]))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_field_read(unit, traffic_mirror_table_entry_1_regidx[mirror_id]
                        , ESW_MUAf, &(pMirrorEntry->mirror_ucast))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_field_read(unit, traffic_mirror_table_entry_1_regidx[mirror_id]
                        , ESW_MMAf, &(pMirrorEntry->mirror_mcast))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_field_read(unit, traffic_mirror_table_entry_1_regidx[mirror_id]
                        , ESW_MBAf, &(pMirrorEntry->mirror_bcast))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_field_read(unit, traffic_mirror_table_entry_2_regidx[mirror_id]
                        , ESW_MBPKTf, &(pMirrorEntry->mirror_badPkt))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_field_read(unit, traffic_mirror_table_entry_2_regidx[mirror_id]
                        , ESW_MGPKTf, &(pMirrorEntry->mirror_goodPkt))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_field_read(unit, traffic_mirror_table_entry_2_regidx[mirror_id]
                        , ESW_MORGf, &(pMirrorEntry->mirror_orginalPkt))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_field_read(unit, traffic_mirror_table_entry_2_regidx[mirror_id]
                        , ESW_FLOWBASEDONLYf, &(pMirrorEntry->flowBasedOnly))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_field_read(unit, traffic_mirror_table_entry_2_regidx[mirror_id]
                        , ESW_DPf, &(pMirrorEntry->mirroring_port))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    MIRROR_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_group_get */

/* Function Name:
 *      dal_esw_mirror_group_set
 * Description:
 *      Set mirror group entry.
 * Input:
 *      unit         - unit id
 *      mirror_id    - mirror id
 *      pMirrorEntry - mirror entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_MIRROR_ID        - invalid mirror id
 *      RT_ERR_PORT_ID          - invalid mirroring port id
 *      RT_ERR_PORT_MASK        - invalid mirrored ingress or egress portmask
 * Note:
 *      None
 */
int32
dal_esw_mirror_group_set(
    uint32              unit,
    uint32              mirror_id,
    rtk_mirror_entry_t  *pMirrorEntry)
{
    int32   ret;
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, pMirrorEntry=%x"
            , unit, pMirrorEntry);
    
    RT_PARAM_CHK(mirror_id > HAL_MIRROR_ID_MAX(unit), RT_ERR_MIRROR_ID);
    RT_PARAM_CHK((NULL == pMirrorEntry), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, pMirrorEntry->mirroring_port) && (pMirrorEntry->mirroring_port != 31), RT_ERR_PORT_ID);
    RT_PARAM_CHK(pMirrorEntry->oper_of_igr_and_egr_ports >= RTK_ENABLE_END, RT_ERR_INPUT);
    RT_PARAM_CHK(pMirrorEntry->cross_vlan >= RTK_ENABLE_END, RT_ERR_INPUT);
    RT_PARAM_CHK(pMirrorEntry->mirror_ucast >= RTK_ENABLE_END, RT_ERR_INPUT);
    RT_PARAM_CHK(pMirrorEntry->mirror_mcast >= RTK_ENABLE_END, RT_ERR_INPUT);
    RT_PARAM_CHK(pMirrorEntry->mirror_bcast >= RTK_ENABLE_END, RT_ERR_INPUT);
    RT_PARAM_CHK(pMirrorEntry->mirror_goodPkt >= RTK_ENABLE_END, RT_ERR_INPUT);
    RT_PARAM_CHK(pMirrorEntry->mirror_badPkt >= RTK_ENABLE_END, RT_ERR_INPUT);
    RT_PARAM_CHK(pMirrorEntry->mirror_orginalPkt >= RTK_ENABLE_END, RT_ERR_INPUT);
    RT_PARAM_CHK(pMirrorEntry->flowBasedOnly >= RTK_ENABLE_END, RT_ERR_INPUT);
    
    MIRROR_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_write(unit, traffic_mirror_table_entry_0_regidx[mirror_id]
                        , ESW_SPMf, &(pMirrorEntry->mirrored_igrPorts.bits[0]))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_field_write(unit, traffic_mirror_table_entry_0_regidx[mirror_id]
                        , ESW_SPMDPMOPf, &(pMirrorEntry->oper_of_igr_and_egr_ports))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_field_write(unit, traffic_mirror_table_entry_0_regidx[mirror_id]
                        , ESW_CROSSVLANf, &(pMirrorEntry->cross_vlan))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_field_write(unit, traffic_mirror_table_entry_1_regidx[mirror_id]
                        , ESW_DPMf, &(pMirrorEntry->mirrored_egrPorts.bits[0]))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_field_write(unit, traffic_mirror_table_entry_1_regidx[mirror_id]
                        , ESW_MUAf, &(pMirrorEntry->mirror_ucast))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_field_write(unit, traffic_mirror_table_entry_1_regidx[mirror_id]
                        , ESW_MMAf, &(pMirrorEntry->mirror_mcast))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_field_write(unit, traffic_mirror_table_entry_1_regidx[mirror_id]
                        , ESW_MBAf, &(pMirrorEntry->mirror_bcast))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_field_write(unit, traffic_mirror_table_entry_2_regidx[mirror_id]
                        , ESW_MBPKTf, &(pMirrorEntry->mirror_badPkt))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_field_write(unit, traffic_mirror_table_entry_2_regidx[mirror_id]
                        , ESW_MGPKTf, &(pMirrorEntry->mirror_goodPkt))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_field_write(unit, traffic_mirror_table_entry_2_regidx[mirror_id]
                        , ESW_MORGf, &(pMirrorEntry->mirror_orginalPkt))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_field_write(unit, traffic_mirror_table_entry_2_regidx[mirror_id]
                        , ESW_FLOWBASEDONLYf, &(pMirrorEntry->flowBasedOnly))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_field_write(unit, traffic_mirror_table_entry_2_regidx[mirror_id]
                        , ESW_DPf, &(pMirrorEntry->mirroring_port))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    MIRROR_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_group_set */

/* Function Name:
 *      dal_esw_mirror_egrMode_get
 * Description:
 *      Get egress filter mode on specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pEgrMode - pointer to egress filter mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Egress filter mode is as following:
 *      - FORWARD_ALL_PKTS
 *      - FORWARD_MIRRORED_PKTS_ONLY
 */
int32
dal_esw_mirror_egrMode_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_mirror_egrMode_t    *pEgrMode)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, port=%d, pEgrMode=%x"
            , unit, port, pEgrMode);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEgrMode), RT_ERR_NULL_POINTER);
    
    MIRROR_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_MIRROR_EGRESS_FILTER_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_MIREGFILTERf, &value)) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    MIRROR_SEM_UNLOCK(unit);
    
    switch (value)
    {
        case 0:
            *pEgrMode = FORWARD_ALL_PKTS;
            break;
        
        case 1:
            *pEgrMode = FORWARD_MIRRORED_PKTS_ONLY;
            break;
        
        default:
            return RT_ERR_FAILED;
    }
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_egrMode_get */

/* Function Name:
 *      dal_esw_mirror_egrMode_set
 * Description:
 *      Set egress filter mode on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      egrMode - egress filter mode
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
 *      Egress filter mode is as following:
 *      - FORWARD_ALL_PKTS
 *      - FORWARD_MIRRORED_PKTS_ONLY
 */
int32
dal_esw_mirror_egrMode_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_mirror_egrMode_t    egrMode)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, port=%d, egrMode=%x"
            , unit, port, egrMode);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    switch (egrMode)
    {
        case FORWARD_ALL_PKTS:
            value = 0;
            break;
        
        case FORWARD_MIRRORED_PKTS_ONLY:
            value = 1;
            break;
        
        default:
            return RT_ERR_INPUT;
    }
    
    MIRROR_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_MIRROR_EGRESS_FILTER_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_MIREGFILTERf, &value)) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    MIRROR_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_egrMode_set */

/* Module Name    : Mirror */
/* Sub-module Name: RSPAN  */
/* Function Name:
 *      dal_esw_mirror_portRspanIgrMode_get
 * Description:
 *      Get ingress mode of RSPAN on specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pIgrMode - pointer to ingress mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Ingress mode is as following:
 *      - RSPAN_IGR_HANDLE_RSPAN_TAG
 *      - RSPAN_IGR_IGNORE_RSPAN_TAG
 */
int32
dal_esw_mirror_portRspanIgrMode_get(uint32 unit, rtk_port_t port, rtk_mirror_rspanIgrMode_t *pIgrMode)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, port=%d, pIgrMode=%x"
            , unit, port, pIgrMode);
    
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pIgrMode), RT_ERR_NULL_POINTER);
    
    MIRROR_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_REMOTE_MIRROR_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_RXRSPTAGf, &value)) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    MIRROR_SEM_UNLOCK(unit);
    
    switch (value)
    {
        case 0:
            *pIgrMode = RSPAN_IGR_IGNORE_RSPAN_TAG;
            break;
        
        case 1:
            *pIgrMode = RSPAN_IGR_HANDLE_RSPAN_TAG;
            break;
        
        default:
            return RT_ERR_FAILED;
    }
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_portRspanIgrMode_get */

/* Function Name:
 *      dal_esw_mirror_portRspanIgrMode_set
 * Description:
 *      Set ingress mode of RSPAN on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      igrMode - ingress mode
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
 *      Ingress mode is as following:
 *      - RSPAN_IGR_HANDLE_RSPAN_TAG
 *      - RSPAN_IGR_IGNORE_RSPAN_TAG
 */
int32
dal_esw_mirror_portRspanIgrMode_set(uint32 unit, rtk_port_t port, rtk_mirror_rspanIgrMode_t igrMode)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, port=%d, igrMode=%u"
            , unit, port, igrMode);
    
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    
    switch (igrMode)
    {
        case RSPAN_IGR_IGNORE_RSPAN_TAG:
            value = 0;
            break;
        
        case RSPAN_IGR_HANDLE_RSPAN_TAG:
            value = 1;
            break;
        
        default:
            return RT_ERR_INPUT;
    }
    
    MIRROR_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_REMOTE_MIRROR_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_RXRSPTAGf, &value)) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    MIRROR_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_portRspanIgrMode_set */

/* Function Name:
 *      dal_esw_mirror_portRspanEgrMode_get
 * Description:
 *      Get egress mode of RSPAN on specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pEgrMode - pointer to egress mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Ingress mode is as following:
 *      - RSPAN_EGR_REMOVE_TAG
 *      - RSPAN_EGR_ADD_TAG
 *      - RSPAN_EGR_NO_MODIFY
 */
int32
dal_esw_mirror_portRspanEgrMode_get(uint32 unit, rtk_port_t port, rtk_mirror_rspanEgrMode_t *pEgrMode)
{
    int32   ret;
    uint32  addTag, removeTag;
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, port=%d, pEgrMode=%x"
            , unit, port, pEgrMode);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEgrMode), RT_ERR_NULL_POINTER);
    
    MIRROR_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_REMOTE_MIRROR_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_TXADDRSPTAGf, &addTag)) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_array_field_read(unit, ESW_PORT_REMOTE_MIRROR_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_TXRMRSPTAGf, &removeTag)) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    MIRROR_SEM_UNLOCK(unit);
    
    switch ((addTag << 1)|removeTag)
    {
        case 0:
            *pEgrMode = RSPAN_EGR_NO_MODIFY;
            break;
        
        case 1:
            *pEgrMode = RSPAN_EGR_REMOVE_TAG;
            break;
        
        case 2:
            *pEgrMode = RSPAN_EGR_ADD_TAG;
            break;
        
        default:
            return RT_ERR_FAILED;
    }
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_portRspanEgrMode_get */

/* Function Name:
 *      dal_esw_mirror_portRspanEgrMode_set
 * Description:
 *      Set egress mode of RSPAN on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      egrMode - egress mode
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
 *      Ingress mode is as following:
 *      - RSPAN_EGR_HANDLE_RSPAN_TAG
 *      - RSPAN_EGR_IGNORE_RSPAN_TAG
 */
int32
dal_esw_mirror_portRspanEgrMode_set(uint32 unit, rtk_port_t port, rtk_mirror_rspanEgrMode_t egrMode)
{
    int32   ret;
    uint32  addTag, removeTag;
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, port=%d, egrMode=%u"
            , unit, port, egrMode);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    switch (egrMode)
    {
        case RSPAN_EGR_NO_MODIFY:
            addTag = 0;
            removeTag = 0;
            break;
        
        case RSPAN_EGR_REMOVE_TAG:
            addTag = 0;
            removeTag = 1;
            break;
        
        case RSPAN_EGR_ADD_TAG:
            addTag = 1;
            removeTag = 0;
            break;
        
        default:
            return RT_ERR_INPUT;
    }
    
    MIRROR_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_REMOTE_MIRROR_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_TXADDRSPTAGf, &addTag)) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_array_field_write(unit, ESW_PORT_REMOTE_MIRROR_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_TXRMRSPTAGf, &removeTag)) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    MIRROR_SEM_UNLOCK(unit);
    
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_portRspanEgrMode_set */

/* Function Name:
 *      dal_esw_mirror_rspanIgrTag_get
 * Description:
 *      Get content of ingress tag on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pIgrTag - pointer to content of ingress tag
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
dal_esw_mirror_rspanIgrTag_get(uint32 unit, rtk_port_t port, rtk_mirror_rspanIgrTag_t *pIgrTag)
{
    int32   ret;
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, port=%d, pIgrTag=%x"
            , unit, port, pIgrTag);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pIgrTag), RT_ERR_NULL_POINTER);
    
    MIRROR_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_RX_RSPAN_VLAN_TAG_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_RXRSPANVIDf, &(pIgrTag->vid))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_array_field_read(unit, ESW_PORT_RX_RSPAN_VLAN_TAG_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_RXRSPANTPIDf, &(pIgrTag->tpid))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    MIRROR_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_rspanIgrTag_get */

/* Function Name:
 *      dal_esw_mirror_rspanIgrTag_set
 * Description:
 *      Set content of ingress tag on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      pIgrTag - content of ingress tag
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_mirror_rspanIgrTag_set(uint32 unit, rtk_port_t port, rtk_mirror_rspanIgrTag_t *pIgrTag)
{
    int32   ret;
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, port=%d, pIgrTag=%x"
            , unit, port, pIgrTag);
    
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pIgrTag), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pIgrTag->vid < RTK_VLAN_ID_MIN) || (pIgrTag->vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    RT_PARAM_CHK((pIgrTag->tpid > RTK_ETHERTYPE_MAX), RT_ERR_ETHER_TYPE);
    
    MIRROR_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_RX_RSPAN_VLAN_TAG_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_RXRSPANVIDf, &(pIgrTag->vid))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_array_field_write(unit, ESW_PORT_RX_RSPAN_VLAN_TAG_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_RXRSPANTPIDf, &(pIgrTag->tpid))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    MIRROR_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_rspanIgrTag_set */

/* Function Name:
 *      dal_esw_mirror_rspanEgrTag_get
 * Description:
 *      Get content of egress tag on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEgrTag - pointer to content of egress tag
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
dal_esw_mirror_rspanEgrTag_get(uint32 unit, rtk_port_t port, rtk_mirror_rspanEgrTag_t *pEgrTag)
{
    int32   ret;
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, port=%d, pEgrTag=%x"
            , unit, port, pEgrTag);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEgrTag), RT_ERR_NULL_POINTER);
    
    MIRROR_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_TX_RSPAN_VLAN_TAG_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_TXRSPANVIDf, &(pEgrTag->vid))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_array_field_read(unit, ESW_PORT_TX_RSPAN_VLAN_TAG_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_TXRSPANTPIDf, &(pEgrTag->tpid))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_array_field_read(unit, ESW_PORT_TX_RSPAN_VLAN_TAG_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_TXRSPANPRIf, &(pEgrTag->pri))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_array_field_read(unit, ESW_PORT_TX_RSPAN_VLAN_TAG_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_TXRSPANCFIf, &(pEgrTag->cfi))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    MIRROR_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_rspanEgrTag_get */

/* Function Name:
 *      dal_esw_mirror_rspanEgrTag_set
 * Description:
 *      Set content of egress tag on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      pEgrTag - content of egress tag
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_mirror_rspanEgrTag_set(uint32 unit, rtk_port_t port, rtk_mirror_rspanEgrTag_t *pEgrTag)
{
    int32   ret;
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, port=%d, pEgrTag=%x"
            , unit, port, pEgrTag);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEgrTag), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pEgrTag->vid < RTK_VLAN_ID_MIN) || (pEgrTag->vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    RT_PARAM_CHK(pEgrTag->pri > RTK_DOT1P_PRIORITY_MAX, RT_ERR_PRIORITY);
    RT_PARAM_CHK((pEgrTag->tpid > RTK_ETHERTYPE_MAX), RT_ERR_ETHER_TYPE);
    
    MIRROR_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_TX_RSPAN_VLAN_TAG_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_TXRSPANVIDf, &(pEgrTag->vid))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_array_field_write(unit, ESW_PORT_TX_RSPAN_VLAN_TAG_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_TXRSPANTPIDf, &(pEgrTag->tpid))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_array_field_write(unit, ESW_PORT_TX_RSPAN_VLAN_TAG_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_TXRSPANPRIf, &(pEgrTag->pri))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_array_field_write(unit, ESW_PORT_TX_RSPAN_VLAN_TAG_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_TXRSPANCFIf, &(pEgrTag->cfi))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    MIRROR_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_rspanEgrTag_set */

/* Module Name    : Mirror             */
/* Sub-module Name: Mirror-based SFLOW */

/* Function Name:
 *      dal_esw_mirror_sflowMirrorSeed_get
 * Description:
 *      Get sampling seed of sflow for mirror group sampling.
 * Input:
 *      unit  - unit id
 * Output:
 *      pSeed - pointer to sampling seed of sflow
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_mirror_sflowMirrorSeed_get(uint32 unit, uint32 *pSeed)
{
    int32   ret;
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, pSeed=%x"
            , unit, pSeed);
    
    RT_PARAM_CHK((NULL == pSeed), RT_ERR_NULL_POINTER);
    
    MIRROR_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, ESW_SEED_FOR_TRAFFIC_MIRROR_TABLEr
                        , ESW_SEEDf, pSeed)) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    MIRROR_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_sflowMirrorSeed_get */

/* Function Name:
 *      dal_esw_mirror_sflowMirrorSeed_set
 * Description:
 *      Set sampling seed of sflow for mirror group sampling.
 * Input:
 *      unit - unit id
 *      seed - sampling seed of sflow
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_mirror_sflowMirrorSeed_set(uint32 unit, uint32 seed)
{
    int32   ret;
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, seed=%x"
            , unit, seed);
    
    MIRROR_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_write(unit, ESW_SEED_FOR_TRAFFIC_MIRROR_TABLEr
                        , ESW_SEEDf, &seed)) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    MIRROR_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_sflowMirrorSeed_set */

/* Function Name:
 *      dal_esw_mirror_sflowMirrorSampleEnable_get
 * Description:
 *      Get enable status of sampling on specified mirror group.
 * Input:
 *      unit      - unit id
 *      mirror_id - mirror id
 * Output:
 *      pEnable   - pointer to enable status of sampling
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_MIRROR_ID        - invalid mirror id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_mirror_sflowMirrorSampleEnable_get(uint32 unit, uint32 mirror_id, rtk_enable_t *pEnable)
{
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, mirror_id=%d, pEnable=%x"
            , unit, mirror_id, pEnable);
    
    RT_PARAM_CHK(mirror_id > HAL_MIRROR_ID_MAX(unit), RT_ERR_MIRROR_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    MIRROR_SEM_LOCK(unit);
    *pEnable = pSflow_mirror_sample_conf[unit][mirror_id].status;
    MIRROR_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_sflowMirrorSampleEnable_get */

/* Function Name:
 *      dal_esw_mirror_sflowMirrorSampleEnable_set
 * Description:
 *      Set enable status of sampling on specified mirror group.
 * Input:
 *      unit      - unit id
 *      mirror_id - mirror id
 *      enable    - enable status of sampling
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_MIRROR_ID        - invalid mirror id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_mirror_sflowMirrorSampleEnable_set(uint32 unit, uint32 mirror_id, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, mirror_id=%d, enable=%u"
            , unit, mirror_id, enable);
    
    RT_PARAM_CHK(mirror_id > HAL_MIRROR_ID_MAX(unit), RT_ERR_MIRROR_ID);
    
    if ( enable == pSflow_mirror_sample_conf[unit][mirror_id].status)
    {
        return RT_ERR_OK;
    }
    
    switch (enable)
    {
        case DISABLED:
            value = 0;
            break;
        
        case ENABLED:
            value = pSflow_mirror_sample_conf[unit][mirror_id].rate;
            break;
        
        default:
            return RT_ERR_INPUT;
    }
    
    MIRROR_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_write(unit, sample_rate_for_traffic_mirror_table_entry_regidx[mirror_id]
                        , ESW_SAMPLE_RATEf, &value)) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    pSflow_mirror_sample_conf[unit][mirror_id].status = enable;
    
    MIRROR_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_sflowMirrorSampleEnable_set */

/* Function Name:
 *      dal_esw_mirror_sflowMirrorSampleRate_get
 * Description:
 *      Get sampling rate of specified mirror group.
 * Input:
 *      unit      - unit id
 *      mirror_id - mirror id
 * Output:
 *      pRate     - pointer to sampling rate
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_MIRROR_ID        - invalid mirror id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_mirror_sflowMirrorSampleRate_get(uint32 unit, uint32 mirror_id, uint32 *pRate)
{
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, mirror_id=%d, pRate=%x"
            , unit, mirror_id, pRate);
    
    RT_PARAM_CHK(mirror_id > HAL_MIRROR_ID_MAX(unit), RT_ERR_MIRROR_ID);
    RT_PARAM_CHK((NULL == pRate), RT_ERR_NULL_POINTER);
    
    MIRROR_SEM_LOCK(unit);
    *pRate = pSflow_mirror_sample_conf[unit][mirror_id].rate;
    MIRROR_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_sflowMirrorSampleRate_get */

/* Function Name:
 *      dal_esw_mirror_sflowMirrorSampleRate_set
 * Description:
 *      Set sampling rate of specified mirror group.
 * Input:
 *      unit      - unit id
 *      mirror_id - mirror id
 *      rate      - sampling rate
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_MIRROR_ID        - invalid mirror id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_mirror_sflowMirrorSampleRate_set(uint32 unit, uint32 mirror_id, uint32 rate)
{
    int32   ret;
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, mirror_id=%d, rate=%u"
            , unit, mirror_id, rate);
    
    RT_PARAM_CHK(mirror_id > HAL_MIRROR_ID_MAX(unit), RT_ERR_MIRROR_ID);
    RT_PARAM_CHK(rate > HAL_SFLOW_RATE_MAX(unit), RT_ERR_OUT_OF_RANGE);
    
    
    MIRROR_SEM_LOCK(unit);
    
    if (ENABLED == pSflow_mirror_sample_conf[unit][mirror_id].status)
    {
        /* get value from CHIP*/
        if ((ret = reg_field_write(unit, sample_rate_for_traffic_mirror_table_entry_regidx[mirror_id]
                            , ESW_SAMPLE_RATEf, &rate)) != RT_ERR_OK)
        {
            MIRROR_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
            return ret;
        }
    }
    
    pSflow_mirror_sample_conf[unit][mirror_id].rate = rate;
    
    MIRROR_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_sflowMirrorSampleRate_set */

/* Function Name:
 *      dal_esw_mirror_sflowMirrorSampleStat_get
 * Description:
 *      Set statistic of sampling on specified mirror group.
 * Input:
 *      unit      - unit id
 *      mirror_id - mirror id
 * Output:
 *      pStat     - pointer to statistic of sampling
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_MIRROR_ID        - invalid mirror id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_mirror_sflowMirrorSampleStat_get(uint32 unit, uint32 mirror_id, rtk_mirror_sampleStat_t *pStat)
{
    int32   ret;
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, mirror_id=%d, pStat=%x"
            , unit, mirror_id, pStat);
    
    RT_PARAM_CHK(mirror_id > HAL_MIRROR_ID_MAX(unit), RT_ERR_MIRROR_ID);
    RT_PARAM_CHK((NULL == pStat), RT_ERR_NULL_POINTER);
    
    MIRROR_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, counter_for_traffic_mirror_table_entry_regidx[mirror_id]
                        , ESW_TOTAL_SAMPLEf, &(pStat->totalSamples))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    if ((ret = reg_field_read(unit, total_counter_for_traffic_mirror_table_entry_regidx[mirror_id]
                        , ESW_TOTAL_COUNTERf, &(pStat->totalSamples))) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    MIRROR_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_sflowMirrorSampleStat_get */

/* Module Name    : Mirror           */
/* Sub-module Name: Port-based SFLOW */

/* Function Name:
 *      dal_esw_mirror_sflowPortSeed_get
 * Description:
 *      Get sampling seed of sflow for port sampling.
 * Input:
 *      unit  - unit id
 * Output:
 *      pSeed - pointer to sampling seed of sflow
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_mirror_sflowPortSeed_get(uint32 unit, uint32 *pSeed)
{
    int32   ret;
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, pSeed=%x"
            , unit, pSeed);
    
    MIRROR_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, ESW_SEED_FOR_PORT_BASED_SFLOWr
                        , ESW_SEEDf, pSeed)) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    MIRROR_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_sflowPortSeed_get */

/* Function Name:
 *      dal_esw_mirror_sflowPortSeed_set
 * Description:
 *      Set sampling seed of sflow for port sampling.
 * Input:
 *      unit - unit id
 *      seed - sampling seed of sflow
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_mirror_sflowPortSeed_set(uint32 unit, uint32 seed)
{
    int32   ret;
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, seed=%x"
            , unit, seed);
    
    MIRROR_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_write(unit, ESW_SEED_FOR_PORT_BASED_SFLOWr
                        , ESW_SEEDf, &seed)) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    MIRROR_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_sflowPortSeed_set */

/* Function Name:
 *      dal_esw_mirror_sflowPortIgrSampleEnable_get
 * Description:
 *      Get enable status of ingress sampling on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of ingress sampling
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
dal_esw_mirror_sflowPortIgrSampleEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, port=%d, pEnable=%x"
            , unit, port, pEnable);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    MIRROR_SEM_LOCK(unit);
    *pEnable = pSflow_port_sample_conf[unit]->igr[port].status;
    MIRROR_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_sflowPortIgrSampleEnable_get */

/* Function Name:
 *      dal_esw_mirror_sflowPortIgrSampleEnable_set
 * Description:
 *      Set enable status of ingress sampling on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of ingress sampling
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
dal_esw_mirror_sflowPortIgrSampleEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, port=%d, enable=%u"
            , unit, port, enable);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    if (enable == pSflow_port_sample_conf[unit]->igr[port].status)
    {
        return RT_ERR_OK;
    }
    
    switch (enable)
    {
        case DISABLED:
            value = 0;
            break;
        
        case ENABLED:
            value = pSflow_port_sample_conf[unit]->igr[port].rate;
            break;
        
        default:
            return RT_ERR_INPUT;
    }
    
    MIRROR_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_SFLOW_INGRESS_SAMPLE_RATE_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_INSAMPLE_RATEf, &value)) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    pSflow_port_sample_conf[unit]->igr[port].status = enable;
    
    MIRROR_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_sflowPortIgrSampleEnable_set */

/* Function Name:
 *      dal_esw_mirror_sflowPortIgrSampleRate_get
 * Description:
 *      Get sampling rate of ingress sampling on specified port.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pRate - pointer to sampling rate of ingress sampling
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
dal_esw_mirror_sflowPortIgrSampleRate_get(uint32 unit, rtk_port_t port, uint32 *pRate)
{
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, port=%d, pRate=%x"
            , unit, port, pRate);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pRate), RT_ERR_NULL_POINTER);
    
    MIRROR_SEM_LOCK(unit);
    *pRate = pSflow_port_sample_conf[unit]->igr[port].rate;
    MIRROR_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_sflowPortIgrSampleRate_get */

/* Function Name:
 *      dal_esw_mirror_sflowPortIgrSampleEnable_set
 * Description:
 *      Set sampling rate of ingress sampling on specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 *      rate - sampling rate of ingress sampling
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
dal_esw_mirror_sflowPortIgrSampleRate_set(uint32 unit, rtk_port_t port, uint32 rate)
{
    int32   ret;
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, port=%d, rate=%u"
            , unit, port, rate);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(rate > HAL_SFLOW_RATE_MAX(unit), RT_ERR_OUT_OF_RANGE);
    
    
    MIRROR_SEM_LOCK(unit);
    
    if (ENABLED == pSflow_port_sample_conf[unit]->igr[port].status)
    {
        /* get value from CHIP*/
        if ((ret = reg_array_field_write(unit, ESW_PORT_SFLOW_INGRESS_SAMPLE_RATE_CONTROLr
                            , port, REG_ARRAY_INDEX_NONE, ESW_INSAMPLE_RATEf, &rate)) != RT_ERR_OK)
        {
            MIRROR_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
            return ret;
        }
    }
    
    pSflow_port_sample_conf[unit]->igr[port].rate = rate;
    
    MIRROR_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_sflowPortIgrSampleRate_set */

/* Function Name:
 *      dal_esw_mirror_sflowPortEgrSampleEnable_get
 * Description:
 *      Get enable status of egress sampling on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of egress sampling
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
dal_esw_mirror_sflowPortEgrSampleEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, port=%d, pEnable=%x"
            , unit, port, pEnable);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    MIRROR_SEM_LOCK(unit);
    *pEnable = pSflow_port_sample_conf[unit]->egr[port].status;
    MIRROR_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_sflowPortEgrSampleEnable_get */

/* Function Name:
 *      dal_esw_mirror_sflowPortEgrSampleEnable_set
 * Description:
 *      Set enable status of egress sampling on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of egress sampling
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
dal_esw_mirror_sflowPortEgrSampleEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, port=%d, enable=%u"
            , unit, port, enable);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    if (enable == pSflow_port_sample_conf[unit]->egr[port].status)
    {
        return RT_ERR_OK;
    }
    
    switch (enable)
    {
        case DISABLED:
            value = 0;
            break;
        
        case ENABLED:
            value = pSflow_port_sample_conf[unit]->egr[port].rate;
            break;
        
        default:
            return RT_ERR_INPUT;
    }
    
    MIRROR_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_SFLOW_EGRESS_SAMPLE_RATE_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_EGSAMPLE_RATEf, &value)) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    pSflow_port_sample_conf[unit]->egr[port].status = enable;
    
    MIRROR_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_sflowPortEgrSampleEnable_set */

/* Function Name:
 *      dal_esw_mirror_sflowPortEgrSampleEnable_get
 * Description:
 *      Get sampling rate of egress sampling on specified port.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pRate - pointer to sampling rate of egress sampling
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
dal_esw_mirror_sflowPortEgrSampleRate_get(uint32 unit, rtk_port_t port, uint32 *pRate)
{
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, port=%d, pRate=%x"
            , unit, port, pRate);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pRate), RT_ERR_NULL_POINTER);
    
    MIRROR_SEM_LOCK(unit);
    *pRate = pSflow_port_sample_conf[unit]->egr[port].rate;
    MIRROR_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_sflowPortEgrSampleRate_get */

/* Function Name:
 *      dal_esw_mirror_sflowPortEgrSampleRate_set
 * Description:
 *      Set sampling rate of egress sampling on specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 *      rate - sampling rate of egress sampling
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
dal_esw_mirror_sflowPortEgrSampleRate_set(uint32 unit, rtk_port_t port, uint32 rate)
{
    int32   ret;
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, port=%d, rate=%u"
            , unit, port, rate);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(rate > HAL_SFLOW_RATE_MAX(unit), RT_ERR_OUT_OF_RANGE);
    
    MIRROR_SEM_LOCK(unit);
    
    if (ENABLED == pSflow_port_sample_conf[unit]->egr[port].status)
    {
        /* get value from CHIP*/
        if ((ret = reg_array_field_write(unit, ESW_PORT_SFLOW_EGRESS_SAMPLE_RATE_CONTROLr
                            , port, REG_ARRAY_INDEX_NONE, ESW_EGSAMPLE_RATEf, &rate)) != RT_ERR_OK)
        {
            MIRROR_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
            return ret;
        }
    }
    
    pSflow_port_sample_conf[unit]->egr[port].rate = rate;
    
    MIRROR_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_sflowPortEgrSampleRate_set */

/* Function Name:
 *      dal_esw_mirror_sflowAddCPUTagEnable_get
 * Description:
 *      Get enable status of CPU tag adding for trapped packet of SFLOW.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of CPU tag adding
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_mirror_sflowAddCPUTagEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, pEnable=%x"
            , unit, pEnable);
    
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    MIRROR_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, ESW_SEED_FOR_PORT_BASED_SFLOWr
                        , ESW_SMPCPUTAGf, &value)) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    MIRROR_SEM_UNLOCK(unit);
    
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
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_sflowAddCPUTagEnable_get */

/* Function Name:
 *      dal_esw_mirror_sflowAddCPUTagEnable_set
 * Description:
 *      Set enable status of CPU tag adding for trapped packet of SFLOW.
 * Input:
 *      unit   - unit id
 *      enable - enable status of CPU tag adding
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_mirror_sflowAddCPUTagEnable_set(uint32 unit, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(mirror_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_MIRROR), "unit=%d, enable=%u"
            , unit, enable);
    
    switch (enable)
    {
        case DISABLED:
            value = 0;
            break;
        
        case ENABLED:
            value = 1;
            break;
        
        default:
            return RT_ERR_INPUT;
    }
    
    MIRROR_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_write(unit, ESW_SEED_FOR_PORT_BASED_SFLOWr
                        , ESW_SMPCPUTAGf, &value)) != RT_ERR_OK)
    {
        MIRROR_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    
    MIRROR_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_mirror_sflowAddCPUTagEnable_set */

