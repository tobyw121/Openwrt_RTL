/*
 * Copyright(c) Realtek Semiconductor Corporation, 2009
 * All rights reserved.
 *
 * $Revision: 17393 $
 * $Date: 2011-04-28 18:08:42 +0800 (Thu, 28 Apr 2011) $
 *
 * Purpose : Definition those public vlan APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) Vlan table configure and modification
 *           2) Accept frame type
 *           3) Vlan ingress/egress filter
 *           4) Port based and protocol based vlan
 *           5) TPID configuration
 *           6) Ingress tag handling
 *           7) Tag format handling
 *
 */

#ifndef __DAL_ESW_VLAN_H__
#define __DAL_ESW_VLAN_H__


/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/vlan.h>


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
 *      dal_esw_vlan_init
 * Description:
 *      Initialize vlan module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      Must initialize vlan module before calling any vlan APIs.
 */
extern int32
dal_esw_vlan_init(uint32 unit);

/* Module Name    : Vlan                                  */
/* Sub-module Name: Vlan table configure and modification */

/* Function Name:
 *      dal_esw_vlan_create
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
 *      RT_ERR_UNIT_ID    - invalid unit id
 *      RT_ERR_VLAN_VID   - invalid vid
 *      RT_ERR_VLAN_EXIST - vlan is exist
 * Note:
 *      1. Default FID and STG be assigned after vlan create.
 *         Default FID is equal with VID (IVL mode) and default STG is CIST.
 *      2. FID and STG can be reassigned later by following APIs.
 *         dal_esw_vlan_fid_set
 *         dal_esw_vlan_stg_set
 */
extern int32
dal_esw_vlan_create(uint32 unit, rtk_vlan_t vid);

/* Function Name:
 *      dal_esw_vlan_destroy
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
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 * Note:
 *      None
 */
extern int32
dal_esw_vlan_destroy(uint32 unit, rtk_vlan_t vid);

/* Function Name:
 *      dal_esw_vlan_destroyAll
 * Description:
 *      Destroy all vlans except default vlan in the specified device.
 * Input:
 *      unit                 - unit id
 *      restore_default_vlan - keep and restore default vlan id or not?
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 * Note:
 *      The restore argument is permit following value:
 *      - 0: remove default vlan
 *      - 1: restore default vlan
 */
extern int32
dal_esw_vlan_destroyAll(uint32 unit, uint32 restore_default_vlan);

/* Function Name:
 *      dal_esw_vlan_fid_get
 * Description:
 *      Get the filter id of the vlan from the specified device.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 * Output:
 *      pFid - pointer buffer of filter id
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      1. fid is equal 0 after vlan create.
 *      2. You don't need to care fid when you use the IVL mode.
 *      3. The API should be used for SVL mode.
 */
extern int32
dal_esw_vlan_fid_get(uint32 unit, rtk_vlan_t vid, rtk_fid_t *pFid);

/* Function Name:
 *      dal_esw_vlan_fid_set
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
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_OUT_OF_RANGE         - input parameter out of range
 * Note:
 *      1. fid is equal 0 after vlan create.
 *      2. You don't need to care fid when you use the IVL mode.
 *      3. The API should be used for SVL mode.
 */
extern int32
dal_esw_vlan_fid_set(uint32 unit, rtk_vlan_t vid, rtk_fid_t fid);

/* Function Name:
 *      dal_esw_vlan_port_add
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
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_VLAN_PORT_MBR_EXIST  - member port exist in the specified vlan
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_OUT_OF_RANGE         - input parameter out of range
 * Note:
 *      1. The valid value of is_untag are {0: tagged, 1: untagged}
 */
extern int32
dal_esw_vlan_port_add(uint32 unit, rtk_vlan_t vid, rtk_port_t port, uint32 is_untag);

/* Function Name:
 *      dal_esw_vlan_port_del
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
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 * Note:
 *      None
 */
extern int32
dal_esw_vlan_port_del(uint32 unit, rtk_vlan_t vid, rtk_port_t port);

