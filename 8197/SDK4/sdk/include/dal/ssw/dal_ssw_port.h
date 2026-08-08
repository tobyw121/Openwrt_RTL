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
 * $Revision: 30194 $
 * $Date: 2012-06-25 10:50:37 +0800 (Mon, 25 Jun 2012) $
 *
 * Purpose : Definition those public Port APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) Port
 *
 */

#ifndef __DAL_SSW_PORT_H__
#define __DAL_SSW_PORT_H__

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
 *      dal_ssw_port_init
 * Description:
 *      Initialize port module of the specified device.
 * Input:
 *      unit          - unit id
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
dal_ssw_port_init(uint32 unit);

/* Function Name:
 *      dal_ssw_port_link_get
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
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_PORT_ID         - invalid port id
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 * Note:
 *      The link status of the port is as following:
 *      - LINKDOWN
 *      - LINKUP
 */
extern int32
dal_ssw_port_link_get(uint32 unit, rtk_port_t port, rtk_port_linkStatus_t *pStatus);

/* Function Name:
 *      dal_ssw_port_linkMedia_get
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
 *          - LINKDOWN  
 *          - LINKUP    
 *      (2) The media type of the port is as following:
 *          - PORT_MEDIA_COPPER
 *          - PORT_MEDIA_FIBER
 *      (3) When the link status is link-down, the return media should be ignored.
 */
extern int32
dal_ssw_port_linkMedia_get(uint32 unit, rtk_port_t port, rtk_port_linkStatus_t *pStatus, rtk_port_media_t *pMedia);

/* Function Name:
 *      dal_ssw_port_txEnable_get
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      The TX enable status of the port is as following:
 *      - DISABLE   
 *      - ENABLE    
 */
extern int32
dal_ssw_port_txEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_ssw_port_txEnable_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      The TX enable status of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
extern int32
dal_ssw_port_txEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_ssw_port_rxEnable_get
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      The RX enable status of the port is as following:
 *      - DISABLE   
 *      - ENABLE    
 */
extern int32
dal_ssw_port_rxEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_ssw_port_rxEnable_set
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
 *      RT_ERR_UNIT_ID   - invalid unit id
 *      RT_ERR_PORT_ID   - invalid port id
 *      RT_ERR_INPUT     - invalid input parameter
 * Note:
 *      The RX enable status of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
extern int32
dal_ssw_port_rxEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_ssw_port_specialCongest_set
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
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_ssw_port_specialCongest_set(uint32 unit, rtk_port_t port, uint32 second);

/* Function Name:
 *      dal_ssw_port_speedDuplex_get
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
 *      RT_ERR_UNIT_ID         - invalid unit id
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
dal_ssw_port_speedDuplex_get(
    uint32            unit,
    rtk_port_t        port,
    rtk_port_speed_t  *pSpeed,
    rtk_port_duplex_t *pDuplex);

/* Function Name:
 *      dal_ssw_port_flowctrl_get
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
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_PORT_ID         - invalid port id
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 *      RT_ERR_PORT_LINKDOWN   - link down port status
 * Note:
 *      None
 */
int32
dal_ssw_port_flowctrl_get(
    uint32            unit,
    rtk_port_t        port,
    uint32            *pTxStatus,
    uint32            *pRxStatus);

/* Function Name:
 *      dal_ssw_port_phyAutoNegoEnable_get
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
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_PORT_ID        - invalid port id
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_ssw_port_phyAutoNegoEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_ssw_port_phyAutoNegoEnable_set
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
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_PORT_ID        - invalid port id
 *      RT_ERR_INPUT          - input parameter out of range
 * Note:
 *      1. ENABLED : switch to PHY auto negotiation mode
 *         DISABLED: switch to PHY force mode
 *      2. Once the abilities of both auto-nego and force mode are set,
 *         you can freely swtich the mode without calling ability setting API again
 */
extern int32
dal_ssw_port_phyAutoNegoEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_ssw_port_phyAutoNegoAbility_get
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
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_PORT_ID        - invalid port id
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_ssw_port_phyAutoNegoAbility_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_port_phy_ability_t  *pAbility);

/* Function Name:
 *      dal_ssw_port_phyAutoNegoAbility_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. You can set these abilities no matter which mode PHY currently stays on
 */
extern int32
dal_ssw_port_phyAutoNegoAbility_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_port_phy_ability_t  *pAbility);

/* Function Name:
 *      dal_ssw_port_phyForceModeAbility_get
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
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_PORT_ID        - invalid port id
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_ssw_port_phyForceModeAbility_get(
    uint32              unit,
    rtk_port_t          port,
    rtk_port_speed_t    *pSpeed,
    rtk_port_duplex_t   *pDuplex,
    rtk_enable_t        *pFlowControl);

/* Function Name:
 *      dal_ssw_port_phyForceModeAbility_set
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
 *      RT_ERR_UNIT_ID         - invalid unit id
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
dal_ssw_port_phyForceModeAbility_set(
    uint32              unit,
    rtk_port_t          port,
    rtk_port_speed_t    speed,
    rtk_port_duplex_t   duplex,
    rtk_enable_t        flowControl);

/* Function Name:
 *      dal_ssw_port_phyReg_get
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
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_PHY_PAGE_ID   - invalid page id
 *      RT_ERR_PHY_REG_ID    - invalid reg id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_ssw_port_phyReg_get(
    uint32              unit,
    rtk_port_t          port,
    uint32              page,
    rtk_port_phy_reg_t  reg,
    uint32              *pData);

/* Function Name:
 *      dal_ssw_port_phyReg_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid page id
 *      RT_ERR_PHY_REG_ID   - invalid reg id
 * Note:
 *      None
 */
