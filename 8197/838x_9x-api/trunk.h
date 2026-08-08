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

#ifndef __RTK_TRUNK_H__
#define __RTK_TRUNK_H__

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/port.h>

/*
 * Symbol Definition
 */
#define TRUNK_DISTRIBUTION_ALGO_SPA_BIT         0x01
#define TRUNK_DISTRIBUTION_ALGO_SMAC_BIT        0x02
#define TRUNK_DISTRIBUTION_ALGO_DMAC_BIT        0x04
#define TRUNK_DISTRIBUTION_ALGO_SIP_BIT         0x08
#define TRUNK_DISTRIBUTION_ALGO_DIP_BIT         0x10
#define TRUNK_DISTRIBUTION_ALGO_SRC_L4PORT_BIT  0x20
#define TRUNK_DISTRIBUTION_ALGO_DST_L4PORT_BIT  0x40
#define TRUNK_DISTRIBUTION_ALGO_MASKALL         0x7F

typedef enum rtk_trunk_mode_e
{
    TRUNK_MODE_NORMAL = 0,
    TRUNK_MODE_DUMB,
    TRUNK_MODE_END
} rtk_trunk_mode_t;

/* Flood mode of trunk */
typedef enum rtk_trunk_floodMode_e
{
    FLOOD_MODE_BY_HASH = 0,
    FLOOD_MODE_BY_CONFIG,
    FLOOD_MODE_END
} rtk_trunk_floodMode_t;

typedef struct  rtk_trunk_hashVal2Port_s
{
    uint8 value[RTK_MAX_NUM_OF_TRUNK_HASH_VAL];
} rtk_trunk_hashVal2Port_t;

typedef struct  rtk_trunk_distAlgoShift_s
{
    uint32 spa_shift;
    uint32 smac_shift;
    uint32 dmac_shift;
    uint32 sip_shift;
    uint32 dip_shift;
    uint32 sport_shift;
    uint32 dport_shift;
} rtk_trunk_distAlgoShift_t;

typedef enum rtk_trunk_separateType_e
{
    SEPARATE_NONE = 0,
    SEPARATE_KNOWN_MULTI,
    SEPARATE_FLOOD,
    SEPARATE_KNOWN_MULTI_AND_FLOOD,
    SEPARATE_END

} rtk_trunk_separateType_t;

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
extern int32
rtk_trunk_init(uint32 unit);

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
extern int32
rtk_trunk_distributionAlgorithm_get(uint32 unit, uint32 trk_gid, uint32 *pAlgo_bitmask);

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
extern int32
rtk_trunk_distributionAlgorithm_set(uint32 unit, uint32 trk_gid, uint32 algo_bitmask);

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
extern int32
rtk_trunk_hashMappingTable_get(
    uint32                   unit,
    uint32                   trk_gid,
    rtk_trunk_hashVal2Port_t *pHash2Port_array);

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
extern int32
rtk_trunk_hashMappingTable_set(
    uint32                   unit,
    uint32                   trk_gid,
    rtk_trunk_hashVal2Port_t *pHash2Port_array);

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
extern int32
rtk_trunk_mode_get(uint32 unit, rtk_trunk_mode_t *pMode);

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
extern int32
rtk_trunk_mode_set(uint32 unit, rtk_trunk_mode_t mode);

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
extern int32
rtk_trunk_port_get(uint32 unit, uint32 trk_gid, rtk_portmask_t *pTrunk_member_portmask);

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
extern int32
rtk_trunk_port_set(uint32 unit, uint32 trk_gid, rtk_portmask_t *pTrunk_member_portmask);

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
extern int32
rtk_trunk_representPort_get(uint32 unit, uint32 trunk_id, rtk_port_t *pRepresPort);

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
extern int32
rtk_trunk_representPort_set(uint32 unit, uint32 trunk_id, rtk_port_t represPort);

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
extern int32
rtk_trunk_floodMode_get(uint32 unit, uint32 trunk_id, rtk_trunk_floodMode_t *pFloodMode);

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
extern int32
rtk_trunk_floodMode_set(uint32 unit, uint32 trunk_id, rtk_trunk_floodMode_t floodMode);

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
extern int32
rtk_trunk_floodPort_get(uint32 unit, uint32 trunk_id, rtk_port_t *pFloodPort);

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
extern int32
rtk_trunk_floodPort_set(uint32 unit, uint32 trunk_id, rtk_port_t floodPort);

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
extern int32
rtk_trunk_port_link_notification(uint32 unit, rtk_port_t trunkMemberPort, rtk_port_linkStatus_t linkStatus);

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
extern int32
rtk_trunk_distributionAlgorithmBind_get(uint32 unit, uint32 trk_gid, uint32 *pAlgo_idx);

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
extern int32
rtk_trunk_distributionAlgorithmBind_set(uint32 unit, uint32 trk_gid, uint32 algo_idx);

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
extern int32
rtk_trunk_distributionAlgorithmParam_get(uint32 unit, uint32 algo_idx, uint32 *pAlgo_bitmask);

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
extern int32
rtk_trunk_distributionAlgorithmParam_set(uint32 unit, uint32 algo_idx, uint32 algo_bitmask);

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
extern int32
rtk_trunk_distributionAlgorithmShift_get(uint32 unit, uint32 algo_idx, rtk_trunk_distAlgoShift_t *pShift);

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
extern int32
rtk_trunk_distributionAlgorithmShift_set(uint32 unit, uint32 algo_idx, rtk_trunk_distAlgoShift_t *pShift);

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
extern int32
rtk_trunk_trafficSeparate_get(uint32 unit, uint32 trk_gid, rtk_trunk_separateType_t *pSeparateType);

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
extern int32
rtk_trunk_trafficSeparate_set(uint32 unit, uint32 trk_gid, rtk_trunk_separateType_t separateType);

#endif /* __RTK_TRUNK_H__ */
