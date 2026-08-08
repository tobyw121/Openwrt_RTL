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
 * $Revision: 52260 $
 * $Date: 2014-10-17 14:14:26 +0800 (Fri, 17 Oct 2014) $
 *
 * Purpose : Definition those public vlan APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) vlan
 *
 */
#ifndef __DAL_CYPRESS_VLAN_H__
#define __DAL_CYPRESS_VLAN_H__

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <rtk/vlan.h>

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */

/*
 * Macro Definition
 */


/*
 * Function Declaration
 */

/* Module Name : vlan */

/* Function Name:
 *      dal_cypress_vlan_init
 * Description:
 *      Initialize vlan module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Must initialize vlan module before calling any vlan APIs.
 */
extern int32
dal_cypress_vlan_init(uint32 unit);

/* Function Name:
 *      dal_cypress_vlan_create
 * Description:
 *      Create the vlan in the specified device.
 * Input:
 *      unit - unit id
 *      vid  - vlan id to be created
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_VLAN_VID   - invalid vid
 *      RT_ERR_VLAN_EXIST - vlan is exist
 * Note:
 *      1. Default FID/STG is assigned to CIST after vlan creation and
 *         FID/STG can be re-assigned later by dal_cypress_vlan_stg_set
 *      2. Default lookup mode for L2 unicast and L2/IP multicast is assigned
 *         to be based on VID
 */
extern int32
dal_cypress_vlan_create(uint32 unit, rtk_vlan_t vid);


/* Function Name:
 *      dal_cypress_vlan_destroy
 * Description:
 *      Destroy the vlan in the specified device.
 * Input:
 *      unit - unit id
 *      vid  - vlan id to be destroyed
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_destroy(uint32 unit, rtk_vlan_t vid);


/* Function Name:
 *      dal_cypress_vlan_destroyAll
 * Description:
 *      Destroy all vlans except default vlan in the specified device.
 *      If restore_default_vlan is enable, default vlan will be restored.
 * Input:
 *      unit                 - unit id
 *      restore_default_vlan - keep or restore default vlan setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      The restore argument is permit following value:
 *      - 0: remove default vlan
 *      - 1: restore default vlan
 */
extern int32
dal_cypress_vlan_destroyAll(uint32 unit, uint32 keep_and_restore_default_vlan);


/* Function Name:
 *      dal_cypress_vlan_port_add
 * Description:
 *      Add one vlan member to the specified device.
 * Input:
 *      unit     - unit id
 *      vid      - vlan id
 *      port     - port id for add
 *      is_untag - untagged or tagged member
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_VLAN_PORT_MBR_EXIST  - member port exist in the specified vlan
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_OUT_OF_RANGE         - input parameter out of range
 * Note:
 *      1. The valid value of is_untag are {0: tagged, 1: untagged}
 */
extern int32
dal_cypress_vlan_port_add(uint32 unit, rtk_vlan_t vid, rtk_port_t port, uint32 is_untag);


/* Function Name:
 *      dal_cypress_vlan_port_del
 * Description:
 *      Delete one vlan member from the specified device.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 *      port - port id for delete
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_port_del(uint32 unit, rtk_vlan_t vid, rtk_port_t port);


/* Function Name:
 *      dal_cypress_vlan_port_get
 * Description:
 *      Get the vlan members from the specified device.
 * Input:
 *      unit              - unit id
 *      vid               - vlan id
 * Output:
 *      pMember_portmask  - pointer buffer of member ports
 *      pUntag_portmask   - pointer buffer of untagged member ports
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_port_get(
    uint32         unit,
    rtk_vlan_t     vid,
    rtk_portmask_t *pMember_portmask,
    rtk_portmask_t *pUntag_portmask);


/* Function Name:
 *      dal_cypress_vlan_port_set
 * Description:
 *      Replace the vlan members in the specified device.
 * Input:
 *      unit              - unit id
 *      vid               - vlan id
 *      pMember_portmask  - member ports
 *      pUntag_portmask   - untagged member ports
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      Replace with new specified VLAN member regardless of original setting
 */
extern int32
dal_cypress_vlan_port_set(
    uint32         unit,
    rtk_vlan_t     vid,
    rtk_portmask_t *pMember_portmask,
    rtk_portmask_t *pUntag_portmask);


/* Function Name:
 *      dal_cypress_vlan_stg_get
 * Description:
 *      Get spanning tree group instance of the vlan from the specified device.
 * Input:
 *      unit  - unit id
 *      vid   - vlan id
 * Output:
 *      pStg  - pointer buffer of spanning tree group instance
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      STG instance also represents FID in RTL8390
 */
extern int32
dal_cypress_vlan_stg_get(uint32 unit, rtk_vlan_t vid, rtk_stg_t *pStg);


/* Function Name:
 *      dal_cypress_vlan_stg_set
 * Description:
 *      Set spanning tree group instance of the vlan to the specified device.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 *      stg  - spanning tree group instance
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_MSTI                 - invalid msti
 * Note:
 *      STG instance also represents FID in RTL8390
 */
extern int32
dal_cypress_vlan_stg_set(uint32 unit, rtk_vlan_t vid, rtk_stg_t stg);


/* Function Name:
 *      dal_cypress_vlan_l2UcastLookupMode_get
 * Description:
 *      Get L2 lookup mode for L2 unicast traffic.
 * Input:
 *      unit  - unit id
 *      vid   - vlan id
 * Output:
 *      pMode - lookup mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      Each VLAN can have its own lookup mode for L2 unicast traffic
 */
extern int32
dal_cypress_vlan_l2UcastLookupMode_get(uint32 unit, rtk_vlan_t vid, rtk_l2_ucastLookupMode_t *pMode);


/* Function Name:
 *      dal_cypress_vlan_l2UcastLookupMode_set
 * Description:
 *      Set L2 lookup mode for L2 unicast traffic.
 * Input:
 *      unit  - unit id
 *      vid   - vlan id
 *      mode  - lookup mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_INPUT                - Invalid input parameter
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 * Note:
 *      Each VLAN can have its own lookup mode for L2 unicast traffic
 */
extern int32
dal_cypress_vlan_l2UcastLookupMode_set(uint32 unit, rtk_vlan_t vid, rtk_l2_ucastLookupMode_t mode);


/* Function Name:
 *      dal_cypress_vlan_l2McastLookupMode_get
 * Description:
 *      Get L2 lookup mode for L2 multicast ans IP multicast traffic.
 * Input:
 *      unit  - unit id
 *      vid   - vlan id
 * Output:
 *      pMode - lookup mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      Each VLAN can have its own lookup mode for L2 mulicast traffic
 */
extern int32
dal_cypress_vlan_l2McastLookupMode_get(uint32 unit, rtk_vlan_t vid, rtk_l2_mcastLookupMode_t *pMode);


