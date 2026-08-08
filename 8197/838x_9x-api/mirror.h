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
 * $Revision: 37509 $
 * $Date: 2013-03-06 17:07:46 +0800 (Wed, 06 Mar 2013) $
 *
 * Purpose : Definition of Mirror API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Port-based mirror
 *           (2) Group-based mirror
 *           (3) RSPAN
 *           (4) Mirror-based SFLOW
 *           (5) Port-based SFLOW
 */

#ifndef __RTK_MIRROR_H__
#define __RTK_MIRROR_H__

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/port.h>

/*
 * Symbol Definition
 */

/* egress mode of mirror on egress port */
typedef enum rtk_mirror_egrMode_e
{
    FORWARD_ALL_PKTS,           /* forwarded and mirrored packet are allowed to send to mirroring port */
    FORWARD_MIRRORED_PKTS_ONLY, /* only mirrored packet is allowed to send to mirroring port */
    MIRROR_EGR_MODE_END
} rtk_mirror_egrMode_t;

/* Ingress mode of RSPAN */
typedef enum rtk_mirror_rspanIgrMode_e
{
    RSPAN_IGR_IGNORE_RSPAN_TAG = 0,/* disable the device to parse RSPAN tag for received packet */
    RSPAN_IGR_HANDLE_RSPAN_TAG,    /* enable the device to parse RSPAN tag for received packet */
    RSPAN_IGR_END
} rtk_mirror_rspanIgrMode_t;

/* Egress mode of RSPAN */
typedef enum rtk_mirror_rspanEgrMode_e
{
    RSPAN_EGR_REMOVE_TAG = 0,
    RSPAN_EGR_ADD_TAG,
    RSPAN_EGR_NO_MODIFY,
    RSPAN_EGR_END
} rtk_mirror_rspanEgrMode_t;

/* sFlow control */
typedef enum rtk_sflowSampleCtrl_e
{
    SFLOW_CTRL_INGRESS,
    SFLOW_CTRL_EGRESS,
    SFLOW_CTRL_END
} rtk_sflowSampleCtrl_t;

/*
 * Data Declaration
 */
/* configuration of mirror entry */
typedef struct rtk_mirror_entry_s
{
#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
    rtk_enable_t    mirror_enable;
#endif
    rtk_port_t      mirroring_port;
    rtk_portmask_t  mirrored_igrPorts;
    rtk_portmask_t  mirrored_egrPorts;
    uint32          oper_of_igr_and_egr_ports;
                    /* 0: Mirror packets that match ingress ports or egress ports
                     * 1: Mirror packets that match ingress ports and egress ports at same time
                     */
#if defined(CONFIG_SDK_RTL8328) || defined(CONFIG_SDK_RTL8380)
    rtk_enable_t    cross_vlan;
                    /* ENABLED: Mirror the packets even they are not in same vlan of mirroring_port
                     * DISABLED: Only mirror the packets that is in same vlan of mirroring_port
                     */
#endif
#if defined(CONFIG_SDK_RTL8328)
    rtk_enable_t    mirror_ucast;
    rtk_enable_t    mirror_mcast;
    rtk_enable_t    mirror_bcast;
    rtk_enable_t    mirror_goodPkt;
    rtk_enable_t    mirror_badPkt;
#endif
    rtk_enable_t    mirror_orginalPkt;
                    /* ENABLED: Mirror original packets
                     * DISABLED: Mirror modified packets
                     */
#if defined(CONFIG_SDK_RTL8328) || defined(CONFIG_SDK_RTL8380)
    rtk_enable_t    flowBasedOnly;
                    /* ENABLED: Will be only used for flow based mirror.
                     * DISABLED: Will work on all type of mirror (flow based and port based)
                     */
#endif
#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
    rtk_mirror_egrMode_t mirroring_port_egrMode;
#endif
#if defined(CONFIG_SDK_RTL8380)
    uint32          duplicate_fltr;     /* if a packet is forwarded to mirroring port and also be mirrored,
                                         * the configuration indicate how many copy of the packet is sent to mirroring port.
                                         * 0x0: both mirrored and forwarded
                                         * 0x1: only forwarded packet
                                         */
    uint32          self_flter;         /* whether to mirror the packet which transmitted from mirroring port.
                                         * 0x0: it could be mirrored to mirroring port
                                         * 0x1: the mirrored packet is filtered
                                         */
    uint32          mir_mode;           /* mirror mode.
                                         * 0x0: RX mirror is original packet and TX mirror is modified packet
                                         * 0x1: follow mirroring port configuration
                                         */
    uint32          mir_qid;            /* specify mirrored traffic output queue id */
    rtk_enable_t    mir_qid_en;         /* force output queue id for mirrored traffic */
    uint32          flowBased_pmsk_ignore;  /*flow based mirror ignore SPM/DPM/MIR_OP settings of mirror*/
#endif
} rtk_mirror_entry_t;

