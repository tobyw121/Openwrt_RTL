/*
 * Copyright(c) Realtek Semiconductor Corporation, 2009
 * All rights reserved.
 *
 * $Revision: 10237 $
 * $Date: 2010-06-14 11:23:26 +0800 (Mon, 14 Jun 2010) $
 *
 * Purpose : Definition those public OAM routing APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) OAM (802.3ah) configuration
 *           2) CFM (802.1ag) configuration
 */

#ifndef __DAL_ESW_OAM_H__
#define __DAL_ESW_OAM_H__

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/oam.h>

/*
 * Symbol Definition
 */


/*
 * Macro Declaration
 */


/*
 * Function Declaration
 */

/* Function Name:
 *      dal_esw_oam_init
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
 * Note:
 *      Must initialize OAM module before calling any OAM APIs.
 */
extern int32
dal_esw_oam_init(uint32 unit);

/* Module Name    : OAM               */
/* Sub-module Name: OAM configuration */

/* Function Name:
 *      dal_esw_oam_oamCounter_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_oam_oamCounter_get(uint32 unit, rtk_port_t port, uint32 *pNumber);

/* Function Name:
 *      dal_esw_oam_txTestFrame_start
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_oam_txTestFrame_start(uint32 unit, rtk_oam_testFrameCfg_t *pTestFrameCfg);

/* Function Name:
 *      dal_esw_oam_txTestFrame_stop
 * Description:
 *      stop transmitting OAM test frame.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 * Note:
 *      None
 */
extern int32
dal_esw_oam_txTestFrame_stop(uint32 unit);

/* Function Name:
 *      dal_esw_oam_txStatusOfTestFrame_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Transmitting status is as following:
 *      - TX_FINISH
 *      - TX_NOT_FINISH
 */
extern int32
dal_esw_oam_txStatusOfTestFrame_get(uint32 unit, rtk_oam_testFrameTxStatus_t *pTxStatus, uint64 *pTxCount);

/* Function Name:
 *      dal_esw_oam_loopbackMode_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      OAM loopback mode is as following:
 *      - OAM_LOOPBACK_ALL_FRAME
 *      - OAM_LOOPBACK_TEST_FRAME
 *      - OAM_LOOPBACK_DROP_TEST_FRAME
 *      - OAM_LOOPBACK_DISABLE
 */
extern int32
dal_esw_oam_loopbackMode_get(
    uint32                  unit, 
    rtk_port_t              port, 
    rtk_oam_loopbackMode_t  *pLoopbackMode);

/* Function Name:
 *      dal_esw_oam_loopbackMode_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      OAM loopback mode is as following:
 *      - OAM_LOOPBACK_ALL_FRAME
 *      - OAM_LOOPBACK_TEST_FRAME
 *      - OAM_LOOPBACK_DROP_TEST_FRAME
 *      - OAM_LOOPBACK_DISABLE
 */
extern int32
dal_esw_oam_loopbackMode_set(
    uint32                  unit, 
    rtk_port_t              port, 
    rtk_oam_loopbackMode_t  loopbackMode);

/* Function Name:
 *      dal_esw_oam_loopbackCtrl_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_oam_loopbackCtrl_get(
    uint32                  unit, 
    rtk_port_t              port, 
    rtk_oam_loopbackCtrl_t  *pLoopbackCtrl);

/* Function Name:
 *      dal_esw_oam_loopbackCtrl_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_oam_loopbackCtrl_set(
    uint32                  unit, 
    rtk_port_t              port, 
    rtk_oam_loopbackCtrl_t  *pLoopbackCtrl);

/* Function Name:
 *      dal_esw_oam_DyingGaspSend_start
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_MASK        - invalid portmask
 * Note:
 *      This API will be used when CPU want to send dying gasp by itself.
 */
extern int32
dal_esw_oam_DyingGaspSend_start(uint32 unit, rtk_portmask_t *pPortmask);

/* Function Name:
 *      dal_esw_oam_autoDyingGaspEnable_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_oam_autoDyingGaspEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_oam_autoDyingGaspEnable_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_oam_autoDyingGaspEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_oam_dyingGaspTLV_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_oam_dyingGaspTLV_get(uint32 unit, rtk_port_t port, rtk_oam_dyingGaspTLV_t *pTLV);

/* Function Name:
 *      dal_esw_oam_dyingGaspTLV_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_oam_dyingGaspTLV_set(uint32 unit, rtk_port_t port, rtk_oam_dyingGaspTLV_t *pTLV);

/* Function Name:
 *      dal_esw_oam_dyingGaspWaitTime_get
 * Description:
 *      Get waiting time of sending dying gasp after voltage is lower than expeted.
 * Input:
 *      unit - unit id
 * Output:
 *      time - waiting time
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Granularity of waiting time is 10 ns.
 */
