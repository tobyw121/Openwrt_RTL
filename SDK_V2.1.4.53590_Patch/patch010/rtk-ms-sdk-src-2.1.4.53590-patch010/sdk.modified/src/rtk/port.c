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
 * Purpose : Definition of Port API
 *
 * Feature : The file have include the following module and sub-modules
 *           (1) Parameter settings for the port-based view
 *           (2) UDLD
 *           (3) RLDP
 */

/*
 * Include Files
 */
#include <common/rt_autoconf.h>
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#if defined(CONFIG_SDK_AUTO_COMBO_MEDIA_BY_GPIO)
#include <hal/common/halctrl.h>
#include <dal/dal_losMon.h>
#endif
#include <dal/dal_mgmt.h>
#include <dal/dal_linkMon.h>
#include <rtk/default.h>
#include <rtk/port.h>
#include <dal/dal_waMon.h>

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */

/*
 * Function Declaration
 */

/* Function Name:
 *      rtk_port_init
 * Description:
 *      Initialize port module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      Must initialize port module before calling any port APIs.
 */
int32
rtk_port_init(uint32 unit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_init(unit);
} /* end of rtk_port_init */

/* Module Name    : Port                                       */
/* Sub-module Name: Parameter settings for the port-based view */

/* Function Name:
 *      rtk_port_link_get
 * Description:
 *      Get the link status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pStatus - pointer to the link status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The link status of the port is as following:
 *      - PORT_LINKDOWN
 *      - PORT_LINKUP
 */
int32
rtk_port_link_get(uint32 unit, rtk_port_t port, rtk_port_linkStatus_t *pStatus)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->port_link_get(unit, phy_port, pStatus);
    }
#else
    return RT_MAPPER(unit)->port_link_get(unit, port, pStatus);
#endif
} /* end of rtk_port_link_get */

/* Function Name:
 *      rtk_port_linkMedia_get
 * Description:
 *      Get the link status with media information of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pStatus - pointer to the link status
 *      pMedia  - pointer to the media type
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The link status of the port is as following:
 *          - PORT_LINKDOWN
 *          - PORT_LINKUP
 *      (2) The media type of the port is as following:
 *          - PORT_MEDIA_COPPER
 *          - PORT_MEDIA_FIBER
 *      (3) When the link status is link-down, the return media should be ignored.
 */
int32
rtk_port_linkMedia_get(uint32 unit, rtk_port_t port, rtk_port_linkStatus_t *pStatus, rtk_port_media_t *pMedia)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_linkMedia_get(unit, port, pStatus, pMedia);
} /* end of rtk_port_linkMedia_get */

/* Function Name:
 *      rtk_port_speedDuplex_get
 * Description:
 *      Get the negotiated port speed and duplex status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pSpeed  - pointer to the port speed
 *      pDuplex - pointer to the port duplex
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 *      RT_ERR_PORT_LINKDOWN - link down port status
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The speed type of the port is as following:
 *          - PORT_SPEED_10M
 *          - PORT_SPEED_100M
 *          - PORT_SPEED_1000M
 *          - PORT_SPEED_2G  (Applicable to 8380)
 *          - PORT_SPEED_10G (Applicable to 8390)
 *      (2) The duplex mode of the port is as following:
 *          - PORT_HALF_DUPLEX
 *          - PORT_FULL_DUPLEX
 */
int32
rtk_port_speedDuplex_get(
    uint32            unit,
    rtk_port_t        port,
    rtk_port_speed_t  *pSpeed,
    rtk_port_duplex_t *pDuplex)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->port_speedDuplex_get(unit, phy_port, pSpeed, pDuplex);
    }
#else
    return RT_MAPPER(unit)->port_speedDuplex_get(unit, port, pSpeed, pDuplex);
#endif
} /* end of rtk_port_speedDuplex_get */

/* Function Name:
 *      rtk_port_flowctrl_get
 * Description:
 *      Get the negotiated flow control status of the specific port
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pTxStatus - pointer to the negotiation result of the Tx flow control
 *      pRxStatus - pointer to the negotiation result of the Rx flow control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 *      RT_ERR_PORT_LINKDOWN - link down port status
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_port_flowctrl_get(
    uint32            unit,
    rtk_port_t        port,
    uint32            *pTxStatus,
    uint32            *pRxStatus)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->port_flowctrl_get(unit, phy_port, pTxStatus, pRxStatus);
    }
#else
    return RT_MAPPER(unit)->port_flowctrl_get(unit, port, pTxStatus, pRxStatus);
#endif
} /* end of rtk_port_flowctrl_get */

/* Function Name:
 *      rtk_port_phyAutoNegoEnable_get
 * Description:
 *      Get PHY ability of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to PHY auto negotiation status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_port_phyAutoNegoEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->port_phyAutoNegoEnable_get(unit, phy_port, pEnable);
    }
#else
    return RT_MAPPER(unit)->port_phyAutoNegoEnable_get(unit, port, pEnable);
#endif
} /* end of rtk_port_phyAutoNegoEnable_get */

/* Function Name:
 *      rtk_port_phyAutoNegoEnable_set
 * Description:
 *      Set PHY ability of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable PHY auto negotiation
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - input parameter out of range
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      - ENABLED : switch to PHY auto negotiation mode
 *      - DISABLED: switch to PHY force mode
 *      - Once the abilities of both auto-nego and force mode are set,
 *        you can freely switch the mode without calling ability setting API again
 */
int32
rtk_port_phyAutoNegoEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->port_phyAutoNegoEnable_set(unit, phy_port, enable);
    }
#else
    return RT_MAPPER(unit)->port_phyAutoNegoEnable_set(unit, port, enable);
#endif
} /* end of rtk_port_phyAutoNegoEnable_set */

/* Function Name:
 *      rtk_port_phyAutoNegoAbility_get
 * Description:
 *      Get PHY auto negotiation ability of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pAbility - pointer to the PHY ability
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_port_phyAutoNegoAbility_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_port_phy_ability_t  *pAbility)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->port_phyAutoNegoAbility_get(unit, phy_port, pAbility);
    }
#else
    return RT_MAPPER(unit)->port_phyAutoNegoAbility_get(unit, port, pAbility);
#endif
} /* end of rtk_port_phyAutoNegoAbility_get */

/* Function Name:
 *      rtk_port_phyAutoNegoAbility_set
 * Description:
 *      Set PHY auto negotiation ability of the specific port
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      pAbility - pointer to the PHY ability
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      You can set these abilities no matter which mode PHY currently stays on
 */
int32
rtk_port_phyAutoNegoAbility_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_port_phy_ability_t  *pAbility)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->port_phyAutoNegoAbility_set(unit, phy_port, pAbility);
    }
#else
    return RT_MAPPER(unit)->port_phyAutoNegoAbility_set(unit, port, pAbility);
#endif
} /* end of rtk_port_phyAutoNegoAbility_set */

/* Function Name:
 *      rtk_port_phyForceModeAbility_get
 * Description:
 *      Get PHY ability status of the specific port
 * Input:
 *      unit         - unit id
 *      port         - port id
 * Output:
 *      pSpeed       - pointer to the port speed
 *      pDuplex      - pointer to the port duplex
 *      pFlowControl - pointer to the flow control enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_port_phyForceModeAbility_get(
    uint32              unit,
    rtk_port_t          port,
    rtk_port_speed_t    *pSpeed,
    rtk_port_duplex_t   *pDuplex,
    rtk_enable_t        *pFlowControl)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->port_phyForceModeAbility_get(unit, phy_port, pSpeed, pDuplex, pFlowControl);
    }
#else
    return RT_MAPPER(unit)->port_phyForceModeAbility_get(unit, port, pSpeed, pDuplex, pFlowControl);
#endif
} /* end of rtk_port_phyForceModeAbility_get */

/* Function Name:
 *      rtk_port_phyForceModeAbility_set
 * Description:
 *      Set the port speed/duplex mode/pause/asy_pause in the PHY force mode
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      speed       - port speed
 *      duplex      - port duplex mode
 *      flowControl - enable flow control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID    - invalid unit id
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_PORT_ID    - invalid port id
 *      RT_ERR_PHY_SPEED  - invalid PHY speed setting
 *      RT_ERR_PHY_DUPLEX - invalid PHY duplex setting
 *      RT_ERR_INPUT      - invalid input parameter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) You can set these abilities no matter which mode PHY currently stays on
 *      (2) The speed type of the port is as following:
 *          - PORT_SPEED_10M
 *          - PORT_SPEED_100M
 *          - PORT_SPEED_1000M (only for fiber media)
 *      (3) The duplex mode of the port is as following:
 *          - PORT_HALF_DUPLEX
 *          - PORT_FULL_DUPLEX
 */