extern int32
dal_ssw_port_phyReg_set(
    uint32              unit,
    rtk_port_t          port,
    uint32              page,
    rtk_port_phy_reg_t  reg,
    uint32              data);

/* Function Name:
 *      dal_ssw_port_cpuPortId_get
 * Description:
 *      Get CPU port id of the specific unit
 * Input:
 *      unit                 - unit id
 * Output:
 *      pPort               - pointer to CPU port id
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_ssw_port_cpuPortId_get(uint32 unit, rtk_port_t *pPort);

/* Function Name:
 *      dal_ssw_port_isolation_get
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
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      1. Default value of each port is 1
 *      2. Enable port isolation in the certain ports if relative portmask bits are set to 1
 */
extern int32
dal_ssw_port_isolation_get(uint32 unit, rtk_port_t port, rtk_portmask_t *pPortmask);

/* Function Name:
 *      dal_ssw_port_isolation_set
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
 *      RT_ERR_UNIT_ID   - invalid unit id
 *      RT_ERR_PORT_ID   - invalid port id
 *      RT_ERR_PORT_MASK - invalid port mask
 * Note:
 *      1. Default value of each port is 1
 *      2. Enable port isolation in the certain ports if relative portmask bits are set to 1
 */
extern int32
dal_ssw_port_isolation_set(uint32 unit, rtk_port_t port, rtk_portmask_t portmask);

/* Function Name:
 *      dal_ssw_port_isolation_add
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
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      1. Default value of each port is 1
 *      2. Port and iso_port will be isolated when this API is called
 *      3. The iso_port to the relative portmask bit will be set to 1
 */
extern int32
dal_ssw_port_isolation_add(uint32 unit, rtk_port_t port, rtk_port_t iso_port);

/* Function Name:
 *      dal_ssw_port_isolation_del
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
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      1. Default value of each port is 1
 *      2. Isolated status between the port and the iso_port is removed when this API is called
 *      3. The iso_port to the relative portmask bit will be set to 0
 */
extern int32
dal_ssw_port_isolation_del(uint32 unit, rtk_port_t port, rtk_port_t iso_port);

/* Function Name:
 *      dal_ssw_port_phyComboPortMedia_get
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
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_PORT_ID        - invalid port id
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_ssw_port_phyComboPortMedia_get(uint32 unit, rtk_port_t port, rtk_port_media_t *pMedia);

/* Function Name:
 *      dal_ssw_port_phyComboPortMedia_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      1. You can set these port media which mode PHY currently stays on
 */
extern int32
dal_ssw_port_phyComboPortMedia_set(uint32 unit, rtk_port_t port, rtk_port_media_t media);

/* Function Name:
 *      dal_ssw_port_adminEnable_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_ssw_port_adminEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnabled);

/* Function Name:
 *      dal_ssw_port_adminEnable_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_ssw_port_adminEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enabled);

/* Function Name:
 *      dal_ssw_port_backpressureEnable_get
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
 *      RT_ERR_UNIT_ID  - invalid unit id
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
dal_ssw_port_backpressureEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_ssw_port_backpressureEnable_set
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
dal_ssw_port_backpressureEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_ssw_port_linkChange_register
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
dal_ssw_port_linkChange_register(uint32 unit, dal_link_change_callback_f link_change_callback);

/* Function Name:
 *      dal_ssw_port_linkChange_unregister
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
dal_ssw_port_linkChange_unregister(uint32 unit);

/* Function Name:
 *      dal_ssw_port_greenEnable_get
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
dal_ssw_port_greenEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_ssw_port_greenEnable_set
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
dal_ssw_port_greenEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_ssw_port_phyCrossOverMode_get
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
dal_ssw_port_phyCrossOverMode_get(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t *pMode);

/* Function Name:
 *      dal_ssw_port_phyCrossOverMode_set
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
dal_ssw_port_phyCrossOverMode_set(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t mode);

/* Function Name:
 *      dal_ssw_port_phyComboPortFiberMedia_get
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
dal_ssw_port_phyComboPortFiberMedia_get(uint32 unit, rtk_port_t port, rtk_port_fiber_media_t *pMedia);

/* Function Name:
 *      dal_ssw_port_phyComboPortFiberMedia_set
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
dal_ssw_port_phyComboPortFiberMedia_set(uint32 unit, rtk_port_t port, rtk_port_fiber_media_t media);

/* Function Name:
 *      dal_ssw_port_linkDownPowerSavingEnable_set
 * Description:
 *      Set the statue of link-down power saving of the specific port in the specific unit
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
dal_ssw_port_linkDownPowerSavingEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t pEnable);

/* Function Name:
 *      dal_ssw_port_linkDownPowerSavingEnable_get
 * Description:
 *      Ge the statue of link-down power saving of the specific port in the specific unit
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
dal_ssw_port_linkDownPowerSavingEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);


/* Function Name:
 *      dal_ssw_port_gigaLiteEnable_set
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
dal_ssw_port_gigaLiteEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_ssw_port_gigaLiteEnable_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_ssw_port_gigaLiteEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

#endif /* __DAL_SSW_PORT_H__ */
