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
 * Purpose : Definition those public Port APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) Port
 *
 */

#ifndef __DAL_CYPRESS_PORT_H__
#define __DAL_CYPRESS_PORT_H__

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <dal/dal_common.h>
#include <rtk/port.h>



/*
 * Data Declaration
 */

/*
 * Macro Definition
 */


/*
 * Function Declaration
 */

/* Function Name:
 *      dal_cypress_port_init
 * Description:
 *      Initialize port module of the specified device.
 * Input:
 *      unit          - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      1. Module must be initialized before using all of APIs in this module
 */
extern int32
dal_cypress_port_init(uint32 unit);

/* Function Name:
 *      dal_cypress_port_link_get
 * Description:
 *      Get the link status of the specific port
 * Input:
 *      unit                  - unit id
 *      port                  - port id
 * Output:
 *      pStatus              - pointer to the link status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID         - invalid port id
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 * Note:
 *      The link status of the port is as following:
 *      - LINKDOWN
 *      - LINKUP
 */
extern int32
dal_cypress_port_link_get(uint32 unit, rtk_port_t port, rtk_port_linkStatus_t *pStatus);

/* Function Name:
 *      dal_cypress_port_txEnable_set
 * Description:
 *      Set the TX enable status of the specific port
 * Input:
 *      unit           - unit id
 *      port           - port id
 *      enable        - enable status of TX
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      The TX enable status of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
extern int32
dal_cypress_port_txEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_cypress_port_rxEnable_set
 * Description:
 *      Set the RX enable status of the specific port
 * Input:
 *      unit            - unit id
 *      port            - port id
 *      enable         - enable status of RX
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID   - invalid port id
 *      RT_ERR_INPUT     - invalid input parameter
 * Note:
 *      The RX enable status of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
extern int32
dal_cypress_port_rxEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_cypress_port_txEnable_get
 * Description:
 *      Get the TX enable status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to the port TX status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The TX enable status of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
extern int32
dal_cypress_port_txEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_cypress_port_rxEnable_get
 * Description:
 *      Get the RX enable status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to the port RX status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The RX enable status of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
extern int32
dal_cypress_port_rxEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_cypress_port_specialCongest_set
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
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_cypress_port_specialCongest_set(uint32 unit, rtk_port_t port, uint32 second);

/* Function Name:
 *      dal_cypress_port_speedDuplex_get
 * Description:
 *      Get the negotiated port speed and duplex status of the specific port
 * Input:
 *      unit                  - unit id
 *      port                  - port id
 * Output:
 *      pSpeed               - pointer to the port speed
 *      pDuplex              - pointer to the port duplex
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID         - invalid port id
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 *      RT_ERR_PORT_LINKDOWN   - link down port status
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
dal_cypress_port_speedDuplex_get(
    uint32            unit,
    rtk_port_t        port,
    rtk_port_speed_t  *pSpeed,
    rtk_port_duplex_t *pDuplex);

/* Function Name:
 *      dal_cypress_port_flowctrl_get
 * Description:
 *      Get the negotiated flow control status of the specific port
 * Input:
 *      unit         - unit id
 *      port         - port id
 * Output:
 *      pTxStatus     - pointer to the negotiation result of the Tx flow control
 *      pRxStatus     - pointer to the negotiation result of the Rx flow control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID         - invalid port id
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 *      RT_ERR_PORT_LINKDOWN   - link down port status
 * Note:
 *      None
 */
int32
dal_cypress_port_flowctrl_get(
    uint32            unit,
    rtk_port_t        port,
    uint32            *pTxStatus,
    uint32            *pRxStatus);

/* Function Name:
 *      dal_cypress_port_phyAutoNegoEnable_get
 * Description:
 *      Get PHY ability of the specific port
 * Input:
 *      unit                 - unit id
 *      port                 - port id
 * Output:
 *      pEnable             - pointer to PHY auto negotiation status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID        - invalid port id
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_port_phyAutoNegoEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_cypress_port_phyAutoNegoEnable_set
 * Description:
 *      Set PHY ability of the specific port
 * Input:
 *      unit                 - unit id
 *      port                 - port id
 *      enable               - enable PHY auto negotiation
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID        - invalid port id
 *      RT_ERR_INPUT          - input parameter out of range
 * Note:
 *      1. ENABLED : switch to PHY auto negotiation mode
 *         DISABLED: switch to PHY force mode
 *      2. Once the abilities of both auto-nego and force mode are set,
 *         you can freely swtich the mode without calling ability setting API again
 */
