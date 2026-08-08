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
 * $Revision: 36386 $
 * $Date: 2013-01-21 10:32:36 +0800 (Mon, 21 Jan 2013) $
 *
 * Purpose : Definition those public OAM routing APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           (1) OAM (802.3ah) configuration
 *           (2) CFM (802.1ag) configuration
 */

#ifndef __RTK_OAM_H__
#define __RTK_OAM_H__

/*
 * Include Files
 */
#include <common/rt_type.h>

/*
 * Symbol Definition
 */
/* length of MAID */
#define RTK_MAX_LEN_OF_CFM_MAID     48

/* Length of dying gasp TLV */
#define RTK_OAM_REMOTE_TLV_LEN      16
#define RTK_OAM_LOCAL_TLV_LEN       16
#define RTK_OAM_DYINGGASPPAYLOAD_MAX    1522


#define RTK_CFM_INSTANCE_MAX        8       /* max instance of CFM */
#define RTK_CFM_MDL_MAX             8       /* max md level of CFM */
#define RTK_CFM_RESET_LIFETIME_MAX  0x3FF
#define RTK_CFM_MEPID_MAX           0x1FFF
#define RTK_CFM_LIFETIME_MAX        8
#define RTK_CFM_MAID_MAX            8
#define RTK_CFM_TX_INTERVAL         0x3FF   /* max tx interval (msec) */
#define RTK_CFM_CCM_PORT_MAX        2       /* max ports of CCM control member */
#define RTK_CFM_TX_INSTANCE_MAX     8       /* max tx instance of CCM control */
#define RTK_CFM_RX_INSTANCE_MAX     8       /* max rx instance of CCM control */

/*
 * Data Declaration
 */
/* Status of transmitting test frame */
typedef enum rtk_oam_testFrameTxStatus_e
{
    TX_FINISH = 0,
    TX_NOT_FINISH,
    TEST_FRAME_TX_END
} rtk_oam_testFrameTxStatus_t;

/* Loopback mode of port */
typedef enum rtk_oam_loopbackMode_e
{
    OAM_LOOPBACK_ALL_FRAME = 0,
    OAM_LOOPBACK_TEST_FRAME,
    OAM_LOOPBACK_DROP_TEST_FRAME,
    OAM_LOOPBACK_DISABLE,
    OAM_LOOPBACK_END
} rtk_oam_loopbackMode_t;

/* SA action of loopback frame */
typedef enum rtk_oam_loopbackSA_action_e
{
    SA_ACTION_KEEP_MAC = 0,
    SA_ACTION_USE_SWITCH_MAC,
    SA_ACTION_USE_DA,
    SA_ACTION_USE_USER_DEFINE_MAC,
    SA_ACTION_END
} rtk_oam_loopbackSA_action_t;

/* DA action of loopback frame */
typedef enum rtk_oam_loopbackDA_action_e
{
    DA_ACTION_KEEP_MAC = 0,
    DA_ACTION_USE_SWITCH_MAC,
    DA_ACTION_USE_SA,
    DA_ACTION_USE_USER_DEFINE_MAC,
    DA_ACTION_END
} rtk_oam_loopbackDA_action_t;

/* Dying Gasp TLV */
typedef struct rtk_oam_dyingGaspTLV_s
{
    uint8   remoteTLV[RTK_OAM_REMOTE_TLV_LEN];
    uint8   localTLV[RTK_OAM_LOCAL_TLV_LEN];
} rtk_oam_dyingGaspTLV_t;

/* CCM transmit interval */
typedef enum rtk_oam_ccmInterval_e
{
    INTERVAL_3_3_MS = 0,
    INTERVAL_10_MS,
    INTERVAL_100_MS,
    INTERVAL_1_S,
    INTERVAL_10_S,
    INTERVAL_1_MIN,
    INTERVAL_10_MIN,
    INTERVAL_END
} rtk_oam_ccmInterval_t;

/* Continuity check status of remote MEP */
typedef enum rtk_oam_ccStatus_e
{
    CC_STATUS_AGEOUT = 0,
    CC_STATUS_NOT_AGE,
    CC_STATUS_END
} rtk_oam_ccStatus_t;

/* Configuration of transmitting test frame */
typedef struct rtk_oam_testFrameCfg_s
{
    rtk_portmask_t  txPortmask;
    rtk_enable_t    continuousTx;
    uint32          length;
    uint32          frameCount;
    uint32          frameInterval;
} rtk_oam_testFrameCfg_t;

/* Loopback control of port */
typedef struct rtk_oam_loopbackCtrl_s
{
    rtk_oam_loopbackSA_action_t     sa_action;
    rtk_mac_t                       user_defined_src_mac;
    rtk_oam_loopbackDA_action_t     da_action;
    rtk_mac_t                       user_defined_dst_mac;
} rtk_oam_loopbackCtrl_t;

/* Header of CCM frame */
typedef struct rtk_oam_ccmFrame_s
{
    rtk_pktType_t   pktType;    /* ETHERNET_II_PACKET or SNAP_PACKET */
    rtk_enable_t    enable_outerTag;
    uint32          outer_tpid;
    rtk_vlan_t      outer_vid;
    rtk_pri_t       outer_pri;
    uint32          outer_dei;
    rtk_enable_t    enable_innerTag;
    uint32          inner_tpid;
    rtk_vlan_t      inner_vid;
    rtk_pri_t       inner_pri;
    uint32          inner_cfi;
    rtk_mac_t       dest_mac;
    rtk_mac_t       source_mac;
} rtk_oam_ccmFrame_t;

/* CFM configuration of each MA */
typedef struct rtk_oam_cfm_s
{
    uint32      md_level;
    uint8       maid[RTK_MAX_LEN_OF_CFM_MAID];
} rtk_oam_cfm_t;

/* CFM configuration of port */
typedef struct rtk_oam_cfmPort_s
{
    uint32      cfm_idx;
    uint32      mepid;
} rtk_oam_cfmPort_t;

/* Control of replying CFM loopback */
typedef struct rtk_oam_cfmLoopbackCtrl_s
{
    rtk_enable_t    keep_innerTag;
    rtk_enable_t    keep_outerTag;
} rtk_oam_cfmLoopbackCtrl_t;

typedef struct rtk_oam_linkFaultEvent_s
{
    uint8   instance;
    uint8   instancePort;
} rtk_oam_linkFaultEvent_t;

typedef int32 (*rtk_oam_linkFaultMon_callback_t)(rtk_oam_linkFaultEvent_t *);

/*
 * Macro Declaration
 */

/*
 * Function Declaration
 */

/* Function Name:
 *      rtk_oam_init
 * Description:
 *      Initialize OAM module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Applicable:
 *      8328, 8390
 * Note:
 *      Must initialize OAM module before calling any OAM APIs.
 */
extern int32
rtk_oam_init(uint32 unit);

/* Module Name    : OAM               */
/* Sub-module Name: OAM configuration */

/* Function Name:
 *      rtk_oam_oamCounter_get
 * Description:
 *      Get number of received OAM PDU on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pNumber - number of received OAM PDU
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
rtk_oam_oamCounter_get(uint32 unit, rtk_port_t port, uint32 *pNumber);

/* Function Name:
 *      rtk_oam_txTestFrame_start
 * Description:
 *      Start transmitting OAM test frame.
 * Input:
 *      unit          - unit id
 *      pTestFrameCfg - configuration of test frame transmitting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      The routine can start transmitting OAM PDU to those portmask by specified
 *      packet length, number and interval in burst or continue mode.
 */
extern int32
rtk_oam_txTestFrame_start(uint32 unit, rtk_oam_testFrameCfg_t *pTestFrameCfg);

/* Function Name:
 *      rtk_oam_txTestFrame_stop
 * Description:
 *      stop transmitting OAM test frame.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 * Applicable:
 *      8328
 * Note:
 *      None
 */
extern int32
rtk_oam_txTestFrame_stop(uint32 unit);

/* Function Name:
 *      rtk_oam_txStatusOfTestFrame_get
 * Description:
 *      Get transmitting status of test frame.
 * Input:
 *      unit      - unit id
 *      pTxStatus - transmitting status
 *      pTxCount  - number of transmitted packets.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      Transmitting status is as following:
 *      - TX_FINISH
 *      - TX_NOT_FINISH
 */
extern int32
rtk_oam_txStatusOfTestFrame_get(uint32 unit, rtk_oam_testFrameTxStatus_t *pTxStatus, uint64 *pTxCount);

/* Function Name:
 *      rtk_oam_loopbackMode_get
 * Description:
 *      Get OAM loopback mode on specified port.
 * Input:
 *      unit          - unit id
 *      port          - port id
 * Output:
 *      pLoopbackMode - pointer to OAM loopback mode
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
 *      OAM loopback mode is as following:
 *      - OAM_LOOPBACK_ALL_FRAME
 *      - OAM_LOOPBACK_TEST_FRAME
 *      - OAM_LOOPBACK_DROP_TEST_FRAME
 *      - OAM_LOOPBACK_DISABLE
 */
extern int32
rtk_oam_loopbackMode_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_oam_loopbackMode_t  *pLoopbackMode);

/* Function Name:
 *      rtk_oam_loopbackMode_set
 * Description:
 *      Set OAM loopback mode on specified port.
 * Input:
 *      unit         - unit id
 *      port         - port id
 *      loopbackMode - OAM loopback mode
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
 *      OAM loopback mode is as following:
 *      - OAM_LOOPBACK_ALL_FRAME
 *      - OAM_LOOPBACK_TEST_FRAME
 *      - OAM_LOOPBACK_DROP_TEST_FRAME
 *      - OAM_LOOPBACK_DISABLE
 */
