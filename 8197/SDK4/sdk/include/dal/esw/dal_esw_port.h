/*
 * Copyright(c) Realtek Semiconductor Corporation, 2009
 * All rights reserved.
 *
 * $Revision: 30053 $
 * $Date: 2012-06-19 14:12:07 +0800 (Tue, 19 Jun 2012) $
 *
 * Purpose : Definition those public PORT APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) Parameter settings for the port-based view
 *           2) RTCT
 *           3) UDLD
 *           4) RLDP
 */


#ifndef __DAL_ESW_PORT_H__
#define __DAL_ESW_PORT_H__


/*
 * Include Files
 */
#include <common/rt_autoconf.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <dal/dal_common.h>
#include <rtk/port.h>


/*
 * Function Declaration
 */

/* Function Name:
 *      dal_esw_port_init
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
 * Note:
 *      1. Module must be initialized before using all of APIs in this module
 */
extern int32
dal_esw_port_init(uint32 unit);

/* Module Name    : Port                                       */
/* Sub-module Name: Parameter settings for the port-based view */

/* Function Name:
 *      dal_esw_port_link_get
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
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The link status of the port is as following:
 *      - LINKDOWN
 *      - LINKUP
 */
extern int32
dal_esw_port_link_get(uint32 unit, rtk_port_t port, rtk_port_linkStatus_t *pStatus);

/* Function Name:
 *      dal_esw_port_linkMedia_get
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
 * Note:
 *      (1) The link status of the port is as following:
 *          - PORT_LINKDOWN
 *          - PORT_LINKUP
 *      (2) The media type of the port is as following:
 *          - PORT_MEDIA_COPPER
 *          - PORT_MEDIA_FIBER
 *      (3) When the link status is link-down, the return media should be ignored.
 */
extern int32
dal_esw_port_linkMedia_get(uint32 unit, rtk_port_t port, rtk_port_linkStatus_t *pStatus, rtk_port_media_t *pMedia);

/* Function Name:
 *      dal_esw_port_speedDuplex_get
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
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 *      RT_ERR_PORT_LINKDOWN - link down port status
 * Note:
 *      1. The speed type of the port is as following:
 *         - PORT_SPEED_10M
 *         - PORT_SPEED_100M
 *         - PORT_SPEED_1000M
 *
 *      2. The duplex mode of the port is as following:
 *         - HALF_DUPLEX
 *         - FULL_DUPLEX
 */
extern int32
dal_esw_port_speedDuplex_get(
    uint32            unit,
    rtk_port_t        port,
    rtk_port_speed_t  *pSpeed,
    rtk_port_duplex_t *pDuplex);

/* Function Name:
 *      dal_esw_port_flowctrl_get
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
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 *      RT_ERR_PORT_LINKDOWN - link down port status
 * Note:
 *      None
 */
extern int32
dal_esw_port_flowctrl_get(
    uint32      unit,
    rtk_port_t  port,
    uint32      *pTxStatus,
    uint32      *pRxStatus);

/* Function Name:
 *      dal_esw_port_phyAutoNegoEnable_get
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
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_port_phyAutoNegoEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_port_phyAutoNegoEnable_set
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
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - input parameter out of range
 * Note:
 *      1. ENABLED : switch to PHY auto negotiation mode
 *         DISABLED: switch to PHY force mode
 *      2. Once the abilities of both auto-nego and force mode are set,
 *         you can freely swtich the mode without calling ability setting API again
 */
extern int32
dal_esw_port_phyAutoNegoEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_port_phyAutoNegoAbility_get
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
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_port_phyAutoNegoAbility_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_port_phy_ability_t  *pAbility);

/* Function Name:
 *      dal_esw_port_phyAutoNegoAbility_set
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
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. You can set these abilities no matter which mode PHY currently stays on
 */
extern int32
dal_esw_port_phyAutoNegoAbility_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_port_phy_ability_t  *pAbility);

/* Function Name:
 *      dal_esw_port_phyForceModeAbility_get
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
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_port_phyForceModeAbility_get(
    uint32              unit,
    rtk_port_t          port,
    rtk_port_speed_t    *pSpeed,
    rtk_port_duplex_t   *pDuplex,
    rtk_enable_t        *pFlowControl);

/* Function Name:
 *      dal_esw_port_phyForceModeAbility_set
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
 *      RT_ERR_PORT_ID    - invalid port id
 *      RT_ERR_PHY_SPEED  - invalid PHY speed setting
 *      RT_ERR_PHY_DUPLEX - invalid PHY duplex setting
 *      RT_ERR_INPUT      - invalid input parameter
 * Note:
 *      1. You can set these abilities no matter which mode PHY currently stays on
 *
 *      2. The speed type of the port is as following:
 *         - PORT_SPEED_10M
 *         - PORT_SPEED_100M
 *
 *      3. The duplex mode of the port is as following:
 *         - HALF_DUPLEX
 *         - FULL_DUPLEX
 */