/* Function Name:
 *      dal_esw_vlan_port_get
 * Description:
 *      Get the vlan members from the specified device.
 * Input:
 *      unit             - unit id
 *      vid              - vlan id
 * Output:
 *      pMember_portmask - pointer buffer of member ports
 *      pUntag_portmask  - pointer buffer of untagged member ports
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_vlan_port_get(
    uint32         unit,
    rtk_vlan_t     vid,
    rtk_portmask_t *pMember_portmask,
    rtk_portmask_t *pUntag_portmask);

/* Function Name:
 *      dal_esw_vlan_port_set
 * Description:
 *      Replace the vlan members in the specified device.
 * Input:
 *      unit             - unit id
 *      vid              - vlan id
 *      pMember_portmask - member ports
 *      pUntag_portmask  - untagged member ports
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      1. Don't care the original vlan members and replace with new configure
 *         directly.
 */
extern int32
dal_esw_vlan_port_set(
    uint32         unit,
    rtk_vlan_t     vid,
    rtk_portmask_t *pMember_portmask,
    rtk_portmask_t *pUntag_portmask);

/* Module Name    : Vlan              */
/* Sub-module Name: Accept frame type */

/* Function Name:
 *      dal_esw_vlan_portAcceptFrameType_get
 * Description:
 *      Get vlan accept frame type of the port from the specified device.
 * Input:
 *      unit               - unit id
 *      port               - port id
 * Output:
 *      pAccept_frame_type - pointer buffer of accept frame type
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. The accept frame type as following:
 *         - ACCEPT_FRAME_TYPE_ALL
 *         - ACCEPT_FRAME_TYPE_TAG_ONLY
 *         - ACCEPT_FRAME_TYPE_UNTAG_ONLY
 *      2. The API is used for 802.1Q tagged  and if you want to get the 802.1ad
 *         accept frame type, please use dal_esw_vlan_portAcceptFrameType_get
 */
extern int32
dal_esw_vlan_portAcceptFrameType_get(
    uint32                     unit,
    rtk_port_t                 port,
    rtk_vlan_acceptFrameType_t *pAccept_frame_type);

/* Function Name:
 *      dal_esw_vlan_portAcceptFrameType_set
 * Description:
 *      Set vlan accept frame type of the port to the specified device.
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
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_PORT_ID                - invalid port id
 *      RT_ERR_VLAN_ACCEPT_FRAME_TYPE - invalid accept frame type
 *      RT_ERR_CHIP_NOT_SUPPORTED     - functions not supported by this chip model
 * Note:
 *      1. The accept frame type as following:
 *         - ACCEPT_FRAME_TYPE_ALL
 *         - ACCEPT_FRAME_TYPE_TAG_ONLY
 *         - ACCEPT_FRAME_TYPE_UNTAG_ONLY
 *      2. The API is used for 802.1Q tagged  and if you want to set the 802.1ad
 *         accept frame type, please use dal_esw_vlan_portAcceptFrameType_set
 */
extern int32
dal_esw_vlan_portAcceptFrameType_set(
    uint32                     unit,
    rtk_port_t                 port,
    rtk_vlan_acceptFrameType_t accept_frame_type);

/* Function Name:
 *      dal_esw_vlan_portOuterAcceptFrameType_get
 * Description:
 *      Get accept frame type of outer tag on specified port.
 * Input:
 *      unit               - unit id
 *      port               - port id
 * Output:
 *      pAccept_frame_type - pointer to accept frame type
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Accept frame type is as following:
 *      - ACCEPT_FRAME_TYPE_ALL
 *      - ACCEPT_FRAME_TYPE_TAG_ONLY
 *      - ACCEPT_FRAME_TYPE_UNTAG_ONLY
 */
extern int32
dal_esw_vlan_portOuterAcceptFrameType_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_vlan_acceptFrameType_t  *pAccept_frame_type);

/* Function Name:
 *      dal_esw_vlan_portOuterAcceptFrameType_set
 * Description:
 *      Set accept frame type of outer tag on specified port.
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
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_PORT_ID                - invalid port id
 *      RT_ERR_VLAN_ACCEPT_FRAME_TYPE - invalid accept frame type
 * Note:
 *      Accept frame type is as following:
 *      - ACCEPT_FRAME_TYPE_ALL
 *      - ACCEPT_FRAME_TYPE_TAG_ONLY
 *      - ACCEPT_FRAME_TYPE_UNTAG_ONLY
 */