int32
rtk_port_phyForceModeAbility_set(
    uint32              unit,
    rtk_port_t          port,
    rtk_port_speed_t    speed,
    rtk_port_duplex_t   duplex,
    rtk_enable_t        flowControl)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->port_phyForceModeAbility_set(unit, phy_port, speed, duplex, flowControl);
    }
#else
    return RT_MAPPER(unit)->port_phyForceModeAbility_set(unit, port, speed, duplex, flowControl);
#endif
} /* end of rtk_port_phyForceModeAbility_set */

/* Function Name:
 *      rtk_port_phyMasterSlave_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      This function only works on giga/ 10g port to get its master/slave mode configuration.
 */
int32
rtk_port_phyMasterSlave_get(
    uint32              unit,
    rtk_port_t          port,
    rtk_port_masterSlave_t   *pMasterSlaveCfg,
    rtk_port_masterSlave_t   *pMasterSlaveActual)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_phyMasterSlave_get(unit, port, pMasterSlaveCfg, pMasterSlaveActual);
}/* end of rtk_port_phyMasterSlave_get */

/* Function Name:
 *      rtk_port_phyMasterSlave_set
 * Description:
 *      Set PHY configuration of master/slave mode of the specific port
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      masterSlave - PHY master slave configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      This function only works on giga/ 10g port to set its master/slave mode configuration.
 */
int32
rtk_port_phyMasterSlave_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_port_masterSlave_t  masterSlave)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_phyMasterSlave_set(unit, port, masterSlave);
}/* end of rtk_port_phyMasterSlave_set */

/* Function Name:
 *      rtk_port_phyReg_get
 * Description:
 *      Get PHY register data of the specific port
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      page  - page id
 *      reg   - reg id
 * Output:
 *      pData - pointer to the PHY reg data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid page id
 *      RT_ERR_PHY_REG_ID   - invalid reg id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_port_phyReg_get(
    uint32              unit,
    rtk_port_t          port,
    uint32              page,
    rtk_port_phy_reg_t  reg,
    uint32              *pData)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->port_phyReg_get(unit, phy_port, page, reg, pData);
    }
#else
    return RT_MAPPER(unit)->port_phyReg_get(unit, port, page, reg, pData);
#endif
} /* end of rtk_port_phyReg_get */

/* Function Name:
 *      rtk_port_phyReg_set
 * Description:
 *      Set PHY register data of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 *      page - page id
 *      reg  - reg id
 *      data - reg data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_PORT_ID     - invalid port id
 *      RT_ERR_PHY_PAGE_ID - invalid page id
 *      RT_ERR_PHY_REG_ID  - invalid reg id
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_port_phyReg_set(
    uint32              unit,
    rtk_port_t          port,
    uint32              page,
    rtk_port_phy_reg_t  reg,
    uint32              data)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->port_phyReg_set(unit, phy_port, page, reg, data);
    }
#else
    return RT_MAPPER(unit)->port_phyReg_set(unit, port, page, reg, data);
#endif
} /* end of rtk_port_phyReg_set */

/* Function Name:
 *      rtk_port_phyReg_broadcast_set
 * Description:
 *      Set PHY register data in broadcast way
 * Input:
 *      unit - unit id
 *      page - page id
 *      reg  - reg id
 *      data - reg data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_PHY_PAGE_ID - invalid page id
 *      RT_ERR_PHY_REG_ID  - invalid reg id
 * Applicable:
 *      8390
 * Note:
 *      Specified PHY register of all connected PHYs are set simultaneously. The API is used when
 *      set PHY register to all connected PHYs at the same time is desirous.
 */
int32
rtk_port_phyReg_broadcast_set(
    uint32              unit,
    uint32              page,
    rtk_port_phy_reg_t  reg,
    uint32              data)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_phyReg_broadcast_set(unit, page, reg, data);
} /* end of rtk_port_phyReg_broadcast_set */

/* Function Name:
 *      rtk_port_phyReg_broadcastID_set
 * Description:
 *      Set PHY broadcast ID
 * Input:
 *      unit        - unit id
 *      broadcastID - broadcast id
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
 *      Use the API to change the PHY broadcast ID. Default broadcast ID is 31.
 */
int32
rtk_port_phyReg_broadcastID_set(
    uint32              unit,
    uint32              broadcastID)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_phyReg_broadcastID_set(unit, broadcastID);
} /* end of rtk_port_phyReg_broadcastID_set */

/* Function Name:
 *      rtk_port_phyExtParkPageReg_get
 * Description:
 *      Get PHY register data of the specific port with extension page and parking page parameters
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      mainPage - main page id
 *      extPage  - extension page id
 *      parkPage - parking page id
 *      reg      - reg id
 * Output:
 *      pData    - pointer to the PHY reg data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid page id
 *      RT_ERR_PHY_REG_ID   - invalid reg id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
rtk_port_phyExtParkPageReg_get(
    uint32              unit,
    rtk_port_t          port,
    uint32              mainPage,
    uint32              extPage,
    uint32              parkPage,
    rtk_port_phy_reg_t  reg,
    uint32              *pData)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_phyExtParkPageReg_get(unit, port, mainPage, extPage, parkPage, reg, pData);
} /* end of rtk_port_phyExtParkPageReg_get */

/* Function Name:
 *      rtk_port_phyExtParkPageReg_set
 * Description:
 *      Set PHY register data of the specific port with extension page and parking page parameters
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      mainPage - main page id
 *      extPage  - extension page id
 *      parkPage - parking page id
 *      reg      - reg id
 *      data     - reg data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_PORT_ID     - invalid port id
 *      RT_ERR_PHY_PAGE_ID - invalid page id
 *      RT_ERR_PHY_REG_ID  - invalid reg id
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
rtk_port_phyExtParkPageReg_set(
    uint32              unit,
    rtk_port_t          port,
    uint32              mainPage,
    uint32              extPage,
    uint32              parkPage,
    rtk_port_phy_reg_t  reg,
    uint32              data)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_phyExtParkPageReg_set(unit, port, mainPage, extPage, parkPage, reg, data);
} /* end of rtk_port_phyExtParkPageReg_set */

/* Function Name:
 *      rtk_port_phymaskExtParkPageReg_set
 * Description:
 *      Set PHY register data of the specific portmask with extension page and parking page parameters
 * Input:
 *      unit      - unit id
 *      pPortmask - pointer to the portmask
 *      mainPage  - main page id
 *      extPage   - extension page id
 *      parkPage  - parking page id
 *      reg       - reg id
 *      data      - reg data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid page id
 *      RT_ERR_PHY_REG_ID   - invalid reg id
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
rtk_port_phymaskExtParkPageReg_set(
    uint32              unit,
    rtk_portmask_t      *pPortmask,
    uint32              mainPage,
    uint32              extPage,
    uint32              parkPage,
    rtk_port_phy_reg_t  reg,
    uint32              data)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_phymaskExtParkPageReg_set(unit, pPortmask, mainPage, extPage, parkPage, reg, data);
} /* end of rtk_port_phymaskExtParkPageReg_set */

/* Function Name:
 *      rtk_port_phyMmdReg_get
 * Description:
 *      Get PHY MMD register data of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      mmdAddr - mmd device address
 *      mmdReg  - mmd reg id
 * Output:
 *      pData   - pointer to the PHY reg data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_port_phyMmdReg_get(
    uint32              unit,
    rtk_port_t          port,
    uint32              mmdAddr,
    uint32              mmdReg,
    uint32              *pData)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_phyMmdReg_get(unit, port, mmdAddr, mmdReg, pData);
} /* end of rtk_port_phyMmdReg_get */

/* Function Name:
 *      rtk_port_phyMmdReg_set
 * Description:
 *      Set PHY MMD register data of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      mmdAddr - mmd device address
 *      mmdReg  - mmd reg id
 *      data    - reg data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_port_phyMmdReg_set(
    uint32              unit,
    rtk_port_t          port,
    uint32              mmdAddr,
    uint32              mmdReg,
    uint32              data)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_phyMmdReg_set(unit, port, mmdAddr, mmdReg, data);
} /* end of rtk_port_phyMmdReg_set */