extern int32
rtk_oam_loopbackMode_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_oam_loopbackMode_t  loopbackMode);

/* Function Name:
 *      rtk_oam_loopbackCtrl_get
 * Description:
 *      Get control of loopback action on specified port.
 * Input:
 *      unit          - unit id
 *      port          - port id
 * Output:
 *      pLoopbackCtrl - pointer to control of loopback action
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
rtk_oam_loopbackCtrl_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_oam_loopbackCtrl_t  *pLoopbackCtrl);

/* Function Name:
 *      rtk_oam_loopbackCtrl_set
 * Description:
 *      Set control of loopback action on specified port.
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      pLoopbackCtrl - control of loopback action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) There are four fields in rtk_oam_loopbackCtrl_t: sa_acion,
 *          user_defined_src_mac, da_action and user_defined_dst_mac.
 *      (2) sa_action as following:
 *          - SA_ACTION_KEEP_MAC
 *          - SA_ACTION_USE_SWITCH_MAC
 *          - SA_ACTION_USE_DA
 *          - SA_ACTION_USE_USER_DEFINE_MAC
 *      (3) da_action as following:
 *          - DA_ACTION_KEEP_MAC
 *          - DA_ACTION_USE_SWITCH_MAC
 *          - DA_ACTION_USE_SA
 *          - DA_ACTION_USE_USER_DEFINE_MAC
 */
extern int32
rtk_oam_loopbackCtrl_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_oam_loopbackCtrl_t  *pLoopbackCtrl);

/* Function Name:
 *      rtk_oam_DyingGaspSend_start
 * Description:
 *      Start sending dying gasp frame to specified ports.
 * Input:
 *      unit      - unit id
 *      pPortmask - ports for sending dying gasp
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID   - invalid unit id
 *      RT_ERR_NOT_INIT  - The module is not initial
 *      RT_ERR_PORT_MASK - invalid portmask
 * Applicable:
 *      8328
 * Note:
 *      (1) This API will be used when CPU want to send dying gasp by itself.
 *      (2) The ports send out dying gasp frame is setting by
 *         rtk_oam_autoDyingGaspEnable_get.
 */
extern int32
rtk_oam_DyingGaspSend_start(uint32 unit, rtk_portmask_t *pPortmask);

/* Function Name:
 *      rtk_oam_portDyingGaspPayload_set
 * Description:
 *      Set the payload of dying gasp frame to specified ports.
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      pPayload    - payload
 *      len         - payload length
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
 *      (1) The ports send out dying gasp frame is set by rtk_oam_autoDyingGaspEnable_set.
 *      (2) rtk_oam_autoDyingGaspEnable_set must be invoked before rtk_oam_portDyingGaspPayload_set.
 *      (3) The pPayload and len should not include CRC. It is handled by lower layer driver internally.
 */
extern int32
rtk_oam_portDyingGaspPayload_set(uint32 unit, rtk_port_t port, uint8 *pPayload,
    uint32 len);

/* Function Name:
 *      rtk_oam_dyingGaspSend_set
 * Description:
 *      Start sending dying gasp frame to specified ports.
 * Input:
 *      unit   - unit id
 *      enable - trigger dying gasp with enabled ports.
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
 *      (1) This API will be used when CPU want to send dying gasp by itself.
 *      (2) The ports send out dying gasp frame is setting by
 *          rtk_oam_autoDyingGaspEnable_get.
 */
extern int32
rtk_oam_dyingGaspSend_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      rtk_oam_autoDyingGaspEnable_get
 * Description:
 *      Get enable status of sending dying gasp automatically on specified port
 *      when voltage is lower than expected.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of sending dying gasp automatically
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390
 * Note:
 *      Time of voltage is lower than expected is set by
 *      rtk_oam_dyingGaspWaitTime_set.
 */
extern int32
rtk_oam_autoDyingGaspEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_oam_autoDyingGaspEnable_set
 * Description:
 *      Set enable status of sending dying gasp automatically on specified port
 *      when voltage is lower than expected.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of sending dying gasp automatically
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
 *      8328, 8390
 * Note:
 *      Time of voltage is lower than expected is set by
 *      rtk_oam_dyingGaspWaitTime_set.
 */
extern int32
rtk_oam_autoDyingGaspEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_oam_dyingGaspTLV_get
 * Description:
 *      Get TLV content of dying gasp frame on specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pTLV - pointer to content of TLV
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
rtk_oam_dyingGaspTLV_get(uint32 unit, rtk_port_t port, rtk_oam_dyingGaspTLV_t *pTLV);

/* Function Name:
 *      rtk_oam_dyingGaspTLV_set
 * Description:
 *      Set TLV content of dying gasp frame on specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 *      pTLV - content of TLV
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
 *      8328
 * Note:
 *      None
 */
extern int32
rtk_oam_dyingGaspTLV_set(uint32 unit, rtk_port_t port, rtk_oam_dyingGaspTLV_t *pTLV);

/* Function Name:
 *      rtk_oam_dyingGaspWaitTime_get
 * Description:
 *      Get sustained time of low voltage detection before triggering dying gasp.
 * Input:
 *      unit - unit id
 * Output:
 *      time - sustained time
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390
 * Note:
 *      (1) Granularity of sustained time is 10 ns. Range of sustained time is 0~0xFFFF.
 *      (2) The status of sending dying gasp automatically is set by rtk_oam_autoDyingGaspEnable_set.
 */
extern int32
rtk_oam_dyingGaspWaitTime_get(uint32 unit, uint32 *pTime);

/* Function Name:
 *      rtk_oam_dyingGaspWaitTime_set
 * Description:
 *      Set sustained time of low voltage detection before triggering dying gasp.
 * Input:
 *      unit - unit id
 *      time - sustained time
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8328, 8390
 * Note:
 *      (1) Granularity of sustained time is 10 ns. Range of sustained time is 0~0xFFFF.
 *      (2) The status of sending dying gasp automatically is set by rtk_oam_autoDyingGaspEnable_set.
 */
extern int32
rtk_oam_dyingGaspWaitTime_set(uint32 unit, uint32 time);

/* Module Name    : OAM               */
/* Sub-module Name: CFM configuration */

/* Function Name:
 *      rtk_oam_cfmEntry_get
 * Description:
 *      Get configuration of specified CFM group.
 * Input:
 *      unit    - unit id
 *      cfm_idx - CFM index
 * Output:
 *      pCfm    - pointer to configuration of specified CFM group
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid cfm_idx is 0~1 in 8328.
 *      (2) CFM group = MA
 */
extern int32
rtk_oam_cfmEntry_get(uint32 unit, uint32 cfm_idx, rtk_oam_cfm_t *pCfm);

/* Function Name:
 *      rtk_oam_cfmEntry_set
 * Description:
 *      Set configuration of specified CFM group.
 * Input:
 *      unit    - unit id
 *      cfm_idx - CFM index
 *      pCfm    - configuration of specified CFM group
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid cfm_idx is 0~1 in 8328.
 *      (2) CFM group = MA
 */
extern int32
rtk_oam_cfmEntry_set(uint32 unit, uint32 cfm_idx, rtk_oam_cfm_t *pCfm);

/* Function Name:
 *      rtk_oam_cfmPortEntry_get
 * Description:
 *      Get CFM configuration of specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pPortCfg - pointer to CFM configuration of port
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
 *      Each port can be only belong to one CFM group.
 *      mepid of configuration is used when transmit CCM packets.
 */
extern int32
rtk_oam_cfmPortEntry_get(uint32 unit, rtk_port_t port, rtk_oam_cfmPort_t *pPortCfg);

/* Function Name:
 *      rtk_oam_cfmPortEntry_set
 * Description:
 *      Set CFM configuration of specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      pPortCfg - CFM configuration of port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      Each port can be only belong to one CFM group.
 *      mepid of configuration is used when transmit CCM packets.
 */
extern int32
rtk_oam_cfmPortEntry_set(uint32 unit, rtk_port_t port, rtk_oam_cfmPort_t *pPortCfg);

/* Function Name:
 *      rtk_oam_cfmMepEnable_get
 * Description:
 *      Get enable status of MEP function on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of MEP function
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
rtk_oam_cfmMepEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_oam_cfmMepEnable_set
 * Description:
 *      Set enable status of MEP function on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of MEP function
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
rtk_oam_cfmMepEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_oam_txCCMFrame_start
 * Description:
 *      Start sending CCM packet on specified ports.
 * Input:
 *      unit      - unit id
 *      pPortmask - portmask for sending CCM frame
 * Output:
 *      None.
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
rtk_oam_txCCMFrame_start(uint32 unit, rtk_portmask_t *pPortmask);

/* Function Name:
 *      rtk_oam_txCCMFrame_stop
 * Description:
 *      Stop sending CCM packet.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 * Applicable:
 *      8328
 * Note:
 *      CCM transmit and OAM's test frame transmit can't be enabled at same time.
 */
extern int32
rtk_oam_txCCMFrame_stop(uint32 unit);

/* Function Name:
 *      rtk_oam_cfmCCMFrame_get
 * Description:
 *      Get the content of CCM frame for specific CFM group.
 * Input:
 *      unit      - unit id
 *      cfm_idx   - CFM index
 * Output:
 *      pCcmFrame - pointer to content of CCM frame
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid cfm_idx is 0~1 in 8328.
 */
