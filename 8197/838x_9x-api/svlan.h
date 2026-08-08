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


#ifndef __RTK_SVLAN_H__
#define __RTK_SVLAN_H__


/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/port.h>
#include <rtk/vlan.h>


/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */


/*
 * Macro Declaration
 */
typedef struct rtk_svlan_data_s
{
    uint32          idx;
    rtk_vlan_t      svid;
    rtk_portmask_t  member_portmask;
} rtk_svlan_data_t;

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
extern int32
rtk_svlan_init(uint32 unit);


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
extern int32
rtk_svlan_create(uint32 unit, rtk_vlan_t svid);


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
extern int32
rtk_svlan_destroy(uint32 unit, rtk_vlan_t svid);


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
extern int32
rtk_svlan_portSvid_get(uint32 unit, rtk_port_t port, rtk_vlan_t *pSvid);


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
extern int32
rtk_svlan_portSvid_set(uint32 unit, rtk_port_t port, rtk_vlan_t svid);


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
extern int32
rtk_svlan_servicePort_add(uint32 unit, rtk_port_t port);


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
extern int32
rtk_svlan_servicePort_del(uint32 unit, rtk_port_t port);


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
extern int32
rtk_svlan_servicePort_get(uint32 unit, rtk_portmask_t *pSvlan_portmask);


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
extern int32
rtk_svlan_servicePort_set(uint32 unit, rtk_portmask_t *pSvlan_portmask);


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
extern int32
rtk_svlan_memberPort_add(uint32 unit, rtk_vlan_t svid, rtk_port_t port);


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
extern int32
rtk_svlan_memberPort_del(uint32 unit, rtk_vlan_t svid, rtk_port_t port);


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
extern int32
rtk_svlan_memberPort_get(uint32 unit, rtk_vlan_t svid, rtk_portmask_t *pSvlan_portmask);


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
extern int32
rtk_svlan_memberPort_set(uint32 unit, rtk_vlan_t svid, rtk_portmask_t *pSvlan_portmask);


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
extern int32
rtk_svlan_memberPortEntry_get(
    uint32         unit,
    uint32         svid_idx,
    rtk_vlan_t     *pSvid,
    rtk_portmask_t *pSvlan_portmask);


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
extern int32
rtk_svlan_memberPortEntry_set(
    uint32         unit,
    uint32         svid_idx,
    rtk_vlan_t     svid,
    rtk_portmask_t *pSvlan_portmask);


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
extern int32
rtk_svlan_nextValidMemberPortEntry_get(
    uint32         unit,
    int32          *pSvid_idx,
    rtk_vlan_t     *pSvid,
    rtk_portmask_t *pSvlan_portmask);


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
extern int32
rtk_svlan_tpidEntry_get(uint32 unit, uint32 svlan_index, uint32 *pSvlan_tag_id);


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
extern int32
rtk_svlan_tpidEntry_set(uint32 unit, uint32 svlan_index, uint32 svlan_tag_id);


#endif /* __RTK_SVLAN_H__ */