/* Function Name:
 *      rtk_port_phymaskMmdReg_set
 * Description:
 *      Set PHY MMD register data of the specific portmask
 * Input:
 *      unit      - unit id
 *      pPortmask - pointer to the portmask
 *      mmdAddr   - mmd device address
 *      mmdReg    - mmd reg id
 *      data      - reg data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_port_phymaskMmdReg_set(
    uint32              unit,
    rtk_portmask_t      *pPortmask,
    uint32              mmdAddr,
    uint32              mmdReg,
    uint32              data)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_phymaskMmdReg_set(unit, pPortmask, mmdAddr, mmdReg, data);
} /* end of rtk_port_phymaskMmdReg_set */

/* Function Name:
 *      rtk_port_cpuPortId_get
 * Description:
 *      Get CPU port id of the specific unit
 * Input:
 *      unit  - unit id
 * Output:
 *      pPort - pointer to CPU port id
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_port_cpuPortId_get(uint32 unit, rtk_port_t *pPort)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_cpuPortId_get(unit, pPort);
} /* end of rtk_port_cpuPortId_get */

/* Function Name:
 *      rtk_port_cpuPortId_set
 * Description:
 *      Set CPU port id of the specific unit
 * Input:
 *      unit  - unit id
 *      port - pointer to CPU port id
 * Output:
*       None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 * Applicable:
 *      8380
 * Note:
 *      None
 */
int32
rtk_port_cpuPortId_set(uint32 unit, rtk_port_t pPort)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_cpuPortId_set(unit, pPort);
} /* end of rtk_port_cpuPortId_get */

/* Function Name:
 *      rtk_port_isolation_get
 * Description:
 *      Get the portmask of the port isolation
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pPortmask - pointer to the portmask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      Each port can specify an allowable egress forwarding portmask.
 */
int32
rtk_port_isolation_get(uint32 unit, rtk_port_t port, rtk_portmask_t *pPortmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32   ret;
    int32   phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);
    rtk_portmask_t  temp_portmask;

    if ((ret = RT_MAPPER(unit)->port_isolation_get(unit, phy_port, &temp_portmask)) != RT_ERR_OK)
        return ret;
    *pPortmask = PHYSICAL_PORTMASK_TO_RTK_PORTMASK(unit, temp_portmask);
    return ret;
    }
#else
    return RT_MAPPER(unit)->port_isolation_get(unit, port, pPortmask);
#endif
} /* end of rtk_port_isolation_get */

/* Function Name:
 *      rtk_port_isolation_set
 * Description:
 *      Set the portmask of the port isolation
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      portmask - pointer to the portmask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID   - invalid unit id
 *      RT_ERR_NOT_INIT  - The module is not initial
 *      RT_ERR_PORT_ID   - invalid port id
 *      RT_ERR_PORT_MASK - invalid port mask
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      Each port can specify an allowable egress forwarding portmask.
 */
int32
rtk_port_isolation_set(uint32 unit, rtk_port_t port, rtk_portmask_t portmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32   phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);
    rtk_portmask_t  temp_portmask;

    temp_portmask = RTK_PORTMASK_TO_PHYSICAL_PORTMASK(unit, portmask);
    return RT_MAPPER(unit)->port_isolation_set(unit, phy_port, temp_portmask);
    }
#else
    return RT_MAPPER(unit)->port_isolation_set(unit, port, portmask);
#endif
} /* end of rtk_port_isolation_set */

/* Function Name:
 *      rtk_port_isolation_add
 * Description:
 *      Add an isolation port to the certain port
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      iso_port - isolation port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) Default value of each port represent bit in portmask is 1
 *      (2) Port and iso_port will be isolated when this API is called
 *      (3) The iso_port to the relative portmask bit will be set to 1
 */
int32
rtk_port_isolation_add(uint32 unit, rtk_port_t port, rtk_port_t iso_port)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);
    int32  phy_iso_port = RTK_PORT_TO_PHYSICAL_PORT(unit, iso_port);

    return RT_MAPPER(unit)->port_isolation_add(unit, phy_port, phy_iso_port);
    }
#else
    return RT_MAPPER(unit)->port_isolation_add(unit, port, iso_port);
#endif
} /* end of rtk_port_isolation_add */

/* Function Name:
 *      rtk_port_isolation_del
 * Description:
 *      Delete an existing isolation port of the certain port
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      iso_port - isolation port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) Default value of each port represent bit in portmask is 1
 *      (2) Isolated status between the port and the iso_port is removed when this API is called
 *      (3) The iso_port to the relative portmask bit will be set to 0
 */
int32
rtk_port_isolation_del(uint32 unit, rtk_port_t port, rtk_port_t iso_port)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);
    int32  phy_iso_port = RTK_PORT_TO_PHYSICAL_PORT(unit, iso_port);

    return RT_MAPPER(unit)->port_isolation_del(unit, phy_port, phy_iso_port);
    }
#else
    return RT_MAPPER(unit)->port_isolation_del(unit, port, iso_port);
#endif
} /* end of rtk_port_isolation_del */

/* Function Name:
 *      rtk_port_phyComboPortMedia_get
 * Description:
 *      Get PHY port media of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pMedia - pointer to the port media
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_port_phyComboPortMedia_get(uint32 unit, rtk_port_t port, rtk_port_media_t *pMedia)
{
#if defined(CONFIG_SDK_AUTO_COMBO_MEDIA_BY_GPIO)
  #if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);
    rtk_portmask_t  portmask;

    /* parameter check */
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, phy_port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pMedia), RT_ERR_NULL_POINTER);

    dal_losMon_swScanPorts_get(unit, &portmask);
    if (RTK_PORTMASK_IS_PORT_SET(portmask, phy_port))
    {
        *pMedia = PORT_MEDIA_FIBER_AUTO_BY_GPIO;
        return RT_ERR_OK;
    }

    return RT_MAPPER(unit)->port_phyComboPortMedia_get(unit, phy_port, pMedia);
  #else
    rtk_portmask_t  portmask;

    /* parameter check */
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pMedia), RT_ERR_NULL_POINTER);

    dal_losMon_swScanPorts_get(unit, &portmask);
    if (RTK_PORTMASK_IS_PORT_SET(portmask, port))
    {
        *pMedia = PORT_MEDIA_FIBER_AUTO_BY_GPIO;
        return RT_ERR_OK;
    }

    return RT_MAPPER(unit)->port_phyComboPortMedia_get(unit, port, pMedia);
  #endif
#else
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

  #if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->port_phyComboPortMedia_get(unit, phy_port, pMedia);
    }
  #else
    return RT_MAPPER(unit)->port_phyComboPortMedia_get(unit, port, pMedia);
  #endif
#endif
} /* end of rtk_port_phyComboPortMedia_get */

/* Function Name:
 *      rtk_port_phyComboPortMedia_set
 * Description:
 *      Set PHY port media of the specific port
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      media - port media
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The media value is as following:
 *          - PORT_MEDIA_COPPER
 *          - PORT_MEDIA_FIBER
 *          - PORT_MEDIA_COPPER_AUTO
 *          - PORT_MEDIA_FIBER_AUTO
 *          - PORT_MEDIA_FIBER_AUTO_BY_GPIO (valid if CONFIG_SDK_AUTO_COMBO_MEDIA_BY_GPIO)
 */
int32
rtk_port_phyComboPortMedia_set(uint32 unit, rtk_port_t port, rtk_port_media_t media)
{
#if defined(CONFIG_SDK_AUTO_COMBO_MEDIA_BY_GPIO)
  #if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);
    rtk_portmask_t  portmask;

    /* parameter check */
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, phy_port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((media >= PORT_MEDIA_END), RT_ERR_INPUT);

    if (media == PORT_MEDIA_FIBER_AUTO_BY_GPIO)
    {
        dal_losMon_swScanPorts_get(unit, &portmask);
        RTK_PORTMASK_PORT_SET(portmask, phy_port);
        dal_losMon_swScanPorts_set(unit, &portmask);
        return RT_ERR_OK;
    }
    else
    {
        dal_losMon_swScanPorts_get(unit, &portmask);
        RTK_PORTMASK_PORT_CLEAR(portmask, phy_port);
        dal_losMon_swScanPorts_set(unit, &portmask);

        return RT_MAPPER(unit)->port_phyComboPortMedia_set(unit, phy_port, media);
    }
  #else
    rtk_portmask_t  portmask;

    /* parameter check */
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((media >= PORT_MEDIA_END), RT_ERR_INPUT);

    if (media == PORT_MEDIA_FIBER_AUTO_BY_GPIO)
    {
        dal_losMon_swScanPorts_get(unit, &portmask);
        RTK_PORTMASK_PORT_SET(portmask, port);
        dal_losMon_swScanPorts_set(unit, &portmask);
        return RT_ERR_OK;
    }
    else
    {
        dal_losMon_swScanPorts_get(unit, &portmask);
        RTK_PORTMASK_PORT_CLEAR(portmask, port);
        dal_losMon_swScanPorts_set(unit, &portmask);

        return RT_MAPPER(unit)->port_phyComboPortMedia_set(unit, port, media);
    }
  #endif
#else
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);
    RT_PARAM_CHK((media >= PORT_MEDIA_END || media == PORT_MEDIA_FIBER_AUTO_BY_GPIO), RT_ERR_INPUT);

  #if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->port_phyComboPortMedia_set(unit, phy_port, media);
    }
  #else
    return RT_MAPPER(unit)->port_phyComboPortMedia_set(unit, port, media);
  #endif