extern int32
rtk_oam_cfmCCMFrame_get(uint32 unit, uint32 cfm_idx, rtk_oam_ccmFrame_t *pCcmFrame);

/* Function Name:
 *      rtk_oam_cfmCCMFrame_set
 * Description:
 *      Fill the content of CCM frame for specific CFM group.
 * Input:
 *      unit      - unit id
 *      cfm_idx   - CFM index
 *      pCcmFrame - content of CCM frame
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid cfm_idx is 0~1 in 8328.
 */
extern int32
rtk_oam_cfmCCMFrame_set(uint32 unit, uint32 cfm_idx, rtk_oam_ccmFrame_t *pCcmFrame);

/* Function Name:
 *      rtk_oam_cfmCCMSnapOui_get
 * Description:
 *      Get OUI of SNAP packets when packet type of CCM packet is SNAP.
 * Input:
 *      unit     - unit id
 * Output:
 *      pSnapoui - pointer to OUI of SNAP packets
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
rtk_oam_cfmCCMSnapOui_get(uint32 unit, rtk_snapOui_t *pSnapoui);

/* Function Name:
 *      rtk_oam_cfmCCMSnapOui_set
 * Description:
 *      Set OUI of SNAP packets when packet type of CCM packet is SNAP.
 * Input:
 *      unit     - unit id
 *      pSnapoui - OUI of SNAP packets
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
extern int32
rtk_oam_cfmCCMSnapOui_set(uint32 unit, rtk_snapOui_t *pSnapoui);

/* Function Name:
 *      rtk_oam_cfmCCMEtype_get
 * Description:
 *      Get ethernet type of CCM packets for specified CFM group.
 * Input:
 *      unit       - unit id
 * Output:
 *      pEtherType - pointer to ethernet type of CCM packets
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
rtk_oam_cfmCCMEtype_get(uint32 unit, uint32 *pEtherType);

/* Function Name:
 *      rtk_oam_cfmCCMEtype_set
 * Description:
 *      Set ethernet type of CCM packets for specified CFM group.
 * Input:
 *      unit      - unit id
 *      etherType - ethernet type of CCM packets
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
 *      Default ethernet type of CCM packets is 0x8902
 */
extern int32
rtk_oam_cfmCCMEtype_set(uint32 unit, uint32 etherType);

/* Function Name:
 *      rtk_oam_cfmCCMOpcode_get
 * Description:
 *      Get opcode of CFM header.
 * Input:
 *      unit    - unit id
 * Output:
 *      pOpcode - pointer to opcode of CCM header
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
rtk_oam_cfmCCMOpcode_get(uint32 unit, uint32 *pOpcode);

/* Function Name:
 *      rtk_oam_cfmCCMOpcode_set
 * Description:
 *      Set opcode of CFM header.
 * Input:
 *      unit   - unit id
 *      opcode - opcode of CCM header
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
 *      Default opcode is 0x1
 */
extern int32
rtk_oam_cfmCCMOpcode_set(uint32 unit, uint32 opcode);

/* Function Name:
 *      rtk_oam_cfmCCMOpcode_get
 * Description:
 *      Get flag of CFM header for specified CFM group.
 * Input:
 *      unit     - unit id
 *      cfm_idx  - CFM index
 * Output:
 *      pCcmFlag - pointer to clag of CCM header
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid cfm_idx is 0~1 in 8328.
 */
extern int32
rtk_oam_cfmCCMFlag_get(uint32 unit, uint32 cfm_idx, uint32 *pCcmFlag);

/* Function Name:
 *      rtk_oam_cfmCCMFlag_set
 * Description:
 *      Set flag of CFM header for specified CFM group.
 * Input:
 *      unit    - unit id
 *      cfm_idx - CFM index
 *      ccmFlag - flag of CCM header
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
 *      (1) The valid cfm_idx is 0~1 in 8328.
 */
extern int32
rtk_oam_cfmCCMFlag_set(uint32 unit, uint32 cfm_idx, uint32 ccmFlag);

/* Function Name:
 *      rtk_oam_cfmCCMInterval_get
 * Description:
 *      Get transmit interval of CCM packets.
 * Input:
 *      unit      - unit id
 * Output:
 *      pInterval - pointer to transmit interval
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      CCM interval is as following
 *      - INTERVAL_3_3_MS
 *      - INTERVAL_10_MS
 *      - INTERVAL_100_MS
 *      - INTERVAL_1_S
 *      - INTERVAL_10_S
 *      - INTERVAL_1_MIN
 *      - INTERVAL_10_MIN
 */
extern int32
rtk_oam_cfmCCMInterval_get(uint32 unit, rtk_oam_ccmInterval_t *pInterval);

/* Function Name:
 *      rtk_oam_cfmCCMInterval_set
 * Description:
 *      Set transmit interval of CCM packets.
 * Input:
 *      unit     - unit id
 *      interval - transmit interval
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
 *      CCM interval is as following
 *      - INTERVAL_3_3_MS
 *      - INTERVAL_10_MS
 *      - INTERVAL_100_MS
 *      - INTERVAL_1_S
 *      - INTERVAL_10_S
 *      - INTERVAL_1_MIN
 *      - INTERVAL_10_MIN
 */
extern int32
rtk_oam_cfmCCMInterval_set(uint32 unit, rtk_oam_ccmInterval_t interval);

/* Function Name:
 *      rtk_oam_cfmIntfStatus_get
 * Description:
 *      Get interface status of CCM TLV on specified port.
 * Input:
 *      unit        - unit id
 *      port        - port id
 * Output:
 *      pIntfStatus - pointer to interface status
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
rtk_oam_cfmIntfStatus_get(uint32 unit, rtk_port_t port, uint32 *pIntfStatus);

/* Function Name:
 *      rtk_oam_cfmIntfStatus_set
 * Description:
 *      Set interface status of CCM TLV on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      intfStatus - interface status
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
rtk_oam_cfmIntfStatus_set(uint32 unit, rtk_port_t port, uint32 intfStatus);

/* Function Name:
 *      rtk_oam_cfmPortStatus_get
 * Description:
 *      Get port status of CCM TLV on specified port.
 * Input:
 *      unit        - unit id
 *      port        - port id
 * Output:
 *      pPortStatus - pointer to port status
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
rtk_oam_cfmPortStatus_get(uint32 unit, rtk_port_t port, uint32 *pPortStatus);

/* Function Name:
 *      rtk_oam_cfmPortStatus_set
 * Description:
 *      Set port status of CCM TLV on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      portStatus - port status
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
rtk_oam_cfmPortStatus_set(uint32 unit, rtk_port_t port, uint32 portStatus);

/* Function Name:
 *      rtk_oam_cfmRemoteMep_del
 * Description:
 *      Delete remote MEP.
 * Input:
 *      unit  - unit id
 *      mepid - remote MEP id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_ENTRY_NOTFOUND - remote mep not found
 * Applicable:
 *      8328
 * Note:
 *      None
 */
extern int32
rtk_oam_cfmRemoteMep_del(uint32 unit, uint32 mepid);

/* Function Name:
 *      rtk_oam_cfmRemoteMep_add
 * Description:
 *      Add remote MEP.
 * Input:
 *      unit  - unit id
 *      mepid - remote MEP id
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
rtk_oam_cfmRemoteMep_add(uint32 unit, uint32 mepid);

/* Function Name:
 *      rtk_oam_cfmRemoteMep_get
 * Description:
 *      Get remote MEP Info.
 * Input:
 *      unit  - unit id
 *      index - remote MEP id index
 * Output:
 *      pMepid - the pointer of the remote MEP id
 *      pValid - the pointer of the valid bit
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of index is 0~15 in 8328.
 */
extern int32
rtk_oam_cfmRemoteMep_get(uint32 unit, uint32 index, uint32* pMepid, uint32 *pValid);

/* Function Name:
 *      rtk_oam_cfmRemoteMep_set
 * Description:
 *      Set remote MEP Info.
 * Input:
 *      unit  - unit id
 *      index - remote MEP id index
 *      mepid -  remote MEP id
 *      valid -  the valid bit
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
 *      (1) The valid range of index is 0~15 in 8328.
 */
extern int32
rtk_oam_cfmRemoteMep_set(uint32 unit, uint32 index, uint32 mepid, uint32 valid);

/* Function Name:
 *      rtk_oam_cfmCCStatus_get
 * Description:
 *      Get continuity check status of remote MEP on specified port.
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      remoteMepid - remote mep id
 * Output:
 *      pRdi        - pointer to RDI of remote mep
 *      pCcStatus   - pointer to status of remote mep aging
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
extern int32
rtk_oam_cfmCCStatus_get(
    uint32              unit,
    rtk_port_t          port,
    uint32              remoteMepid,
    uint32              *pRdi,
    rtk_oam_ccStatus_t  *pCcStatus);

/* Function Name:
 *      rtk_oam_cfmCCStatus_reset
 * Description:
 *      Reset continuity check status of all remote mep on specified port.
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
rtk_oam_cfmCCStatus_reset(uint32 unit, rtk_port_t port);

/* Function Name:
 *      rtk_oam_cfmLoopbackReplyEnable_get
 * Description:
 *      Get enable status of auto reply for CFM loopback packets.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status
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
rtk_oam_cfmLoopbackReplyEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_oam_cfmLoopbackReplyEnable_set
 * Description:
 *      Set enable status of auto reply for CFM loopback packets.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status
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
rtk_oam_cfmLoopbackReplyEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_oam_cfmLoopbackReplyEnable_get
 * Description:
 *      Get control of CFM loopback action.
 * Input:
 *      unit  - unit id
 * Output:
 *      pCtrl - pointer to loopback control
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
rtk_oam_cfmLoopbackReplyCtrl_get(uint32 unit, rtk_oam_cfmLoopbackCtrl_t *pCtrl);

/* Function Name:
 *      rtk_oam_cfmLoopbackReplyEnable_set
 * Description:
 *      Set control of CFM loopback action.
 * Input:
 *      unit  - unit id
 *      pCtrl - loopback control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The rtk_oam_cfmLoopbackCtrl_t have two fileds: keep_innerTag and keep_outerTag.
 *      (2) The valid value of above two field is ENABLED or DISABLED.
 */
