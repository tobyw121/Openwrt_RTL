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
 * $Revision: 28107 $
 * $Date: 2012-04-18 18:08:50 +0800 (Wed, 18 Apr 2012) $
 *
 * Purpose : Definition of TRUNK API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) User Configuration Trunk
 *
 */

/*
 * Include Files
 */
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <dal/dal_mgmt.h>
#include <rtk/default.h>
#include <rtk/trunk.h>

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

/* Module Name : TRUNK */

/* Function Name:
 *      rtk_trunk_init
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
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      Must initialize trunk module before calling any trunk APIs.
 */
int32
rtk_trunk_init(uint32 unit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trunk_init(unit);
} /* end of rtk_trunk_init */

/* Module Name    : Trunk                    */
/* Sub-module Name: User configuration trunk */

/* Function Name:
 *      rtk_trunk_distributionAlgorithm_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328
 * Note:
 *      (1) The valid range of trk_gid is 0~7 in 8389, 8328.
 *      (2) You can use OR opertions in following bits to decide your algorithm.
 *          - TRUNK_DISTRIBUTION_ALGO_SPA_BIT        (source port)
 *          - TRUNK_DISTRIBUTION_ALGO_SMAC_BIT       (source mac)
 *          - TRUNK_DISTRIBUTION_ALGO_DMAC_BIT       (destination mac)
 *          - TRUNK_DISTRIBUTION_ALGO_SIP_BIT        (source ip)
 *          - TRUNK_DISTRIBUTION_ALGO_DIP_BIT        (destination ip)
 *          - TRUNK_DISTRIBUTION_ALGO_SRC_L4PORT_BIT (source layer4 port)
 *          - TRUNK_DISTRIBUTION_ALGO_DST_L4PORT_BIT (destination layer4 port)
 */
int32
rtk_trunk_distributionAlgorithm_get(uint32 unit, uint32 trk_gid, uint32 *pAlgo_bitmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trunk_distributionAlgorithm_get(unit, trk_gid, pAlgo_bitmask);
} /* end of rtk_trunk_distributionAlgorithm_get */

/* Function Name:
 *      rtk_trunk_distributionAlgorithm_set
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
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_LA_TRUNK_ID - invalid trunk ID
 *      RT_ERR_LA_HASHMASK - invalid hash mask
 * Applicable:
 *      8389, 8328
 * Note:
 *      (1) The valid range of trk_gid is 0~7 in 8389, 8328.
 *      (2) You can use OR opertions in following bits to decide your algorithm.
 *          - TRUNK_DISTRIBUTION_ALGO_SPA_BIT        (source port)
 *          - TRUNK_DISTRIBUTION_ALGO_SMAC_BIT       (source mac)
 *          - TRUNK_DISTRIBUTION_ALGO_DMAC_BIT       (destination mac)
 *          - TRUNK_DISTRIBUTION_ALGO_SIP_BIT        (source ip)
 *          - TRUNK_DISTRIBUTION_ALGO_DIP_BIT        (destination ip)
 *          - TRUNK_DISTRIBUTION_ALGO_SRC_L4PORT_BIT (source layer4 port)
 *          - TRUNK_DISTRIBUTION_ALGO_DST_L4PORT_BIT (destination layer4 port)
 */
int32
rtk_trunk_distributionAlgorithm_set(uint32 unit, uint32 trk_gid, uint32 algo_bitmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trunk_distributionAlgorithm_set(unit, trk_gid, algo_bitmask);
} /* end of rtk_trunk_distributionAlgorithm_set */

/* Function Name:
 *      rtk_trunk_hashMappingTable_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328
 * Note:
 *      (1) The valid range of trk_gid is 0~7 in 8389 and 8328
 */
int32
rtk_trunk_hashMappingTable_get(
    uint32                   unit,
    uint32                   trk_gid,
    rtk_trunk_hashVal2Port_t *pHash2Port_array)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trunk_hashMappingTable_get(unit, trk_gid, pHash2Port_array);
} /* end of rtk_trunk_hashMappingTable_get */

