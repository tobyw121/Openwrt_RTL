/*
 * Copyright (C) 2011 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 27486 $
 * $Date: 2012-03-26 08:53:40 +0800 (Mon, 26 Mar 2012) $
 *
 * Purpose : Definition those public MPLS routing APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *              1) MPLS
 */

/*
 * Include Files
 */
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <dal/dal_mgmt.h>
#include <rtk/default.h>
#include <rtk/mpls.h>

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */

/*
 * Macro Declaration
 */

/*
 * Function Declaration
 */

/* Module Name : MPLS */

/* Function Name:
 *      rtk_mpls_init
 * Description:
 *      Initialize mpls module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
rtk_mpls_init(uint32 unit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mpls_init(unit);
} /* end of rtk_mpls_init */

/* Function Name:
 *      rtk_mpls_ttlInherit_get
 * Description:
 *      Get MPLS TTL inherit properties
 * Input:
 *      unit     - unit id
 * Output:
 *      pInherit - pointer buffer of MPLS TTL inherit information
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      (1) inherit = uniform, the TTL of MPLS header inherits from IP TTL and decrement 1.
 *      (2) inherit = pipe, the TTL of MPLS header is from rtk_mpls_encap_set.
 */
int32
rtk_mpls_ttlInherit_get(uint32 unit, rtk_mpls_ttlInherit_t *pInherit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mpls_ttlInherit_get(unit, pInherit);
} /* end of rtk_mpls_ttlInherit_get */

/* Function Name:
 *      rtk_mpls_ttlInherit_set
 * Description:
 *      Set MPLS TTL inherit properties
 * Input:
 *      unit    - Device number
 *      inherit - MPLS TTL inherit information
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8390
 * Note:
 *      (1) inherit = uniform, the TTL of MPLS header inherits from IP TTL and decrement 1.
 *      (2) inherit = pipe, the TTL of MPLS header is from rtk_mpls_encap_set.
 */
int32
rtk_mpls_ttlInherit_set(uint32 unit, rtk_mpls_ttlInherit_t inherit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mpls_ttlInherit_set(unit, inherit);
} /* end of rtk_mpls_ttlInherit_set */

/* Function Name:
 *      rtk_mpls_encap_get
 * Description:
 *      Get MPLS encapsulation properties
 * Input:
 *      unit    - Device number
 *      lib_idx - the index of MPLS table
 * Output:
 *      pInfo    - pointer buffer of MPLS encapsulation information
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      (1) For single label operation, lib_idx ranges from 0 to 511 and only info->label0 is used.
 *          For double label operation, lib_idx ranges from 0 to 255 and both info->label0 and info->label1 are used.
 *      (2) The TLL of MPLS header inheritance is controlled by 'rtk_mpls_ttlInherit_set'.
 */
int32
rtk_mpls_encap_get(uint32 unit, uint32 lib_idx, rtk_mpls_encap_t *pInfo)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mpls_encap_get(unit, lib_idx, pInfo);
} /* end of rtk_mpls_encap_get */

/* Function Name:
 *      rtk_mpls_encap_set
 * Description:
 *      Set MPLS encapsulation properties
 * Input:
 *      unit    - Device number
 *      lib_idx - the index of MPLS table
 *      pInfo   - MPLS encapsulation information
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      (1) For single label operation, lib_idx ranges from 0 to 511 and only info->label0 is used.
 *          For double label operation, lib_idx ranges from 0 to 255 and both info->label0 and info->label1 are used.
 *      (2) The TLL of MPLS header inheritance is controlled by 'rtk_mpls_ttlInherit_set'.
 */
int32
rtk_mpls_encap_set(uint32 unit, uint32 lib_idx, rtk_mpls_encap_t *pInfo)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mpls_encap_set(unit, lib_idx, pInfo);
} /* end of rtk_mpls_encap_set */

/* Function Name:
 *      rtk_mpls_enable_get
 * Description:
 *      Get MPLS state
 * Input:
 *      unit    - Device number
 * Output:
 *      pEnable - MPLS state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      MPLS packet is treated as L2 unknown packet if MPLS function is disabled.
 */
int32
rtk_mpls_enable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mpls_enable_get(unit, pEnable);
} /* end of rtk_mpls_enable_get */

/* Function Name:
 *      rtk_mpls_enable_set
 * Description:
 *      Set MPLS state
 * Input:
 *      unit    - Device number
 *      enable  - state of MPLS
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8390
 * Note:
 *      MPLS packet is treated as L2 unknown packet if MPLS function is disabled.
 */
int32
rtk_mpls_enable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mpls_enable_set(unit, enable);
} /* end of rtk_mpls_enable_set */
