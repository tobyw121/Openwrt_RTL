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
 * $Revision: 32753 $
 * $Date: 2012-09-17 16:28:18 +0800 (Mon, 17 Sep 2012) $
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

/*
 * Include Files
 */
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <dal/dal_mgmt.h>
#include <rtk/default.h>
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
int32
rtk_mirror_init(uint32 unit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_init(unit);
} /* end of rtk_mirror_init */

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
int32
rtk_mirror_portBased_create(uint32 unit, rtk_port_t mirroring_port)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_portBased_create(unit, mirroring_port);
} /* end of rtk_mirror_portBased_create */

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
int32
rtk_mirror_portBased_destroy(uint32 unit, rtk_port_t mirroring_port)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_portBased_destroy(unit, mirroring_port);
} /* end of rtk_mirror_portBased_destroy */

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
int32
rtk_mirror_portBased_destroyAll(uint32 unit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_portBased_destroyAll(unit);
} /* end of rtk_mirror_portBased_destroyAll */

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
int32
rtk_mirror_portBased_get(
    uint32          unit,
    rtk_port_t      mirroring_port,
    rtk_portmask_t  *pMirrored_rx_portmask,
    rtk_portmask_t  *pMirrored_tx_portmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_portBased_get(unit, mirroring_port, pMirrored_rx_portmask, pMirrored_tx_portmask);
} /* end of rtk_mirror_portBased_get */

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
int32
rtk_mirror_portBased_set(
    uint32          unit,
    rtk_port_t      mirroring_port,
    rtk_portmask_t  *pMirrored_rx_portmask,
    rtk_portmask_t  *pMirrored_tx_portmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_portBased_set(unit, mirroring_port, pMirrored_rx_portmask, pMirrored_tx_portmask);
} /* end of rtk_mirror_portBased_set */

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
int32
rtk_mirror_group_init(
    uint32              unit,
    rtk_mirror_entry_t  *pMirrorEntry)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_group_init(unit, pMirrorEntry);
} /* end of rtk_mirror_group_init */

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
int32
rtk_mirror_group_get(
    uint32              unit,
    uint32              mirror_id,
    rtk_mirror_entry_t  *pMirrorEntry)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32   ret;
    rtk_mirror_entry_t  temp_mirrorEntry;

    if ((ret = RT_MAPPER(unit)->mirror_group_get(unit, mirror_id, &temp_mirrorEntry)) != RT_ERR_OK)
        return ret;
    *pMirrorEntry = temp_mirrorEntry;
    (*pMirrorEntry).mirroring_port = PHYSICAL_PORT_TO_RTK_PORT(unit, temp_mirrorEntry.mirroring_port);
    (*pMirrorEntry).mirrored_igrPorts = PHYSICAL_PORTMASK_TO_RTK_PORTMASK(unit, temp_mirrorEntry.mirrored_igrPorts);
    (*pMirrorEntry).mirrored_egrPorts = PHYSICAL_PORTMASK_TO_RTK_PORTMASK(unit, temp_mirrorEntry.mirrored_egrPorts);
    return ret;
    }
#else
    return RT_MAPPER(unit)->mirror_group_get(unit, mirror_id, pMirrorEntry);
#endif
} /* end of rtk_mirror_group_get */

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
int32
rtk_mirror_group_set(
    uint32              unit,
    uint32              mirror_id,
    rtk_mirror_entry_t  *pMirrorEntry)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    rtk_mirror_entry_t  temp_mirrorEntry;

    temp_mirrorEntry = *pMirrorEntry;
    temp_mirrorEntry.mirroring_port = RTK_PORT_TO_PHYSICAL_PORT(unit, (*pMirrorEntry).mirroring_port);
    temp_mirrorEntry.mirrored_igrPorts = RTK_PORTMASK_TO_PHYSICAL_PORTMASK(unit, (*pMirrorEntry).mirrored_igrPorts);
    temp_mirrorEntry.mirrored_egrPorts = RTK_PORTMASK_TO_PHYSICAL_PORTMASK(unit, (*pMirrorEntry).mirrored_egrPorts);
    return RT_MAPPER(unit)->mirror_group_set(unit, mirror_id, &temp_mirrorEntry);
    }
#else
    return RT_MAPPER(unit)->mirror_group_set(unit, mirror_id, pMirrorEntry);
