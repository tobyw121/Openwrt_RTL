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
 * $Revision: 38989 $
 * $Date: 2013-04-28 09:21:28 +0800 (Sun, 28 Apr 2013) $
 *
 * Purpose : Definition of Switch Global API
 *
 * Feature : The file have include the following module and sub-modules
 *           (1) Switch parameter settings
 *           (2) Management address and vlan configuration.
 *
 */

#ifndef __RTK_SWITCH_H__
#define __RTK_SWITCH_H__

/*
 * Include Files
 */
#include <common/rt_autoconf.h>
#include <common/rt_type.h>
#include <hal/chipdef/chip.h>

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */
/* information of device */
typedef struct rtk_switch_devInfo_s
{
    uint32  chipId;
    uint32  revision;
    uint32  familyId;
    uint32  port_number;
    rt_portType_info_t fe;
    rt_portType_info_t ge;
    rt_portType_info_t ge_combo;
    rt_portType_info_t serdes;
    rt_portType_info_t ge_10ge;
    rt_portType_info_t ether;
    rt_portType_info_t all;
    rt_register_capacity_t  capacityInfo;
    int32   cpuPort; /* use (-1) for VALUE_NO_INIT */
} rtk_switch_devInfo_t;

typedef struct switch_reg_value_s
{
    uint32  reg;
    uint32  val;
} switch_reg_value_t;


/* type of the acceptable direction */
typedef enum rtk_switch_pktDir_e
{
    PKTDIR_BOTH = 0,
    PKTDIR_RX,
    PKTDIR_TX,
    PKTDIR_END
} rtk_switch_pktDir_t;

/* type of the acceptable packet length */
typedef enum rtk_switch_maxPktLen_e
{
    MAXPKTLEN_1522B = 0,
    MAXPKTLEN_1536B,
    MAXPKTLEN_1552B,
    MAXPKTLEN_9216B,
    MAXPKTLEN_END
} rtk_switch_maxPktLen_t;

/* type of the acceptable packet length in different link speed */
typedef enum rtk_switch_maxPktLen_linkSpeed_e
{
    MAXPKTLEN_LINK_SPEED_FE = 0,
    MAXPKTLEN_LINK_SPEED_GE,
    MAXPKTLEN_LINK_SPEED_END,
} rtk_switch_maxPktLen_linkSpeed_t;

/* type of checksum fail */
typedef enum rtk_switch_chksum_fail_e
{
    LAYER2_CHKSUM_FAIL = 0,
    LAYER3_CHKSUM_FAIL, /* Applicable to 8328 */
    LAYER4_CHKSUM_FAIL, /* Applicable to 8328 */
    CHKSUM_FAIL_END
} rtk_switch_chksum_fail_t;

/* type of HW delay */
typedef enum rtk_switch_delayType_e
{
    DELAY_TYPE_INTRA_LINK0_RX = 0,
    DELAY_TYPE_INTRA_LINK0_TX,
    DELAY_TYPE_INTRA_LINK1_RX,
    DELAY_TYPE_INTRA_LINK1_TX,
    DELAY_TYPE_END
} rtk_switch_delayType_t;

typedef enum rtk_switch_pkt2CpuType_e
{
    PKT2CPU_TYPE_FORWARD,
    PKT2CPU_TYPE_TRAP,
    PKT2CPU_TYPE_COPY,      /*reserved for future use*/
    PKT2CPU_TYPE_END,
} rtk_switch_pkt2CpuType_t;

/*
 * Macro Definition
 */
#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
/* port transfer */
#define RTK_PORT_TO_PHYSICAL_PORT(unit, rtk_port)       rtk_switch_rtkPortToPhysicalPort(unit, rtk_port)
#define PHYSICAL_PORT_TO_RTK_PORT(unit, physical_port)  rtk_switch_physicalPortToRtkPort(unit, physical_port)
/* port mask transfer */
#define RTK_PORTMASK_TO_PHYSICAL_PORTMASK(unit, rtk_portmask)       rtk_switch_rtkPortmaskToPhysicalPortmask(unit, rtk_portmask)
#define PHYSICAL_PORTMASK_TO_RTK_PORTMASK(unit, physical_portmask)  rtk_switch_physicalPortmaskToRtkPortmask(unit, physical_portmask)
#endif

