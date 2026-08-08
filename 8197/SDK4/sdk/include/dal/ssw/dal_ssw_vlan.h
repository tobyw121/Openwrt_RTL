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
 * $Revision: 20153 $
 * $Date: 2011-07-29 09:48:05 +0800 (Fri, 29 Jul 2011) $
 *
 * Purpose : Definition those public vlan APIs and its data type in the SDK.
 * 
 * Feature : The file have include the following module and sub-modules
 *           1) vlan
 * 
 */ 
#ifndef __DAL_SSW_VLAN_H__ 
#define __DAL_SSW_VLAN_H__

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
 *      dal_ssw_vlan_create
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
 *         dal_ssw_vlan_fid_set
 *         dal_ssw_vlan_stg_set
 */
extern int32
dal_ssw_vlan_create(uint32 unit, rtk_vlan_t vid);


/* Function Name:
 *      dal_ssw_vlan_destroy
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
dal_ssw_vlan_destroy(uint32 unit, rtk_vlan_t vid);


/* Function Name:
 *      dal_ssw_vlan_destroyAll
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
dal_ssw_vlan_destroyAll(uint32 unit, uint32 keep_and_restore_default_vlan);


/* Function Name:
 *      dal_ssw_vlan_init
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
dal_ssw_vlan_init(uint32 unit);


/* Function Name:
 *      dal_ssw_vlan_fid_get
 * Description:
 *      Get the filter id of the vlan from the specified device.
 * Input: 
 *      unit  - unit id
 *      vid   - vlan id
 * Output:
 *      *pFid - pointer buffer of filter id
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note: 
 *      1. In IVL mode, fid is equal with vid after vlan create.
 *      2. You don't need to care fid when you use the IVL mode.
 *      3. The API should be used for SVL mode.
 */
extern int32
dal_ssw_vlan_fid_get(uint32 unit, rtk_vlan_t vid, rtk_fid_t *pFid);


/* Function Name:
 *      dal_ssw_vlan_fid_set
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
 *      1. In IVL mode, fid is equal with vid after vlan create.
 *      2. You don't need to care fid when you use the IVL mode.
 *      3. The API should be used for SVL mode.
 */
extern int32
dal_ssw_vlan_fid_set(uint32 unit, rtk_vlan_t vid, rtk_fid_t fid);


/* Function Name:
 *      dal_ssw_vlan_port_add
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
dal_ssw_vlan_port_add(uint32 unit, rtk_vlan_t vid, rtk_port_t port, uint32 is_untag);


/* Function Name:
 *      dal_ssw_vlan_port_del
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
dal_ssw_vlan_port_del(uint32 unit, rtk_vlan_t vid, rtk_port_t port);


/* Function Name:
 *      dal_ssw_vlan_port_get
 * Description:
 *      Get the vlan members from the specified device.
 * Input:
 *      unit              - unit id
 *      vid               - vlan id
 * Output:
 *      *pMember_portmask - pointer buffer of member ports
 *      *pUntag_portmask  - pointer buffer of untagged member ports
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
dal_ssw_vlan_port_get(
    uint32         unit,
    rtk_vlan_t     vid,
    rtk_portmask_t *pMember_portmask,
    rtk_portmask_t *pUntag_portmask);


/* Function Name:
 *      dal_ssw_vlan_port_set
 * Description:
 *      Replace the vlan members in the specified device.
 * Input:
 *      unit              - unit id
 *      vid               - vlan id
 *      *pMember_portmask - member ports
 *      *pUntag_portmask  - untagged member ports
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
dal_ssw_vlan_port_set(
    uint32         unit,
    rtk_vlan_t     vid,
    rtk_portmask_t *pMember_portmask,
    rtk_portmask_t *pUntag_portmask);


/* Function Name:
 *      dal_ssw_vlan_stg_get
 * Description:
 *      Get spanning tree group instance of the vlan from the specified device.
 * Input:
 *      unit  - unit id
 *      vid   - vlan id
 * Output:
 *      *pStg - pointer buffer of spanning tree group instance
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
dal_ssw_vlan_stg_get(uint32 unit, rtk_vlan_t vid, rtk_stg_t *pStg);


/* Function Name:
 *      dal_ssw_vlan_stg_set
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
dal_ssw_vlan_stg_set(uint32 unit, rtk_vlan_t vid, rtk_stg_t stg);


/* Module Name     : vlan           */
/* Sub-module Name : vlan attribute */