/* Function Name:
 *      dal_cypress_vlan_l2McastLookupMode_set
 * Description:
 *      Set L2 lookup mode for L2 multicast ans IP multicast traffic.
 * Input:
 *      unit  - unit id
 *      vid   - vlan id
 *      mode  - lookup mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_INPUT                - Invalid input parameter
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 * Note:
 *      Each VLAN can have its own lookup mode for L2 mulicast traffic
 */
extern int32
dal_cypress_vlan_l2McastLookupMode_set(uint32 unit, rtk_vlan_t vid, rtk_l2_mcastLookupMode_t mode);


/* Function Name:
 *      dal_cypress_vlan_profileIdx_get
 * Description:
 *      Get VLAN profile index of specified VLAN.
 * Input:
 *      unit    - unit id
 *      vid     - vlan id
 * Output:
 *      pIdx    - VLAN profile index
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      Each VLAN can map to a VLAN profile (0~7)
 */
extern int32
dal_cypress_vlan_profileIdx_get(uint32 unit, rtk_vlan_t vid, uint32 *pIdx);


/* Function Name:
 *      dal_cypress_vlan_profileIdx_set
 * Description:
 *      Set VLAN profile index of specified VLAN.
 * Input:
 *      unit    - unit id
 *      vid     - vlan id
 *      idx     - VLAN profile index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_OUT_OF_RANGE         - input value is out of range
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 * Note:
 *      Each VLAN can map to a VLAN profile (0~7)
 */
extern int32
dal_cypress_vlan_profileIdx_set(uint32 unit, rtk_vlan_t vid, uint32 idx);


/* Function Name:
 *      dal_cypress_vlan_profile_get
 * Description:
 *      Get specific VLAN profile.
 * Input:
 *      unit        - unit id
 *      idx         - VLAN profile index
 * Output:
 *      pProfile    - VLAN profile configuration
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_OUT_OF_RANGE         - profile index is out of range
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      Totally 8 profile (0~7) are supported
 */
extern int32
dal_cypress_vlan_profile_get(uint32 unit, uint32 idx, rtk_vlan_profile_t *pProfile);


/* Function Name:
 *      dal_cypress_vlan_profile_set
 * Description:
 *      Set specific VLAN profile.
 * Input:
 *      unit     - unit id
 *      idx      - VLAN profile index
 *      pProfile - VLAN profile configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_OUT_OF_RANGE         - profile index is out of range
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      Totally 8 profile (0~7) are supported
 */
extern int32
dal_cypress_vlan_profile_set(uint32 unit, uint32 idx, rtk_vlan_profile_t *pProfile);


/* Module Name     : vlan           */
/* Sub-module Name : vlan attribute */


/* Module Name     : vlan                */
/* Sub-module Name : vlan port attribute */

/* Function Name:
 *      dal_cypress_vlan_portAcceptFrameType_get
 * Description:
 *      Get inner vlan accept frame type of the port from the specified device.
 * Input:
 *      unit                - unit id
 *      port                - port id
 * Output:
 *      pAccept_frame_type  - pointer buffer of accept frame type
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The API is used for 802.1Q tagged  and if you want to get the 802.1ad
 *      accept frame type, please use dal_cypress_vlan_portOuterAcceptFrameType_get
 */
extern int32
dal_cypress_vlan_portAcceptFrameType_get(
    uint32                     unit,
    rtk_port_t                 port,
    rtk_vlan_acceptFrameType_t *pAccept_frame_type);


/* Function Name:
 *      dal_cypress_vlan_portAcceptFrameType_set
 * Description:
 *      Set inner vlan accept frame type of the port to the specified device.
 * Input:
 *      unit              - unit id
 *      port              - port id
 *      accept_frame_type - accept frame type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_PORT_ID                - invalid port id
 *      RT_ERR_VLAN_ACCEPT_FRAME_TYPE - invalid accept frame type
 *      RT_ERR_INPUT                  - invalid input parameter
 * Note:
 *      The API is used for 802.1Q tagged  and if you want to set the 802.1ad
 *      accept frame type, please use dal_cypress_vlan_portOuterAcceptFrameType_set
 */
extern int32
dal_cypress_vlan_portAcceptFrameType_set(
    uint32                     unit,
    rtk_port_t                 port,
    rtk_vlan_acceptFrameType_t accept_frame_type);


/* Function Name:
 *      dal_cypress_vlan_portOuterAcceptFrameType_get
 * Description:
 *      Get outer vlan accept frame type of the port from the specified device.
 * Input:
 *      unit                - unit id
 *      port                - port id
 * Output:
 *      pAccept_frame_type  - pointer buffer of accept frame type
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
dal_cypress_vlan_portOuterAcceptFrameType_get(
    uint32                     unit,
    rtk_port_t                 port,
    rtk_vlan_acceptFrameType_t *pAccept_frame_type);


/* Function Name:
 *      dal_cypress_vlan_portOuterAcceptFrameType_set
 * Description:
 *      Set outer vlan accept frame type of the port to the specified device.
 * Input:
 *      unit              - unit id
 *      port              - port id
 *      accept_frame_type - accept frame type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_PORT_ID                - invalid port id
 *      RT_ERR_VLAN_ACCEPT_FRAME_TYPE - invalid accept frame type
 *      RT_ERR_INPUT                  - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_portOuterAcceptFrameType_set(
    uint32                     unit,
    rtk_port_t                 port,
    rtk_vlan_acceptFrameType_t accept_frame_type);


/* Function Name:
 *      dal_cypress_vlan_portIgrFilter_get
 * Description:
 *      Get vlan ingress filter configuration of the port from the specified device.
 * Input:
 *      unit        - unit id
 *      port        - port id
 * Output:
 *      pIgr_filter - pointer buffer of ingress filter configuration
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
dal_cypress_vlan_portIgrFilter_get(uint32 unit, rtk_port_t port, rtk_vlan_ifilter_t *pIgr_filter);


/* Function Name:
 *      dal_cypress_vlan_portIgrFilter_set
 * Description:
 *      Set vlan ingress filter configuration of the port from the specified device.
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      igr_filter  - ingress filter configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_portIgrFilter_set(uint32 unit, rtk_port_t port, rtk_vlan_ifilter_t igr_filter);


/* Function Name:
 *      dal_cypress_vlan_portEgrFilterEnable_get
 * Description:
 *      Get enable status of egress filtering on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of egress filtering
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_portEgrFilterEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);


/* Function Name:
 *      dal_cypress_vlan_portEgrFilterEnable_set
 * Description:
 *      Set enable status of egress filtering on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of egress filtering
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_portEgrFilterEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);


/* Function Name:
 *      dal_cypress_vlan_mcastLeakyEnable_get
 * Description:
 *      Get vlan egress leaky status of the system from the specified device.
 * Input:
 *      unit    - unit id
 * Output:
 *      pLeaky  - pointer buffer of vlan leaky of egress
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Enable leaky function to allow L2/IP multicast traffic to across VLAN.
 *      That is, egress VLAN filtering is bypassed by L2/IP multicast traffic.
 */
