/*
 * Copyright(c) Realtek Semiconductor Corporation, 2009
 * All rights reserved.
 *
 * $Revision: 8999 $
 * $Date: 2010-04-12 15:42:36 +0800 (Mon, 12 Apr 2010) $
 *
 * Purpose : Definition those public MIRROR APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) Port-based mirror
 *           2) Group-based mirror
 *           3) RSPAN
 *           4) Mirror-based SFLOW
 *           5) Port-based SFLOW
 */

#ifndef __DAL_ESW_MIRROR_H__
#define __DAL_ESW_MIRROR_H__


/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/mirror.h>


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

/* Module Name : Mirror */

/* Function Name:
 *      dal_esw_mirror_init
 * Description:
 *      Initialize the mirroring database.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 * Note:
 *      Must initialize Mirror module before calling any Mirror APIs.
 */
extern int32
dal_esw_mirror_init(uint32 unit);

/* Module Name    : Mirror            */
/* Sub-module Name: Port-based mirror */

/* Function Name:
 *      dal_esw_mirror_portBased_create
 * Description:
 *      Create one mirroring session in the specified device.
 * Input:
 *      unit           - unit id
 *      mirroring_port - mirroring port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_PORT_ID           - invalid port id
 *      RT_ERR_MIRROR_PORT_FULL  - Exceeds maximum number of supported mirroring port
 *      RT_ERR_MIRROR_PORT_EXIST - mirroring port already exists
 * Note:
 *      None
 */
extern int32
dal_esw_mirror_portBased_create(uint32 unit, rtk_port_t mirroring_port);


/* Function Name:
 *      dal_esw_mirror_portBased_destroy
 * Description:
 *      Destroy one mirroring session from the specified device.
 * Input:
 *      unit           - unit id
 *      mirroring_port - mirroring port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - invalid unit id
 *      RT_ERR_PORT_ID               - invalid port id
 *      RT_ERR_MIRROR_PORT_NOT_EXIST - mirroring port does not exists
 * Note:
 *      1. In RTL8329/RTL8389, if you have sess-0 (mirroring_port A with TX) and
 *         sess-1 (mirroring_port A with RX), remove mirroring_port A will
 *         remove those 2 sessions at the same time.
 */
extern int32
dal_esw_mirror_portBased_destroy(uint32 unit, rtk_port_t mirroring_port);


/* Function Name:
 *      dal_esw_mirror_portBased_destroyAll
 * Description:
 *      Destroy all mirroring sessions from the specified device.
 * Input:
 *      unit           - unit id
 *      mirroring_port - mirroring port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      None
 */
extern int32
dal_esw_mirror_portBased_destroyAll(uint32 unit);


/* Function Name:
 *      dal_esw_mirror_portBased_get
 * Description:
 *      Get the mirroring session information by mirroring port from
 *      the specified device.
 * Input:
 *      unit                  - unit id
 *      mirroring_port        - mirroring port
 * Output:
 *      pMirrored_rx_portmask - pointer buffer of rx of mirrored ports
 *      pMirrored_tx_portmask - pointer buffer of tx of mirrored ports
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_PORT_ID                - invalid port id
 *      RT_ERR_NULL_POINTER           - input parameter may be null pointer
 *      RT_ERR_MIRROR_SESSION_NOEXIST - mirroring session not exist
 *      RT_ERR_MIRROR_PORT_NOT_EXIST  - mirroring port does not exists
 * Note:
 *      1. In RTL8329/RTL8389, if you have sess-0 (mirroring_port A with TX) and
 *         sess-1 (mirroring_port A with RX), get mirroring_port A will
 *         reply rx_portmask and tx_portmask at the same time.
 */
extern int32
dal_esw_mirror_portBased_get(
    uint32          unit,
    rtk_port_t      mirroring_port,
    rtk_portmask_t  *pMirrored_rx_portmask,
    rtk_portmask_t  *pMirrored_tx_portmask);


/* Function Name:
 *      dal_esw_mirror_portBased_set
 * Description:
 *      Set the mirroring session information by mirroring port to
 *      the specified device.
 * Input:
 *      unit                  - unit id
 *      mirroring_port        - mirroring port
 *      pMirrored_rx_portmask - rx of mirrored ports
 *      pMirrored_tx_portmask - tx of mirrored ports
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_PORT_ID                - invalid port id
 *      RT_ERR_MIRROR_SESSION_NOEXIST - mirroring session not exist
 *      RT_ERR_MIRROR_PORT_NOT_EXIST  - mirroring port does not exists
 * Note:
 *      None
 */
