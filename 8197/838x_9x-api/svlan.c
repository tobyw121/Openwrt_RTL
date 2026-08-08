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
 * $Revision: 20153 $
 * $Date: 2011-07-29 09:48:05 +0800 (Fri, 29 Jul 2011) $
 *
 * Purpose : Definition of SVLAN API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) 802.1ad, SVLAN [VLAN Stacking] 
 *
 */

/*
 * Include Files
 */
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <dal/dal_mgmt.h>
#include <rtk/default.h>
#include <rtk/svlan.h>
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

/* Module Name : SVLAN */

/* Function Name:
 *      rtk_svlan_init
 * Description:
 *      Initialize svlan module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Applicable:
 *      8389
 * Note:
 *      Must initialize svlan module before calling any svlan APIs.
 */
int32
rtk_svlan_init(uint32 unit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->svlan_init(unit);
} /* end of rtk_svlan_init */


/* Function Name:
 *      rtk_svlan_create
 * Description:
 *      Create the svlan in the specified device.
 * Input:
 *      unit - unit id
 *      svid - svlan id to be created
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_SVLAN_EXIST - SVLAN entry is exist
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid range of svid is 0~4095
 */
int32
rtk_svlan_create(uint32 unit, rtk_vlan_t svid)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->svlan_create(unit, svid);
} /* end of rtk_svlan_create */


/* Function Name:
 *      rtk_svlan_destroy
 * Description:
 *      Destroy the svlan in the specified device.
 * Input:
 *      unit - unit id
 *      svid - svlan id to be destroyed
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - invalid unit id
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND - specified svlan entry not found
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid range of svid is 0~4095
 */
int32
rtk_svlan_destroy(uint32 unit, rtk_vlan_t svid)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->svlan_destroy(unit, svid);
} /* end of rtk_svlan_destroy */


/* Function Name:
 *      rtk_svlan_portSvid_get
 * Description:
 *      Get port default svlan id from the specified device.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pSvid - pointer buffer of port default svlan id
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid range of svid is 0~4095
 */
int32
rtk_svlan_portSvid_get(uint32 unit, rtk_port_t port, rtk_vlan_t *pSvid)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->svlan_portSvid_get(unit, port, pSvid);
} /* end of rtk_svlan_portSvid_get */


/* Function Name:
 *      rtk_svlan_portSvid_set
 * Description:
 *      Set port default svlan id to the specified device.
 * Input:
 *      unit - unit id
 *      port - port id
 *      svid - port default svlan id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_PORT_ID - invalid port id
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid range of svid is 0~4095
 */
int32
rtk_svlan_portSvid_set(uint32 unit, rtk_port_t port, rtk_vlan_t svid)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->svlan_portSvid_set(unit, port, svid);
} /* end of rtk_svlan_portSvid_set */


/* Function Name:
 *      rtk_svlan_servicePort_add
 * Description:
 *      Enable one service port in the specified device.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Applicable:
 *      8389
 * Note:
 *      None
 */
int32
rtk_svlan_servicePort_add(uint32 unit, rtk_port_t port)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->svlan_servicePort_add(unit, port);
} /* end of rtk_svlan_servicePort_add */


/* Function Name:
 *      rtk_svlan_servicePort_del
 * Description:
 *      Disable one service port in the specified device.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Applicable:
 *      8389
 * Note:
 *      None
 */
int32
rtk_svlan_servicePort_del(uint32 unit, rtk_port_t port)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->svlan_servicePort_del(unit, port);
} /* end of rtk_svlan_servicePort_del */


/* Function Name:
 *      rtk_svlan_servicePort_get
 * Description:
 *      Get service ports from the specified device.
 * Input:
 *      unit            - unit id
 * Output:
 *      pSvlan_portmask - pointer buffer of svlan ports
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389
 * Note:
 *      None
 */
int32
rtk_svlan_servicePort_get(uint32 unit, rtk_portmask_t *pSvlan_portmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->svlan_servicePort_get(unit, pSvlan_portmask);
} /* end of rtk_svlan_servicePort_get */


/* Function Name:
 *      rtk_svlan_servicePort_set
 * Description:
 *      Set service ports to the specified device.
 * Input:
 *      unit            - unit id
 *      pSvlan_portmask - svlan ports
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Applicable:
 *      8389
 * Note:
 *      None
 */
int32
rtk_svlan_servicePort_set(uint32 unit, rtk_portmask_t *pSvlan_portmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->svlan_servicePort_set(unit, pSvlan_portmask);
} /* end of rtk_svlan_servicePort_set */


/* Function Name:
 *      rtk_svlan_memberPort_add
 * Description:
 *      Add one member of the svlan to the specified device.
 * Input:
 *      unit - unit id
 *      svid - svlan id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid range of svid is 0~4095
 *      (2) svlan portmask only for svlan ingress filter checking
 */
int32
rtk_svlan_memberPort_add(uint32 unit, rtk_vlan_t svid, rtk_port_t port)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->svlan_memberPort_add(unit, svid, port);
} /* end of rtk_svlan_memberPort_add */


/* Function Name:
 *      rtk_svlan_memberPort_del
 * Description:
 *      Delete one member of the svlan from the specified device.
 * Input:
 *      unit - unit id
 *      svid - svlan id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - invalid unit id
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND - specified svlan entry not found
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid range of svid is 0~4095
 *      (2) svlan portmask only for svlan ingress filter checking
 */
int32
rtk_svlan_memberPort_del(uint32 unit, rtk_vlan_t svid, rtk_port_t port)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->svlan_memberPort_del(unit, svid, port);
} /* end of rtk_svlan_memberPort_del */