#endif
} /* end of rtk_mirror_group_set */

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
int32
rtk_mirror_rspanIgrMode_get(uint32 unit, uint32 mirror_id, rtk_mirror_rspanIgrMode_t *pIgrMode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_rspanIgrMode_get(unit, mirror_id, pIgrMode);
} /* end of rtk_mirror_rspanIgrMode_get */

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
int32
rtk_mirror_rspanIgrMode_set(uint32 unit, uint32 mirror_id, rtk_mirror_rspanIgrMode_t igrMode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_rspanIgrMode_set(unit, mirror_id, igrMode);
} /* end of rtk_mirror_rspanIgrMode_set */

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
int32
rtk_mirror_rspanEgrMode_get(uint32 unit, uint32 mirror_id, rtk_mirror_rspanEgrMode_t *pEgrMode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_rspanEgrMode_get(unit, mirror_id, pEgrMode);
} /* rtk_mirror_rspanEgrMode_get */

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
int32
rtk_mirror_rspanEgrMode_set(uint32 unit, uint32 mirror_id, rtk_mirror_rspanEgrMode_t egrMode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_rspanEgrMode_set(unit, mirror_id, egrMode);
} /* rtk_mirror_rspanEgrMode_set */

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
int32
rtk_mirror_egrMode_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_mirror_egrMode_t    *pEgrMode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_egrMode_get(unit, port, pEgrMode);
} /* end of rtk_mirror_egrMode_get */

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
int32
rtk_mirror_egrMode_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_mirror_egrMode_t    egrMode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_egrMode_set(unit, port, egrMode);
} /* end of rtk_mirror_egrMode_set */

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
int32
rtk_mirror_portRspanIgrMode_get(uint32 unit, rtk_port_t port, rtk_mirror_rspanIgrMode_t *pIgrMode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_portRspanIgrMode_get(unit, port, pIgrMode);
} /* end of rtk_mirror_portRspanIgrMode_get */

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
int32
rtk_mirror_portRspanIgrMode_set(uint32 unit, rtk_port_t port, rtk_mirror_rspanIgrMode_t igrMode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_portRspanIgrMode_set(unit, port, igrMode);
} /* end of rtk_mirror_portRspanIgrMode_set */

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
int32
rtk_mirror_portRspanEgrMode_get(uint32 unit, rtk_port_t port, rtk_mirror_rspanEgrMode_t *pEgrMode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_portRspanEgrMode_get(unit, port, pEgrMode);
} /* end of rtk_mirror_portRspanEgrMode_get */

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
int32
rtk_mirror_portRspanEgrMode_set(uint32 unit, rtk_port_t port, rtk_mirror_rspanEgrMode_t egrMode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_portRspanEgrMode_set(unit, port, egrMode);
} /* end of rtk_mirror_portRspanEgrMode_set */

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
int32
rtk_mirror_rspanIgrTag_get(uint32 unit, rtk_port_t port, rtk_mirror_rspanIgrTag_t *pIgrTag)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_rspanIgrTag_get(unit, port, pIgrTag);
} /* end of rtk_mirror_rspanIgrTag_get */

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
int32
rtk_mirror_rspanIgrTag_set(uint32 unit, rtk_port_t port, rtk_mirror_rspanIgrTag_t *pIgrTag)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_rspanIgrTag_set(unit, port, pIgrTag);
} /* end of rtk_mirror_rspanIgrTag_set */

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
int32
rtk_mirror_rspanEgrTag_get(uint32 unit, rtk_port_t port, rtk_mirror_rspanEgrTag_t *pEgrTag)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_rspanEgrTag_get(unit, port, pEgrTag);
} /* end of rtk_mirror_rspanEgrTag_get */

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
int32
rtk_mirror_rspanEgrTag_set(uint32 unit, rtk_port_t port, rtk_mirror_rspanEgrTag_t *pEgrTag)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_rspanEgrTag_set(unit, port, pEgrTag);
} /* end of rtk_mirror_rspanEgrTag_set */

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
int32
rtk_mirror_rspanTag_get(uint32 unit, uint32 mirror_id, rtk_mirror_rspanTag_t *pTag)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_rspanTag_get(unit, mirror_id, pTag);
} /* end of rtk_mirror_rspanTag_get */

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
int32
rtk_mirror_rspanTag_set(uint32 unit, uint32 mirror_id, rtk_mirror_rspanTag_t *pTag)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_rspanTag_set(unit, mirror_id, pTag);
} /* end of rtk_mirror_rspanTag_set */

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
int32
rtk_mirror_sflowMirrorSeed_get(uint32 unit, uint32 *pSeed)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_sflowMirrorSeed_get(unit, pSeed);
} /* end of rtk_mirror_sflowMirrorSeed_get */

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
int32
rtk_mirror_sflowMirrorSeed_set(uint32 unit, uint32 seed)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_sflowMirrorSeed_set(unit, seed);
} /* end of rtk_mirror_sflowMirrorSeed_set */

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
int32
rtk_mirror_sflowMirrorSampleEnable_get(uint32 unit, uint32 mirror_id, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_sflowMirrorSampleEnable_get(unit, mirror_id, pEnable);
} /* end of rtk_mirror_sflowMirrorSampleEnable_get */

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
int32
rtk_mirror_sflowMirrorSampleEnable_set(uint32 unit, uint32 mirror_id, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_sflowMirrorSampleEnable_set(unit, mirror_id, enable);
} /* end of rtk_mirror_sflowMirrorSampleEnable_set */

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
int32
rtk_mirror_sflowMirrorSampleRate_get(uint32 unit, uint32 mirror_id, uint32 *pRate)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_sflowMirrorSampleRate_get(unit, mirror_id, pRate);
} /* end of rtk_mirror_sflowMirrorSampleRate_get */

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
int32
rtk_mirror_sflowMirrorSampleRate_set(uint32 unit, uint32 mirror_id, uint32 rate)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_sflowMirrorSampleRate_set(unit, mirror_id, rate);
} /* end of rtk_mirror_sflowMirrorSampleRate_set */

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
int32
rtk_mirror_sflowMirrorSampleStat_get(uint32 unit, uint32 mirror_id, rtk_mirror_sampleStat_t *pStat)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_sflowMirrorSampleStat_get(unit, mirror_id, pStat);
} /* end of rtk_mirror_sflowMirrorSampleStat_get */

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
int32
rtk_mirror_sflowPortSeed_get(uint32 unit, uint32 *pSeed)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_sflowPortSeed_get(unit, pSeed);
} /* end of rtk_mirror_sflowPortSeed_get */

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
int32
rtk_mirror_sflowPortSeed_set(uint32 unit, uint32 seed)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_sflowPortSeed_set(unit, seed);
} /* end of rtk_mirror_sflowPortSeed_set */

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
int32
rtk_mirror_sflowPortIgrSampleEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_sflowPortIgrSampleEnable_get(unit, port, pEnable);
} /* end of rtk_mirror_sflowPortIgrSampleEnable_get */

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
int32
rtk_mirror_sflowPortIgrSampleEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_sflowPortIgrSampleEnable_set(unit, port, enable);
} /* end of rtk_mirror_sflowPortIgrSampleEnable_set */

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
int32
rtk_mirror_sflowPortIgrSampleRate_get(uint32 unit, rtk_port_t port, uint32 *pRate)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_sflowPortIgrSampleRate_get(unit, port, pRate);
} /* end of rtk_mirror_sflowPortIgrSampleRate_get */

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
int32
rtk_mirror_sflowPortIgrSampleRate_set(uint32 unit, rtk_port_t port, uint32 rate)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_sflowPortIgrSampleRate_set(unit, port, rate);
} /* end of rtk_mirror_sflowPortIgrSampleRate_set */

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
int32
rtk_mirror_sflowPortEgrSampleEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_sflowPortEgrSampleEnable_get(unit, port, pEnable);
} /* end of rtk_mirror_sflowPortEgrSampleEnable_get */

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
int32
rtk_mirror_sflowPortEgrSampleEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_sflowPortEgrSampleEnable_set(unit, port, enable);
} /* end of rtk_mirror_sflowPortEgrSampleEnable_set */

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
int32
rtk_mirror_sflowPortEgrSampleRate_get(uint32 unit, rtk_port_t port, uint32 *pRate)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_sflowPortEgrSampleRate_get(unit, port, pRate);
} /* end of rtk_mirror_sflowPortEgrSampleRate_get */

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
int32
rtk_mirror_sflowPortEgrSampleRate_set(uint32 unit, rtk_port_t port, uint32 rate)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_sflowPortEgrSampleRate_set(unit, port, rate);
} /* end of rtk_mirror_sflowPortEgrSampleRate_set */

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
int32
rtk_mirror_sflowAddCPUTagEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_sflowAddCPUTagEnable_get(unit, pEnable);
} /* end of rtk_mirror_sflowAddCPUTagEnable_get */

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
int32
rtk_mirror_sflowAddCPUTagEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_sflowAddCPUTagEnable_set(unit, enable);
} /* end of rtk_mirror_sflowAddCPUTagEnable_set */

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
int32
rtk_mirror_sflowSampleCtrl_get(uint32 unit, rtk_sflowSampleCtrl_t *pCtrl)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_sflowSampleCtrl_get(unit, pCtrl);
} /* end of rtk_mirror_sflowSampleCtrl_get */

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
int32
rtk_mirror_sflowSampleCtrl_set(uint32 unit, rtk_sflowSampleCtrl_t ctrl)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->mirror_sflowSampleCtrl_set(unit, ctrl);
} /* end of rtk_mirror_sflowSampleCtrl_set */
