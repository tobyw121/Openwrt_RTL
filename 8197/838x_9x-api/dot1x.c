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
 * $Revision: 27436 $
 * $Date: 2012-03-22 15:50:04 +0800 (Thu, 22 Mar 2012) $
 *
 * Purpose : Definition of 802.1x API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Unauth packet handling
 *           (2) 802.1X port-based NAC
 *           (3) 802.1X MAC-based NAC
 *           (4) 802.1X parameter
 *           (5) Parameter for trapped packets
 *
 */

/*
 * Include Files
 */
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <dal/dal_mgmt.h>
#include <rtk/default.h>
#include <rtk/dot1x.h>

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
 *      rtk_dot1x_init
 * Description:
 *      Initial the dot1x module of the specified device..
 * Input:
 *      unit - unit id
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Applicable:
 *      8389, 8328
 * Note:
 *      Must initialize dot1x module before calling any dot1x APIs.
 */
int32
rtk_dot1x_init(uint32 unit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_init(unit);
} /* end of rtk_dot1x_init */


/* Module Name    : Dot1x                  */
/* Sub-module Name: Unauth packet handling */

/* Function Name:
 *      rtk_dot1x_unauthPacketOper_get
 * Description:
 *      Get the configuration of unauthorized behavior for both 802.1x port and mac based network access control.
 * Input:
 *      unit           - unit id
 * Output:
 *      pUnauth_action - The action of how to handle unauthorized packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389
 * Note:
 *      The action of how to handle unauthorized packet is as following:
 *      - DOT1X_ACTION_DROP
 *      - DOT1X_ACTION_TRAP2CPU
 */
int32
rtk_dot1x_unauthPacketOper_get(uint32 unit, rtk_dot1x_unauth_action_t *pUnauth_action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_unauthPacketOper_get(unit, pUnauth_action);
} /* end of rtk_dot1x_unauthPacketOper_get */

/* Function Name:
 *      rtk_dot1x_unauthPacketOper_set
 * Description:
 *      Set the configuration of unauthorized behavior for both 802.1x port and mac based network access control.
 * Input:
 *      unit          - unit id
 *      unauth_action - The action of how to handle unauthorized packet
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8389
 * Note:
 *      (1) User can manually configure how packet should be handled when port or mac based network access control
 *          is unauthorized.
 *
 *      (2) The action of how to handle unauthorized packet is as following:
 *          - DOT1X_ACTION_DROP
 *          - DOT1X_ACTION_TRAP2CPU
 */
int32
rtk_dot1x_unauthPacketOper_set(uint32 unit, rtk_dot1x_unauth_action_t unauth_action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_unauthPacketOper_set(unit, unauth_action);
} /* end of rtk_dot1x_unauthPacketOper_set */

/* Function Name:
 *      rtk_dot1x_portUnauthPacketOper_get
 * Description:
 *      Get the configuration of unauthorized behavior for both 802.1x port and mac based network access control on specified port.
 * Input:
 *      unit           - unit id
 *      port           - port id
 * Output:
 *      pUnauth_action - The action of how to handle unauthorized packet
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
 *      Forwarding action for unauth packet is as following
 *      - DOT1X_ACTION_DROP
 *      - DOT1X_ACTION_TRAP2CPU
 *      - DOT1X_ACTION_TO_GUEST_VLAN
 */
int32
rtk_dot1x_portUnauthPacketOper_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_dot1x_unauth_action_t   *pUnauthAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_portUnauthPacketOper_get(unit, port, pUnauthAction);
} /* end of rtk_dot1x_portUnauthPacketOper_get */

/* Function Name:
 *      rtk_dot1x_portUnauthPacketOper_set
 * Description:
 *      Set the configuration of unauthorized behavior for both 802.1x port and mac based network access control on specified port.
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      unauth_action - The action of how to handle unauthorized packet
 * Output:
 *      None.
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
 *      Forwarding action for unauth packet is as following
 *      - DOT1X_ACTION_DROP
 *      - DOT1X_ACTION_TRAP2CPU
 *      - DOT1X_ACTION_TO_GUEST_VLAN
 */