extern int32
dal_cypress_port_phyAutoNegoEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_cypress_port_phyAutoNegoAbility_get
 * Description:
 *      Get PHY auto negotiation ability of the specific port
 * Input:
 *      unit                 - unit id
 *      port                 - port id
 * Output:
 *      pAbility            - pointer to the PHY ability
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID        - invalid port id
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_port_phyAutoNegoAbility_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_port_phy_ability_t  *pAbility);

/* Function Name:
 *      dal_cypress_port_phyAutoNegoAbility_set
 * Description:
 *      Set PHY auto negotiation ability of the specific port
 * Input:
 *      unit               - unit id
 *      port               - port id
 *      pAbility          - pointer to the PHY ability
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. You can set these abilities no matter which mode PHY currently stays on
 */
extern int32
dal_cypress_port_phyAutoNegoAbility_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_port_phy_ability_t  *pAbility);

/* Function Name:
 *      dal_cypress_port_phyForceModeAbility_get
 * Description:
 *      Get PHY ability status of the specific port
 * Input:
 *      unit                 - unit id
 *      port                 - port id
 * Output:
 *      pSpeed              - pointer to the port speed
 *      pDuplex             - pointer to the port duplex
 *      pFlowControl        - pointer to the flow control enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID        - invalid port id
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_port_phyForceModeAbility_get(
    uint32              unit,
    rtk_port_t          port,
    rtk_port_speed_t    *pSpeed,
    rtk_port_duplex_t   *pDuplex,
    rtk_enable_t        *pFlowControl);

/* Function Name:
 *      dal_cypress_port_phyForceModeAbility_set
 * Description:
 *      Set the port speed/duplex mode/pause/asy_pause in the PHY force mode
 * Input:
 *      unit                  - unit id
 *      port                  - port id
 *      speed                 - port speed
 *      duplex                - port duplex mode
 *      flowControl           - enable flow control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID         - invalid port id
 *      RT_ERR_PHY_SPEED       - invalid PHY speed setting
 *      RT_ERR_PHY_DUPLEX      - invalid PHY duplex setting
 *      RT_ERR_INPUT           - invalid input parameter
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
dal_cypress_port_phyForceModeAbility_set(
    uint32              unit,
    rtk_port_t          port,
    rtk_port_speed_t    speed,
    rtk_port_duplex_t   duplex,
    rtk_enable_t        flowControl);

/* Function Name:
 *      dal_cypress_port_phyMasterSlave_get
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
dal_cypress_port_phyMasterSlave_get(
    uint32              unit,
    rtk_port_t          port,
    rtk_port_masterSlave_t   *pMasterSlaveCfg,
    rtk_port_masterSlave_t   *pMasterSlaveActual);

/* Function Name:
 *      dal_cypress_port_phyMasterSlave_set
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
dal_cypress_port_phyMasterSlave_set(
    uint32              unit,
    rtk_port_t          port,
    rtk_port_masterSlave_t   masterSlave);

/* Function Name:
 *      dal_cypress_port_gigaLiteEnable_get
 * Description:
 *      Get the statue of Giga Lite the specific port in the specific unit
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to status of Giga Lite
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
dal_cypress_port_gigaLiteEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_cypress_port_gigaLiteEnable_set
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
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - input parameter out of range
 * Note:
 *      None
 */
extern int32
dal_cypress_port_gigaLiteEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_cypress_port_phyExtParkPageReg_get
 * Description:
 *      Get PHY register data of the specific port with extension page and parking page parameters
 * Input:
 *      unit                - unit id
 *      port                - port id
 *      mainPage            - main page id
 *      extPage             - extension page id
 *      parkPage            - parking page id
 *      reg                 - reg id
 * Output:
 *      pData              - pointer to the PHY reg data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_PHY_PAGE_ID   - invalid page id
 *      RT_ERR_PHY_REG_ID    - invalid reg id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_port_phyExtParkPageReg_get(
    uint32              unit,
    rtk_port_t          port,
    uint32              mainPage,
    uint32              extPage,
    uint32              parkPage,
    rtk_port_phy_reg_t  reg,
    uint32              *pData);