/*
 * Function Declaration
 */

/* Module Name    : Switch     */
/* Sub-module Name: Switch parameter settings */

/* Function Name:
 *      rtk_switch_init
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
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      Must initialize switch module before calling any switch APIs.
 */
extern int32
rtk_switch_init(uint32 unit);

/* Module Name    : Switch     */
/* Sub-module Name: Switch parameter settings */

/* Function Name:
 *      rtk_switch_deviceInfo_get
 * Description:
 *      Get device information of the specific unit
 * Input:
 *      unit     - unit id
 * Output:
 *      pDevInfo - pointer to the device information
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The routine have provided following information based on SDK probe result
 *      - chip id, chip revision id, chip capacity
 *      - total port number and cpu port id
 *      - detail information in each port type
 */
extern int32
rtk_switch_deviceInfo_get(uint32 unit, rtk_switch_devInfo_t *pDevInfo);

/* Function Name:
 *      rtk_switch_cpuMaxPktLen_get
 * Description:
 *      Get the max packet length setting on CPU port of the specific unit
 * Input:
 *      unit - unit id
 *      dir  - direction of packet
 * Output:
 *      pLen - pointer to the max packet length
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid packet direction
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) Supported Maximum packet lenght is 16383B.
 *      (2) Supported management frame is as following:
 *          - rtk_switch_pktDir_t \ Chip:   8380   8390
 *          - PKTDIR_BOTH                   X       X
 *          - PKTDIR_RX                     O       O
 *          - PKTDIR_TX                     O       O
 */
extern int32
rtk_switch_cpuMaxPktLen_get(uint32 unit, rtk_switch_pktDir_t dir, uint32 *pLen);

/* Function Name:
 *      rtk_switch_cpuMaxPktLen_set
 * Description:
 *      Set the max packet length setting on CPU port of the specific unit
 * Input:
 *      unit - unit id
 *      dir  - direction of packet
 *      len  - max packet length
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid packet direction
 *      RT_ERR_OUT_OF_RANGE - invalid packet length
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) Supported Maximum packet lenght is 16383B.
 *      (2) Supported management frame is as following:
 *          - rtk_switch_pktDir_t \ Chip:   8380   8390
 *          - PKTDIR_BOTH                   X       X
 *          - PKTDIR_RX                     O       O
 *          - PKTDIR_TX                     O       O
 */
extern int32
rtk_switch_cpuMaxPktLen_set(uint32 unit, rtk_switch_pktDir_t dir, uint32 len);

/* Function Name:
 *      rtk_switch_maxPktLen_get
 * Description:
 *      Get the max packet length setting of the specific unit
 * Input:
 *      unit - unit id
 * Output:
 *      pLen - pointer to the max packet length
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389
 * Note:
 *      The valid value of maximum packet length
 *      - MAXPKTLEN_1522B
 *      - MAXPKTLEN_1536B
 *      - MAXPKTLEN_1552B
 *      - MAXPKTLEN_9216B
 */
extern int32
rtk_switch_maxPktLen_get(
    uint32                 unit,
    rtk_switch_maxPktLen_t *pLen);

/* Function Name:
 *      rtk_switch_maxPktLen_set
 * Description:
 *      Set the max packet length of the specific unit
 * Input:
 *      unit - unit id
 *      len  - max packet length
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid enum packet length
 * Applicable:
 *      8389
 * Note:
 *      The valid value of maximum packet length
 *      - MAXPKTLEN_1522B
 *      - MAXPKTLEN_1536B
 *      - MAXPKTLEN_1552B
 *      - MAXPKTLEN_9216B
 */