/* Function Name:
 *      rtk_svlan_memberPort_get
 * Description:
 *      Get the svlan members from the specified device.
 * Input:
 *      unit            - unit id
 *      svid            - svlan id
 * Output:
 *      pSvlan_portmask - pointer buffer of svlan member ports
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - invalid unit id
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND - specified svlan entry not found
 *      RT_ERR_NULL_POINTER          - input parameter may be null pointer
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid range of svid is 0~4095
 *      (2) svlan portmask only for svlan ingress filter checking
 */
int32
rtk_svlan_memberPort_get(uint32 unit, rtk_vlan_t svid, rtk_portmask_t *pSvlan_portmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->svlan_memberPort_get(unit, svid, pSvlan_portmask);
} /* end of rtk_svlan_memberPort_get */


/* Function Name:
 *      rtk_svlan_memberPort_set
 * Description:
 *      Replace the svlan members in the specified device.
 * Input:
 *      unit            - unit id
 *      svid            - svlan id
 *      pSvlan_portmask - svlan member ports
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - invalid unit id
 *      RT_ERR_SVLAN_ENTRY_INDEX     - invalid svid entry no
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND - specified svlan entry not found
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid range of svid is 0~4095
 *      (2) Don't care the original svlan members and replace with new configure
 *          directly.
 *      (3) svlan portmask only for svlan ingress filter checking
 */
int32
rtk_svlan_memberPort_set(uint32 unit, rtk_vlan_t svid, rtk_portmask_t *pSvlan_portmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->svlan_memberPort_set(unit, svid, pSvlan_portmask);
} /* end of rtk_svlan_memberPort_set */


/* Function Name:
 *      rtk_svlan_memberPortEntry_get
 * Description:
 *      Get the svlan id and members by svlan table index from the specified device.
 * Input:
 *      unit            - unit id
 *      svid_idx        - svlan table index
 * Output:
 *      pSvid           - pointer buffer of svlan id
 *      pSvlan_portmask - pointer buffer of svlan member ports
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_SVLAN_ENTRY_INDEX - invalid svid entry no
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid range of svid_idx is 0~63
 *      (2) The valid range of svid is 0~4095
 *      (3) svlan portmask only for svlan ingress filter checking
 */
int32
rtk_svlan_memberPortEntry_get(
    uint32         unit,
    uint32         svid_idx,
    rtk_vlan_t     *pSvid,
    rtk_portmask_t *pSvlan_portmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->svlan_memberPortEntry_get(unit, svid_idx, pSvid, pSvlan_portmask);
} /* end of rtk_svlan_memberPortEntry_get */


/* Function Name:
 *      rtk_svlan_memberPortEntry_set
 * Description:
 *      Set the svlan id and members by svlan table index to the specified device.
 * Input:
 *      unit            - unit id
 *      svid_idx        - svlan table index
 *      svid            - svlan id
 *      pSvlan_portmask - svlan member ports
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_SVLAN_ENTRY_INDEX - invalid svid entry no
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid range of svid_idx is 0~63
 *      (2) The valid range of svid is 0~4095
 *      (3) svlan portmask only for svlan ingress filter checking
 */
int32
rtk_svlan_memberPortEntry_set(
    uint32         unit,
    uint32         svid_idx,
    rtk_vlan_t     svid,
    rtk_portmask_t *pSvlan_portmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->svlan_memberPortEntry_set(unit, svid_idx, svid, pSvlan_portmask);
} /* end of rtk_svlan_memberPortEntry_set */


/* Function Name:
 *      rtk_svlan_nextValidMemberPortEntry_get
 * Description:
 *      Get next valid svlan table entry from the specified device.
 * Input:
 *      unit            - unit id
 *      pSvid_idx       - input svlan table index for get next
 * Output:
 *      pSvid           - pointer buffer of svlan id
 *      pSvlan_portmask - pointer buffer of svlan member ports
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_SVLAN_ENTRY_INDEX - invalid svid entry no
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 * Applicable:
 *      8389
 * Note:
 *      (1) svlan portmask only for svlan ingress filter checking
 *      (2) Please input -1 for get the first entry of svlan table.
 *      (3) The pSvid_idx is the input and output key BOTH.
 */
int32
rtk_svlan_nextValidMemberPortEntry_get(
    uint32         unit,
    int32          *pSvid_idx,
    rtk_vlan_t     *pSvid,
    rtk_portmask_t *pSvlan_portmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->svlan_nextValidMemberPortEntry_get(unit, pSvid_idx, pSvid, pSvlan_portmask);
} /* end of rtk_svlan_nextValidMemberPortEntry_get */


/* Function Name:
 *      rtk_svlan_tpidEntry_get
 * Description:
 *      Get the svlan TPID from the specified device.
 * Input:
 *      unit          - unit id
 *      svlan_index   - index of svlan table
 * Output:
 *      pSvlan_tag_id - pointer buffer of svlan TPID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid svlan_index is 0 in 8389
 *      (2) The default svlan TPID is 0x88A8
 */
int32
rtk_svlan_tpidEntry_get(uint32 unit, uint32 svlan_index, uint32 *pSvlan_tag_id)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->svlan_tpidEntry_get(unit, svlan_index, pSvlan_tag_id);
} /* end of rtk_svlan_tpidEntry_get */


/* Function Name:
 *      rtk_svlan_tpidEntry_set
 * Description:
 *      Set the svlan TPID to the specified device.
 * Input:
 *      unit         - unit id
 *      svlan_index  - index of svlan table
 *      svlan_tag_id - svlan TPID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid svlan_index is 0 in 8389
 *      (2) The default svlan TPID is 0x88A8
 */
int32
rtk_svlan_tpidEntry_set(uint32 unit, uint32 svlan_index, uint32 svlan_tag_id)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->svlan_tpidEntry_set(unit, svlan_index, svlan_tag_id);
} /* end of rtk_svlan_tpidEntry_set */
