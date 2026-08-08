/* 
 * Copyright(c) Realtek Semiconductor Corporation, 2008 
 * All rights reserved. 
 * 
 * $Revision: 10405 $
 * $Date: 2010-06-23 19:15:05 +0800 (Wed, 23 Jun 2010) $
 *
 * Purpose : Definition those public TRUNK APIs and its data type in the SDK.
 * 
 * Feature : The file have include the following module and sub-modules
 *           1) Trunk
 * 
 */ 

#ifndef __DAL_ESW_TRUNK_H__
#define __DAL_ESW_TRUNK_H__

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/trunk.h>

/*
 * Symbol Definition
 */
#define MAX_TRUNK_ENTRY  8

/*
 * Data Declaration
 */

/*
 * Function Declaration
 */

/* Module Name : TRUNK */

/* Function Name:
 *      dal_esw_trunk_init
 * Description:
 *      Initialize trunk module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      Must initialize trunk module before calling any trunk APIs.
 */
extern int32
dal_esw_trunk_init(uint32 unit);

/* Module Name    : Trunk                    */
/* Sub-module Name: User configuration trunk */

/* Function Name:
 *      dal_esw_trunk_distributionAlgorithm_get
 * Description:
 *      Get the distribution algorithm of the trunk group id from the specified device.
 * Input:
 *      unit          - unit id
 *      trk_gid       - trunk group id
 * Output:
 *      pAlgo_bitmask - pointer buffer of bitmask of the distribution algorithm
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. You can use OR opertions in following bits to decide your algorithm.
 *      - TRUNK_DISTRIBUTION_ALGO_SPA_BIT        (source port)
 *      - TRUNK_DISTRIBUTION_ALGO_SMAC_BIT       (source mac)
 *      - TRUNK_DISTRIBUTION_ALGO_DMAC_BIT       (destination mac)
 *      - TRUNK_DISTRIBUTION_ALGO_SIP_BIT        (source ip)
 *      - TRUNK_DISTRIBUTION_ALGO_DIP_BIT        (destination ip)
 *      - TRUNK_DISTRIBUTION_ALGO_SRC_L4PORT_BIT (source layer4 port)
 *      - TRUNK_DISTRIBUTION_ALGO_DST_L4PORT_BIT (destination layer4 port)
 */
extern int32
dal_esw_trunk_distributionAlgorithm_get(uint32 unit, uint32 trk_gid, uint32 *pAlgo_bitmask);


/* Function Name:
 *      dal_esw_trunk_distributionAlgorithm_set
 * Description:
 *      Set the distribution algorithm of the trunk group id from the specified device.
 * Input:
 *      unit         - unit id
 *      trk_gid      - trunk group id
 *      algo_bitmask - bitmask of the distribution algorithm
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_LA_TRUNK_ID - invalid trunk ID
 *      RT_ERR_LA_HASHMASK - invalid hash mask
 * Note:
 *      1. You can use OR opertions in following bits to decide your algorithm.
 *      - TRUNK_DISTRIBUTION_ALGO_SPA_BIT        (source port)
 *      - TRUNK_DISTRIBUTION_ALGO_SMAC_BIT       (source mac)
 *      - TRUNK_DISTRIBUTION_ALGO_DMAC_BIT       (destination mac)
 *      - TRUNK_DISTRIBUTION_ALGO_SIP_BIT        (source ip)
 *      - TRUNK_DISTRIBUTION_ALGO_DIP_BIT        (destination ip)
 *      - TRUNK_DISTRIBUTION_ALGO_SRC_L4PORT_BIT (source layer4 port)
 *      - TRUNK_DISTRIBUTION_ALGO_DST_L4PORT_BIT (destination layer4 port)
 */
extern int32
dal_esw_trunk_distributionAlgorithm_set(uint32 unit, uint32 trk_gid, uint32 algo_bitmask);

/* Function Name:
 *      dal_esw_trunk_hashMappingTable_get
 * Description:
 *      Get hash value to port array in the trunk group id from the specified device.
 * Input:
 *      unit             - unit id
 *      trk_gid          - trunk group id
 * Output:
 *      pHash2Port_array - pointer buffer of ports associate with the hash value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_trunk_hashMappingTable_get(
    uint32                   unit,
    uint32                   trk_gid,
    rtk_trunk_hashVal2Port_t *pHash2Port_array);

/* Function Name:
 *      dal_esw_trunk_hashMappingTable_set
 * Description:
 *      Set hash value to port array in the trunk group id from the specified device.
 * Input:
 *      unit             - unit id
 *      trk_gid          - trunk group id
 *      pHash2Port_array - ports associate with the hash value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID            - invalid unit id
 *      RT_ERR_LA_TRUNK_ID        - invalid trunk ID
 *      RT_ERR_NULL_POINTER       - input parameter may be null pointer
 *      RT_ERR_LA_TRUNK_NOT_EXIST - the trunk doesn't exist
 *      RT_ERR_LA_NOT_MEMBER_PORT - the port is not a member port of the trunk
 *      RT_ERR_LA_CPUPORT         - CPU port can not be aggregated port
 * Note:
 *      None
 */
extern int32
dal_esw_trunk_hashMappingTable_set(
    uint32                   unit,
    uint32                   trk_gid,
    rtk_trunk_hashVal2Port_t *pHash2Port_array);

/* Function Name:
 *      dal_esw_trunk_mode_get
 * Description:
 *      Get the trunk mode from the specified device.
 * Input:
 *      unit  - unit id
 * Output:
 *      pMode - pointer buffer of trunk mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. The enum of the trunk mode as following
 *      - TRUNK_MODE_NORMAL
 *      - TRUNK_MODE_DUMB
 */
