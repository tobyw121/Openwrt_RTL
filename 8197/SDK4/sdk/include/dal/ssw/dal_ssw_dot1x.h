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
 * Purpose : Definition those public dot1x APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *            1) 802.1X Port-based Network Access Control
 *            2) 802.1X MAC-based Network Access Control
 *
 */

#ifndef __DAL_SSW_DOT1X_H__
#define __DAL_SSW_DOT1X_H__


/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/dot1x.h>


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

/* Function Name:
 *      dal_ssw_dot1x_init
 * Description:
 *      Initial the dot1x module of the specified device..
 * Input:
 *      unit     - unit id
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
extern int32
dal_ssw_dot1x_init(uint32 unit);

/* Function Name:
 *      dal_ssw_dot1x_unauthPacketOper_get
 * Description:
 *      Get the configuration of unauthorized behavior for both 802.1x port and mac based network access control.
 * Input:
 *      unit                    - unit id
 * Output:
 *      *pUnauth_action         - The action of how to handle unauthorized packet 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER      - NULL pointer
 * Note:
 *    The action of how to handle unauthorized packet is as following:
 *    - DROP
 *    - TRAP2CPU
 */
int32
dal_ssw_dot1x_unauthPacketOper_get(uint32 unit, rtk_dot1x_unauth_action_t *pUnauth_action);

/* Function Name:
 *      dal_ssw_dot1x_unauthPacketOper_set
 * Description:
 *      Set the configuration of unauthorized behavior for both 802.1x port and mac based network access control.
 * Input:
 *      unit                  - unit id
 *      unauth_action         - The action of how to handle unauthorized packet 
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - Invalid unit id
 *      RT_ERR_INPUT            - Invalid input parameter
 * Note:
 *    1. User can manually configure how packet should be handled when port or mac based network access control
 *       is unauthorized.
 *
 *    2. The action of how to handle unauthorized packet is as following:
 *       - DROP
 *       - TRAP2CPU
 */
int32
dal_ssw_dot1x_unauthPacketOper_set(uint32 unit, rtk_dot1x_unauth_action_t unauth_action);

/* Function Name:
 *      dal_ssw_dot1x_eapolFrame2CpuEnable_get
 * Description:
 *      Get the configuration of 802.1x EAPOL frame trap to CPU.
 * Input:
 *      unit                    - unit id
 * Output:
 *      *pEnable                - The status of EAPOL frame trap to CPU
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_UNIT_ID           - Invalid unit id
 *      RT_ERR_NULL_POINTER      - NULL pointer
 * Note:
 *    The status of EAPOL frame trap to CPU is as following:
 *    - DISABLED
 *    - ENABLED
 */
extern int32
dal_ssw_dot1x_eapolFrame2CpuEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_ssw_dot1x_eapolFrame2CpuEnable_set
 * Description:
 *      Set the configuration of 802.1x EAPOL frame trap to CPU.
 * Input:
 *      unit                    - unit id
 *      enable                  - The status of EAPOL frame trap to CPU
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID           - Invalid unit id
 *      RT_ERR_INPUT            - Invalid input parameter
 * Note:
 *    1. To support 802.1x authentication functionality, EAPOL frame (ether type = 888E) has to
 *       be trapped to CPU.
 *
 *    2. The status of EAPOL frame trap to CPU is as following:
 *       - DISABLED
 *       - ENABLED
 */
extern int32
dal_ssw_dot1x_eapolFrame2CpuEnable_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      dal_ssw_dot1x_portBasedEnable_get
 * Description:
 *      Get the status of 802.1x port-based network access control on a specific port.
 * Input:
 *      unit        - unit id
 *      port        - port id
 * Output:
 *      *pEnable    - The status of 802.1x port-based network access control.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_NULL_POINTER  - NULL pointer
  * Note:
 *    1. If a port is 802.1x port based network access control "enabled",
 *       it should be authenticated so packets from that port wont be dropped or trapped to CPU.
 *
 *    2. The status of 802.1x port-based network access control is as following:
 *       - DISABLED
 *       - ENABLED
 */
extern int32
dal_ssw_dot1x_portBasedEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_ssw_dot1x_portBasedEnable_set
 * Description:
 *      Set the status of 802.1x port-based network access control on a specific port
 * Input:
 *      unit            - unit id
 *      port            - port id
 *      enable          - The status of 802.1x port-based network access control.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_INPUT         - Invalid input parameter
 * Note:
 *    1. If a port is 802.1x port based network access control "enabled",
 *       it should be authenticated so packets from that port wont be dropped or trapped to CPU.
 *
 *    2. The status of 802.1x port-based network access control is as following:
 *       - DISABLED
 *       - ENABLED
 */
extern int32
dal_ssw_dot1x_portBasedEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_ssw_dot1x_portBasedAuthStatus_get
 * Description:
 *      Get the authenticated status of 802.1x port-based network access control on a specific port.
 * Input:
 *      unit        - unit id
 *      port        - port id
 * Output:
 *      *pPort_auth     - The status of 802.1x port-based network access controlx is authenticated
 *                        or unauthenticated.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_NULL_POINTER  - NULL pointer
 * Note:
 *    The authenticated status of 802.1x port-based network access control is as following:
 *    - UNAUTH
 *    - AUTH
 */
extern int32
dal_ssw_dot1x_portBasedAuthStatus_get(
    uint32                    unit,
    rtk_port_t                port,
    rtk_dot1x_auth_status_t   *pPort_auth);

