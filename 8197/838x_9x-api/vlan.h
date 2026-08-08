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
 * $Revision: 52260 $
 * $Date: 2014-10-17 14:14:26 +0800 (Fri, 17 Oct 2014) $
 *
 * Purpose : Definition of VLAN API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Vlan table configure and modification
 *           (2) Accept frame type
 *           (3) Vlan ingress/egress filter
 *           (4) Port based and protocol based vlan
 *           (5) TPID configuration
 *           (6) Ingress tag handling
 *           (7) Tag format handling
 *
 */

#ifndef __RTK_VLAN_H__
#define __RTK_VLAN_H__

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/port.h>
#include <rtk/l2.h>

/*
 * Symbol Definition
 */

/* acceptable frame type of 802.1Q and 802.1ad */
typedef enum rtk_vlan_acceptFrameType_e
{
    ACCEPT_FRAME_TYPE_ALL = 0,          /* untagged, priority-tagged and tagged  */
    ACCEPT_FRAME_TYPE_TAG_ONLY,         /* tagged                                */
    ACCEPT_FRAME_TYPE_UNTAG_ONLY,       /* untagged and priority-tagged          */
    ACCEPT_FRAME_TYPE_END
} rtk_vlan_acceptFrameType_t;

/* tagged mode of VLAN - reference realtek private specification */
typedef enum rtk_vlan_tagMode_e
{
    VLAN_TAG_MODE_ORIGINAL = 0,
    VLAN_TAG_MODE_KEEP_FORMAT,
    VLAN_TAG_MODE_PRI,
    VLAN_TAG_MODE_END
} rtk_vlan_tagMode_t;

/* packet forwarding mode of vlan*/
typedef enum rtk_vlan_fwdMode_e
{
    VLAN_FWD_ON_ALE = 0,
    VLAN_FWD_ON_VLAN_MEMBER,
    VLAN_FWD_END
} rtk_vlan_fwdMode_t;

/* TPID mode of inner tag at egress.
 * TPID mode will decide what kind of packet should be replaced inner TPID with configured one.
 */
typedef enum rtk_vlan_egrTpidMode_e
{
    EGR_TPID_MODE_IGR_UNTAG_AND_EGR_TAG = 0,
    EGR_TPID_MODE_ALL_PACKETS,
    EGR_TPID_MODE_END
} rtk_vlan_egrTpidMode_t;

/* Source of tag content */
typedef enum rtk_vlan_tagSource_e
{
    TAG_SOURCE_FROM_ALE = 0,
    TAG_SOURCE_FROM_ORIG_INNER_TAG,
    TAG_SOURCE_FROM_ORIG_OUTER_TAG,
    TAG_SOURCE_NULL,
    TAG_SOURCE_END
} rtk_vlan_tagSource_t;

/* Port-based VLAN mode */
typedef enum rtk_vlan_pbVlan_mode_e
{
    PBVLAN_MODE_UNTAG_AND_PRITAG = 0,
    PBVLAN_MODE_UNTAG_ONLY,
    PBVLAN_MODE_ALL_PKT,
    PBVLAN_MODE_END
} rtk_vlan_pbVlan_mode_t;

/* VLAN ingress filtering */
typedef enum rtk_vlan_ifilter_e
{
    INGRESS_FILTER_FWD = ACTION_FORWARD,
    INGRESS_FILTER_DROP = ACTION_DROP,
    INGRESS_FILTER_TRAP = ACTION_TRAP2CPU,
    INGRESS_FILTER_END
} rtk_vlan_ifilter_t;

/* Ingress VLAN conversion block mode */
typedef enum rtk_vlan_igrVlanCnvtBlk_mode_e
{
    CONVERSION_MODE_C2SC = 0,
    CONVERSION_MODE_MAC_BASED,
    CONVERSION_MODE_IP_SUBNET_BASED,
    CONVERSION_MODE_END
} rtk_vlan_igrVlanCnvtBlk_mode_t;


/* Egress tag status */
typedef enum rtk_vlan_tagSts_e
{
    TAG_STATUS_UNTAG = 0,
    TAG_STATUS_TAGGED,
    TAG_STATUS_PRIORITY_TAGGED,
    TAG_STATUS_END
} rtk_vlan_tagSts_t;

/*VLAN tag keep type*/
typedef enum rtk_vlan_tagKeepType_e
{
    TAG_KEEP_TYPE_NOKEEP = 0,
    TAG_KEEP_TYPE_FORMAT,
    TAG_KEEP_TYPE_CONTENT,
    TAG_KEEP_TYPE_END
} rtk_vlan_tagKeepType_t;

/*VLAN Translation Control Lookupmiss Act*/
typedef enum rtk_vlan_lookupMissAct_e
{
    LOOKUPMISS_FWD = ACTION_FORWARD,
    LOOKUPMISS_DROP = ACTION_DROP,
    LOOKUPMISS_END
} rtk_vlan_lookupMissAct_t;

typedef enum rtk_vlan_priTagVidSrc_e
{
    LEARNING_VID_PRI = 0,
    LEARNING_VID_PBASED,
    LEARNING_VID_END
} rtk_vlan_priTagVidSrc_t;

typedef enum rtk_vlan_l2tblCnvtUnknAct_e
{
    UNKN_TFC_NMLFWD = 0,
    UNKN_TFC_UNTAG,
    UNKN_TFC_PRITAG,
    UNKN_TFC_PBASEDOVERWT,
    UNKN_TFC_END
} rtk_vlan_l2tblCnvtUnknAct_t;

typedef enum rtk_vlan_l2mactype_e
{
    L2MACTYPE_UC = 0,
    L2MACTYPE_MC,
    L2MACTYPE_BC,
    L2MACTYPE_END
} rtk_vlan_l2mactype_t;

typedef enum rtk_vlan_tagType_e
{
    VLAN_TAG_TYPE_INNER,
    VLAN_TAG_TYPE_OUTER,
    VLAN_TAG_TYPE_EXTRA,
    VLAN_TAG_TYPE_END
} rtk_vlan_tagType_t;

typedef struct rtk_vlan_egrVlanCnvtRangeCheck_vid_s
{
    uint32              vid_upper_bound;    /* VID range upper bound */
    uint32              vid_lower_bound;    /* VID range lower bound */
    uint32              reverse;            /* reverse operation */
    rtk_vlan_tagType_t  vid_type;
} rtk_vlan_egrVlanCnvtRangeCheck_vid_t;

/*
 * Data Declaration
 */
/* Content of protocol vlan group */
typedef struct rtk_vlan_protoGroup_s
{
    rtk_vlan_protoVlan_frameType_t  frametype;
    uint32                          framevalue;
} rtk_vlan_protoGroup_t;

/* configuration of each protocol vlan */
typedef struct rtk_vlan_protoVlanCfg_s
{
#if defined(CONFIG_SDK_RTL8328) || defined(CONFIG_SDK_RTL8390)
    uint32      valid;
#endif
    rtk_vlan_t  vid;
    rtk_pri_t   pri;
#if defined(CONFIG_SDK_RTL8328)
    uint32      dei;
#endif
} rtk_vlan_protoVlanCfg_t;

/* configuration of ingress VLAN conversion (C2SC) table */
typedef struct rtk_vlan_igrVlanCnvtEntry_s
{
    uint32      valid;
    uint8       vid_ignore;         /* set to 1 to ignore the search key 'vid' */
    rtk_vlan_t  vid;                /* search key */
    uint32      rngchk_result;      /* search key, bitmask result of range check configurations */
    uint32      rngchk_result_mask; /* mask bits, each bit maps to the corresponding range check
                                     * configurations. set a bit to 0 to ignore comparing result
                                     * for the corresponding range check configuration.
                                     */
    uint8       priority_ignore;    /* set to 1 to ignore the search key 'priority' */
    rtk_pri_t   priority;           /* search key, inner user priority */
    uint8       port_ignore;        /* set to 1 to ignore the search key 'port' */
    rtk_port_t  port;               /* search key, ingress port */

    uint32      vid_cnvt_sel;       /* 0: convert to new inner VID 1: convert to outer VID */
    uint32      vid_shift;          /* 0: directly assign with 'vid_new' 1: shift with 'vid_new' */
    uint32      vid_shift_sel;      /* 0: positive, 1: negative */
    rtk_vlan_t  vid_new;            /* value to assign or shift */
    uint32      pri_assign;         /* convert user priority to 'pri_new' */
    rtk_pri_t   pri_new;            /* the field is valid only if 'pri_assign' = 1.
                                     * 'vid_cnvt_sel' = 0, user priority is converted to 'pri_new'
                                     * 'vid_cnvt_sel' = 1, 'pri_new' is used as user priority of
                                     * outer tagged packet when transmitting.
                                     */
    uint32      inner_tag_sts;      /* force egress inner tag status for outgoing packet
                                     * 0: inner untag 1: inner tag 2: don't force
                                     */
    uint32      outer_tag_sts;      /* force egress outer tag status for outgoing packet
                                     * 0: outer untag 1: outer tag 2: don't force
                                     */
    uint32      tpid_assign;        /* force TPID value of egress inner/outer tagged packet
                                     * 0: don't force 1: force to the value indicated by 'tpid_idx'
                                     */
    uint32      tpid_idx;           /* the field is valid only if 'tpid_assign' = 1.
                                     * 'vid_cnvt_sel' = 0, index to inner TPID list
                                     * 'vid_cnvt_sel' = 1, index to outer TPID list
                                     */
} rtk_vlan_igrVlanCnvtEntry_t;

/* configuration of egress VLAN conversion (SC2C) table */
typedef struct rtk_vlan_egrVlanCnvtEntry_s
{
    uint32      valid;
    uint8       vid_ignore;         /* set to 1 to ignore the search key 'vid' */
    rtk_vlan_t  vid;                /* search key, vid either from inner(C' -> C) or outer(S->C) tag,
                                     * refer to 'rtk_vlan_egrVlanCnvtVidSource_set'
                                     */

    uint8       orgpri_ignore;         /* set to 1 to ignore the search key 'vid' */
    rtk_pri_t   orgpri;             /* search key, priority either from inner(C' -> C) or outer(S->C) tag,
                                     * refer to 'rtk_vlan_egrVlanCnvtVidSource_set'
                                     */

    uint32      rngchk_result;      /* search key, bitmask result of range check configurations */
    uint32      rngchk_result_mask; /* mask bits, each bit maps to the corresponding range check
                                     * configurations. set a bit to 0 to ignore comparing result for
                                     * the corresponding range check configuration.
                                     */

    uint8       port_ignore;        /* set to 1 to ignore the search key 'port' */
    rtk_port_t  port;               /* search key, egress port */

    uint32      vid_shift;          /* 0: directly assign with 'vid_new' 1: shift with 'vid_new' */
    uint32      vid_shift_sel;      /* 0: positive, 1: negative */
    rtk_vlan_t  vid_new;            /* value to assign or shift */
    uint32      pri_assign;         /* convert user priority to 'pri_new' */
    rtk_pri_t   pri_new;            /* the field is valid only if 'pri_assign' = 1 */

#if defined(CONFIG_SDK_RTL8390)
    uint32      itpid_assign;       /* force TPID value of egress inner tagged packet
                                     * 0: don't force 1: force to the value indicated by 'tpid_idx'
                                     */
    uint32      itpid_idx;          /* index to inner TPID list, the field is valid only if 'tpid_assign' = 1 */
#endif
} rtk_vlan_egrVlanCnvtEntry_t;

/* vlan profile configuration */
typedef struct rtk_vlan_profile_s
{
    uint32  learn;                  /* 0: not learn 1: learn */
    uint32  l2_mcast_dlf_pm_idx;    /* index to multicast table which stores the unknown L2 multicast
                                       flooding portmask */
    uint32  ip4_mcast_dlf_pm_idx;   /* index to multicast table which stores the unknown IP4 multicast
                                       flooding portmask */
    uint32  ip6_mcast_dlf_pm_idx;   /* index to multicast table which stores the unknown IP6 multicast
                                       flooding portmask */
} rtk_vlan_profile_t;

/*
 * Macro Declaration
 */

/*
 * Function Declaration
 */

/* Function Name:
 *      rtk_vlan_init
 * Description:
 *      Initialize vlan module of the specified device.
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
 *      Must initialize vlan module before calling any vlan APIs.
 */
extern int32
rtk_vlan_init(uint32 unit);

/* Module Name    : Vlan                                  */
/* Sub-module Name: Vlan table configure and modification */