extern int32
dal_esw_port_phyForceModeAbility_set(
    uint32              unit,
    rtk_port_t          port,
    rtk_port_speed_t    speed,
    rtk_port_duplex_t   duplex,
    rtk_enable_t        flowControl);

/* Function Name:
 *      dal_esw_port_phyReg_get
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
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid page id
 *      RT_ERR_PHY_REG_ID   - invalid reg id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_port_phyReg_get(
    uint32              unit,
    rtk_port_t          port,
    uint32              page,
    rtk_port_phy_reg_t  reg,
    uint32              *pData);

/* Function Name:
 *      dal_esw_port_phyReg_set
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
 *      RT_ERR_PORT_ID     - invalid port id
 *      RT_ERR_PHY_PAGE_ID - invalid page id
 *      RT_ERR_PHY_REG_ID  - invalid reg id
 * Note:
 *      None
 */
extern int32
dal_esw_port_phyReg_set(
    uint32              unit,
    rtk_port_t          port,
    uint32              page,
    rtk_port_phy_reg_t  reg,
    uint32              data);

/* Function Name:
 *      dal_esw_port_cpuPortId_get
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
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_port_cpuPortId_get(uint32 unit, rtk_port_t *pPort);

/* 8389M+8328M */
/* Function Name:
 *      dal_esw_port_isolation_get
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
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. Default value of each port is 1
 *      2. Enable port isolation in the certain ports if relative portmask bits are set to 1
 */
extern int32
dal_esw_port_isolation_get(uint32 unit, rtk_port_t port, rtk_portmask_t *pPortmask);

/* 8389M+8328M */
/* Function Name:
 *      dal_esw_port_isolation_set
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
 *      RT_ERR_PORT_ID   - invalid port id
 *      RT_ERR_PORT_MASK - invalid port mask
 * Note:
 *      1. Default value of each port is 1
 *      2. Enable port isolation in the certain ports if relative portmask bits are set to 1
 */
extern int32
dal_esw_port_isolation_set(uint32 unit, rtk_port_t port, rtk_portmask_t portmask);

/* Function Name:
 *      dal_esw_port_isolation_add
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
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      1. Default value of each port is 1
 *      2. Port and iso_port will be isolated when this API is called
 *      3. The iso_port to the relative portmask bit will be set to 1
 */
extern int32
dal_esw_port_isolation_add(uint32 unit, rtk_port_t port, rtk_port_t iso_port);

/* Function Name:
 *      dal_esw_port_isolation_del
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
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      1. Default value of each port is 1
 *      2. Isolated status between the port and the iso_port is removed when this API is called
 *      3. The iso_port to the relative portmask bit will be set to 0
 */
extern int32
dal_esw_port_isolation_del(uint32 unit, rtk_port_t port, rtk_port_t iso_port);

/* Function Name:
 *      dal_esw_port_phyComboPortMedia_get
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
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_port_phyComboPortMedia_get(uint32 unit, rtk_port_t port, rtk_port_media_t *pMedia);

/* Function Name:
 *      dal_esw_port_phyComboPortMedia_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. You can set these port media which mode PHY currently stays on
 */
extern int32
dal_esw_port_phyComboPortMedia_set(uint32 unit, rtk_port_t port, rtk_port_media_t media);

/* 8389M+8328M */
/* Function Name:
 *      dal_esw_port_backpressureEnable_get
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      1. The mac local loopback enable status of the port is as following:
 *         - DISABLE
 *         - ENABLE
 *      2. Used to support backpressure in half mode.
 */
extern int32
dal_esw_port_backpressureEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* 8389M+8328M */
/* Function Name:
 *      dal_esw_port_backpressureEnable_set
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
 * Note:
 *      1. The mac local loopback enable status of the port is as following:
 *         - DISABLED
 *         - ENABLED
 *      2. Used to support backpressure in half mode.
 */