int32
rtk_dot1x_portUnauthPacketOper_set(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_dot1x_unauth_action_t   unauthAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_portUnauthPacketOper_set(unit, port, unauthAction);
} /* end of rtk_dot1x_portUnauthPacketOper_set */

/* Function Name:
 *      rtk_dot1x_portUnauthTagPacketOper_get
 * Description:
 *      Get forwarding action for tagged unauth packet on specified port.
 * Input:
 *      unit          - unit id
 *      port          - port id
 * Output:
 *      pUnauthAction - pointer to forwarding action for tagged unauth packet
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
 *      Forwarding action for tagged unauth packet is as following
 *      - DOT1X_ACTION_DROP
 *      - DOT1X_ACTION_TRAP2CPU
 *      - DOT1X_ACTION_TO_GUEST_VLAN
 */
int32
rtk_dot1x_portUnauthTagPacketOper_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_dot1x_unauth_action_t   *pUnauthAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_portUnauthTagPacketOper_get(unit, port, pUnauthAction);
} /* end of rtk_dot1x_portUnauthTagPacketOper_get */

/* Function Name:
 *      rtk_dot1x_portUnauthTagPacketOper_set
 * Description:
 *      Set forwarding action for tagged unauth packet on specified port.
 * Input:
 *      unit         - unit id
 *      port         - port id
 *      unauthAction - forwarding action for tagged unauth packet
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
 *      Forwarding action for tagged unauth packet is as following
 *      - DOT1X_ACTION_DROP
 *      - DOT1X_ACTION_TRAP2CPU
 *      - DOT1X_ACTION_TO_GUEST_VLAN
 */
int32
rtk_dot1x_portUnauthTagPacketOper_set(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_dot1x_unauth_action_t   unauthAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_portUnauthTagPacketOper_set(unit, port, unauthAction);
} /* end of rtk_dot1x_portUnauthTagPacketOper_set */

/* Function Name:
 *      rtk_dot1x_portUnauthUntagPacketOper_get
 * Description:
 *      Get forwarding action for untagged unauth packet on specified port.
 * Input:
 *      unit          - unit id
 *      port          - port id
 * Output:
 *      pUnauthAction - pointer to forwarding action for untagged unauth packet
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
 *      Forwarding action for untagged unauth packet is as following
 *      - DOT1X_ACTION_DROP
 *      - DOT1X_ACTION_TRAP2CPU
 *      - DOT1X_ACTION_TO_GUEST_VLAN
 */
int32
rtk_dot1x_portUnauthUntagPacketOper_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_dot1x_unauth_action_t   *pUnauthAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_portUnauthUntagPacketOper_get(unit, port, pUnauthAction);
} /* end of rtk_dot1x_portUnauthUntagPacketOper_get */

/* Function Name:
 *      rtk_dot1x_portUnauthUntagPacketOper_set
 * Description:
 *      Set forwarding action for untagged unauth packet on specified port.
 * Input:
 *      unit         - unit id
 *      port         - port id
 *      unauthAction - forwarding action for untagged unauth packet
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
 *      Forwarding action for untagged unauth packet is as following
 *      - DOT1X_ACTION_DROP
 *      - DOT1X_ACTION_TRAP2CPU
 *      - DOT1X_ACTION_TO_GUEST_VLAN
 */
int32
rtk_dot1x_portUnauthUntagPacketOper_set(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_dot1x_unauth_action_t   unauthAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_portUnauthUntagPacketOper_set(unit, port, unauthAction);
} /* end of rtk_dot1x_portUnauthUntagPacketOper_set */


/* Module Name    : Dot1x                 */
/* Sub-module Name: 802.1X port-based NAC */

/* Function Name:
 *      rtk_dot1x_eapolFrame2CpuEnable_get
 * Description:
 *      Get the configuration of 802.1x EAPOL frame trap to CPU.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - The status of EAPOL frame trap to CPU
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389
 * Note:
 *      The status of EAPOL frame trap to CPU is as following:
 *      - DISABLED
 *      - ENABLED
 */