extern int32
dal_cypress_vlan_mcastLeakyEnable_get(uint32 unit, rtk_enable_t *pLeaky);


/* Function Name:
 *      dal_cypress_vlan_mcastLeakyEnable_set
 * Description:
 *      Set vlan egress leaky configure of the system to the specified device.
 * Input:
 *      unit  - unit id
 *      leaky - vlan leaky of egress
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 *      RT_ERR_OUT_OF_RANGE - input value is out of range
 * Note:
 *      Enable leaky function to allow L2/IP multicast traffic to across VLAN.
 *      That is, egress VLAN filtering is bypassed by L2/IP multicast traffic.
 */
extern int32
dal_cypress_vlan_mcastLeakyEnable_set(uint32 unit, rtk_enable_t leaky);


/* Function Name:
 *      dal_cypress_vlan_portPvidMode_get
 * Description:
 *      Get the configuration of inner port-based VLAN mode.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pMode   - pointer to inner port-based VLAN mode configuration
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Inner port-based VLAN can apply on different tag format, refer to
 *      rtk_vlan_pbVlan_mode_t
 */
extern int32
dal_cypress_vlan_portPvidMode_get(uint32 unit, rtk_port_t port, rtk_vlan_pbVlan_mode_t *pMode);


/* Function Name:
 *      dal_cypress_vlan_portPvidMode_set
 * Description:
 *      Set the configuration of inner port-based VLAN mode.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      mode    - inner port-based VLAN mode configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Inner port-based VLAN can apply on different tag format, refer to
 *      rtk_vlan_pbVlan_mode_t
 */
extern int32
dal_cypress_vlan_portPvidMode_set(uint32 unit, rtk_port_t port, rtk_vlan_pbVlan_mode_t mode);


/* Function Name:
 *      dal_cypress_vlan_portOuterPvidMode_get
 * Description:
 *      Get the configuration of outer port-based VLAN mode.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pMode   - pointer to outer port-based VLAN mode configuration
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Outer port-based VLAN can apply on different tag format, refer to
 *      rtk_vlan_pbVlan_mode_t
 */
extern int32
dal_cypress_vlan_portOuterPvidMode_get(uint32 unit, rtk_port_t port, rtk_vlan_pbVlan_mode_t *pMode);


/* Function Name:
 *      dal_cypress_vlan_portOuterPvidMode_set
 * Description:
 *      Set the configuration of outer port-based VLAN mode.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      mode    - outer port-based VLAN mode configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Outer port-based VLAN can apply on different tag format, refer to
 *      rtk_vlan_pbVlan_mode_t
 */
extern int32
dal_cypress_vlan_portOuterPvidMode_set(uint32 unit, rtk_port_t port, rtk_vlan_pbVlan_mode_t mode);