/* Function Name:
 *      rtk_trunk_hashMappingTable_set
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
 *      RT_ERR_NOT_INIT           - The module is not initial
 *      RT_ERR_LA_TRUNK_ID        - invalid trunk ID
 *      RT_ERR_NULL_POINTER       - input parameter may be null pointer
 *      RT_ERR_LA_TRUNK_NOT_EXIST - the trunk doesn't exist
 *      RT_ERR_LA_NOT_MEMBER_PORT - the port is not a member port of the trunk
 *      RT_ERR_LA_CPUPORT         - CPU port can not be aggregated port
 * Applicable:
 *      8389, 8328
 * Note:
 *      (1) The valid range of trk_gid is 0~7 in 8389 and 8328
 */
int32
rtk_trunk_hashMappingTable_set(
    uint32                   unit,
    uint32                   trk_gid,
    rtk_trunk_hashVal2Port_t *pHash2Port_array)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trunk_hashMappingTable_set(unit, trk_gid, pHash2Port_array);
} /* end of rtk_trunk_hashMappingTable_set */

/* Function Name:
 *      rtk_trunk_mode_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328
 * Note:
 *      The enum of the trunk mode as following
 *      - TRUNK_MODE_NORMAL
 *      - TRUNK_MODE_DUMB
 */
int32
rtk_trunk_mode_get(uint32 unit, rtk_trunk_mode_t *pMode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trunk_mode_get(unit, pMode);
} /* end of rtk_trunk_mode_get */

/* Function Name:
 *      rtk_trunk_mode_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8389, 8328
 * Note:
 *      The enum of the trunk mode as following
 *      - TRUNK_MODE_NORMAL
 *      - TRUNK_MODE_DUMB
 */
int32
rtk_trunk_mode_set(uint32 unit, rtk_trunk_mode_t mode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trunk_mode_set(unit, mode);
} /* end of rtk_trunk_mode_set */

/* Function Name:
 *      rtk_trunk_port_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The valid range of trk_gid is 0~7 in 8389, 8328 and 8380, 0~15 in 8390.
 */
int32
rtk_trunk_port_get(uint32 unit, uint32 trk_gid, rtk_portmask_t *pTrunk_member_portmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32   ret;
    rtk_portmask_t  temp_portmask;

    if ((ret = RT_MAPPER(unit)->trunk_port_get(unit, trk_gid, &temp_portmask)) != RT_ERR_OK)
        return ret;
    *pTrunk_member_portmask = PHYSICAL_PORTMASK_TO_RTK_PORTMASK(unit, temp_portmask);
    return ret;
    }
#else
    return RT_MAPPER(unit)->trunk_port_get(unit, trk_gid, pTrunk_member_portmask);
#endif   
} /* end of rtk_trunk_port_get */

/* Function Name:
 *      rtk_trunk_port_set
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
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_LA_TRUNK_ID       - invalid trunk ID
 *      RT_ERR_LA_MEMBER_OVERLAP - the specified port mask is overlapped with other group
 *      RT_ERR_LA_PORTNUM_DUMB   - it can only aggregate at most four ports when 802.1ad dumb mode
 *      RT_ERR_LA_PORTNUM_NORMAL - it can only aggregate at most eight ports when 802.1ad normal mode
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The valid range of trk_gid is 0~7 in 8389, 8328 and 8380, 0~15 in 8390.
 */
int32
rtk_trunk_port_set(uint32 unit, uint32 trk_gid, rtk_portmask_t *pTrunk_member_portmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    rtk_portmask_t  temp_portmask;

    temp_portmask = RTK_PORTMASK_TO_PHYSICAL_PORTMASK(unit, *pTrunk_member_portmask);
    return RT_MAPPER(unit)->trunk_port_set(unit, trk_gid, &temp_portmask);
    }