extern int32
dal_esw_mirror_portBased_set(
    uint32          unit,
    rtk_port_t      mirroring_port,
    rtk_portmask_t  *pMirrored_rx_portmask,
    rtk_portmask_t  *pMirrored_tx_portmask);

/* Module Name    : Mirror             */
/* Sub-module Name: Group-based mirror */

/* Function Name:
 *      dal_esw_mirror_group_init
 * Description:
 *      Initialization mirror group entry.
 * Input:
 *      unit         - unit id
 *      pMirrorEntry - mirror entry
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_MIRROR_ID        - invalid mirror id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_mirror_group_init(
    uint32              unit,
    rtk_mirror_entry_t  *pMirrorEntry);

/* Function Name:
 *      dal_esw_mirror_group_get
 * Description:
 *      Get mirror group entry.
 * Input:
 *      unit         - unit id
 *      mirror_id    - mirror id
 * Output:
 *      pMirrorEntry - mirror entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_MIRROR_ID        - invalid mirror id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_mirror_group_get(
    uint32              unit,
    uint32              mirror_id,
    rtk_mirror_entry_t  *pMirrorEntry);

/* Function Name:
 *      dal_esw_mirror_group_set
 * Description:
 *      Set mirror group entry.
 * Input:
 *      unit         - unit id
 *      mirror_id    - mirror id
 *      pMirrorEntry - mirror entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_MIRROR_ID        - invalid mirror id
 *      RT_ERR_PORT_ID          - invalid mirroring port id
 *      RT_ERR_PORT_MASK        - invalid mirrored ingress or egress portmask
 * Note:
 *      None
 */
extern int32
dal_esw_mirror_group_set(
    uint32              unit,
    uint32              mirror_id,
    rtk_mirror_entry_t  *pMirrorEntry);

/* Function Name:
 *      dal_esw_mirror_egrMode_get
 * Description:
 *      Get egress filter mode on specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pEgrMode - pointer to egress filter mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Egress filter mode is as following:
 *      - FORWARD_ALL_PKTS
 *      - FORWARD_MIRRORED_PKTS_ONLY
 */
extern int32
dal_esw_mirror_egrMode_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_mirror_egrMode_t    *pEgrMode);

/* Function Name:
 *      dal_esw_mirror_egrMode_set
 * Description:
 *      Set egress filter mode on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      egrMode - egress filter mode
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
 *      Egress filter mode is as following:
 *      - FORWARD_ALL_PKTS
 *      - FORWARD_MIRRORED_PKTS_ONLY
 */
extern int32
dal_esw_mirror_egrMode_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_mirror_egrMode_t    egrMode);

/* Module Name    : Mirror */
/* Sub-module Name: RSPAN  */
/* Function Name:
 *      dal_esw_mirror_portRspanIgrMode_get
 * Description:
 *      Get ingress mode of RSPAN on specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pIgrMode - pointer to ingress mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Ingress mode is as following:
 *      - RSPAN_IGR_HANDLE_RSPAN_TAG
 *      - RSPAN_IGR_IGNORE_RSPAN_TAG
 */
extern int32
dal_esw_mirror_portRspanIgrMode_get(uint32 unit, rtk_port_t port, rtk_mirror_rspanIgrMode_t *pIgrMode);

/* Function Name:
 *      dal_esw_mirror_portRspanIgrMode_set
 * Description:
 *      Set ingress mode of RSPAN on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      igrMode - ingress mode
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
 *      Ingress mode is as following:
 *      - RSPAN_IGR_HANDLE_RSPAN_TAG
 *      - RSPAN_IGR_IGNORE_RSPAN_TAG
 */
extern int32
dal_esw_mirror_portRspanIgrMode_set(uint32 unit, rtk_port_t port, rtk_mirror_rspanIgrMode_t igrMode);

/* Function Name:
 *      dal_esw_mirror_portRspanEgrMode_get
 * Description:
 *      Get egress mode of RSPAN on specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pEgrMode - pointer to egress mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Ingress mode is as following:
 *      - RSPAN_EGR_HANDLE_RSPAN_TAG
 *      - RSPAN_EGR_IGNORE_RSPAN_TAG
 */
