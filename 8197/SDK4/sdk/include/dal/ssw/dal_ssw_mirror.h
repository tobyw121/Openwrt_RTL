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
 * $Revision: 6401 $
 * $Date: 2009-10-14 16:03:12 +0800 (Wed, 14 Oct 2009) $
 *
 * Purpose : Definition those public MIRROR APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *              1) port mirroring
 *
 */

#ifndef __DAL_SSW_MIRROR_H__
#define __DAL_SSW_MIRROR_H__


/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/mirror.h>


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

/* Module Name : Mirror */

/* Function Name:
 *      dal_ssw_mirror_init
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
extern int32
dal_ssw_mirror_init(uint32 unit);

/* Function Name:
 *      dal_ssw_mirror_portBased_create
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
 *      RT_ERR_UNIT_ID             - invalid unit id
 *      RT_ERR_PORT_ID             - invalid port id
 *      RT_ERR_MIRROR_PORT_FULL    - Exceeds maximum number of supported mirroring port
 *      RT_ERR_MIRROR_PORT_EXIST   - mirroring port already exists
 * Note:
 *      1. The maximum number of mirroring session support is
 *         RTK_MAX_NUM_OF_MIRRORING_SESSIONS-1.
 */
extern int32
dal_ssw_mirror_portBased_create(uint32 unit, rtk_port_t mirroring_port);


/* Function Name:
 *      dal_ssw_mirror_portBased_destroy
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
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_PORT_ID                - invalid port id
 *      RT_ERR_MIRROR_PORT_NOT_EXIST  - mirroring port does not exists
 * Note:
 *      1. The maximum number of mirroring session support is
 *         RTK_MAX_NUM_OF_MIRRORING_SESSIONS-1.
 *      2. In RTL8329/RTL8389, if you have sess-0 (mirroring_port A with TX) and
 *         sess-1 (mirroring_port A with RX), remove mirroring_port A will
 *         remove those 2 sessions at the same time.
 */
extern int32
dal_ssw_mirror_portBased_destroy(uint32 unit, rtk_port_t mirroring_port);


/* Function Name:
 *      dal_ssw_mirror_portBased_destroyAll
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
 *      The maximum number of mirroring session support is
 *      RTK_MAX_NUM_OF_MIRRORING_SESSIONS-1.
 */
extern int32
dal_ssw_mirror_portBased_destroyAll(uint32 unit);


/* Function Name:
 *      dal_ssw_mirror_portBased_get
 * Description:
 *      Get the mirroring session information by mirroring port from
 *      the specified device.
 * Input:
 *      unit                   - unit id
 *      mirroring_port         - mirroring port
 * Output:
 *      *pMirrored_rx_portmask - pointer buffer of rx of mirrored ports
 *      *pMirrored_tx_portmask - pointer buffer of tx of mirrored ports
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_PORT_ID                - invalid port id
 *      RT_ERR_NULL_POINTER           - input parameter may be null pointer
 *      RT_ERR_MIRROR_SESSION_NOEXIST - mirroring session not exist
 *      RT_ERR_MIRROR_PORT_NOT_EXIST  - mirroring port does not exists
 * Note:
 *      1. The maximum number of mirroring session support is
 *         RTK_MAX_NUM_OF_MIRRORING_SESSIONS-1.
 *      2. In RTL8329/RTL8389, if you have sess-0 (mirroring_port A with TX) and
 *         sess-1 (mirroring_port A with RX), get mirroring_port A will
 *         reply rx_portmask and tx_portmask at the same time.
 */
extern int32
dal_ssw_mirror_portBased_get(
    uint32          unit,
    rtk_port_t      mirroring_port,
    rtk_portmask_t  *pMirrored_rx_portmask,
    rtk_portmask_t  *pMirrored_tx_portmask);


/* Function Name:
 *      dal_ssw_mirror_portBased_set
 * Description:
 *      Set the mirroring session information by mirroring port to
 *      the specified device.
 * Input:
 *      unit                   - unit id
 *      mirroring_port         - mirroring port
 *      *pMirrored_rx_portmask - rx of mirrored ports
 *      *pMirrored_tx_portmask - tx of mirrored ports
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
 *      1. The maximum number of mirroring session support is
 *         RTK_MAX_NUM_OF_MIRRORING_SESSIONS-1.
 */
extern int32
dal_ssw_mirror_portBased_set(
    uint32          unit,
    rtk_port_t      mirroring_port,
    rtk_portmask_t  *pMirrored_rx_portmask,
    rtk_portmask_t  *pMirrored_tx_portmask);


#endif /* __DAL_SSW_MIRROR_H__ */

