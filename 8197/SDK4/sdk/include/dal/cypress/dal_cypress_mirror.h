/*
 * Copyright(c) Realtek Semiconductor Corporation, 2009
 * All rights reserved.
 *
 * $Revision: 32753 $
 * $Date: 2012-09-17 16:28:18 +0800 (Mon, 17 Sep 2012) $
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

#ifndef __DAL_CYPRESS_MIRROR_H__
#define __DAL_CYPRESS_MIRROR_H__


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
 *      dal_cypress_mirror_init
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
dal_cypress_mirror_init(uint32 unit);


/* Module Name    : Mirror             */
/* Sub-module Name: Group-based mirror */

/* Function Name:
 *      dal_cypress_mirror_group_init
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
 *      RT_ERR_MIRROR_ID        - invalid mirror id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_mirror_group_init(
    uint32              unit,
    rtk_mirror_entry_t  *pMirrorEntry);

/* Function Name:
 *      dal_cypress_mirror_group_get
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
 *      RT_ERR_MIRROR_ID        - invalid mirror id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_mirror_group_get(
    uint32              unit,
    uint32              mirror_id,
    rtk_mirror_entry_t  *pMirrorEntry);

/* Function Name:
 *      dal_cypress_mirror_group_set
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
 *      RT_ERR_NOT_INIT  - The module is not initial
 *      RT_ERR_MIRROR_ID - invalid mirror id
 *      RT_ERR_PORT_ID   - invalid mirroring port id
 *      RT_ERR_INPUT     - invalid input parameter
 *      RT_ERR_MIRROR_DP_IN_SPM_DPM - mirroring port can not be in ingress or egress mirrored portmask of any mirroring set
 * Note:
 *      None
 */
extern int32
dal_cypress_mirror_group_set(
    uint32              unit,
    uint32              mirror_id,
    rtk_mirror_entry_t  *pMirrorEntry);


/* Module Name    : Mirror */
/* Sub-module Name: RSPAN  */
/* Function Name:
 *      dal_cypress_mirror_rspanIgrMode_get
 * Description:
 *      Get ingress mode of RSPAN on specified mirror group.
 * Input:
 *      unit        - unit id
 *      mirror_id   - mirror id
 * Output:
 *      pIgrMode - pointer to ingress mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Ingress mode is as following:
 *      - RSPAN_IGR_HANDLE_RSPAN_TAG
 *      - RSPAN_IGR_IGNORE_RSPAN_TAG
 */
extern int32
dal_cypress_mirror_rspanIgrMode_get(uint32 unit, uint32 mirror_id, rtk_mirror_rspanIgrMode_t *pIgrMode);

/* Function Name:
 *      dal_cypress_mirror_rspanIgrMode_set
 * Description:
 *      Set ingress mode of RSPAN on specified mirror group.
 * Input:
 *      unit    - unit id
 *      mirror_id    - mirror id
 *      igrMode - ingress mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Ingress mode is as following:
 *      - RSPAN_IGR_HANDLE_RSPAN_TAG
 *      - RSPAN_IGR_IGNORE_RSPAN_TAG
 */
extern int32
dal_cypress_mirror_rspanIgrMode_set(uint32 unit, uint32 mirror_id, rtk_mirror_rspanIgrMode_t igrMode);

/* Function Name:
 *      dal_cypress_mirror_rspanEgrMode_get
 * Description:
 *      Get egress mode of RSPAN on specified mirror group.
 * Input:
 *      unit     - unit id
 *      mirror_id     - mirror id
 * Output:
 *      pEgrMode - pointer to egress mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Ingress mode is as following:
 *      - RSPAN_EGR_REMOVE_TAG
 *      - RSPAN_EGR_ADD_TAG
 *      - RSPAN_EGR_NO_MODIFY
 */
extern int32
dal_cypress_mirror_rspanEgrMode_get(uint32 unit, uint32 mirror_id, rtk_mirror_rspanEgrMode_t *pEgrMode);

/* Function Name:
 *      dal_cypress_mirror_rspanEgrMode_set
 * Description:
 *      Set egress mode of RSPAN on specified mirror group.
 * Input:
 *      unit    - unit id
 *      mirror_id    - mirror id
 *      egrMode - egress mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT  - The module is not initial
 *      RT_ERR_INPUT     - invalid input parameter
 *      RT_ERR_MIRROR_ID - invalid mirror ID
 * Note:
 *      Ingress mode is as following:
 *      - RSPAN_EGR_HANDLE_RSPAN_TAG
 *      - RSPAN_EGR_IGNORE_RSPAN_TAG
 */
extern int32
dal_cypress_mirror_rspanEgrMode_set(uint32 unit, uint32 mirror_id, rtk_mirror_rspanEgrMode_t egrMode);

/* Function Name:
 *      dal_cypress_mirror_rspanTag_get
 * Description:
 *      Get content of RSPAN tag on specified mirror group.
 * Input:
 *      unit    - unit id
 *      mirror_id    - mirror id
 * Output:
 *      pTag - pointer to content of RSPAN tag
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_mirror_rspanTag_get(uint32 unit, uint32 mirror_id, rtk_mirror_rspanTag_t *pTag);

/* Function Name:
 *      dal_cypress_mirror_rspanTag_set
 * Description:
 *      Set content of RSPAN tag on specified mirroring group.
 * Input:
 *      unit    - unit id
 *      mirror_id    - mirror id
 *      pTag - content of RSPAN tag
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_MIRROR_ID    - invalid mirror ID
 *      RT_ERR_PRIORITY     - invalid priority
 *      RT_ERR_VLAN_VID     - invalid vid
 * Note:
 *      None
 */