extern int32
dal_esw_mirror_portRspanEgrMode_get(uint32 unit, rtk_port_t port, rtk_mirror_rspanEgrMode_t *pEgrMode);

/* Function Name:
 *      dal_esw_mirror_portRspanEgrMode_set
 * Description:
 *      Set egress mode of RSPAN on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      egrMode - egress mode
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
 *      Ingress mode is as following:
 *      - RSPAN_EGR_HANDLE_RSPAN_TAG
 *      - RSPAN_EGR_IGNORE_RSPAN_TAG
 */
extern int32
dal_esw_mirror_portRspanEgrMode_set(uint32 unit, rtk_port_t port, rtk_mirror_rspanEgrMode_t egrMode);

/* Function Name:
 *      dal_esw_mirror_rspanIgrTag_get
 * Description:
 *      Get content of ingress tag on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pIgrTag - pointer to content of ingress tag
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
dal_esw_mirror_rspanIgrTag_get(uint32 unit, rtk_port_t port, rtk_mirror_rspanIgrTag_t *pIgrTag);

/* Function Name:
 *      dal_esw_mirror_rspanIgrTag_set
 * Description:
 *      Set content of ingress tag on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      pIgrTag - content of ingress tag
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
dal_esw_mirror_rspanIgrTag_set(uint32 unit, rtk_port_t port, rtk_mirror_rspanIgrTag_t *pIgrTag);

/* Function Name:
 *      dal_esw_mirror_rspanEgrTag_get
 * Description:
 *      Get content of egress tag on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEgrTag - pointer to content of egress tag
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
dal_esw_mirror_rspanEgrTag_get(uint32 unit, rtk_port_t port, rtk_mirror_rspanEgrTag_t *pEgrTag);

/* Function Name:
 *      dal_esw_mirror_rspanEgrTag_set
 * Description:
 *      Set content of egress tag on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      pEgrTag - content of egress tag
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
dal_esw_mirror_rspanEgrTag_set(uint32 unit, rtk_port_t port, rtk_mirror_rspanEgrTag_t *pEgrTag);

/* Module Name    : Mirror             */
/* Sub-module Name: Mirror-based SFLOW */

/* Function Name:
 *      dal_esw_mirror_sflowMirrorSeed_get
 * Description:
 *      Get sampling seed of sflow for mirror group sampling.
 * Input:
 *      unit  - unit id
 * Output:
 *      pSeed - pointer to sampling seed of sflow
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
dal_esw_mirror_sflowMirrorSeed_get(uint32 unit, uint32 *pSeed);

/* Function Name:
 *      dal_esw_mirror_sflowMirrorSeed_set
 * Description:
 *      Set sampling seed of sflow for mirror group sampling.
 * Input:
 *      unit - unit id
 *      seed - sampling seed of sflow
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
dal_esw_mirror_sflowMirrorSeed_set(uint32 unit, uint32 seed);

/* Function Name:
 *      dal_esw_mirror_sflowMirrorSampleEnable_get
 * Description:
 *      Get enable status of sampling on specified mirror group.
 * Input:
 *      unit      - unit id
 *      mirror_id - mirror id
 * Output:
 *      pEnable   - pointer to enable status of sampling
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_MIRROR_ID        - invalid mirror id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_mirror_sflowMirrorSampleEnable_get(uint32 unit, uint32 mirror_id, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_mirror_sflowMirrorSampleEnable_set
 * Description:
 *      Set enable status of sampling on specified mirror group.
 * Input:
 *      unit      - unit id
 *      mirror_id - mirror id
 *      enable    - enable status of sampling
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_MIRROR_ID        - invalid mirror id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_mirror_sflowMirrorSampleEnable_set(uint32 unit, uint32 mirror_id, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_mirror_sflowMirrorSampleRate_get
 * Description:
 *      Get sampling rate of specified mirror group.
 * Input:
 *      unit      - unit id
 *      mirror_id - mirror id
 * Output:
 *      pRate     - pointer to sampling rate
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_MIRROR_ID        - invalid mirror id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_mirror_sflowMirrorSampleRate_get(uint32 unit, uint32 mirror_id, uint32 *pRate);

/* Function Name:
 *      dal_esw_mirror_sflowMirrorSampleRate_set
 * Description:
 *      Set sampling rate of specified mirror group.
 * Input:
 *      unit      - unit id
 *      mirror_id - mirror id
 *      rate      - sampling rate
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_MIRROR_ID        - invalid mirror id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_mirror_sflowMirrorSampleRate_set(uint32 unit, uint32 mirror_id, uint32 rate);

/* Function Name:
 *      dal_esw_mirror_sflowMirrorSampleStat_get
 * Description:
 *      Set statistic of sampling on specified mirror group.
 * Input:
 *      unit      - unit id
 *      mirror_id - mirror id
 * Output:
 *      pStat     - pointer to statistic of sampling
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_MIRROR_ID        - invalid mirror id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_mirror_sflowMirrorSampleStat_get(uint32 unit, uint32 mirror_id, rtk_mirror_sampleStat_t *pStat);

/* Module Name    : Mirror           */
/* Sub-module Name: Port-based SFLOW */