extern int32
rtk_switch_maxPktLen_set(
    uint32                 unit,
    rtk_switch_maxPktLen_t len);

/* Function Name:
 *      rtk_switch_maxPktLenLinkSpeed_get
 * Description:
 *      Get the max packet length setting of the specific speed type
 * Input:
 *      unit  - unit id
 *      speed - speed type
 * Output:
 *      pLen  - pointer to the max packet length
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid enum speed type
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) Maximum packet length filtering is examined on both receiving and transmitting ports
 *          according to port's link speed.
 *      (2) Supported Maximum packet lenght is 16383B.
 *      (3) Max packet length setting speed type
 *          - MAXPKTLEN_LINK_SPEED_FE
 *          - MAXPKTLEN_LINK_SPEED_GE
 */
extern int32
rtk_switch_maxPktLenLinkSpeed_get(uint32 unit, rtk_switch_maxPktLen_linkSpeed_t speed, uint32 *pLen);

/* Function Name:
 *      rtk_switch_maxPktLenLinkSpeed_set
 * Description:
 *      Set the max packet length of the specific speed type
 * Input:
 *      unit  - unit id
 *      speed - speed type
 *      len   - max packet length
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid enum speed type
 *      RT_ERR_OUT_OF_RANGE - invalid packet length
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) Maximum packet length filtering is examined on both receiving and transmitting ports
 *          according to port's link speed.
 *      (2) Supported Maximum packet lenght is 16383B.
 *      (3) Max packet length setting speed type
 *          - MAXPKTLEN_LINK_SPEED_FE
 *          - MAXPKTLEN_LINK_SPEED_GE
 */
extern int32
rtk_switch_maxPktLenLinkSpeed_set(uint32 unit, rtk_switch_maxPktLen_linkSpeed_t speed, uint32 len);

/* Function Name:
 *      rtk_switch_maxPktLenTagLenCntIncEnable_get
 * Description:
 *      Get include or exclude tag length state of max packet length in the specified device.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer buffer of including/excluding tag length
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      Inner(4B) and outer(4B) tag length can be included or excluded to the maximum packet length filtering.
 */
extern int32
rtk_switch_maxPktLenTagLenCntIncEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_switch_maxPktLenTagLenCntIncEnable_set
 * Description:
 *      Set the max packet length to include or exclude tag length in the specified device.
 * Input:
 *      unit   - unit id
 *      enable - include/exclude Tag length
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      Inner(4B) and outer(4B) tag length can be included or excluded to the maximum packet length filtering.
 */
extern int32
rtk_switch_maxPktLenTagLenCntIncEnable_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      rtk_switch_portMaxPktLen_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The valid range of length is up to 9K in 8328
 */
extern int32
rtk_switch_portMaxPktLen_get(uint32 unit, rtk_port_t port, uint32 *pLength);

/* Function Name:
 *      rtk_switch_portMaxPktLen_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      The valid range of length is up to 9K in 8328
 */
extern int32
rtk_switch_portMaxPktLen_set(uint32 unit, rtk_port_t port, uint32 length);

/* Function Name:
 *      rtk_switch_snapMode_get
 * Description:
 *      Get the status of SNAP qualification mode.
 * Input:
 *      unit      - unit id
 * Output:
 *      pSnapMode - pointer to SNAP mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      OUI(00-00-00) comparsion can be ommited for SNAP qualification.
 *      SNAP mode is as following
 *      - SNAP_MODE_AAAA03000000
 *      - SNAP_MODE_AAAA03
 */
extern int32
rtk_switch_snapMode_get(uint32 unit, rtk_snapMode_t *pSnapMode);

/* Function Name:
 *      rtk_switch_snapMode_set
 * Description:
 *      Set the status of SNAP qualification mode.
 * Input:
 *      unit     - unit id
 *      snapMode - SNAP mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      OUI(00-00-00) comparsion can be ommited for SNAP qualification.
 *      SNAP mode is as following
 *      - SNAP_MODE_AAAA03000000
 *      - SNAP_MODE_AAAA03
 */