/* Ingress tag of RSPAN */
typedef struct rtk_mirror_rspanIgrTag_s
{
    uint32      tpid;
    rtk_vlan_t  vid;
} rtk_mirror_rspanIgrTag_t;

/* Egress tag of RSPAN */
typedef struct rtk_mirror_rspanEgrTag_s
{
    uint32          tpid;
    rtk_vlan_t      vid;
    rtk_pri_t       pri;
    uint32          cfi;
} rtk_mirror_rspanEgrTag_t;

/* Tag of RSPAN */
typedef struct rtk_mirror_rspanTag_s
{
#if defined(CONFIG_SDK_RTL8380)
    uint32          tpid;
#endif
#if defined(CONFIG_SDK_RTL8390)
    uint32          tpidIdx;
#endif
    rtk_vlan_t      vid;
    rtk_pri_t       pri;
    uint32          cfi;
} rtk_mirror_rspanTag_t;

/* statistic of sampling */
typedef struct rtk_mirror_sampleStat_s
{
    uint32  totalSamples;
    uint32  totalPkts;
} rtk_mirror_sampleStat_t;

/*
 * Macro Declaration
 */

/*
 * Function Declaration
 */

/* Module Name : Mirror */

/* Function Name:
 *      rtk_mirror_init
 * Description:
 *      Initialize the mirroring database.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      Must initialize Mirror module before calling any Mirror APIs.
 */
extern int32
rtk_mirror_init(uint32 unit);

/* Module Name    : Mirror            */
/* Sub-module Name: Port-based mirror */

/* Function Name:
 *      rtk_mirror_portBased_create
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
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_PORT_ID           - invalid port id
 *      RT_ERR_MIRROR_PORT_FULL  - Exceeds maximum number of supported mirroring port
 *      RT_ERR_MIRROR_PORT_EXIST - mirroring port already exists
 * Applicable:
 *      8389
 * Note:
 *      None
 */
extern int32
rtk_mirror_portBased_create(uint32 unit, rtk_port_t mirroring_port);

/* Function Name:
 *      rtk_mirror_portBased_destroy
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
 *      RT_ERR_NOT_INIT              - The module is not initial
 *      RT_ERR_PORT_ID               - invalid port id
 *      RT_ERR_MIRROR_PORT_NOT_EXIST - mirroring port does not exists
 * Applicable:
 *      8389
 * Note:
 *      (1) If you have sess-0 (mirroring_port A with TX) and sess-1 (mirroring_port A with RX),
 *          remove mirroring_port A will remove those 2 sessions at the same time.
 */
extern int32
rtk_mirror_portBased_destroy(uint32 unit, rtk_port_t mirroring_port);

/* Function Name:
 *      rtk_mirror_portBased_destroyAll
 * Description:
 *      Destroy all mirroring sessions from the specified device.
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
 *      8389
 * Note:
 *      None
 */
extern int32
rtk_mirror_portBased_destroyAll(uint32 unit);

