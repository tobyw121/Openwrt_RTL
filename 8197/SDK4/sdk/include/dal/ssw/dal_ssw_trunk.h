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
 * $Revision: 8999 $
 * $Date: 2010-04-12 15:42:36 +0800 (Mon, 12 Apr 2010) $
 *
 * Purpose : Definition those public TRUNK APIs and its data type in the SDK.
 * 
 * Feature : The file have include the following module and sub-modules
 *           1) Trunk
 * 
 */ 
#ifndef __DAL_SSW_TRUNK_H__ 
#define __DAL_SSW_TRUNK_H__

/*  
 * Include Files 
 */
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <rtk/trunk.h>

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

/* Module Name : TRUNK */

/* Function Name:
 *      dal_ssw_trunk_init
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
 *      Must initialize trunk module before calling any vlan APIs.
 */
extern int32
dal_ssw_trunk_init(uint32 unit);


/* Function Name:
 *      dal_ssw_trunk_distributionAlgorithm_get
 * Description:
 *      Get the distribution algorithm of the trunk group id from the specified device.
 * Input:
 *      unit           - unit id
 *      trk_gid        - trunk group id
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
 *      - #define TRUNK_DISTRIBUTION_ALGO_SPA_BIT   (source port)
 *      - #define TRUNK_DISTRIBUTION_ALGO_SMAC_BIT  (source mac)
 *      - #define TRUNK_DISTRIBUTION_ALGO_DMAC_BIT  (destination mac)
 *      - #define TRUNK_DISTRIBUTION_ALGO_SIP_BIT   (source ip)
 *      - #define TRUNK_DISTRIBUTION_ALGO_DIP_BIT   (destination ip)
 *      2. Normal mode support 8 trunk members in each group, but dumb mode only 4.
 */
extern int32
dal_ssw_trunk_distributionAlgorithm_get(uint32 unit, uint32 trk_gid, uint32 *pAlgo_bitmask);


/* Function Name:
 *      dal_ssw_trunk_distributionAlgorithm_set
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
 *      - #define TRUNK_DISTRIBUTION_ALGO_SPA_BIT   (source port)
 *      - #define TRUNK_DISTRIBUTION_ALGO_SMAC_BIT  (source mac)
 *      - #define TRUNK_DISTRIBUTION_ALGO_DMAC_BIT  (destination mac)
 *      - #define TRUNK_DISTRIBUTION_ALGO_SIP_BIT   (source ip)
 *      - #define TRUNK_DISTRIBUTION_ALGO_DIP_BIT   (destination ip)
 *      2. Normal mode support 8 trunk members in each group, but dumb mode only 4.
 */
extern int32
dal_ssw_trunk_distributionAlgorithm_set(uint32 unit, uint32 trk_gid, uint32 algo_bitmask);


/* Function Name:
 *      dal_ssw_trunk_hashMappingTable_get
 * Description:
 *      Get hash value to port array in the trunk group id from the specified device.
 * Input:
 *      unit              - unit id
 *      trk_gid           - trunk group id
 * Output:
 *      pHash2Port_array - pointer buffer of ports associate with the hash value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. Normal mode support 8 trunk members in each group, but dumb mode only 4.
 */
extern int32
dal_ssw_trunk_hashMappingTable_get(
    uint32                   unit,
    uint32                   trk_gid,
    rtk_trunk_hashVal2Port_t *pHash2Port_array);


/* Function Name:
 *      dal_ssw_trunk_hashMappingTable_set
 * Description:
 *      Set hash value to port array in the trunk group id from the specified device.
 * Input:
 *      unit              - unit id
 *      trk_gid           - trunk group id
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
 *      1. Normal mode support 8 trunk members in each group, but dumb mode only 4.
 */
extern int32
dal_ssw_trunk_hashMappingTable_set(
    uint32                   unit,
    uint32                   trk_gid,
    rtk_trunk_hashVal2Port_t *pHash2Port_array);


/* Function Name:
 *      dal_ssw_trunk_mode_get
 * Description:
 *      Get the trunk mode from the specified device.
 * Input:
 *      unit   - unit id
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
 *      2. Normal mode support 8 trunk members in each group, but dumb mode only 4.
 */
extern int32
dal_ssw_trunk_mode_get(uint32 unit, rtk_trunk_mode_t *pMode);


/* Function Name:
 *      dal_ssw_trunk_mode_set
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
 *      2. Normal mode support 8 trunk members in each group, but dumb mode only 4.
 */
extern int32
dal_ssw_trunk_mode_set(uint32 unit, rtk_trunk_mode_t mode);


/* Function Name:
 *      dal_ssw_trunk_port_get
 * Description:
 *      Get the members of the trunk id from the specified device.
 * Input:
 *      unit                    - unit id
 *      trk_gid                 - trunk group id
 * Output:
 *      pTrunk_member_portmask - pointer buffer of trunk member ports
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. Normal mode support 8 trunk members in each group, but dumb mode only 4.
 */
extern int32
dal_ssw_trunk_port_get(uint32 unit, uint32 trk_gid, rtk_portmask_t *pTrunk_member_portmask);


/* Function Name:
 *      dal_ssw_trunk_port_set
 * Description:
 *      Set the members of the trunk id to the specified device.
 * Input:
 *      unit                    - unit id
 *      trk_gid                 - trunk group id
 *      pTrunk_member_portmask - trunk member ports
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_LA_TRUNK_ID       - invalid trunk ID
 *      RT_ERR_NULL_POINTER      - null pointer
 *      RT_ERR_LA_MEMBER_OVERLAP - the specified port mask is overlapped with other group
 *      RT_ERR_LA_PORTNUM_DUMB   - it can only aggregate at most four ports when 802.1ad dumb mode
 *      RT_ERR_LA_PORTNUM_NORMAL - it can only aggregate at most eight ports when 802.1ad normal mode
 *      RT_ERR_LA_PORTMASK       - error port mask
 * Note:
 *      1. Normal mode support 8 trunk members in each group, but dumb mode only 4.
 */
extern int32
dal_ssw_trunk_port_set(uint32 unit, uint32 trk_gid, rtk_portmask_t *pTrunk_member_portmask);

/* Function Name:
 *      dal_ssw_trunk_port_link_notification
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
dal_ssw_trunk_port_link_notification(uint32 unit, rtk_port_t Trunk_member_port, rtk_port_linkStatus_t LinkStatus);

#endif /* __DAL_SSW_TRUNK_H__ */