/* Function Name:
 *      dal_cypress_vlan_portPvid_get
 * Description:
 *      Get port default inner vlan id from the specified device.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pPvid  - pointer buffer of port default inner vlan id
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
dal_cypress_vlan_portPvid_get(uint32 unit, rtk_port_t port, uint32 *pPvid);


/* Function Name:
 *      dal_cypress_vlan_portPvid_set
 * Description:
 *      Set port default inner vlan id to the specified device.
 * Input:
 *      unit - unit id
 *      port - port id
 *      pvid - port default inner vlan id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_VLAN_VID - invalid vid
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_portPvid_set(uint32 unit, rtk_port_t port, uint32 pvid);


/* Function Name:
 *      dal_cypress_vlan_portOuterPvid_get
 * Description:
 *      Get port default outer vlan id from the specified device.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pPvid  - pointer buffer of port default outer vlan id
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
dal_cypress_vlan_portOuterPvid_get(uint32 unit, rtk_port_t port, uint32 *pPvid);


/* Function Name:
 *      dal_cypress_vlan_portOuterPvid_set
 * Description:
 *      Set port default outer vlan id to the specified device.
 * Input:
 *      unit - unit id
 *      port - port id
 *      pvid - port default outer vlan id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_VLAN_VID - invalid vid
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_portOuterPvid_set(uint32 unit, rtk_port_t port, uint32 pvid);


/* Function Name:
 *      dal_cypress_vlan_macBasedVlan_get
 * Description:
 *      Get MAC-based vlan.
 * Input:
 *      unit     - unit id
 *      index    - entry index
 * Output:
 *      valid    - validate or invalidate the entry
 *      smac     - source mac address
 *      vid      - MAC-based VLAN ID
 *      priority - assign internal priority for untag packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_OUT_OF_RANGE         - index is out of range
 *      RT_ERR_VLAN_C2SC_BLOCK_MODE - try to set/get a entry to an incompatiable block
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_macBasedVlan_get(uint32 unit, uint32 index, uint32 *valid,
        rtk_mac_t *smac, rtk_vlan_t *vid, rtk_pri_t *priority);


/* Function Name:
 *      dal_cypress_vlan_macBasedVlan_set
 * Description:
 *      Set MAC-based vlan.
 * Input:
 *      unit     - unit id
 *      index    - entry index
 *      valid    - validate or invalidate the entry
 *      smac     - source mac address
 *      vid      - MAC-based VLAN ID
 *      priority - assign internal priority for untag packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_C2SC_BLOCK_MODE - try to set/get a entry to an incompatiable block
 *      RT_ERR_OUT_OF_RANGE         - index is out of range
 *      RT_ERR_MAC                  - invalid mac address
 *      RT_ERR_VLAN_VID             - invalid vlan id
 *      RT_ERR_PRIORITY             - invalid priority value
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_macBasedVlan_set(uint32 unit, uint32 index, uint32 valid, rtk_mac_t *smac, rtk_vlan_t vid, rtk_pri_t priority);

/* Function Name:
 *      dal_cypress_vlan_macBasedVlanWithMsk_get
 * Description:
 *      Get MAC-based vlan with source MAC address mask.
 * Input:
 *      unit     - unit id
 *      index    - entry index
 * Output:
 *      valid    - validate or invalidate the entry
 *      smac     - source mac address
 *      smsk     - source mac address mask
 *      vid      - MAC-based VLAN ID
 *      priority - assign internal priority for untag packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_OUT_OF_RANGE         - index is out of range
 *      RT_ERR_VLAN_C2SC_BLOCK_MODE - try to set/get a entry to an incompatiable block
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_macBasedVlanWithMsk_get(uint32 unit, uint32 index, uint32 *valid,
        rtk_mac_t *smac, rtk_mac_t *smsk, rtk_vlan_t *vid, rtk_pri_t *priority);

/* Function Name:
 *      dal_cypress_vlan_macBasedVlanWithMsk_set
 * Description:
 *      Set MAC-based vlan with source MAC address mask.
 * Input:
 *      unit     - unit id
 *      index    - entry index
 *      valid    - validate or invalidate the entry
 *      smac     - source mac address
 *      smsk     - source mac address mask
 *      vid      - MAC-based VLAN ID
 *      priority - assign internal priority for untag packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_C2SC_BLOCK_MODE - try to set/get a entry to an incompatiable block
 *      RT_ERR_OUT_OF_RANGE         - index is out of range
 *      RT_ERR_MAC                  - invalid mac address
 *      RT_ERR_VLAN_VID             - invalid vlan id
 *      RT_ERR_PRIORITY             - invalid priority value
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_macBasedVlanWithMsk_set(uint32 unit, uint32 index, uint32 valid,
        rtk_mac_t *smac, rtk_mac_t *smsk, rtk_vlan_t vid, rtk_pri_t priority);

/* Function Name:
 *      dal_cypress_vlan_macBasedVlanWithPort_get
 * Description:
 *      Get MAC-based vlan with source port ID mask.
 * Input:
 *      unit     - unit id
 *      index    - entry index
 * Output:
 *      valid    - validate or invalidate the entry
 *      smac     - source mac address
 *      smsk     - source mac address mask
 *      port     - source port ID
 *      pmsk     - source port ID mask
 *      vid      - MAC-based VLAN ID
 *      priority - assign internal priority for untag packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_OUT_OF_RANGE         - index is out of range
 *      RT_ERR_VLAN_C2SC_BLOCK_MODE - try to set/get a entry to an incompatiable block
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_macBasedVlanWithPort_get(uint32 unit, uint32 index, uint32 *valid,
        rtk_mac_t *smac, rtk_mac_t *smsk, 
        rtk_port_t  *port, rtk_port_t *pmsk, rtk_vlan_t *vid, rtk_pri_t *priority);

/* Function Name:
 *      dal_cypress_vlan_macBasedVlanWithPort_set
 * Description:
 *      Set MAC-based vlan with source port ID mask.
 * Input:
 *      unit     - unit id
 *      index    - entry index
 *      valid    - validate or invalidate the entry
 *      smac     - source mac address
 *      smsk     - source mac address mask
 *      port     - source port ID
 *      pmsk     - source port ID mask
 *      vid      - MAC-based VLAN ID
 *      priority - assign internal priority for untag packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_C2SC_BLOCK_MODE - try to set/get a entry to an incompatiable block
 *      RT_ERR_OUT_OF_RANGE         - index or port ID bit-mask is out of range
 *      RT_ERR_MAC                  - invalid mac address
 *      RT_ERR_VLAN_VID             - invalid vlan id
 *      RT_ERR_PRIORITY             - invalid priority value
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_macBasedVlanWithPort_set(uint32 unit, uint32 index, uint32 valid,
        rtk_mac_t *smac, rtk_mac_t *smsk, 
        rtk_port_t  port, rtk_port_t pmsk, rtk_vlan_t vid, rtk_pri_t priority);

/* Function Name:
 *      dal_cypress_vlan_ipSubnetBasedVlan_get
 * Description:
 *      Get IP-Subnet-based vlan.
 * Input:
 *      unit     - unit id
 *      index    - entry index
 * Output:
 *      valid    - validate or invalidate the entry
 *      sip      - source IP address
 *      sip_mask - source IP address mask
 *      vid      - VLAN ID
 *      priority - assign internal priority for untag packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_C2SC_BLOCK_MODE - try to set/get a entry to an incompatiable block
 *      RT_ERR_OUT_OF_RANGE         - index is out of range
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_ipSubnetBasedVlan_get(uint32 unit, uint32 index, uint32 *valid,
        ipaddr_t *sip, ipaddr_t *sip_mask, rtk_vlan_t *vid, rtk_pri_t *priority);


/* Function Name:
 *      dal_cypress_vlan_ipSubnetBasedVlan_set
 * Description:
 *      Set IP-Subnet-based vlan.
 * Input:
 *      unit     - unit id
 *      index    - entry index
 *      valid    - validate or invalidate the entry
 *      sip      - source IP address
 *      sip_mask - source IP address mask
 *      vid      - VLAN ID
 *      priority - assign internal priority for untag packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_C2SC_BLOCK_MODE - try to set/get a entry to an incompatiable block
 *      RT_ERR_OUT_OF_RANGE         - index is out of range
 *      RT_ERR_MAC                  - invalid mac address
 *      RT_ERR_VLAN_VID             - invalid vlan id
 *      RT_ERR_PRIORITY             - invalid priority value 
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_ipSubnetBasedVlan_set(uint32 unit, uint32 index, uint32 valid,
        ipaddr_t sip, ipaddr_t sip_mask, rtk_vlan_t vid, rtk_pri_t priority);

/* Function Name:
 *      dal_cypress_vlan_ipSubnetBasedVlanWithPort_get
 * Description:
 *      Get IP-Subnet-based vlan with source port ID mask.
 * Input:
 *      unit     - unit id
 *      index    - entry index
 * Output:
 *      valid    - validate or invalidate the entry
 *      sip      - source IP address
 *      sip_mask - source IP address mask
 *      port      - source port ID
 *      port_mask - source port ID mask
 *      vid      - VLAN ID
 *      priority - assign internal priority for untag packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_C2SC_BLOCK_MODE - try to set/get a entry to an incompatiable block
 *      RT_ERR_OUT_OF_RANGE         - index is out of range
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_ipSubnetBasedVlanWithPort_get(uint32 unit, uint32 index, uint32 *valid,
        ipaddr_t *sip, ipaddr_t *sip_mask, rtk_port_t *port, rtk_port_t *port_mask, rtk_vlan_t *vid, rtk_pri_t *priority);

/* Function Name:
 *      dal_cypress_vlan_ipSubnetBasedVlanWithPort_set
 * Description:
 *      Set IP-Subnet-based vlan with source port ID mask.
 * Input:
 *      unit     - unit id
 *      index    - entry index
 *      valid    - validate or invalidate the entry
 *      sip      - source IP address
 *      sip_mask - source IP address mask
 *      port      - source port ID
 *      port_mask - source port ID mask
 *      vid      - VLAN ID
 *      priority - assign internal priority for untag packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_C2SC_BLOCK_MODE - try to set/get a entry to an incompatiable block
 *      RT_ERR_OUT_OF_RANGE         - index or port ID mask is out of range
 *      RT_ERR_MAC                  - invalid mac address
 *      RT_ERR_VLAN_VID             - invalid vlan id
 *      RT_ERR_PRIORITY             - invalid priority value 
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_ipSubnetBasedVlanWithPort_set(uint32 unit, uint32 index, uint32 valid,
        ipaddr_t sip, ipaddr_t sip_mask, rtk_port_t port, rtk_port_t port_mask, rtk_vlan_t vid, rtk_pri_t priority);

/* Function Name:
 *      dal_cypress_vlan_protoGroup_get
 * Description:
 *      Get protocol group for protocol based vlan.
 * Input:
 *      unit            - unit id
 *      protoGroup_idx  - protocol group index
 * Output:
 *      pProtoGroup     - pointer to protocol group
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_OUT_OF_RANGE     - protocol group index is out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_protoGroup_get(
    uint32                  unit,
    uint32                  protoGroup_idx,
    rtk_vlan_protoGroup_t   *pProtoGroup);


/* Function Name:
 *      dal_cypress_vlan_protoGroup_set
 * Description:
 *      Set protocol group for protocol based vlan.
 * Input:
 *      unit            - unit id
 *      protoGroup_idx  - protocol group index
 *      protoGroup      - protocol group
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_OUT_OF_RANGE     - protocol group index is out of range
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_protoGroup_set(
    uint32                  unit,
    uint32                  protoGroup_idx,
    rtk_vlan_protoGroup_t   *pProtoGroup);


/* Function Name:
 *      dal_cypress_vlan_portProtoVlan_get
 * Description:
 *      Get vlan of specificed protocol group on specified port.
 * Input:
 *      unit            - unit id
 *      port            - port id
 *      protoGroup_idx  - protocol group index
 * Output:
 *      pVlan_cfg       - pointer to vlan configuration of protocol group
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_OUT_OF_RANGE     - protocol group index is out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_portProtoVlan_get(
    uint32                  unit,
    rtk_port_t              port,
    uint32                  protoGroup_idx,
    rtk_vlan_protoVlanCfg_t *pVlan_cfg);


/* Function Name:
 *      dal_cypress_vlan_portProtoVlan_set
 * Description:
 *      Set vlan of specificed protocol group on specified port.
 * Input:
 *      unit            - unit id
 *      port            - port id
 *      protoGroup_idx  - protocol group index
 *      pVlan_cfg       - vlan configuration of protocol group
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_OUT_OF_RANGE     - protocol group index is out of range
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_portProtoVlan_set(
    uint32                  unit,
    rtk_port_t              port,
    uint32                  protoGroup_idx,
    rtk_vlan_protoVlanCfg_t *pVlan_cfg);


/* Function Name:
 *      dal_cypress_vlan_portIgrTagKeepEnable_get
 * Description:
 *      Get enable status of keep tag format at ingress.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pKeepOuter - enable status of keep outer tag format
 *      pKeepInner - enable status of keep inner tag format
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Packet will be transmitted with original tag format when following condition are true.
 *      - Enable keep tag format at ingress
 *      - Enable keep tag format at egress
 */