#endif
} /* end of rtk_port_phyComboPortMedia_set */

/* Function Name:
 *      rtk_port_backpressureEnable_get
 * Description:
 *      Get the half duplex backpressure enable status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to the enable status of backpressure in half duplex mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The mac back pressure enable status of the port is as following:
 *          - DISABLED
 *          - ENABLED
 *      (2) Used to support backpressure in half mode.
 */
int32
rtk_port_backpressureEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->port_backpressureEnable_get(unit, phy_port, pEnable);
    }
#else
    return RT_MAPPER(unit)->port_backpressureEnable_get(unit, port, pEnable);
#endif
} /* end of rtk_port_backpressureEnable_get */

/* Function Name:
 *      rtk_port_backpressureEnable_set
 * Description:
 *      Set the half duplex backpressure enable status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of backpressure in half duplex mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The mac back pressure enable status of the port is as following:
 *          - DISABLED
 *          - ENABLED
 *      (2) Used to support backpressure in half mode.
 */
int32
rtk_port_backpressureEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->port_backpressureEnable_set(unit, phy_port, enable);
    }
#else
    return RT_MAPPER(unit)->port_backpressureEnable_set(unit, port, enable);
#endif
} /* end of rtk_port_backpressureEnable_set */

/* Function Name:
 *      rtk_port_adminEnable_get
 * Description:
 *      Get port admin status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to the port admin status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_port_adminEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->port_adminEnable_get(unit, phy_port, pEnable);
    }
#else
    return RT_MAPPER(unit)->port_adminEnable_get(unit, port, pEnable);
#endif
} /* end of rtk_port_adminEnable_get */

/* Function Name:
 *      rtk_port_adminEnable_set
 * Description:
 *      Set port admin configuration of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - port admin configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_port_adminEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->port_adminEnable_set(unit, phy_port, enable);
    }
#else
    return RT_MAPPER(unit)->port_adminEnable_set(unit, port, enable);
#endif
} /* end of rtk_port_adminEnable_set */

/* Function Name:
 *      rtk_port_linkMon_enable
 * Description:
 *      Enable link monitor thread
 * Input:
 *      scan_interval_us - scan interval in us.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - initialize success
 *      RT_ERR_FAILED       - initialize fail
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - scan interval is too small
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      When enable link monitor thread, all link change interrupt will be handled by thread.
 */
int32
rtk_port_linkMon_enable(uint32 scan_interval_us)
{
    return dal_linkMon_enable(scan_interval_us);
} /* end of rtk_port_linkMon_enable */

/* Function Name:
 *      rtk_port_linkMon_disable
 * Description:
 *      Disable link scan thread
 * Input:
 *      None.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - initialize success
 *      RT_ERR_FAILED   - initialize fail
 *      RT_ERR_NOT_INIT - The module is not initial
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      When disable link monitor thread, all link change interrupt will be callback to upper layer.
 */
int32
rtk_port_linkMon_disable(void)
{
    return dal_linkMon_disable();
} /* end of rtk_port_linkMon_disable */

/* Function Name:
 *      rtk_port_linkMon_register
 * Description:
 *      Register callback function for link change notification
 * Input:
 *      linkMon_callback - callback function for link change
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_port_linkMon_register(rtk_port_linkMon_callback_t linkMon_callback)
{
    return dal_linkMon_register(linkMon_callback);
} /* end of rtk_port_linkMon_register */

/* Function Name:
 *      rtk_port_linkMon_unregister
 * Description:
 *      Unregister callback function for link change notification
 * Input:
 *      linkMon_callback    - callback function for link change
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_port_linkMon_unregister(rtk_port_linkMon_callback_t linkMon_callback)
{
    return dal_linkMon_unregister(linkMon_callback);
} /* end of rtk_port_linkMon_unregister */

/* Function Name:
 *      rtk_port_linkMon_swScanPorts_set
 * Description:
 *      Configure portmask of software linkscan for certain unit
 * Input:
 *      unit             - callback function for link change
 *      pSwScan_portmask - portmask for software scan
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - initialize success
 *      RT_ERR_FAILED       - initialize fail
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_port_linkMon_swScanPorts_set(uint32 unit, rtk_portmask_t *pSwScan_portmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    rtk_portmask_t  temp_swScanPortmask;

    temp_swScanPortmask = RTK_PORTMASK_TO_PHYSICAL_PORTMASK(unit, *pSwScan_portmask);
    return dal_linkMon_swScanPorts_set(unit, &temp_swScanPortmask);
    }
#else
    return dal_linkMon_swScanPorts_set(unit, pSwScan_portmask);
#endif
} /* end of rtk_port_linkMon_swScanPorts_set */

/* Function Name:
 *      rtk_port_linkMon_swScanPorts_get
 * Description:
 *      Get portmask of software linkscan for certain unit
 * Input:
 *      unit             - callback function for link change
 * Output:
 *      pSwScan_portmask - portmask for software scan
 * Return:
 *      RT_ERR_OK           - initialize success
 *      RT_ERR_FAILED       - initialize fail
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_port_linkMon_swScanPorts_get(uint32 unit, rtk_portmask_t *pSwScan_portmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32   ret;
    rtk_portmask_t  temp_swScanPortmask;

    if ((ret = dal_linkMon_swScanPorts_get(unit, &temp_swScanPortmask)) != RT_ERR_OK)
        return ret;
    *pSwScan_portmask = PHYSICAL_PORTMASK_TO_RTK_PORTMASK(unit, temp_swScanPortmask);
    return ret;
    }
#else
    return dal_linkMon_swScanPorts_get(unit, pSwScan_portmask);
#endif
} /* end of rtk_port_linkMon_swScanPorts_get */

/* Function Name:
 *      rtk_port_txEnable_get
 * Description:
 *      Get the TX enable status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pEnable - pointer to the port TX status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The TX enable status of the port is as following:
 *      - DISABLED
 *      - ENABLED
 */
int32
rtk_port_txEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->port_txEnable_get(unit, phy_port, pEnable);
    }
#else
    return RT_MAPPER(unit)->port_txEnable_get(unit, port, pEnable);
#endif
} /* end of rtk_port_txEnable_get */

/* Function Name:
 *      rtk_port_txEnable_set
 * Description:
 *      Set the TX enable status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of TX
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The TX enable status of the port is as following:
 *      - DISABLED
 *      - ENABLED
 */
int32
rtk_port_txEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->port_txEnable_set(unit, phy_port, enable);
    }
#else
    return RT_MAPPER(unit)->port_txEnable_set(unit, port, enable);
#endif
} /* end of rtk_port_txEnable_set */

/* Function Name:
 *      rtk_port_rxEnable_get
 * Description:
 *      Get the RX enable status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pEnable - pointer to the port RX status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The RX enable status of the port is as following:
 *      - DISABLED
 *      - ENABLED
 */
int32
rtk_port_rxEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->port_rxEnable_get(unit, phy_port, pEnable);
    }
#else
    return RT_MAPPER(unit)->port_rxEnable_get(unit, port, pEnable);
#endif
} /* end of rtk_port_rxEnable_get */

/* Function Name:
 *      rtk_port_rxEnable_set
 * Description:
 *      Set the RX enable status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of RX
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The RX enable status of the port is as following:
 *      - DISABLED
 *      - ENABLED
 */
int32
rtk_port_rxEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->port_rxEnable_set(unit, phy_port, enable);
    }
#else
    return RT_MAPPER(unit)->port_rxEnable_set(unit, port, enable);
#endif
} /* end of rtk_port_rxEnable_set */