int32
rtk_dot1x_eapolFrame2CpuEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_eapolFrame2CpuEnable_get(unit, pEnable);
} /* end of rtk_dot1x_eapolFrame2CpuEnable_get */

/* Function Name:
 *      rtk_dot1x_eapolFrame2CpuEnable_set
 * Description:
 *      Set the configuration of 802.1x EAPOL frame trap to CPU.
 * Input:
 *      unit   - unit id
 *      enable - The status of EAPOL frame trap to CPU
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8389
 * Note:
 *      (1) To support 802.1x authentication functionality, EAPOL frame (ether type = 888E) has to
 *          be trapped to CPU.
 *
 *      (2) The status of EAPOL frame trap to CPU is as following:
 *          - DISABLED
 *          - ENABLED
 */
int32
rtk_dot1x_eapolFrame2CpuEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_eapolFrame2CpuEnable_set(unit, enable);
} /* end of rtk_dot1x_eapolFrame2CpuEnable_set */

/* Function Name:
 *      rtk_dot1x_portBasedEnable_get
 * Description:
 *      Get the status of 802.1x port-based network access control on a specific port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - The status of 802.1x port-based network access control.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328
 * Note:
 *      (1) If a port is 802.1x port based network access control "enabled",
 *          it should be authenticated so packets from that port wont be dropped or trapped to CPU.
 *
 *      (2) The status of 802.1x port-based network access control is as following:
 *          - DISABLED
 *          - ENABLED
 */
int32
rtk_dot1x_portBasedEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_portBasedEnable_get(unit, port, pEnable);
} /* end of rtk_dot1x_portBasedEnable_get */

/* Function Name:
 *      rtk_dot1x_portBasedEnable_set
 * Description:
 *      Set the status of 802.1x port-based network access control on a specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - The status of 802.1x port-based network access control.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8389, 8328
 * Note:
 *      (1) If a port is 802.1x port based network access control "enabled",
 *          it should be authenticated so packets from that port wont be dropped or trapped to CPU.
 *
 *      (2) The status of 802.1x port-based network access control is as following:
 *          - DISABLED
 *          - ENABLED
 */
int32
rtk_dot1x_portBasedEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_portBasedEnable_set(unit, port, enable);
} /* end of rtk_dot1x_portBasedEnable_set */

/* Function Name:
 *      rtk_dot1x_portBasedAuthStatus_get
 * Description:
 *      Get the authenticated status of 802.1x port-based network access control on a specific port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pPort_auth - The status of 802.1x port-based network access controlx is authenticated
 *                   or unauthenticated.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328
 * Note:
 *      The authenticated status of 802.1x port-based network access control is as following:
 *      - UNAUTH
 *      - AUTH
 */
int32
rtk_dot1x_portBasedAuthStatus_get(
    uint32                    unit,
    rtk_port_t                port,
    rtk_dot1x_auth_status_t   *pPort_auth)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_portBasedAuthStatus_get(unit, port, pPort_auth);
} /* end of rtk_dot1x_portBasedAuthStatus_get */

/* Function Name:
 *      rtk_dot1x_portBasedAuthStatus_set
 * Description:
 *      Set the authenticated status of 802.1x port-based network access control on a specific
 *      port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      port_auth - The status of 802.1x port-based network access control is authenticated
 *                  or unauthenticated.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8389, 8328
 * Note:
 *      The authenticated status of 802.1x port-based network access control is as following:
 *      - UNAUTH
 *      - AUTH
 */
int32
rtk_dot1x_portBasedAuthStatus_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_dot1x_auth_status_t port_auth)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_portBasedAuthStatus_set(unit, port, port_auth);
} /* end of rtk_dot1x_portBasedAuthStatus_set */

/* Function Name:
 *      rtk_dot1x_portBasedDirection_get
 * Description:
 *      Get the operate controlled direction 802.1x port-based network access control on a specific
 *      port.
 * Input:
 *      unit            - unit id
 *      port            - port id
 *
 * Output:
 *      pPort_direction - The controlled direction of 802.1x port-based network access control is BOTH
 *                        or IN.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328
 * Note:
 *      The operate controlled direction of 802.1x port-based network access control is as following:
 *      - BOTH
 *      - IN
 */