/* Function Name:
 *      dal_cypress_port_phyExtParkPageReg_set
 * Description:
 *      Set PHY register data of the specific port with extension page and parking page parameters
 * Input:
 *      unit               - unit id
 *      port               - port id
 *      mainPage           - main page id
 *      extPage            - extension page id
 *      parkPage           - parking page id
 *      reg                - reg id
 *      data               - reg data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid page id
 *      RT_ERR_PHY_REG_ID   - invalid reg id
 * Note:
 *      None
 */
extern int32
dal_cypress_port_phyExtParkPageReg_set(
    uint32              unit,
    rtk_port_t          port,
    uint32              mainPage,
    uint32              extPage,
    uint32              parkPage,
    rtk_port_phy_reg_t  reg,
    uint32              data);

/* Function Name:
 *      dal_cypress_port_phymaskExtParkPageReg_set
 * Description:
 *      Set PHY register data of the specific portmask with extension page and parking page parameters
 * Input:
 *      unit               - unit id
 *      pPortmask          - pointer to the portmask
 *      mainPage           - main page id
 *      extPage            - extension page id
 *      parkPage           - parking page id
 *      reg                - reg id
 *      data               - reg data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid page id
 *      RT_ERR_PHY_REG_ID   - invalid reg id
 * Note:
 *      None
 */
extern int32
dal_cypress_port_phymaskExtParkPageReg_set(
    uint32              unit,
    rtk_portmask_t      *pPortmask,
    uint32              mainPage,
    uint32              extPage,
    uint32              parkPage,
    rtk_port_phy_reg_t  reg,
    uint32              data);

/* Function Name:
 *      dal_cypress_port_phyMmdReg_get
 * Description:
 *      Get PHY MMD register data of the specific port
 * Input:
 *      unit                - unit id
 *      port                - port id
 *      mmdAddr             - mmd device address
 *      mmdReg              - mmd reg id
 * Output:
 *      pData              - pointer to the PHY reg data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_port_phyMmdReg_get(
    uint32              unit,
    rtk_port_t          port,
    uint32              mmdAddr,
    uint32              mmdReg,
    uint32              *pData);

/* Function Name:
 *      dal_cypress_port_phyMmdReg_set
 * Description:
 *      Set PHY MMD register data of the specific port
 * Input:
 *      unit               - unit id
 *      port               - port id
 *      mmdAddr            - mmd device address
 *      mmdReg             - mmd reg id
 *      data               - reg data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      None
 */
extern int32
dal_cypress_port_phyMmdReg_set(
    uint32              unit,
    rtk_port_t          port,
    uint32              mmdAddr,
    uint32              mmdReg,
    uint32              data);

/* Function Name:
 *      dal_cypress_port_phymaskMmdReg_set
 * Description:
 *      Set PHY MMD register data of the specific portmask
 * Input:
 *      unit               - unit id
 *      pPortmask          - pointer to the portmask
 *      mmdAddr            - mmd device address
 *      mmdReg             - mmd reg id
 *      data               - reg data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_port_phymaskMmdReg_set(
    uint32              unit,
    rtk_portmask_t      *pPortmask,
    uint32              mmdAddr,
    uint32              mmdReg,
    uint32              data);


/* Function Name:
 *      dal_cypress_port_phyReg_get
 * Description:
 *      Get PHY register data of the specific port
 * Input:
 *      unit                - unit id
 *      port                - port id
 *      page                - page id
 *      reg                 - reg id
 * Output:
 *      pData              - pointer to the PHY reg data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_PHY_PAGE_ID   - invalid page id
 *      RT_ERR_PHY_REG_ID    - invalid reg id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_port_phyReg_get(
    uint32              unit,
    rtk_port_t          port,
    uint32              page,
    rtk_port_phy_reg_t  reg,
    uint32              *pData);

/* Function Name:
 *      dal_cypress_port_phyReg_set
 * Description:
 *      Set PHY register data of the specific port
 * Input:
 *      unit               - unit id
 *      port               - port id
 *      page               - page id
 *      reg                - reg id
 *      data               - reg data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid page id
 *      RT_ERR_PHY_REG_ID   - invalid reg id
 * Note:
 *      None
 */