extern int32
dal_esw_trunk_mode_get(uint32 unit, rtk_trunk_mode_t *pMode);

/* Function Name:
 *      dal_esw_trunk_mode_set
 * Description:
 *      Set the trunk mode to the specified device.
 * Input:
 *      unit - unit id
 *      mode - trunk mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      1. The enum of the trunk mode as following
 *      - TRUNK_MODE_NORMAL
 *      - TRUNK_MODE_DUMB
 */
extern int32
dal_esw_trunk_mode_set(uint32 unit, rtk_trunk_mode_t mode);

/* Function Name:
 *      dal_esw_trunk_port_get
 * Description:
 *      Get the members of the trunk id from the specified device.
 * Input:
 *      unit                   - unit id
 *      trk_gid                - trunk group id
 * Output:
 *      pTrunk_member_portmask - pointer buffer of trunk member ports
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_trunk_port_get(uint32 unit, uint32 trk_gid, rtk_portmask_t *pTrunk_member_portmask);

/* Function Name:
 *      dal_esw_trunk_port_set
 * Description:
 *      Set the members of the trunk id to the specified device.
 * Input:
 *      unit                   - unit id
 *      trk_gid                - trunk group id
 *      pTrunk_member_portmask - trunk member ports
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_LA_TRUNK_ID       - invalid trunk ID
 *      RT_ERR_LA_MEMBER_OVERLAP - the specified port mask is overlapped with other group
 *      RT_ERR_LA_PORTNUM_DUMB   - it can only aggregate at most four ports when 802.1ad dumb mode
 *      RT_ERR_LA_PORTNUM_NORMAL - it can only aggregate at most eight ports when 802.1ad normal mode
 * Note:
 *      None
 */
extern int32
dal_esw_trunk_port_set(uint32 unit, uint32 trk_gid, rtk_portmask_t *pTrunk_member_portmask);

/* Function Name:
 *      dal_esw_trunk_port_link_notification
 * Description:
 *      Notify link state change of ports to trunk failover handling.
 * Input:
 *      unit            - unit id
 *      trunkMemberPort - link state change port 
 *      linkStatus      - the new link state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_LA_TRUNK_ID - invalid trunk ID
 * Note:
 *      None
 */
extern int32
dal_esw_trunk_port_link_notification(uint32 unit, rtk_port_t Trunk_member_port, rtk_port_linkStatus_t LinkStatus);

/* Function Name:
 *      dal_esw_trunk_representPort_get
 * Description:
 *      Get represent port of trunk.
 * Input:
 *      unit        - unit id
 *      trunk_id    - trunk id
 * Output:
 *      pRepresPort - pointer to represent port of trunk
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_LA_TRUNK_ID      - invalid trunk id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_trunk_representPort_get(uint32 unit, uint32 trunk_id, rtk_port_t *pRepresPort);

/* Function Name:
 *      dal_esw_trunk_representPort_set
 * Description:
 *      Set represent port of trunk.
 * Input:
 *      unit       - unit id
 *      trunk_id   - trunk id
 *      represPort - represent port of trunk
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_LA_TRUNK_ID      - invalid trunk id
 *      RT_ERR_PORT_ID          - invalid port id
 * Note:
 *      None
 */
extern int32
dal_esw_trunk_representPort_set(uint32 unit, uint32 trunk_id, rtk_port_t represPort);

/* Function Name:
 *      dal_esw_trunk_floodMode_get
 * Description:
 *      Get flood mode of trunk.
 * Input:
 *      unit       - unit id
 *      trunk_id   - trunk id
 * Output:
 *      pFloodMode - pointer to flood mode of trunk
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_LA_TRUNK_ID      - invalid trunk id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Flood mode is as following
 *      - FLOOD_PORT_BY_HASH
 *      - FLOOD_PORT_BY_CONFIG
 */
extern int32
dal_esw_trunk_floodMode_get(uint32 unit, uint32 trunk_id, rtk_trunk_floodMode_t *pFloodMode);

/* Function Name:
 *      dal_esw_trunk_floodMode_set
 * Description:
 *      Set flood mode of trunk.
 * Input:
 *      unit      - unit id
 *      trunk_id  - trunk id
 *      floodMode - flood mode of trunk
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_LA_TRUNK_ID      - invalid trunk id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Flood mode is as following
 *      - FLOOD_PORT_BY_HASH
 *      - FLOOD_PORT_BY_CONFIG
 */
extern int32
dal_esw_trunk_floodMode_set(uint32 unit, uint32 trunk_id, rtk_trunk_floodMode_t floodMode);

/* Function Name:
 *      dal_esw_trunk_floodPort_get
 * Description:
 *      Get flooding port of trunk.
 * Input:
 *      unit       - unit id
 *      trunk_id   - trunk id
 * Output:
 *      pFloodPort - pointer to flooding port of trunk
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_LA_TRUNK_ID      - invalid trunk id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_trunk_floodPort_get(uint32 unit, uint32 trunk_id, rtk_port_t *pFloodPort);

/* Function Name:
 *      dal_esw_trunk_floodPort_set
 * Description:
 *      Set flooding port of trunk.
 * Input:
 *      unit      - unit id
 *      trunk_id  - trunk id
 *      floodPort - flooding port of trunk
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_LA_TRUNK_ID      - invalid trunk id
 *      RT_ERR_PORT_ID          - invalid port id
 * Note:
 *      None
 */
extern int32
dal_esw_trunk_floodPort_set(uint32 unit, uint32 trunk_id, rtk_port_t floodPort);

#endif /*__DAL_ESW_TRUNK_H__*/