extern int32
rtk_switch_snapMode_set(uint32 unit, rtk_snapMode_t snapMode);

/* Function Name:
 *      rtk_switch_portSnapMode_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The valid value of SNAP mode is as following
 *      - SNAP_MODE_AAAA03000000
 *      - SNAP_MODE_AAAA03
 */
extern int32
rtk_switch_portSnapMode_get(uint32 unit, rtk_port_t port, rtk_snapMode_t *pSnapMode);

/* Function Name:
 *      rtk_switch_portSnapMode_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      The valid value of SNAP mode is as following
 *      - SNAP_MODE_AAAA03000000
 *      - SNAP_MODE_AAAA03
 */
extern int32
rtk_switch_portSnapMode_set(uint32 unit, rtk_port_t port, rtk_snapMode_t snapMode);

/* Function Name:
 *      rtk_switch_chksumFailAction_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) Checksum fail type is as following \ Chip :     8328    8390
 *          - LAYER2_CHKSUM_FAIL                            O       O
 *          - LAYER3_CHKSUM_FAIL                            O       X
 *          - LAYER4_CHKSUM_FAIL                            O       X
 *      (2) Forwarding action is as following \ Chip :      8328    8390
 *          - ACTION_FORWARD                                O       O
 *          - ACTION_DROP                                   O       O
 */
extern int32
rtk_switch_chksumFailAction_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_switch_chksum_fail_t    failType,
    rtk_action_t                *pAction);

/* Function Name:
 *      rtk_switch_chksumFailAction_set
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
 *      RT_ERR_UNIT_ID    - invalid unit id
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_PORT_ID    - invalid port id
 *      RT_ERR_FWD_ACTION - invalid error forwarding action
 *      RT_ERR_INPUT      - invalid input parameter
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) Checksum fail type is as following \ Chip :     8328    8390
 *          - LAYER2_CHKSUM_FAIL                            O       O
 *          - LAYER3_CHKSUM_FAIL                            O       X
 *          - LAYER4_CHKSUM_FAIL                            O       X
 *      (2) Forwarding action is as following \ Chip :      8328    8390
 *          - ACTION_FORWARD                                O       O
 *          - ACTION_DROP                                   O       O
 */
extern int32
rtk_switch_chksumFailAction_set(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_switch_chksum_fail_t    failType,
    rtk_action_t                action);

/* Function Name:
 *      rtk_switch_recalcCRCEnable_get
 * Description:
 *      Get enable status of recaculate CRC on specified egress port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of recaculate CRC
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      When enable, mirrored packet with bad CRC will be recaculate at mirroring port.
 *      When enable, only when packet is modified CRC will be recalculated.
 */
extern int32
rtk_switch_recalcCRCEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_switch_recalcCRCEnable_set
 * Description:
 *      Set enable status of recaculate CRC on specified egress port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of recaculate CRC
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
 *      8328, 8390, 8380
 * Note:
 *      When enable, mirrored packet with bad CRC will be recaculate at mirroring port.
 *      When enable, only when packet is modified CRC will be recalculated.
 */
extern int32
rtk_switch_recalcCRCEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Module Name    : Switch     */
/* Sub-module Name: Management address and vlan configuration */

/* Function Name:
 *      rtk_switch_mgmtVlanId_get
 * Description:
 *      Get management vlan of switch.
 * Input:
 *      unit - unit id
 * Output:
 *      pVid - pointer to vlan id
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The valid range of vid is 0~4095
 */
extern int32
rtk_switch_mgmtVlanId_get(uint32 unit, rtk_vlan_t *pVid);

/* Function Name:
 *      rtk_switch_mgmtVlanId_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_VLAN_VID - Invalid vlan id
 * Applicable:
 *      8328
 * Note:
 *      The valid range of vid is 0~4095
 */