extern int32
rtk_oam_cfmLoopbackReplyCtrl_set(uint32 unit, rtk_oam_cfmLoopbackCtrl_t *pCtrl);

/* Function Name:
 *      rtk_oam_loopbackMacSwapEnable_get
 * Description:
 *      Get enable status of swap MAC address (source MAC & destination MAC)
 *      for OAM loopback feature.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of MAC swap function
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      (1) Swap action takes effect for following condition:
 *          - OAMPDU action is set to "ACTION_TRAP2CPU". It can be configured by 'rtk_trap_oamPDUAction_set'.
 *          - parser action is set to "Loopback". It can be configured by 'rtk_trap_portOamLoopbackParAction_set'.
 *      (2) Swap action only applies to non-OAMPDU packet.
 */
extern int32
rtk_oam_loopbackMacSwapEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_oam_loopbackMacSwapEnable_set
 * Description:
 *      Set enable status of swap MAC address (source MAC & destination MAC)
 *      for OAM loopback feature.
 * Input:
 *      unit   - unit id
 *      enable - enable status of MAC swap
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
 *      (1) Swap action takes effect for following condition:
 *          - OAMPDU action is set to "ACTION_TRAP2CPU". It can be configured by 'rtk_trap_oamPDUAction_set'.
 *          - parser action is set to "Loopback". It can be configured by 'rtk_trap_portOamLoopbackParAction_set'.
 *      (2) Swap action only applies to non-OAMPDU packet.
 */
extern int32
rtk_oam_loopbackMacSwapEnable_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      rtk_oam_portLoopbackMuxAction_get
 * Description:
 *      Get action of multiplexer on specific port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pAction - pointer to multiplexer action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      (1) Multiplexer action will be token effect
 *          when loopback status is enable (ACTION_TRAP2CPU) is setting by
 *          rtk_trap_oamPDUAction_set.
 *      (2) Multiplexer action is as following:
 *          - ACTION_FORWARD
 *          - ACTION_DROP
 */
extern int32
rtk_oam_portLoopbackMuxAction_get(uint32 unit, rtk_port_t port, rtk_action_t *pAction);

/* Function Name:
 *      rtk_oam_portLoopbackMuxAction_set
 * Description:
 *      Set action of multiplexer on specific port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      action - multiplexer action
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
 *      (1) Multiplexer action will be token effect
 *          when loopback status is enable (ACTION_TRAP2CPU) is setting by
 *          rtk_trap_oamPDUAction_set.
 *      (2) Multiplexer action is as following:
 *          - ACTION_FORWARD
 *          - ACTION_DROP
 */
extern int32
rtk_oam_portLoopbackMuxAction_set(uint32 unit, rtk_port_t port, rtk_action_t action);

/* Function Name:
 *      rtk_oam_cfmCcmPcp_get
 * Description:
 *      Get priority code point value for generate CCM frame.
 * Input:
 *      unit - unit id
 * Output:
 *      pPcp - pointer buffer of priority code point value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      Support configurate fields for generate content of CCM frame is as following:
 *      - enable                is setting by rtk_oam_cfmCcmInstTxStatus_set
 *      - TPID                  is setting by rtk_oam_cfmCcmTpid_set
 *      - tag status            is setting by rtk_oam_cfmCcmInstTagStatus_set
 *      - priority code point   is setting by rtk_oam_cfmCcmPcp_set
 *      - CFI                   is setting by rtk_oam_cfmCcmCfi_set
 *      - VID                   is setting by rtk_oam_cfmCcmInstVid_set
 *      - MEPID                 is setting by rtk_oam_cfmCcmMepid_set
 *      - interval field        is setting by rtk_oam_cfmCcmIntervalField_set
 *      - MD level              is setting by rtk_oam_cfmCcmMdl_set
 *      - MAID                  is setting by rtk_oam_cfmCcmInstMaid_set
 *      - tx interval           is setting by rtk_oam_cfmCcmInstInterval_set
 *      - port                  is setting by rtk_oam_cfmCcmTxInstPort_set
 */
extern int32
rtk_oam_cfmCcmPcp_get(uint32 unit, uint32 *pPcp);

/* Function Name:
 *      rtk_oam_cfmCcmPcp_set
 * Description:
 *      Set priority code point value for generate CCM frame.
 * Input:
 *      unit - unit id
 *      pcp  - priority code point value for generate CCM frame.
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
 *      Support configurate fields for generate content of CCM frame is as following:
 *      - enable                is setting by rtk_oam_cfmCcmInstTxStatus_set
 *      - TPID                  is setting by rtk_oam_cfmCcmTpid_set
 *      - tag status            is setting by rtk_oam_cfmCcmInstTagStatus_set
 *      - priority code point   is setting by rtk_oam_cfmCcmPcp_set
 *      - CFI                   is setting by rtk_oam_cfmCcmCfi_set
 *      - VID                   is setting by rtk_oam_cfmCcmInstVid_set
 *      - MEPID                 is setting by rtk_oam_cfmCcmMepid_set
 *      - interval field        is setting by rtk_oam_cfmCcmIntervalField_set
 *      - MD level              is setting by rtk_oam_cfmCcmMdl_set
 *      - MAID                  is setting by rtk_oam_cfmCcmInstMaid_set
 *      - tx interval           is setting by rtk_oam_cfmCcmInstInterval_set
 *      - port                  is setting by rtk_oam_cfmCcmTxInstPort_set
 */
extern int32
rtk_oam_cfmCcmPcp_set(uint32 unit, uint32 pcp);

/* Function Name:
 *      rtk_oam_cfmCcmCfi_get
 * Description:
 *      Get canonical format identifier value for generate CCM frame.
 * Input:
 *      unit - unit id
 * Output:
 *      pCfi - pointer buffer of canonical format identifier value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      Support configurate fields for generate content of CCM frame is as following:
 *      - enable                is setting by rtk_oam_cfmCcmInstTxStatus_set
 *      - TPID                  is setting by rtk_oam_cfmCcmTpid_set
 *      - tag status            is setting by rtk_oam_cfmCcmInstTagStatus_set
 *      - priority code point   is setting by rtk_oam_cfmCcmPcp_set
 *      - CFI                   is setting by rtk_oam_cfmCcmCfi_set
 *      - VID                   is setting by rtk_oam_cfmCcmInstVid_set
 *      - MEPID                 is setting by rtk_oam_cfmCcmMepid_set
 *      - interval field        is setting by rtk_oam_cfmCcmIntervalField_set
 *      - MD level              is setting by rtk_oam_cfmCcmMdl_set
 *      - MAID                  is setting by rtk_oam_cfmCcmInstMaid_set
 *      - tx interval           is setting by rtk_oam_cfmCcmInstInterval_set
 *      - port                  is setting by rtk_oam_cfmCcmTxInstPort_set
 */
extern int32
rtk_oam_cfmCcmCfi_get(uint32 unit, uint32 *pCfi);

/* Function Name:
 *      rtk_oam_cfmCcmCfi_set
 * Description:
 *      Set canonical format identifier value for generate CCM frame.
 * Input:
 *      unit - unit id
 *      cfi  - canonical format identifier value for generate CCM frame.
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
 *      Support configurate fields for generate content of CCM frame is as following:
 *      - enable                is setting by rtk_oam_cfmCcmInstTxStatus_set
 *      - TPID                  is setting by rtk_oam_cfmCcmTpid_set
 *      - tag status            is setting by rtk_oam_cfmCcmInstTagStatus_set
 *      - priority code point   is setting by rtk_oam_cfmCcmPcp_set
 *      - CFI                   is setting by rtk_oam_cfmCcmCfi_set
 *      - VID                   is setting by rtk_oam_cfmCcmInstVid_set
 *      - MEPID                 is setting by rtk_oam_cfmCcmMepid_set
 *      - interval field        is setting by rtk_oam_cfmCcmIntervalField_set
 *      - MD level              is setting by rtk_oam_cfmCcmMdl_set
 *      - MAID                  is setting by rtk_oam_cfmCcmInstMaid_set
 *      - tx interval           is setting by rtk_oam_cfmCcmInstInterval_set
 *      - port                  is setting by rtk_oam_cfmCcmTxInstPort_set
 */
extern int32
rtk_oam_cfmCcmCfi_set(uint32 unit, uint32 cfi);