/* Function Name:
 *      rtk_vlan_create
 * Description:
 *      Create the vlan in the specified device.
 * Input:
 *      unit - unit id
 *      vid  - vlan id to be created
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID    - invalid unit id
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_VLAN_VID   - invalid vid
 *      RT_ERR_VLAN_EXIST - vlan is exist
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The valid range of vid is 0~4095
 *      (2) Default FID and STG is assigned after vlan creation.
 *      (3) Default FID is equal with VID (IVL mode) and default STG is CIST.
 *      (4) FID and STG can be reassigned later by following APIs.
 *          - rtk_vlan_fid_set
 *          - rtk_vlan_stg_set
 */
extern int32
rtk_vlan_create(uint32 unit, rtk_vlan_t vid);

/* Function Name:
 *      rtk_vlan_destroy
 * Description:
 *      Destroy the vlan in the specified device.
 * Input:
 *      unit - unit id
 *      vid  - vlan id to be destroyed
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The valid range of vid is 0~4095
 */
extern int32
rtk_vlan_destroy(uint32 unit, rtk_vlan_t vid);

/* Function Name:
 *      rtk_vlan_destroyAll
 * Description:
 *      Destroy all vlans except default vlan in the specified device.
 * Input:
 *      unit                 - unit id
 *      restore_default_vlan - keep and restore default vlan id or not?
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The restore argument is permit following value:
 *      - 0: remove default vlan
 *      - 1: restore default vlan
 */
extern int32
rtk_vlan_destroyAll(uint32 unit, uint32 restore_default_vlan);

/* Function Name:
 *      rtk_vlan_fid_get
 * Description:
 *      Get the filtering database id of the vlan from the specified device.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 * Output:
 *      pFid - pointer buffer of filtering database id
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The valid range of vid is 0~4095
 *      (2) FID is useless when running in IVL mode.
 *      (3) The API should only be used when running in SVL mode.
 *      (4) FID instance also represents STG in 8390 and 8380
 */
extern int32
rtk_vlan_fid_get(uint32 unit, rtk_vlan_t vid, rtk_fid_t *pFid);

/* Function Name:
 *      rtk_vlan_fid_set
 * Description:
 *      Set the filter id of the vlan to the specified device.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 *      fid  - filter id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_OUT_OF_RANGE         - input parameter out of range
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The valid range of vid is 0~4095
 *      (2) You don't need to care fid when you use the IVL mode.
 *      (3) The API should be used for SVL mode.
 *      (4) FID instance also represents STG in 8390 and 8380
 */
extern int32
rtk_vlan_fid_set(uint32 unit, rtk_vlan_t vid, rtk_fid_t fid);

/* Function Name:
 *      rtk_vlan_port_add
 * Description:
 *      Add one vlan member to the specified device.
 * Input:
 *      unit     - unit id
 *      vid      - vlan id
 *      port     - port id for add
 *      is_untag - untagged or tagged member
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_VLAN_PORT_MBR_EXIST  - member port exist in the specified vlan
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_OUT_OF_RANGE         - input parameter out of range
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The valid range of vid is 0~4095
 *      (2) The valid value of is_untag are {0: tagged, 1: untagged}
 */
extern int32
rtk_vlan_port_add(uint32 unit, rtk_vlan_t vid, rtk_port_t port, uint32 is_untag);

/* Function Name:
 *      rtk_vlan_port_del
 * Description:
 *      Delete one vlan member from the specified device.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 *      port - port id for delete
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The valid range of vid is 0~4095
 */
extern int32
rtk_vlan_port_del(uint32 unit, rtk_vlan_t vid, rtk_port_t port);

/* Function Name:
 *      rtk_vlan_port_get
 * Description:
 *      Get the vlan members from the specified device.
 * Input:
 *      unit             - unit id
 *      vid              - vlan id
 * Output:
 *      pMember_portmask - pointer buffer of member ports
 *      pUntag_portmask  - pointer buffer of untagged member ports
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The valid range of vid is 0~4095
 */
extern int32
rtk_vlan_port_get(
    uint32         unit,
    rtk_vlan_t     vid,
    rtk_portmask_t *pMember_portmask,
    rtk_portmask_t *pUntag_portmask);

/* Function Name:
 *      rtk_vlan_port_set
 * Description:
 *      Replace the vlan members in the specified device.
 * Input:
 *      unit             - unit id
 *      vid              - vlan id
 *      pMember_portmask - member ports
 *      pUntag_portmask  - untagged member ports
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The valid range of vid is 0~4095
 *      (2) Replace with new specified VLAN member regardless of original setting.
 */
extern int32
rtk_vlan_port_set(
    uint32         unit,
    rtk_vlan_t     vid,
    rtk_portmask_t *pMember_portmask,
    rtk_portmask_t *pUntag_portmask);

/* Function Name:
 *      rtk_vlan_stg_get
 * Description:
 *      Get spanning tree group instance of the vlan from the specified device.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 * Output:
 *      pStg - pointer buffer of spanning tree group instance
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The valid range of vid is 0~4095
 *      (2) STG instance also represents FID in 8390 and 8380
 */
extern int32
rtk_vlan_stg_get(uint32 unit, rtk_vlan_t vid, rtk_stg_t *pStg);

/* Function Name:
 *      rtk_vlan_stg_set
 * Description:
 *      Set spanning tree group instance of the vlan to the specified device.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 *      stg  - spanning tree group instance
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_MSTI                 - invalid msti
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The valid range of vid is 0~4095
 *      (2) STG instance also represents FID in 8390 and 8380
 */
extern int32
rtk_vlan_stg_set(uint32 unit, rtk_vlan_t vid, rtk_stg_t stg);

/* Function Name:
 *      rtk_vlan_l2UcastLookupMode_get
 * Description:
 *      Get L2 lookup mode for L2 unicast traffic.
 * Input:
 *      unit  - unit id
 *      vid   - vlan id
 * Output:
 *      pMode - lookup mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      Each VLAN can have its own lookup mode for L2 unicast traffic
 */
extern int32
rtk_vlan_l2UcastLookupMode_get(uint32 unit, rtk_vlan_t vid, rtk_l2_ucastLookupMode_t *pMode);

/* Function Name:
 *      rtk_vlan_l2UcastLookupMode_set
 * Description:
 *      Set L2 lookup mode for L2 unicast traffic.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 *      mode - lookup mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_INPUT                - Invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      Each VLAN can have its own lookup mode for L2 unicast traffic
 */
extern int32
rtk_vlan_l2UcastLookupMode_set(uint32 unit, rtk_vlan_t vid, rtk_l2_ucastLookupMode_t mode);

/* Function Name:
 *      rtk_vlan_l2McastLookupMode_get
 * Description:
 *      Get L2 lookup mode for L2 multicast and IP multicast traffic.
 * Input:
 *      unit  - unit id
 *      vid   - vlan id
 * Output:
 *      pMode - lookup mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      Each VLAN can have its own lookup mode for L2 and IP mulicast traffic.
 */
extern int32
rtk_vlan_l2McastLookupMode_get(uint32 unit, rtk_vlan_t vid, rtk_l2_mcastLookupMode_t *pMode);

/* Function Name:
 *      rtk_vlan_l2McastLookupMode_set
 * Description:
 *      Set L2 lookup mode for L2 multicast and IP multicast traffic.
 * Input:
 *      unit  - unit id
 *      vid   - vlan id
 *      mode  - lookup mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_INPUT                - Invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      Each VLAN can have its own lookup mode for L2 and IP mulicast traffic.
 */
extern int32
rtk_vlan_l2McastLookupMode_set(uint32 unit, rtk_vlan_t vid, rtk_l2_mcastLookupMode_t mode);

/* Function Name:
 *      rtk_vlan_profileIdx_get
 * Description:
 *      Get VLAN profile index of specified VLAN.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 * Output:
 *      pIdx - VLAN profile index
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      Each VLAN can bind to a VLAN profile.
 */
extern int32
rtk_vlan_profileIdx_get(uint32 unit, rtk_vlan_t vid, uint32 *pIdx);

/* Function Name:
 *      rtk_vlan_profileIdx_set
 * Description:
 *      Set VLAN profile index of specified VLAN.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 *      idx  - VLAN profile index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_OUT_OF_RANGE         - profile index is out of range
 * Applicable:
 *      8390, 8380
 * Note:
 *      Each VLAN can bind to a VLAN profile.
 */
extern int32
rtk_vlan_profileIdx_set(uint32 unit, rtk_vlan_t vid, uint32 idx);

/* Module Name     : vlan         */
/* Sub-module Name : vlan profile */

/* Function Name:
 *      rtk_vlan_profile_get
 * Description:
 *      Get specific VLAN profile.
 * Input:
 *      unit     - unit id
 *      idx      - VLAN profile index
 * Output:
 *      pProfile - VLAN profile configuration
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_OUT_OF_RANGE         - profile index is out of range
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_vlan_profile_get(uint32 unit, uint32 idx, rtk_vlan_profile_t *pProfile);

/* Function Name:
 *      rtk_vlan_profile_set
 * Description:
 *      Set specific VLAN profile.
 * Input:
 *      unit     - unit id
 *      idx      - VLAN profile index
 *      pProfile - VLAN profile configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - profile index is out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_vlan_profile_set(uint32 unit, uint32 idx, rtk_vlan_profile_t *pProfile);

/* Module Name     : vlan           */
/* Sub-module Name : vlan attribute */

/* Module Name     : vlan                */
/* Sub-module Name : vlan port attribute */

/* Function Name:
 *      rtk_vlan_portAcceptFrameType_get
 * Description:
 *      Get vlan accept frame type of the port from the specified device.
 * Input:
 *      unit               - unit id
 *      port               - port id
 * Output:
 *      pAccept_frame_type - pointer buffer of accept frame type
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
 *      (1) The accept frame type as following:
 *          - ACCEPT_FRAME_TYPE_ALL
 *          - ACCEPT_FRAME_TYPE_TAG_ONLY
 *          - ACCEPT_FRAME_TYPE_UNTAG_ONLY
 *      (2) The API is used for 802.1Q tagged  and if you want to get the 802.1ad
 *          accept frame type, please use rtk_vlan_portOuterAcceptFrameType_get.
 */
extern int32
rtk_vlan_portAcceptFrameType_get(
    uint32                     unit,
    rtk_port_t                 port,
    rtk_vlan_acceptFrameType_t *pAccept_frame_type);

/* Function Name:
 *      rtk_vlan_portAcceptFrameType_set
 * Description:
 *      Set vlan accept frame type of the port to the specified device.
 * Input:
 *      unit              - unit id
 *      port              - port id
 *      accept_frame_type - accept frame type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_PORT_ID                - invalid port id
 *      RT_ERR_VLAN_ACCEPT_FRAME_TYPE - invalid accept frame type
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The accept frame type as following:
 *          - ACCEPT_FRAME_TYPE_ALL
 *          - ACCEPT_FRAME_TYPE_TAG_ONLY
 *          - ACCEPT_FRAME_TYPE_UNTAG_ONLY
 *      (2) The API is used for 802.1Q tagged  and if you want to set the 802.1ad
 *          accept frame type, please use rtk_vlan_portOuterAcceptFrameType_set.
 */
extern int32
rtk_vlan_portAcceptFrameType_set(
    uint32                     unit,
    rtk_port_t                 port,
    rtk_vlan_acceptFrameType_t accept_frame_type);

/* Function Name:
 *      rtk_vlan_portOuterAcceptFrameType_get
 * Description:
 *      Get accept frame type of outer tag on specified port.
 * Input:
 *      unit               - unit id
 *      port               - port id
 * Output:
 *      pAccept_frame_type - pointer to accept frame type
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
 *      Accept frame type is as following:
 *      - ACCEPT_FRAME_TYPE_ALL
 *      - ACCEPT_FRAME_TYPE_TAG_ONLY
 *      - ACCEPT_FRAME_TYPE_UNTAG_ONLY
 */
extern int32
rtk_vlan_portOuterAcceptFrameType_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_vlan_acceptFrameType_t  *pAccept_frame_type);

/* Function Name:
 *      rtk_vlan_portOuterAcceptFrameType_set
 * Description:
 *      Set accept frame type of outer tag on specified port.
 * Input:
 *      unit              - unit id
 *      port              - port id
 *      accept_frame_type - accept frame type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_PORT_ID                - invalid port id
 *      RT_ERR_VLAN_ACCEPT_FRAME_TYPE - invalid accept frame type
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      Accept frame type is as following:
 *      - ACCEPT_FRAME_TYPE_ALL
 *      - ACCEPT_FRAME_TYPE_TAG_ONLY
 *      - ACCEPT_FRAME_TYPE_UNTAG_ONLY
 */
extern int32
rtk_vlan_portOuterAcceptFrameType_set(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_vlan_acceptFrameType_t  accept_frame_type);