extern int32
dal_cypress_vlan_portIgrTagKeepEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pKeepOuter, rtk_enable_t *pKeepInner);


/* Function Name:
 *      dal_cypress_vlan_portIgrTagKeepEnable_set
 * Description:
 *      Set enable status of keep tag format at ingress.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      keepOuter - enable status of keep outer tag format
 *      keepInner - enable status of keep inner tag format
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Packet will be transmitted with original tag format when following condition are true.
 *      - Enable keep tag format at ingress
 *      - Enable keep tag format at egress
 */
extern int32
dal_cypress_vlan_portIgrTagKeepEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t keepOuter, rtk_enable_t keepInner);


/* Function Name:
 *      dal_cypress_vlan_portEgrTagKeepEnable_get
 * Description:
 *      Get enable status of keep tag format at egress.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pKeepOuter - pointer to enable status of keep outer tag format
 *      pKeepInner - pointer to enable status of keep inner tag format
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Packet will be transmitted with original tag format when following condition are true.
 *      - Enable keep tag format at ingress
 *      - Enable keep tag format at egress
 */
extern int32
dal_cypress_vlan_portEgrTagKeepEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pKeepOuter, rtk_enable_t *pKeepInner);


/* Function Name:
 *      dal_cypress_vlan_portEgrTagKeepEnable_set
 * Description:
 *      Set enable status of keep tag format at egress.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      keepOuter - enable status of keep outer tag format
 *      keepInner - enable status of keep inner tag format
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Packet will be transmitted with original tag format when following condition are true.
 *      - Enable keep tag format at ingress
 *      - Enable keep tag format at egress
 */
extern int32
dal_cypress_vlan_portEgrTagKeepEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t keepOuter, rtk_enable_t keepInner);

/* Function Name:
 *      dal_cypress_vlan_igrVlanCnvtBlkMode_get
 * Description:
 *      Get the operation mode of ingress VLAN conversion table block.
 * Input:
 *      unit    - unit id
 *      blk_idx - block index
 * Output:
 *      pMode   - operation mode of ingress VLAN conversion block
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - block index is out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. Valid block index ranges from 0 to 3.
 *      2. Ingress VLAN conversion table block can be used for doing ingress VLAN conversion
 *         or MAC-basd VLAN or IP-Subnet-based VLAN.
 */
extern int32
dal_cypress_vlan_igrVlanCnvtBlkMode_get(uint32 unit, uint32 blk_idx, rtk_vlan_igrVlanCnvtBlk_mode_t *pMode);


/* Function Name:
 *      dal_cypress_vlan_igrVlanCnvtBlkMode_set
 * Description:
 *      Get the operation mode of ingress VLAN conversion table block.
 * Input:
 *      unit    - unit id
 *      blk_idx - block index
 *      mode    - operation mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - block index is out of range
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      1. Valid block index ranges from 0 to 3.
 *      2. Ingress VLAN conversion table block can be used for doing ingress VLAN conversion
 *         or MAC-basd VLAN or IP-Subnet-based VLAN.
 */
extern int32
dal_cypress_vlan_igrVlanCnvtBlkMode_set(uint32 unit, uint32 blk_idx, rtk_vlan_igrVlanCnvtBlk_mode_t mode);


