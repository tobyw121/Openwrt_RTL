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
 * $Revision: 57334 $
 * $Date: 2015-03-30 14:13:45 +0800 (Mon, 30 Mar 2015) $
 *
 * Purpose : Definition of Port API
 *
 * Feature : The file have include the following module and sub-modules
 *           (1) Parameter settings for the port-based view
 *           (2) UDLD
 *           (3) RLDP
 */

#ifndef __RTK_PORT_H__
#define __RTK_PORT_H__

/*
 * Include Files
 */
#include <common/rt_type.h>

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */
typedef enum rtk_port_linkStatus_e
{
    PORT_LINKDOWN = 0,
    PORT_LINKUP,
    PORT_LINKSTATUS_END
} rtk_port_linkStatus_t;

typedef enum rtk_port_duplex_e
{
    PORT_HALF_DUPLEX = 0,
    PORT_FULL_DUPLEX,
    PORT_DUPLEX_END
} rtk_port_duplex_t;

typedef enum rtk_port_speed_e
{
    PORT_SPEED_10M = 0,
    PORT_SPEED_100M,
    PORT_SPEED_1000M,
    PORT_SPEED_500M,
    PORT_SPEED_2G,   /* Applicable to 8380 */
    PORT_SPEED_10G, /* Applicable to 8390 */
    PORT_SPEED_END
} rtk_port_speed_t;

typedef enum rtk_port_crossOver_mode_e
{
    PORT_CROSSOVER_MODE_AUTO = 0,
    PORT_CROSSOVER_MODE_MDI,
    PORT_CROSSOVER_MODE_MDIX,
    PORT_CROSSOVER_MODE_END
} rtk_port_crossOver_mode_t;

typedef enum rtk_port_crossOver_status_e
{
    PORT_CROSSOVER_STATUS_MDI = 0,
    PORT_CROSSOVER_STATUS_MDIX,
    PORT_CROSSOVER_STATUS_END
} rtk_port_crossOver_status_t;


typedef enum rtk_port_masterSlave_e
{
    PORT_AUTO_MODE = 0,
    PORT_SLAVE_MODE,
    PORT_MASTER_MODE,
    PORT_MASTER_SLAVE_END
} rtk_port_masterSlave_t;

typedef enum rtk_port_localPacket_action_e
{
    PORT_LOCAL_PACKET_DROP = 0,
    PORT_LOCAL_PACKET_FORWARD,
    PORT_LOCAL_PACKET_END
} rtk_port_localPacket_action_t;

typedef enum rtk_port_phy_reg_e
{
    PHY_REG_CONTROL             = 0,
    PHY_REG_STATUS,
    PHY_REG_IDENTIFIER_1,
    PHY_REG_IDENTIFIER_2,
    PHY_REG_AN_ADVERTISEMENT,
    PHY_REG_AN_LINKPARTNER,
    PHY_REG_1000_BASET_CONTROL  = 9,
    PHY_REG_1000_BASET_STATUS,
    PHY_REG_END                 = 32
} rtk_port_phy_reg_t;

typedef struct rtk_port_phy_ability_s
{
    uint32 Half_10:1;
    uint32 Full_10:1;
    uint32 Half_100:1;
    uint32 Full_100:1;
    uint32 Half_1000:1;
    uint32 Full_1000:1;
#if defined(CONFIG_SDK_RTL8390)
    uint32 Half_10G:1;
    uint32 Full_10G:1;
#endif
    uint32 FC:1;
    uint32 AsyFC:1;
} rtk_port_phy_ability_t;

typedef enum rtk_port_vlanIsolationSrc_e
{
    VLAN_ISOLATION_SRC_INNER = 0,
    VLAN_ISOLATION_SRC_OUTER,
    VLAN_ISOLATION_SRC_FORWARD,
    VLAN_ISOLATION_SRC_END
} rtk_port_vlanIsolationSrc_t;

typedef struct rtk_port_vlanIsolationEntry_e
{
    rtk_enable_t enable;
    rtk_vlan_t vid; /* VID low bound for 8390 */
#if defined(CONFIG_SDK_RTL8390)
    rtk_vlan_t vid_high;
#endif
    rtk_portmask_t portmask;
} rtk_port_vlanIsolationEntry_t;

/* UDLD status */
typedef enum rtk_port_udldStatus_e
{
    UDLD_UNIDIR = 0,        /* unidirectional link */
    UDLD_BIDIR,             /* bidirectional link */
    UDLD_UNDETERMINE,       /* test is not finished */
    UDLD_END
} rtk_port_udldStatus_t;

typedef enum rtk_port_udldInterval_e
{
    UDLD_INTERVAL_2S = 0,
    UDLD_INTERVAL_4S,
    UDLD_INTERVAL_8S,
    UDLD_INTERVAL_16S,
    UDLD_INTERVAL_END
} rtk_port_udldInterval_t;

typedef enum rtk_port_udldEchoAction_e
{
    UDLD_ACTION_SELFLOOPBACK = 0,
    UDLD_ACTION_FORWARD,
    UDLD_ACTION_END
} rtk_port_udldEchoAction_t;

typedef enum rtk_port_udldLinkStatus_e
{
    UDLD_LINK_STATUS_NORMAL = 0,
    UDLD_LINK_STATUS_DISABLE,
    UDLD_LINK_STATUS_END
} rtk_port_udldLinkStatus_t;

/* RLDP: status of normal loop */
typedef enum rtk_port_rldpNormalStatus_e
{
    RLDP_NORMAL_LOOP = 0,
    RLDP_NO_NORMAL_LOOP,
    RLDP_NORMAL_END
} rtk_port_rldpNormalStatus_t;

/* RLDP: status of self loop */
typedef enum rtk_port_rldpSelfStatus_e
{
    RLDP_FORWARDING = 0,
    RLDP_LISTEN,
    RLDP_BLOCK,
    RLDP_END
} rtk_port_rldpSelfStatus_t;

typedef enum rtk_port_10gMeida_e
{
    PORT_10GMEDIA_FIBER_10G,
    PORT_10GMEDIA_FIBER_1G,
    PORT_10GMEDIA_DAC_50CM,
    PORT_10GMEDIA_DAC_100CM,
    PORT_10GMEDIA_DAC_300CM,
    PORT_10GMEDIA_END
} rtk_port_10gMedia_t;