extern int32
dal_esw_port_backpressureEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_port_linkChange_register
 * Description:
 *      Register callback function for notification of link change 
 * Input:
 *      unit           - unit id
 *      link_change_callback      - Callback function for link change
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 */
extern int32
dal_esw_port_linkChange_register(uint32 unit, dal_link_change_callback_f link_change_callback);

/* Function Name:
 *      dal_esw_port_linkChange_unregister
 * Description:
 *      Unregister callback function for notification of link change 
 * Input:
 *      unit           - unit id
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 * Note:
 */
extern int32
dal_esw_port_linkChange_unregister(uint32 unit);

/* 8389M+8328M */
/* Function Name:
 *      dal_esw_port_adminEnable_get
 * Description:
 *      Get port admin status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      pEnable - pointer to the port admin status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_port_adminEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* 8389M+8328M */
/* Function Name:
 *      dal_esw_port_adminEnable_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_port_adminEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_port_txEnable_get
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
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      The TX enable status of the port is as following:
 *      - DISABLED
 *      - ENABLED
 */
extern int32
dal_esw_port_txEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_port_txEnable_set
 * Description:
 *      Set the TX enable status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - MAC TX configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      The TX enable status of the port is as following:
 *      - DISABLED
 *      - ENABLED
 */
extern int32
dal_esw_port_txEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_port_rxEnable_get
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
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      The RX enable status of the port is as following:
 *      - DISABLED
 *      - ENABLED
 */
extern int32
dal_esw_port_rxEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_port_rxEnable_set
 * Description:
 *      Set the RX enable status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - port admin configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      The RX enable status of the port is as following:
 *      - DISABLED
 *      - ENABLED
 */
extern int32
dal_esw_port_rxEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Module Name    : Port */
/* Sub-module Name: UDLD */

/* Function Name:
 *      dal_esw_port_udldEnable_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_port_udldEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_port_udldEnable_set
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_port_udldEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_port_udldLinkUpAutoTriggerEnable_get
 * Description:
 *      Get enable status of link up auto trigger UDLD test.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of link up auto trigger UDLD
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_port_udldLinkUpAutoTriggerEnable_get(uint32 unit, rtk_enable_t *pEnable);

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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_port_udldLinkUpAutoTriggerEnable_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_port_udldTrigger_start
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 * Note:
 *      None
 */
extern int32
dal_esw_port_udldTrigger_start(uint32 unit, rtk_port_t port);

/* Function Name:
 *      dal_esw_port_udldAutoDisableFailedPortEnable_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      UDLD test status is as following
 *      - UDLD_UNIDIR           UDLD test result is unidirection
 *      - UDLD_BIDIR            UDLD test result is bidirection
 *      - UDLD_UNDETERMINE      UDLD test is on going. Wait more time to get result.
 */
extern int32
dal_esw_port_udldStatus_get(uint32 unit, rtk_port_t port, rtk_port_udldStatus_t *pStatus);

/* Function Name:
 *      dal_esw_port_udldAutoDisableFailedPortEnable_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_port_udldAutoDisableFailedPortEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_port_udldAutoDisableFailedPortEnable_set
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_port_udldAutoDisableFailedPortEnable_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_port_udldInterval_get
 * Description:
 *      Get UDLD echo generate interval.
 * Input:
 *      unit      - unit id
 * Output:
 *      pInterval - pointer to interval of UDLD echo generate (unit: second)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Following value is available on 8328M:
 *      2 second, 4 second, 8 second, 16 second
 */
extern int32
dal_esw_port_udldInterval_get(uint32 unit, rtk_port_udldInterval_t *pInterval);

/* Function Name:
 *      dal_esw_port_udldInterval_set
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      Following value is available on 8328M:
 *      2 second, 4 second, 8 second, 16 second
 */
extern int32
dal_esw_port_udldInterval_set(uint32 unit, rtk_port_udldInterval_t interval);

/* Function Name:
 *      dal_esw_port_udldRetryCount_get
 * Description:
 *      Get retry count of UDLD.
 * Input:
 *      unit        - unit id
 * Output:
 *      pRetryCount - pointer to retry count of UDLD
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Following retryCount is available in 8328M:
 *      1, 2, 4, 8
 */
extern int32
dal_esw_port_udldRetryCount_get(uint32 unit, uint32 *pRetryCount);

/* Function Name:
 *      dal_esw_port_udldRetryCount_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Following retryCount is available in 8328M:
 *      1, 2, 4, 8
 */
extern int32
dal_esw_port_udldRetryCount_set(uint32 unit, uint32 retryCount);