extern int32
rtk_switch_mgmtVlanId_set(uint32 unit, rtk_vlan_t vid);

/* Function Name:
 *      rtk_switch_outerMgmtVlanId_get
 * Description:
 *      Get outer vlan of switch.
 * Input:
 *      unit      - unit id
 * Output:
 *      pOuterVid - pointer to outer vlan id
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The valid range of outerVid is 0~4095
 */
extern int32
rtk_switch_outerMgmtVlanId_get(uint32 unit, rtk_vlan_t *pOuterVid);

/* Function Name:
 *      rtk_switch_outerMgmtVlanId_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_VLAN_VID - Invalid vlan id
 * Applicable:
 *      8328
 * Note:
 *      The valid range of outerVid is 0~4095
 */
extern int32
rtk_switch_outerMgmtVlanId_set(uint32 unit, rtk_vlan_t outerVid);

/* Function Name:
 *      rtk_switch_mgmtMacAddr_get
 * Description:
 *      Get MAC address of switch.
 * Input:
 *      unit - unit id
 * Output:
 *      pMac - pointer to MAC address
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The MAC address is used when sending Pause frame.
 */
extern int32
rtk_switch_mgmtMacAddr_get(uint32 unit, rtk_mac_t *pMac);

/* Function Name:
 *      rtk_switch_mgmtMacAddr_set
 * Description:
 *      Set MAC address of switch.
 * Input:
 *      unit - unit id
 *      pMac - MAC address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_MAC          - invalid mac address
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The MAC address is used when sending Pause frame.
 */
extern int32
rtk_switch_mgmtMacAddr_set(uint32 unit, rtk_mac_t *pMac);

/* Function Name:
 *      rtk_switch_IPv4Addr_get
 * Description:
 *      Get IPv4 address of switch.
 * Input:
 *      unit    - unit id
 * Output:
 *      pIpAddr - pointer to IPv4 address
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_switch_IPv4Addr_get(uint32 unit, uint32 *pIpAddr);

/* Function Name:
 *      rtk_switch_IPv4Addr_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_IPV4_ADDRESS - invalid IPv4 address
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_switch_IPv4Addr_set(uint32 unit, uint32 ipAddr);

/* Function Name:
 *      rtk_switch_IPv6Addr_get
 * Description:
 *      Get IPv6 address of switch.
 * Input:
 *      unit      - unit id
 * Output:
 *      pIpv6Addr - pointer to IPv6 address
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
extern int32
rtk_switch_IPv6Addr_get(uint32 unit, rtk_ipv6_addr_t *pIpv6Addr);

/* Function Name:
 *      rtk_switch_IPv6Addr_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_IPV6_ADDRESS - invalid IPv6 address
 * Applicable:
 *      8328
 * Note:
 *      None
 */
extern int32
rtk_switch_IPv6Addr_set(uint32 unit, rtk_ipv6_addr_t ipv6Addr);

/* Function Name:
 *      rtk_switch_hwInterfaceDelayEnable_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328
 * Note:
 *      The valid value of type is as following:
 *      - DELAY_TYPE_INTRA_LINK0_RX
 *      - DELAY_TYPE_INTRA_LINK0_TX
 *      - DELAY_TYPE_INTRA_LINK1_RX
 *      - DELAY_TYPE_INTRA_LINK1_TX
 */