typedef int32 (*rtk_port_linkMon_callback_t)(int32 unit, rtk_port_t port, rtk_port_linkStatus_t new_linkStatus);
typedef int32 (*rtk_port_phyReconfig_callback_t)(int32 unit, rtk_portmask_t * portmask);


/*
 * Macro Declaration
 */
/* for backward compatiable */
#define rtk_port_macRemoteLoopbackEnable_get rtk_diag_portMacRemoteLoopbackEnable_get
#define rtk_port_macRemoteLoopbackEnable_set rtk_diag_portMacRemoteLoopbackEnable_set
#define rtk_port_macLocalLoopbackEnable_get rtk_diag_portMacLocalLoopbackEnable_get
#define rtk_port_macLocalLoopbackEnable_set rtk_diag_portMacLocalLoopbackEnable_set
#define rtk_port_rtctResult_get rtk_diag_portRtctResult_get
#define rtk_port_rtctEnable_set rtk_diag_rtctEnable_set

/*
 * Function Declaration
 */

/* Function Name:
 *      rtk_port_init
 * Description:
 *      Initialize port module of the specified device.
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
 *      Must initialize port module before calling any port APIs.
 */
extern int32
rtk_port_init(uint32 unit);

/* Module Name    : Port                                       */
/* Sub-module Name: Parameter settings for the port-based view */

/* Function Name:
 *      rtk_port_link_get
 * Description:
 *      Get the link status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pStatus - pointer to the link status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The link status of the port is as following:
 *      - PORT_LINKDOWN
 *      - PORT_LINKUP
 */
extern int32
rtk_port_link_get(uint32 unit, rtk_port_t port, rtk_port_linkStatus_t *pStatus);

/* Function Name:
 *      rtk_port_linkMedia_get
 * Description:
 *      Get the link status with media information of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pStatus - pointer to the link status
 *      pMedia  - pointer to the media type
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The link status of the port is as following:
 *          - PORT_LINKDOWN
 *          - PORT_LINKUP
 *      (2) The media type of the port is as following:
 *          - PORT_MEDIA_COPPER
 *          - PORT_MEDIA_FIBER
 *      (3) When the link status is link-down, the return media should be ignored.
 */
extern int32
rtk_port_linkMedia_get(uint32 unit, rtk_port_t port, rtk_port_linkStatus_t *pStatus, rtk_port_media_t *pMedia);

/* Function Name:
 *      rtk_port_speedDuplex_get
 * Description:
 *      Get the negotiated port speed and duplex status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pSpeed  - pointer to the port speed
 *      pDuplex - pointer to the port duplex
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 *      RT_ERR_PORT_LINKDOWN - link down port status
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The speed type of the port is as following:
 *          - PORT_SPEED_10M
 *          - PORT_SPEED_100M
 *          - PORT_SPEED_1000M
 *          - PORT_SPEED_2G  (Applicable to 8380)
 *          - PORT_SPEED_10G (Applicable to 8390)
 *      (2) The duplex mode of the port is as following:
 *          - PORT_HALF_DUPLEX
 *          - PORT_FULL_DUPLEX
 */
extern int32
rtk_port_speedDuplex_get(
    uint32            unit,
    rtk_port_t        port,
    rtk_port_speed_t  *pSpeed,
    rtk_port_duplex_t *pDuplex);

/* Function Name:
 *      rtk_port_flowctrl_get
 * Description:
 *      Get the negotiated flow control status of the specific port
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pTxStatus - pointer to the negotiation result of the Tx flow control
 *      pRxStatus - pointer to the negotiation result of the Rx flow control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 *      RT_ERR_PORT_LINKDOWN - link down port status
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_port_flowctrl_get(
    uint32      unit,
    rtk_port_t  port,
    uint32      *pTxStatus,
    uint32      *pRxStatus);

/* Function Name:
 *      rtk_port_phyAutoNegoEnable_get
 * Description:
 *      Get PHY ability of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to PHY auto negotiation status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_port_phyAutoNegoEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_port_phyAutoNegoEnable_set
 * Description:
 *      Set PHY ability of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable PHY auto negotiation
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - input parameter out of range
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      - ENABLED : switch to PHY auto negotiation mode
 *      - DISABLED: switch to PHY force mode
 *      - Once the abilities of both auto-nego and force mode are set,
 *        you can freely switch the mode without calling ability setting API again
 */
extern int32
rtk_port_phyAutoNegoEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_port_phyAutoNegoAbility_get
 * Description:
 *      Get PHY auto negotiation ability of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pAbility - pointer to the PHY ability
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_port_phyAutoNegoAbility_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_port_phy_ability_t  *pAbility);

/* Function Name:
 *      rtk_port_phyAutoNegoAbility_set
 * Description:
 *      Set PHY auto negotiation ability of the specific port
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      pAbility - pointer to the PHY ability
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      You can set these abilities no matter which mode PHY currently stays on
 */
extern int32
rtk_port_phyAutoNegoAbility_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_port_phy_ability_t  *pAbility);

/* Function Name:
 *      rtk_port_phyForceModeAbility_get
 * Description:
 *      Get PHY ability status of the specific port
 * Input:
 *      unit         - unit id
 *      port         - port id
 * Output:
 *      pSpeed       - pointer to the port speed
 *      pDuplex      - pointer to the port duplex
 *      pFlowControl - pointer to the flow control enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_port_phyForceModeAbility_get(
    uint32              unit,
    rtk_port_t          port,
    rtk_port_speed_t    *pSpeed,
    rtk_port_duplex_t   *pDuplex,
    rtk_enable_t        *pFlowControl);

/* Function Name:
 *      rtk_port_phyForceModeAbility_set
 * Description:
 *      Set the port speed/duplex mode/pause/asy_pause in the PHY force mode
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      speed       - port speed
 *      duplex      - port duplex mode
 *      flowControl - enable flow control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID    - invalid unit id
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_PORT_ID    - invalid port id
 *      RT_ERR_PHY_SPEED  - invalid PHY speed setting
 *      RT_ERR_PHY_DUPLEX - invalid PHY duplex setting
 *      RT_ERR_INPUT      - invalid input parameter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) You can set these abilities no matter which mode PHY currently stays on
 *      (2) The speed type of the port is as following:
 *          - PORT_SPEED_10M
 *          - PORT_SPEED_100M
 *          - PORT_SPEED_1000M (only for fiber media)
 *      (3) The duplex mode of the port is as following:
 *          - PORT_HALF_DUPLEX
 *          - PORT_FULL_DUPLEX
 */