/* Function Name:
 *      rtk_mirror_portBased_get
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
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_PORT_ID                - invalid port id
 *      RT_ERR_NULL_POINTER           - input parameter may be null pointer
 *      RT_ERR_MIRROR_SESSION_NOEXIST - mirroring session not exist
 *      RT_ERR_MIRROR_PORT_NOT_EXIST  - mirroring port does not exists
 * Applicable:
 *      8389
 * Note:
 *      (1) If you have sess-0 (mirroring_port A with TX) and sess-1 (mirroring_port A with RX),
 *          get mirroring_port A will reply rx_portmask and tx_portmask at the same time.
 */
extern int32
rtk_mirror_portBased_get(
    uint32          unit,
    rtk_port_t      mirroring_port,
    rtk_portmask_t  *pMirrored_rx_portmask,
    rtk_portmask_t  *pMirrored_tx_portmask);

/* Function Name:
 *      rtk_mirror_portBased_set
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
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_PORT_ID                - invalid port id
 *      RT_ERR_MIRROR_SESSION_NOEXIST - mirroring session not exist
 *      RT_ERR_MIRROR_PORT_NOT_EXIST  - mirroring port does not exists
 * Applicable:
 *      8389
 * Note:
 *      None
 */
extern int32
rtk_mirror_portBased_set(
    uint32          unit,
    rtk_port_t      mirroring_port,
    rtk_portmask_t  *pMirrored_rx_portmask,
    rtk_portmask_t  *pMirrored_tx_portmask);

/* Module Name    : Mirror             */
/* Sub-module Name: Group-based mirror */

/* Function Name:
 *      rtk_mirror_group_init
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      Initialize the mirror entry. The operation is set to ingress OR egress ports.
 *      The mirroring_port, mirrored_igrPorts, and mirrored_egrPorts fields are set to empty,
 *      and should be assigned later by rtk_mirror_group_set API.
 */
extern int32
rtk_mirror_group_init(
    uint32              unit,
    rtk_mirror_entry_t  *pMirrorEntry);

/* Function Name:
 *      rtk_mirror_group_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_MIRROR_ID    - invalid mirror id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) The valid range of mirror_id is 0~3 in 8328, 8390, 8380.
 */
extern int32
rtk_mirror_group_get(
    uint32              unit,
    uint32              mirror_id,
    rtk_mirror_entry_t  *pMirrorEntry);

/* Function Name:
 *      rtk_mirror_group_set
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
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_MIRROR_ID            - invalid mirror id
 *      RT_ERR_PORT_ID              - invalid mirroring port id
 *      RT_ERR_PORT_MASK            - invalid mirrored ingress or egress portmask
 *      RT_ERR_INPUT                - invalid input parameter
 *      RT_ERR_MIRROR_DP_IN_SPM_DPM - mirroring port can not be in ingress or egress mirrored portmask of any mirroring set
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) The valid range of mirror_id is 0~3 in 8328, 8390, 8380.
 */
extern int32
rtk_mirror_group_set(
    uint32              unit,
    uint32              mirror_id,
    rtk_mirror_entry_t  *pMirrorEntry);

/* Function Name:
 *      rtk_mirror_rspanIgrMode_get
 * Description:
 *      Get ingress mode of RSPAN on specified mirror group.
 * Input:
 *      unit      - unit id
 *      mirror_id - mirror id
 * Output:
 *      pIgrMode  - pointer to ingress mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_MIRROR_ID    - invalid mirror ID
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) Ingress mode is as following:
 *          - RSPAN_IGR_HANDLE_RSPAN_TAG
 *          - RSPAN_IGR_IGNORE_RSPAN_TAG
 *      (2) Set RSPAN igress mode to RSPAN_IGR_HANDLE_RSPAN_TAG for destination switch.
 */
extern int32
rtk_mirror_rspanIgrMode_get(uint32 unit, uint32 mirror_id, rtk_mirror_rspanIgrMode_t *pIgrMode);