extern int32
dal_esw_vlan_portOuterAcceptFrameType_set(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_vlan_acceptFrameType_t  accept_frame_type);

/* Module Name    : Vlan                       */
/* Sub-module Name: Vlan ingress/egress filter */

/* Function Name:
 *      dal_esw_vlan_portIgrFilterEnable_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. The status of vlan function is as following:
 *      - DISABLED
 *      - ENABLED
 */
extern int32
dal_esw_vlan_portIgrFilterEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pIgr_filter);


/* Function Name:
 *      dal_esw_vlan_portIgrFilterEnable_set
 * Description:
 *      Set vlan ingress filter status of the port to the specified device.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      igr_filter - ingress filter configure
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
 *      1. If the chip is per system configuration, set any port will apply to
 *      whole device. like RTL8329/RTL8389 chip.
 *      2. The status of vlan ingress filter is as following:
 *      - DISABLED
 *      - ENABLED
 */
extern int32
dal_esw_vlan_portIgrFilterEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t igr_filter);

/* Function Name:
 *      dal_esw_vlan_portEgrFilterEnable_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_vlan_portEgrFilterEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_vlan_portEgrFilterEnable_set
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
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_vlan_portEgrFilterEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Module Name    : Vlan                               */
/* Sub-module Name: Port based and protocol based vlan */

/* Function Name:
 *      dal_esw_vlan_portPvid_get
 * Description:
 *      Get port default vlan id from the specified device.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pPvid - pointer buffer of port default vlan id
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_vlan_portPvid_get(uint32 unit, rtk_port_t port, uint32 *pPvid);

/* Function Name:
 *      dal_esw_vlan_portPvid_set
 * Description:
 *      Set port default vlan id to the specified device.
 * Input:
 *      unit - unit id
 *      port - port id
 *      pvid - port default vlan id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_VLAN_VID - invalid vid
 * Note:
 *      None
 */
extern int32
dal_esw_vlan_portPvid_set(uint32 unit, rtk_port_t port, uint32 pvid);

/* Function Name:
 *      dal_esw_vlan_portOuterPvid_get
 * Description:
 *      Get outer port based vlan on specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pPvid - pointer to outer port based vlan id
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_vlan_portOuterPvid_get(uint32 unit, rtk_port_t port, rtk_vlan_t *pPvid);

/* Function Name:
 *      dal_esw_vlan_portOuterPvid_set
 * Description:
 *      Set outer port based vlan on specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 *      pvid - outer port based vlan id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_VLAN_VID - invalid vlan id
 * Note:
 *      None
 */
extern int32
dal_esw_vlan_portOuterPvid_set(uint32 unit, rtk_port_t port, rtk_vlan_t pvid);

/* Function Name:
 *      dal_esw_vlan_protoGroup_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_OUT_OF_RANGE - protocol group index is out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_vlan_protoGroup_get(
    uint32                  unit,
    uint32                  protoGroup_idx,
    rtk_vlan_protoGroup_t   *pProtoGroup);

/* Function Name:
 *      dal_esw_vlan_protoGroup_set
 * Description:
 *      Set protocol group for protocol based vlan.
 * Input:
 *      unit           - unit id
 *      protoGroup_idx - protocol group index
 *      pProtoGroup    - pointer to protocol group
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_VLAN_FRAME_TYPE - invalid frame type
 *      RT_ERR_OUT_OF_RANGE    - protocol group index is out of range
 *      RT_ERR_INPUT           - invalid input parameter
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 * Note:
 *      Frame type is as following:
 *      - FRAME_TYPE_ETHERNET
 *      - FRAME_TYPE_RFC1042 (SNAP)
 *      - FRAME_TYPE_LLCOTHER
 */
extern int32
dal_esw_vlan_protoGroup_set(
    uint32                  unit,
    uint32                  protoGroup_idx,
    rtk_vlan_protoGroup_t   *pProtoGroup);