extern int32
dal_cypress_port_phyReg_set(
    uint32              unit,
    rtk_port_t          port,
    uint32              page,
    rtk_port_phy_reg_t  reg,
    uint32              data);

/* Function Name:
 *      dal_cypress_port_phyReg_broadcast_set
 * Description:
 *      Set PHY register data with broadcast mechanism
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
 *      RT_ERR_PHY_PAGE_ID - invalid page id
 *      RT_ERR_PHY_REG_ID  - invalid reg id
 * Note:
 *      None
 */
extern int32
dal_cypress_port_phyReg_broadcast_set(
    uint32              unit,
    uint32              page,
    rtk_port_phy_reg_t  reg,
    uint32              data);

/* Function Name:
 *      dal_cypress_port_phyReg_broadcast_set
 * Description:
 *      Set PHY broadcast ID
 * Input:
 *      unit - unit id
 *      broadcastID - broadcast id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
extern int32
dal_cypress_port_phyReg_broadcastID_set(
    uint32              unit,
    uint32              broadcastID);

/* Function Name:
 *      dal_cypress_port_cpuPortId_get
 * Description:
 *      Get CPU port id of the specific unit
 * Input:
 *      unit                 - unit id
 * Output:
 *      pPort               - pointer to CPU port id
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_port_cpuPortId_get(uint32 unit, rtk_port_t *pPort);

/* Function Name:
 *      dal_cypress_port_isolation_get
 * Description:
 *      Get the portmask of the port isolation
 * Input:
 *      unit                - unit id
 *      port                - port id
 * Output:
 *      pPortmask          - pointer to the portmask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      1. Default value of each port is 1
 *      2. Enable port isolation in the certain ports if relative portmask bits are set to 1
 */
extern int32
dal_cypress_port_isolation_get(uint32 unit, rtk_port_t port, rtk_portmask_t *pPortmask);

/* Function Name:
 *      dal_cypress_port_isolation_set
 * Description:
 *      Set the portmask of the port isolation
 * Input:
 *      unit            - unit id
 *      port            - port id
 *      portmask        - pointer to the portmask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID   - invalid port id
 *      RT_ERR_PORT_MASK - invalid port mask
 * Note:
 *      1. Default value of each port is 1
 *      2. Enable port isolation in the certain ports if relative portmask bits are set to 1
 */
extern int32
dal_cypress_port_isolation_set(uint32 unit, rtk_port_t port, rtk_portmask_t portmask);

/* Function Name:
 *      dal_cypress_port_isolation_add
 * Description:
 *      Add an isolation port to the certain port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      iso_port      - isolation port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      1. Default value of each port is 1
 *      2. Port and iso_port will be isolated when this API is called
 *      3. The iso_port to the relative portmask bit will be set to 1
 */
extern int32
dal_cypress_port_isolation_add(uint32 unit, rtk_port_t port, rtk_port_t iso_port);

/* Function Name:
 *      dal_cypress_port_isolation_del
 * Description:
 *      Delete an existing isolation port of the certain port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      iso_port      - isolation port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      1. Default value of each port is 1
 *      2. Isolated status between the port and the iso_port is removed when this API is called
 *      3. The iso_port to the relative portmask bit will be set to 0
 */
extern int32
dal_cypress_port_isolation_del(uint32 unit, rtk_port_t port, rtk_port_t iso_port);

/* Function Name:
 *      dal_cypress_port_phyComboPortMedia_get
 * Description:
 *      Get PHY port media of the specific port
 * Input:
 *      unit                 - unit id
 *      port                 - port id
 * Output:
 *      pMedia              - pointer to the port media
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID        - invalid port id
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_port_phyComboPortMedia_get(uint32 unit, rtk_port_t port, rtk_port_media_t *pMedia);

/* Function Name:
 *      dal_cypress_port_phyComboPortMedia_set
 * Description:
 *      Set PHY port media of the specific port
 * Input:
 *      unit               - unit id
 *      port               - port id
 *      pMedia            - pointer to the port media
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      1. You can set these port media which mode PHY currently stays on
 */
extern int32
dal_cypress_port_phyComboPortMedia_set(uint32 unit, rtk_port_t port, rtk_port_media_t media);