/* Function Name:
 *      rtk_port_specialCongest_set
 * Description:
 *      Set the congest seconds of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      second - congest timer (seconds)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8389, 8390, 8380
 * Note:
 *      (1) The valid range of congest seconds is 0~15 in 8389.
 *      (2) If a port is continuously congested for congest seconds, the system would drain out the packets of the port,
 *          so the control traffic has chance to transmit to solve the congest situation.
 */
int32
rtk_port_specialCongest_set(uint32 unit, rtk_port_t port, uint32 second)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->port_specialCongest_set(unit, phy_port, second);
    }
#else
    return RT_MAPPER(unit)->port_specialCongest_set(unit, port, second);
#endif
} /* end of rtk_port_specialCongest_set */

/* Function Name:
 *      rtk_port_greenEnable_get
 * Description:
 *      Get the status of green feature of the specific port in the specific unit
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to status of green feature
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_port_greenEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_greenEnable_get(unit, port, pEnable);
} /* end of rtk_port_greenEnable_get */

/* Function Name:
 *      rtk_port_greenEnable_set
 * Description:
 *      Set the statue of green feature of the specific port in the specific unit
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of green feature
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      Needs to restart auto-negotiation for the configuration to take effect.
 */
int32
rtk_port_greenEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_greenEnable_set(unit, port, enable);
} /* end of rtk_port_greenEnable_set */

/* Module Name    : Port */
/* Sub-module Name: UDLD */

/* Function Name:
 *      rtk_port_udldEnable_get
 * Description:
 *      Get enable status of UDLD on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of UDLD
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_port_udldEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_udldEnable_get(unit, port, pEnable);
} /* end of rtk_port_udldEnable_get */

/* Function Name:
 *      rtk_port_udldEnable_set
 * Description:
 *      Set enable status of UDLD on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of UDLD
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_port_udldEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_udldEnable_set(unit, port, enable);
} /* end of rtk_port_udldEnable_set */

/* Function Name:
 *      rtk_port_udldLinkUpAutoTriggerEnable_get
 * Description:
 *      Get enable status of link up auto trigger UDLD test.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of link up auto trigger UDLD
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_port_udldLinkUpAutoTriggerEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_udldLinkUpAutoTriggerEnable_get(unit, pEnable);
} /* end of rtk_port_udldLinkUpAutoTriggerEnable_get */

/* Function Name:
 *      rtk_port_udldLinkUpAutoTriggerEnable_set
 * Description:
 *      Set enable status of link up auto trigger UDLD test.
 * Input:
 *      unit   - unit id
 *      enable - enable status of link up auto trigger UDLD
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_port_udldLinkUpAutoTriggerEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_udldLinkUpAutoTriggerEnable_set(unit, enable);
} /* end of rtk_port_udldLinkUpAutoTriggerEnable_set */

/* Function Name:
 *      rtk_port_udldTrigger_start
 * Description:
 *      Trigger UDLD test on specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_port_udldTrigger_start(uint32 unit, rtk_port_t port)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_udldTrigger_start(unit, port);
} /* end of rtk_port_udldTrigger_start */

/* Function Name:
 *      rtk_port_udldAutoDisableFailedPortEnable_get
 * Description:
 *      Get UDLD test status of specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pStatus - pointer to UDLD test status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      UDLD test status is as following
 *      - UDLD_UNIDIR           UDLD test result is unidirection
 *      - UDLD_BIDIR            UDLD test result is bidirection
 *      - UDLD_UNDETERMINE      UDLD test is on going. Wait more time to get result.
 */
int32
rtk_port_udldStatus_get(uint32 unit, rtk_port_t port, rtk_port_udldStatus_t *pStatus)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_udldStatus_get(unit, port, pStatus);
} /* end of rtk_port_udldStatus_get */

/* Function Name:
 *      rtk_port_udldAutoDisableFailedPortEnable_get
 * Description:
 *      Get enable status of auto disable UDLD failed port.
 *      If enabled, switch will automatically disable unidirectional port.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of auto disabled UDLD failed port
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_port_udldAutoDisableFailedPortEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_udldAutoDisableFailedPortEnable_get(unit, pEnable);
} /* end of rtk_port_udldAutoDisableFailedPortEnable_get */

/* Function Name:
 *      rtk_port_udldAutoDisableFailedPortEnable_set
 * Description:
 *      Set enable status of auto disable UDLD failed port.
 *      If enabled, switch will automatically disable unidirectional port.
 * Input:
 *      unit   - unit id
 *      enable - enable status of auto disabled UDLD failed port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_port_udldAutoDisableFailedPortEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_udldAutoDisableFailedPortEnable_set(unit, enable);
} /* end of rtk_port_udldAutoDisableFailedPortEnable_set */

/* Function Name:
 *      rtk_port_udldInterval_get
 * Description:
 *      Get UDLD echo generate interval.
 * Input:
 *      unit      - unit id
 * Output:
 *      pInterval - pointer to interval of UDLD echo generate (unit: second)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The valid interval value is 2, 4, 8 and 16 seconds
 */
int32
rtk_port_udldInterval_get(uint32 unit, rtk_port_udldInterval_t *pInterval)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_udldInterval_get(unit, pInterval);
} /* end of rtk_port_udldInterval_get */

/* Function Name:
 *      rtk_port_udldInterval_set
 * Description:
 *      Set UDLD echo generate interval.
 * Input:
 *      unit     - unit id
 *      interval - interval of UDLD echo generate (unit: second)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      The valid interval value is 2, 4, 8 and 16 seconds
 */
int32
rtk_port_udldInterval_set(uint32 unit, rtk_port_udldInterval_t interval)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_udldInterval_set(unit, interval);
} /* end of rtk_port_udldInterval_set */

/* Function Name:
 *      rtk_port_udldRetryCount_get
 * Description:
 *      Get retry count of UDLD.
 * Input:
 *      unit        - unit id
 * Output:
 *      pRetryCount - pointer to retry count of UDLD
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The valid retryCount value is 1, 2, 4, 8
 */
int32
rtk_port_udldRetryCount_get(uint32 unit, uint32 *pRetryCount)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_udldRetryCount_get(unit, pRetryCount);
} /* end of rtk_port_udldRetryCount_get */

/* Function Name:
 *      rtk_port_udldRetryCount_set
 * Description:
 *      Set retry count of UDLD.
 * Input:
 *      unit       - unit id
 *      retryCount - retry count of UDLD
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      The valid retryCount value is 1, 2, 4, 8
 */
int32
rtk_port_udldRetryCount_set(uint32 unit, uint32 retryCount)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_udldRetryCount_set(unit, retryCount);
} /* end of rtk_port_udldRetryCount_set */

/* Function Name:
 *      rtk_port_udldLedIndicateEnable_get
 * Description:
 *      Get enable status of LED indication when unidirectional link detection.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of LED indication
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_port_udldLedIndicateEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_udldLedIndicateEnable_get(unit, pEnable);
} /* end of rtk_port_udldLedIndicateEnable_get */

/* Function Name:
 *      rtk_port_udldLedIndicateEnable_set
 * Description:
 *      Set enable status of LED indication when unidirectional link detection.
 * Input:
 *      unit   - unit id
 *      enable - enable status of LED indication
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_port_udldLedIndicateEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_udldLedIndicateEnable_set(unit, enable);
} /* end of rtk_port_udldLedIndicateEnable_set */

/* Function Name:
 *      rtk_port_udldEchoAction_get
 * Description:
 *      Get udld echo packet action.
 * Input:
 *      unit   - unit id
 * Output:
 *      pAction - echo packet action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The valid action of udld echo packet is as following:
 *      - UDLD_ACTION_SELFLOOPBACK
 *      - UDLD_ACTION_FORWARD
 */
int32
rtk_port_udldEchoAction_get(uint32 unit, rtk_port_udldEchoAction_t *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_udldEchoAction_get(unit, pAction);
} /* end of rtk_port_udldEchoAction_get */

/* Function Name:
 *      rtk_port_udldEchoAction_set
 * Description:
 *      Set udld echo packet action.
 * Input:
 *      unit   - unit id
 *      action - echo packet action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      The valid action of udld echo packet is as following:
 *      - UDLD_ACTION_SELFLOOPBACK
 *      - UDLD_ACTION_FORWARD
 */
int32
rtk_port_udldEchoAction_set(uint32 unit, rtk_port_udldEchoAction_t action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_udldEchoAction_set(unit, action);
} /* end of rtk_port_udldEchoAction_set */