/* Function Name:
 *      dal_cypress_vlan_igrVlanCnvtEntry_get
 * Description:
 *      Get ingress VLAN conversion (C2SC) entry.
 * Input:
 *      unit  - unit id
 *      index - entry index
 * Output:
 *      pData - configuration of ingress VLAN conversion (C2SC) entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_OUT_OF_RANGE         - entry index is out of range
 *      RT_ERR_VLAN_C2SC_BLOCK_MODE - try to set/get a entry to an incompatiable block
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_igrVlanCnvtEntry_get(uint32 unit, uint32 index, rtk_vlan_igrVlanCnvtEntry_t *pData);


/* Function Name:
 *      dal_cypress_vlan_igrVlanCnvtEntry_set
 * Description:
 *      Set ingress VLAN conversion (C2SC) entry.
 * Input:
 *      unit  - unit id
 *      index - entry index
 * Output:
 *      pData - configuration of ingress VLAN conversion (C2SC) entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_OUT_OF_RANGE         - entry index is out of range
 *      RT_ERR_VLAN_C2SC_BLOCK_MODE - try to set/get a entry to an incompatiable block
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_QOS_1P_PRIORITY      - Invalid 802.1p priority
 *      RT_ERR_VLAN_TPID_INDEX      - Invalid TPID index
 *      RT_ERR_INPUT                - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_igrVlanCnvtEntry_set(uint32 unit, uint32 index, rtk_vlan_igrVlanCnvtEntry_t *pData);


/* Function Name:
 *      dal_cypress_vlan_egrVlanCnvtDblTagEnable_get
 * Description:
 *      Enable egress VLAN conversion for dobule(outer+inner) tagged packet.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_egrVlanCnvtDblTagEnable_get(uint32 unit, rtk_enable_t *pEnable);


/* Function Name:
 *      dal_cypress_vlan_egrVlanCnvtDblTagEnable_set
 * Description:
 *      Enable egress VLAN conversion for dobule(outer+inner) tagged packet.
 * Input:
 *      unit    - unit id
 *      enable  - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_egrVlanCnvtDblTagEnable_set(uint32 unit, rtk_enable_t enable);


/* Function Name:
 *      dal_cypress_vlan_egrVlanCnvtVidSource_get
 * Description:
 *      Specify the VID source for doing egress VLAN conversion.
 * Input:
 *      unit    - unit id
 * Output:
 *      pSrc    - pointer to VID source
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_egrVlanCnvtVidSource_get(uint32 unit, rtk_l2_vlanMode_t *pSrc);


/* Function Name:
 *      dal_cypress_vlan_egrVlanCnvtVidSource_set
 * Description:
 *      Specify the VID source for doing egress VLAN conversion.
 * Input:
 *      unit    - unit id
 *      src     - VID source
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_egrVlanCnvtVidSource_set(uint32 unit, rtk_l2_vlanMode_t src);


/* Function Name:
 *      dal_cypress_vlan_egrVlanCnvtEntry_get
 * Description:
 *      Get egress VLAN conversion (SC2C) entry.
 * Input:
 *      unit  - unit id
 *      index - entry index
 * Output:
 *      pData - configuration of egress VLAN conversion (SC2C) entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - entry index is out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_egrVlanCnvtEntry_get(uint32 unit, uint32 index, rtk_vlan_egrVlanCnvtEntry_t *pData);


/* Function Name:
 *      dal_cypress_vlan_egrVlanCnvtEntry_set
 * Description:
 *      Set egress VLAN conversion (SC2C) entry.
 * Input:
 *      unit  - unit id
 *      index - entry index
 * Output:
 *      pData - configuration of egress VLAN conversion (SC2C) entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_OUT_OF_RANGE    - entry index is out of range
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 *      RT_ERR_VLAN_VID        - invalid vid
 *      RT_ERR_PORT_ID         - invalid port id
 *      RT_ERR_QOS_1P_PRIORITY - Invalid 802.1p priority
 *      RT_ERR_VLAN_TPID_INDEX - Invalid TPID index
 *      RT_ERR_INPUT           - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_egrVlanCnvtEntry_set(uint32 unit, uint32 index, rtk_vlan_egrVlanCnvtEntry_t *pData);


/* Function Name:
 *      dal_cypress_vlan_innerTpidEntry_get
 * Description:
 *      Get inner TPID value from global inner TPID pool.
 * Input:
 *      unit     - unit id
 *      tpid_idx - index of TPID entry
 * Output:
 *      pTpid    - pointer to TPID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_OUT_OF_RANGE    - entry index is out of range
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 *      RT_ERR_VLAN_VID        - invalid vid
 *      RT_ERR_PORT_ID         - invalid port id
 *      RT_ERR_QOS_1P_PRIORITY - Invalid 802.1p priority
 *      RT_ERR_VLAN_TPID_INDEX - Invalid TPID index
 *      RT_ERR_INPUT           - invalid input parameter
 * Note:
 *      Global four inner TPID can be specified.
 */
extern int32
dal_cypress_vlan_innerTpidEntry_get(uint32 unit, uint32 tpid_idx, uint32 *pTpid);


/* Function Name:
 *      dal_cypress_vlan_innerTpidEntry_set
 * Description:
 *      Set inner TPID value to global inner TPID pool.
 * Input:
 *      unit     - unit id
 *      tpid_idx - index of TPID entry
 *      tpid     - TPID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      Global four inner TPID can be specified.
 */
extern int32
dal_cypress_vlan_innerTpidEntry_set(uint32 unit, uint32 tpid_idx, uint32 tpid);


/* Function Name:
 *      dal_cypress_vlan_outerTpidEntry_get
 * Description:
 *      Get outer TPID value from global outer TPID pool.
 * Input:
 *      unit     - unit id
 *      tpid_idx - index of TPID entry
 * Output:
 *      pTpid    - pointer to TPID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Global four outer TPID can be specified.
 */
extern int32
dal_cypress_vlan_outerTpidEntry_get(uint32 unit, uint32 tpid_idx, uint32 *pTpid);


/* Function Name:
 *      dal_cypress_vlan_outerTpidEntry_set
 * Description:
 *      Set outer TPID value to global outer TPID pool.
 * Input:
 *      unit     - unit id
 *      tpid_idx - index of TPID entry
 *      tpid     - TPID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      Global four outer TPID can be specified.
 */
extern int32
dal_cypress_vlan_outerTpidEntry_set(uint32 unit, uint32 tpid_idx, uint32 tpid);


/* Function Name:
 *      dal_cypress_vlan_extraTpidEntry_get
 * Description:
 *      Get the TPID value of extra tag.
 * Input:
 *      unit     - unit id
 *      tpid_idx - index of TPID entry
 * Output:
 *      pTpid    - pointer to TPID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      1. Only one extra tag is supported. System will bypass extra tag
 *         and keeps parsing the remaining payload.
 *      2. Following tag combination are supported for parsing an extra tag:
 *         outer+innet+extra, outer+extra, inner+extra
 */
extern int32
dal_cypress_vlan_extraTpidEntry_get(uint32 unit, uint32 tpid_idx, uint32 *pTpid);


/* Function Name:
 *      dal_cypress_vlan_extraTpidEntry_set
 * Description:
 *      Set TPID value of extra tag.
 * Input:
 *      unit     - unit id
 *      tpid_idx - index of TPID entry
 *      tpid     - TPID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 * Note:
 *      1. Only one extra tag is supported. System will bypass extra tag
 *         and keeps parsing the remaining payload.
 *      2. Following tag combination are supported for parsing an extra tag:
 *         outer+innet+extra, outer+extra, inner+extra
 */