/* Function Name:
 *      dal_cypress_port_adminEnable_get
 * Description:
 *      Get port admin status of the specific port
 * Input:
 *      unit               - unit id
 *      port               - port id
 *      pEnabled          - pointer to the port admin status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_port_adminEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnabled);

/* Function Name:
 *      dal_cypress_port_adminEnable_set
 * Description:
 *      Set port admin configuration of the specific port
 * Input:
 *      unit               - unit id
 *      port               - port id
 *      enabled            - port admin configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_cypress_port_adminEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enabled);

/* Function Name:
 *      dal_cypress_port_macRemoteLoopbackEnable_get
 * Description:
 *      Get the mac remote loopback enable status of the specific port
 * Input:
 *      unit           - unit id
 *      port           - port id
 * Output:
 *      pEnable       - - pointer to the enable status of mac remote loopback
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. The mac remote loopback enable status of the port is as following:
 *         - DISABLE
 *         - ENABLE
 *      2. Remote loopback is used to loopback packet RX to switch core back to the outer interface.
 */
extern int32
dal_cypress_port_macRemoteLoopbackEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_cypress_port_macRemoteLoopbackEnable_set
 * Description:
 *      Set the mac remote loopback enable status of the specific port
 * Input:
 *      unit           - unit id
 *      port           - port id
 *      enable        - enable status of mac remote loopback
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      1. The mac remote loopback enable status of the port is as following:
 *         - DISABLE
 *         - ENABLE
 *      2. Remote loopback is used to loopback packet RX to switch core back to the outer interface.
 */
extern int32
dal_cypress_port_macRemoteLoopbackEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_cypress_port_macLocalLoopbackEnable_get
 * Description:
 *      Get the mac local loopback enable status of the specific port
 * Input:
 *      unit           - unit id
 *      port           - port id
 * Output:
 *      pEnable       - - pointer to the enable status of mac local loopback
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. The mac local loopback enable status of the port is as following:
 *         - DISABLE
 *         - ENABLE
 *      2. Local loopback is used to loopback packet TX from switch core back to switch core.
 */
extern int32
dal_cypress_port_macLocalLoopbackEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_cypress_port_macLocalLoopbackEnable_set
 * Description:
 *      Set the mac local loopback enable status of the specific port
 * Input:
 *      unit           - unit id
 *      port           - port id
 *      enable        - enable status of mac local loopback
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      1. The mac local loopback enable status of the port is as following:
 *         - DISABLE
 *         - ENABLE
 *      2. Local loopback is used to loopback packet TX from switch core back to switch core.
 */
extern int32
dal_cypress_port_macLocalLoopbackEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_cypress_port_backpressureEnable_get
 * Description:
 *      Get the half duplex backpressure enable status of the specific port
 * Input:
 *      unit           - unit id
 *      port           - port id
 * Output:
 *      pEnable       - - pointer to the enable status of backpressure in half duplex mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. The mac local loopback enable status of the port is as following:
 *         - DISABLE
 *         - ENABLE
 *      2. Used to support backpressure in half mode.
 */
extern int32
dal_cypress_port_backpressureEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_cypress_port_backpressureEnable_set
 * Description:
 *      Set the half duplex backpressure enable status of the specific port
 * Input:
 *      unit           - unit id
 *      port           - port id
 *      enable        - enable status of backpressure in half duplex mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
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
dal_cypress_port_backpressureEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_cypress_port_linkChange_register
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
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 */
extern int32
dal_cypress_port_linkChange_register(uint32 unit, dal_link_change_callback_f link_change_callback);

/* Function Name:
 *      dal_cypress_port_linkChange_register
 * Description:
 *      Unregister callback function for notification of link change
 * Input:
 *      unit           - unit id
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 * Note:
 */
extern int32
dal_cypress_port_linkChange_unregister(uint32 unit);

/* Function Name:
 *      dal_cypress_port_greenEnable_get
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
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_port_greenEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_cypress_port_greenEnable_set
 * Description:
 *      Set the status of green feature of the specific port in the specific unit
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of green feature
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      None
 */