/* Function Name:
 *      dal_esw_vlan_portProtoVlan_get
 * Description:
 *      Get vlan of specificed protocol group on specified port.
 * Input:
 *      unit           - unit id
 *      port           - port id
 *      protoGroup_idx - protocol group index
 * Output:
 *      pVlan_cfg      - pointer to vlan configuration of protocol group
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_OUT_OF_RANGE - protocol group index is out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_vlan_portProtoVlan_get(
    uint32                  unit,
    rtk_port_t              port,
    uint32                  protoGroup_idx,
    rtk_vlan_protoVlanCfg_t *pVlan_cfg);

/* Function Name:
 *      dal_esw_vlan_portProtoVlan_set
 * Description:
 *      Set vlan of specificed protocol group on specified port.
 * Input:
 *      unit           - unit id
 *      port           - port id
 *      protoGroup_idx - protocol group index
 *      pVlan_cfg      - vlan configuration of protocol group
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_OUT_OF_RANGE - protocol group index is out of range
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_vlan_portProtoVlan_set(
    uint32                  unit,
    rtk_port_t              port,
    uint32                  protoGroup_idx,
    rtk_vlan_protoVlanCfg_t *pVlan_cfg);

/* Function Name:
 *      dal_esw_vlan_portOuterProtoVlan_get
 * Description:
 *      Get outer vlan of specificed protocol group on specified port.
 * Input:
 *      unit           - unit id
 *      port           - port id
 *      protoGroup_idx - protocol group index
 * Output:
 *      pVlan_cfg      - pointer to vlan configuration of protocol group
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_OUT_OF_RANGE - protocol group index is out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_vlan_portOuterProtoVlan_get(
    uint32                  unit,
    rtk_port_t              port,
    uint32                  protoGroup_idx,
    rtk_vlan_protoVlanCfg_t *pVlan_cfg);

/* Function Name:
 *      dal_esw_vlan_portOuterProtoVlan_set
 * Description:
 *      Set outer vlan of specificed protocol group on specified port.
 * Input:
 *      unit           - unit id
 *      port           - port id
 *      protoGroup_idx - protocol group index
 *      pVlan_cfg      - vlan configuration of protocol group
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_OUT_OF_RANGE - protocol group index is out of range
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_vlan_portOuterProtoVlan_set(
    uint32                  unit,
    rtk_port_t              port,
    uint32                  protoGroup_idx,
    rtk_vlan_protoVlanCfg_t *pVlan_cfg);

/* Module Name    : Vlan               */
/* Sub-module Name: TPID configuration */

/* Function Name:
 *      dal_esw_vlan_portTpidEntry_get
 * Description:
 *      Get TPID of TPID entry on specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      tpid_idx - index of TPID entry
 * Output:
 *      pTpid    - pointer to TPID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_vlan_portTpidEntry_get(uint32 unit, rtk_port_t port, uint32 tpid_idx, uint32 *pTpid);

/* Function Name:
 *      dal_esw_vlan_portTpidEntry_set
 * Description:
 *      Set tpid of TPID entry on specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      tpid_idx - index of TPID entry
 *      tpid     - TPID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *
 */
extern int32
dal_esw_vlan_portTpidEntry_set(uint32 unit, rtk_port_t port, uint32 tpid_idx, uint32 tpid);

/* Function Name:
 *      dal_esw_vlan_portEgrInnerTpidMode_get
 * Description:
 *      Get TPID mode of inner tag at egress.
 *      TPID mode will decide what kind of packet should be replaced inner TPID with configured one.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pTpid_mode - pointer to inner TPID mode at egress
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Inner TPID mode is as following:
 *      - EGR_TPID_MODE_IGR_UNTAG_AND_EGR_TAG
 *      - EGR_TPID_MODE_ALL_PACKETS
 */
extern int32
dal_esw_vlan_portEgrInnerTpidMode_get(uint32 unit, rtk_port_t port, rtk_vlan_egrTpidMode_t *pTpid_mode);