/* Function Name:
 *      rtk_port_udldLinkStatus_get
 * Description:
 *      Get udld port link status.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pStatus - link status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The valid value of udld link status is as following:
 *      - UDLD_LINK_STATUS_NORMAL
 *      - UDLD_LINK_STATUS_DISABLE
 */
int32
rtk_port_udldLinkStatus_get(uint32 unit, rtk_port_t port, rtk_port_udldLinkStatus_t *pStatus)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_udldLinkStatus_get(unit, port, pStatus);
} /* end of rtk_port_udldLinkStatus_get */

/* Function Name:
 *      rtk_port_udldLinkStatus_set
 * Description:
 *      Set udld link status.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      status - link status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      The valid value of udld link status is as following:
 *      - UDLD_LINK_STATUS_NORMAL
 *      - UDLD_LINK_STATUS_DISABLE
 */
int32
rtk_port_udldLinkStatus_set(uint32 unit, rtk_port_t port, rtk_port_udldLinkStatus_t status)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_udldLinkStatus_set(unit, port, status);
} /* end of rtk_port_udldLinkStatus_set */

/* Module Name    : Port */
/* Sub-module Name: RLDP */

 /* Function Name:
 *      rtk_port_rldpEnable_get
 * Description:
 *      Get enable status of RLDP on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of RLDP
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_port_rldpEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_rldpEnable_get(unit, port, pEnable);
} /* end of rtk_port_rldpEnable_get */

 /* Function Name:
 *      rtk_port_rldpEnable_set
 * Description:
 *      Set enable status of RLDP on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of RLDP
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_port_rldpEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_rldpEnable_set(unit, port, enable);
} /* end of rtk_port_rldpEnable_set */

/* Function Name:
 *      rtk_port_rldpStatus_get
 * Description:
 *      Get result of RLDP test on specified port.
 * Input:
 *      unit          - unit id
 *      port          - port id
 * Output:
 *      pNormalStatus - Status of normal loop test
 *      pSelfStatus   - Status of self loop test
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) Result of RLDP self loop is as following
 *          - RLDP_FORWARDING
 *          - RLDP_LISTEN
 *          - RLDP_BLOCK
 *      (2) Result of RLDP normal loop is as following
 *          - RLDP_NORMAL_LOOP
 *          - RLDP_NO_NORMAL_LOOP
 */
int32
rtk_port_rldpStatus_get(
    uint32                          unit,
    rtk_port_t                      port,
    rtk_port_rldpNormalStatus_t     *pNormalStatus,
    rtk_port_rldpSelfStatus_t       *pSelfStatus)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_rldpStatus_get(unit, port, pNormalStatus, pSelfStatus);
} /* end of rtk_port_rldpStatus_get */

/* Function Name:
 *      rtk_port_rldpAutoBlockEnable_get
 * Description:
 *      Get enable status of auto blocking self loop port on specified port.
 *      When enabled, switch will automatically block self loop port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of auto blocking self loop port
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_port_rldpAutoBlockEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_rldpAutoBlockEnable_get(unit, port, pEnable);
} /* end of rtk_port_rldpAutoBlockEnable_get */

/* Function Name:
 *      rtk_port_rldpAutoBlockEnable_set
 * Description:
 *      Set enable status of auto blocking self loop port on specified port.
 *      When enabled, switch will automatically block self loop port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of auto blocking self loop port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_port_rldpAutoBlockEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_rldpAutoBlockEnable_set(unit, port, enable);
} /* end of rtk_port_rldpAutoBlockEnable_set */

/* Function Name:
 *      rtk_port_rldpInterval_get
 * Description:
 *      Get interval of RLDP hello interval on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pInterval - pointer to interval of RLDP hello interval(unit: second)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The valid value of RLDP hello interval is 1, 2, 4, 8 second
 */
int32
rtk_port_rldpInterval_get(uint32 unit, rtk_port_t port, uint32 *pInterval)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_rldpInterval_get(unit, port, pInterval);
} /* end of rtk_port_rldpInterval_get */

/* Function Name:
 *      rtk_port_rldpInterval_set
 * Description:
 *      Set interval of RLDP hello interval on specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      interval - interval of RLDP hello interval(unit: second)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      The valid value of RLDP hello interval is 1, 2, 4, 8 second
 */
int32
rtk_port_rldpInterval_set(uint32 unit, rtk_port_t port, uint32 interval)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_rldpInterval_set(unit, port, interval);
} /* end of rtk_port_rldpInterval_set */

/* Function Name:
 *      rtk_port_rldpSelfLoopAgingTime_get
 * Description:
 *      Get aging time of self loop on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pAgingTime - pointer to aging time of self loop(unit: second)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The valid value of aging time of RLDP self loop is 1, 2, 4, 8 second
 */
int32
rtk_port_rldpSelfLoopAgingTime_get(uint32 unit, rtk_port_t port, uint32 *pAgingTime)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_rldpSelfLoopAgingTime_get(unit, port, pAgingTime);
} /* end of rtk_port_rldpSelfLoopAgingTime_get */

/* Function Name:
 *      rtk_port_rldpSelfLoopAgingTime_set
 * Description:
 *      Set aging time of self loop on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      agingTime - aging time of self loop(unit: second)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      The valid value of aging time of RLDP self loop is 1, 2, 4, 8 second
 */
int32
rtk_port_rldpSelfLoopAgingTime_set(uint32 unit, rtk_port_t port, uint32 agingTime)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_rldpSelfLoopAgingTime_set(unit, port, agingTime);
} /* end of rtk_port_rldpSelfLoopAgingTime_set */

/* Function Name:
 *      rtk_port_rldpNormalLoopAgingTime_get
 * Description:
 *      Get aging time of normal loop on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pAgingTime - pointer to aging time of normal loop(unit: second)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The valid value of aging time of RLDP normal loop is 1, 2, 4, 8 second
 */
int32
rtk_port_rldpNormalLoopAgingTime_get(uint32 unit, rtk_port_t port, uint32 *pAgingTime)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_rldpNormalLoopAgingTime_get(unit, port, pAgingTime);
} /* end of rtk_port_rldpNormalLoopAgingTime_get */

/* Function Name:
 *      rtk_port_rldpNormalLoopAgingTime_set
 * Description:
 *      Set aging time of normal loop on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      agingTime - aging time of normal loop(unit: second)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      The valid value of aging time of RLDP normal loop is 1, 2, 4, 8 second
 */
int32
rtk_port_rldpNormalLoopAgingTime_set(uint32 unit, rtk_port_t port, uint32 agingTime)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_rldpNormalLoopAgingTime_set(unit, port, agingTime);
} /* end of rtk_port_rldpNormalLoopAgingTime_set */

/* Function Name:
 *      rtk_port_phyCrossOverMode_get
 * Description:
 *      Get cross over mode in the specified port.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pMode - pointer to cross over mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The valid value of mode is as following:
 *      - PORT_CROSSOVER_MODE_AUTO
 *      - PORT_CROSSOVER_MODE_MDI
 *      - PORT_CROSSOVER_MODE_MDIX
 */
int32
rtk_port_phyCrossOverMode_get(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t *pMode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_phyCrossOverMode_get(unit, port, pMode);
} /* end of rtk_port_phyCrossOverMode_get */

/* Function Name:
 *      rtk_port_phyCrossOverMode_set
 * Description:
 *      Set cross over mode in the specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 *      mode - cross over mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The valid value of mode is as following:
 *      - PORT_CROSSOVER_MODE_AUTO
 *      - PORT_CROSSOVER_MODE_MDI
 *      - PORT_CROSSOVER_MODE_MDIX
 */
int32
rtk_port_phyCrossOverMode_set(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t mode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_phyCrossOverMode_set(unit, port, mode);
} /* end of rtk_port_phyCrossOverMode_set */

/* Function Name:
 *      rtk_port_phyCrossOverStatus_get
 * Description:
 *      Get cross over status in the specified port.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pStatus - pointer to cross over mode status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_PHY_FIBER_LINKUP - This feature is not supported in this mode
 * Applicable:
 *      8390, 8380
 * Note:
 *      The valid value of mode is as following:
 *      - PORT_CROSSOVER_MODE_MDI
 *      - PORT_CROSSOVER_MODE_MDIX
 */
int32
rtk_port_phyCrossOverStatus_get(uint32 unit, rtk_port_t port, rtk_port_crossOver_status_t *pStatus)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_phyCrossOverStatus_get(unit, port, pStatus);
} /* end of rtk_port_phyCrossOverStatus_get */