extern int32
rtk_port_phyForceModeAbility_set(
    uint32              unit,
    rtk_port_t          port,
    rtk_port_speed_t    speed,
    rtk_port_duplex_t   duplex,
    rtk_enable_t        flowControl);

/* Function Name:
 *      rtk_port_phyMasterSlave_get
 * Description:
 *      Get PHY configuration of master/slave mode of the specific port
 * Input:
 *      unit                - unit id
 *      port                - port id
 * Output:
 *      pMasterSlaveCfg     - pointer to the PHY master slave configuration
 *      pMasterSlaveActual  - pointer to the PHY master slave actual link status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      This function only works on giga/ 10g port to get its master/slave mode configuration.
 */
extern int32
rtk_port_phyMasterSlave_get(
    uint32              unit,
    rtk_port_t          port,
    rtk_port_masterSlave_t   *pMasterSlaveCfg,
    rtk_port_masterSlave_t   *pMasterSlaveActual);

/* Function Name:
 *      rtk_port_phyMasterSlave_set
 * Description:
 *      Set PHY configuration of master/slave mode of the specific port
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      masterSlave - PHY master slave configuration
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
 *      8390, 8380
 * Note:
 *      This function only works on giga/ 10g port to set its master/slave mode configuration.
 */
extern int32
rtk_port_phyMasterSlave_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_port_masterSlave_t  masterSlave);

/* Function Name:
 *      rtk_port_phyReg_get
 * Description:
 *      Get PHY register data of the specific port
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      page  - page id
 *      reg   - reg id
 * Output:
 *      pData - pointer to the PHY reg data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid page id
 *      RT_ERR_PHY_REG_ID   - invalid reg id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_port_phyReg_get(
    uint32              unit,
    rtk_port_t          port,
    uint32              page,
    rtk_port_phy_reg_t  reg,
    uint32              *pData);

/* Function Name:
 *      rtk_port_phyReg_set
 * Description:
 *      Set PHY register data of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 *      page - page id
 *      reg  - reg id
 *      data - reg data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_PORT_ID     - invalid port id
 *      RT_ERR_PHY_PAGE_ID - invalid page id
 *      RT_ERR_PHY_REG_ID  - invalid reg id
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_port_phyReg_set(
    uint32              unit,
    rtk_port_t          port,
    uint32              page,
    rtk_port_phy_reg_t  reg,
    uint32              data);

/* Function Name:
 *      rtk_port_phyReg_broadcast_set
 * Description:
 *      Set PHY register data in broadcast way
 * Input:
 *      unit - unit id
 *      page - page id
 *      reg  - reg id
 *      data - reg data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_PHY_PAGE_ID - invalid page id
 *      RT_ERR_PHY_REG_ID  - invalid reg id
 * Applicable:
 *      8390
 * Note:
 *      Specified PHY register of all connected PHYs are set simultaneously. The API is used when
 *      set PHY register to all connected PHYs at the same time is desirous.
 */
extern int32
rtk_port_phyReg_broadcast_set(
    uint32              unit,
    uint32              page,
    rtk_port_phy_reg_t  reg,
    uint32              data);

/* Function Name:
 *      rtk_port_phyReg_broadcastID_set
 * Description:
 *      Set PHY broadcast ID
 * Input:
 *      unit        - unit id
 *      broadcastID - broadcast id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8390
 * Note:
 *      Use the API to change the PHY broadcast ID. Default broadcast ID is 31.
 */
extern int32
rtk_port_phyReg_broadcastID_set(
    uint32              unit,
    uint32              broadcastID);

/* Function Name:
 *      rtk_port_phyExtParkPageReg_get
 * Description:
 *      Get PHY register data of the specific port with extension page and parking page parameters
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      mainPage - main page id
 *      extPage  - extension page id
 *      parkPage - parking page id
 *      reg      - reg id
 * Output:
 *      pData    - pointer to the PHY reg data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid page id
 *      RT_ERR_PHY_REG_ID   - invalid reg id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      None
 */
extern int32
rtk_port_phyExtParkPageReg_get(
    uint32              unit,
    rtk_port_t          port,
    uint32              mainPage,
    uint32              extPage,
    uint32              parkPage,
    rtk_port_phy_reg_t  reg,
    uint32              *pData);

/* Function Name:
 *      rtk_port_phyExtParkPageReg_set
 * Description:
 *      Set PHY register data of the specific port with extension page and parking page parameters
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      mainPage - main page id
 *      extPage  - extension page id
 *      parkPage - parking page id
 *      reg      - reg id
 *      data     - reg data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_PORT_ID     - invalid port id
 *      RT_ERR_PHY_PAGE_ID - invalid page id
 *      RT_ERR_PHY_REG_ID  - invalid reg id
 * Applicable:
 *      8390
 * Note:
 *      None
 */
extern int32
rtk_port_phyExtParkPageReg_set(
    uint32              unit,
    rtk_port_t          port,
    uint32              mainPage,
    uint32              extPage,
    uint32              parkPage,
    rtk_port_phy_reg_t  reg,
    uint32              data);

/* Function Name:
 *      rtk_port_phymaskExtParkPageReg_set
 * Description:
 *      Set PHY register data of the specific portmask with extension page and parking page parameters
 * Input:
 *      unit      - unit id
 *      pPortmask - pointer to the portmask
 *      mainPage  - main page id
 *      extPage   - extension page id
 *      parkPage  - parking page id
 *      reg       - reg id
 *      data      - reg data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid page id
 *      RT_ERR_PHY_REG_ID   - invalid reg id
 * Applicable:
 *      8390
 * Note:
 *      None
 */
extern int32
rtk_port_phymaskExtParkPageReg_set(
    uint32              unit,
    rtk_portmask_t      *pPortmask,
    uint32              mainPage,
    uint32              extPage,
    uint32              parkPage,
    rtk_port_phy_reg_t  reg,
    uint32              data);

