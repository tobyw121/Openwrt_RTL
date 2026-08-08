/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * $Revision: 30053 $
 * $Date: 2012-06-19 14:12:07 +0800 (Tue, 19 Jun 2012) $
 *
 * Purpose : Definition those public global APIs and its data type in the SDK.
 *
 * Feature :  Parameter settings for the system-wise view 
 *
 */

#ifndef __DAL_ESW_SWITCH_H__
#define __DAL_ESW_SWITCH_H__

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/switch.h>

/*
 * Symbol Definition
 */

/*
 * Macro Declaration
 */

/*
 * Function Declaration
 */

/* Module Name    : Switch     */
/* Sub-module Name: Switch parameter settings */

/* Function Name:
 *      dal_esw_switch_init
 * Description:
 *      Initialize switch module of the specified device.
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
dal_esw_switch_init(uint32 unit);

/* Function Name:
 *      dal_esw_switch_portMaxPktLen_get
 * Description:
 *      Get maximum packet length on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pLength - pointer to maximum packet length
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
dal_esw_switch_portMaxPktLen_get(uint32 unit, rtk_port_t port, uint32 *pLength);

/* Function Name:
 *      dal_esw_switch_portMaxPktLen_set
 * Description:
 *      Set maximum packet length on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      length - maximum packet length
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
dal_esw_switch_portMaxPktLen_set(uint32 unit, rtk_port_t port, uint32 length);

/* Function Name:
 *      dal_esw_switch_portSnapMode_get
 * Description:
 *      Get SNAP mode on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pSnapMode - pointer to SNAP mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      SNAP mode is as following
 *      - SNAP_MODE_AAAA03000000
 *      - SNAP_MODE_AAAA03
 */
extern int32
dal_esw_switch_portSnapMode_get(uint32 unit, rtk_port_t port, rtk_snapMode_t *pSnapMode);

/* Function Name:
 *      dal_esw_switch_portSnapMode_set
 * Description:
 *      Set SNAP mode on specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      snapMode - SNAP mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      SNAP mode is as following
 *      - SNAP_MODE_AAAA03000000
 *      - SNAP_MODE_AAAA03
 */
extern int32
dal_esw_switch_portSnapMode_set(uint32 unit, rtk_port_t port, rtk_snapMode_t snapMode);

/* Function Name:
 *      dal_esw_switch_chksumFailAction_get
 * Description:
 *      Get forwarding action of checksum error on specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      failType - checksum fail type
 * Output:
 *      pAction  - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Checksum fail type is as following
 *      - LAYER2_CHKSUM_FAIL
 *      - LAYER3_CHKSUM_FAIL
 *      - LAYER4_CHKSUM_FAIL
 *
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 */
extern int32
dal_esw_switch_chksumFailAction_get(
    uint32                              unit, 
    rtk_port_t                          port, 
    rtk_switch_chksum_fail_t            failType, 
    rtk_action_t                        *pAction);

/* Function Name:
 *      dal_esw_switch_chksumFailAction_set
 * Description:
 *      Set forwarding action of checksum error on specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      failType - checksum fail type
 *      action   - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_FWD_ACTION       - invalid error forwarding action
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Checksum fail type is as following
 *      - LAYER2_CHKSUM_FAIL
 *      - LAYER3_CHKSUM_FAIL
 *      - LAYER4_CHKSUM_FAIL
 *
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 */
extern int32
dal_esw_switch_chksumFailAction_set(
    uint32                              unit, 
    rtk_port_t                          port, 
    rtk_switch_chksum_fail_t            failType, 
    rtk_action_t                        action);

/* Function Name:
 *      dal_esw_switch_recalcCRCEnable_get
 * Description:
 *      Get enable status of recaculate CRC on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of recaculate CRC
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      When enable, mirrored packet with bad CRC will be recaculate at mirroring port.
 */
extern int32
dal_esw_switch_recalcCRCEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_switch_recalcCRCEnable_set
 * Description:
 *      Set enable status of recaculate CRC on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of recaculate CRC
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
 *      When enable, mirrored packet with bad CRC will be recaculate at mirroring port.
 */
extern int32
dal_esw_switch_recalcCRCEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Module Name    : Switch     */
/* Sub-module Name: Management address and vlan configuration */

/* Function Name:
 *      dal_esw_switch_mgmtVlanId_get
 * Description:
 *      Get management vlan of switch.
 * Input:
 *      unit - unit id
 * Output:
 *      pVid - pointer to vlan id
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
dal_esw_switch_mgmtVlanId_get(uint32 unit, rtk_vlan_t *pVid);

/* Function Name:
 *      dal_esw_switch_mgmtVlanId_set
 * Description:
 *      Set management vlan of switch.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_VLAN_VID         - Invalid vlan id
 * Note:
 *      None
 */
extern int32
dal_esw_switch_mgmtVlanId_set(uint32 unit, rtk_vlan_t vid);