/* Function Name:
 *      rtk_mirror_rspanIgrMode_set
 * Description:
 *      Set ingress mode of RSPAN on specified mirror group.
 * Input:
 *      unit      - unit id
 *      mirror_id - mirror id
 *      igrMode   - ingress mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID   - invalid unit id
 *      RT_ERR_NOT_INIT  - The module is not initial
 *      RT_ERR_MIRROR_ID - invalid mirror ID
 *      RT_ERR_INPUT     - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) Ingress mode is as following:
 *          - RSPAN_IGR_HANDLE_RSPAN_TAG
 *          - RSPAN_IGR_IGNORE_RSPAN_TAG
 *      (2) Set RSPAN igress mode to RSPAN_IGR_HANDLE_RSPAN_TAG for destination switch.
 */
extern int32
rtk_mirror_rspanIgrMode_set(uint32 unit, uint32 mirror_id, rtk_mirror_rspanIgrMode_t igrMode);

/* Function Name:
 *      rtk_mirror_rspanEgrMode_get
 * Description:
 *      Get egress mode of RSPAN on specified mirror group.
 * Input:
 *      unit      - unit id
 *      mirror_id - mirror id
 * Output:
 *      pEgrMode  - pointer to egress mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_MIRROR_ID    - invalid mirror ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) RSPAN egress mode should be set to RSPAN_EGR_ADD_TAG for source switch and set to RSPAN_EGR_REMOVE_TAG
 *          for destination switch.
 *      (2) Egress mode is as following:
 *          - RSPAN_EGR_REMOVE_TAG
 *          - RSPAN_EGR_ADD_TAG
 *          - RSPAN_EGR_NO_MODIFY
 */
extern int32
rtk_mirror_rspanEgrMode_get(uint32 unit, uint32 mirror_id, rtk_mirror_rspanEgrMode_t *pEgrMode);

/* Function Name:
 *      rtk_mirror_rspanEgrMode_set
 * Description:
 *      Set egress mode of RSPAN on specified mirror group.
 * Input:
 *      unit      - unit id
 *      mirror_id - mirror id
 *      egrMode   - egress mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID   - invalid unit id
 *      RT_ERR_NOT_INIT  - The module is not initial
 *      RT_ERR_MIRROR_ID - invalid mirror ID
 *      RT_ERR_INPUT     - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) RSPAN egress mode should be set to RSPAN_EGR_ADD_TAG for source switch and set to RSPAN_EGR_REMOVE_TAG
 *          for destination switch.
 *      (2) Ingress mode is as following:
 *          - RSPAN_EGR_REMOVE_TAG
 *          - RSPAN_EGR_ADD_TAG
 *          - RSPAN_EGR_NO_MODIFY
 */
extern int32
rtk_mirror_rspanEgrMode_set(uint32 unit, uint32 mirror_id, rtk_mirror_rspanEgrMode_t egrMode);

/* Function Name:
 *      rtk_mirror_egrMode_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      Egress filter mode is as following:
 *      - FORWARD_ALL_PKTS
 *      - FORWARD_MIRRORED_PKTS_ONLY
 */
extern int32
rtk_mirror_egrMode_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_mirror_egrMode_t    *pEgrMode);

/* Function Name:
 *      rtk_mirror_egrMode_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      Egress filter mode is as following:
 *      - FORWARD_ALL_PKTS
 *      - FORWARD_MIRRORED_PKTS_ONLY
 */
extern int32
rtk_mirror_egrMode_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_mirror_egrMode_t    egrMode);

/* Module Name    : Mirror */
/* Sub-module Name: RSPAN  */

/* Function Name:
 *      rtk_mirror_portRspanIgrMode_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      Ingress mode is as following:
 *      - RSPAN_IGR_HANDLE_RSPAN_TAG
 *      - RSPAN_IGR_IGNORE_RSPAN_TAG
 */
extern int32
rtk_mirror_portRspanIgrMode_get(uint32 unit, rtk_port_t port, rtk_mirror_rspanIgrMode_t *pIgrMode);