/* Function Name:
 *      dal_esw_port_udldLedIndicateEnable_get
 * Description:
 *      Get enable status of LED indication when unidirectional link detection.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of LED indication
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_port_udldLedIndicateEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_port_udldLedIndicateEnable_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_port_udldLedIndicateEnable_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_port_udldEchoAction_get
 * Description:
 *      Get udld echo packet action.
 * Input:
 *      unit   - unit id
 * Output:
 *      pAction - echo packet action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_port_udldEchoAction_get(uint32 unit, rtk_port_udldEchoAction_t *pAction);


/* Function Name:
 *      dal_esw_port_udldEchoAction_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_port_udldEchoAction_set(uint32 unit, rtk_port_udldEchoAction_t action);

/* Function Name:
 *      dal_esw_port_udldLinkStatus_get
 * Description:
 *      Get udld port link status.
 * Input:
 *      unit   - unit id
 *      port  - port id
 * Output:
 *      pStatus - link status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_port_udldLinkStatus_get(uint32 unit, rtk_port_t port, rtk_port_udldLinkStatus_t *pStatus);

/* Function Name:
 *      dal_esw_port_udldLinkStatus_set
 * Description:
 *      Set udld link status.
 * Input:
 *      unit   - unit id
 *      port  - port id
 *      status - link status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_port_udldLinkStatus_set(uint32 unit, rtk_port_t port, rtk_port_udldLinkStatus_t status);


/* Module Name    : Port */
/* Sub-module Name: RLDP */

 /* Function Name:
 *      dal_esw_port_rldpEnable_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_port_rldpEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);
 
 /* Function Name:
 *      dal_esw_port_rldpEnable_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_port_rldpEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_port_rldpStatus_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Result of RLDP self loop is as following
 *      - RLDP_FORWARDING
 *      - RLDP_LISTEN
 *      - RLDP_BLOCK
 *
 *      Result of RLDP normal loop is as following
 *      - RLDP_NORMAL_LOOP
 *      - RLDP_NO_NORMAL_LOOP
 */
extern int32
dal_esw_port_rldpStatus_get(
    uint32                          unit, 
    rtk_port_t                      port, 
    rtk_port_rldpNormalStatus_t     *pNormalStatus, 
    rtk_port_rldpSelfStatus_t       *pSelfStatus); 

/* Function Name:
 *      dal_esw_port_rldpAutoBlockEnable_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_port_rldpAutoBlockEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);
   
/* Function Name:
 *      dal_esw_port_rldpAutoBlockEnable_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_port_rldpAutoBlockEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_port_rldpInterval_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Following is available interval for 8328M
 *      1, 2, 4, 8 second
 */
extern int32
dal_esw_port_rldpInterval_get(uint32 unit, rtk_port_t port, uint32 *pInterval);

/* Function Name:
 *      dal_esw_port_rldpInterval_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Following is available interval for 8328M
 *      1, 2, 4, 8 second
 */
extern int32
dal_esw_port_rldpInterval_set(uint32 unit, rtk_port_t port, uint32 interval);

/* Function Name:
 *      dal_esw_port_rldpSelfLoopAgingTime_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Following is available aging time for 8328M
 *      1, 2, 4, 8 second
 */
extern int32
dal_esw_port_rldpSelfLoopAgingTime_get(uint32 unit, rtk_port_t port, uint32 *pAgingTime);

/* Function Name:
 *      dal_esw_port_rldpSelfLoopAgingTime_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Following is available aging time for 8328M
 *      1, 2, 4, 8 second
 */
extern int32
dal_esw_port_rldpSelfLoopAgingTime_set(uint32 unit, rtk_port_t port, uint32 agingTime);

/* Function Name:
 *      dal_esw_port_rldpNormalLoopAgingTime_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Following is available aging time for 8328M
 *      1, 2, 4, 8 second
 */
extern int32
dal_esw_port_rldpNormalLoopAgingTime_get(uint32 unit, rtk_port_t port, uint32 *pAgingTime);

/* Function Name:
 *      dal_esw_port_rldpNormalLoopAgingTime_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Following is available aging time for 8328M
 *      1, 2, 4, 8 second
 */
extern int32
dal_esw_port_rldpNormalLoopAgingTime_set(uint32 unit, rtk_port_t port, uint32 agingTime);

/* Function Name:
 *      dal_esw_port_greenEnable_get
 * Description:
 *      Get the statue of green feature of the specific port in the specific unit
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pEnable - pointer to status of green feature
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_port_greenEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_port_greenEnable_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      None
 */