extern int32
dal_cypress_port_greenEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_cypress_port_vlanBasedIsolationEntry_get
 * Description:
 *      Set the statue of linkdown green feature of the specific port in the specific unit
 * Input:
 *      unit   - unit id
 *      index  - index id
 * Output:
 *      pEntry - pointer to vlan-based port isolation entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_port_vlanBasedIsolationEntry_get(uint32 unit, uint32 index, rtk_port_vlanIsolationEntry_t* pEntry);

/* Function Name:
 *      dal_cypress_port_vlanBasedIsolationEntry_set
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
 *      RT_ERR_NULL_POINTER - NULL pointer
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_VLAN_VID     - invalid vid
 *      RT_ERR_PORT_VLAN_ISO_VID_EXIST_IN_OTHER_IDX - vid exists in other entry
 * Note:
 *      None
 */
extern int32
dal_cypress_port_vlanBasedIsolationEntry_set(uint32 unit, uint32 index, rtk_port_vlanIsolationEntry_t* pEntry);

/* Function Name:
 *      dal_cypress_port_vlanBasedIsolation_vlanSource_get
 * Description:
 *      Get comparing VID type of VLAN-based port isolation
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of linkdown green feature
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_port_vlanBasedIsolation_vlanSource_get(uint32 unit, rtk_port_vlanIsolationSrc_t *pVlanSrc);

/* Function Name:
 *      dal_cypress_port_vlanBasedIsolation_vlanSource_set
 * Description:
 *      Set comparing VID type of VLAN-based port isolation
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of linkdown green feature
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_cypress_port_vlanBasedIsolation_vlanSource_set(uint32 unit, rtk_port_vlanIsolationSrc_t vlanSrc);

/* Function Name:
 *      dal_cypress_port_phyCrossOverMode_get
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
dal_cypress_port_phyCrossOverMode_get(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t *pMode);

/* Function Name:
 *      dal_cypress_port_phyCrossOverMode_set
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
dal_cypress_port_phyCrossOverMode_set(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t mode);

/* Function Name:
 *      dal_cypress_port_phyCrossOverStatus_get
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
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_PHY_FIBER_LINKUP - This feature is not supported in this mode
 * Note:
 *      Following value is valid
 *      - PORT_CROSSOVER_STATUS_MDI
 *      - PORT_CROSSOVER_STATUS_MDIX
 */
extern int32
dal_cypress_port_phyCrossOverStatus_get(uint32 unit, rtk_port_t port, rtk_port_crossOver_status_t *pStatus);


/* Function Name:
 *      dal_cypress_port_flowCtrlEnable_get
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
dal_cypress_port_flowCtrlEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_cypress_port_flowCtrlEnable_set
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
dal_cypress_port_flowCtrlEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_cypress_port_phyComboPortFiberMedia_get
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
dal_cypress_port_phyComboPortFiberMedia_get(uint32 unit, rtk_port_t port, rtk_port_fiber_media_t *pMedia);

/* Function Name:
 *      dal_cypress_port_phyComboPortFiberMedia_set
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
dal_cypress_port_phyComboPortFiberMedia_set(uint32 unit, rtk_port_t port, rtk_port_fiber_media_t media);

/* Function Name:
 *      dal_cypress_port_linkDownPowerSavingEnable_get
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
dal_cypress_port_linkDownPowerSavingEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_cypress_port_linkDownPowerSavingEnable_set
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
dal_cypress_port_linkDownPowerSavingEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_cypress_port_linkMedia_get
 * Description:
 *      Get link status and media
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
dal_cypress_port_linkMedia_get(uint32 unit, rtk_port_t port,
    rtk_port_linkStatus_t *pStatus, rtk_port_media_t *pMedia);

/* Function Name:
 *      dal_cypress_port_phy_watchdog
 * Description:
 *      Monitor for phy problem.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is monitor for phy problem and patch it.
 *      Protect PHY page by port semaphore.
 */
extern int32
dal_cypress_port_phy_watchdog(uint32 unit);

#if defined(CONFIG_SDK_WA_SERDES_WATCHDOG)
/* Function Name:
 *      dal_cypress_port_serdes_watchdog
 * Description:
 *      Monitor for serdes link statuse.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is monitor serdes link down and patch it.
 *      Protect PHY page by port semaphore.
 */
int32
dal_cypress_port_serdes_watchdog(uint32 unit);
#endif