/* Function Name:
 *      rtk_port_phyMmdReg_get
 * Description:
 *      Get PHY MMD register data of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      mmdAddr - mmd device address
 *      mmdReg  - mmd reg id
 * Output:
 *      pData   - pointer to the PHY reg data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_port_phyMmdReg_get(
    uint32              unit,
    rtk_port_t          port,
    uint32              mmdAddr,
    uint32              mmdReg,
    uint32              *pData);

/* Function Name:
 *      rtk_port_phyMmdReg_set
 * Description:
 *      Set PHY MMD register data of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      mmdAddr - mmd device address
 *      mmdReg  - mmd reg id
 *      data    - reg data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_port_phyMmdReg_set(
    uint32              unit,
    rtk_port_t          port,
    uint32              mmdAddr,
    uint32              mmdReg,
    uint32              data);

/* Function Name:
 *      rtk_port_phymaskMmdReg_set
 * Description:
 *      Set PHY MMD register data of the specific portmask
 * Input:
 *      unit      - unit id
 *      pPortmask - pointer to the portmask
 *      mmdAddr   - mmd device address
 *      mmdReg    - mmd reg id
 *      data      - reg data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_port_phymaskMmdReg_set(
    uint32              unit,
    rtk_portmask_t      *pPortmask,
    uint32              mmdAddr,
    uint32              mmdReg,
    uint32              data);

/* Function Name:
 *      rtk_port_cpuPortId_get
 * Description:
 *      Get CPU port id of the specific unit
 * Input:
 *      unit  - unit id
 * Output:
 *      pPort - pointer to CPU port id
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_port_cpuPortId_get(uint32 unit, rtk_port_t *pPort);

/* Function Name:
 *      rtk_port_cpuPortId_set
 * Description:
 *      Set CPU port id of the specific unit
 * Input:
 *      unit  - unit id
 *      port - pointer to CPU port id
 * Output:
*       None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 * Applicable:
 *      8380
 * Note:
 *      None
 */
extern int32
rtk_port_cpuPortId_set(uint32 unit, rtk_port_t port);

/* Function Name:
 *      rtk_port_isolation_get
 * Description:
 *      Get the portmask of the port isolation
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pPortmask - pointer to the portmask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      Each port can specify an allowable egress forwarding portmask.
 */
extern int32
rtk_port_isolation_get(uint32 unit, rtk_port_t port, rtk_portmask_t *pPortmask);

/* Function Name:
 *      rtk_port_isolation_set
 * Description:
 *      Set the portmask of the port isolation
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      portmask - pointer to the portmask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID   - invalid unit id
 *      RT_ERR_NOT_INIT  - The module is not initial
 *      RT_ERR_PORT_ID   - invalid port id
 *      RT_ERR_PORT_MASK - invalid port mask
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      Each port can specify an allowable egress forwarding portmask.
 */
extern int32
rtk_port_isolation_set(uint32 unit, rtk_port_t port, rtk_portmask_t portmask);

/* Function Name:
 *      rtk_port_isolation_add
 * Description:
 *      Add an isolation port to the certain port
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      iso_port - isolation port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) Default value of each port represent bit in portmask is 1
 *      (2) Port and iso_port will be isolated when this API is called
 *      (3) The iso_port to the relative portmask bit will be set to 1
 */
extern int32
rtk_port_isolation_add(uint32 unit, rtk_port_t port, rtk_port_t iso_port);

/* Function Name:
 *      rtk_port_isolation_del
 * Description:
 *      Delete an existing isolation port of the certain port
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      iso_port - isolation port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) Default value of each port represent bit in portmask is 1
 *      (2) Isolated status between the port and the iso_port is removed when this API is called
 *      (3) The iso_port to the relative portmask bit will be set to 0
 */
extern int32
rtk_port_isolation_del(uint32 unit, rtk_port_t port, rtk_port_t iso_port);

/* Function Name:
 *      rtk_port_phyComboPortMedia_get
 * Description:
 *      Get PHY port media of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pMedia - pointer to the port media
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_port_phyComboPortMedia_get(uint32 unit, rtk_port_t port, rtk_port_media_t *pMedia);

/* Function Name:
 *      rtk_port_phyComboPortMedia_set
 * Description:
 *      Set PHY port media of the specific port
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      media - port media
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
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The media value is as following:
 *          - PORT_MEDIA_COPPER
 *          - PORT_MEDIA_FIBER
 *          - PORT_MEDIA_COPPER_AUTO
 *          - PORT_MEDIA_FIBER_AUTO
 *          - PORT_MEDIA_FIBER_AUTO_BY_GPIO (valid if CONFIG_SDK_AUTO_COMBO_MEDIA_BY_GPIO)
 */
extern int32
rtk_port_phyComboPortMedia_set(uint32 unit, rtk_port_t port, rtk_port_media_t media);

/* Function Name:
 *      rtk_port_backpressureEnable_get
 * Description:
 *      Get the half duplex backpressure enable status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to the enable status of backpressure in half duplex mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The mac back pressure enable status of the port is as following:
 *          - DISABLED
 *          - ENABLED
 *      (2) Used to support backpressure in half mode.
 */
extern int32
rtk_port_backpressureEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_port_backpressureEnable_set
 * Description:
 *      Set the half duplex backpressure enable status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of backpressure in half duplex mode
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
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The mac back pressure enable status of the port is as following:
 *          - DISABLED
 *          - ENABLED
 *      (2) Used to support backpressure in half mode.
 */
extern int32
rtk_port_backpressureEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_port_adminEnable_get
 * Description:
 *      Get port admin status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to the port admin status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_port_adminEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_port_adminEnable_set
 * Description:
 *      Set port admin configuration of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - port admin configuration
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
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_port_adminEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_port_linkMon_enable
 * Description:
 *      Enable link monitor thread
 * Input:
 *      scan_interval_us - scan interval in us.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - initialize success
 *      RT_ERR_FAILED       - initialize fail
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - scan interval is too small
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      When enable link monitor thread, all link change interrupt will be handled by thread.
 */
extern int32
rtk_port_linkMon_enable(uint32 scan_interval_us);

/* Function Name:
 *      rtk_port_linkMon_disable
 * Description:
 *      Disable link scan thread
 * Input:
 *      None.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - initialize success
 *      RT_ERR_FAILED   - initialize fail
 *      RT_ERR_NOT_INIT - The module is not initial
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      When disable link monitor thread, all link change interrupt will be callback to upper layer.
 */
extern int32
rtk_port_linkMon_disable(void);