extern int32
rtk_switch_hwInterfaceDelayEnable_get(uint32 unit, rtk_switch_delayType_t type, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_switch_hwInterfaceDelayEnable_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8389, 8328
 * Note:
 *      The valid value of type is as following:
 *      - DELAY_TYPE_INTRA_LINK0_RX
 *      - DELAY_TYPE_INTRA_LINK0_TX
 *      - DELAY_TYPE_INTRA_LINK1_RX
 *      - DELAY_TYPE_INTRA_LINK1_TX
 */
extern int32
rtk_switch_hwInterfaceDelayEnable_set(uint32 unit, rtk_switch_delayType_t type, rtk_enable_t enable);

/* Function Name:
 *      rtk_switch_pkt2CpuFormat_get
 * Description:
 *      Get the packet to CPU format in the specified unit.
 * Input:
 *      unit    - unit id
 * Output:
 *      pFormat - pointer to packet format
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) The valid value of format is as following:
 *          - ORIGINAL_PACKET
 *          - MODIFIED_PACKET
 *      (2) For 8390 and 8380, the function is backward compatible RTL8328 APIs.
 *          If need to get the format with different type,
 *          please use the new API: rtk_switch_pkt2CpuTypeFormat_get
 */
extern int32
rtk_switch_pkt2CpuFormat_get(uint32 unit, rtk_pktFormat_t *pFormat);

/* Function Name:
 *      rtk_switch_pkt2CpuFormat_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) The valid value of format is as following:
 *          - ORIGINAL_PACKET
 *          - MODIFIED_PACKET
 *      (2) For 8390 and 8380, the function is backward compatible RTL8328 APIs.
 *          If need to configure the different type with different format,
 *          please use the new API: rtk_switch_pkt2CpuTypeFormat_set
 */
extern int32
rtk_switch_pkt2CpuFormat_set(uint32 unit, rtk_pktFormat_t format);

/* Function Name:
 *      rtk_switch_softwareResetCounter_get
 * Description:
 *      Get the switch software reset counter of the specified device.
 * Input:
 *      unit     - unit id
 * Output:
 *      pCounter - pointer buffer of software reset counter
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
extern int32
rtk_switch_softwareResetCounter_get(uint32 unit, uint32 *pCounter);

/* Function Name:
 *      rtk_switch_pkt2CpuTypeFormat_get
 * Description:
 *      Get the configuration about content state for packet to CPU
 * Input:
 *      unit    - unit id
 *      type    - method of packet to CPU
 * Output:
 *      pFormat - type of format
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      The type of packet to CPU:
 *      - PKT2CPU_TYPE_FORWARD
 *      - PKT2CPU_TYPE_TRAP
 *
 *      The type of format:
 *      - TRAP_PKT_ORIGINAL
 *      - TRAP_PKT_MODIFIED
 */
extern int32
rtk_switch_pkt2CpuTypeFormat_get(
    uint32                      unit,
    rtk_switch_pkt2CpuType_t    type,
    rtk_pktFormat_t             *pFormat);

/* Function Name:
 *      rtk_switch_pkt2CpuTypeFormat_set
 * Description:
 *      Set the configuration about content state for packet to CPU
 * Input:
 *      unit   - unit id
 *      type   - method of packet to CPU
 *      format - packet format to CPU
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      The type of format:
 *      - TRAP_PKT_ORIGINAL
 *      - TRAP_PKT_MODIFIED
 */
extern int32
rtk_switch_pkt2CpuTypeFormat_set(
    uint32                      unit,
    rtk_switch_pkt2CpuType_t    type,
    rtk_pktFormat_t             format);

/* Function Name:
 *      rtk_switch_pppoePassthrough_get
 * Description:
 *      Get enable status of PPPoE pass-through functionality.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of PPPoE parse functionality
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      Enable the device to recognize PPPoE frame. The PPPoE frame is treated as unknown L2 frame if
 *      PPPoE pass-through is enabled.
 */
extern int32
rtk_switch_pppoePassthrough_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_switch_pppoePassthrough_set
 * Description:
 *      Set enable status of PPPoE pass-through functionality.
 * Input:
 *      unit   - unit id
 *      enable - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      Enable the device to recognize PPPoE frame. The PPPoE frame is treated as unknown L2 frame if
 *      PPPoE pass-through is enabled.
 */
extern int32
rtk_switch_pppoePassthrough_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      rtk_switch_cpuPktTruncateEnable_get
 * Description:
 *      Get CPU port truncation function state.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - enable state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      (1) Packet is truncated to specified length if it is trap/forward to CPU and packet length is
 *          over than the truncation length.
 *      (2) The truncation length is specified by rtk_switch_cpuPktTruncateLen_set.
 */
extern int32
rtk_switch_cpuPktTruncateEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_switch_cpuPktTruncateEnable_set
 * Description:
 *      Set CPU port truncation function state.
 * Input:
 *      unit    - unit id
 *      enable  - enable state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8390
 * Note:
 *      (1) Packet is truncated to specified length if it is trap/forward to CPU and packet length is
 *          over than the truncation length.
 *      (2) The truncation length is specified by rtk_switch_cpuPktTruncateLen_set.
 */
extern int32
rtk_switch_cpuPktTruncateEnable_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      rtk_switch_cpuPktTruncateLen_get
 * Description:
 *      Get the packet length for CPU port truncation function.
 * Input:
 *      unit - unit id
 * Output:
 *      pLen  - packet length to truncate
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      (1) Packet is truncated to specified length if it is trap/forward to CPU and packet length is
 *          over than the truncation length.
 *      (2) The truncation function takes effect if rtk_switch_cpuPktTruncateEnable_set is enabled.
 */
extern int32
rtk_switch_cpuPktTruncateLen_get(uint32 unit, uint32 *pLen);

/* Function Name:
 *      rtk_switch_cpuPktTruncateLen_set
 * Description:
 *      Set the packet length for CPU port truncation function.
 * Input:
 *      unit - unit id
 *      len  - packet length to truncate
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - invalid truncation length
 * Applicable:
 *      8390
 * Note:
 *      (1) Packet is truncated to specified length if it is trap/forward to CPU and packet length is
 *          over than the truncation length.
 *      (2) The truncation function takes effect if rtk_switch_cpuPktTruncateEnable_set is enabled.
 */
extern int32
rtk_switch_cpuPktTruncateLen_set(uint32 unit, uint32 len);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
/* Function Name:
 *      rtk_switch_rtkPortToPhysicalPort
 * Description:
 *      Translate the rtk port id to physical port id.
 * Input:
 *      unit     - unit id
 *      rtk_port - rtk port id
 * Output:
 *      None
 * Return:
 *      physical port id
 * Applicable:
 *      8380
 * Note:
 *      None
 */
extern int32
rtk_switch_rtkPortToPhysicalPort(uint32 unit, rtk_port_t rtk_port);

/* Function Name:
 *      rtk_switch_physicalPortToRtkPort
 * Description:
 *      Translate the physical port id to rtk port id.
 * Input:
 *      unit          - unit id
 *      physical_port - physical port id
 * Output:
 *      None
 * Return:
 *      rtk port id
 * Applicable:
 *      8380
 * Note:
 *      None
 */
extern int32
rtk_switch_physicalPortToRtkPort(uint32 unit, rtk_port_t physical_port);

/* Function Name:
 *      rtk_switch_rtkPortmaskToPhysicalPortmask
 * Description:
 *      Translate the rtk portmask to physical portmask.
 * Input:
 *      unit         - unit id
 *      rtk_portmask - rtk portmask
 * Output:
 *      None
 * Return:
 *      physical portmask
 * Applicable:
 *      8380
 * Note:
 *      None
 */
extern rtk_portmask_t
rtk_switch_rtkPortmaskToPhysicalPortmask(uint32 unit, rtk_portmask_t rtk_portmask);

/* Function Name:
 *      rtk_switch_physicalPortmaskToRtkPortmask
 * Description:
 *      Translate the physical portmask to rtk portmask.
 * Input:
 *      unit              - unit id
 *      physical_portmask - physical portmask
 * Output:
 *      None
 * Return:
 *      rtk portmask
 * Applicable:
 *      8380
 * Note:
 *      None
 */
extern rtk_portmask_t
rtk_switch_physicalPortmaskToRtkPortmask(uint32 unit, rtk_portmask_t physical_portmask);

#endif
#endif /* __RTK_SWITCH_H__ */