extern int32
dal_esw_oam_dyingGaspWaitTime_get(uint32 unit, uint32 *pTime);

/* Function Name:
 *      dal_esw_oam_dyingGaspWaitTime_set
 * Description:
 *      Set waiting time of sending dying gasp after voltage is lower than expeted.
 * Input:
 *      unit - unit id
 *      time - waiting time
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 * Note:
 *      Granularity of waiting time is 10 ns.
 */
extern int32
dal_esw_oam_dyingGaspWaitTime_set(uint32 unit, uint32 time);

/* Module Name    : OAM               */
/* Sub-module Name: CFM configuration */

/* Function Name:
 *      dal_esw_oam_cfmEntry_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      CFM group = MA
 */
extern int32
dal_esw_oam_cfmEntry_get(uint32 unit, uint32 cfm_idx, rtk_oam_cfm_t *pCfm);

/* Function Name:
 *      dal_esw_oam_cfmEntry_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      CFM group = MA
 */
extern int32
dal_esw_oam_cfmEntry_set(uint32 unit, uint32 cfm_idx, rtk_oam_cfm_t *pCfm);

/* Function Name:
 *      dal_esw_oam_cfmPortEntry_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Each port can be only belong to one CFM group.
 *      mepid of configuration is used when transmit CCM packets.
 */
extern int32
dal_esw_oam_cfmPortEntry_get(uint32 unit, rtk_port_t port, rtk_oam_cfmPort_t *pPortCfg);

/* Function Name:
 *      dal_esw_oam_cfmPortEntry_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Each port can be only belong to one CFM group.
 *      mepid of configuration is used when transmit CCM packets.
 */
extern int32
dal_esw_oam_cfmPortEntry_set(uint32 unit, rtk_port_t port, rtk_oam_cfmPort_t *pPortCfg);

/* Function Name:
 *      dal_esw_oam_cfmMepEnable_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_oam_cfmMepEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_oam_cfmMepEnable_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_oam_cfmMepEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_oam_txCCMFrame_start
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_oam_txCCMFrame_start(uint32 unit, rtk_portmask_t *pPortmask);

/* Function Name:
 *      dal_esw_oam_txCCMFrame_stop
 * Description:
 *      Stop sending CCM packet.
 * Input:
 *      unit   - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 * Note:
 *      CCM transmit and OAM's test frame transmit can't be enabled at same time.
 */
extern int32
dal_esw_oam_txCCMFrame_stop(uint32 unit);

/* Function Name:
 *      dal_esw_oam_cfmCCMFrame_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_oam_cfmCCMFrame_get(uint32 unit, uint32 cfm_idx, rtk_oam_ccmFrame_t *pCcmFrame);

/* Function Name:
 *      dal_esw_oam_cfmCCMFrame_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_oam_cfmCCMFrame_set(uint32 unit, uint32 cfm_idx, rtk_oam_ccmFrame_t *pCcmFrame);

/* Function Name:
 *      dal_esw_oam_cfmCCMSnapOui_get
 * Description:
 *      Get OUI of SNAP packets when packet type of CCM packet is SNAP.
 * Input:
 *      unit     - unit id
 * Output:
 *      pSnapoui - pointer to OUI of SNAP packets
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_oam_cfmCCMSnapOui_get(uint32 unit, rtk_snapOui_t *pSnapoui);

/* Function Name:
 *      dal_esw_oam_cfmCCMSnapOui_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_oam_cfmCCMSnapOui_set(uint32 unit, rtk_snapOui_t *pSnapoui);

/* Function Name:
 *      dal_esw_oam_cfmCCMEtype_get
 * Description:
 *      Get ethernet type of CCM packets for specified CFM group.
 * Input:
 *      unit       - unit id
 * Output:
 *      pEtherType - pointer to ethernet type of CCM packets
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_oam_cfmCCMEtype_get(uint32 unit, uint32 *pEtherType);

/* Function Name:
 *      dal_esw_oam_cfmCCMEtype_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Default ethernet type of CCM packets is 0x8902
 */
extern int32
dal_esw_oam_cfmCCMEtype_set(uint32 unit, uint32 etherType);