extern int32
dal_cypress_vlan_extraTpidEntry_set(uint32 unit, uint32 tpid_idx, uint32 tpid);


/* Function Name:
 *      dal_cypress_vlan_portIgrInnerTpid_get
 * Description:
 *      Get TPIDs of inner tag at ingress.
 * Input:
 *      unit             - unit id
 *      port             - port id
 * Output:
 *      pTpid_idx_mask - pointer to mask for index of tpid entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Specify which TPID to compare from TPID pool for parsing a inner-tagged packet
 */
extern int32
dal_cypress_vlan_portIgrInnerTpid_get(uint32 unit, rtk_port_t port, uint32 *pTpid_idx_mask);


/* Function Name:
 *      dal_cypress_vlan_portIgrInnerTpid_set
 * Description:
 *      Set TPIDs of inner tag at ingress.
 * Input:
 *      unit            - unit id
 *      port            - port id
 *      tpid_idx_mask   - mask for index of tpid entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Specify which TPID to compare from TPID pool for parsing a inner-tagged packet
 */
extern int32
dal_cypress_vlan_portIgrInnerTpid_set(uint32 unit, rtk_port_t port, uint32 tpid_idx_mask);

/* Function Name:
 *      dal_cypress_vlan_portIgrOuterTpid_get
 * Description:
 *      Get TPIDs of outer tag at ingress.
 * Input:
 *      unit            - unit id
 *      port            - port id
 * Output:
 *      pTpid_idx_mask  - pointer to mask for index of tpid entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Specify which TPID to compare from TPID pool for parsing a outer-tagged packet
 */
extern int32
dal_cypress_vlan_portIgrOuterTpid_get(uint32 unit, rtk_port_t port, uint32 *pTpid_idx_mask);


/* Function Name:
 *      dal_cypress_vlan_portIgrOuterTpid_set
 * Description:
 *      Set TPIDs of outer tag at ingress.
 * Input:
 *      unit            - unit id
 *      port            - port id
 *      tpid_idx_mask   - mask for index of tpid entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Specify which TPID to compare from TPID pool for parsing a outer-tagged packet
 */
extern int32
dal_cypress_vlan_portIgrOuterTpid_set(uint32 unit, rtk_port_t port, uint32 tpid_idx_mask);


/* Function Name:
 *      dal_cypress_vlan_portIgrExtraTagEnable_get
 * Description:
 *      Get enable state of extra tag comparsion.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      rtk_enable_t    - pointer to the extra tag comparsion state
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
dal_cypress_vlan_portIgrExtraTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);


/* Function Name:
 *      dal_cypress_vlan_portIgrExtraTagEnable_set
 * Description:
 *      Enable extra tag comparsion.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - extra tag comparsion state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_portIgrExtraTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);


/* Function Name:
 *      dal_cypress_vlan_portVlanAggrEnable_get
 * Description:
 *      Enable N:1 VLAN aggregation support on egress port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pEnable - pointer to enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_portVlanAggrEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);


/* Function Name:
 *      dal_cypress_vlan_portVlanAggrEnable_set
 * Description:
 *      Enable N:1 VLAN aggregation support on egress port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_portVlanAggrEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);


/* Function Name:
 *      dal_cypress_vlan_portEgrInnerTagSts_get
 * Description:
 *      Set inner tag status of egress port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pSts   - tag status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_portEgrInnerTagSts_get(uint32 unit, rtk_port_t port, rtk_vlan_tagSts_t *pSts);


/* Function Name:
 *      dal_cypress_vlan_portEgrInnerTagSts_set
 * Description:
 *      Set inner tag status of egress port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      sts    - tag status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_portEgrInnerTagSts_set(uint32 unit, rtk_port_t port, rtk_vlan_tagSts_t sts);


/* Function Name:
 *      dal_cypress_vlan_portEgrOuterTagSts_get
 * Description:
 *      Set inner tag status of egress port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pSts   - tag status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_portEgrOuterTagSts_get(uint32 unit, rtk_port_t port, rtk_vlan_tagSts_t *pSts);


/* Function Name:
 *      dal_cypress_vlan_portEgrOuterTagSts_set
 * Description:
 *      Set inner tag status of egress port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      sts    - tag status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_portEgrOuterTagSts_set(uint32 unit, rtk_port_t port, rtk_vlan_tagSts_t sts);

/* Function Name:
 *      dal_cypress_vlan_portEgrInnerTpid_get
 * Description:
 *      Get inner TPID for inner tag encapsulation when transmiiting a inner-tagged pacekt.
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pTpid_idx - pointer to index of inner TPID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      (1) The valid range of tpid_idx is 0~3.
 *      (2) The inner TPID is used when transmiiting a inner-tagged pacekt only if it is received as
 *          an inner-untag pacekt.
 */
extern int32
dal_cypress_vlan_portEgrInnerTpid_get(uint32 unit, rtk_port_t port,
        uint32 *pTpid_idx);

/* Function Name:
 *      dal_cypress_vlan_portEgrInnerTpid_set
 * Description:
 *      Set inner TPID for inner tag encapsulation when transmiiting a inner-tagged pacekt.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      tpid_idx - index of inner TPID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 * Note:
 *      (1) The valid range of tpid_idx is 0~3.
 *      (2) The inner TPID is used when transmiiting a inner-tagged pacekt only if it is received as
 *          an inner-untag pacekt.
 */
extern int32
dal_cypress_vlan_portEgrInnerTpid_set(uint32 unit, rtk_port_t port,
        uint32 tpid_idx);

/* Function Name:
 *      dal_cypress_vlan_portEgrOuterTpid_get
 * Description:
 *      Get outer TPID for outer tag encapsulation when transmiiting a outer-tagged pacekt.
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pTpid_idx - pointer to index of outer TPID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_cypress_vlan_portEgrOuterTpid_get(uint32 unit, rtk_port_t port,
        uint32 *pTpid_idx);

/* Function Name:
 *      dal_cypress_vlan_portEgrOuterTpid_set
 * Description:
 *      Set outer TPID for outer tag encapsulation when transmiiting a outer-tagged pacekt.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      tpid_idx - index of outer TPID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 * Note:
 *      None
 */
int32
dal_cypress_vlan_portEgrOuterTpid_set(uint32 unit, rtk_port_t port,
        uint32 tpid_idx);

/* Function Name:
 *      dal_cypress_vlan_leakyStpFilter_get
 * Description:
 *      Get leaky STP filter status for multicast leaky is enabled.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to status of leaky STP filter
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_leakyStpFilter_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_cypress_vlan_leakyStpFilter_set
 * Description:
 *      Set leaky STP filter status for multicast leaky is enabled.
 * Input:
 *      unit    - unit id
 *      enable  - status of leaky STP filter
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_leakyStpFilter_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      dal_cypress_vlan_except_get
 * Description:
 *      Get VLAN except action.
 * Input:
 *      unit    - unit id
 * Output:
 *      pAction - pointer to action of VLAN except
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following
 *      - ACTION_DROP
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 */
extern int32
dal_cypress_vlan_except_get(uint32 unit, rtk_action_t *pAction);

