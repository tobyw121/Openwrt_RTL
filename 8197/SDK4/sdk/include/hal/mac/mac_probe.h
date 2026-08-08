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
 * $Revision: 6573 $
 * $Date: 2009-10-26 18:36:02 +0800 (Mon, 26 Oct 2009) $
 *
 * Purpose : MAC probe and init service APIs in the SDK.
 *
 * Feature : MAC probe and init service APIs
 *
 */

#ifndef __HAL_MAC_PROBE_H__
#define __HAL_MAC_PROBE_H__

/*
 * Data Declaration
 */

/*
 * Function Declaration
 */

/* Function Name:
 *      mac_probe
 * Description:
 *      Probe the MAC chip in the specified chip.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      None
 */
extern int32
mac_probe(uint32 unit);

/* Function Name:
 *      mac_init
 * Description:
 *      Init the MAC chip in the specified chip.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      None
 */
extern int32
mac_init(uint32 unit);

extern int32
hal_portInfo_update(uint32 unit, int32 port, rt_port_type_t type);

#endif  /* __HAL_MAC_PROBE_H__ */