/* Module Name     : vlan                */
/* Sub-module Name : vlan port attribute */

/* Function Name:
 *      dal_ssw_vlan_portAcceptFrameType_get
 * Description:
 *      Get vlan accept frame type of the port from the specified device.
 * Input:
 *      unit                - unit id
 *      port                - port id
 * Output:
 *      *pAccept_frame_type - pointer buffer of accept frame type
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. The accept frame type as following:
 *          - ACCEPT_FRAME_TYPE_ALL
 *          - ACCEPT_FRAME_TYPE_TAG_ONLY
 *          - ACCEPT_FRAME_TYPE_UNTAG_ONLY
 *      2. The API is used for 802.1Q tagged  and if you want to get the 802.1ad
 *         accept frame type, please use dal_ssw_svlan_portAcceptFrameType_get
 */
extern int32
dal_ssw_vlan_portAcceptFrameType_get(
    uint32                     unit,
    rtk_port_t                 port,
    rtk_vlan_acceptFrameType_t *pAccept_frame_type);


/* Function Name: 
 *      dal_ssw_vlan_portAcceptFrameType_set
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
 *      RT_ERR_INPUT                  - invalid input parameter
 *      RT_ERR_CHIP_NOT_SUPPORTED     - functions not supported by this chip model
 * Note:
 *      The accept frame type as following:
 *          - ACCEPT_FRAME_TYPE_ALL
 *          - ACCEPT_FRAME_TYPE_TAG_ONLY
 *          - ACCEPT_FRAME_TYPE_UNTAG_ONLY
 *      2. The API is used for 802.1Q tagged  and if you want to set the 802.1ad
 *         accept frame type, please use dal_ssw_svlan_portAcceptFrameType_set
 */
extern int32
dal_ssw_vlan_portAcceptFrameType_set(
    uint32                     unit,
    rtk_port_t                 port,
    rtk_vlan_acceptFrameType_t accept_frame_type);


/* Function Name:
 *      dal_ssw_vlan_vlanFunctionEnable_get
 * Description:
 *      Get the VLAN enable status of the specified device.
 * Input:
 *      unit    - unit id
 *      enable  - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_UNIT_ID        - invalid unit id
 * Note:
 *      1. The status of vlan function is as following:
 *	    - DISABLED
 *	    - ENABLED
 */
extern int32
dal_ssw_vlan_vlanFunctionEnable_get(uint32 unit, rtk_enable_t *pEnable);


/* Function Name:
 *      dal_ssw_vlan_vlanFunctionEnable_set
 * Description:
 *      Set the VLAN enable status of the specified device.
 * Input:
 *      unit    - unit id
 *      enable  - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_INPUT          - invalid input parameter
 * Note:
 *      1. The status of vlan function is as following:
 *	    - DISABLED
 *	    - ENABLED
 */
extern int32
dal_ssw_vlan_vlanFunctionEnable_set(uint32 unit, rtk_enable_t enable);

/* Function Name: 
 *      dal_ssw_vlan_igrFilterEnable_get
 * Description:
 *      Get vlan ingress filter status from the specified device.
 * Input:
 *      unit         - unit id
 * Output:
 *      *pIgr_filter - pointer buffer of ingress filter status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. The status of vlan ingress filter is as following:
 *		- DISABLED
 *		- ENABLED
 */
extern int32
dal_ssw_vlan_igrFilterEnable_get(uint32 unit, rtk_enable_t *pIgr_filter);


/* Function Name:
 *      dal_ssw_vlan_igrFilterEnable_set
 * Description:
 *      Set vlan ingress filter status of the port to the specified device.
 * Input:
 *      unit       - unit id
 *      igr_filter - ingress filter configure
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      1. If the chip is per system configuration, set any port will apply to 
 *      whole device. like RTL8329/RTL8389 chip.
 *      2. The status of vlan ingress filter is as following:
 *		- DISABLED
 *		- ENABLED
 */
extern int32
dal_ssw_vlan_igrFilterEnable_set(uint32 unit, rtk_enable_t igr_filter);