/* Function Name:
 *      rtk_vlan_vlanFunctionEnable_get
 * Description:
 *      Get the VLAN enable status of the specified device.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer buffer of status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389
 * Note:
 *      The status of vlan function is as following:
 *      - DISABLED
 *      - ENABLED
 */
extern int32
rtk_vlan_vlanFunctionEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_vlan_vlanFunctionEnable_set
 * Description:
 *      Set the VLAN enable status of the specified device.
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
 *      8389
 * Note:
 *      The status of vlan function is as following:
 *      - DISABLED
 *      - ENABLED
 */
extern int32
rtk_vlan_vlanFunctionEnable_set(uint32 unit, rtk_enable_t enable);

/* Module Name    : Vlan                       */
/* Sub-module Name: Vlan ingress/egress filter */

/* Function Name:
 *      rtk_vlan_igrFilterEnable_get
 * Description:
 *      Get vlan ingress filter status from the specified device.
 * Input:
 *      unit        - unit id
 * Output:
 *      pIgr_filter - pointer buffer of ingress filter status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389
 * Note:
 *      The status of vlan ingress filter is as following:
 *      - DISABLED
 *      - ENABLED
 */
extern int32
rtk_vlan_igrFilterEnable_get(uint32 unit, rtk_enable_t *pIgr_filter);

/* Function Name:
 *      rtk_vlan_igrFilterEnable_set
 * Description:
 *      Set vlan ingress filter status to the specified device.
 * Input:
 *      unit       - unit id
 *      igr_filter - ingress filter configure
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8389
 * Note:
 *      (1) If the chip is per system configuration, set any port will apply to
 *          whole device. like RTL8329/RTL8389 chip.
 *      (2) The status of vlan ingress filter is as following:
 *          - DISABLED
 *          - ENABLED
 */
extern int32
rtk_vlan_igrFilterEnable_set(uint32 unit, rtk_enable_t igr_filter);

/* Function Name:
 *      rtk_vlan_portIgrFilterEnable_get
 * Description:
 *      Get vlan ingress filter status of the port from the specified device.
 * Input:
 *      unit        - unit id
 *      port        - port id
 * Output:
 *      pIgr_filter - pointer buffer of ingress filter status
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
 *      (1) The status of ingress filter is as following:
 *          - DISABLED
 *          - ENABLED
 *      (2) For 8390 and 8380, the function is backward compatible RTL8328 APIs.
 *          The 'drop' action is return enable; 'forward' or 'trap' action is return disable.
 */
extern int32
rtk_vlan_portIgrFilterEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pIgr_filter);

/* Function Name:
 *      rtk_vlan_portIgrFilterEnable_set
 * Description:
 *      Set vlan ingress filter status of the port to the specified device.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      igr_filter - ingress filter configure
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
 *      (1) The status of vlan ingress filter is as following:
 *          - DISABLED
 *          - ENABLED
 *      (2) For 8390 and 8380, the function is backward compatible RTL8328 APIs.
 *          Congiure enable is mapped to 'drop' action; disable is mapped to 'forward' action.
 */
extern int32
rtk_vlan_portIgrFilterEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t igr_filter);

/* Function Name:
 *      rtk_vlan_portEgrFilterEnable_get
 * Description:
 *      Get enable status of egress filtering on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of egress filtering
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
 *      None
 */
extern int32
rtk_vlan_portEgrFilterEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_vlan_portEgrFilterEnable_set
 * Description:
 *      Set enable status of egress filtering on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of egress filtering
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
 *      None
 */
extern int32
rtk_vlan_portEgrFilterEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_vlan_portIgrFilter_get
 * Description:
 *      Get vlan ingress filter configuration of the port from the specified device.
 * Input:
 *      unit        - unit id
 *      port        - port id
 * Output:
 *      pIgr_filter - pointer buffer of ingress filter configuration
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
 *      VLAN ingress filter checks if the packet's ingress port belongs to the given(forwarding) VLAN
 *      when the packet comes in the switch.
 */
extern int32
rtk_vlan_portIgrFilter_get(uint32 unit, rtk_port_t port, rtk_vlan_ifilter_t *pIgr_filter);

/* Function Name:
 *      rtk_vlan_portIgrFilter_set
 * Description:
 *      Set vlan ingress filter configuration of the port from the specified device.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      igr_filter - ingress filter configuration
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
 *      VLAN ingress filter checks if the packet's ingress port belongs to the given(forwarding) VLAN
 *      when the packet comes in the switch.
 */
extern int32
rtk_vlan_portIgrFilter_set(uint32 unit, rtk_port_t port, rtk_vlan_ifilter_t igr_filter);

/* Function Name:
 *      rtk_vlan_mcastLeakyEnable_get
 * Description:
 *      Get vlan egress leaky status of the system from the specified device.
 * Input:
 *      unit   - unit id
 * Output:
 *      pLeaky - pointer buffer of vlan leaky of egress
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8390, 8380
 * Note:
 *      (1) Enable leaky function to allow known L2/IP multicast traffic to across VLAN.
 *          That is, egress VLAN filtering is bypassed by known L2/IP multicast traffic.
 *      (2) The status of vlan egress multicast leaky is as following:
 *          - DISABLED
 *          - ENABLED
 */
extern int32
rtk_vlan_mcastLeakyEnable_get(uint32 unit, rtk_enable_t *pLeaky);

/* Function Name:
 *      rtk_vlan_mcastLeakyEnable_set
 * Description:
 *      Set vlan egress leaky configure of the system to the specified device.
 * Input:
 *      unit  - unit id
 *      leaky - vlan leaky of egress
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8389, 8390, 8380
 * Note:
 *      (1) Enable leaky function to allow known L2/IP MULTICAST traffic to across VLAN.
 *          That is, egress VLAN filtering is bypassed by known L2/IP multicast traffic.
 *      (2) The status of vlan egress multicast leaky is as following:
 *          - DISABLED
 *          - ENABLED
 */
extern int32
rtk_vlan_mcastLeakyEnable_set(uint32 unit, rtk_enable_t leaky);

/* Function Name:
 *      rtk_vlan_mcastLeakyPortEnable_get
 * Description:
 *      Get vlan egress leaky status of the port from the specified device.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pLeaky - pointer buffer of vlan leaky of egress
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328
 * Note:
 *      The status of vlan egress multicast leaky is as following:
 *      - DISABLED
 *      - ENABLED
 */
extern int32
rtk_vlan_mcastLeakyPortEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pLeaky);

/* Function Name:
 *      rtk_vlan_mcastLeakyPortEnable_set
 * Description:
 *      Set vlan egress leaky configure of the port to the specified device.
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      leaky - vlan leaky of egress
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
 *      8389, 8328
 * Note:
 *      (1) egress vlan leaky configuration is apply to L2/IP multicast packet only,
 *          no apply to unicast.
 *      (2) If the chip is per system configuration, set any port will apply
 *          to whole device.
 *      (3) The status of vlan egress multicast leaky is as following:
 *          - DISABLED
 *          - ENABLED
 */
extern int32
rtk_vlan_mcastLeakyPortEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t leaky);

/* Module Name    : Vlan                               */
/* Sub-module Name: Port based and protocol based vlan */

/* Function Name:
 *      rtk_vlan_portPvidMode_get
 * Description:
 *      Get the configuration of inner port-based VLAN mode.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pMode - pointer to inner port-based VLAN mode configuration
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
 *      Inner port-based VLAN can apply on different tag format, refer to enum rtk_vlan_pbVlan_mode_t.
 */
extern int32
rtk_vlan_portPvidMode_get(uint32 unit, rtk_port_t port, rtk_vlan_pbVlan_mode_t *pMode);

/* Function Name:
 *      rtk_vlan_portPvidMode_set
 * Description:
 *      Set the configuration of inner port-based VLAN mode.
 * Input:
 *      unit - unit id
 *      port - port id
 *      mode - inner port-based VLAN mode configuration
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
 *      Inner port-based VLAN can apply on different tag format, refer to enum rtk_vlan_pbVlan_mode_t.
 */
extern int32
rtk_vlan_portPvidMode_set(uint32 unit, rtk_port_t port, rtk_vlan_pbVlan_mode_t mode);

/* Function Name:
 *      rtk_vlan_portOuterPvidMode_get
 * Description:
 *      Get the configuration of outer port-based VLAN mode.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pMode - pointer to outer port-based VLAN mode configuration
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
 *      Outer port-based VLAN can apply on different tag format, refer to enum rtk_vlan_pbVlan_mode_t.
 */
extern int32
rtk_vlan_portOuterPvidMode_get(uint32 unit, rtk_port_t port, rtk_vlan_pbVlan_mode_t *pMode);

/* Function Name:
 *      rtk_vlan_portOuterPvidMode_set
 * Description:
 *      Set the configuration of outer port-based VLAN mode.
 * Input:
 *      unit - unit id
 *      port - port id
 *      mode - outer port-based VLAN mode configuration
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
 *      Outer port-based VLAN can apply on different tag format, refer to enum rtk_vlan_pbVlan_mode_t.
 */
extern int32
rtk_vlan_portOuterPvidMode_set(uint32 unit, rtk_port_t port, rtk_vlan_pbVlan_mode_t mode);

/* Function Name:
 *      rtk_vlan_portPvid_get
 * Description:
 *      Get port default vlan id from the specified device.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pPvid - pointer buffer of port default vlan id
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
rtk_vlan_portPvid_get(uint32 unit, rtk_port_t port, rtk_vlan_t *pPvid);

/* Function Name:
 *      rtk_vlan_portPvid_set
 * Description:
 *      Set port default vlan id to the specified device.
 * Input:
 *      unit - unit id
 *      port - port id
 *      pvid - port default vlan id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_VLAN_VID - invalid vid
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_vlan_portPvid_set(uint32 unit, rtk_port_t port, rtk_vlan_t pvid);

/* Function Name:
 *      rtk_vlan_portOuterPvid_get
 * Description:
 *      Get outer port based vlan on specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pVid - pointer to outer port based vlan id
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
 *      None
 */
extern int32
rtk_vlan_portOuterPvid_get(uint32 unit, rtk_port_t port, rtk_vlan_t *pPvid);

/* Function Name:
 *      rtk_vlan_portOuterPvid_set
 * Description:
 *      Set outer port based vlan on specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 *      vid  - outer port based vlan id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_VLAN_VID - invalid vlan id
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_vlan_portOuterPvid_set(uint32 unit, rtk_port_t port, rtk_vlan_t pvid);

/* Function Name:
 *      rtk_vlan_macBasedVlan_get
 * Description:
 *      Get MAC-based vlan.
 * Input:
 *      unit      - unit id
 *      index     - entry index
 * Output:
 *      pValid    - pointer buffer of validate or invalidate the entry
 *      pSmac     - pointer buffer of source mac address
 *      pVid      - pointer buffer of MAC-based VLAN ID
 *      pPriority - pointer buffer of assign internal priority for untag packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_OUT_OF_RANGE         - index is out of range
 *      RT_ERR_VLAN_C2SC_BLOCK_MODE - try to set/get a entry to an incompatiable block
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      (1) Totally 1024 entries are supported but the entries are shared by ingress VLAN
 *          conversion, MAC-based VLAN and IP-Subnet-based VLAN functions per-block based.
 *      (2) The corresponding block must be in MAC-based VLAN mode before calling this function,
 *          refer to rtk_vlan_igrVlanCnvtBlkMode_set.
 */
extern int32
rtk_vlan_macBasedVlan_get(
    uint32      unit,
    uint32      index,
    uint32      *pValid,
    rtk_mac_t   *pSmac,
    rtk_vlan_t  *pVid,
    rtk_pri_t   *pPriority);

/* Function Name:
 *      rtk_vlan_macBasedVlan_set
 * Description:
 *      Set MAC-based vlan.
 * Input:
 *      unit     - unit id
 *      index    - entry index
 *      valid    - validate or invalidate the entry
 *      pSmac    - source mac address
 *      vid      - MAC-based VLAN ID
 *      priority - assign internal priority for untag packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_OUT_OF_RANGE         - index is out of range
 *      RT_ERR_VLAN_C2SC_BLOCK_MODE - try to set/get a entry to an incompatiable block
 *      RT_ERR_MAC                  - invalid mac address
 *      RT_ERR_VLAN_VID             - invalid vlan id
 *      RT_ERR_PRIORITY             - invalid priority value
 * Applicable:
 *      8390
 * Note:
 *      (1) Totally 1024 entries are supported but the entries are shared by ingress VLAN
 *          conversion, MAC-based VLAN and IP-Subnet-based VLAN functions per-block based.
 *      (2) The corresponding block must be in MAC-based VLAN mode before calling this function,
 *          refer to rtk_vlan_igrVlanCnvtBlkMode_set.
 */