int32
rtk_dot1x_portBasedDirection_get(uint32 unit, rtk_port_t port, rtk_dot1x_direction_t *pPort_direction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_portBasedDirection_get(unit, port, pPort_direction);
} /* end of rtk_dot1x_portBasedDirection_get */

/* Function Name:
 *      rtk_dot1x_portBasedDirection_set
 * Description:
 *      Set the operate controlled direction 802.1x port-based network access control on a specific
 *      port.
 * Input:
 *      unit           - unit id
 *      port           - port id
 *      port_direction - The controlled direction of 802.1x port-based network access control is BOTH
 *                        or IN.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8389, 8328
 * Note:
 *    The operate controlled direction of 802.1x port-based network access control is as following:
 *    - BOTH
 *    - IN
 */
int32
rtk_dot1x_portBasedDirection_set(uint32 unit, rtk_port_t port, rtk_dot1x_direction_t port_direction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_portBasedDirection_set(unit, port, port_direction);
} /* end of rtk_dot1x_portBasedDirection_set */


/* Module Name    : Dot1x                */
/* Sub-module Name: 802.1x MAC-based NAC */

/* Function Name:
 *      rtk_dot1x_macBasedEnable_get
 * Description:
 *      Get the status of 802.1x MAC-based network access control on a specific port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - The status of 802.1x MAC-based network access control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328
 * Note:
 *    (1) If a port is 802.1x MAC based network access control "enabled",
 *        the incoming packets should be authenticated so packets from that port wont be dropped
 *        or trapped to CPU.
 *    (2) The status of 802.1x MAC-based network access control is as following:
 *        - DISABLED
 *        - ENABLED
 */
int32
rtk_dot1x_macBasedEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_macBasedEnable_get(unit, port, pEnable);
} /* end of rtk_dot1x_macBasedEnable_get */

/* Function Name:
 *      rtk_dot1x_macBasedEnable_set
 * Description:
 *      Set the status of 802.1x MAC-based network access control on a specific port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - The status of 802.1x MAC-based network access control
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8389, 8328
 * Note:
 *      (1) If a port is 802.1x MAC based network access control "enabled",
 *          the incoming packets should be authenticated so packets from that port wont be dropped
 *          or trapped to CPU.
 *      (2) The status of 802.1x MAC-based network access control is as following:
 *          - DISABLED
 *          - ENABLED
 */
int32
rtk_dot1x_macBasedEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_macBasedEnable_set(unit, port, enable);
} /* end of rtk_dot1x_macBasedEnable_set */

/* Function Name:
 *      rtk_dot1x_macBasedAuthMac_add
 * Description:
 *      Add an authenticated MAC to ASIC
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      vid       - vlan id
 *      pAuth_mac - The authenticated MAC
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_VLAN_ID  - Invalid vlan id
 *      RT_ERR_MAC      - Invalid MAC address
 * Applicable:
 *      8389, 8328
 * Note:
 *      None
 */
int32
rtk_dot1x_macBasedAuthMac_add(
    uint32      unit,
    rtk_port_t  port,
    rtk_vlan_t  vid,
    rtk_mac_t   *pAuth_mac)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_macBasedAuthMac_add(unit, port, vid, pAuth_mac);
} /* end of rtk_dot1x_macBasedAuthMac_add */

/* Function Name:
 *      rtk_dot1x_macBasedAuthMac_del
 * Description:
 *      Delete an authenticated MAC from ASIC
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      vid       - vlan id
 *      pAuth_mac - The authenticated MAC
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_VLAN_ID  - Invalid vlan id
 *      RT_ERR_MAC      - Invalid MAC address
 * Applicable:
 *      8389, 8328
 * Note:
 *      None
 */
int32
rtk_dot1x_macBasedAuthMac_del(
    uint32      unit,
    rtk_port_t  port,
    rtk_vlan_t  vid,
    rtk_mac_t   *pAuth_mac)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_macBasedAuthMac_del(unit, port, vid, pAuth_mac);
} /* end of rtk_dot1x_macBasedAuthMac_del */