/* Function Name:
 *      dal_esw_vlan_portEgrInnerTpidMode_set
 * Description:
 *      Set TPID mode of inner tag at egress.
 *      TPID mode will decide what kind of packet should be replaced inner TPID with configured one.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      tpid_mode - inner TPID mode at egress
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
 *      Inner TPID mode is as following:
 *      - EGR_TPID_MODE_IGR_UNTAG_AND_EGR_TAG
 *      - EGR_TPID_MODE_ALL_PACKETS
 */
extern int32
dal_esw_vlan_portEgrInnerTpidMode_set(uint32 unit, rtk_port_t port, rtk_vlan_egrTpidMode_t tpid_mode);

/* Function Name:
 *      dal_esw_vlan_portIgrInnerTpid_get
 * Description:
 *      Get TPIDs of inner tag at ingress.
 * Input:
 *      unit             - unit id
 *      port             - port id
 * Output:
 *      pTpid_entry_mask - pointer to mask for index of tpid entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_vlan_portIgrInnerTpid_get(uint32 unit, rtk_port_t port, uint32 *pTpid_idx_mask);

/* Function Name:
 *      dal_esw_vlan_portIgrInnerTpid_set
 * Description:
 *      Set TPIDs of inner tag at ingress.
 * Input:
 *      unit            - unit id
 *      port            - port id
 *      tpid_entry_mask - mask for index of tpid entry
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
 *      None
 */
extern int32
dal_esw_vlan_portIgrInnerTpid_set(uint32 unit, rtk_port_t port, uint32 tpid_idx_mask);

/* Function Name:
 *      dal_esw_vlan_portEgrInnerTpid_get
 * Description:
 *      Get TPID of inner tag at egress.
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pTpid_idx - pointer to index of inner TPID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_vlan_portEgrInnerTpid_get(uint32 unit, rtk_port_t port, uint32 *pTpid_idx);

/* Function Name:
 *      dal_esw_vlan_portEgrInnerTpid_set
 * Description:
 *      Set TPID of inner tag at egress.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      tpid_idx - index of inner TPID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
extern int32
dal_esw_vlan_portEgrInnerTpid_set(uint32 unit, rtk_port_t port, uint32 tpid_idx);

/* Function Name:
 *      dal_esw_vlan_portEgrOuterTpidMode_get
 * Description:
 *      Get TPID mode of outer tag at egress.
 *      TPID mode will decide what kind of packet should be replaced outer TPID with configured one.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pTpid_mode - pointer to outer TPID mode at egress
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Inner TPID mode is as following:
 *      - EGR_TPID_MODE_IGR_UNTAG_AND_EGR_TAG
 *      - EGR_TPID_MODE_ALL_PACKETS
 */
extern int32
dal_esw_vlan_portEgrOuterTpidMode_get(uint32 unit, rtk_port_t port, rtk_vlan_egrTpidMode_t *pTpid_mode);

/* Function Name:
 *      dal_esw_vlan_portEgrOuterTpidMode_set
 * Description:
 *      Set TPID mode of outer tag at egress.
 *      TPID mode will decide what kind of packet should be replaced outer TPID with configured one.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      tpid_mode - outer TPID mode at egress
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
 *      Inner TPID mode is as following:
 *      - EGR_TPID_MODE_IGR_UNTAG_AND_EGR_TAG
 *      - EGR_TPID_MODE_ALL_PACKETS
 */
extern int32
dal_esw_vlan_portEgrOuterTpidMode_set(uint32 unit, rtk_port_t port, rtk_vlan_egrTpidMode_t tpid_mode);

/* Function Name:
 *      dal_esw_vlan_portIgrOuterTpid_get
 * Description:
 *      Get TPIDs of outer tag at ingress.
 * Input:
 *      unit           - unit id
 *      port           - port id
 * Output:
 *      pTpid_idx_mask - pointer to mask for index of tpid entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_vlan_portIgrOuterTpid_get(uint32 unit, rtk_port_t port, uint32 *pTpid_idx_mask);

/* Function Name:
 *      dal_esw_vlan_portIgrOuterTpid_set
 * Description:
 *      Set TPIDs of outer tag at ingress.
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      tpid_idx_mask - mask for index of tpid entry
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
 *      None
 */