/* Function Name:
 *      dal_esw_switch_outerMgmtVlanId_get
 * Description:
 *      Get outer vlan of switch.
 * Input:
 *      unit      - unit id
 * Output:
 *      pOuterVid - pointer to outer vlan id
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
dal_esw_switch_outerMgmtVlanId_get(uint32 unit, rtk_vlan_t *pOuterVid);

/* Function Name:
 *      dal_esw_switch_outerMgmtVlanId_set
 * Description:
 *      Set outer vlan of switch.
 * Input:
 *      unit     - unit id
 *      outerVid - outer vlan id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_VLAN_VID         - Invalid vlan id
 * Note:
 *      None
 */
extern int32
dal_esw_switch_outerMgmtVlanId_set(uint32 unit, rtk_vlan_t outerVid);

/* Function Name:
 *      dal_esw_switch_mgmtMacAddr_get
 * Description:
 *      Get Mac address of switch.
 * Input:
 *      unit    - unit id
 * Output:
 *      pMac - pointer to Mac address
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
dal_esw_switch_mgmtMacAddr_get(uint32 unit, rtk_mac_t *pMac);

/* Function Name:
 *      dal_esw_switch_mgmtMacAddr_set
 * Description:
 *      Set Mac address of switch.
 * Input:
 *      unit   - unit id
 *      pMac - pointer to Mac address
 * Output:
 *      None
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
dal_esw_switch_mgmtMacAddr_set(uint32 unit, rtk_mac_t *pMac);

/* Function Name:
 *      dal_esw_switch_IPv4Addr_get
 * Description:
 *      Get IPv4 address of switch.
 * Input:
 *      unit    - unit id
 * Output:
 *      pIpAddr - pointer to IPv4 address
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
dal_esw_switch_IPv4Addr_get(uint32 unit, uint32 *pIpAddr);

/* Function Name:
 *      dal_esw_switch_IPv4Addr_set
 * Description:
 *      Set IPv4 address of switch.
 * Input:
 *      unit   - unit id
 *      ipAddr - IPv4 address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_IPV4_ADDRESS     - invalid IPv4 address
 * Note:
 *      None
 */
extern int32
dal_esw_switch_IPv4Addr_set(uint32 unit, uint32 ipAddr);

/* Function Name:
 *      dal_esw_switch_IPv6Addr_get
 * Description:
 *      Get IPv6 address of switch.
 * Input:
 *      unit      - unit id
 * Output:        
 *      pIpv6Addr - pointer to IPv6 address
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
dal_esw_switch_IPv6Addr_get(uint32 unit, rtk_ipv6_addr_t *pIpv6Addr);

/* Function Name:
 *      dal_esw_switch_IPv6Addr_set
 * Description:
 *      Set IPv6 address of switch.
 * Input:
 *      unit     - unit id
 *      ipv6Addr - IPv6 address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_IPV6_ADDRESS     - invalid IPv6 address
 * Note:
 *      None
 */
extern int32
dal_esw_switch_IPv6Addr_set(uint32 unit, rtk_ipv6_addr_t ipv6Addr);

/* Function Name:
 *      dal_esw_switch_hwInterfaceDelayEnable_get
 * Description:
 *      Get the delay state of the specified type in the specified unit.
 * Input:
 *      unit    - unit id
 *      type    - interface delay type
 * Output:
 *      pEnable - pointer to enable state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_switch_hwInterfaceDelayEnable_get(uint32 unit, rtk_switch_delayType_t type, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_switch_hwInterfaceDelayEnable_set
 * Description:
 *      Set the delay state of the specified type in the specified unit.
 * Input:
 *      unit   - unit id
 *      type   - interface delay type
 *      enable - enable state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 * Note:
 *      None
 */
extern int32
dal_esw_switch_hwInterfaceDelayEnable_set(uint32 unit, rtk_switch_delayType_t type, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_switch_pkt2CpuFormat_get
 * Description:
 *      Get the packet to CPU format in the specified unit.
 * Input:
 *      unit    - unit id
 * Output:
 *      pFormat - pointer to packet format
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_switch_pkt2CpuFormat_get(uint32 unit, rtk_pktFormat_t *pFormat);

/* Function Name:
 *      dal_esw_switch_pkt2CpuFormat_set
 * Description:
 *      Set the packet to CPU format in the specified unit.
 * Input:
 *      unit   - unit id
 *      format - packet format
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
dal_esw_switch_pkt2CpuFormat_set(uint32 unit, rtk_pktFormat_t format);

/* Function Name:
 *      dal_esw_switch_sofware_reset_sem
 * Description:
 *      Take/Give software reset semaphore resource in specified device.
 * Input:
 *      unit - unit id
 *      type - semaphore type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      The type value 0 mean lock the semaphore; 1 mean unlock the semaphore.
 */
extern int32 
dal_esw_switch_sofware_reset_sem(uint32 unit, uint32 type);

/* Function Name:
 *      dal_esw_switch_softwareResetCounter_get
 * Description:
 *      Get the switch software reset counter of the specified device.
 * Input:
 *      unit     - unit id
 * Output:
 *      pCounter - pointer buffer of software reset counter
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_switch_softwareResetCounter_get(uint32 unit, uint32 *pCounter);

/* Function Name:
 *      dal_esw_switch_pktbuf_watchdog
 * Description:
 *      Monitor for packet buffer problem of the specified device.
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
 */
extern int32
dal_esw_switch_pktbuf_watchdog(uint32 unit);

#endif /*__DAL_ESW_SWITCH_H__*/

