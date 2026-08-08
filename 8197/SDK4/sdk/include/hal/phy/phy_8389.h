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
 * $Revision: 34988 $
 * $Date: 2012-12-05 13:58:28 +0800 (Wed, 05 Dec 2012) $
 *
 * Purpose : MAC 8389 internal PHY Driver APIs.
 *
 * Feature : MAC 8389 internal PHY Driver APIs
 *
 */

#ifndef __HAL_PHY_PHY_8389_H__
#define __HAL_PHY_PHY_8389_H__

/*
 * Include Files
 */
#include <common/rt_type.h>

/*
 * Data Declaration
 */
extern rt_phydrv_t phy_8389_serdes_ge;


/* Function Name:
 *      phy_8389_init
 * Description:
 *      Initialize 8389 MAC internal serdes PHY.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
extern int32
phy_8389_init(uint32 unit, rtk_port_t port);

/* Function Name:
 *      phy_8389_media_get
 * Description:
 *      Get 8389 serdes PHY media type.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pMedia - pointer buffer of phy media type
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - invalid parameter
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *      1. media type is PORT_MEDIA_FIBER
 */
extern int32
phy_8389_media_get(uint32 unit, rtk_port_t port, rtk_port_media_t *pMedia);

/* Function Name:
 *      phy_8389_gigaLiteEnable_get
 * Description:
 *      Get the status of Giga Lite of the specific port in the specific unit
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to status of Giga Lite
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      1. The RTL8389 is not supported the per-port Giga Lite feature.
 */
extern int32
phy_8389_gigaLiteEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

#endif /* __HAL_PHY_PHY_8389_H__ */