/* Function Name:
 *      dal_cypress_port_pktbuf_watchdog
 * Description:
 *      Monitor for packet buffer problem.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is monitor for detect packet buffer problem and patch it.
 *      Protect PHY page by port semaphore.
 */
extern int32
dal_cypress_port_pktbuf_watchdog(uint32 unit);

/* Function Name:
 *      dal_cypress_port_fiber_rx_watchdog
 * Description:
 *      check fiber RX is normal or not
 * Input:
 *      None.
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED - initialize fail
 *      RT_ERR_OK     - initialize success
 * Note:
 *
 */
extern int32
dal_cypress_port_fiber_rx_watchdog(uint32 unit);


/* Function Name:
 *      dal_cypress_port_fiberDownSpeedEnable_get
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
 * Note:
 *      None
 */
extern int32
dal_cypress_port_fiberDownSpeedEnable_get(uint32 unit, rtk_port_t port,
    rtk_enable_t *pEnable);

/* Function Name:
 *      dal_cypress_port_fiberDownSpeedEnable_set
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
 * Note:
 *      None
 */
extern int32
dal_cypress_port_fiberDownSpeedEnable_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable);

/* Function Name:
 *      dal_cypress_port_downSpeedEnable_get
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
 * Note:
 *      None
 */
extern int32
dal_cypress_port_downSpeedEnable_get(uint32 unit, rtk_port_t port,
    rtk_enable_t *pEnable);

/* Function Name:
 *      dal_cypress_port_downSpeedEnable_set
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
 * Note:
 *      None
 */
extern int32
dal_cypress_port_downSpeedEnable_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable);

/* Function Name:
 *      dal_cypress_port_fiberNwayForceLinkEnable_get
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
 * Note:
 *      None
 */
extern int32
dal_cypress_port_fiberNwayForceLinkEnable_get(uint32 unit, rtk_port_t port,
    rtk_enable_t *pEnable);

/* Function Name:
 *      dal_cypress_port_fiberMedia_set
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
 * Note:
 *      None
 */
extern int32
dal_cypress_port_fiberNwayForceLinkEnable_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable);

/* Function Name:
 *      dal_cypress_port_fiberOAMLoopBack_set
 * Description:
 *      Set OAM Loopback featrue of the specific Fiber-port
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
 * Note:
 *      None
 */
extern int32
dal_cypress_port_fiberOAMLoopBack_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable);

/* Function Name:
 *      dal_cypress_port_fiberInternalLoopBack_set
 * Description:
 *      Set Internal Loopback featrue of the specific Fiber-port
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
 * Note:
 *      SERDES port is not included.
 */
extern int32
dal_cypress_port_fiberInternalLoopBack_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable);


/* Function Name:
 *      dal_cypress_port_10gMedia_set
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
 * Note:
 *      (1) The media value is as following:
 *          - PORT_10GMEDIA_FIBER_10G,
 *          - PORT_10GMEDIA_FIBER_1G,
 *          - PORT_10GMEDIA_DAC_50CM,
 *          - PORT_10GMEDIA_DAC_100CM,
 *          - PORT_10GMEDIA_DAC_300CM,
 */
extern int32
dal_cypress_port_10gMedia_set(uint32 unit, rtk_port_t port, rtk_port_10gMedia_t media);

/* Function Name:
 *      dal_cypress_port_10gMedia_get
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
 * Note:
 *      (1) The media type of the port is as following:
 *          - PORT_10GMEDIA_FIBER_10G,
 *          - PORT_10GMEDIA_FIBER_1G,
 *          - PORT_10GMEDIA_DAC_50CM,
 *          - PORT_10GMEDIA_DAC_100CM,
 *          - PORT_10GMEDIA_DAC_300CM,
 */
extern int32
dal_cypress_port_10gMedia_get(uint32 unit, rtk_port_t port, rtk_port_10gMedia_t *media);

/* Function Name:
 *      dal_cypress_port_10gSds_restart
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
 * Note:
 *      None
 */
extern int32
dal_cypress_port_10gSds_restart(uint32 unit, rtk_port_t port);

/* Function Name:
 *      dal_cypress_port_10g_init
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
 * Note:
 *      None
 */
extern int32
dal_cypress_port_10g_init(uint32 unit, rtk_port_t port);

#endif /* __DAL_CYPRESS_PORT_H__ */