extern int32
rtk_vlan_macBasedVlan_set(
    uint32      unit,
    uint32      index,
    uint32      valid,
    rtk_mac_t   *pSmac,
    rtk_vlan_t  vid,
    rtk_pri_t   priority);

/* Function Name:
 *      rtk_vlan_macBasedVlanWithMsk_get
 * Description:
 *      Get MAC-based vlan with source MAC address mask.
 * Input:
 *      unit      - unit id
 *      index     - entry index
 * Output:
 *      pValid    - pointer buffer of validate or invalidate the entry
 *      pSmac     - pointer buffer of source mac address
 *      pSmsk     - pointer buffer of source mac address mask
 *      pVid      - pointer buffer of MAC-based VLAN ID
 *      pPriority - pointer buffer of assign internal priority for untag packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_OUT_OF_RANGE         - index is out of range
 *      RT_ERR_VLAN_C2SC_BLOCK_MODE - try to set/get a entry to an incompatiable block
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      (1) Totally 1024 entries are supported but the entries are shared by ingress VLAN
 *          conversion, MAC-based VLAN and IP-Subnet-based VLAN functions per-block based.
 *      (2) The corresponding block must be in MAC-based VLAN mode before calling this function,
 *          refer to rtk_vlan_igrVlanCnvtBlkMode_set.
 */
extern int32
rtk_vlan_macBasedVlanWithMsk_get(
    uint32      unit,
    uint32      index,
    uint32      *pValid,
    rtk_mac_t   *pSmac,
    rtk_mac_t   *pSmsk,
    rtk_vlan_t  *pVid,
    rtk_pri_t   *pPriority);

/* Function Name:
 *      rtk_vlan_macBasedVlanWithMsk_set
 * Description:
 *      Set MAC-based vlan with source MAC address mask.
 * Input:
 *      unit     - unit id
 *      index    - entry index
 *      valid    - validate or invalidate the entry
 *      pSmac    - source mac address
 *      pSmsk    - source mac address mask
 *      vid      - MAC-based VLAN ID
 *      priority - assign internal priority for untag packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_OUT_OF_RANGE         - index is out of range
 *      RT_ERR_VLAN_C2SC_BLOCK_MODE - try to set/get a entry to an incompatiable block
 *      RT_ERR_MAC                  - invalid mac address
 *      RT_ERR_VLAN_VID             - invalid vlan id
 *      RT_ERR_PRIORITY             - invalid priority value
 * Applicable:
 *      8390
 * Note:
 *      (1) Totally 1024 entries are supported but the entries are shared by ingress VLAN
 *          conversion, MAC-based VLAN and IP-Subnet-based VLAN functions per-block based.
 *      (2) The corresponding block must be in MAC-based VLAN mode before calling this function,
 *          refer to rtk_vlan_igrVlanCnvtBlkMode_set.
 */
extern int32
rtk_vlan_macBasedVlanWithMsk_set(
    uint32      unit,
    uint32      index,
    uint32      valid,
    rtk_mac_t   *pSmac,
    rtk_mac_t   *pSmsk,
    rtk_vlan_t  vid,
    rtk_pri_t   priority);

/* Function Name:
 *      rtk_vlan_macBasedVlanWithPort_get
 * Description:
 *      Get MAC-based vlan with source port ID mask.
 * Input:
 *      unit      - unit id
 *      index     - entry index
 * Output:
 *      pValid    - pointer buffer of validate or invalidate the entry
 *      pSmac     - pointer buffer of source mac address
 *      pSmsk     - pointer buffer of source mac address mask
 *      pPort     - pointer buffer of source port ID
 *      pPmsk     - pointer buffer of source port ID mask
 *      pVid      - pointer buffer of MAC-based VLAN ID
 *      pPriority - pointer buffer of assign internal priority for untag packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_OUT_OF_RANGE         - index is out of range
 *      RT_ERR_VLAN_C2SC_BLOCK_MODE - try to set/get a entry to an incompatiable block
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      (1) Totally 1024 entries are supported but the entries are shared by ingress VLAN
 *          conversion, MAC-based VLAN and IP-Subnet-based VLAN functions per-block based.
 *      (2) The corresponding block must be in MAC-based VLAN mode before calling this function,
 *          refer to rtk_vlan_igrVlanCnvtBlkMode_set.
 */
extern int32
rtk_vlan_macBasedVlanWithPort_get(
    uint32      unit,
    uint32      index,
    uint32      *pValid,
    rtk_mac_t   *pSmac,
    rtk_mac_t   *pSmsk,
    rtk_port_t  *pPort,
    rtk_port_t  *pPmsk,
    rtk_vlan_t  *pVid,
    rtk_pri_t   *pPriority);

/* Function Name:
 *      rtk_vlan_macBasedVlanWithPort_set
 * Description:
 *      Set MAC-based vlan with source port ID mask.
 * Input:
 *      unit     - unit id
 *      index    - entry index
 *      valid    - validate or invalidate the entry
 *      pSmac    - source mac address
 *      pSmsk    - source mac address mask
 *      port     - source port ID
 *      pmsk     - source port ID mask
 *      vid      - MAC-based VLAN ID
 *      priority - assign internal priority for untag packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_OUT_OF_RANGE         - index or port ID bit-mask is out of range 
 *      RT_ERR_VLAN_C2SC_BLOCK_MODE - try to set/get a entry to an incompatiable block
 *      RT_ERR_MAC                  - invalid mac address
 *      RT_ERR_VLAN_VID             - invalid vlan id
 *      RT_ERR_PRIORITY             - invalid priority value
 * Applicable:
 *      8390
 * Note:
 *      (1) Totally 1024 entries are supported but the entries are shared by ingress VLAN
 *          conversion, MAC-based VLAN and IP-Subnet-based VLAN functions per-block based.
 *      (2) The corresponding block must be in MAC-based VLAN mode before calling this function,
 *          refer to rtk_vlan_igrVlanCnvtBlkMode_set.
 */
extern int32
rtk_vlan_macBasedVlanWithPort_set(
    uint32      unit,
    uint32      index,
    uint32      valid,
    rtk_mac_t   *pSmac,
    rtk_mac_t   *pSmsk,
    rtk_port_t  port,
    rtk_port_t  pmsk,
    rtk_vlan_t  vid,
    rtk_pri_t   priority);

/* Function Name:
 *      rtk_vlan_ipSubnetBasedVlan_get
 * Description:
 *      Get IP-Subnet-based vlan.
 * Input:
 *      unit      - unit id
 *      index     - entry index
 * Output:
 *      pValid    - pointer buffer of validate or invalidate the entry
 *      pSip      - pointer buffer of source IP address
 *      pSip_mask - pointer buffer of source IP address mask
 *      pVid      - pointer buffer of VLAN ID
 *      pPriority - pointer buffer of assign internal priority for untag packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_OUT_OF_RANGE         - index is out of range
 *      RT_ERR_VLAN_C2SC_BLOCK_MODE - try to set/get a entry to an incompatiable block
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      (1) Totally 1024 entries are supported but the entries are shared by ingress VLAN
 *          conversion, MAC-based VLAN and IP-Subnet-based VLAN functions per-block based.
 *      (2) The corresponding block must be in IP-Subnet-based VLAN mode before calling this function,
 *          refer to rtk_vlan_igrVlanCnvtBlkMode_set.
 */
extern int32
rtk_vlan_ipSubnetBasedVlan_get(
    uint32      unit,
    uint32      index,
    uint32      *pValid,
    ipaddr_t    *pSip,
    ipaddr_t    *pSip_mask,
    rtk_vlan_t  *pVid,
    rtk_pri_t   *pPriority);

/* Function Name:
 *      rtk_vlan_ipSubnetBasedVlan_set
 * Description:
 *      Set IP-Subnet-based vlan.
 * Input:
 *      unit     - unit id
 *      index    - entry index
 *      valid    - validate or invalidate the entry
 *      sip      - source IP address
 *      sip_mask - source IP address mask
 *      vid      - VLAN ID
 *      priority - assign internal priority for untag packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_OUT_OF_RANGE         - index is out of range
 *      RT_ERR_VLAN_C2SC_BLOCK_MODE - try to set/get a entry to an incompatiable block
 *      RT_ERR_VLAN_VID             - invalid vlan id
 *      RT_ERR_PRIORITY             - invalid priority value
 * Applicable:
 *      8390
 * Note:
 *      (1) Totally 1024 entries are supported but the entries are shared by ingress VLAN
 *          conversion, MAC-based VLAN and IP-Subnet-based VLAN functions per-block based.
 *      (2) The corresponding block must be in IP-Subnet-based VLAN mode before calling this function,
 *          refer to rtk_vlan_igrVlanCnvtBlkMode_set.
 */
extern int32
rtk_vlan_ipSubnetBasedVlan_set(
    uint32      unit,
    uint32      index,
    uint32      valid,
    ipaddr_t    sip,
    ipaddr_t    sip_mask,
    rtk_vlan_t  vid,
    rtk_pri_t   priority);

/* Function Name:
 *      rtk_vlan_ipSubnetBasedVlanWithPort_get
 * Description:
 *      Get IP-Subnet-based vlan with source port ID mask.
 * Input:
 *      unit      - unit id
 *      index     - entry index
 * Output:
 *      pValid    - pointer buffer of validate or invalidate the entry
 *      pSip      - pointer buffer of source IP address
 *      pSip_mask - pointer buffer of source IP address mask
 *      pPort      - pointer buffer of source IP address
 *      pPort_mask - pointer buffer of source IP address mask
 *      pVid      - pointer buffer of VLAN ID
 *      pPriority - pointer buffer of assign internal priority for untag packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_OUT_OF_RANGE         - index is out of range
 *      RT_ERR_VLAN_C2SC_BLOCK_MODE - try to set/get a entry to an incompatiable block
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      (1) Totally 1024 entries are supported but the entries are shared by ingress VLAN
 *          conversion, MAC-based VLAN and IP-Subnet-based VLAN functions per-block based.
 *      (2) The corresponding block must be in IP-Subnet-based VLAN mode before calling this function,
 *          refer to rtk_vlan_igrVlanCnvtBlkMode_set.
 */
extern int32
rtk_vlan_ipSubnetBasedVlanWithPort_get(
    uint32      unit,
    uint32      index,
    uint32      *pValid,
    ipaddr_t    *pSip,
    ipaddr_t    *pSip_mask,
    rtk_port_t  *pPort,
    rtk_port_t  *pPort_mask,
    rtk_vlan_t  *pVid,
    rtk_pri_t   *pPriority);

/* Function Name:
 *      rtk_vlan_ipSubnetBasedVlanWithPort_set
 * Description:
 *      Set IP-Subnet-based vlan with source port ID mask.
 * Input:
 *      unit     - unit id
 *      index    - entry index
 *      valid    - validate or invalidate the entry
 *      sip      - source IP address
 *      sip_mask - source IP address mask
 *      port      - source port ID
 *      port_mask - source port ID mask
 *      vid      - VLAN ID
 *      priority - assign internal priority for untag packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_OUT_OF_RANGE         - index or port ID mask is out of range
 *      RT_ERR_VLAN_C2SC_BLOCK_MODE - try to set/get a entry to an incompatiable block
 *      RT_ERR_VLAN_VID             - invalid vlan id
 *      RT_ERR_PRIORITY             - invalid priority value
 * Applicable:
 *      8390
 * Note:
 *      (1) Totally 1024 entries are supported but the entries are shared by ingress VLAN
 *          conversion, MAC-based VLAN and IP-Subnet-based VLAN functions per-block based.
 *      (2) The corresponding block must be in IP-Subnet-based VLAN mode before calling this function,
 *          refer to rtk_vlan_igrVlanCnvtBlkMode_set.
 */
extern int32
rtk_vlan_ipSubnetBasedVlanWithPort_set(
    uint32      unit,
    uint32      index,
    uint32      valid,
    ipaddr_t    sip,
    ipaddr_t    sip_mask,
    rtk_port_t  port,
    rtk_port_t  port_mask,
    rtk_vlan_t  vid,
    rtk_pri_t   priority);

/* Function Name:
 *      rtk_vlan_protoGroup_get
 * Description:
 *      Get protocol group for protocol based vlan.
 * Input:
 *      unit           - unit id
 *      protoGroup_idx - protocol group index
 * Output:
 *      pProtoGroup    - pointer to protocol group
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - protocol group index is out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390
 * Note:
 *      (1) The valid range of protoGroup_idx is 0~7
 */
extern int32
rtk_vlan_protoGroup_get(
    uint32                  unit,
    uint32                  protoGroup_idx,
    rtk_vlan_protoGroup_t   *pProtoGroup);