/* Function Name:
 *      rtk_oam_cfmCcmTpid_get
 * Description:
 *      Get TPID value for generate CCM frame.
 * Input:
 *      unit  - unit id
 * Output:
 *      pTpid - pointer buffer of TPID value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      Support configurate fields for generate content of CCM frame is as following:
 *      - enable                is setting by rtk_oam_cfmCcmInstTxStatus_set
 *      - TPID                  is setting by rtk_oam_cfmCcmTpid_set
 *      - tag status            is setting by rtk_oam_cfmCcmInstTagStatus_set
 *      - priority code point   is setting by rtk_oam_cfmCcmPcp_set
 *      - CFI                   is setting by rtk_oam_cfmCcmCfi_set
 *      - VID                   is setting by rtk_oam_cfmCcmInstVid_set
 *      - MEPID                 is setting by rtk_oam_cfmCcmMepid_set
 *      - interval field        is setting by rtk_oam_cfmCcmIntervalField_set
 *      - MD level              is setting by rtk_oam_cfmCcmMdl_set
 *      - MAID                  is setting by rtk_oam_cfmCcmInstMaid_set
 *      - tx interval           is setting by rtk_oam_cfmCcmInstInterval_set
 *      - port                  is setting by rtk_oam_cfmCcmTxInstPort_set
 */
extern int32
rtk_oam_cfmCcmTpid_get(uint32 unit, uint32 *pTpid);

/* Function Name:
 *      rtk_oam_cfmCcmTpid_set
 * Description:
 *      Set TPID value for generate CCM frame.
 * Input:
 *      unit - unit id
 *      tpid - TPID value for generate CCM frame.
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
 *      Support configurate fields for generate content of CCM frame is as following:
 *      - enable                is setting by rtk_oam_cfmCcmInstTxStatus_set
 *      - TPID                  is setting by rtk_oam_cfmCcmTpid_set
 *      - tag status            is setting by rtk_oam_cfmCcmInstTagStatus_set
 *      - priority code point   is setting by rtk_oam_cfmCcmPcp_set
 *      - CFI                   is setting by rtk_oam_cfmCcmCfi_set
 *      - VID                   is setting by rtk_oam_cfmCcmInstVid_set
 *      - MEPID                 is setting by rtk_oam_cfmCcmMepid_set
 *      - interval field        is setting by rtk_oam_cfmCcmIntervalField_set
 *      - MD level              is setting by rtk_oam_cfmCcmMdl_set
 *      - MAID                  is setting by rtk_oam_cfmCcmInstMaid_set
 *      - tx interval           is setting by rtk_oam_cfmCcmInstInterval_set
 *      - port                  is setting by rtk_oam_cfmCcmTxInstPort_set
 */
extern int32
rtk_oam_cfmCcmTpid_set(uint32 unit, uint32 tpid);

/* Function Name:
 *      rtk_oam_cfmCcmInstLifetime_get
 * Description:
 *      Get lifetime of specified instance.
 * Input:
 *      unit      - unit id
 *      instance  - CCM instance
 * Output:
 *      pLifetime - pointer buffer to lifetime of the instance
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      (1) The unit of lifetime is mili-second. An internal alive timer keeps count down every ms and is
 *          reset to the value of lifetime when receiving the corresponding CCM frame.
 *          A CCM interrupt is triggered if the internal timer counts down to 0.
 *      (2) Support information for receiving CCM frame is as following:
 *          - lifetime  is setting by rtk_oam_cfmCcmInstLifetime_set
 *          - vid       is setting by rtk_oam_cfmCcmRxInstVid_set
 *          - port      is setting by rtk_oam_cfmCcmRxInstPort_set
 *          - keepalive is get by rtk_oam_cfmCcmInstAliveTime_get
 */
extern int32
rtk_oam_cfmCcmInstLifetime_get(uint32 unit, uint32 instance, uint32 *pLifetime);

/* Function Name:
 *      rtk_oam_cfmCcmInstLifetime_set
 * Description:
 *      Set lifetime to specified instance.
 * Input:
 *      unit     - unit id
 *      instance - CCM instance
 *      lifetime - lifetime of the instance
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
 *      (1) The unit of lifetime is mili-second. An internal alive timer keeps count down every ms and is
 *          reset to the value of lifetime when receiving the corresponding CCM frame.
 *          A CCM interrupt is triggered if the internal timer counts down to 0.
 *      (2) Support information for receiving CCM frame is as following:
 *          - lifetime  is setting by rtk_oam_cfmCcmInstLifetime_set
 *          - vid       is setting by rtk_oam_cfmCcmRxInstVid_set
 *          - port      is setting by rtk_oam_cfmCcmRxInstPort_set
 *          - keepalive is get by rtk_oam_cfmCcmInstAliveTime_get
 */
extern int32
rtk_oam_cfmCcmInstLifetime_set(uint32 unit, uint32 instance, uint32 lifetime);

/* Function Name:
 *      rtk_oam_cfmCcmMepid_get
 * Description:
 *      Get MEPID to be inserted to generated CCM frame.
 * Input:
 *      unit   - unit id
 * Output:
 *      pMepid - pointer buffer of MEPID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      Support configurate fields for generate content of CCM frame is as following:
 *      - enable                is setting by rtk_oam_cfmCcmInstTxStatus_set
 *      - TPID                  is setting by rtk_oam_cfmCcmTpid_set
 *      - tag status            is setting by rtk_oam_cfmCcmInstTagStatus_set
 *      - priority code point   is setting by rtk_oam_cfmCcmPcp_set
 *      - CFI                   is setting by rtk_oam_cfmCcmCfi_set
 *      - VID                   is setting by rtk_oam_cfmCcmInstVid_set
 *      - MEPID                 is setting by rtk_oam_cfmCcmMepid_set
 *      - interval field        is setting by rtk_oam_cfmCcmIntervalField_set
 *      - MD level              is setting by rtk_oam_cfmCcmMdl_set
 *      - MAID                  is setting by rtk_oam_cfmCcmInstMaid_set
 *      - tx interval           is setting by rtk_oam_cfmCcmInstInterval_set
 *      - port                  is setting by rtk_oam_cfmCcmTxInstPort_set
 */
extern int32
rtk_oam_cfmCcmMepid_get(uint32 unit, uint32 *pMepid);

/* Function Name:
 *      rtk_oam_cfmCcmMepid_set
 * Description:
 *      Set MEPID to be inserted to generated CCM frame.
 * Input:
 *      unit  - unit id
 *      mepid - MEP id.
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
 *      Support configurate fields for generate content of CCM frame is as following:
 *      - enable                is setting by rtk_oam_cfmCcmInstTxStatus_set
 *      - TPID                  is setting by rtk_oam_cfmCcmTpid_set
 *      - tag status            is setting by rtk_oam_cfmCcmInstTagStatus_set
 *      - priority code point   is setting by rtk_oam_cfmCcmPcp_set
 *      - CFI                   is setting by rtk_oam_cfmCcmCfi_set
 *      - VID                   is setting by rtk_oam_cfmCcmInstVid_set
 *      - MEPID                 is setting by rtk_oam_cfmCcmMepid_set
 *      - interval field        is setting by rtk_oam_cfmCcmIntervalField_set
 *      - MD level              is setting by rtk_oam_cfmCcmMdl_set
 *      - MAID                  is setting by rtk_oam_cfmCcmInstMaid_set
 *      - tx interval           is setting by rtk_oam_cfmCcmInstInterval_set
 *      - port                  is setting by rtk_oam_cfmCcmTxInstPort_set
 */
extern int32
rtk_oam_cfmCcmMepid_set(uint32 unit, uint32 mepid);

/* Function Name:
 *      rtk_oam_cfmCcmIntervalField_get
 * Description:
 *      Get value to be inserted to interval field in flag for generated CCM frame.
 * Input:
 *      unit      - unit id
 * Output:
 *      pInterval - pointer buffer of interval field in flag.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      Support configurate fields for generate content of CCM frame is as following:
 *      - enable                is setting by rtk_oam_cfmCcmInstTxStatus_set
 *      - TPID                  is setting by rtk_oam_cfmCcmTpid_set
 *      - tag status            is setting by rtk_oam_cfmCcmInstTagStatus_set
 *      - priority code point   is setting by rtk_oam_cfmCcmPcp_set
 *      - CFI                   is setting by rtk_oam_cfmCcmCfi_set
 *      - VID                   is setting by rtk_oam_cfmCcmInstVid_set
 *      - MEPID                 is setting by rtk_oam_cfmCcmMepid_set
 *      - interval field        is setting by rtk_oam_cfmCcmIntervalField_set
 *      - MD level              is setting by rtk_oam_cfmCcmMdl_set
 *      - MAID                  is setting by rtk_oam_cfmCcmInstMaid_set
 *      - tx interval           is setting by rtk_oam_cfmCcmInstInterval_set
 *      - port                  is setting by rtk_oam_cfmCcmTxInstPort_set
 */
extern int32
rtk_oam_cfmCcmIntervalField_get(uint32 unit, uint32 *pInterval);

/* Function Name:
 *      rtk_oam_cfmCcmIntervalField_set
 * Description:
 *      Set value to be inserted to interval field in flag for generated CCM frame.
 * Input:
 *      unit     - unit id
 *      interval - interval field in flag.
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
 *      Support configurate fields for generate content of CCM frame is as following:
 *      - enable                is setting by rtk_oam_cfmCcmInstTxStatus_set
 *      - TPID                  is setting by rtk_oam_cfmCcmTpid_set
 *      - tag status            is setting by rtk_oam_cfmCcmInstTagStatus_set
 *      - priority code point   is setting by rtk_oam_cfmCcmPcp_set
 *      - CFI                   is setting by rtk_oam_cfmCcmCfi_set
 *      - VID                   is setting by rtk_oam_cfmCcmInstVid_set
 *      - MEPID                 is setting by rtk_oam_cfmCcmMepid_set
 *      - interval field        is setting by rtk_oam_cfmCcmIntervalField_set
 *      - MD level              is setting by rtk_oam_cfmCcmMdl_set
 *      - MAID                  is setting by rtk_oam_cfmCcmInstMaid_set
 *      - tx interval           is setting by rtk_oam_cfmCcmInstInterval_set
 *      - port                  is setting by rtk_oam_cfmCcmTxInstPort_set
 */
