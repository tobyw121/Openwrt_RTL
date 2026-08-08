/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * $Revision: 10405 $
 * $Date: 2010-06-23 19:15:05 +0800 (Wed, 23 Jun 2010) $
 *
 * Purpose : Definition those public statistic APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) statistic counter reset
 *           2) statistic counter get
 * 
 */ 


#ifndef __DAL_ESW_STAT_H__
#define __DAL_ESW_STAT_H__

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/stat.h>

/*
 * Symbol Definition
 */


/*
 * Macro Declaration
 */


/*
 * Function Declaration
 */

/* Module Name : STAT */

/* Function Name:
 *      dal_esw_stat_init
 * Description:
 *      Initialize stat module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - invalid unit id
 *      RT_ERR_STAT_GLOBAL_CNTR_FAIL - Could not retrieve/reset Global Counter
 *      RT_ERR_STAT_PORT_CNTR_FAIL   - Could not retrieve/reset Port Counter
 * Note:
 *      Must initialize stat module before calling any stat APIs.
 */
extern int32
dal_esw_stat_init(uint32 unit);


/* Function Name:
 *      dal_esw_stat_global_reset
 * Description:
 *      Reset the global counters in the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - invalid unit id
 *      RT_ERR_STAT_GLOBAL_CNTR_FAIL - Could not retrieve/reset Global Counter
 * Note:
 *      None
 */
extern int32
dal_esw_stat_global_reset(uint32 unit);


/* Function Name:
 *      dal_esw_stat_port_reset
 * Description:
 *      Reset the specified port counters in the specified device.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID             - invalid unit id
 *      RT_ERR_PORT_ID             - invalid port id
 *      RT_ERR_STAT_PORT_CNTR_FAIL - Could not retrieve/reset Port Counter
 * Note:
 *      None
 */
extern int32
dal_esw_stat_port_reset(uint32 unit, rtk_port_t port);


/* Function Name:
 *      dal_esw_stat_global_get
 * Description:
 *      Get one specified global counter in the specified device.
 * Input:
 *      unit     - unit id
 *      cntr_idx - specified global counter index
 * Output:
 *      pCntr    - pointer buffer of counter value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                  - invalid unit id
 *      RT_ERR_NULL_POINTER             - input parameter may be null pointer
 *      RT_ERR_STAT_GLOBAL_CNTR_FAIL    - Could not retrieve/reset Global Counter
 *      RT_ERR_STAT_INVALID_GLOBAL_CNTR - Invalid Global Counter
 * Note:
 *      None
 */
extern int32
dal_esw_stat_global_get(uint32 unit, rtk_stat_global_type_t cntr_idx, uint64 *pCntr);


/* Function Name:
 *      dal_esw_stat_global_getAll
 * Description:
 *      Get all global counters in the specified device.
 * Input:
 *      unit          - unit id
 * Output:
 *      pGlobal_cntrs - pointer buffer of global counter structure
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                  - invalid unit id
 *      RT_ERR_NULL_POINTER             - input parameter may be null pointer
 *      RT_ERR_STAT_GLOBAL_CNTR_FAIL    - Could not retrieve/reset Global Counter
 *      RT_ERR_STAT_INVALID_GLOBAL_CNTR - Invalid Global Counter
 * Note:
 *      None
 */
extern int32
dal_esw_stat_global_getAll(uint32 unit, rtk_stat_global_cntr_t *pGlobal_cntrs);


/* Function Name:
 *      dal_esw_stat_port_get
 * Description:
 *      Get one specified port counter in the specified device.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      cntr_idx - specified port counter index
 * Output:
 *      pCntr    - pointer buffer of counter value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID             - invalid unit id
 *      RT_ERR_PORT_ID             - invalid port id
 *      RT_ERR_NULL_POINTER        - input parameter may be null pointer
 *      RT_ERR_STAT_PORT_CNTR_FAIL - Could not retrieve/reset Port Counter
 * Note:
 *      None
 */
extern int32
dal_esw_stat_port_get(uint32 unit, rtk_port_t port, rtk_stat_port_type_t cntr_idx, uint64 *pCntr);


/* Function Name:
 *      dal_esw_stat_port_getAll
 * Description:
 *      Get all counters of one specified port in the specified device.
 * Input:
 *      unit        - unit id
 *      port        - port id
 * Output:
 *      pPort_cntrs - pointer buffer of counter value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID             - invalid unit id
 *      RT_ERR_PORT_ID             - invalid port id
 *      RT_ERR_NULL_POINTER        - input parameter may be null pointer
 *      RT_ERR_STAT_PORT_CNTR_FAIL - Could not retrieve/reset Port Counter
 * Note:
 *      None
 */
extern int32
dal_esw_stat_port_getAll(uint32 unit, rtk_port_t port, rtk_stat_port_cntr_t *pPort_cntrs);

/* Function Name:
 *      dal_esw_stat_smon_get
 * Description:
 *      Get one specified SMON counter in specified device.
 * Input:
 *      unit            - unit id
 *      cntr_idx        - index of specified SMON counter 
 *      pri             - priority
 * Output:              
 *      pCntr           - pointer buffer of counte value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                  - invalid unit id
 *      RT_ERR_NULL_POINTER             - inputt parameter may be null pointer
 *      RT_ERR_STAT_SMON_CNTR_FAIL      - Could not retrieve/reset SMON Counter
 *      RT_ERR_STAT_INVALID_SMON_CNTR   - Could not retrieve/reset SMON Counter
 * Note:
 *      None
 */
extern int32
dal_esw_stat_smon_get(uint32 unit, rtk_pri_t pri, rtk_stat_smon_type_t cntr_idx,  uint64 *pCntr);

/* Function Name:
 *      dal_esw_stat_smon_getAll
 * Description:
 *      Get all specified SMON counter in specified device.
 * Input:
 *      unit            - unit id
 *      cntr_idx        - index of specified SMON counter 
 *      pri             - priority
 * Output:              
 *      pCntr           - pointer buffer of counte value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                  - invalid unit id
 *      RT_ERR_NULL_POINTER             - inputt parameter may be null pointer
 *      RT_ERR_STAT_SMON_CNTR_FAIL      - Could not retrieve/reset SMON Counter
 * Note:
 *      None
 */
extern int32
dal_esw_stat_smon_getAll(uint32 unit, rtk_pri_t pri, rtk_stat_smon_cntr_t *pCntr);


#endif /*__DAL_ESW_STAT_H__*/