/* Function Name:
 *      rtk_vlan_protoGroup_set
 * Description:
 *      Set protocol group for protocol based vlan.
 * Input:
 *      unit           - unit id
 *      protoGroup_idx - protocol group index
 *      protoGroup     - protocol group
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_VLAN_FRAME_TYPE - invalid frame type
 *      RT_ERR_OUT_OF_RANGE    - protocol group index is out of range
 *      RT_ERR_INPUT           - invalid input parameter
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 * Applicable:
 *      8328, 8390
 * Note:
 *      (1) The valid range of protoGroup_idx is 0~7
 *      (2) Frame type is as following:
 *          - FRAME_TYPE_ETHERNET
 *          - FRAME_TYPE_RFC1042 (SNAP)
 *          - FRAME_TYPE_LLCOTHER
 */
extern int32
rtk_vlan_protoGroup_set(
    uint32                  unit,
    uint32                  protoGroup_idx,
    rtk_vlan_protoGroup_t   *pProtoGroup);

/* Function Name:
 *      rtk_vlan_portProtoVlan_get
 * Description:
 *      Get VLAN and priority configuration of specified protocol group on port.
 * Input:
 *      unit           - unit id
 *      port           - port id
 *      protoGroup_idx - protocol group index
 * Output:
 *      pVlan_cfg      - pointer to vlan configuration of protocol group
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_OUT_OF_RANGE - protocol group index is out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390
 * Note:
 *      (1) The valid range of protoGroup_idx is 0~7
 */
extern int32
rtk_vlan_portProtoVlan_get(
    uint32                  unit,
    rtk_port_t              port,
    uint32                  protoGroup_idx,
    rtk_vlan_protoVlanCfg_t *pVlan_cfg);

/* Function Name:
 *      rtk_vlan_portProtoVlan_set
 * Description:
 *      Set VLAN and priority configuration of specified protocol group on port.
 * Input:
 *      unit           - unit id
 *      port           - port id
 *      protoGroup_idx - protocol group index
 *      pVlan_cfg      - vlan configuration of protocol group
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_PORT_ID         - invalid port id
 *      RT_ERR_OUT_OF_RANGE    - protocol group index is out of range
 *      RT_ERR_VLAN_VID        - invalid vlan id
 *      RT_ERR_QOS_1P_PRIORITY - Invalid 802.1p priority
 *      RT_ERR_INPUT           - invalid input parameter
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 * Applicable:
 *      8328, 8390
 * Note:
 *      (1) The valid range of protoGroup_idx is 0~7
 *      (2) Frame type is as following:
 *          - FRAME_TYPE_ETHERNET
 *          - FRAME_TYPE_RFC1042 (SNAP)
 *          - FRAME_TYPE_LLCOTHER
 */
extern int32
rtk_vlan_portProtoVlan_set(
    uint32                  unit,
    rtk_port_t              port,
    uint32                  protoGroup_idx,
    rtk_vlan_protoVlanCfg_t *pVlan_cfg);

/* Function Name:
 *      rtk_vlan_portOuterProtoVlan_get
 * Description:
 *      Get outer vlan of specificed protocol group on specified port.
 * Input:
 *      unit           - unit id
 *      port           - port id
 *      protoGroup_idx - protocol group index
 * Output:
 *      pVlan_cfg      - pointer to vlan configuration of protocol group
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_OUT_OF_RANGE - protocol group index is out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of protoGroup_idx is 0~7
 */
extern int32
rtk_vlan_portOuterProtoVlan_get(
    uint32                  unit,
    rtk_port_t              port,
    uint32                  protoGroup_idx,
    rtk_vlan_protoVlanCfg_t *pVlan_cfg);

/* Function Name:
 *      rtk_vlan_portOuterProtoVlan_set
 * Description:
 *      Set outer vlan of specificed protocol group on specified port.
 * Input:
 *      unit           - unit id
 *      port           - port id
 *      protoGroup_idx - protocol group index
 *      pVlan_cfg      - vlan configuration of protocol group
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_OUT_OF_RANGE - protocol group index is out of range
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of protoGroup_idx is 0~7
 *      (2) Frame type is as following:
 *          - FRAME_TYPE_ETHERNET
 *          - FRAME_TYPE_RFC1042 (SNAP)
 *          - FRAME_TYPE_LLCOTHER
 */
extern int32
rtk_vlan_portOuterProtoVlan_set(
    uint32                  unit,
    rtk_port_t              port,
    uint32                  protoGroup_idx,
    rtk_vlan_protoVlanCfg_t *pVlan_cfg);

/* Module Name    : Vlan               */
/* Sub-module Name: TPID configuration */

/* Function Name:
 *      rtk_vlan_innerTpidEntry_get
 * Description:
 *      Get inner TPID value from global inner TPID pool.
 * Input:
 *      unit     - unit id
 *      tpid_idx - index of TPID entry
 * Output:
 *      pTpid    - pointer to TPID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      Global four inner TPID can be specified.
 */
extern int32
rtk_vlan_innerTpidEntry_get(uint32 unit, uint32 tpid_idx, uint32 *pTpid);

/* Function Name:
 *      rtk_vlan_innerTpidEntry_set
 * Description:
 *      Set inner TPID value to global inner TPID pool.
 * Input:
 *      unit     - unit id
 *      tpid_idx - index of TPID entry
 *      tpid     - TPID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8390, 8380
 * Note:
 *      Global four inner TPID can be specified.
 */
extern int32
rtk_vlan_innerTpidEntry_set(uint32 unit, uint32 tpid_idx, uint32 tpid);

/* Function Name:
 *      rtk_vlan_outerTpidEntry_get
 * Description:
 *      Get outer TPID value from global outer TPID pool.
 * Input:
 *      unit     - unit id
 *      tpid_idx - index of TPID entry
 * Output:
 *      pTpid    - pointer to TPID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      Global four outer TPID can be specified.
 */
extern int32
rtk_vlan_outerTpidEntry_get(uint32 unit, uint32 tpid_idx, uint32 *pTpid);

/* Function Name:
 *      rtk_vlan_outerTpidEntry_set
 * Description:
 *      Set outer TPID value to global outer TPID pool.
 * Input:
 *      unit     - unit id
 *      tpid_idx - index of TPID entry
 *      tpid     - TPID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8390, 8380
 * Note:
 *      Global four outer TPID can be specified.
 */
extern int32
rtk_vlan_outerTpidEntry_set(uint32 unit, uint32 tpid_idx, uint32 tpid);

/* Function Name:
 *      rtk_vlan_extraTpidEntry_get
 * Description:
 *      Get the TPID value of extra tag.
 * Input:
 *      unit     - unit id
 *      tpid_idx - index of TPID entry
 * Output:
 *      pTpid    - pointer to TPID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) Only one extra tag is supported. System will bypass extra tag
 *          and keeps parsing the remaining payload.
 *      (2) Following tag combination are supported for parsing an extra tag:
 *          outer+innet+extra, outer+extra, inner+extra
 */
extern int32
rtk_vlan_extraTpidEntry_get(uint32 unit, uint32 tpid_idx, uint32 *pTpid);

/* Function Name:
 *      rtk_vlan_extraTpidEntry_set
 * Description:
 *      Set TPID value of extra tag.
 * Input:
 *      unit     - unit id
 *      tpid_idx - index of TPID entry
 *      tpid     - TPID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) Only one extra tag is supported. System will bypass extra tag
 *          and keeps parsing the remaining payload.
 *      (2) Following tag combination are supported for parsing an extra tag:
 *          outer+innet+extra, outer+extra, inner+extra
 */
extern int32
rtk_vlan_extraTpidEntry_set(uint32 unit, uint32 tpid_idx, uint32 tpid);

/* Function Name:
 *      rtk_vlan_portTpidEntry_get
 * Description:
 *      Get TPID of TPID entry on specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      tpid_idx - index of TPID entry
 * Output:
 *      pTpid    - pointer to TPID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The valid range of tpid_idx is 0~3 in 8328
 */
extern int32
rtk_vlan_portTpidEntry_get(uint32 unit, rtk_port_t port, uint32 tpid_idx, uint32 *pTpid);

/* Function Name:
 *      rtk_vlan_portTpidEntry_set
 * Description:
 *      Set tpid of TPID entry on specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      tpid_idx - index of TPID entry
 *      tpid     - TPID
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
 *      8328
 * Note:
 *      The valid range of tpid_idx is 0~3 in 8328
 */
extern int32
rtk_vlan_portTpidEntry_set(uint32 unit, rtk_port_t port, uint32 tpid_idx, uint32 tpid);

/* Function Name:
 *      rtk_vlan_portEgrInnerTpidMode_get
 * Description:
 *      Get TPID mode of inner tag at egress.
 *      TPID mode will decide what kind of packet should be replaced inner TPID with configured one.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pTpid_mode - pointer to inner TPID mode at egress
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
 *      Inner TPID mode is as following:
 *      - EGR_TPID_MODE_IGR_UNTAG_AND_EGR_TAG
 *      - EGR_TPID_MODE_ALL_PACKETS
 */
extern int32
rtk_vlan_portEgrInnerTpidMode_get(uint32 unit, rtk_port_t port, rtk_vlan_egrTpidMode_t *pTpid_mode);

/* Function Name:
 *      rtk_vlan_portEgrInnerTpidMode_set
 * Description:
 *      Set TPID mode of inner tag at egress.
 *      TPID mode will decide what kind of packet should be replaced inner TPID with configured one.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      tpid_mode - inner TPID mode at egress
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
 *      Inner TPID mode is as following:
 *      - EGR_TPID_MODE_IGR_UNTAG_AND_EGR_TAG
 *      - EGR_TPID_MODE_ALL_PACKETS
 */
extern int32
rtk_vlan_portEgrInnerTpidMode_set(uint32 unit, rtk_port_t port, rtk_vlan_egrTpidMode_t tpid_mode);

/* Function Name:
 *      rtk_vlan_portIgrInnerTpid_get
 * Description:
 *      Get inner TPIDs comparsion configration on specified port.
 * Input:
 *      unit           - unit id
 *      port           - port id
 * Output:
 *      pTpid_idx_mask - pointer to mask for index of tpid entry
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
 *      Specify which TPID to compare from TPID pool for parsing a inner-tagged packet.
 *      The valid mask bits of tpid_idx is bit[3:0].
 */
extern int32
rtk_vlan_portIgrInnerTpid_get(uint32 unit, rtk_port_t port, uint32 *pTpid_idx_mask);

/* Function Name:
 *      rtk_vlan_portIgrInnerTpid_set
 * Description:
 *      Set inner TPIDs comparsion configration on specified port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      tpid_idx_mask - mask for index of tpid entry
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
 *      Specify which TPID to compare from TPID pool for parsing a inner-tagged packet.
 *      The valid mask bits of tpid_idx is bit[3:0].
 */
extern int32
rtk_vlan_portIgrInnerTpid_set(uint32 unit, rtk_port_t port, uint32 tpid_idx_mask);

/* Function Name:
 *      rtk_vlan_portEgrInnerTpid_get
 * Description:
 *      Get inner TPID for inner tag encapsulation when transmiiting a inner-tagged pacekt.
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pTpid_idx - pointer to index of inner TPID
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
 *      (1) The valid range of tpid_idx is 0~3.
 *      (2) The inner TPID is used when transmiiting a inner-tagged pacekt only if it is received as
 *          an inner-untag pacekt.
 */
extern int32
rtk_vlan_portEgrInnerTpid_get(uint32 unit, rtk_port_t port, uint32 *pTpid_idx);

/* Function Name:
 *      rtk_vlan_portEgrInnerTpid_set
 * Description:
 *      Set inner TPID for inner tag encapsulation when transmiiting a inner-tagged pacekt.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      tpid_idx - index of inner TPID
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
 *      8328, 8390, 8380
 * Note:
 *      (1) The valid range of tpid_idx is 0~3.
 *      (2) The inner TPID is used when transmiiting a inner-tagged pacekt only if it is received as
 *          an inner-untag pacekt.
 */
extern int32
rtk_vlan_portEgrInnerTpid_set(uint32 unit, rtk_port_t port, uint32 tpid_idx);

/* Function Name:
 *      rtk_vlan_portEgrOuterTpidMode_get
 * Description:
 *      Get TPID mode of outer tag at egress.
 *      TPID mode will decide what kind of packet should be replaced outer TPID with configured one.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pTpid_mode - pointer to outer TPID mode at egress
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
 *      Inner TPID mode is as following:
 *      - EGR_TPID_MODE_IGR_UNTAG_AND_EGR_TAG
 *      - EGR_TPID_MODE_ALL_PACKETS
 */