/* Function Name:
 *      rtk_port_flowCtrlEnable_get
 * Description:
 *      Get the flow control status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to the status of the flow control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The API get the flow control status by port based, no matter N-WAY is enabled or disabled.
 */
int32
rtk_port_flowCtrlEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_flowCtrlEnable_get(unit, port, pEnable);
} /* end of rtk_port_flowCtrlEnable_get */

/* Function Name:
 *      rtk_port_flowCtrlEnable_get
 * Description:
 *      Set the flow control status to the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of flow control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The API is apply the flow control status by port based, no matter N-WAY is enabled or disabled.
 */
int32
rtk_port_flowCtrlEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_flowCtrlEnable_set(unit, port, enable);
} /* end of rtk_port_flowCtrlEnable_set */

/* Function Name:
 *      rtk_port_phyComboPortFiberMedia_get
 * Description:
 *      Get PHY port fiber media of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pMedia - pointer to the port fiber media
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      1. fiber media type value is as following:
 *      - PORT_FIBER_MEDIA_1000
 *      - PORT_FIBER_MEDIA_100
 *      - PORT_FIBER_MEDIA_AUTO
 */
int32
rtk_port_phyComboPortFiberMedia_get(uint32 unit, rtk_port_t port, rtk_port_fiber_media_t *pMedia)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_phyComboPortFiberMedia_get(unit, port, pMedia);
} /* end of rtk_port_phyComboPortFiberMedia_get */

/* Function Name:
 *      rtk_port_phyComboPortFiberMedia_set
 * Description:
 *      Set PHY port fiber media of the specific port
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      media - port fiber media
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - invalid input parameter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      1. fiber media type value is as following:
 *      - PORT_FIBER_MEDIA_1000
 *      - PORT_FIBER_MEDIA_100
 *      - PORT_FIBER_MEDIA_AUTO
 */
int32
rtk_port_phyComboPortFiberMedia_set(uint32 unit, rtk_port_t port, rtk_port_fiber_media_t media)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);
    RT_PARAM_CHK((media >= PORT_FIBER_MEDIA_END), RT_ERR_INPUT);

    return RT_MAPPER(unit)->port_phyComboPortFiberMedia_set(unit, port, media);
} /* end of rtk_port_phyComboPortFiberMedia_set */

/* Function Name:
 *      rtk_port_linkDownPowerSavingEnable_get
 * Description:
 *      Get the statue of link-down power saving of the specific port in the specific unit
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to status of green feature
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_port_linkDownPowerSavingEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_linkDownPowerSavingEnable_get(unit, port, pEnable);
} /* end of rtk_port_linkDownPowerSavingEnable_get */

/* Function Name:
 *      rtk_port_linkDownPowerSavingEnable_set
 * Description:
 *      Set the statue of link-down power saving of the specific port in the specific unit
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of green feature
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - input parameter out of range
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_port_linkDownPowerSavingEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_linkDownPowerSavingEnable_set(unit, port, enable);
} /* end of rtk_port_linkDownPowerSavingEnable_set */

/* Function Name:
 *      rtk_port_vlanBasedIsolationEntry_get
 * Description:
 *      Get VLAN-based port isolation entry
 * Input:
 *      unit   - unit id
 *      index  - index id
 * Output:
 *      pEntry - pointer to vlan-based port isolation entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      Valid index is 0~15.
 */
int32
rtk_port_vlanBasedIsolationEntry_get(uint32 unit, uint32 index, rtk_port_vlanIsolationEntry_t *pEntry)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32   ret;
    rtk_port_vlanIsolationEntry_t   temp_entry;

    if ((ret = RT_MAPPER(unit)->port_vlanBasedIsolationEntry_get(unit, index, &temp_entry)) != RT_ERR_OK)
        return ret;
    *pEntry = temp_entry;
    (*pEntry).portmask = PHYSICAL_PORTMASK_TO_RTK_PORTMASK(unit, temp_entry.portmask);
    return ret;
    }
#else
    return RT_MAPPER(unit)->port_vlanBasedIsolationEntry_get(unit, index, pEntry);
#endif
} /* end of rtk_port_vlanBasedIsolationEntry_get */

/* Function Name:
 *      rtk_port_vlanBasedIsolationEntry_set
 * Description:
 *      Set VLAN-based port isolation entry
 * Input:
 *      unit   - unit id
 *      index  - index id
 *      pEntry - pointer to vlan-based port isolation entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                              - invalid unit id
 *      RT_ERR_NOT_INIT                             - The module is not initial
 *      RT_ERR_INPUT                                - invalid input parameter
 *      RT_ERR_NULL_POINTER                         - input parameter may be null pointer
 *      RT_ERR_VLAN_VID                             - invalid vid
 *      RT_ERR_PORT_VLAN_ISO_VID_EXIST_IN_OTHER_IDX - vid exists in other entry
 * Applicable:
 *      8390, 8380
 * Note:
 *      Valid index is 0~15.
 */
int32
rtk_port_vlanBasedIsolationEntry_set(uint32 unit, uint32 index, rtk_port_vlanIsolationEntry_t *pEntry)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    rtk_port_vlanIsolationEntry_t   temp_entry;

    temp_entry = *pEntry;
    temp_entry.portmask = RTK_PORTMASK_TO_PHYSICAL_PORTMASK(unit, (*pEntry).portmask);
    return RT_MAPPER(unit)->port_vlanBasedIsolationEntry_set(unit, index, &temp_entry);
    }
#else
    return RT_MAPPER(unit)->port_vlanBasedIsolationEntry_set(unit, index, pEntry);
#endif
} /* end of rtk_port_vlanBasedIsolationEntry_set */

/* Function Name:
 *      rtk_port_vlanBasedIsolation_vlanSource_get
 * Description:
 *      Get VLAN source of VLAN-based port isolation
 * Input:
 *      unit     - unit id
 * Output:
 *      pVlanSrc - pointer to vlan source
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_port_vlanBasedIsolation_vlanSource_get(uint32 unit, rtk_port_vlanIsolationSrc_t *pVlanSrc)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_vlanBasedIsolation_vlanSource_get(unit, pVlanSrc);
} /* end of rtk_port_vlanBasedIsolation_vlanSource_get */

/* Function Name:
 *      rtk_port_vlanBasedIsolation_vlanSource_set
 * Description:
 *      Set VLAN source of VLAN-based port isolation
 * Input:
 *      unit    - unit id
 *      vlanSrc - vlan source
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_port_vlanBasedIsolation_vlanSource_set(uint32 unit, rtk_port_vlanIsolationSrc_t vlanSrc)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_vlanBasedIsolation_vlanSource_set(unit, vlanSrc);
} /* end of rtk_port_vlanBasedIsolation_vlanSource_set */

/* Function Name:
 *      rtk_port_gigaLiteEnable_get
 * Description:
 *      Get the statue of Giga Lite of the specific port in the specific unit
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to status of Giga Lite
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_port_gigaLiteEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
	return RT_ERR_CHIP_NOT_SUPPORTED;

} /* end of rtk_port_gigaLiteEnable_get */


/* Function Name:
 *      rtk_port_gigaLiteEnable_set
 * Description:
 *      Set the statue of Giga Lite of the specific port in the specific unit
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of Giga Lite
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - input parameter out of range
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_port_gigaLiteEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
	return RT_ERR_CHIP_NOT_SUPPORTED;

} /* end of rtk_port_gigaLiteEnable_set */

/* Function Name:
 *      rtk_port_phyReconfig_register
 * Description:
 *      Register callback function for PHY need to reconfigure notification
 * Input:
 *       phyNotification_callback    - callback function for reconfigure notification
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_port_phyReconfig_register(rtk_port_phyReconfig_callback_t phyNotification_callback)
{
#if !defined(__MODEL_USER__)
    return dal_waMon_phyReconfig_register(phyNotification_callback);
#endif
    return 0;
} /* end of rtk_port_linkMon_register */

/* Function Name:
 *      rtk_port_phyReconfig_unregister
 * Description:
 *      UnRegister callback function for PHY need to reconfigure notification
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_port_phyReconfig_unregister(void)
{
#if !defined(__MODEL_USER__)
    return dal_waMon_phyReconfig_unregister();
#endif
    return 0;
} /* end of rtk_port_linkMon_unregister */