/* Function Name:
 *      dal_cypress_vlan_except_set
 * Description:
 *      Set VLAN except action.
 * Input:
 *      unit    - unit id
 *      action  - action of VLAN except
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      Forwarding action is as following
 *      - ACTION_DROP
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 */
extern int32
dal_cypress_vlan_except_set(uint32 unit, rtk_action_t action);

/* Function Name:
 *      dal_cypress_vlan_portIgrCnvtDfltAct_get
 * Description:
 *      Set action for packet without hit ingress ACL and C2SC.
 * Input:
 *      unit    - unit id
 *      port     - port id for configure
 * Output:
 *      pAction - pointer to VLAN conversion default action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following
 *      - ACTION_DROP
 *      - ACTION_FORWARD
 */
extern int32
dal_cypress_vlan_portIgrCnvtDfltAct_get(uint32 unit, rtk_port_t port,
        rtk_action_t *pAction);

/* Function Name:
 *      dal_cypress_vlan_portIgrCnvtDfltAct_set
 * Description:
 *      Set action for packet without hit ingress ACL and C2SC.
 * Input:
 *      unit    - unit id
 *      port     - port id for configure
 *      action  - VLAN conversion default action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      Forwarding action is as following
 *      - ACTION_DROP
 *      - ACTION_FORWARD
 */
extern int32
dal_cypress_vlan_portIgrCnvtDfltAct_set(uint32 unit, rtk_port_t port,
        rtk_action_t action);

/* Function Name:
 *      dal_cypress_vlan_igrVlanCnvtEntry_delAll
 * Description:
 *      Delete all ingress VLAN conversion entry.
 * Input:
 *      unit  - unit id
 *      index - entry index
 * Output:
 *      pData - configuration of egress VLAN conversion entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_igrVlanCnvtEntry_delAll(uint32 unit);

/* Function Name:
 *      dal_cypress_vlan_egrVlanCnvtEntry_delAll
 * Description:
 *      Delete all egress VLAN conversion entry.
 * Input:
 *      unit  - unit id
 *      index - entry index
 * Output:
 *      pData - configuration of egress VLAN conversion entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_egrVlanCnvtEntry_delAll(uint32 unit);

/* Function Name:
 *      dal_cypress_vlan_egrVlanCnvtRangeCheckVid_get
 * Description:
 *      Get the configuration of VID range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 * Output:
 *      pData - configuration of VID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_egrVlanCnvtRangeCheckVid_get(uint32 unit, uint32 index,
    rtk_vlan_egrVlanCnvtRangeCheck_vid_t *pData);

/* Function Name:
 *      dal_cypress_vlan_egrVlanCnvtRangeCheckVid_set
 * Description:
 *      Set the configuration of VID range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 *      pData - configuration of VID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_cypress_vlan_egrVlanCnvtRangeCheckVid_set(uint32 unit, uint32 index,
    rtk_vlan_egrVlanCnvtRangeCheck_vid_t *pData);

#if defined(CONFIG_SDK_DRIVER_RTK_BACKWARD_COMPATIBLE_8328)
/*
 * For some feature in 8390 can full compatible the original 8328 API case, 
 * SDK provide those backward compatible RTL8328 APIs for customer. 
 * If customer only need the original 8328 feature, can use those API and nothing to change.
 * If customer want to use the enhance 8390 feature, need to call the new 8390 APIs.
 */

/* Function Name:
 *      dal_cypress_vlan_fid_get
 * Description:
 *      Get the filtering database id of the vlan from the specified device.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 * Output:
 *      pFid - pointer buffer of filtering database id
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      STG instance also represents FID in RTL8390
 */
extern int32
dal_cypress_vlan_fid_get(uint32 unit, rtk_vlan_t vid, rtk_fid_t *pFid);

/* Function Name:
 *      dal_cypress_vlan_fid_set
 * Description:
 *      Set the filter id of the vlan to the specified device.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 *      fid  - filter id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_OUT_OF_RANGE         - input parameter out of range
 * Note:
 *      STG instance also represents FID in RTL8390
 */
extern int32
dal_cypress_vlan_fid_set(uint32 unit, rtk_vlan_t vid, rtk_fid_t fid);

/* Function Name:
 *      dal_cypress_vlan_portIgrFilterEnable_get
 * Description:
 *      Get vlan ingress filter status of the port from the specified device.
 * Input:
 *      unit        - unit id
 *      port        - port id
 * Output:
 *      pIgr_filter - pointer buffer of ingress filter status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The function is backward compatible RTL8328 APIs. 
 *      The 'drop' action is return enable; 'forward' or 'trap' action is return disable.
 */
extern int32
dal_cypress_vlan_portIgrFilterEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pIgr_filter);

/* Function Name:
 *      dal_cypress_vlan_portIgrFilterEnable_set
 * Description:
 *      Set vlan ingress filter status of the port to the specified device.
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      igr_filter  - ingress filter status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      The function is backward compatible RTL8328 APIs. 
 *      Congiure enable is mapped to 'drop' action; disable is mapped to 'forward' action.
 */
extern int32
dal_cypress_vlan_portIgrFilterEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t igr_filter);

/* Function Name:
 *      dal_cypress_vlan_portEgrInnerTagEnable_get
 * Description:
 *      Get enable status of inner tag capability at egress.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of inner tag capability
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The function is backward compatible RTL8328 APIs.
 *      The 'tag' is return enable; 'untag' is return disable.
 */
extern int32
dal_cypress_vlan_portEgrInnerTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_cypress_vlan_portEgrInnerTagEnable_set
 * Description:
 *      Set enable status of inner tag capability at egress.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of inner tag capability
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      The function is backward compatible RTL8328 APIs.
 *      The 'enable' is mapped to tag; 'disable' is mapped to untagg.
 */
extern int32
dal_cypress_vlan_portEgrInnerTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_cypress_vlan_portEgrOuterTagEnable_get
 * Description:
 *      Get enable status of outer tag capability at egress.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of outer tag capability
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The function is backward compatible RTL8328 APIs.
 *      The 'tag' is return enable; 'untag' is return disable.
 */
extern int32
dal_cypress_vlan_portEgrOuterTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_cypress_vlan_portEgrOuterTagEnable_set
 * Description:
 *      Set enable status of outer tag capability at egress.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of outer tag capability
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      The function is backward compatible RTL8328 APIs.
 *      The 'enable' is mapped to tag; 'disable' is mapped to untagg.
 */
extern int32
dal_cypress_vlan_portEgrOuterTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

#endif
#endif /* __DAL_CYPRESS_VLAN_H__ */