/* Function Name:
 *      dal_ssw_vlan_mcastLeakyEnable_get
 * Description:
 *      Get vlan egress leaky status of the system from the specified device.
 * Input:
 *      unit    - unit id
 * Output:
 *      *pLeaky - pointer buffer of vlan leaky of egress
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. The status of vlan egress multicast leaky is as following:
 *		- DISABLED
 *		- ENABLED
 */
extern int32
dal_ssw_vlan_mcastLeakyEnable_get(uint32 unit, rtk_enable_t *pLeaky);


/* Function Name:
 *      dal_ssw_vlan_mcastLeakyEnable_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      1. egress vlan leaky configuration is apply to L2/IP multicast packet only,
 *         no apply to unicast.
 *      2. The status of vlan egress multicast leaky is as following:
 *		- DISABLED
 *		- ENABLED
 */
extern int32
dal_ssw_vlan_mcastLeakyEnable_set(uint32 unit, rtk_enable_t leaky);


/* Function Name:
 *      dal_ssw_vlan_mcastLeakyPortEnable_get
 * Description:
 *      Get vlan egress leaky status of the port from the specified device.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      *pLeaky - pointer buffer of vlan leaky of egress
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT           - The module is not initial
 *      RT_ERR_UNIT_ID            - invalid unit id
 *      RT_ERR_PORT_ID            - invalid port id
 *      RT_ERR_NULL_POINTER       - input parameter may be null pointer
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 * Note:
 *      1. The status of vlan egress multicast leaky is as following:
 *		- DISABLED
 *		- ENABLED
 */
extern int32
dal_ssw_vlan_mcastLeakyPortEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pLeaky);


/* Function Name:
 *      dal_ssw_vlan_mcastLeakyPortEnable_set
 * Description:
 *      Set vlan egress leaky configure of the port to the specified device.
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      leaky - vlan leaky of egress
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT           - The module is not initial
 *      RT_ERR_UNIT_ID            - invalid unit id
 *      RT_ERR_PORT_ID            - invalid port id
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 * Note:
 *      1. egress vlan leaky configuration is apply to L2/IP multicast packet only,
 *         no apply to unicast.
 *      2. If the chip is per system configuration, set any port will apply
 *         to whole device.
 *      3. The status of vlan egress multicast leaky is as following:
 *		- DISABLED
 *		- ENABLED
 */
extern int32
dal_ssw_vlan_mcastLeakyPortEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t leaky);


/* Function Name:
 *      dal_ssw_vlan_portPvid_get
 * Description:
 *      Get port default vlan id from the specified device.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      *pPvid - pointer buffer of port default vlan id
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
dal_ssw_vlan_portPvid_get(uint32 unit, rtk_port_t port, uint32 *pPvid);


/* Function Name:
 *      dal_ssw_vlan_portPvid_set
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
dal_ssw_vlan_portPvid_set(uint32 unit, rtk_port_t port, uint32 pvid);


/* Function Name:
 *      dal_ssw_vlan_tagMode_get
 * Description:
 *      Get vlan tagged mode of the port from the specified device.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      *pTag_mode - pointer buffer of vlan tagged mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. The vlan tagged mode as following:
 *          - VLAN_TAG_MODE_ORIGINAL        (depend on chip normal decision)
 *          - VLAN_TAG_MODE_KEEP_FORMAT     (keep ingress format to egress)
 *          - VLAN_TAG_MODE_PRI             (always priority tag out)
 */
extern int32
dal_ssw_vlan_tagMode_get(uint32 unit, rtk_port_t port, rtk_vlan_tagMode_t *pTag_mode);


/* Function Name:
 *      dal_ssw_vlan_tagMode_set
 * Description:
 *      Set vlan tagged mode of the port to the specified device.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      tag_mode - vlan tagged mode
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
 *      1. The vlan tagged mode as following:
 *          - VLAN_TAG_MODE_ORIGINAL        (depend on chip normal decision)
 *          - VLAN_TAG_MODE_KEEP_FORMAT     (keep ingress format to egress)
 *          - VLAN_TAG_MODE_PRI             (always priority tag out)
 */
extern int32
dal_ssw_vlan_tagMode_set(uint32 unit, rtk_port_t port, rtk_vlan_tagMode_t tag_mode);

#endif /* __DAL_SSW_VLAN_H__ */