/* Function Name:
 *      rtk_port_downSpeedEnable_get
 * Description:
 *      Get down speed status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pEnable - down speed status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_port_downSpeedEnable_get(uint32 unit, rtk_port_t port,
    rtk_enable_t *pEnable)
{
    /* function body */
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    if (NULL == RT_MAPPER(unit)->port_downSpeedEnable_get)
        return RT_ERR_DRIVER_NOT_FOUND;
    return RT_MAPPER(unit)->port_downSpeedEnable_get(unit, port, pEnable);
}   /* end of rtk_port_downSpeedEnable_get */

/* Function Name:
 *      rtk_port_downSpeedEnable_set
 * Description:
 *      Set down speed status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - down speed status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_port_downSpeedEnable_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable)
{
    /* function body */
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    if (NULL == RT_MAPPER(unit)->port_downSpeedEnable_set)
        return RT_ERR_DRIVER_NOT_FOUND;
    return RT_MAPPER(unit)->port_downSpeedEnable_set(unit, port, enable);
}   /* end of rtk_port_downSpeedEnable_set */


/* Function Name:
 *      rtk_port_fiberDownSpeedEnable_get
 * Description:
 *      Get fiber down speed status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pEnable - fiber down speed status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
rtk_port_fiberDownSpeedEnable_get(uint32 unit, rtk_port_t port,
    rtk_enable_t *pEnable)
{
    /* function body */
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    if (NULL == RT_MAPPER(unit)->port_fiberDownSpeedEnable_get)
        return RT_ERR_DRIVER_NOT_FOUND;
    return RT_MAPPER(unit)->port_fiberDownSpeedEnable_get(unit, port, pEnable);
}   /* end of rtk_port_fiberDownSpeedEnable_get */

/* Function Name:
 *      rtk_port_fiberDownSpeedEnable_set
 * Description:
 *      Set fiber down speed status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - fiber down speed status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
rtk_port_fiberDownSpeedEnable_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable)
{
    /* function body */
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    if (NULL == RT_MAPPER(unit)->port_fiberDownSpeedEnable_set)
        return RT_ERR_DRIVER_NOT_FOUND;
    return RT_MAPPER(unit)->port_fiberDownSpeedEnable_set(unit, port, enable);
}   /* end of rtk_port_fiberDownSpeedEnable_set */

/* Function Name:
 *      rtk_port_fiberNwayForceLinkEnable_get
 * Description:
 *      When fiber port is configured N-way,
 *      which can link with link partner is configured force mode.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pEnable - fiber Nway force links status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
rtk_port_fiberNwayForceLinkEnable_get(uint32 unit, rtk_port_t port,
    rtk_enable_t *pEnable)
{
    /* function body */
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    if (NULL == RT_MAPPER(unit)->port_fiberNwayForceLinkEnable_get)
        return RT_ERR_DRIVER_NOT_FOUND;
    return RT_MAPPER(unit)->port_fiberNwayForceLinkEnable_get(unit, port, pEnable);
}   /* end of rtk_port_fiberNwayForceLinkEnable_get */

/* Function Name:
 *      rtk_port_fiberNwayForceLinkEnable_set
 * Description:
 *      When fiber port is configured N-way,
 *      which can link with link partner is configured force mode.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - fiber Nway force links status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
rtk_port_fiberNwayForceLinkEnable_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable)
{
    /* function body */
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    if (NULL == RT_MAPPER(unit)->port_fiberNwayForceLinkEnable_set)
        return RT_ERR_DRIVER_NOT_FOUND;
    return RT_MAPPER(unit)->port_fiberNwayForceLinkEnable_set(unit, port, enable);
}   /* end of rtk_port_fiberNwayForceLinkEnable_set */


/* Function Name:
 *      rtk_port_fiberOAMLoopBackEnable_set
 * Description:
 *     Set Fiber-port OAM Loopback enable or not,
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - fiber oam loopback enable or not
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Applicable:
 *      8380,8390
 * Note:
 *      None
 */
int32
rtk_port_fiberOAMLoopBackEnable_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable)
{
    /* function body */
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    if (NULL == RT_MAPPER(unit)->port_fiberOAMLoopBackEnable_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    return RT_MAPPER(unit)->port_fiberOAMLoopBackEnable_set(unit, port, enable);

}   /* end of rtk_port_fiberOAMLoopBackEnable_set */

/* Function Name:
 *      rtk_port_fiberInternalLoopBackEnable_set
 * Description:
 *     Set Fiber-port Internal Loopback enable or not,
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - fiber internal loopback enable or not
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Applicable:
 *      8380,8390
 * Note:
 *      None
 */
int32
rtk_port_fiberInternalLoopBackEnable_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable)
{
    /* function body */
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    if (NULL == RT_MAPPER(unit)->port_fiberInternalLoopBackEnable_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    return RT_MAPPER(unit)->port_fiberInternalLoopBackEnable_set(unit, port, enable);

}   /* end of rtk_port_fiberInternalLoopBackEnable_set */


/* Function Name:
 *      rtk_port_10gMedia_set
 * Description:
 *      Set 10G port media of the specific port
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      media - port media
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8390
 * Note:
 *      (1) The media value is as following:
 *          - PORT_10GMEDIA_FIBER_10G,
 *          - PORT_10GMEDIA_FIBER_1G,
 *          - PORT_10GMEDIA_DAC_50CM,
 *          - PORT_10GMEDIA_DAC_100CM,
 *          - PORT_10GMEDIA_DAC_300CM,
 */
int32
rtk_port_10gMedia_set(uint32 unit, rtk_port_t port, rtk_port_10gMedia_t media)
{
    /* function body */
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    if (NULL == RT_MAPPER(unit)->port_10gMedia_set)
        return RT_ERR_DRIVER_NOT_FOUND;
    return RT_MAPPER(unit)->port_10gMedia_set(unit, port, media);
}   /* end of rtk_port_10gMedia_set */

/* Function Name:
 *      rtk_port_10gMedia_get
 * Description:
 *      Get 10G port media of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      media   - pointer to the media type
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      (1) The media type of the port is as following:
 *          - PORT_10GMEDIA_FIBER_10G,
 *          - PORT_10GMEDIA_FIBER_1G,
 *          - PORT_10GMEDIA_DAC_50CM,
 *          - PORT_10GMEDIA_DAC_100CM,
 *          - PORT_10GMEDIA_DAC_300CM,
 */
int32
rtk_port_10gMedia_get(uint32 unit, rtk_port_t port, rtk_port_10gMedia_t *media)
{
    /* function body */
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    if (NULL == RT_MAPPER(unit)->port_10gMedia_get)
        return RT_ERR_DRIVER_NOT_FOUND;
    return RT_MAPPER(unit)->port_10gMedia_get(unit, port, media);
}   /* end of rtk_port_10gMedia_get */

/* Function Name:
 *      rtk_port_10gSds_restart
 * Description:
 *      Restart 10G port serdes
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
rtk_port_10gSds_restart(uint32 unit, rtk_port_t port)
{
    /* function body */
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    if (NULL == RT_MAPPER(unit)->port_10gSds_restart)
        return RT_ERR_DRIVER_NOT_FOUND;
    return RT_MAPPER(unit)->port_10gSds_restart(unit, port);
}   /* end of rtk_port_10gSds_restart */

/* Function Name:
 *      rtk_port_10g_init
 * Description:
 *      Init 10G port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
rtk_port_10g_init(uint32 unit, rtk_port_t port)
{
    /* function body */
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    if (NULL == RT_MAPPER(unit)->port_10g_init)
        return RT_ERR_DRIVER_NOT_FOUND;
    return RT_MAPPER(unit)->port_10g_init(unit, port);
}   /* end of rtk_port_10g_init */

/* Function Name:
 *      rtk_port_phyFiberTxDis_set
 * Description:
 *      Set PHY fiber Tx disable signal
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      enable  - ENABLED: Enable Tx disable signal;
 *                DISABLED: Disable Tx disable signal.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Applicable:
 *      8380, 8390
 * Note:
 *      None
 */
int32
rtk_port_phyFiberTxDis_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    /* function body */
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_phyFiberTxDis_set(unit, port, enable);
}   /* end of rtk_port_phyFiberTxDis_set */

/* Function Name:
 *      rtk_port_phyFiberTxDisPin_set
 * Description:
 *      Set PHY fiber Tx disable signal GPO output
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      data      - GPO pin value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Applicable:
 *      8380, 8390
 * Note:
 *      None
 */
int32
rtk_port_phyFiberTxDisPin_set(uint32 unit, rtk_port_t port, uint32 data)
{
    /* function body */
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->port_phyFiberTxDisPin_set(unit, port, data);
}   /* end of rtk_port_phyFiberTxDisPin_set */