/* Function Name:
 *      rtk_dot1x_macBasedDirection_get
 * Description:
 *      Get the operate controlled direction 802.1x mac-based network access control on system.
 * Input:
 *      unit           - unit id
 * Output:
 *      pMac_direction - The controlled direction of 802.1x mac-based
 *                       network access control is BOTH or IN.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328
 * Note:
 *      The operate controlled direction of 802.1x mac-based network access control is as following:
 *      - BOTH
 *      - IN
 */
int32
rtk_dot1x_macBasedDirection_get(uint32 unit, rtk_dot1x_direction_t *pMac_direction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_macBasedDirection_get(unit, pMac_direction);
} /* end of rtk_dot1x_macBasedDirection_get */

/* Function Name:
 *      rtk_dot1x_macBasedDirection_set
 * Description:
 *      Set the operate controlled direction 802.1x mac-based network access control on system.
 * Input:
 *      unit          - unit id
 *      mac_direction - The controlled direction of 802.1x mac-based
 *                      network access control is BOTH or IN.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8389, 8328
 * Note:
 *      The operate controlled direction of 802.1x mac-based network access control is as following:
 *      - BOTH
 *      - IN
 */
int32
rtk_dot1x_macBasedDirection_set(uint32 unit, rtk_dot1x_direction_t mac_direction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_macBasedDirection_set(unit, mac_direction);
} /* end of rtk_dot1x_macBasedDirection_set */


/* Module Name    : Dot1x            */
/* Sub-module Name: 802.1X parameter */

/* Function Name:
 *      rtk_dot1x_portGuestVlan_get
 * Description:
 *      Get guest vlan on specified port.
 * Input:
 *      unit        - unit id
 *      port        - port id
 * Output:
 *      pGuest_vlan - pointer to guest vlan id
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
rtk_dot1x_portGuestVlan_get(uint32 unit, rtk_port_t port, rtk_vlan_t *pGuest_vlan)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_portGuestVlan_get(unit, port, pGuest_vlan);
} /* end of rtk_dot1x_portGuestVlan_get */

/* Function Name:
 *      rtk_dot1x_portGuestVlan_set
 * Description:
 *      Set guest vlan on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      guest_vlan - guest vlan id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_VLAN_VID - invalid vlan id
 * Applicable:
 *      8328
 * Note:
 *      (1) Configure the port's forwarding guest vlan setting.
 *      (2) When the packet is unauthenticated and action is DOT1X_ACTION_TO_GUEST_VLAN,
 *          the packet will based on the guest vlan to forward.
 */
int32
rtk_dot1x_portGuestVlan_set(uint32 unit, rtk_port_t port, rtk_vlan_t guest_vlan)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_portGuestVlan_set(unit, port, guest_vlan);
} /* end of rtk_dot1x_portGuestVlan_set */

/* Function Name:
 *      rtk_dot1x_guestVlanBehavior_get
 * Description:
 *      Get forwarding behavior for host in guest vlan.
 * Input:
 *      unit      - unit id
 * Output:
 *      pBehavior - pointer to Forwarding behavior
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      Forwarding behavior is as following
 *      - DISALLOW_TO_AUTH_DA
 *      - ALLOW_TO_AUTH_DA
 */
int32
rtk_dot1x_guestVlanBehavior_get(uint32 unit, rtk_dot1x_guestVlanBehavior_t *pBehavior)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_guestVlanBehavior_get(unit, pBehavior);
} /* end of rtk_dot1x_guestVlanBehavior_get */

/* Function Name:
 *      rtk_dot1x_guestVlanBehavior_set
 * Description:
 *      Set forwarding behavior for host in guest vlan.
 * Input:
 *      unit     - unit id
 *      behavior - Forwarding behavior
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
 *      Forwarding behavior is as following
 *      - DISALLOW_TO_AUTH_DA
 *      - ALLOW_TO_AUTH_DA
 */