extern int32
dal_esw_vlan_portIgrOuterTpid_set(uint32 unit, rtk_port_t port, uint32 tpid_idx_mask);

/* Function Name:
 *      dal_esw_vlan_portEgrOuterTpid_get
 * Description:
 *      Get TPID of outer tag at egress.
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pTpid_idx - pointer to index of outer TPID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_vlan_portEgrOuterTpid_get(uint32 unit, rtk_port_t port, uint32 *pTpid_idx);

/* Function Name:
 *      dal_esw_vlan_portEgrOuterTpid_set
 * Description:
 *      Set TPID of outer tag at egress.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      tpid_idx - index of outer TPID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
extern int32
dal_esw_vlan_portEgrOuterTpid_set(uint32 unit, rtk_port_t port, uint32 tpid_idx);

/* Function Name:
 *      dal_esw_vlan_portIgrExtraTpid_get
 * Description:
 *      Get TPIDs of extra tag at ingress.
 * Input:
 *      unit           - unit id
 *      port           - port id
 * Output:
 *      pTpid_idx_mask - pointer to mask for index of tpid entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_vlan_portIgrExtraTpid_get(uint32 unit, rtk_port_t port, uint32 *pTpid_idx_mask);

/* Function Name:
 *      dal_esw_vlan_portIgrExtraTpid_set
 * Description:
 *      Set TPIDs of extra tag at ingress.
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      tpid_idx_mask - mask for index of tpid entry
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
 *      None
 */
extern int32
dal_esw_vlan_portIgrExtraTpid_set(uint32 unit, rtk_port_t port, uint32 tpid_idx_mask);

/* Module Name    : Vlan                 */
/* Sub-module Name: Ingress tag handling */

/* Function Name:
 *      dal_esw_vlan_portIgrIgnoreInnerTagEnable_get
 * Description:
 *      Get enable status of ignore inner tag at ingress.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      enable - enable status of ignore inner tag
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_vlan_portIgrIgnoreInnerTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_vlan_portIgrIgnoreInnerTagEnable_set
 * Description:
 *      Set enable status of ignore inner tag at ingress.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of ignore inner tag
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
 *      None
 */
extern int32
dal_esw_vlan_portIgrIgnoreInnerTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_vlan_portIgrIgnoreOuterTagEnable_get
 * Description:
 *      Get enable status of ignore outer tag at ingress.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of ignore outer tag
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_vlan_portIgrIgnoreOuterTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_vlan_portIgrIgnoreOuterTagEnable_set
 * Description:
 *      Set enable status of ignore outer tag at ingress.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of ignore outer tag
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
 *      None
 */
extern int32
dal_esw_vlan_portIgrIgnoreOuterTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Module Name    : Vlan                */
/* Sub-module Name: Egress tag handling */

/* Function Name:
 *      dal_esw_vlan_portEgrInnerTagEnable_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Egress packet will send out with inner tag when following is true.
 *      - inner tag capability is enable
 *      - this port is tag member of specified vlan
 */
extern int32
dal_esw_vlan_portEgrInnerTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_vlan_portEgrInnerTagEnable_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      Egress packet will send out with inner tag when following is true.
 *      - inner tag capability is enable
 *      - this port is tag member of specified vlan
 */
extern int32
dal_esw_vlan_portEgrInnerTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_vlan_portEgrOuterTagEnable_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Egress packet will send out with outer tag when following is true.
 *      - outer tag capability is enable
 *      - this port is tag member of specified vlan
 */
extern int32
dal_esw_vlan_portEgrOuterTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_vlan_portEgrOuterTagEnable_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      Egress packet will send out with outer tag when following is true.
 *      - outer tag capability is enable
 *      - this port is tag member of specified vlan
 */
extern int32
dal_esw_vlan_portEgrOuterTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_vlan_portIgrExtraTagEnable_get
 * Description:
 *      Get enable status of recognizing extra tag at ingress.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of recognizing extra tag
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_vlan_portIgrExtraTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_vlan_portIgrExtraTagEnable_set
 * Description:
 *      Set enable status of recognizing extra tag at ingress.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of recognizing extra tag
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
 *      None
 */