/* Function Name:
 *      rtk_port_linkMon_register
 * Description:
 *      Register callback function for link change notification
 * Input:
 *      linkMon_callback - callback function for link change
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_port_linkMon_register(rtk_port_linkMon_callback_t linkMon_callback);

/* Function Name:
 *      rtk_port_linkMon_unregister
 * Description:
 *      Unregister callback function for link change notification
 * Input:
 *      linkMon_callback    - callback function for link change
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_port_linkMon_unregister(rtk_port_linkMon_callback_t linkMon_callback);

/* Function Name:
 *      rtk_port_linkMon_swScanPorts_set
 * Description:
 *      Configure portmask of software linkscan for certain unit
 * Input:
 *      unit             - callback function for link change
 *      pSwScan_portmask - portmask for software scan
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - initialize success
 *      RT_ERR_FAILED       - initialize fail
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_port_linkMon_swScanPorts_set(uint32 unit, rtk_portmask_t *pSwScan_portmask);

/* Function Name:
 *      rtk_port_linkMon_swScanPorts_get
 * Description:
 *      Get portmask of software linkscan for certain unit
 * Input:
 *      unit             - callback function for link change
 * Output:
 *      pSwScan_portmask - portmask for software scan
 * Return:
 *      RT_ERR_OK           - initialize success
 *      RT_ERR_FAILED       - initialize fail
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_port_linkMon_swScanPorts_get(uint32 unit, rtk_portmask_t *pSwScan_portmask);

/* Function Name:
 *      rtk_port_txEnable_get
 * Description:
 *      Get the TX enable status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pEnable - pointer to the port TX status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The TX enable status of the port is as following:
 *      - DISABLED
 *      - ENABLED
 */
extern int32
rtk_port_txEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_port_txEnable_set
 * Description:
 *      Set the TX enable status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of TX
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
 *      8389, 8328, 8390, 8380
 * Note:
 *      The TX enable status of the port is as following:
 *      - DISABLED
 *      - ENABLED
 */
extern int32
rtk_port_txEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_port_rxEnable_get
 * Description:
 *      Get the RX enable status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pEnable - pointer to the port RX status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The RX enable status of the port is as following:
 *      - DISABLED
 *      - ENABLED
 */
extern int32
rtk_port_rxEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_port_rxEnable_set
 * Description:
 *      Set the RX enable status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of RX
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
 *      8389, 8328, 8390, 8380
 * Note:
 *      The RX enable status of the port is as following:
 *      - DISABLED
 *      - ENABLED
 */
extern int32
rtk_port_rxEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_port_specialCongest_set
 * Description:
 *      Set the congest seconds of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      second - congest timer (seconds)
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
 *      8389, 8390, 8380
 * Note:
 *      (1) The valid range of congest seconds is 0~15 in 8389.
 *      (2) If a port is continuously congested for congest seconds, the system would drain out the packets of the port,
 *          so the control traffic has chance to transmit to solve the congest situation.
 */
extern int32
rtk_port_specialCongest_set(uint32 unit, rtk_port_t port, uint32 second);

/* Function Name:
 *      rtk_port_greenEnable_get
 * Description:
 *      Get the status of green feature of the specific port in the specific unit
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to status of green feature
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390
 * Note:
 *      None
 */
extern int32
rtk_port_greenEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_port_greenEnable_set
 * Description:
 *      Set the statue of green feature of the specific port in the specific unit
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of green feature
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
 *      8389, 8328, 8390
 * Note:
 *      Needs to restart auto-negotiation for the configuration to take effect.
 */
extern int32
rtk_port_greenEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Module Name    : Port */
/* Sub-module Name: UDLD */

/* Function Name:
 *      rtk_port_udldEnable_get
 * Description:
 *      Get enable status of UDLD on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of UDLD
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
extern int32
rtk_port_udldEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_port_udldEnable_set
 * Description:
 *      Set enable status of UDLD on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of UDLD
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
 *      None
 */
extern int32
rtk_port_udldEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_port_udldLinkUpAutoTriggerEnable_get
 * Description:
 *      Get enable status of link up auto trigger UDLD test.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of link up auto trigger UDLD
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
rtk_port_udldLinkUpAutoTriggerEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_port_udldLinkUpAutoTriggerEnable_set
 * Description:
 *      Set enable status of link up auto trigger UDLD test.
 * Input:
 *      unit   - unit id
 *      enable - enable status of link up auto trigger UDLD
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
extern int32
rtk_port_udldLinkUpAutoTriggerEnable_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      rtk_port_udldTrigger_start
 * Description:
 *      Trigger UDLD test on specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 * Applicable:
 *      8328
 * Note:
 *      None
 */
extern int32
rtk_port_udldTrigger_start(uint32 unit, rtk_port_t port);

/* Function Name:
 *      rtk_port_udldAutoDisableFailedPortEnable_get
 * Description:
 *      Get UDLD test status of specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pStatus - pointer to UDLD test status
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
 *      UDLD test status is as following
 *      - UDLD_UNIDIR           UDLD test result is unidirection
 *      - UDLD_BIDIR            UDLD test result is bidirection
 *      - UDLD_UNDETERMINE      UDLD test is on going. Wait more time to get result.
 */
extern int32
rtk_port_udldStatus_get(uint32 unit, rtk_port_t port, rtk_port_udldStatus_t *pStatus);

/* Function Name:
 *      rtk_port_udldAutoDisableFailedPortEnable_get
 * Description:
 *      Get enable status of auto disable UDLD failed port.
 *      If enabled, switch will automatically disable unidirectional port.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of auto disabled UDLD failed port
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
rtk_port_udldAutoDisableFailedPortEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_port_udldAutoDisableFailedPortEnable_set
 * Description:
 *      Set enable status of auto disable UDLD failed port.
 *      If enabled, switch will automatically disable unidirectional port.
 * Input:
 *      unit   - unit id
 *      enable - enable status of auto disabled UDLD failed port
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
extern int32
rtk_port_udldAutoDisableFailedPortEnable_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      rtk_port_udldInterval_get
 * Description:
 *      Get UDLD echo generate interval.
 * Input:
 *      unit      - unit id
 * Output:
 *      pInterval - pointer to interval of UDLD echo generate (unit: second)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The valid interval value is 2, 4, 8 and 16 seconds
 */