extern int32
rtk_oam_cfmCcmIntervalField_set(uint32 unit, uint32 interval);

/* Function Name:
 *      rtk_oam_cfmCcmMdl_get
 * Description:
 *      Get MD level to be inserted to generated CCM frame.
 * Input:
 *      unit - unit id
 * Output:
 *      pMdl  - pointer buffer of MD level
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      Support configurate fields for generate content of CCM frame is as following:
 *      - enable                is setting by rtk_oam_cfmCcmInstTxStatus_set
 *      - TPID                  is setting by rtk_oam_cfmCcmTpid_set
 *      - tag status            is setting by rtk_oam_cfmCcmInstTagStatus_set
 *      - priority code point   is setting by rtk_oam_cfmCcmPcp_set
 *      - CFI                   is setting by rtk_oam_cfmCcmCfi_set
 *      - VID                   is setting by rtk_oam_cfmCcmInstVid_set
 *      - MEPID                 is setting by rtk_oam_cfmCcmMepid_set
 *      - interval field        is setting by rtk_oam_cfmCcmIntervalField_set
 *      - MD level              is setting by rtk_oam_cfmCcmMdl_set
 *      - MAID                  is setting by rtk_oam_cfmCcmInstMaid_set
 *      - tx interval           is setting by rtk_oam_cfmCcmInstInterval_set
 *      - port                  is setting by rtk_oam_cfmCcmTxInstPort_set
 */
extern int32
rtk_oam_cfmCcmMdl_get(uint32 unit, uint32 *pMdl);

/* Function Name:
 *      rtk_oam_cfmCcmMdl_set
 * Description:
 *      Set MD level to be inserted to generated CCM frame.
 * Input:
 *      unit - unit id
 *      mdl  - MD level insert to CCM frame
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
 *      Support configurate fields for generate content of CCM frame is as following:
 *      - enable                is setting by rtk_oam_cfmCcmInstTxStatus_set
 *      - TPID                  is setting by rtk_oam_cfmCcmTpid_set
 *      - tag status            is setting by rtk_oam_cfmCcmInstTagStatus_set
 *      - priority code point   is setting by rtk_oam_cfmCcmPcp_set
 *      - CFI                   is setting by rtk_oam_cfmCcmCfi_set
 *      - VID                   is setting by rtk_oam_cfmCcmInstVid_set
 *      - MEPID                 is setting by rtk_oam_cfmCcmMepid_set
 *      - interval field        is setting by rtk_oam_cfmCcmIntervalField_set
 *      - MD level              is setting by rtk_oam_cfmCcmMdl_set
 *      - MAID                  is setting by rtk_oam_cfmCcmInstMaid_set
 *      - tx interval           is setting by rtk_oam_cfmCcmInstInterval_set
 *      - port                  is setting by rtk_oam_cfmCcmTxInstPort_set
 */
extern int32
rtk_oam_cfmCcmMdl_set(uint32 unit, uint32 mdl);

/* Function Name:
 *      rtk_oam_cfmCcmInstTagStatus_get
 * Description:
 *      Get VLAN tag status of the generated CCM frame.
 * Input:
 *      unit     - unit id
 *      instance - tx control entry instance
 * Output:
 *      pEnable  - pointer buffer of VLAN tag status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      Indicate whether to insert VLAN tag to the generated CCM frame.
 *      Support configurate fields for generate content of CCM frame is as following:
 *      - enable                is setting by rtk_oam_cfmCcmInstTxStatus_set
 *      - TPID                  is setting by rtk_oam_cfmCcmTpid_set
 *      - tag status            is setting by rtk_oam_cfmCcmInstTagStatus_set
 *      - priority code point   is setting by rtk_oam_cfmCcmPcp_set
 *      - CFI                   is setting by rtk_oam_cfmCcmCfi_set
 *      - VID                   is setting by rtk_oam_cfmCcmInstVid_set
 *      - MEPID                 is setting by rtk_oam_cfmCcmMepid_set
 *      - interval field        is setting by rtk_oam_cfmCcmIntervalField_set
 *      - MD level              is setting by rtk_oam_cfmCcmMdl_set
 *      - MAID                  is setting by rtk_oam_cfmCcmInstMaid_set
 *      - tx interval           is setting by rtk_oam_cfmCcmInstInterval_set
 *      - port                  is setting by rtk_oam_cfmCcmTxInstPort_set
 */