extern int32
rtk_vlan_portEgrOuterTpidMode_get(uint32 unit, rtk_port_t port, rtk_vlan_egrTpidMode_t *pTpid_mode);

/* Function Name:
 *      rtk_vlan_portEgrOuterTpidMode_set
 * Description:
 *      Set TPID mode of outer tag at egress.
 *      TPID mode will decide what kind of packet should be replaced outer TPID with configured one.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      tpid_mode - outer TPID mode at egress
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
 *      Inner TPID mode is as following:
 *      - EGR_TPID_MODE_IGR_UNTAG_AND_EGR_TAG
 *      - EGR_TPID_MODE_ALL_PACKETS
 */
extern int32
rtk_vlan_portEgrOuterTpidMode_set(uint32 unit, rtk_port_t port, rtk_vlan_egrTpidMode_t tpid_mode);

/* Function Name:
 *      rtk_vlan_portIgrOuterTpid_get
 * Description:
 *      Get outer TPIDs comparsion configration on specified port.
 * Input:
 *      unit           - unit id
 *      port           - port id
 * Output:
 *      pTpid_idx_mask - pointer to mask for index of tpid entry
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
 *      Specify which TPID to compare from TPID pool for parsing a outer-tagged packet.
 *      The valid mask bits of tpid_idx is bit[3:0].
 */
extern int32
rtk_vlan_portIgrOuterTpid_get(uint32 unit, rtk_port_t port, uint32 *pTpid_idx_mask);

/* Function Name:
 *      rtk_vlan_portIgrOuterTpid_set
 * Description:
 *      Set outer TPIDs comparsion configration on specified port.
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      tpid_idx_mask - mask for index of tpid entry
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
 *      Specify which TPID to compare from TPID pool for parsing a outer-tagged packet.
 *      The valid mask bits of tpid_idx is bit[3:0].
 */
extern int32
rtk_vlan_portIgrOuterTpid_set(uint32 unit, rtk_port_t port, uint32 tpid_idx_mask);

/* Function Name:
 *      rtk_vlan_portEgrOuterTpid_get
 * Description:
 *      Get outer TPID for outer tag encapsulation when transmiiting a outer-tagged pacekt.
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pTpid_idx - pointer to index of outer TPID
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
 *      (1) The valid range of tpid_idx is 0~3.
 *      (2) The outer TPID is used when transmiiting a outer-tagged pacekt only if it is received as
 *          an outer-untag pacekt.
 */
extern int32
rtk_vlan_portEgrOuterTpid_get(uint32 unit, rtk_port_t port, uint32 *pTpid_idx);

/* Function Name:
 *      rtk_vlan_portEgrOuterTpid_set
 * Description:
 *      Set outer TPID for outer tag encapsulation when transmiiting a outer-tagged pacekt.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      tpid_idx - index of outer TPID
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
 *      8328, 8390, 8380
 * Note:
 *      (1) The valid range of tpid_idx is 0~3.
 *      (2) The outer TPID is used when transmiiting a outer-tagged pacekt only if it is received as
 *          an outer-untag pacekt.
 */
extern int32
rtk_vlan_portEgrOuterTpid_set(uint32 unit, rtk_port_t port, uint32 tpid_idx);

/* Function Name:
 *      rtk_vlan_portIgrExtraTpid_get
 * Description:
 *      Get extra TPIDs comparsion configration on specified port.
 * Input:
 *      unit           - unit id
 *      port           - port id
 * Output:
 *      pTpid_idx_mask - pointer to mask for index of tpid entry
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
 *      The valid mask bits of tpid_idx is bit[3:0].
 */
extern int32
rtk_vlan_portIgrExtraTpid_get(uint32 unit, rtk_port_t port, uint32 *pTpid_idx_mask);

/* Function Name:
 *      rtk_vlan_portIgrExtraTpid_set
 * Description:
 *      Set extra TPIDs comparsion configration on specified port.
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      tpid_idx_mask - mask for index of tpid entry
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
 *      The valid mask bits of tpid_idx is bit[3:0].
 */
extern int32
rtk_vlan_portIgrExtraTpid_set(uint32 unit, rtk_port_t port, uint32 tpid_idx_mask);

/* Module Name    : Vlan                 */
/* Sub-module Name: Ingress tag handling */

/* Function Name:
 *      rtk_vlan_portIgrIgnoreInnerTagEnable_get
 * Description:
 *      Get enable status of ignore inner tag at ingress.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - enable status of ignore inner tag
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
rtk_vlan_portIgrIgnoreInnerTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_vlan_portIgrIgnoreInnerTagEnable_set
 * Description:
 *      Set enable status of ignore inner tag at ingress.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of ignore inner tag
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
rtk_vlan_portIgrIgnoreInnerTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_vlan_portIgrIgnoreOuterTagEnable_get
 * Description:
 *      Get enable status of ignore outer tag at ingress.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of ignore outer tag
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
rtk_vlan_portIgrIgnoreOuterTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_vlan_portIgrIgnoreOuterTagEnable_set
 * Description:
 *      Set enable status of ignore outer tag at ingress.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of ignore outer tag
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
rtk_vlan_portIgrIgnoreOuterTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Module Name    : Vlan                */
/* Sub-module Name: Egress tag handling */

/* Function Name:
 *      rtk_vlan_portEgrInnerTagEnable_get
 * Description:
 *      Get enable status of inner tag capability at egress.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of inner tag capability
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
 *      (1) Egress packet will send out with inner tag when following is true.
 *          - inner tag capability is enable
 *          - this port is tag member of specified vlan
 *      (2) For 8390 and 8380, the function is backward compatible RTL8328 APIs.
 */
extern int32
rtk_vlan_portEgrInnerTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_vlan_portEgrInnerTagEnable_set
 * Description:
 *      Set enable status of inner tag capability at egress.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of inner tag capability
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
 *      (1) Egress packet will send out with inner tag when following is true.
 *          - inner tag capability is enable
 *          - this port is tag member of specified vlan
 *      (2) For 8390 and 8380, the function is backward compatible RTL8328 APIs.
 */
extern int32
rtk_vlan_portEgrInnerTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_vlan_portEgrOuterTagEnable_get
 * Description:
 *      Get enable status of outer tag capability at egress.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of outer tag capability
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
 *      (1) Egress packet will send out with outer tag when following is true.
 *          - outer tag capability is enable
 *          - this port is tag member of specified vlan
 *      (2) For 8390 and 8380, the function is backward compatible RTL8328 APIs.
 */
extern int32
rtk_vlan_portEgrOuterTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_vlan_portEgrOuterTagEnable_set
 * Description:
 *      Set enable status of outer tag capability at egress.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of outer tag capability
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
 *      (1) Egress packet will send out with outer tag when following is true.
 *          - outer tag capability is enable
 *          - this port is tag member of specified vlan
 *      (2) For 8390 and 8380, the function is backward compatible RTL8328 APIs.
 */
extern int32
rtk_vlan_portEgrOuterTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_vlan_portIgrExtraTagEnable_get
 * Description:
 *      Get enable status of recognizing extra tag at ingress.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of recognizing extra tag
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
 *      None
 */
extern int32
rtk_vlan_portIgrExtraTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_vlan_portIgrExtraTagEnable_set
 * Description:
 *      Set enable status of recognizing extra tag at ingress.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of recognizing extra tag
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
 *      None
 */
extern int32
rtk_vlan_portIgrExtraTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_vlan_portEgrExtraTagEnable_get
 * Description:
 *      Get enable status of recognizing extra tag at egress.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of recognizing extra tag
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
rtk_vlan_portEgrExtraTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_vlan_portEgrExtraTagEnable_set
 * Description:
 *      Set enable status of recognizing extra tag at egress.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of recognizing extra tag
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
rtk_vlan_portEgrExtraTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_vlan_portEgrInnerTagSts_get
 * Description:
 *      Set inner tag status of egress port.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pSts - tag status
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
 *      Egress packet will send out with inner tag when following is true.
 *      - inner tag status is specified to be tagged
 *      - this port is not in VLAN untag set
 */
extern int32
rtk_vlan_portEgrInnerTagSts_get(uint32 unit, rtk_port_t port, rtk_vlan_tagSts_t *pSts);

/* Function Name:
 *      rtk_vlan_portEgrInnerTagSts_set
 * Description:
 *      Set inner tag status of egress port.
 * Input:
 *      unit - unit id
 *      port - port id
 *      sts  - tag status
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
 *      Egress packet will send out with inner tag when following is true.
 *      - inner tag status is specified to be tagged
 *      - this port is not in VLAN untag set
 */
extern int32
rtk_vlan_portEgrInnerTagSts_set(uint32 unit, rtk_port_t port, rtk_vlan_tagSts_t sts);

/* Function Name:
 *      rtk_vlan_portEgrOuterTagSts_get
 * Description:
 *      Set inner tag status of egress port.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pSts - tag status
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
 *      Egress packet will send out with outer tag when following is true.
 *      - outer tag status is specified to be tagged
 *      - this port is not in VLAN untag set
 */
extern int32
rtk_vlan_portEgrOuterTagSts_get(uint32 unit, rtk_port_t port, rtk_vlan_tagSts_t *pSts);

/* Function Name:
 *      rtk_vlan_portEgrOuterTagSts_set
 * Description:
 *      Set inner tag status of egress port.
 * Input:
 *      unit - unit id
 *      port - port id
 *      sts  - tag status
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
 *      Egress packet will send out with outer tag when following is true.
 *      - outer tag status is specified to be tagged
 *      - this port is not in VLAN untag set
 */
extern int32
rtk_vlan_portEgrOuterTagSts_set(uint32 unit, rtk_port_t port, rtk_vlan_tagSts_t sts);

/* Function Name:
 *      rtk_vlan_portEgrInnerVidSource_get
 * Description:
 *      Get source of vid field in inner tag.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pVidSource - pointer to source of vid field in inner tag
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
 *      Vid source is as following
 *      - TAG_SOURCE_FROM_ALE
 *      - TAG_SOURCE_FROM_ORIG_INNER_TAG
 *      - TAG_SOURCE_FROM_ORIG_OUTER_TAG
 */
extern int32
rtk_vlan_portEgrInnerVidSource_get(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t *pVidSource);

/* Function Name:
 *      rtk_vlan_portEgrInnerVidSource_set
 * Description:
 *      Set source of vid field in inner tag.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      vidSource - source of vid field in inner tag
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
 *      Vid source is as following
 *      - TAG_SOURCE_FROM_ALE
 *      - TAG_SOURCE_FROM_ORIG_INNER_TAG
 *      - TAG_SOURCE_FROM_ORIG_OUTER_TAG
 */
extern int32
rtk_vlan_portEgrInnerVidSource_set(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t vidSource);

/* Function Name:
 *      rtk_vlan_portEgrInnerPriSource_get
 * Description:
 *      Get source of priority field in inner tag.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pPriSource - pointer to source of priority field in inner tag
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
 *      Vid source is as following
 *      - TAG_SOURCE_FROM_ORIG_INNER_TAG
 *      - TAG_SOURCE_FROM_ORIG_OUTER_TAG
 *      - TAG_SOURCE_NULL
 */
extern int32
rtk_vlan_portEgrInnerPriSource_get(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t *pPriSource);

/* Function Name:
 *      rtk_vlan_portEgrInnerPriSource_set
 * Description:
 *      Set source of priority field in inner tag.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      priSource - source of priority field in inner tag
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
 *      Vid source is as following
 *      - TAG_SOURCE_FROM_ORIG_INNER_TAG
 *      - TAG_SOURCE_FROM_ORIG_OUTER_TAG
 *      - TAG_SOURCE_NULL
 */
extern int32
rtk_vlan_portEgrInnerPriSource_set(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t priSource);

/* Function Name:
 *      rtk_vlan_portEgrOuterVidSource_get
 * Description:
 *      Get source of vid field in outer tag.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pVidSource - pointer to source of vid field in outer tag
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
 *      Vid source is as following
 *      - TAG_SOURCE_FROM_ALE
 *      - TAG_SOURCE_FROM_ORIG_INNER_TAG
 *      - TAG_SOURCE_FROM_ORIG_OUTER_TAG
 */
extern int32
rtk_vlan_portEgrOuterVidSource_get(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t *pVidSource);

/* Function Name:
 *      rtk_vlan_portEgrOuterVidSource_set
 * Description:
 *      Set source of vid field in outer tag.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      vidSource - source of vid field in outer tag
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
 *      Vid source is as following
 *      - TAG_SOURCE_FROM_ALE
 *      - TAG_SOURCE_FROM_ORIG_INNER_TAG
 *      - TAG_SOURCE_FROM_ORIG_OUTER_TAG
 */