#else
    return RT_MAPPER(unit)->trunk_port_set(unit, trk_gid, pTrunk_member_portmask);
#endif
} /* end of rtk_trunk_port_set */

/* Function Name:
 *      rtk_trunk_representPort_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of trk_gid is 0~7 in 8389 and 8328
 */
int32
rtk_trunk_representPort_get(uint32 unit, uint32 trunk_id, rtk_port_t *pRepresPort)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trunk_representPort_get(unit, trunk_id, pRepresPort);
} /* end of rtk_trunk_representPort_get */

/* Function Name:
 *      rtk_trunk_representPort_set
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
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_LA_TRUNK_ID - invalid trunk id
 *      RT_ERR_PORT_ID     - invalid port id
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of trk_gid is 0~7 in 8389 and 8328
 */
int32
rtk_trunk_representPort_set(uint32 unit, uint32 trunk_id, rtk_port_t represPort)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trunk_representPort_set(unit, trunk_id, represPort);
} /* end of rtk_trunk_representPort_set */

/* Function Name:
 *      rtk_trunk_floodMode_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of trk_gid is 0~7 in 8389 and 8328
 *      (2) The valid value of flood mode is as following
 *          - FLOOD_PORT_BY_HASH
 *          - FLOOD_PORT_BY_CONFIG
 */
int32
rtk_trunk_floodMode_get(uint32 unit, uint32 trunk_id, rtk_trunk_floodMode_t *pFloodMode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trunk_floodMode_get(unit, trunk_id, pFloodMode);
} /* end of rtk_trunk_floodMode_get */

/* Function Name:
 *      rtk_trunk_floodMode_set
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
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_LA_TRUNK_ID - invalid trunk id
 *      RT_ERR_INPUT       - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of trk_gid is 0~7 in 8389 and 8328
 *      (2) The valid value of flood mode is as following
 *          - FLOOD_PORT_BY_HASH
 *          - FLOOD_PORT_BY_CONFIG
 */
int32
rtk_trunk_floodMode_set(uint32 unit, uint32 trunk_id, rtk_trunk_floodMode_t floodMode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trunk_floodMode_set(unit, trunk_id, floodMode);
} /* end of rtk_trunk_floodMode_set */

/* Function Name:
 *      rtk_trunk_floodPort_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of trk_gid is 0~7 in 8389 and 8328
 */
int32
rtk_trunk_floodPort_get(uint32 unit, uint32 trunk_id, rtk_port_t *pFloodPort)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trunk_floodPort_get(unit, trunk_id, pFloodPort);
} /* end of rtk_trunk_floodPort_get */

/* Function Name:
 *      rtk_trunk_floodPort_set
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
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_LA_TRUNK_ID - invalid trunk id
 *      RT_ERR_PORT_ID     - invalid port id
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of trk_gid is 0~7 in 8389 and 8328
 */
int32
rtk_trunk_floodPort_set(uint32 unit, uint32 trunk_id, rtk_port_t floodPort)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trunk_floodPort_set(unit, trunk_id, floodPort);
} /* end of rtk_trunk_floodPort_set */

/* Function Name:
 *      rtk_trunk_port_link_notification
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
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_LA_TRUNK_ID - invalid trunk ID
 * Applicable:
 *      8389, 8328
 * Note:
 *      None
 */
int32
rtk_trunk_port_link_notification(uint32 unit, rtk_port_t trunkMemberPort, rtk_port_linkStatus_t linkStatus)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trunk_port_link_notification(unit, trunkMemberPort, linkStatus);
} /* end of rtk_trunk_port_link_notification */

/* Function Name:
 *      rtk_trunk_distributionAlgorithmBind_get
 * Description:
 *      Get the distribution algorithm ID binded for a trunk group from the specified device.
 * Input:
 *      unit      - unit id
 *      trk_gid   - trunk group id
 * Output:
 *      pAlgo_idx - pointer buffer of the distribution algorithm ID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) The valid range of trk_gid is 0~7 in 8380, 0~15 in 8390.
 *      (2) The valid range of algo_idx is 0~1 in 8380, 0~3 in 8390.
 */