/* Function Name:
 *      dal_esw_mirror_sflowPortSeed_get
 * Description:
 *      Get sampling seed of sflow for port sampling.
 * Input:
 *      unit  - unit id
 * Output:
 *      pSeed - pointer to sampling seed of sflow
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
dal_esw_mirror_sflowPortSeed_get(uint32 unit, uint32 *pSeed);

/* Function Name:
 *      dal_esw_mirror_sflowPortSeed_set
 * Description:
 *      Set sampling seed of sflow for port sampling.
 * Input:
 *      unit - unit id
 *      seed - sampling seed of sflow
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
dal_esw_mirror_sflowPortSeed_set(uint32 unit, uint32 seed);

/* Function Name:
 *      dal_esw_mirror_sflowPortIgrSampleEnable_get
 * Description:
 *      Get enable status of ingress sampling on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of ingress sampling
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
dal_esw_mirror_sflowPortIgrSampleEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_mirror_sflowPortIgrSampleEnable_set
 * Description:
 *      Set enable status of ingress sampling on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of ingress sampling
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
dal_esw_mirror_sflowPortIgrSampleEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_mirror_sflowPortIgrSampleEnable_get
 * Description:
 *      Get sampling rate of ingress sampling on specified port.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pRate - pointer to sampling rate of ingress sampling
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
dal_esw_mirror_sflowPortIgrSampleRate_get(uint32 unit, rtk_port_t port, uint32 *pRate);

/* Function Name:
 *      dal_esw_mirror_sflowPortIgrSampleEnable_set
 * Description:
 *      Set sampling rate of ingress sampling on specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 *      rate - sampling rate of ingress sampling
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
dal_esw_mirror_sflowPortIgrSampleRate_set(uint32 unit, rtk_port_t port, uint32 rate);

/* Function Name:
 *      dal_esw_mirror_sflowPortEgrSampleEnable_get
 * Description:
 *      Get enable status of egress sampling on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of egress sampling
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
dal_esw_mirror_sflowPortEgrSampleEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_mirror_sflowPortEgrSampleEnable_set
 * Description:
 *      Set enable status of egress sampling on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of egress sampling
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
dal_esw_mirror_sflowPortEgrSampleEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_mirror_sflowPortEgrSampleEnable_get
 * Description:
 *      Get sampling rate of egress sampling on specified port.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pRate - pointer to sampling rate of egress sampling
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
dal_esw_mirror_sflowPortEgrSampleRate_get(uint32 unit, rtk_port_t port, uint32 *pRate);

/* Function Name:
 *      dal_esw_mirror_sflowPortEgrSampleRate_set
 * Description:
 *      Set sampling rate of egress sampling on specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 *      rate - sampling rate of egress sampling
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
dal_esw_mirror_sflowPortEgrSampleRate_set(uint32 unit, rtk_port_t port, uint32 rate);

/* Function Name:
 *      dal_esw_mirror_sflowAddCPUTagEnable_get
 * Description:
 *      Get enable status of CPU tag adding for trapped packet of SFLOW.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of CPU tag adding
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_mirror_sflowAddCPUTagEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_mirror_sflowAddCPUTagEnable_set
 * Description:
 *      Set enable status of CPU tag adding for trapped packet of SFLOW.
 * Input:
 *      unit   - unit id
 *      enable - enable status of CPU tag adding
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
dal_esw_mirror_sflowAddCPUTagEnable_set(uint32 unit, rtk_enable_t enable);


#endif /* __DAL_ESW_MIRROR_H__ */