extern int32
rtk_vlan_portEgrOuterVidSource_set(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t vidSource);

/* Function Name:
 *      rtk_vlan_portEgrOuterPriSource_get
 * Description:
 *      Get source of priority field in outer tag.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pPriSource - pointer to source of priority field in outer tag
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
 *      Vid source is as following
 *      - TAG_SOURCE_FROM_ORIG_INNER_TAG
 *      - TAG_SOURCE_FROM_ORIG_OUTER_TAG
 *      - TAG_SOURCE_NULL
 */
extern int32
rtk_vlan_portEgrOuterPriSource_get(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t *pPriSource);

/* Function Name:
 *      rtk_vlan_portEgrOuterPriSource_set
 * Description:
 *      Set source of priority field in outer tag.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      priSource - source of priority field in outer tag
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
 *      Vid source is as following
 *      - TAG_SOURCE_FROM_ORIG_INNER_TAG
 *      - TAG_SOURCE_FROM_ORIG_OUTER_TAG
 *      - TAG_SOURCE_NULL
 */
extern int32
rtk_vlan_portEgrOuterPriSource_set(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t priSource);

/* Module Name    : Vlan                */
/* Sub-module Name: Tag format handling */

/* Function Name:
 *      rtk_vlan_tagMode_get
 * Description:
 *      Get vlan tagged mode of the port from the specified device.
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pTag_mode - pointer buffer of vlan tagged mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389
 * Note:
 *      The vlan tagged mode as following:
 *      - VLAN_TAG_MODE_ORIGINAL        (depend on chip normal decision)
 *      - VLAN_TAG_MODE_KEEP_FORMAT     (keep ingress format to egress)
 *      - VLAN_TAG_MODE_PRI             (always priority tag out)
 */
extern int32
rtk_vlan_tagMode_get(uint32 unit, rtk_port_t port, rtk_vlan_tagMode_t *pTag_mode);

/* Function Name:
 *      rtk_vlan_tagMode_set
 * Description:
 *      Set vlan tagged mode of the port to the specified device.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      tag_mode - vlan tagged mode
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
 *      8389
 * Note:
 *      The vlan tagged mode as following:
 *      - VLAN_TAG_MODE_ORIGINAL        (depend on chip normal decision)
 *      - VLAN_TAG_MODE_KEEP_FORMAT     (keep ingress format to egress)
 *      - VLAN_TAG_MODE_PRI             (always priority tag out)
 */
extern int32
rtk_vlan_tagMode_set(uint32 unit, rtk_port_t port, rtk_vlan_tagMode_t tag_mode);

/* Function Name:
 *      rtk_vlan_portIgrTagKeepEnable_get
 * Description:
 *      Get enable status of keep tag format at ingress.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pKeepOuter - enable status of keep outer tag format
 *      pKeepInner - enable status of keep inner tag format
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointers
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      Packet will be transmitted with original tag format when following condition are true.
 *      - Enable keep tag format at ingress
 *      - Enable keep tag format at egress
 */
extern int32
rtk_vlan_portIgrTagKeepEnable_get(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    *pKeepOuter,
    rtk_enable_t    *pKeepInner);

/* Function Name:
 *      rtk_vlan_portIgrTagKeepEnable_set
 * Description:
 *      Set enable status of keep tag format at ingress.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      keepOuter - enable status of keep outer tag format
 *      keepInner - enable status of keep inner tag format
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
 *      Packet will be transmitted with original tag format when following condition are true.
 *      - Enable keep tag format at ingress
 *      - Enable keep tag format at egress
 */
extern int32
rtk_vlan_portIgrTagKeepEnable_set(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    keepOuter,
    rtk_enable_t    keepInner);

/* Function Name:
 *      rtk_vlan_portEgrTagKeepEnable_get
 * Description:
 *      Get enable status of keep tag format at egress.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pKeepOuter - pointer to enable status of keep outer tag format
 *      pKeepInner - pointer to enable status of keep inner tag format
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
 *      Packet will be transmitted with original tag format when following condition are true.
 *      - Enable keep tag format at ingress
 *      - Enable keep tag format at egress
 */
extern int32
rtk_vlan_portEgrTagKeepEnable_get(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    *pKeepOuter,
    rtk_enable_t    *pKeepInner);

/* Function Name:
 *      rtk_vlan_portEgrTagKeepEnable_set
 * Description:
 *      Set enable status of keep tag format at egress.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      keepOuter - enable status of keep outer tag format
 *      keepInner - enable status of keep inner tag format
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
 *      Packet will be transmitted with original tag format when following condition are true.
 *      - Enable keep tag format at ingress
 *      - Enable keep tag format at egress
 */
extern int32
rtk_vlan_portEgrTagKeepEnable_set(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    keepOuter,
    rtk_enable_t    keepInner);

/*
 * MISC
 */

/* Function Name:
 *      rtk_vlan_fwdMode_get
 * Description:
 *      Get packet forwarding mode on specified vlan.
 *      Packet can be forwarded by ALE decision or forward to members of vlan.
 * Input:
 *      unit  - unit id
 *      vid   - vlan id
 * Output:
 *      pMode - pointer to packet forwarding mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_VLAN_VID     - invalid vlan id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of vid is 0~4095
 *      (2) Packet forwarding mode is as following:
 *          - VLAN_FWD_ON_ALE
 *          - VLAN_FWD_ON_VLAN_MEMBER
 */
extern int32
rtk_vlan_fwdMode_get(uint32 unit, rtk_vlan_t vid, rtk_vlan_fwdMode_t *pMode);

/* Function Name:
 *      rtk_vlan_fwdMode_set
 * Description:
 *      Set packet forwarding mode on specified vlan.
 *      Packet can be forwarded by ALE decision or forward to members of vlan.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 *      mode - packet forwarding mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_VLAN_VID - invalid vlan id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of vid is 0~4095
 *      (2) Packet forwarding mode is as following:
 *          - VLAN_FWD_ON_ALE
 *          - VLAN_FWD_ON_VLAN_MEMBER
 */
extern int32
rtk_vlan_fwdMode_set(uint32 unit, rtk_vlan_t vid, rtk_vlan_fwdMode_t mode);

/* Function Name:
 *      rtk_vlan_igrVlanCnvtBlkMode_get
 * Description:
 *      Get the operation mode of ingress VLAN conversion table block.
 * Input:
 *      unit    - unit id
 *      blk_idx - block index
 * Output:
 *      pMode   - operation mode of ingress VLAN conversion block
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - block index is out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      (1) Valid block index ranges from 0 to 3.
 *      (2) Ingress VLAN conversion table block can be used for doing ingress VLAN conversion
 *         or MAC-basd VLAN or IP-Subnet-based VLAN.
 */
extern int32
rtk_vlan_igrVlanCnvtBlkMode_get(uint32 unit, uint32 blk_idx, rtk_vlan_igrVlanCnvtBlk_mode_t *pMode);

/* Function Name:
 *      rtk_vlan_igrVlanCnvtBlkMode_set
 * Description:
 *      Set the operation mode of ingress VLAN conversion table block.
 * Input:
 *      unit    - unit id
 *      blk_idx - block index
 *      mode    - operation mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - block index is out of range
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8390
 * Note:
 *      (1) Valid block index ranges from 0 to 3 and each block contains 256 entries.
 *      (2) Ingress VLAN conversion table block can be used for doing ingress VLAN conversion
 *         or MAC-basd VLAN or IP-Subnet-based VLAN.
 */
extern int32
rtk_vlan_igrVlanCnvtBlkMode_set(uint32 unit, uint32 blk_idx, rtk_vlan_igrVlanCnvtBlk_mode_t mode);

/* Function Name:
 *      rtk_vlan_igrVlanCnvtEntry_get
 * Description:
 *      Get ingress VLAN conversion entry.
 * Input:
 *      unit  - unit id
 *      index - entry index
 * Output:
 *      pData - configuration of ingress VLAN conversion entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_OUT_OF_RANGE         - entry index is out of range
 *      RT_ERR_VLAN_C2SC_BLOCK_MODE - try to set/get a entry to an incompatiable block
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      (1) Totally 1024 entries are supported but the entries are shared by ingress VLAN
 *          conversion, MAC-based VLAN and IP-Subnet-based VLAN functions per-block based.
 *      (2) The corresponding block must be in ingress VLAN conversion mode before calling this function,
 *          refer to rtk_vlan_igrVlanCnvtBlkMode_set.
 */
extern int32
rtk_vlan_igrVlanCnvtEntry_get(uint32 unit, uint32 index, rtk_vlan_igrVlanCnvtEntry_t *pData);

/* Function Name:
 *      rtk_vlan_igrVlanCnvtEntry_set
 * Description:
 *      Set ingress VLAN conversion entry.
 * Input:
 *      unit  - unit id
 *      index - entry index
 * Output:
 *      pData - configuration of ingress VLAN conversion entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_OUT_OF_RANGE         - entry index is out of range
 *      RT_ERR_VLAN_C2SC_BLOCK_MODE - try to set/get a entry to an incompatiable block
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_QOS_1P_PRIORITY      - Invalid 802.1p priority
 *      RT_ERR_VLAN_TPID_INDEX      - Invalid TPID index
 *      RT_ERR_INPUT                - invalid input parameter
 * Applicable:
 *      8390
 * Note:
 *      (1) Totally 1024 entries are supported but the entries are shared by ingress VLAN
 *          conversion, MAC-based VLAN and IP-Subnet-based VLAN functions per-block based.
 *      (2) The corresponding block must be in ingress VLAN conversion mode before calling this function,
 *          refer to rtk_vlan_igrVlanCnvtBlkMode_set.
 */
extern int32
rtk_vlan_igrVlanCnvtEntry_set(uint32 unit, uint32 index, rtk_vlan_igrVlanCnvtEntry_t *pData);

/* Function Name:
 *      rtk_vlan_egrVlanCnvtDblTagEnable_get
 * Description:
 *      Enable egress VLAN conversion for dobule(outer+inner) tagged packet.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) Enable egress VLAN conversion for dobule tagged packet.
 *      (2) If the function is disabled, egress VLAN conversion only applies to single tagged packet.
 */
extern int32
rtk_vlan_egrVlanCnvtDblTagEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_vlan_egrVlanCnvtDblTagEnable_set
 * Description:
 *      Enable egress VLAN conversion for dobule(outer+inner) tagged packet.
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
 *      (1) Enable egress VLAN conversion for dobule tagged packet.
 *      (2) If the function is disabled, egress VLAN conversion only applies to single tagged packet.
 */
extern int32
rtk_vlan_egrVlanCnvtDblTagEnable_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      rtk_vlan_egrVlanCnvtVidSource_get
 * Description:
 *      Specify the VID source for doing egress VLAN conversion.
 * Input:
 *      unit - unit id
 * Output:
 *      pSrc - pointer to VID source
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      (1) VID source can be either from inner tag or outer tag.
 *      (2) The selected VID will be used as comparing key for doing egress VLAN conversion.
 */
extern int32
rtk_vlan_egrVlanCnvtVidSource_get(uint32 unit, rtk_l2_vlanMode_t *pSrc);

/* Function Name:
 *      rtk_vlan_egrVlanCnvtVidSource_set
 * Description:
 *      Specify the VID source for doing egress VLAN conversion.
 * Input:
 *      unit - unit id
 *      src  - VID source
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
 *      (1) VID source can be either from inner tag or outer tag.
 *      (2) The selected VID will be used as comparing key for doing egress VLAN conversion.
 */
extern int32
rtk_vlan_egrVlanCnvtVidSource_set(uint32 unit, rtk_l2_vlanMode_t src);

/* Function Name:
 *      rtk_vlan_egrVlanCnvtEntry_get
 * Description:
 *      Set egress VLAN conversion entry.
 * Input:
 *      unit  - unit id
 *      index - entry index
 * Output:
 *      pData - configuration of egress VLAN conversion entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - entry index is out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      Totally 1024 entries are supported.
 */
extern int32
rtk_vlan_egrVlanCnvtEntry_get(uint32 unit, uint32 index, rtk_vlan_egrVlanCnvtEntry_t *pData);

/* Function Name:
 *      rtk_vlan_egrVlanCnvtEntry_set
 * Description:
 *      Set egress VLAN conversion entry.
 * Input:
 *      unit  - unit id
 *      index - entry index
 * Output:
 *      pData - configuration of egress VLAN conversion entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_OUT_OF_RANGE         - entry index is out of range
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_QOS_1P_PRIORITY      - Invalid 802.1p priority
 *      RT_ERR_VLAN_TPID_INDEX      - Invalid TPID index
 *      RT_ERR_INPUT                - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      Totally 1024 entries are supported.
 */