int32
rtk_dot1x_guestVlanBehavior_set(uint32 unit, rtk_dot1x_guestVlanBehavior_t behavior)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_guestVlanBehavior_set(unit, behavior);
} /* end of rtk_dot1x_guestVlanBehavior_set */

/* Function Name:
 *      rtk_dot1x_guestVlanRouteBehavior_get
 * Description:
 *      Get forwarding behavior of packets in guest vlan and routed.
 * Input:
 *      unit       - unit id
 * Output:
 *      pFwdAction - pointer to forwarding behavior
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      Forwarding behavior is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 */
int32
rtk_dot1x_guestVlanRouteBehavior_get(uint32 unit, rtk_action_t *pFwdAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_guestVlanRouteBehavior_get(unit, pFwdAction);
} /* end of rtk_dot1x_guestVlanRouteBehavior_get */

/* Function Name:
 *      rtk_dot1x_guestVlanRouteBehavior_set
 * Description:
 *      Set forwarding behavior of packets in guest vlan and routed.
 * Input:
 *      unit      - unit id
 *      fwdAction - forwarding behavior
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID    - invalid unit id
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_FWD_ACTION - invalid forwarding action
 * Applicable:
 *      8328
 * Note:
 *      Forwarding behavior is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 */
int32
rtk_dot1x_guestVlanRouteBehavior_set(uint32 unit, rtk_action_t fwdAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_guestVlanRouteBehavior_set(unit, fwdAction);
} /* end of rtk_dot1x_guestVlanRouteBehavior_set */

/* Module Name    : Dot1x                         */
/* Sub-module Name: Parameter for trapped packets */

/* Function Name:
 *      rtk_dot1x_trapPri_get
 * Description:
 *      Get priority of trapped dot1x packets.
 * Input:
 *      unit      - unit id
 * Output:
 *      pPriority - pointer to priority
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) Get the dot1x trap to cpu priority and valid range is 0-7.
 */
int32
rtk_dot1x_trapPri_get(uint32 unit, rtk_pri_t *pPriority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_trapPri_get(unit, pPriority);
} /* end of rtk_dot1x_trapPri_get */

/* Function Name:
 *      rtk_dot1x_trapPri_set
 * Description:
 *      Set priority of trapped dot1x packet.
 * Input:
 *      unit     - unit id
 *      priority - priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PRIORITY - invalid priority value
 * Applicable:
 *      8328
 * Note:
 *      (1) Config the dot1x trap to cpu priority and valid range is 0-7.
 */
int32
rtk_dot1x_trapPri_set(uint32 unit, rtk_pri_t priority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_trapPri_set(unit, priority);
} /* end of rtk_dot1x_trapPri_set */

/* Function Name:
 *      rtk_dot1x_trapPriEnable_get
 * Description:
 *      Get priority status of trapped dot1x packets.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to trap priority status
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
rtk_dot1x_trapPriEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_trapPriEnable_get(unit, pEnable);
} /* end of rtk_dot1x_trapPriEnable_get */

/* Function Name:
 *      rtk_dot1x_trapPriEnable_set
 * Description:
 *      Set priority status of trapped dot1x packet.
 * Input:
 *      unit   - unit id
 *      enable - configure trap priority status
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
rtk_dot1x_trapPriEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_trapPriEnable_set(unit, enable);
} /* end of rtk_dot1x_trapPriEnable_set */

/* Function Name:
 *      rtk_dot1x_trapAddCPUTagEnable_get
 * Description:
 *      Get enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of CPU tag adding
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
rtk_dot1x_trapAddCPUTagEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_trapAddCPUTagEnable_get(unit, pEnable);
} /* end of rtk_dot1x_trapAddCPUTagEnable_get */

/* Function Name:
 *      rtk_dot1x_trapAddCPUTagEnable_set
 * Description:
 *      Set enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit   - unit id
 *      enable - enable status of CPU tag adding
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
rtk_dot1x_trapAddCPUTagEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->dot1x_trapAddCPUTagEnable_set(unit, enable);
} /* end of rtk_dot1x_trapAddCPUTagEnable_set */