extern int32
dal_esw_vlan_portIgrExtraTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_vlan_portEgrExtraTagEnable_get
 * Description:
 *      Get enable status of recognizing extra tag at egress.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of recognizing extra tag
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_vlan_portEgrExtraTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_vlan_portEgrExtraTagEnable_set
 * Description:
 *      Set enable status of recognizing extra tag at egress.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of recognizing extra tag
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
 *      None
 */
extern int32
dal_esw_vlan_portEgrExtraTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_vlan_portEgrInnerVidSource_get
 * Description:
 *      Get source of vid field in inner tag.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pVidSource - pointer to source of vid field in inner tag
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Vid source is as following
 *      - TAG_SOURCE_FROM_ALE
 *      - TAG_SOURCE_FROM_ORIG_INNER_TAG
 *      - TAG_SOURCE_FROM_ORIG_OUTER_TAG
 */
extern int32
dal_esw_vlan_portEgrInnerVidSource_get(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t *pVidSource);

/* Function Name:
 *      dal_esw_vlan_portEgrInnerVidSource_set
 * Description:
 *      Set source of vid field in inner tag.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      vidSource - source of vid field in inner tag
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
 *      Vid source is as following
 *      - TAG_SOURCE_FROM_ALE
 *      - TAG_SOURCE_FROM_ORIG_INNER_TAG
 *      - TAG_SOURCE_FROM_ORIG_OUTER_TAG
 */
extern int32
dal_esw_vlan_portEgrInnerVidSource_set(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t vidSource);

/* Function Name:
 *      dal_esw_vlan_portEgrInnerPriSource_get
 * Description:
 *      Get source of priority field in inner tag.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pPriSource - pointer to source of priority field in inner tag
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Vid source is as following
 *      - TAG_SOURCE_FROM_ORIG_INNER_TAG
 *      - TAG_SOURCE_FROM_ORIG_OUTER_TAG
 *      - TAG_SOURCE_NULL
 */
extern int32
dal_esw_vlan_portEgrInnerPriSource_get(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t *pPriSource);

/* Function Name:
 *      dal_esw_vlan_portEgrInnerPriSource_set
 * Description:
 *      Set source of priority field in inner tag.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      priSource - source of priority field in inner tag
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
 *      Vid source is as following
 *      - TAG_SOURCE_FROM_ORIG_INNER_TAG
 *      - TAG_SOURCE_FROM_ORIG_OUTER_TAG
 *      - TAG_SOURCE_NULL
 */
extern int32
dal_esw_vlan_portEgrInnerPriSource_set(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t priSource);

/* Function Name:
 *      dal_esw_vlan_portEgrOuterVidSource_get
 * Description:
 *      Get source of vid field in outer tag.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pVidSource - pointer to source of vid field in outer tag
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Vid source is as following
 *      - TAG_SOURCE_FROM_ALE
 *      - TAG_SOURCE_FROM_ORIG_INNER_TAG
 *      - TAG_SOURCE_FROM_ORIG_OUTER_TAG
 */
extern int32
dal_esw_vlan_portEgrOuterVidSource_get(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t *pVidSource);

/* Function Name:
 *      dal_esw_vlan_portEgrOuterVidSource_set
 * Description:
 *      Set source of vid field in outer tag.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      vidSource - source of vid field in outer tag
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
 *      Vid source is as following
 *      - TAG_SOURCE_FROM_ALE
 *      - TAG_SOURCE_FROM_ORIG_INNER_TAG
 *      - TAG_SOURCE_FROM_ORIG_OUTER_TAG
 */
extern int32
dal_esw_vlan_portEgrOuterVidSource_set(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t vidSource);

/* Function Name:
 *      dal_esw_vlan_portEgrOuterPriSource_get
 * Description:
 *      Get source of priority field in outer tag.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pPriSource - pointer to source of priority field in outer tag
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Vid source is as following
 *      - TAG_SOURCE_FROM_ORIG_INNER_TAG
 *      - TAG_SOURCE_FROM_ORIG_OUTER_TAG
 *      - TAG_SOURCE_NULL
 */