/* Function Name:
 *      dal_ssw_dot1x_portBasedAuthStatus_set
 * Description:
 *      Set the authenticated status of 802.1x port-based network access control on a specific
 *      port.
 * Input:
 *      unit         - unit id  
 *      port         - port id
 *      port_auth    - The status of 802.1x port-based network access control is authenticated or unauthenticated.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_INPUT         - Invalid input parameter
 * Note:
 *    The authenticated status of 802.1x port-based network access control is as following:
 *    - UNAUTH
 *    - AUTH
 */
extern int32
dal_ssw_dot1x_portBasedAuthStatus_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_dot1x_auth_status_t port_auth);

/* Function Name:
 *      dal_ssw_dot1x_portBasedDirection_get
 * Description:
 *      Get the operate controlled direction of 802.1x Port-based network access control on a specific port.
 * Input:
 *      unit        - unit id
 *      port        - port id
 * Output:
 *      *pDirection    - The operate controlled direction  of 802.1x Port-based network access control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_NULL_POINTER  - NULL pointer
 * Note:
 *    None.
 */
extern int32
dal_ssw_dot1x_portBasedDirection_get(uint32 unit, rtk_port_t port, uint32 *pDirection);

/* Function Name:
 *      dal_ssw_dot1x_portBasedDirection_set
 * Description:
 *      Set the operate controlled direction of 802.1x Port-based network access control on a specific port.
 * Input:
 *      unit            - unit id
 *      port            - port id
 *      direction       - The operate controlled direction of 802.1x Port-based network access control
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_INPUT         - Invalid input parameter
 * Note:
 *    None.
 */
extern int32
dal_ssw_dot1x_portBasedDirection_set(uint32 unit, rtk_port_t port, uint32 direction);

/* Function Name:
 *      dal_ssw_dot1x_macBasedEnable_get
 * Description:
 *      Get the status of 802.1x MAC-based network access control on a specific port.
 * Input:
 *      unit        - unit id
 *      port        - port id
 * Output:
 *      *pEnable    - The status of 802.1x MAC-based network access control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_NULL_POINTER  - NULL pointer
 * Note:
 *    1. If a port is 802.1x MAC based network access control "enabled",
 *       the incoming packets should be authenticated so packets from that port wont be dropped
 *       or trapped to CPU.
 *
 *    2. The status of 802.1x MAC-based network access control is as following:
 *       - DISABLED
 *       - ENABLED
 */
extern int32
dal_ssw_dot1x_macBasedEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_ssw_dot1x_macBasedEnable_set
 * Description:
 *      Set the status of 802.1x MAC-based network access control on a specific port.
 * Input:
 *      unit            - unit id
 *      port            - port id
 *      enable          - The status of 802.1x MAC-based network access control
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_INPUT         - Invalid input parameter
 * Note:
 *    1. If a port is 802.1x MAC based network access control "enabled",
 *       the incoming packets should be authenticated so packets from that port wont be dropped
 *       or trapped to CPU.
 *
 *    2. The status of 802.1x MAC-based network access control is as following:
 *       - DISABLED
 *       - ENABLED
 */
extern int32
dal_ssw_dot1x_macBasedEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_ssw_dot1x_macBasedAuthMac_add
 * Description:
 *      Add an authenticated MAC to ASIC
 * Input:
 *      unit            - unit id
 *      port            - port id
 *      vid             - vlan id
 *      *pAuth_mac      - The authenticated MAC
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_VLAN_ID       - Invalid vlan id
 *      RT_ERR_MAC           - Invalid MAC address
 *      RT_ERR_L2_NO_EMPTY_ENTRY - No empty entry in L2 table
 * Note:
 *      None.
 */
extern int32
dal_ssw_dot1x_macBasedAuthMac_add(
    uint32      unit,
    rtk_port_t  port,
    rtk_vlan_t  vid,
    rtk_mac_t   *pAuth_mac);

/* Function Name:
 *      dal_ssw_dot1x_macBasedAuthMac_del
 * Description:
 *      Delete an authenticated MAC from ASIC
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      vid         - vlan id
 *      *pAuth_mac  - The authenticated MAC
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_VLAN_ID       - Invalid vlan id
 *      RT_ERR_MAC           - Invalid MAC address
 *      RT_ERR_L2_NO_EMPTY_ENTRY - No empty entry in L2 table
 * Note:
 *      None.
 */
extern int32
dal_ssw_dot1x_macBasedAuthMac_del(
    uint32      unit,
    rtk_port_t  port,
    rtk_vlan_t  vid,
    rtk_mac_t   *pAuth_mac);

/* Function Name:
 *      dal_ssw_dot1x_macBasedDirection_get
 * Description:
 *      Get the operate controlled direction 802.1x mac-based network access control on system.
 *     
 * Input:
 *      unit         - unit id  
 *
 * Output:
 *      pMac_direction - The controlled direction of 802.1x mac-based network access control is BOTH
 *                       or IN.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - NULL pointer
 * Note:
 *    The operate controlled direction of 802.1x mac-based network access control is as following:
 *    - BOTH
 *    - IN
 */
extern int32
dal_ssw_dot1x_macBasedDirection_get(uint32 unit, rtk_dot1x_direction_t *pMac_direction);

/* Function Name:
 *      dal_ssw_dot1x_macBasedDirection_set
 * Description:
 *      Set the operate controlled direction 802.1x mac-based network access control on system.
 *     
 * Input:
 *      unit          - unit id  
 *      mac_direction - The controlled direction of 802.1x mac-based network access control is BOTH
 *                       or IN.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - Invalid input parameter
 * Note:
 *    The operate controlled direction of 802.1x mac-based network access control is as following:
 *    - BOTH
 *    - IN
 */
extern int32
dal_ssw_dot1x_macBasedDirection_set(uint32 unit, rtk_dot1x_direction_t mac_direction);


#endif /* __DAL_SSW_DOT1X_H__ */

