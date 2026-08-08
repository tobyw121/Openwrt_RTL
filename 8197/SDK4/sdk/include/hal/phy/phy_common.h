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
 * $Revision: 49167 $
 * $Date: 2014-07-11 12:00:33 +0800 (Fri, 11 Jul 2014) $
 *
 * Purpose : PHY Common Driver APIs.
 *
 * Feature : PHY Common Driver APIs
 *
 */

#ifndef __HAL_PHY_PHY_COMMON_H__
#define __HAL_PHY_PHY_COMMON_H__

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/port.h>


/* Function Name:
 *      phy_common_unavail
 * Description:
 *      Return chip not support
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_CHIP_NOT_SUPPORTED   - functions not supported by this chip model
 * Note:
 *      None
 */
extern int32
phy_common_unavail(void);

/* Function Name:
 *      phy_common_AutoNegoEnable_get
 * Description:
 *      Get autonegotiation enable status of the specific port
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
phy_common_autoNegoEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);


/* Function Name:
 *      phy_common_autoNegoEnable_set
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
phy_common_autoNegoEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      phy_common_duplex_get
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
phy_common_duplex_get(uint32 unit, rtk_port_t port, uint32 *pDuplex);

/* Function Name:
 *      phy_common_duplex_get
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
phy_common_duplex_set(uint32 unit, rtk_port_t port, uint32 duplex);

/* Function Name:
 *      phy_common_enable_set
 * Description:
 *      Set PHY interface status of the specific port
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
phy_common_enable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      phy_common_speed_get
 * Description:
 *      Get PHY negotiated speed of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      pSpeed        - PHY negotiated speed
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
extern int32
phy_common_speed_get(uint32 unit, rtk_port_t port, uint32 *pSpeed);


/* Function Name:
 *      phy_common_masterSlave_get
 * Description:
 *      Get PHY configuration of master/slave mode of the specific port
 * Input:
 *      unit                - unit id
 *      port                - port id
 * Output:
 *      pMasterSlaveCfg     - pointer to the PHY master slave configuration
 *      pMasterSlaveActual  - pointer to the PHY master slave actual link status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      This function only works on giga/ 10g port to get its master/slave mode configuration.
 */
extern int32
phy_common_masterSlave_get(uint32 unit, rtk_port_t port, rtk_port_masterSlave_t *pMasterSlaveCfg, rtk_port_masterSlave_t *pMasterSlaveActual);

/* Function Name:
 *      phy_common_masterSlave_set
 * Description:
 *      Set PHY configuration of master/slave mode of the specific port
 * Input:
 *      unit                - unit id
 *      port                - port id
 *      masterSlave         - PHY master slave configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_INPUT         - invalid input parameter
 * Note:
 *      None
 */
extern int32
phy_common_masterSlave_set(uint32 unit, rtk_port_t port, rtk_port_masterSlave_t masterSlave);

#endif /* __HAL_PHY_PHY_COMMON_H__ */