extern int32
dal_esw_vlan_portEgrOuterPriSource_get(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t *pPriSource);

/* Function Name:
 *      dal_esw_vlan_portEgrOuterPriSource_set
 * Description:
 *      Set source of priority field in outer tag.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      priSource - source of priority field in outer tag
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
 *      Vid source is as following
 *      - TAG_SOURCE_FROM_ORIG_INNER_TAG
 *      - TAG_SOURCE_FROM_ORIG_OUTER_TAG
 *      - TAG_SOURCE_NULL
 */
extern int32
dal_esw_vlan_portEgrOuterPriSource_set(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t priSource);

/* Module Name    : Vlan                */
/* Sub-module Name: Tag format handling */

/* Function Name:
 *      dal_esw_vlan_portIgrTagKeepEnable_get
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
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      Packet will be transmitted with original tag format when following condition are true.
 *      - Enable keep tag format at ingress.
 *      - Enable keep tag format at egress
 */
extern int32
dal_esw_vlan_portIgrTagKeepEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pKeepOuter, rtk_enable_t *pKeepInner);

/* Function Name:
 *      dal_esw_vlan_portIgrTagKeepEnable_set
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
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      Packet will be transmitted with original tag format when following condition are true.
 *      - Enable keep tag format at ingress.
 *      - Enable keep tag format at egress
 */
extern int32
dal_esw_vlan_portIgrTagKeepEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t keepOuter, rtk_enable_t keepInner);

/* Function Name:
 *      dal_esw_vlan_portEgrTagKeepEnable_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Packet will be transmitted with original tag format when following condition are true.
 *      - Enable keep tag format at ingress.
 *      - Enable keep tag format at egress
 */
extern int32
dal_esw_vlan_portEgrTagKeepEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pKeepOuter, rtk_enable_t *pKeepInner);

/* Function Name:
 *      dal_esw_vlan_portEgrTagKeepEnable_set
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
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      Packet will be transmitted with original tag format when following condition are true.
 *      - Enable keep tag format at ingress.
 *      - Enable keep tag format at egress
 */
extern int32
dal_esw_vlan_portEgrTagKeepEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t keepOuter, rtk_enable_t keepInner);

/*
 * MISC
 */

/* Function Name:
 *      dal_esw_vlan_fwdMode_get
 * Description:
 *      Get packet forwarding mode on specified vlan.
 *      Packet can be forwarded by ALE decision or forward to members of vlan.
 * Input:
 *      unit  - unit id
 *      vid   - vlan id
 * Output:
 *      pMode - pointer to packet forwarding mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_VLAN_VID     - invalid vlan id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Packet forwarding mode is as following:
 *      - VLAN_FWD_ON_ALE
 *      - VLAN_FWD_ON_VLAN_MEMBER
 */
extern int32
dal_esw_vlan_fwdMode_get(uint32 unit, rtk_vlan_t vid, rtk_vlan_fwdMode_t *pMode);

/* Function Name:
 *      dal_esw_vlan_fwdMode_set
 * Description:
 *      Set packet forwarding mode on specified vlan.
 *      Packet can be forwarded by ALE decision or forward to members of vlan.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 *      mode - packet forwarding mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_VLAN_VID - invalid vlan id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      Packet forwarding mode is as following:
 *      - VLAN_FWD_ON_ALE
 *      - VLAN_FWD_ON_VLAN_MEMBER
 *
 */
extern int32
dal_esw_vlan_fwdMode_set(uint32 unit, rtk_vlan_t vid, rtk_vlan_fwdMode_t mode);

/* Function Name:
 *      dal_esw_vlan_stg_get
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
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 */
extern int32
dal_esw_vlan_stg_get(uint32 unit, rtk_vlan_t vid, rtk_stg_t *pStg);

/* Function Name:
 *      dal_esw_vlan_stg_set
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
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_MSTI                 - invalid msti
 * Note:
 */
extern int32
dal_esw_vlan_stg_set(uint32 unit, rtk_vlan_t vid, rtk_stg_t stg);

#endif /* __DAL_ESW_VLAN_H__ */