extern int32
rtk_port_udldInterval_get(uint32 unit, rtk_port_udldInterval_t *pInterval);

/* Function Name:
 *      rtk_port_udldInterval_set
 * Description:
 *      Set UDLD echo generate interval.
 * Input:
 *      unit     - unit id
 *      interval - interval of UDLD echo generate (unit: second)
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
 *      The valid interval value is 2, 4, 8 and 16 seconds
 */
extern int32
rtk_port_udldInterval_set(uint32 unit, rtk_port_udldInterval_t interval);

/* Function Name:
 *      rtk_port_udldRetryCount_get
 * Description:
 *      Get retry count of UDLD.
 * Input:
 *      unit        - unit id
 * Output:
 *      pRetryCount - pointer to retry count of UDLD
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The valid retryCount value is 1, 2, 4, 8
 */
extern int32
rtk_port_udldRetryCount_get(uint32 unit, uint32 *pRetryCount);

/* Function Name:
 *      rtk_port_udldRetryCount_set
 * Description:
 *      Set retry count of UDLD.
 * Input:
 *      unit       - unit id
 *      retryCount - retry count of UDLD
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
 *      The valid retryCount value is 1, 2, 4, 8
 */
extern int32
rtk_port_udldRetryCount_set(uint32 unit, uint32 retryCount);

/* Function Name:
 *      rtk_port_udldLedIndicateEnable_get
 * Description:
 *      Get enable status of LED indication when unidirectional link detection.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of LED indication
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
rtk_port_udldLedIndicateEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_port_udldLedIndicateEnable_set
 * Description:
 *      Set enable status of LED indication when unidirectional link detection.
 * Input:
 *      unit   - unit id
 *      enable - enable status of LED indication
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
extern int32
rtk_port_udldLedIndicateEnable_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      rtk_port_udldEchoAction_get
 * Description:
 *      Get udld echo packet action.
 * Input:
 *      unit   - unit id
 * Output:
 *      pAction - echo packet action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The valid action of udld echo packet is as following:
 *      - UDLD_ACTION_SELFLOOPBACK
 *      - UDLD_ACTION_FORWARD
 */
extern int32
rtk_port_udldEchoAction_get(uint32 unit, rtk_port_udldEchoAction_t *pAction);

/* Function Name:
 *      rtk_port_udldEchoAction_set
 * Description:
 *      Set udld echo packet action.
 * Input:
 *      unit   - unit id
 *      action - echo packet action
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
 *      The valid action of udld echo packet is as following:
 *      - UDLD_ACTION_SELFLOOPBACK
 *      - UDLD_ACTION_FORWARD
 */
extern int32
rtk_port_udldEchoAction_set(uint32 unit, rtk_port_udldEchoAction_t action);

/* Function Name:
 *      rtk_port_udldLinkStatus_get
 * Description:
 *      Get udld port link status.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pStatus - link status
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
 *      The valid value of udld link status is as following:
 *      - UDLD_LINK_STATUS_NORMAL
 *      - UDLD_LINK_STATUS_DISABLE
 */
extern int32
rtk_port_udldLinkStatus_get(uint32 unit, rtk_port_t port, rtk_port_udldLinkStatus_t *pStatus);

/* Function Name:
 *      rtk_port_udldLinkStatus_set
 * Description:
 *      Set udld link status.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      status - link status
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
 *      The valid value of udld link status is as following:
 *      - UDLD_LINK_STATUS_NORMAL
 *      - UDLD_LINK_STATUS_DISABLE
 */
extern int32
rtk_port_udldLinkStatus_set(uint32 unit, rtk_port_t port, rtk_port_udldLinkStatus_t status);

/* Module Name    : Port */
/* Sub-module Name: RLDP */

 /* Function Name:
 *      rtk_port_rldpEnable_get
 * Description:
 *      Get enable status of RLDP on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of RLDP
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
extern int32
rtk_port_rldpEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

 /* Function Name:
 *      rtk_port_rldpEnable_set
 * Description:
 *      Set enable status of RLDP on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of RLDP
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
 *      None
 */
extern int32
rtk_port_rldpEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_port_rldpStatus_get
 * Description:
 *      Get result of RLDP test on specified port.
 * Input:
 *      unit          - unit id
 *      port          - port id
 * Output:
 *      pNormalStatus - Status of normal loop test
 *      pSelfStatus   - Status of self loop test
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
 *      (1) Result of RLDP self loop is as following
 *          - RLDP_FORWARDING
 *          - RLDP_LISTEN
 *          - RLDP_BLOCK
 *      (2) Result of RLDP normal loop is as following
 *          - RLDP_NORMAL_LOOP
 *          - RLDP_NO_NORMAL_LOOP
 */
extern int32
rtk_port_rldpStatus_get(
    uint32                          unit,
    rtk_port_t                      port,
    rtk_port_rldpNormalStatus_t     *pNormalStatus,
    rtk_port_rldpSelfStatus_t       *pSelfStatus);

/* Function Name:
 *      rtk_port_rldpAutoBlockEnable_get
 * Description:
 *      Get enable status of auto blocking self loop port on specified port.
 *      When enabled, switch will automatically block self loop port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of auto blocking self loop port
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
extern int32
rtk_port_rldpAutoBlockEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_port_rldpAutoBlockEnable_set
 * Description:
 *      Set enable status of auto blocking self loop port on specified port.
 *      When enabled, switch will automatically block self loop port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of auto blocking self loop port
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
 *      None
 */
extern int32
rtk_port_rldpAutoBlockEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_port_rldpInterval_get
 * Description:
 *      Get interval of RLDP hello interval on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pInterval - pointer to interval of RLDP hello interval(unit: second)
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
 *      The valid value of RLDP hello interval is 1, 2, 4, 8 second
 */
extern int32
rtk_port_rldpInterval_get(uint32 unit, rtk_port_t port, uint32 *pInterval);

/* Function Name:
 *      rtk_port_rldpInterval_set
 * Description:
 *      Set interval of RLDP hello interval on specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      interval - interval of RLDP hello interval(unit: second)
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
 *      The valid value of RLDP hello interval is 1, 2, 4, 8 second
 */
extern int32
rtk_port_rldpInterval_set(uint32 unit, rtk_port_t port, uint32 interval);