/* Function Name:
 *      dal_esw_oam_cfmCCMOpcode_get
 * Description:
 *      Get opcode of CFM header.
 * Input:
 *      unit    - unit id
 * Output:
 *      pOpcode - pointer to opcode of CCM header
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_oam_cfmCCMOpcode_get(uint32 unit, uint32 *pOpcode);

/* Function Name:
 *      dal_esw_oam_cfmCCMOpcode_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Default opcode is 0x1
 */
extern int32
dal_esw_oam_cfmCCMOpcode_set(uint32 unit, uint32 opcode);

/* Function Name:
 *      dal_esw_oam_cfmCCMOpcode_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_oam_cfmCCMFlag_get(uint32 unit, uint32 cfm_idx, uint32 *pCcmFlag);

/* Function Name:
 *      dal_esw_oam_cfmCCMFlag_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_oam_cfmCCMFlag_set(uint32 unit, uint32 cfm_idx, uint32 ccmFlag);

/* Function Name:
 *      dal_esw_oam_cfmCCMInterval_get
 * Description:
 *      Get transmit interval of CCM packets.
 * Input:
 *      unit      - unit id
 * Output:
 *      pInterval - pointer to transmit interval
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
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
dal_esw_oam_cfmCCMInterval_get(uint32 unit, rtk_oam_ccmInterval_t *pInterval);

/* Function Name:
 *      dal_esw_oam_cfmCCMInterval_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_oam_cfmCCMInterval_set(uint32 unit, rtk_oam_ccmInterval_t interval);

/* Function Name:
 *      dal_esw_oam_cfmIntfStatus_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_oam_cfmIntfStatus_get(uint32 unit, rtk_port_t port, uint32 *pIntfStatus);

/* Function Name:
 *      dal_esw_oam_cfmIntfStatus_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_oam_cfmIntfStatus_set(uint32 unit, rtk_port_t port, uint32 intfStatus);

/* Function Name:
 *      dal_esw_oam_cfmPortStatus_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_oam_cfmPortStatus_get(uint32 unit, rtk_port_t port, uint32 *pPortStatus);

/* Function Name:
 *      dal_esw_oam_cfmPortStatus_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_oam_cfmPortStatus_set(uint32 unit, rtk_port_t port, uint32 portStatus);

/* Function Name:
 *      dal_esw_oam_cfmRemoteMep_del
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_ENTRY_NOTFOUND   - remote mep not found
 * Note:
 *      None
 */
extern int32
dal_esw_oam_cfmRemoteMep_del(uint32 unit, uint32 mepid);

/* Function Name:
 *      dal_esw_oam_cfmRemoteMep_add
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_oam_cfmRemoteMep_add(uint32 unit, uint32 mepid);

/* Function Name:
 *      dal_esw_oam_cfmRemoteMep_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_oam_cfmRemoteMep_get(uint32 unit, uint32 index, uint32* pMepid, uint32 *pValid);

   /*end of dal_esw_oam_cfmRemoteMep_get*/

/* Function Name:
 *      dal_esw_oam_cfmRemoteMep_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_oam_cfmRemoteMep_set(uint32 unit, uint32 index, uint32 mepid, uint32 valid);

/* Function Name:
 *      dal_esw_oam_cfmCCStatus_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_oam_cfmCCStatus_get(
    uint32              unit, 
    rtk_port_t          port, 
    uint32              remoteMepid,
    uint32              *pRdi,
    rtk_oam_ccStatus_t  *pCcStatus);

/* Function Name:
 *      dal_esw_oam_cfmCCStatus_reset
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 * Note:
 *      None
 */
extern int32
dal_esw_oam_cfmCCStatus_reset(uint32 unit, rtk_port_t port);

/* Function Name:
 *      dal_esw_oam_cfmLoopbackReplyEnable_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_oam_cfmLoopbackReplyEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_oam_cfmLoopbackReplyEnable_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_oam_cfmLoopbackReplyEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_oam_cfmLoopbackReplyEnable_get
 * Description:
 *      Get control of CFM loopback action.
 * Input:
 *      unit  - unit id
 * Output:
 *      pCtrl - pointer to loopback control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_oam_cfmLoopbackReplyCtrl_get(uint32 unit, rtk_oam_cfmLoopbackCtrl_t *pCtrl);

/* Function Name:
 *      dal_esw_oam_cfmLoopbackReplyEnable_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_oam_cfmLoopbackReplyCtrl_set(uint32 unit, rtk_oam_cfmLoopbackCtrl_t *pCtrl);

#endif /* __DAL_ESW_OAM_H__ */