int32
rtk_trunk_distributionAlgorithmBind_get(uint32 unit, uint32 trk_gid, uint32 *pAlgo_idx)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trunk_distributionAlgorithmBind_get(unit, trk_gid, pAlgo_idx);
} /* end of rtk_trunk_distributionAlgorithmBind_get */

/* Function Name:
 *      rtk_trunk_distributionAlgorithmBind_set
 * Description:
 *      Set the distribution algorithm ID binded for a trunk group from the specified device.
 * Input:
 *      unit     - unit id
 *      trk_gid  - trunk group id
 *      algo_idx - index the distribution algorithm
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_LA_TRUNK_ID - invalid trunk ID
 *      RT_ERR_LA_ALGO_ID  - invalid trunk algorithm ID
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) The valid range of trk_gid is 0~7 in 8380, 0~15 in 8390.
 *      (2) The valid range of algo_idx is 0~1 in 8380, 0~3 in 8390.
 */
int32
rtk_trunk_distributionAlgorithmBind_set(uint32 unit, uint32 trk_gid, uint32 algo_idx)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trunk_distributionAlgorithmBind_set(unit, trk_gid, algo_idx);
} /* end of rtk_trunk_distributionAlgorithmBind_set */

/* Function Name:
 *      rtk_trunk_distributionAlgorithmParam_get
 * Description:
 *      Get the distribution algorithm by algorithm ID from the specified device.
 * Input:
 *      unit          - unit id
 *      algo_idx      - algorithm index
 * Output:
 *      pAlgo_bitmask - pointer buffer of bitmask of the distribution algorithm
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_LA_ALGO_ID   - invalid trunk algorithm ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) The valid range of algo_idx is 0~1 in 8380, 0~3 in 8390.
 */
int32
rtk_trunk_distributionAlgorithmParam_get(uint32 unit, uint32 algo_idx, uint32 *pAlgo_bitmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trunk_distributionAlgorithmParam_get(unit, algo_idx, pAlgo_bitmask);
} /* end of rtk_trunk_distributionAlgorithmParam_get */

/* Function Name:
 *      rtk_trunk_distributionAlgorithmParam_set
 * Description:
 *      Set the distribution algorithm by algorithm ID from the specified device.
 * Input:
 *      unit         - unit id
 *      algo_idx     - algorithm index
 *      algo_bitmask - bitmask of the distribution algorithm
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_LA_ALGO_ID  - invalid trunk algorithm ID
 *      RT_ERR_LA_HASHMASK - invalid hash mask
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) The valid range of algo_idx is 0~1 in 8380, 0~3 in 8390.
 *      (2) You can use OR opertions in following bits to decide your algorithm.
 *          - TRUNK_DISTRIBUTION_ALGO_SPA_BIT        (source port)
 *          - TRUNK_DISTRIBUTION_ALGO_SMAC_BIT       (source mac)
 *          - TRUNK_DISTRIBUTION_ALGO_DMAC_BIT       (destination mac)
 *          - TRUNK_DISTRIBUTION_ALGO_SIP_BIT        (source ip)
 *          - TRUNK_DISTRIBUTION_ALGO_DIP_BIT        (destination ip)
 *          - TRUNK_DISTRIBUTION_ALGO_SRC_L4PORT_BIT (source layer4 port)
 *          - TRUNK_DISTRIBUTION_ALGO_DST_L4PORT_BIT (destination layer4 port)
 */
int32
rtk_trunk_distributionAlgorithmParam_set(uint32 unit, uint32 algo_idx, uint32 algo_bitmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trunk_distributionAlgorithmParam_set(unit, algo_idx, algo_bitmask);
} /* end of rtk_trunk_distributionAlgorithmParam_set */

