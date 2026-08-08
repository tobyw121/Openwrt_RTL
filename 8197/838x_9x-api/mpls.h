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
 * $Revision: 27279 $
 * $Date: 2012-03-26 08:53:40 +0800 (Mon, 26 Mar 2012) $
 *
 * Purpose : Definition those public MPLS routing APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *              1) MPLS
 */

#ifndef __RTK_MPLS_H__
#define __RTK_MPLS_H__

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <common/rt_error.h>

/*
 * Symbol Definition
 */
/* MPLS related flags */
#define RTK_MPLS_EXP_MAX    7
#define RTK_MPLS_TTL_MAX    255

/*
 * MPLS label operation
 */
typedef enum rtk_mpls_label_oper_e
{
    RTK_MPLS_LABEL_OPER_SINGLE,
    RTK_MPLS_LABEL_OPER_DOUBEL,
    RTK_MPLS_LABEL_OPER_MAX,
} rtk_mpls_label_oper_t;

/*
 * MPLS TTL inherit type
 */
typedef enum rtk_mpls_ttlInherit_e {
    RTK_MPLS_TTL_INHERIT_UNIFORM,
    RTK_MPLS_TTL_INHERIT_PIPE,
    RTK_MPLS_TTL_INHERIT_MAX,
} rtk_mpls_ttlInherit_t;

/*
 * MPLS encapsulation information structure.
 * Include inherit, inner/outer label
 */
typedef struct rtk_mpls_encap_s {
    uint32                  label0;     /* VC label */
    uint32                  label1;     /* Tunnel label */
    int                     exp0;       /* VC label experimental */
    int                     exp1;       /* Tunnel label experimental */
    int                     ttl0;       /* VC label TTL */
    int                     ttl1;       /* Tunnel label TTL */
    rtk_mpls_label_oper_t   oper;       /* push operation */
} rtk_mpls_encap_t;

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
extern int32
rtk_mpls_init(uint32 unit);

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
extern int32
rtk_mpls_ttlInherit_get(uint32 unit, rtk_mpls_ttlInherit_t *pInherit);

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
extern int32
rtk_mpls_ttlInherit_set(uint32 unit, rtk_mpls_ttlInherit_t inherit);

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
extern int32
rtk_mpls_encap_get(uint32 unit, uint32 lib_idx, rtk_mpls_encap_t *pInfo);

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
extern int32
rtk_mpls_encap_set(uint32 unit, uint32 lib_idx, rtk_mpls_encap_t *pInfo);

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
extern int32
rtk_mpls_enable_get(uint32 unit, rtk_enable_t *pEnable);

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
extern int32
rtk_mpls_enable_set(uint32 unit, rtk_enable_t enable);

#endif /* __RTK_MPLS_H__ */