/* Function Name:
 *      rtk_port_rldpSelfLoopAgingTime_get
 * Description:
 *      Get aging time of self loop on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pAgingTime - pointer to aging time of self loop(unit: second)
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
 *      The valid value of aging time of RLDP self loop is 1, 2, 4, 8 second
 */
extern int32
rtk_port_rldpSelfLoopAgingTime_get(uint32 unit, rtk_port_t port, uint32 *pAgingTime);

/* Function Name:
 *      rtk_port_rldpSelfLoopAgingTime_set
 * Description:
 *      Set aging time of self loop on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      agingTime - aging time of self loop(unit: second)
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
 *      The valid value of aging time of RLDP self loop is 1, 2, 4, 8 second
 */
extern int32
rtk_port_rldpSelfLoopAgingTime_set(uint32 unit, rtk_port_t port, uint32 agingTime);

/* Function Name:
 *      rtk_port_rldpNormalLoopAgingTime_get
 * Description:
 *      Get aging time of normal loop on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pAgingTime - pointer to aging time of normal loop(unit: second)
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
 *      The valid value of aging time of RLDP normal loop is 1, 2, 4, 8 second
 */
extern int32
rtk_port_rldpNormalLoopAgingTime_get(uint32 unit, rtk_port_t port, uint32 *pAgingTime);

/* Function Name:
 *      rtk_port_rldpNormalLoopAgingTime_set
 * Description:
 *      Set aging time of normal loop on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      agingTime - aging time of normal loop(unit: second)
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
 *      The valid value of aging time of RLDP normal loop is 1, 2, 4, 8 second
 */
extern int32
rtk_port_rldpNormalLoopAgingTime_set(uint32 unit, rtk_port_t port, uint32 agingTime);

/* Function Name:
 *      rtk_port_phyCrossOverMode_get
 * Description:
 *      Get cross over mode in the specified port.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pMode - pointer to cross over mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The valid value of mode is as following:
 *      - PORT_CROSSOVER_MODE_AUTO
 *      - PORT_CROSSOVER_MODE_MDI
 *      - PORT_CROSSOVER_MODE_MDIX
 */
extern int32
rtk_port_phyCrossOverMode_get(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t *pMode);

/* Function Name:
 *      rtk_port_phyCrossOverMode_set
 * Description:
 *      Set cross over mode in the specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 *      mode - cross over mode
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
 *      8389, 8328, 8390, 8380
 * Note:
 *      The valid value of mode is as following:
 *      - PORT_CROSSOVER_MODE_AUTO
 *      - PORT_CROSSOVER_MODE_MDI
 *      - PORT_CROSSOVER_MODE_MDIX
 */
extern int32
rtk_port_phyCrossOverMode_set(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t mode);

/* Function Name:
 *      rtk_port_phyCrossOverStatus_get
 * Description:
 *      Get cross over status in the specified port.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pStatus - pointer to cross over mode status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_PHY_FIBER_LINKUP - This feature is not supported in this mode
 * Applicable:
 *      8390, 8380
 * Note:
 *      The valid value of mode is as following:
 *      - PORT_CROSSOVER_MODE_MDI
 *      - PORT_CROSSOVER_MODE_MDIX
 */
extern int32
rtk_port_phyCrossOverStatus_get(uint32 unit, rtk_port_t port, rtk_port_crossOver_status_t *pStatus);


/* Function Name:
 *      rtk_port_flowCtrlEnable_get
 * Description:
 *      Get the flow control status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to the status of the flow control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The API get the flow control status by port based, no matter N-WAY is enabled or disabled.
 */
extern int32
rtk_port_flowCtrlEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_port_flowCtrlEnable_get
 * Description:
 *      Set the flow control status to the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of flow control
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
 *      8389, 8328, 8390, 8380
 * Note:
 *      The API is apply the flow control status by port based, no matter N-WAY is enabled or disabled.
 */
extern int32
rtk_port_flowCtrlEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_port_phyComboPortFiberMedia_get
 * Description:
 *      Get PHY port fiber media of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pMedia - pointer to the port fiber media
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      1. fiber media type value is as following:
 *      - PORT_FIBER_MEDIA_1000
 *      - PORT_FIBER_MEDIA_100
 *      - PORT_FIBER_MEDIA_AUTO
 */
extern int32
rtk_port_phyComboPortFiberMedia_get(uint32 unit, rtk_port_t port, rtk_port_fiber_media_t *pMedia);

/* Function Name:
 *      rtk_port_phyComboPortFiberMedia_set
 * Description:
 *      Set PHY port fiber media of the specific port
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      media - port fiber media
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - invalid input parameter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      1. fiber media type value is as following:
 *      - PORT_FIBER_MEDIA_1000
 *      - PORT_FIBER_MEDIA_100
 *      - PORT_FIBER_MEDIA_AUTO
 */
extern int32
rtk_port_phyComboPortFiberMedia_set(uint32 unit, rtk_port_t port, rtk_port_fiber_media_t media);

/* Function Name:
 *      rtk_port_linkDownPowerSavingEnable_get
 * Description:
 *      Get the statue of link-down power saving of the specific port in the specific unit
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to status of green feature
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_port_linkDownPowerSavingEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_port_linkDownPowerSavingEnable_set
 * Description:
 *      Set the statue of link-down power saving of the specific port in the specific unit
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of green feature
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - input parameter out of range
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_port_linkDownPowerSavingEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_port_vlanBasedIsolationEntry_get
 * Description:
 *      Get VLAN-based port isolation entry
 * Input:
 *      unit   - unit id
 *      index  - index id
 * Output:
 *      pEntry - pointer to vlan-based port isolation entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      Valid index is 0~15.
 */
extern int32
rtk_port_vlanBasedIsolationEntry_get(uint32 unit, uint32 index, rtk_port_vlanIsolationEntry_t* pEntry);

/* Function Name:
 *      rtk_port_vlanBasedIsolationEntry_set
 * Description:
 *      Set VLAN-based port isolation entry
 * Input:
 *      unit   - unit id
 *      index  - index id
 *      pEntry - pointer to vlan-based port isolation entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                              - invalid unit id
 *      RT_ERR_NOT_INIT                             - The module is not initial
 *      RT_ERR_INPUT                                - invalid input parameter
 *      RT_ERR_NULL_POINTER                         - input parameter may be null pointer
 *      RT_ERR_VLAN_VID                             - invalid vid
 *      RT_ERR_PORT_VLAN_ISO_VID_EXIST_IN_OTHER_IDX - vid exists in other entry
 * Applicable:
 *      8390, 8380
 * Note:
 *      Valid index is 0~15.
 */