extern int32
dal_cypress_mirror_rspanTag_set(uint32 unit, uint32 mirror_id, rtk_mirror_rspanTag_t *pTag);

/* Module Name    : Mirror             */
/* Sub-module Name: Mirror-based SFLOW */

/* Function Name:
 *      dal_cypress_mirror_sflowMirrorSampleRate_get
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
 *      RT_ERR_MIRROR_ID        - invalid mirror id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_mirror_sflowMirrorSampleRate_get(uint32 unit, uint32 mirror_id, uint32 *pRate);

/* Function Name:
 *      dal_cypress_mirror_sflowMirrorSampleRate_set
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
 *      RT_ERR_MIRROR_ID        - invalid mirror id
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 * Note:
 *      None
 */
extern int32
dal_cypress_mirror_sflowMirrorSampleRate_set(uint32 unit, uint32 mirror_id,
                                             uint32 rate);

/* Module Name    : Mirror           */
/* Sub-module Name: Port-based SFLOW */

/* Function Name:
 *      dal_cypress_mirror_sflowSampleCtrl_get
 * Description:
 *      Get sampling control status for simple hit ingress and egress
 *      which direction  take hit at first.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pCtrl - pointer to sampling preference
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_mirror_sflowSampleCtrl_get(uint32 unit, rtk_sflowSampleCtrl_t *pCtrl);

/* Function Name:
 *      dal_cypress_mirror_sflowSampleCtrl_set
 * Description:
 *      Set sampling control status for simple hit ingress and egress
 *      which direction  take hit at first.
 * Input:
 *      unit - unit id
 *      rate - status of sampling preference
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_cypress_mirror_sflowSampleCtrl_set(uint32 unit, rtk_sflowSampleCtrl_t ctrl);

/* Function Name:
 *      dal_cypress_mirror_sflowPortIgrSampleRate_get
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
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_mirror_sflowPortIgrSampleRate_get(uint32 unit, rtk_port_t port,
                                              uint32 *pRate);

/* Function Name:
 *      dal_cypress_mirror_sflowPortIgrSampleRate_set
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
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_cypress_mirror_sflowPortIgrSampleRate_set(uint32 unit, rtk_port_t port,
                                              uint32 rate);

/* Function Name:
 *      dal_cypress_mirror_sflowPortEgrSampleRate_get
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
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_cypress_mirror_sflowPortEgrSampleRate_get(uint32 unit, rtk_port_t port,
                                              uint32 *pRate);

/* Function Name:
 *      dal_cypress_mirror_sflowPortEgrSampleRate_set
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
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_cypress_mirror_sflowPortEgrSampleRate_set(uint32 unit, rtk_port_t port,
                                              uint32 rate);

#if defined(CONFIG_SDK_DRIVER_RTK_BACKWARD_COMPATIBLE_8328)
/*
 * For some feature in 8390 can full compatible the original 8328 API case,
 * SDK provide those backward compatible RTL8328 APIs for customer.
 * If customer only need the original 8328 feature, can use those API and nothing to change.
 * If customer want to use the enhance 8390 feature, need to call the new 8390 APIs.
 */

/* Function Name:
 *      dal_cypress_mirror_sflowMirrorSampleEnable_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_MIRROR_ID    - invalid mirror id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The function is backward compatible RTL8328 APIs.
 */
extern int32
dal_cypress_mirror_sflowMirrorSampleEnable_get(uint32 unit, uint32 mirror_id, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_cypress_mirror_sflowMirrorSampleEnable_set
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
 *      RT_ERR_NOT_INIT  - The module is not initial
 *      RT_ERR_MIRROR_ID - invalid mirror id
 *      RT_ERR_INPUT     - invalid input parameter
 * Note:
 *      The function is backward compatible RTL8328 APIs.
 */
extern int32
dal_cypress_mirror_sflowMirrorSampleEnable_set(uint32 unit, uint32 mirror_id, rtk_enable_t enable);

/* Function Name:
 *      dal_cypress_mirror_sflowPortIgrSampleEnable_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The function is backward compatible RTL8328 APIs.
 */
extern int32
dal_cypress_mirror_sflowPortIgrSampleEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_cypress_mirror_sflowPortIgrSampleEnable_set
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
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      The function is backward compatible RTL8328 APIs.
 */
extern int32
dal_cypress_mirror_sflowPortIgrSampleEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_cypress_mirror_sflowPortEgrSampleEnable_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The function is backward compatible RTL8328 APIs.
 */
extern int32
dal_cypress_mirror_sflowPortEgrSampleEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_cypress_mirror_sflowPortEgrSampleEnable_set
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
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      The function is backward compatible RTL8328 APIs.
 */
extern int32
dal_cypress_mirror_sflowPortEgrSampleEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

#endif
#endif /* __DAL_CYPRESS_MIRROR_H__ */