/* Function Name:
 *      rtk_mirror_portRspanIgrMode_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      Ingress mode is as following:
 *      - RSPAN_IGR_HANDLE_RSPAN_TAG
 *      - RSPAN_IGR_IGNORE_RSPAN_TAG
 */
extern int32
rtk_mirror_portRspanIgrMode_set(uint32 unit, rtk_port_t port, rtk_mirror_rspanIgrMode_t igrMode);

/* Function Name:
 *      rtk_mirror_portRspanEgrMode_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      Egress mode is as following:
 *      - RSPAN_EGR_REMOVE_TAG
 *      - RSPAN_EGR_ADD_TAG
 *      - RSPAN_EGR_NO_MODIFY
 */
extern int32
rtk_mirror_portRspanEgrMode_get(uint32 unit, rtk_port_t port, rtk_mirror_rspanEgrMode_t *pEgrMode);

/* Function Name:
 *      rtk_mirror_portRspanEgrMode_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      Egress mode is as following:
 *      - RSPAN_EGR_REMOVE_TAG
 *      - RSPAN_EGR_ADD_TAG
 *      - RSPAN_EGR_NO_MODIFY
 */
extern int32
rtk_mirror_portRspanEgrMode_set(uint32 unit, rtk_port_t port, rtk_mirror_rspanEgrMode_t egrMode);

/* Function Name:
 *      rtk_mirror_rspanIgrTag_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      There are 2 fields in rtk_mirror_rspanIgrTag_t: tpid and vid.
 */
extern int32
rtk_mirror_rspanIgrTag_get(uint32 unit, rtk_port_t port, rtk_mirror_rspanIgrTag_t *pIgrTag);

/* Function Name:
 *      rtk_mirror_rspanIgrTag_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      There are 2 fields in rtk_mirror_rspanIgrTag_t: tpid and vid.
 */
extern int32
rtk_mirror_rspanIgrTag_set(uint32 unit, rtk_port_t port, rtk_mirror_rspanIgrTag_t *pIgrTag);

/* Function Name:
 *      rtk_mirror_rspanEgrTag_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      There are 4 fields in rtk_mirror_rspanEgrTag_t: tpid, vid, pri and cfi.
 */
extern int32
rtk_mirror_rspanEgrTag_get(uint32 unit, rtk_port_t port, rtk_mirror_rspanEgrTag_t *pEgrTag);

/* Function Name:
 *      rtk_mirror_rspanEgrTag_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      There are 4 fields in rtk_mirror_rspanEgrTag_t: tpid, vid, pri and cfi.
 */
extern int32
rtk_mirror_rspanEgrTag_set(uint32 unit, rtk_port_t port, rtk_mirror_rspanEgrTag_t *pEgrTag);

/* Function Name:
 *      rtk_mirror_rspanTag_get
 * Description:
 *      Get content of RSPAN tag on specified mirror group.
 * Input:
 *      unit      - unit id
 *      mirror_id - mirror id
 * Output:
 *      pTag      - pointer to content of RSPAN tag
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_MIRROR_ID    - invalid mirror ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) Each mirror group can specify its RSPAN tag content.
 *      (2) pTag->tpidIdx is the index to VLAN outer TPID list and rtk_vlan_outerTpidEntry_set could be used
 *          to configure the outer VLAN TPID.
 */
extern int32
rtk_mirror_rspanTag_get(uint32 unit, uint32 mirror_id, rtk_mirror_rspanTag_t *pTag);

/* Function Name:
 *      rtk_mirror_rspanTag_set
 * Description:
 *      Set content of RSPAN tag on specified mirroring group.
 * Input:
 *      unit      - unit id
 *      mirror_id - mirror id
 *      pTag      - content of RSPAN tag
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_MIRROR_ID    - invalid mirror ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_PRIORITY     - invalid priority
 *      RT_ERR_VLAN_VID     - invalid vid
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) Each mirror group can specify its RSPAN tag content.
 *      (2) pTag->tpidIdx is the index to VLAN outer TPID list and rtk_vlan_outerTpidEntry_set could be used
 *          to configure the outer VLAN TPID.
 */