extern int32
rtk_oam_cfmCcmInstTagStatus_get(uint32 unit, uint32 instance, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_oam_cfmCcmInstTagStatus_set
 * Description:
 *      Set VLAN tag status of the generated CCM frame.
 * Input:
 *      unit     - unit id
 *      instance - tx control entry instance
 *      enable   - tag status
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
 *      Indicate whether to insert VLAN tag to the generated CCM frame.
 *      Support configurate fields for generate content of CCM frame is as following:
 *      - enable                is setting by rtk_oam_cfmCcmInstTxStatus_set
 *      - TPID                  is setting by rtk_oam_cfmCcmTpid_set
 *      - tag status            is setting by rtk_oam_cfmCcmInstTagStatus_set
 *      - priority code point   is setting by rtk_oam_cfmCcmPcp_set
 *      - CFI                   is setting by rtk_oam_cfmCcmCfi_set
 *      - VID                   is setting by rtk_oam_cfmCcmInstVid_set
 *      - MEPID                 is setting by rtk_oam_cfmCcmMepid_set
 *      - interval field        is setting by rtk_oam_cfmCcmIntervalField_set
 *      - MD level              is setting by rtk_oam_cfmCcmMdl_set
 *      - MAID                  is setting by rtk_oam_cfmCcmInstMaid_set
 *      - tx interval           is setting by rtk_oam_cfmCcmInstInterval_set
 *      - port                  is setting by rtk_oam_cfmCcmTxInstPort_set
 */
extern int32
rtk_oam_cfmCcmInstTagStatus_set(uint32 unit, uint32 instance, rtk_enable_t enable);

/* Function Name:
 *      rtk_oam_cfmCcmInstVid_get
 * Description:
 *      Get vlan id for instance member to be inserted to generated CCM frame.
 * Input:
 *      unit     - unit id
 *      instance - tx control entry instance
 * Output:
 *      pVid     - pointer buffer of vlan id
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      Support configurate fields for generate content of CCM frame is as following:
 *      - enable                is setting by rtk_oam_cfmCcmInstTxStatus_set
 *      - TPID                  is setting by rtk_oam_cfmCcmTpid_set
 *      - tag status            is setting by rtk_oam_cfmCcmInstTagStatus_set
 *      - priority code point   is setting by rtk_oam_cfmCcmPcp_set
 *      - CFI                   is setting by rtk_oam_cfmCcmCfi_set
 *      - VID                   is setting by rtk_oam_cfmCcmInstVid_set
 *      - MEPID                 is setting by rtk_oam_cfmCcmMepid_set
 *      - interval field        is setting by rtk_oam_cfmCcmIntervalField_set
 *      - MD level              is setting by rtk_oam_cfmCcmMdl_set
 *      - MAID                  is setting by rtk_oam_cfmCcmInstMaid_set
 *      - tx interval           is setting by rtk_oam_cfmCcmInstInterval_set
 *      - port                  is setting by rtk_oam_cfmCcmTxInstPort_set
 */
extern int32
rtk_oam_cfmCcmInstVid_get(uint32 unit, uint32 instance, rtk_vlan_t *pVid);

/* Function Name:
 *      rtk_oam_cfmCcmInstVid_set
 * Description:
 *      Set vlan id for instance member to be inserted to generated CCM frame.
 * Input:
 *      unit     - unit id
 *      instance - tx control entry instance
 *      vid      - vlan id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 *      RT_ERR_VLAN_VID - invalid vid
 * Applicable:
 *      8390
 * Note:
 *      Support configurate fields for generate content of CCM frame is as following:
 *      - enable                is setting by rtk_oam_cfmCcmInstTxStatus_set
 *      - TPID                  is setting by rtk_oam_cfmCcmTpid_set
 *      - tag status            is setting by rtk_oam_cfmCcmInstTagStatus_set
 *      - priority code point   is setting by rtk_oam_cfmCcmPcp_set
 *      - CFI                   is setting by rtk_oam_cfmCcmCfi_set
 *      - VID                   is setting by rtk_oam_cfmCcmInstVid_set
 *      - MEPID                 is setting by rtk_oam_cfmCcmMepid_set
 *      - interval field        is setting by rtk_oam_cfmCcmIntervalField_set
 *      - MD level              is setting by rtk_oam_cfmCcmMdl_set
 *      - MAID                  is setting by rtk_oam_cfmCcmInstMaid_set
 *      - tx interval           is setting by rtk_oam_cfmCcmInstInterval_set
 *      - port                  is setting by rtk_oam_cfmCcmTxInstPort_set
 */
extern int32
rtk_oam_cfmCcmInstVid_set(uint32 unit, uint32 instance, rtk_vlan_t vid);

/* Function Name:
 *      rtk_oam_cfmCcmInstMaid_get
 * Description:
 *      Get MAID for instance member to be inserted to generated CCM frame.
 * Input:
 *      unit     - unit id
 *      instance - tx control entry instance
 * Output:
 *      pMaid    - pointer buffer of MAID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      Support configurate fields for generate content of CCM frame is as following:
 *      - enable                is setting by rtk_oam_cfmCcmInstTxStatus_set
 *      - TPID                  is setting by rtk_oam_cfmCcmTpid_set
 *      - tag status            is setting by rtk_oam_cfmCcmInstTagStatus_set
 *      - priority code point   is setting by rtk_oam_cfmCcmPcp_set
 *      - CFI                   is setting by rtk_oam_cfmCcmCfi_set
 *      - VID                   is setting by rtk_oam_cfmCcmInstVid_set
 *      - MEPID                 is setting by rtk_oam_cfmCcmMepid_set
 *      - interval field        is setting by rtk_oam_cfmCcmIntervalField_set
 *      - MD level              is setting by rtk_oam_cfmCcmMdl_set
 *      - MAID                  is setting by rtk_oam_cfmCcmInstMaid_set
 *      - tx interval           is setting by rtk_oam_cfmCcmInstInterval_set
 *      - port                  is setting by rtk_oam_cfmCcmTxInstPort_set
 */
extern int32
rtk_oam_cfmCcmInstMaid_get(uint32 unit, uint32 instance, uint32 *pMaid);

/* Function Name:
 *      rtk_oam_cfmCcmInstMaid_set
 * Description:
 *      Set MAID for instance member to be inserted to generated CCM frame.
 * Input:
 *      unit     - unit id
 *      instance - tx control entry instance
 *      maid     - MA id
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
 *      Support configurate fields for generate content of CCM frame is as following:
 *      - enable                is setting by rtk_oam_cfmCcmInstTxStatus_set
 *      - TPID                  is setting by rtk_oam_cfmCcmTpid_set
 *      - tag status            is setting by rtk_oam_cfmCcmInstTagStatus_set
 *      - priority code point   is setting by rtk_oam_cfmCcmPcp_set
 *      - CFI                   is setting by rtk_oam_cfmCcmCfi_set
 *      - VID                   is setting by rtk_oam_cfmCcmInstVid_set
 *      - MEPID                 is setting by rtk_oam_cfmCcmMepid_set
 *      - interval field        is setting by rtk_oam_cfmCcmIntervalField_set
 *      - MD level              is setting by rtk_oam_cfmCcmMdl_set
 *      - MAID                  is setting by rtk_oam_cfmCcmInstMaid_set
 *      - tx interval           is setting by rtk_oam_cfmCcmInstInterval_set
 *      - port                  is setting by rtk_oam_cfmCcmTxInstPort_set
 */
extern int32
rtk_oam_cfmCcmInstMaid_set(uint32 unit, uint32 instance, uint32 maid);

/* Function Name:
 *      rtk_oam_cfmCcmInstTxStatus_get
 * Description:
 *      Get enable status of transmitting CCM frame on specified instance.
 * Input:
 *      unit     - unit id
 *      instance - tx control ent instance
 * Output:
 *      pEnable  - pointer buffer of enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      Support configurate fields for generate content of CCM frame is as following:
 *      - enable                is setting by rtk_oam_cfmCcmInstTxStatus_set
 *      - TPID                  is setting by rtk_oam_cfmCcmTpid_set
 *      - tag status            is setting by rtk_oam_cfmCcmInstTagStatus_set
 *      - priority code point   is setting by rtk_oam_cfmCcmPcp_set
 *      - CFI                   is setting by rtk_oam_cfmCcmCfi_set
 *      - VID                   is setting by rtk_oam_cfmCcmInstVid_set
 *      - MEPID                 is setting by rtk_oam_cfmCcmMepid_set
 *      - interval field        is setting by rtk_oam_cfmCcmIntervalField_set
 *      - MD level              is setting by rtk_oam_cfmCcmMdl_set
 *      - MAID                  is setting by rtk_oam_cfmCcmInstMaid_set
 *      - tx interval           is setting by rtk_oam_cfmCcmInstInterval_set
 *      - port                  is setting by rtk_oam_cfmCcmTxInstPort_set
 */
extern int32
rtk_oam_cfmCcmInstTxStatus_get(uint32 unit, uint32 instance, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_oam_cfmCcmInstTxStatus_set
 * Description:
 *      Set enable status of transmitting CCM frame on specified instance.
 * Input:
 *      unit     - unit id
 *      instance - tx control entry instance
 *      enable   - tx status
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
 *      Support configurate fields for generate content of CCM frame is as following:
 *      - enable                is setting by rtk_oam_cfmCcmInstTxStatus_set
 *      - TPID                  is setting by rtk_oam_cfmCcmTpid_set
 *      - tag status            is setting by rtk_oam_cfmCcmInstTagStatus_set
 *      - priority code point   is setting by rtk_oam_cfmCcmPcp_set
 *      - CFI                   is setting by rtk_oam_cfmCcmCfi_set
 *      - VID                   is setting by rtk_oam_cfmCcmInstVid_set
 *      - MEPID                 is setting by rtk_oam_cfmCcmMepid_set
 *      - interval field        is setting by rtk_oam_cfmCcmIntervalField_set
 *      - MD level              is setting by rtk_oam_cfmCcmMdl_set
 *      - MAID                  is setting by rtk_oam_cfmCcmInstMaid_set
 *      - tx interval           is setting by rtk_oam_cfmCcmInstInterval_set
 *      - port                  is setting by rtk_oam_cfmCcmTxInstPort_set
 */
extern int32
rtk_oam_cfmCcmInstTxStatus_set(uint32 unit, uint32 instance, rtk_enable_t enable);

/* Function Name:
 *      rtk_oam_cfmCcmInstInterval_get
 * Description:
 *      Get CCM frame transmit interval.
 * Input:
 *      unit     - unit id
 *      instance - tx control entry instance
 * Output:
 *      pInterval - pointer buffer of CCM frame transmit interval
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      Support configurate fields for generate content of CCM frame is as following:
 *      - enable                is setting by rtk_oam_cfmCcmInstTxStatus_set
 *      - TPID                  is setting by rtk_oam_cfmCcmTpid_set
 *      - tag status            is setting by rtk_oam_cfmCcmInstTagStatus_set
 *      - priority code point   is setting by rtk_oam_cfmCcmPcp_set
 *      - CFI                   is setting by rtk_oam_cfmCcmCfi_set
 *      - VID                   is setting by rtk_oam_cfmCcmInstVid_set
 *      - MEPID                 is setting by rtk_oam_cfmCcmMepid_set
 *      - interval field        is setting by rtk_oam_cfmCcmIntervalField_set
 *      - MD level              is setting by rtk_oam_cfmCcmMdl_set
 *      - MAID                  is setting by rtk_oam_cfmCcmInstMaid_set
 *      - tx interval           is setting by rtk_oam_cfmCcmInstInterval_set
 *      - port                  is setting by rtk_oam_cfmCcmTxInstPort_set
 */
extern int32
rtk_oam_cfmCcmInstInterval_get(uint32 unit, uint32 instance, uint32 *pInterval);

/* Function Name:
 *      rtk_oam_cfmCcmInstInterval_set
 * Description:
 *      Set CCM frame transmit interval.
 * Input:
 *      unit     - unit id
 *      instance - tx control entry instance
 *      interval - transmit interval
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
 *      Support configurate fields for generate content of CCM frame is as following:
 *      - enable                is setting by rtk_oam_cfmCcmInstTxStatus_set
 *      - TPID                  is setting by rtk_oam_cfmCcmTpid_set
 *      - tag status            is setting by rtk_oam_cfmCcmInstTagStatus_set
 *      - priority code point   is setting by rtk_oam_cfmCcmPcp_set
 *      - CFI                   is setting by rtk_oam_cfmCcmCfi_set
 *      - VID                   is setting by rtk_oam_cfmCcmInstVid_set
 *      - MEPID                 is setting by rtk_oam_cfmCcmMepid_set
 *      - interval field        is setting by rtk_oam_cfmCcmIntervalField_set
 *      - MD level              is setting by rtk_oam_cfmCcmMdl_set
 *      - MAID                  is setting by rtk_oam_cfmCcmInstMaid_set
 *      - tx interval           is setting by rtk_oam_cfmCcmInstInterval_set
 *      - port                  is setting by rtk_oam_cfmCcmTxInstPort_set
 */
extern int32
rtk_oam_cfmCcmInstInterval_set(uint32 unit, uint32 instance, uint32 interval);

/* Function Name:
 *      rtk_oam_cfmCcmTxInstPort_get
 * Description:
 *      Get tx instance member.
 * Input:
 *      unit     - unit id
 *      instance - tx control entry instance
 *      index    - instance member index
 * Output:
 *      pPort    - pointer buffer of tx instance member
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      Support configurate fields for generate content of CCM frame is as following:
 *      - enable                is setting by rtk_oam_cfmCcmInstTxStatus_set
 *      - TPID                  is setting by rtk_oam_cfmCcmTpid_set
 *      - tag status            is setting by rtk_oam_cfmCcmInstTagStatus_set
 *      - priority code point   is setting by rtk_oam_cfmCcmPcp_set
 *      - CFI                   is setting by rtk_oam_cfmCcmCfi_set
 *      - VID                   is setting by rtk_oam_cfmCcmInstVid_set
 *      - MEPID                 is setting by rtk_oam_cfmCcmMepid_set
 *      - interval field        is setting by rtk_oam_cfmCcmIntervalField_set
 *      - MD level              is setting by rtk_oam_cfmCcmMdl_set
 *      - MAID                  is setting by rtk_oam_cfmCcmInstMaid_set
 *      - tx interval           is setting by rtk_oam_cfmCcmInstInterval_set
 *      - port                  is setting by rtk_oam_cfmCcmTxInstPort_set
 */
extern int32
rtk_oam_cfmCcmTxInstPort_get(uint32 unit, uint32 instance, uint32 index, rtk_port_t *pPort);

/* Function Name:
 *      rtk_oam_cfmCcmTxInstPort_set
 * Description:
 *      Set tx instance member.
 * Input:
 *      unit     - unit id
 *      instance - tx control entry instance
 *      index    - instance member index
 *      port     - instance member port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 *      RT_ERR_PORT_ID  - invalid port id
 * Applicable:
 *      8390
 * Note:
 *      Support configurate fields for generate content of CCM frame is as following:
 *      - enable                is setting by rtk_oam_cfmCcmInstTxStatus_set
 *      - TPID                  is setting by rtk_oam_cfmCcmTpid_set
 *      - tag status            is setting by rtk_oam_cfmCcmInstTagStatus_set
 *      - priority code point   is setting by rtk_oam_cfmCcmPcp_set
 *      - CFI                   is setting by rtk_oam_cfmCcmCfi_set
 *      - VID                   is setting by rtk_oam_cfmCcmInstVid_set
 *      - MEPID                 is setting by rtk_oam_cfmCcmMepid_set
 *      - interval field        is setting by rtk_oam_cfmCcmIntervalField_set
 *      - MD level              is setting by rtk_oam_cfmCcmMdl_set
 *      - MAID                  is setting by rtk_oam_cfmCcmInstMaid_set
 *      - tx interval           is setting by rtk_oam_cfmCcmInstInterval_set
 *      - port                  is setting by rtk_oam_cfmCcmTxInstPort_set
 */
extern int32
rtk_oam_cfmCcmTxInstPort_set(uint32 unit, uint32 instance, uint32 index, rtk_port_t port);

/* Function Name:
 *      rtk_oam_cfmCcmRxInstVid_get
 * Description:
 *      Get binding VLAN of rx instance.
 * Input:
 *      unit     - unit id
 *      instance - rx control entry instance
 * Output:
 *      pVid     - pointer buffer of binding VLAN
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      Set vid=0 to disable binding VLAN to instance.
 *      Support information for receiving CCM frame is as following:
 *      - lifetime  is setting by rtk_oam_cfmCcmInstLifetime_set
 *      - vid       is setting by rtk_oam_cfmCcmRxInstVid_set
 *      - port      is setting by rtk_oam_cfmCcmRxInstPort_set
 *      - keepalive is get by rtk_oam_cfmCcmInstAliveTime_get
 */
extern int32
rtk_oam_cfmCcmRxInstVid_get(uint32 unit, uint32 instance, rtk_vlan_t *pVid);

/* Function Name:
 *      rtk_oam_cfmCcmRxInstVid_set
 * Description:
 *      Set binding VLAN of rx instance.
 * Input:
 *      unit     - unit id
 *      instance - rx control entry instance
 *      vid      - accept vlan id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 *      RT_ERR_VLAN_VID - invalid vid
 * Applicable:
 *      8390
 * Note:
 *      Set vid=0 to disable binding VLAN to instance.
 *      Support information for receiving CCM frame is as following:
 *      - lifetime  is setting by rtk_oam_cfmCcmInstLifetime_set
 *      - vid       is setting by rtk_oam_cfmCcmRxInstVid_set
 *      - port      is setting by rtk_oam_cfmCcmRxInstPort_set
 *      - keepalive is get by rtk_oam_cfmCcmInstAliveTime_get
 */
extern int32
rtk_oam_cfmCcmRxInstVid_set(uint32 unit, uint32 instance, rtk_vlan_t vid);

/* Function Name:
 *      rtk_oam_cfmCcmRxInstPort_get
 * Description:
 *      Get rx instance member.
 * Input:
 *      unit     - unit id
 *      instance - rx control entry instance
 *      index    - instance member index
 * Output:
 *      pPort    - pointer buffer of rx instance member
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      Support information for receiving CCM frame is as following:
 *      - lifetime  is setting by rtk_oam_cfmCcmInstLifetime_set
 *      - vid       is setting by rtk_oam_cfmCcmRxInstVid_set
 *      - port      is setting by rtk_oam_cfmCcmRxInstPort_set
 *      - keepalive is get by rtk_oam_cfmCcmInstAliveTime_get
 */
extern int32
rtk_oam_cfmCcmRxInstPort_get(uint32 unit, uint32 instance, uint32 index, rtk_port_t *pPort);

/* Function Name:
 *      rtk_oam_cfmCcmRxInstPort_set
 * Description:
 *      Set rx instance member.
 * Input:
 *      unit     - unit id
 *      instance - rx control entry instance
 *      index    - instance member index
 *      port     - instance member port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 *      RT_ERR_PORT_ID  - invalid port id
 * Applicable:
 *      8390
 * Note:
 *      Support information for receiving CCM frame is as following:
 *      - lifetime  is setting by rtk_oam_cfmCcmInstLifetime_set
 *      - vid       is setting by rtk_oam_cfmCcmRxInstVid_set
 *      - port      is setting by rtk_oam_cfmCcmRxInstPort_set
 *      - keepalive is get by rtk_oam_cfmCcmInstAliveTime_get
 */
extern int32
rtk_oam_cfmCcmRxInstPort_set(uint32 unit, uint32 instance, uint32 index, rtk_port_t port);

/* Function Name:
 *      rtk_oam_cfmCcmInstAliveTime_get
 * Description:
 *      Get rx instance member alive time.
 * Input:
 *      unit     - unit id
 *      instance - rx control entry instance
 *      index    - instance member index
 * Output:
 *      pTime    - pointer buffer of rx instance member alive time
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      (1) The unit of time is mili-second.
 *      (2) Each instance member port has an internal alive timer. It keeps count down every ms
 *          and is reset to the value of lifetime when receiving the corresponding CCM frame.
 *          A CCM interrupt is triggered if the internal timer counts down to 0.
 *      (3) Support information for receiving CCM frame is as following:
 *          - lifetime  is setting by rtk_oam_cfmCcmInstLifetime_set
 *          - vid       is setting by rtk_oam_cfmCcmRxInstVid_set
 *          - port      is setting by rtk_oam_cfmCcmRxInstPort_set
 *          - keepalive is get by rtk_oam_cfmCcmInstAliveTime_get
 */
extern int32
rtk_oam_cfmCcmInstAliveTime_get(uint32 unit, uint32 instance, uint32 index, uint32 *pTime);

/* Function Name:
 *      rtk_oam_cfmPortEthDmEnable_get
 * Description:
 *      Get enable status of CFM ETH-DM feature on the specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      None
 */
extern int32
rtk_oam_cfmPortEthDmEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_oam_cfmPortEthDmEnable_set
 * Description:
 *      Set enable status of CFM ETH-DM feature on the specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status
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
 *      None
 */
extern int32
rtk_oam_cfmPortEthDmEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_oam_dyingGaspPktCnt_get
 * Description:
 *      Get dying gasp send packet count.
 * Input:
 *      unit - unit id
 * Output:
 *      pCnt - packet count configuration
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      (1) Multiple dying gasp packets can be sent continously for robustness.
 *      (2) The valid packet count is 0 ~ 7.
 */
extern int32
rtk_oam_dyingGaspPktCnt_get(uint32 unit, uint32 *pCnt);

/* Function Name:
 *      rtk_oam_dyingGaspPktCnt_set
 * Description:
 *      Set dying gasp send packet count.
 * Input:
 *      unit - unit id
 *      cnt  - trigger dying gasp with enabled ports.
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
 *      (1) Multiple dying gasp packets can be sent continously for robustness.
 *      (2) The valid packet count is 0 ~ 7.
 */
extern int32
rtk_oam_dyingGaspPktCnt_set(uint32 unit, uint32 cnt);

/* Function Name:
 *      rtk_oam_linkFaultMonEnable_set
 * Description:
 *      Set enable status of link fault monitor
 * Input:
 *      enable - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - initialize success
 *      RT_ERR_FAILED   - initialize fail
 *      RT_ERR_NOT_INIT - The module is not initial
 * Applicable:
 *      8390
 * Note:
 *      When enable link fault monitor, all CCM interrupt will be callback to upper layer.
 */
extern int32 rtk_oam_linkFaultMonEnable_set(rtk_enable_t enable);

/* Function Name:
 *      rtk_oam_linkFaultMon_register
 * Description:
 *      Register callback function for link fault detect notification
 * Input:
 *      callback - callback function for link fault detect
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      None
 */
extern int32 rtk_oam_linkFaultMon_register(rtk_oam_linkFaultMon_callback_t callback);

/* Function Name:
 *      rtk_oam_linkFaultMon_unregister
 * Description:
 *      Unregister callback function for link fault detect notification
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Applicable:
 *      8390
 * Note:
 *      None
 */
extern int32 rtk_oam_linkFaultMon_unregister(void);

#endif /* __RTK_OAM_H__ */