/* Function Name:
 *      rtk_trunk_distributionAlgorithmShift_get
 * Description:
 *      Get the shift bits of distribution algorithm parameters from the specified device.
 * Input:
 *      unit     - unit id
 *      algo_idx - algorithm index
 * Output:
 *      pShift   - pointer buffer of shift bits of the distribution algorithm
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_LA_ALGO_ID   - invalid trunk algorithm ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      Instead use the fixed hash algorithm provided by the device, the API can shift each hash algorithm
 *      factor to have different distribution path.
 */
int32
rtk_trunk_distributionAlgorithmShift_get(uint32 unit, uint32 algo_idx, rtk_trunk_distAlgoShift_t *pShift)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trunk_distributionAlgorithmShift_get(unit, algo_idx, pShift);
} /* end of rtk_trunk_distributionAlgorithmShift_get */

/* Function Name:
 *      rtk_trunk_distributionAlgorithmShift_set
 * Description:
 *      Set the shift bits of distribution algorithm parameters from the specified device.
 * Input:
 *      unit     - unit id
 *      algo_idx - algorithm index
 *      pShift   - shift bits of the distribution algorithm parameters
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 *      RT_ERR_LA_ALGO_ID    - invalid trunk algorithm ID
 *      RT_ERR_LA_ALGO_SHIFT - invalid trunk algorithm shift
 * Applicable:
 *      8390, 8380
 * Note:
 *      Instead use the fixed hash algorithm provided by the device, the API can shift each hash algorithm
 *      factor to have different distribution path.
 */
int32
rtk_trunk_distributionAlgorithmShift_set(uint32 unit, uint32 algo_idx, rtk_trunk_distAlgoShift_t *pShift)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trunk_distributionAlgorithmShift_set(unit, algo_idx, pShift);
} /* end of rtk_trunk_distributionAlgorithmShift_set */

/* Function Name:
 *      rtk_trunk_trafficSeparate_get
 * Description:
 *      Get the traffic separation setting of a trunk group from the specified device.
 * Input:
 *      unit          - unit id
 *      trk_gid       - trunk group id
 * Output:
 *      pSeparateType - pointer separated traffic type
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      SEPARATE_NONE: disable traffic separation
 *      SEPARATE_KNOWN_MULTI: trunk MSB link up port is dedicated to TX known multicast traffic
 *      SEPARATE_FLOOD: trunk MSB link up port is dedicated to TX flooding (L2 lookup miss) traffic
 *      SEPARATE_KNOWN_MULTI_AND_FLOOD: trunk MSB link up port is dedicated to TX known multicast and flooding (L2 lookup miss) traffic
 */
int32
rtk_trunk_trafficSeparate_get(uint32 unit, uint32 trk_gid, rtk_trunk_separateType_t *pSeparateType)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trunk_trafficSeparate_get(unit, trk_gid, pSeparateType);
} /* end of rtk_trunk_trafficSeparate_get */

/* Function Name:
 *      rtk_trunk_trafficSeparate_set
 * Description:
 *      Set the traffic separation setting of a trunk group from the specified device.
 * Input:
 *      unit         - unit id
 *      trk_gid      - trunk group id
 *      separateType - traffic separation setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_LA_TRUNK_ID - invalid trunk ID
 *      RT_ERR_INPUT       - Invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      SEPARATE_NONE: disable traffic separation
 *      SEPARATE_KNOWN_MULTI: trunk MSB link up port is dedicated to TX known multicast traffic
 *      SEPARATE_FLOOD: trunk MSB link up port is dedicated to TX flooding (L2 lookup miss) traffic
 *      SEPARATE_KNOWN_MULTI_AND_FLOOD: trunk MSB link up port is dedicated to TX known multicast and flooding (L2 lookup miss) traffic
 */
int32
rtk_trunk_trafficSeparate_set(uint32 unit, uint32 trk_gid, rtk_trunk_separateType_t separateType)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trunk_trafficSeparate_set(unit, trk_gid, separateType);
} /* end of rtk_trunk_trafficSeparate_set */
