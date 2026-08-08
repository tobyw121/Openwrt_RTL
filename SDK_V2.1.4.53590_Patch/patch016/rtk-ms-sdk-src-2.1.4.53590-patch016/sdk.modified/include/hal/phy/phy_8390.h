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
 * $Revision: 57334 $
 * $Date: 2015-03-30 14:13:45 +0800 (Mon, 30 Mar 2015) $
 *
 * Purpose : MAC 8390 internal PHY Driver APIs.
 *
 * Feature : MAC 8390 internal PHY Driver APIs
 *
 */

#ifndef __HAL_PHY_PHY_8390_H__
#define __HAL_PHY_PHY_8390_H__

/*
 * Include Files
 */
#include <common/rt_type.h>

/*
 * Symbol Definition
 */
#define PHY_8390_10G_85_OHM

/*
 * Data Declaration
 */
extern rt_phydrv_t phy_8390_serdes_ge;

/* Function Name:
 *      phy_8390_init
 * Description:
 *      Initialize 8390 MAC internal serdes PHY.
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
phy_8390_init(uint32 unit, rtk_port_t port);

/* Function Name:
 *      phy_8390_media_get
 * Description:
 *      Get 8390 serdes PHY media type.
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
phy_8390_media_get(uint32 unit, rtk_port_t port, rtk_port_media_t *pMedia);

/* Function Name:
 *      phy_8390_gigaLiteEnable_get
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
 *      1. The RTL8390 is not supported the per-port Giga Lite feature.
 */
extern int32
phy_8390_gigaLiteEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      phy_8390_speed_get
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
phy_8390_speed_get(uint32 unit, rtk_port_t port, uint32 *pSpeed);

/* Function Name:
 *      phy_8390_speed_set
 * Description:
 *      Set speed mode status of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      speed         - link speed status 10/100/1000
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                 - OK
 *      RT_ERR_FAILED             - invalid parameter
 *      RT_ERR_CHIP_NOT_SUPPORTED - copper media chip is not supported Force-1000
 * Note:
 *      None
 */
extern int32
phy_8390_speed_set(uint32 unit, rtk_port_t port, uint32 speed);

/* Function Name:
 *      phy_8390_duplex_get
 * Description:
 *      Get duplex mode status of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pDuplex - pointer to PHY duplex mode status
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
extern int32
phy_8390_duplex_get(uint32 unit, rtk_port_t port, uint32 *pDuplex);

/* Function Name:
 *      phy_8390_duplex_set
 * Description:
 *      Set duplex mode status of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      duplex        - duplex mode of the port, full or half
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
extern int32
phy_8390_duplex_set(uint32 unit, rtk_port_t port, uint32 duplex);

/* Function Name:
 *      phy_8390_enable_set
 * Description:
 *      Set interface status of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      enable        - admin configuration of PHY interface
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
extern int32
phy_8390_enable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      phy_8390_autoNegoEnable_get
 * Description:
 *      Get autonegotiation enable status of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pEnable -   auto negotiation status
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
extern int32
phy_8390_autoNegoEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      phy_8390_autoNegoEnable_set
 * Description:
 *      Set autonegotiation enable status of the specific port
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
phy_8390_autoNegoEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      phy_8390_autoNegoAbility_get
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
phy_8390_autoNegoAbility_get(uint32 unit, rtk_port_t port, rtk_port_phy_ability_t *pAbility);

/* Function Name:
 *      phy_8390_autoNegoAbility_set
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
phy_8390_autoNegoAbility_set(uint32 unit, rtk_port_t port, rtk_port_phy_ability_t *pAbility);

#if defined(CONFIG_SDK_WA_SERDES_FIBER_LINKDOWN_WATCHDOG)
/* Function Name:
 *      phy_8390_linkdown_watchdog
 * Description:
 *      For direct fiber port link down, update MAC link status
 * Input:
 *      unit          - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
void phy_8390_linkdown_watchdog(uint32 unit);
#endif

void phy_8390_serdes_10g_leq_init (uint32 unit, rtk_port_t port);
int32 phy_8390_10gSds_restart(uint32 unit, rtk_port_t port);

int32
phy_8390_10gMedia_get(uint32 unit, rtk_port_t port, rtk_port_10gMedia_t *media);

int32
phy_8390_10gMedia_set(uint32 unit, rtk_port_t port, rtk_port_10gMedia_t media);

#endif /* __HAL_PHY_PHY_8390_H__ */