extern int32
dal_esw_port_greenEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_port_linkdownPowerSaving_workaround
 * Description:
 *      Workaround for link down power saving problem of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is workaround for patch the link down power saving problem
 */
extern int32
dal_esw_port_linkdownPowerSaving_workaround(uint32 unit);

/* Function Name:
 *      dal_esw_port_backpressure_workaround
 * Description:
 *      Workaround for back pressure problem of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is workaround for patch the back pressure problem
 */
extern int32
dal_esw_port_backpressure_workaround(uint32 unit);

/* Function Name:
 *      dal_esw_port_intraLinkDelay_workaround
 * Description:
 *      Workaround for intraLink delay problem of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is workaround for patch the intraLink delay problem
 */
extern int32
dal_esw_port_intraLinkDelay_workaround(uint32 unit);

/* Function Name:
 *      dal_esw_port_phyCrossOverMode_get
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
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Following value is valid
 *      - PORT_CROSSOVER_MODE_AUTO
 *      - PORT_CROSSOVER_MODE_MDI
 *      - PORT_CROSSOVER_MODE_MDIX
 */
extern int32
dal_esw_port_phyCrossOverMode_get(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t *pMode);

/* Function Name:
 *      dal_esw_port_phyCrossOverMode_set
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
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      Following value is valid
 *      - PORT_CROSSOVER_MODE_AUTO
 *      - PORT_CROSSOVER_MODE_MDI
 *      - PORT_CROSSOVER_MODE_MDIX
 */
extern int32
dal_esw_port_phyCrossOverMode_set(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t mode);

/* Function Name:
 *      dal_esw_port_flowCtrlEnable_get
 * Description:
 *      Get the flow control status of the specific port
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pEnable - pointer to the status of the flow control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      The API get the flow control status by port based, no matter N-WAY is enabled or disabled.
 */
extern int32
dal_esw_port_flowCtrlEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_port_flowCtrlEnable_set
 * Description:
 *      Set the flow control status to the specific port
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      enable    - enable status of flow control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      The API is apply the flow control status by port based, no matter N-WAY is enabled or disabled.
 */
extern int32
dal_esw_port_flowCtrlEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_port_phyComboPortFiberMedia_get
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
 * Note:
 *      1. fiber media type value is as following:
 *      - PORT_FIBER_MEDIA_1000
 *      - PORT_FIBER_MEDIA_100
 *      - PORT_FIBER_MEDIA_AUTO
 */
extern int32
dal_esw_port_phyComboPortFiberMedia_get(uint32 unit, rtk_port_t port, rtk_port_fiber_media_t *pMedia);

/* Function Name:
 *      dal_esw_port_phyComboPortFiberMedia_set
 * Description:
 *      Set PHY port fiber media of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      pMedia - pointer to the port fiber media
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. fiber media type value is as following:
 *      - PORT_FIBER_MEDIA_1000
 *      - PORT_FIBER_MEDIA_100
 *      - PORT_FIBER_MEDIA_AUTO
 */
extern int32
dal_esw_port_phyComboPortFiberMedia_set(uint32 unit, rtk_port_t port, rtk_port_fiber_media_t media);

#if defined(CONFIG_SDK_WA_88E6063_COMPATIBLE)
/* Function Name:
 *      dal_esw_port_FeNway10MCompatible88E6063_workaround
 * Description:
 *      Workaround for link up issue with Marvell 88E6063 when connect in Nway 10M of the specified device.
 * Input:
 *      unit - unit id
 *      port - port id
 *      linkState - currently new link state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      1) The API is called by ISR and don't need to lock semaphore.
 *      2) The function is called when link change is happened and linkState is means the new state.
 */
extern int32 
dal_esw_port_FeNway10MCompatible88E6063_workaround(uint32 unit, rtk_port_t port, rtk_port_linkStatus_t linkState);

#endif /* #if defined(CONFIG_SDK_WA_88E6063_COMPATIBLE) */

/* Function Name:
 *      dal_esw_port_linkDownPowerSavingEnable_get
 * Description:
 *      Get the statue of link-down power saving of the specific port in the specific unit
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to status of link-down power saving
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_port_linkDownPowerSavingEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_port_linkDownPowerSavingEnable_set
 * Description:
 *      Set the statue of link-down power saving of the specific port in the specific unit
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of link-down power saving
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - input parameter out of range
 * Note:
 *      None
 */
extern int32
dal_esw_port_linkDownPowerSavingEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

#endif /* __DAL_ESW_PORT_H__ */