extern int32
rtk_vlan_egrVlanCnvtEntry_set(uint32 unit, uint32 index, rtk_vlan_egrVlanCnvtEntry_t *pData);

/* Function Name:
 *      rtk_vlan_portVlanAggrEnable_get
 * Description:
 *      Enable N:1 VLAN aggregation support on egress port.
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
 *      8390, 8380
 * Note:
 *      (1) When the N:1 VLAN aggregation function is enabled on a egress port, the inner VID
 *          of the downstream packet will be replaced by the VID which learnt in L2 table from
 *          upstream packet.
 *      (2) N:1 VLAN replacement only applies to the inner tagged and known unicast packet.
 */
extern int32
rtk_vlan_portVlanAggrEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_vlan_portVlanAggrEnable_set
 * Description:
 *      Enable N:1 VLAN aggregation support on egress port.
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
 *      8390, 8380
 * Note:
 *      (1) When the N:1 VLAN aggregation function is enabled on a egress port, the inner VID
 *          of the downstream packet will be replaced by the VID which learnt in L2 table from
 *          upstream packet.
 *      (2) N:1 VLAN replacement only applies to the inner tagged and known unicast packet.
 */
extern int32
rtk_vlan_portVlanAggrEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_vlan_leakyStpFilter_get
 * Description:
 *      Get leaky STP filter status for multicast leaky is enabled.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to status of leaky STP filter
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
rtk_vlan_leakyStpFilter_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_vlan_leakyStpFilter_set
 * Description:
 *      Set leaky STP filter status for multicast leaky is enabled.
 * Input:
 *      unit   - unit id
 *      enable - status of leaky STP filter
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
rtk_vlan_leakyStpFilter_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      rtk_vlan_except_get
 * Description:
 *      Get VLAN except action.
 * Input:
 *      unit    - unit id
 * Output:
 *      pAction - pointer to action of VLAN except
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      (1) VLAN exception is asserted if
 *          received packet with VID=4095 or its VLAN member=0.
 *      (2) Forwarding action is as following
 *          - ACTION_DROP
 *          - ACTION_FORWARD
 *          - ACTION_TRAP2CPU
 */
extern int32
rtk_vlan_except_get(uint32 unit, rtk_action_t *pAction);

/* Function Name:
 *      rtk_vlan_except_set
 * Description:
 *      Set VLAN except action.
 * Input:
 *      unit   - unit id
 *      action - action of VLAN except
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
 *      (1) VLAN exception is asserted if
 *          received packet with VID=4095 or its VLAN member=0.
 *      (2) Forwarding action is as following
 *          - ACTION_DROP
 *          - ACTION_FORWARD
 *          - ACTION_TRAP2CPU
 */
extern int32
rtk_vlan_except_set(uint32 unit, rtk_action_t action);

/* Function Name:
 *      rtk_vlan_portIgrCnvtDfltAct_get
 * Description:
 *      Get action for packet that doesn't hit ingress ACL and C2SC.
 * Input:
 *      unit    - unit id
 *      port    - port id for configure
 * Output:
 *      pAction - pointer to VLAN conversion default action
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
 *      Forwarding action is as following
 *      - ACTION_DROP
 *      - ACTION_FORWARD
 */
extern int32
rtk_vlan_portIgrCnvtDfltAct_get(uint32 unit, rtk_port_t port, rtk_action_t *pAction);

/* Function Name:
 *      rtk_vlan_portIgrCnvtDfltAct_set
 * Description:
 *      Set action for packet that doesn't hit ingress ACL and C2SC.
 * Input:
 *      unit   - unit id
 *      port   - port id for configure
 *      action - VLAN conversion default action
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
 *      Forwarding action is as following
 *      - ACTION_DROP
 *      - ACTION_FORWARD
 */
extern int32
rtk_vlan_portIgrCnvtDfltAct_set(uint32 unit, rtk_port_t port, rtk_action_t action);

/* Function Name:
 *      rtk_vlan_portIgrTagKeepType_get
 * Description:
 *      Get keep type of keep tag format at igress.
 * Input:
 *      unit           - unit id
 *      port           - port id
 * Output:
 *      pKeeptypeOuter - pointer to enable status of keep outer tag format
 *      pKeeptypeInner - pointer to enable status of keep inner tag format
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
  * Applicable:
 *      8380
 * Note:
 *      Packet will be transmitted with original tag format when following condition are true.
 *      - Enable keep tag format at ingress
 *      - Enable keep tag format at egress
 */
extern int32
rtk_vlan_portIgrTagKeepType_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_vlan_tagKeepType_t  *pKeeptypeOuter,
    rtk_vlan_tagKeepType_t  *pKeeptypeInner);

/* Function Name:
 *      rtk_vlan_portIgrTagKeepType_set
 * Description:
 *      Set keep type of keep tag format at igress.
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      keeptypeOuter - keep type of keep outer tag format
 *      keeptypeInner - keep type of keep inner tag format
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
 *      8380
 * Note:
 *      Packet will be transmitted with original tag format when following condition are true.
 *      - Set keep type of keep tag format at ingress
 *      - Set keep type of keep tag format at egress
 */
extern int32
rtk_vlan_portIgrTagKeepType_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_vlan_tagKeepType_t  keeptypeOuter,
    rtk_vlan_tagKeepType_t  keeptypeInner);

/* Function Name:
 *      rtk_vlan_portEgrTagKeepType_get
 * Description:
 *      Get keep type of keep tag format at egress.
 * Input:
 *      unit           - unit id
 *      port           - port id
 * Output:
 *      pKeeptypeOuter - pointer to enable status of keep outer tag format
 *      pKeeptypeInner - pointer to enable status of keep inner tag format
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      Packet will be transmitted with original tag format when following condition are true.
 *      - Enable keep tag format at ingress
 *      - Enable keep tag format at egress
 */
extern int32
rtk_vlan_portEgrTagKeepType_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_vlan_tagKeepType_t  *pKeeptypeOuter,
    rtk_vlan_tagKeepType_t  *pKeeptypeInner);

/* Function Name:
 *      rtk_vlan_portEgrTagKeepType_set
 * Description:
 *      Set keep type of keep tag format at egress.
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      keeptypeOuter - keep type of keep outer tag format
 *      keeptypeInner - keep type of keep inner tag format
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
 *      8380
 * Note:
 *      Packet will be transmitted with original tag format when following condition are true.
 *      - Set keep type of keep tag format at ingress
 *      - Set keep type of keep tag format at egress
 */
extern int32
rtk_vlan_portEgrTagKeepType_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_vlan_tagKeepType_t  keeptypeOuter,
    rtk_vlan_tagKeepType_t  keeptypeInner);

/* Function Name:
 *      rtk_vlan_portVlanAggrVidSource_get
 * Description:
 *      Get port ingress learning int address table and egress conversion VID selection inner VID or outer VID.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pSrc - pointer to source selection
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      None
 */
extern int32
rtk_vlan_portVlanAggrVidSource_get(uint32 unit, rtk_port_t port, rtk_l2_vlanMode_t *pSrc);

/* Function Name:
 *      rtk_vlan_portVlanAggrVidSource_set
 * Description:
 *      Set port ingress learning int address table and egress conversion VID selection inner VID or outer VID.
 * Input:
 *      unit - unit id
 *      port - port id
 *      src  - vlan mode source
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
 *      8380
 * Note:
 *      None
 */
extern int32
rtk_vlan_portVlanAggrVidSource_set(uint32 unit, rtk_port_t port, rtk_l2_vlanMode_t src);

/* Function Name:
 *      rtk_vlan_portVlanAggrPriTagVidSource_get
 * Description:
 *      Get ingress port priority-tagged packet learning VID.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pSrc - pointer to priority-tagged packet learning VID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      None
 */
extern int32
rtk_vlan_portVlanAggrPriTagVidSource_get(uint32 unit, rtk_port_t port, rtk_vlan_priTagVidSrc_t *pSrc);

/* Function Name:
 *      rtk_vlan_portVlanAggrPriTagVidSource_set
 * Description:
 *      Set ingress port priority-tagged packet learning VID 0 or port-based VID.
 * Input:
 *      unit - unit id
 *      port - port id
 *      src  - priority-tagged packet learning VID
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
 *      8380
 * Note:
 *      None
 */
extern int32
rtk_vlan_portVlanAggrPriTagVidSource_set(uint32 unit, rtk_port_t port, rtk_vlan_priTagVidSrc_t src);

/* Function Name:
 *      rtk_vlan_egrVlanCnvtVidSource_get
 * Description:
 *      Specify the VID source for doing egress VLAN conversion.
 * Input:
 *      unit - unit id
 * Output:
 *      pSrc - pointer to VID source
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      None
 */
extern int32
rtk_vlan_portEgrVlanCnvtVidSource_get(uint32 unit, rtk_port_t port, rtk_l2_vlanMode_t *pSrc);

/* Function Name:
 *      rtk_vlan_egrVlanCnvtVidSource_set
 * Description:
 *      Specify the VID source for doing egress VLAN conversion.
 * Input:
 *      unit - unit id
 *      src  - VID source
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8380
 * Note:
 *      None
 */
extern int32
rtk_vlan_portEgrVlanCnvtVidSource_set(uint32 unit, rtk_port_t port, rtk_l2_vlanMode_t src);

/* Function Name:
 *      rtk_vlan_egrVlanCnvtVidTarget_get
 * Description:
 *      Specify the VID target for doing egress VLAN conversion.
 * Input:
 *      unit - unit id
 * Output:
 *      pTgt - pointer to VID source
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      None
 */
extern int32
rtk_vlan_portEgrVlanCnvtVidTarget_get(uint32 unit, rtk_port_t port, rtk_l2_vlanMode_t *pTgt);

/* Function Name:
 *      rtk_vlan_egrVlanCnvtVidTarget_set
 * Description:
 *      Specify the VID target for doing egress VLAN conversion.
 * Input:
 *      unit - unit id
 *      tgt  - VID source
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8380
 * Note:
 *      None
 */
extern int32
rtk_vlan_portEgrVlanCnvtVidTarget_set(uint32 unit, rtk_port_t port, rtk_l2_vlanMode_t tgt);

/* Function Name:
 *      rtk_vlan_portEgrVlanCnvtLookupMissAct_get
 * Description:
 *      Specify egress port VID conversion table lookup miss action.
 * Input:
 *      unit - unit id
 * Output:
 *      pAct - pointer to action selection
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      None
 */
extern int32
rtk_vlan_portEgrVlanCnvtLookupMissAct_get(uint32 unit, rtk_port_t port, rtk_vlan_lookupMissAct_t *pAct);

/* Function Name:
 *      rtk_vlan_portEgrVlanCnvtLkmissAct_set
 * Description:
 *      Specify egress port VID conversion table lookup miss action.
 * Input:
 *      unit - unit id
 *      act  - action selection
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8380
 * Note:
 *      None
 */
extern int32
rtk_vlan_portEgrVlanCnvtLookupMissAct_set(uint32 unit, rtk_port_t port, rtk_vlan_lookupMissAct_t act);

/* Function Name:
 *      rtk_vlan_igrVlanCnvtEntry_delAll
 * Description:
 *      Delete all ingress VLAN conversion entry.
 * Input:
 *      unit  - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 * Applicable:
 *      8390
 * Note:
 *      All ingress VLAN conversion entries are removed regardless of the operation mode.
 */
extern int32
rtk_vlan_igrVlanCnvtEntry_delAll(uint32 unit);

/* Function Name:
 *      rtk_vlan_egrVlanCnvtEntry_delAll
 * Description:
 *      Delete all egress VLAN conversion entry.
 * Input:
 *      unit  - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_vlan_egrVlanCnvtEntry_delAll(uint32 unit);

/* Function Name:
 *      rtk_vlan_egrVlanCnvtRangeCheckVid_get
 * Description:
 *      Get the configuration of VID range check for egress VLAN conversion.
 * Input:
 *      unit  - unit id
 *      index - entry index
 * Output:
 *      pData - configuration of VID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_vlan_egrVlanCnvtRangeCheckVid_get(
    uint32                                  unit,
    uint32                                  index,
    rtk_vlan_egrVlanCnvtRangeCheck_vid_t    *pData);

/* Function Name:
 *      rtk_vlan_egrVlanCnvtRangeCheckVid_set
 * Description:
 *      Set the configuration of VID range check for egress VLAN conversion.
 * Input:
 *      unit  - unit id
 *      index - entry index
 *      pData - configuration of VID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_vlan_egrVlanCnvtRangeCheckVid_set(
    uint32                                  unit,
    uint32                                  index,
    rtk_vlan_egrVlanCnvtRangeCheck_vid_t    *pData);

#endif /* __RTK_VLAN_H__ */