extern int32
rtk_mirror_rspanTag_set(uint32 unit, uint32 mirror_id, rtk_mirror_rspanTag_t *pTag);

/* Module Name    : Mirror             */
/* Sub-module Name: Mirror-based SFLOW */

/* Function Name:
 *      rtk_mirror_sflowMirrorSeed_get
 * Description:
 *      Get sampling seed of sflow for mirror group sampling.
 * Input:
 *      unit  - unit id
 * Output:
 *      pSeed - pointer to sampling seed of sflow
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
rtk_mirror_sflowMirrorSeed_get(uint32 unit, uint32 *pSeed);

/* Function Name:
 *      rtk_mirror_sflowMirrorSeed_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
extern int32
rtk_mirror_sflowMirrorSeed_set(uint32 unit, uint32 seed);

/* Function Name:
 *      rtk_mirror_sflowMirrorSampleEnable_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_MIRROR_ID    - invalid mirror id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390
 * Note:
 *      (1) The valid range of mirror_id is 0~3 in 8328.
 *      (2) For 8390, the function is backward compatible RTL8328 APIs.
 */
extern int32
rtk_mirror_sflowMirrorSampleEnable_get(uint32 unit, uint32 mirror_id, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_mirror_sflowMirrorSampleEnable_set
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
 *      RT_ERR_UNIT_ID   - invalid unit id
 *      RT_ERR_NOT_INIT  - The module is not initial
 *      RT_ERR_MIRROR_ID - invalid mirror id
 *      RT_ERR_INPUT     - invalid input parameter
 * Applicable:
 *      8328, 8390
 * Note:
 *      (1) The valid range of mirror_id is 0~3 in 8328.
 *      (2) For 8390, the function is backward compatible RTL8328 APIs.
 */
extern int32
rtk_mirror_sflowMirrorSampleEnable_set(uint32 unit, uint32 mirror_id, rtk_enable_t enable);

/* Function Name:
 *      rtk_mirror_sflowMirrorSampleRate_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_MIRROR_ID    - invalid mirror id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      Set rate to N means that one packet is sampled out of N mirrored packets.
 *      (1) The valid range of mirror_id is 0~3 in 8328.
 *      (2) The valid range of rate is 0~0xFFFF in 8328.
 *      (3) The function is disabled if the rate is set to 0.
 */
extern int32
rtk_mirror_sflowMirrorSampleRate_get(uint32 unit, uint32 mirror_id, uint32 *pRate);

/* Function Name:
 *      rtk_mirror_sflowMirrorSampleRate_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_MIRROR_ID    - invalid mirror id
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      Set rate to N means that one packet is sampled out of N mirrored packets.
 *      (1) The valid range of mirror_id is 0~3 in 8328.
 *      (2) The valid range of rate is 0~0xFFFF in 8328.
 *      (3) The function is disabled if the rate is set to 0.
 */
extern int32
rtk_mirror_sflowMirrorSampleRate_set(uint32 unit, uint32 mirror_id, uint32 rate);

/* Function Name:
 *      rtk_mirror_sflowMirrorSampleStat_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_MIRROR_ID    - invalid mirror id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of mirror_id is 0~3 in 8328.
 */
extern int32
rtk_mirror_sflowMirrorSampleStat_get(uint32 unit, uint32 mirror_id, rtk_mirror_sampleStat_t *pStat);

/* Module Name    : Mirror           */
/* Sub-module Name: Port-based SFLOW */

/* Function Name:
 *      rtk_mirror_sflowPortSeed_get
 * Description:
 *      Get sampling seed of sflow for port sampling.
 * Input:
 *      unit  - unit id
 * Output:
 *      pSeed - pointer to sampling seed of sflow
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
rtk_mirror_sflowPortSeed_get(uint32 unit, uint32 *pSeed);

/* Function Name:
 *      rtk_mirror_sflowPortSeed_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
extern int32
rtk_mirror_sflowPortSeed_set(uint32 unit, uint32 seed);

/* Function Name:
 *      rtk_mirror_sflowPortIgrSampleEnable_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390
 * Note:
 *      For 8390, the function is backward compatible RTL8328 APIs.
 */
extern int32
rtk_mirror_sflowPortIgrSampleEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_mirror_sflowPortIgrSampleEnable_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328, 8390
 * Note:
 *      For 8390, the function is backward compatible RTL8328 APIs.
 */
extern int32
rtk_mirror_sflowPortIgrSampleEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_mirror_sflowPortIgrSampleEnable_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390
 * Note:
 *      (1) The valid range of rate is 0~0xFFFF in 8328.
 *      (2) The function is disabled if the rate is set to 0 in 8390.
 */
extern int32
rtk_mirror_sflowPortIgrSampleRate_get(uint32 unit, rtk_port_t port, uint32 *pRate);

/* Function Name:
 *      rtk_mirror_sflowPortIgrSampleEnable_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8328, 8390
 * Note:
 *      (1) The valid range of rate is 0~0xFFFF in 8328.
 *      (2) The function is disabled if the rate is set to 0 in 8390.
 */
extern int32
rtk_mirror_sflowPortIgrSampleRate_set(uint32 unit, rtk_port_t port, uint32 rate);

/* Function Name:
 *      rtk_mirror_sflowPortEgrSampleEnable_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390
 * Note:
 *      For 8390, the function is backward compatible RTL8328 APIs.
 */
extern int32
rtk_mirror_sflowPortEgrSampleEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_mirror_sflowPortEgrSampleEnable_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328, 8390
 * Note:
 *      For 8390, the function is backward compatible RTL8328 APIs.
 */
extern int32
rtk_mirror_sflowPortEgrSampleEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_mirror_sflowPortEgrSampleEnable_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390
 * Note:
 *      (1) The valid range of rate is 0~0xFFFF in 8328.
 *      (2) The function is disabled if the rate is set to 0 in 8390.
 */
extern int32
rtk_mirror_sflowPortEgrSampleRate_get(uint32 unit, rtk_port_t port, uint32 *pRate);

/* Function Name:
 *      rtk_mirror_sflowPortEgrSampleRate_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8328, 8390
 * Note:
 *      (1) The valid range of rate is 0~0xFFFF in 8328.
 *      (2) The function is disabled if the rate is set to 0 in 8390.
 */
extern int32
rtk_mirror_sflowPortEgrSampleRate_set(uint32 unit, rtk_port_t port, uint32 rate);

/* Function Name:
 *      rtk_mirror_sflowAddCPUTagEnable_get
 * Description:
 *      Get enable status of CPU tag adding for trapped packet of SFLOW.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of CPU tag adding
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
rtk_mirror_sflowAddCPUTagEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_mirror_sflowAddCPUTagEnable_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
extern int32
rtk_mirror_sflowAddCPUTagEnable_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      rtk_mirror_sflowSampleCtrl_get
 * Description:
 *      Get sampling preference when a packet is both ingress and egress sampled.
 * Input:
 *      unit  - unit id
 * Output:
 *      pCtrl - pointer to sampling preference
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      The API indicate which sampling to take if a packet is both ingress and egress sampled.
 */
extern int32
rtk_mirror_sflowSampleCtrl_get(uint32 unit, rtk_sflowSampleCtrl_t *pCtrl);

/* Function Name:
 *      rtk_mirror_sflowSampleCtrl_set
 * Description:
 *      Set sampling preference when a packet is both ingress and egress sampled.
 * Input:
 *      unit - unit id
 *      ctrl - sampling preference
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
 *      The API indicate which sampling to take if a packet is both ingress and egress sampled.
 */
extern int32
rtk_mirror_sflowSampleCtrl_set(uint32 unit, rtk_sflowSampleCtrl_t ctrl);

#endif /* __RTK_MIRROR_H__ */