extern int32
rtk_port_vlanBasedIsolationEntry_set(uint32 unit, uint32 index, rtk_port_vlanIsolationEntry_t* pEntry);

/* Function Name:
 *      rtk_port_vlanBasedIsolation_vlanSource_get
 * Description:
 *      Get VLAN source of VLAN-based port isolation
 * Input:
 *      unit     - unit id
 * Output:
 *      pVlanSrc - pointer to vlan source
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_port_vlanBasedIsolation_vlanSource_get(uint32 unit, rtk_port_vlanIsolationSrc_t *pVlanSrc);

/* Function Name:
 *      rtk_port_vlanBasedIsolation_vlanSource_set
 * Description:
 *      Set VLAN source of VLAN-based port isolation
 * Input:
 *      unit    - unit id
 *      vlanSrc - vlan source
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
 *      None
 */
extern int32
rtk_port_vlanBasedIsolation_vlanSource_set(uint32 unit, rtk_port_vlanIsolationSrc_t vlanSrc);

/* Function Name:
 *      rtk_port_gigaLiteEnable_get
 * Description:
 *      Get the statue of Giga Lite of the specific port in the specific unit
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to status of Giga Lite
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_port_gigaLiteEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_port_gigaLiteEnable_set
 * Description:
 *      Set the statue of Giga Lite of the specific port in the specific unit
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of Giga Lite
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - input parameter out of range
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_port_gigaLiteEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_port_phyReconfig_register
 * Description:
 *      Register callback function for PHY need to reconfigure notification
 * Input:
 *       phyNotification_callback    - callback function for reconfigure notification
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_port_phyReconfig_register(rtk_port_phyReconfig_callback_t phyNotification_callback);

/* Function Name:
 *      rtk_port_phyReconfig_unregister
 * Description:
 *      UnRegister callback function for PHY need to reconfigure notification
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_port_phyReconfig_unregister(void);

/* Function Name:
 *      rtk_port_downSpeedEnable_get
 * Description:
 *      Get down speed status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pEnable - down speed status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_port_downSpeedEnable_get(uint32 unit, rtk_port_t port,
    rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_port_downSpeedEnable_set
 * Description:
 *      Set down speed status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - down speed status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_port_downSpeedEnable_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable);

/* Function Name:
 *      rtk_port_fiberDownSpeedEnable_get
 * Description:
 *      Get fiber down speed status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pEnable - fiber down speed status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Applicable:
 *      8390
 * Note:
 *      None
 */
extern int32
rtk_port_fiberDownSpeedEnable_get(uint32 unit, rtk_port_t port,
    rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_port_fiberDownSpeedEnable_set
 * Description:
 *      Set fiber down speed status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - fiber down speed status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Applicable:
 *      8390
 * Note:
 *      None
 */
extern int32
rtk_port_fiberDownSpeedEnable_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable);

/* Function Name:
 *      rtk_port_fiberNwayForceLinkEnable_get
 * Description:
 *      When fiber port is configured N-way,
 *      which can link with link partner is configured force mode.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pEnable - fiber Nway force links status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Applicable:
 *      8390
 * Note:
 *      None
 */
extern int32
rtk_port_fiberNwayForceLinkEnable_get(uint32 unit, rtk_port_t port,
    rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_port_fiberMedia_set
 * Description:
 *      When fiber port is configured N-way,
 *      which can link with link partner is configured force mode.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - fiber Nway force links status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Applicable:
 *      8390
 * Note:
 *      None
 */
extern int32
rtk_port_fiberNwayForceLinkEnable_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable);


/* Function Name:
 *      rtk_port_fiberOAMLoopBackEnable_set
 * Description:
 *     Set Fiber-port OAM Loopback enable or not,
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - fiber oam loopback enable or not
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Applicable:
 *      8380,8390
 * Note:
 *      None
 */
extern int32
rtk_port_fiberOAMLoopBackEnable_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable);

/* Function Name:
 *      rtk_port_fiberInternalLoopBackEnable_set
 * Description:
 *     Set Fiber-port Internal Loopback enable or not,
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - fiber internal loopback enable or not
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Applicable:
 *      8380,8390
 * Note:
 *      None
 */
extern int32
rtk_port_fiberInternalLoopBackEnable_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable);


/* Function Name:
 *      rtk_port_10gMedia_set
 * Description:
 *      Set 10G port media of the specific port
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      media - port media
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
 *      8390
 * Note:
 *      (1) The media value is as following:
 *          - PORT_10GMEDIA_FIBER_10G,
 *          - PORT_10GMEDIA_FIBER_1G,
 *          - PORT_10GMEDIA_DAC_50CM,
 *          - PORT_10GMEDIA_DAC_100CM,
 *          - PORT_10GMEDIA_DAC_300CM,
 */
extern int32
rtk_port_10gMedia_set(uint32 unit, rtk_port_t port, rtk_port_10gMedia_t media);

/* Function Name:
 *      rtk_port_10gMedia_get
 * Description:
 *      Get 10G port media of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      media   - pointer to the media type
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      (1) The media type of the port is as following:
 *          - PORT_10GMEDIA_FIBER_10G,
 *          - PORT_10GMEDIA_FIBER_1G,
 *          - PORT_10GMEDIA_DAC_50CM,
 *          - PORT_10GMEDIA_DAC_100CM,
 *          - PORT_10GMEDIA_DAC_300CM,
 */
extern int32
rtk_port_10gMedia_get(uint32 unit, rtk_port_t port, rtk_port_10gMedia_t *media);

/* Function Name:
 *      rtk_port_10gSds_restart
 * Description:
 *      Restart 10G port serdes
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Applicable:
 *      8390
 * Note:
 *      None
 */
extern int32
rtk_port_10gSds_restart(uint32 unit, rtk_port_t port);

/* Function Name:
 *      rtk_port_10g_init
 * Description:
 *      Init 10G port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Applicable:
 *      8390
 * Note:
 *      None
 */
extern int32
rtk_port_10g_init(uint32 unit, rtk_port_t port);

#endif /* __RTK_PORT_H__ */
