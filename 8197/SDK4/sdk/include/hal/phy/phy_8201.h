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
 * Purpose : PHY 8201 Driver APIs.
 *
 * Feature : PHY 8201 Driver APIs
 *
 */

#ifndef __HAL_PHY_PHY_8201_H__
#define __HAL_PHY_PHY_8201_H__

/*
 * Include Files
 */
#include <common/rt_type.h>

/*
 * Data Declaration
 */
extern rt_phydrv_t phy_8201drv_fe;

/*
 * Function Declaration
 */

/* Function Name:
 *      phy_8201_autoNegoAbility_get
 * Description:
 *      Get ability advertisement for auto negotiation of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pAbility - pointer to PHY auto negotiation ability
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
extern int32
phy_8201_autoNegoAbility_get(uint32 unit, rtk_port_t port, rtk_port_phy_ability_t *pAbility);

/* Function Name:
 *      phy_8201_autoNegoAbility_set
 * Description:
 *      Set ability advertisement for auto negotiation of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 *      pAbility  - auto negotiation ability that is going to set to PHY
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
extern int32
phy_8201_autoNegoAbility_set(uint32 unit, rtk_port_t port, rtk_port_phy_ability_t *pAbility);

/* Function Name:
 *      phy_8201_speed_get
 * Description:
 *      Get link speed status of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pSpeed - pointer to PHY link speed
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
extern int32
phy_8201_speed_get(uint32 unit, rtk_port_t port, uint32 *pSpeed);

/* Function Name:
 *      phy_8201_speed_set
 * Description:
 *      Set speed mode status of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      speed         - link speed status 10/100/1000
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
extern int32
phy_8201_speed_set(uint32 unit, rtk_port_t port, uint32 speed);

/* Function Name:
 *      phy_8201_media_get
 * Description:
 *      Get 8201 serdes PHY media type.
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
 *      1. media type is PORT_MEDIA_COPPER
 */
extern int32
phy_8201_media_get(uint32 unit, rtk_port_t port, rtk_port_media_t *pMedia);

/* Function Name:
 *      phy_8201_gigaLiteEnable_get
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
 *      1. The RTL8201 is not supported the per-port Giga Lite feature.
 */
extern int32
phy_8201_gigaLiteEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

#endif /* __HAL_PHY_PHY_8201_H__ */
