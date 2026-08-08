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
 * $Revision: 6401 $
 * $Date: 2009-10-14 16:03:12 +0800 (Wed, 14 Oct 2009) $
 *
 * Purpose : Definition those public filter APIs and its data type in the SDK.
 * 
 * Feature : The file have include the following module and sub-modules
 *           1) Flow table
 *           2) Ingress ACL
 * 
 */ 


#ifndef __DAL_SSW_FILTER_H__
#define __DAL_SSW_FILTER_H__


/*
 * Include Files 
 */
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <rtk/filter.h>


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

/* Module Name     : filter */
/* Sub-module Name : global */

/* Function Name:
 *      dal_ssw_filter_blkCutline_get
 * Description:
 *      Get the cutline value from the specified device.
 * Input:
 *      unit      - unit id
 * Output:
 *      pCutline - pointer buffer of cutline value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. RTL8329/RTL8389: default cutline value is 4, mean block 0-3 is used by flow table
 *         and 4-7 is used by ingress acl table.
 *      2. cutline value is configurable and 0~(cutline-1) is used by flow table
 *         and cutline~RTK_MAX_NUM_OF_FILTER_BLOCK is used by ingress acl table.
 */
extern int32
dal_ssw_filter_blkCutline_get(uint32 unit, uint32 *pCutline);


/* Function Name:
 *      dal_ssw_filter_blkCutline_set
 * Description:
 *      Set the cutline value to the specified device.
 * Input:
 *      unit    - unit id
 *      cutline - cutline value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_FILTER_CUTLINE - invalid cutline value
 * Note:
 *      1. RTL8329/RTL8389: default cutline value is 4, mean block 0-3 is used by flow table
 *         and 4-7 is used by ingress acl table.
 *      2. cutline value is configurable and 0~(cutline-1) is used by flow table
 *         and cutline~RTK_MAX_NUM_OF_FILTER_BLOCK is used by ingress acl table.
 */
extern int32
dal_ssw_filter_blkCutline_set(uint32 unit, uint32 cutline);

/* Function Name:
 *      dal_ssw_filter_pieEnable_get
 * Description:
 *      Get the PIE enable status from the specified device.
 * Input:
 *      unit      - unit id
 * Output:
 *      pEnable - pointer of enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None.
 */
extern int32
dal_ssw_filter_pieEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_ssw_filter_pieEnable_set
 * Description:
 *      Set the PIE enable status to the specified device.
 * Input:
 *      unit    - unit id
 *      enable  - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_UNIT_ID        - invalid unit id
 * Note:
 *      None.
 */
extern int32
dal_ssw_filter_pieEnable_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      dal_ssw_filter_init
 * Description:
 *      Initialize filter module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      Must initialize filter module before calling any filter APIs.
 */
extern int32
dal_ssw_filter_init(uint32 unit);


/* Module Name     : filter        */
/* Sub-module Name : pattern match */

/* Function Name:
 *      dal_ssw_filter_patternMatch_get
 * Description:
 *      Get per port pattern match from the specified device.
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pMode    - pointer buffer of pattern match mode
 *      pPattern - pointer buffer of pattern to be matched.
 *      pMask    - pointer buffer of pattrn character care mask.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The pattern match mode as following:
 *          - PM_4BYTE_MODE       (two 4-bytes mode)
 *          - PM_8BYTE_MODE       (one 8-bytes mode)
 */
extern int32
dal_ssw_filter_patternMatch_get(
    uint32                          unit,
    rtk_port_t                      port,
    rtk_filter_patternMatch_mode_t  *pMode,
    uint8                           *pPattern,
    uint32                          *pMask);


/* Function Name:
 *      dal_ssw_filter_patternMatch_set
 * Description:
 *      Set per port pattern match to the specified device.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      mode      - pattern match mode
 *      pPattern - pointer buffer of pattern to be matched.
 *      mask      - pattrn character care mask.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_PM_LENGTH    - invalid pattern length (the pattern string is not 8 bytes)
 *      RT_ERR_PM_MASK      - invalid pattern match mask (the mask value is more than 8 bits)
 *      RT_ERR_PM_MODE      - invalid pattern match mode
 * Note:
 *      1. The pattern match mode as following:
 *          - PM_4BYTE_MODE       (two 4-bytes mode)
 *          - PM_8BYTE_MODE       (one 8-bytes mode)
 *
 *      2. The API can set ether in 4-Byte mode or 8-Byte mode.
 *         In 4-Byte mode, pattern of set0 should place in pattern[0~3] and
 *         that of set1 should place in pattern[4~7].
 *         In 4-Byte mode, the first byte of set0 and set1 must care even
 *         if you don't speciy set1.
 *         This is due to ASIC limitation.
 *         If you really want not set1 to be compared, just set the care-bit 
 *         of "PatternMatch" field in ACL to 2 ('10' in binary).
 *
 *         In 8-Byte mode, the first byte must care.
 *         
 *         Here are some 4-Byte mode examples:
 *             Example 1. To compare "abcd" and "efgh"
 *                 input parameter:
 *                 mode        = 0
 *                 pattern[]   = "abcdefgh"
 *                 mask        = 0xff
 *             Example 2. To compare "abcd" and "efg"
 *                 input parameter:
 *                 mode        = 0
 *                 pattern[]   = "abcdefg " <---   ### Leave 1 blank
 *                 mask        = 0xfe
 *             Example 3. To compare "abc" and "efgh"
 *                 input parameter:
 *                 mode        = 0
 *                 pattern[]   = "abc efgh"
 *                 mask        = 0xef
 *             Example 4. To compare "abc" and "efg"
 *                 input parameter:
 *                 mode        = 0
 *                 pattern[]   = "abc efg "
 *                 mask        = 0xee          
 *             Example 5. To compare "abcd"
 *                 input parameter:
 *                 mode        = 0
 *                 pattern[]   = "abcd    " <---   ### Leave 4 blank
 *                 mask        = 0xf1
 *             Example 6. To compare "abc"
 *                 input parameter:
 *                 mode        = 0
 *                 pattern[]   = "abc     " <---   ### Leave 5 blank
 *                 mask        = 0xe1
 *             Example 7. To compare "a*cd"
 *                 input parameter:
 *                 mode        = 0
 *                 pattern[]   = "a cd    " <---   ### There are 5 blank
 *                 mask        = 0xb1
 *         
 *         In above case 5,6 and 7, although set1 is not specified, we still
 *         care its first byte.
 *         
 *         
 *         Here are some 8-Byte mode examples:
 *             Example 1. To compare "abcdefgh"
 *                 input parameter:
 *                 mode        = 1
 *                 pattern[]   = "abcdefgh"
 *                 mask        = 0xff
 *             Example 2. To compare "abcde"
 *                 input parameter:
 *                 mode        = 1
 *                 pattern[]   = "abcde   "
 *                 mask        = 0xf8
 *             Example 3. To compare "abc*e"
 *                 input parameter:
 *                 mode        = 1
 *                 pattern[]   = "abcde   "
 *                 mask        = 0xe8
 */
extern int32
dal_ssw_filter_patternMatch_set(
    uint32                          unit,
    rtk_port_t                      port,
    rtk_filter_patternMatch_mode_t  mode,
    uint8                           *pPattern,
    uint32                          mask);


/* Module Name     : filter     */
/* Sub-module Name : flow table */

/* Function Name:
 *      dal_ssw_filter_flowTbl_del
 * Description:
 *      Delete one flow table entry from the specified device.
 * Input:
 *      unit      - unit id
 *      filter_id - filter id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_FILTER_ENTRYIDX        - invalid entry index
 *      RT_ERR_FILTER_FLOWTBL_RULENUM - invalid flow table rulenum
 *      RT_ERR_FILTER_FLOWTBL_TYPE    - entry type isn't a flow table rule
 *      RT_ERR_FILTER_FLOWTBL_EMPTY   - flow table entry is empty
 * Note:
 *      Valid range of filter id of flow table is 0 .. RTK_MAX_ENTRY_OF_BLOCK*cutline -1.
 */
extern int32
dal_ssw_filter_flowTbl_del(uint32 unit, rtk_filter_id_t filter_id);


/* Function Name:
 *      dal_ssw_filter_flowTbl_delAll
 * Description:
 *      Delete all flow table entries from the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 * Note:
 *      None
 */
extern int32
dal_ssw_filter_flowTbl_delAll(uint32 unit);


/* Function Name:
 *      dal_ssw_filter_flowTbl_get
 * Description:
 *      Get one flow table entry from the specified device.
 * Input:
 *      unit         - unit id
 *      filter_id    - filter id
 * Output:
 *      pFilter_cfg - pointer buffer of flow table data
 *      pAction     - pointer buffer of flow table action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_FILTER_ENTRYIDX        - invalid entry index
 *      RT_ERR_FILTER_FLOWTBL_RULENUM - invalid flow table rulenum
 *      RT_ERR_NULL_POINTER           - input parameter may be null pointer
 * Note:
 *      Valid range of filter id of flow table is 0 .. RTK_MAX_ENTRY_OF_BLOCK*cutline -1.
 */
extern int32
dal_ssw_filter_flowTbl_get(
    uint32                  unit,
    rtk_filter_id_t         filter_id,
    rtk_filter_flowTbl_t    *pFilter_cfg,
    rtk_filter_action_t     *pAction);

/* Function Name:
 *      dal_ssw_filter_flowTbl_set
 * Description:
 *      Set one flow table entry to the specified device.
 * Input:
 *      unit         - unit id
 *      filter_id    - filter id
 * Output:
 *      pFilter_cfg - pointer buffer of flow table data
 *      pAction     - pointer buffer of flow table action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_FILTER_ENTRYIDX        - invalid entry index
 *      RT_ERR_FILTER_FLOWTBL_RULENUM - invalid flow table rulenum
 *      RT_ERR_NULL_POINTER           - input parameter may be null pointer
 * Note:
 *      Valid range of filter id of flow table is 0 .. RTK_MAX_ENTRY_OF_BLOCK*cutline -1.
 */
extern int32
dal_ssw_filter_flowTbl_set(
    uint32                  unit,
    rtk_filter_id_t         filter_id,
    rtk_filter_flowTbl_t    *pFilter_cfg,
    rtk_filter_action_t     *pAction);


/* Function Name:
 *      dal_ssw_filter_flowTbl_add
 * Description:
 *      Add one flow table entry to the specified device.
 * Input:
 *      unit         - unit id
 *      filter_id    - filter id
 *      pFilter_cfg - pointer buffer of flow table data
 *      pAction     - pointer buffer of flow table action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_FILTER_ENTRYIDX        - invalid entry index
 *      RT_ERR_FILTER_FLOWTBL_RULENUM - invalid flow table rulenum
 *      RT_ERR_FILTER_FLOWTBL_TYPE    - entry type isn't an flow table rule
 *      RT_ERR_FILTER_ACTION          - action doesn't consist to entry type
 * Note:
 *      Valid range of filter id of flow table is 0 .. RTK_MAX_ENTRY_OF_BLOCK*cutline -1.
 *
 *      Caller have to set the valid bit by himself, if who want to write 
 *      a valid entry into PIE.  
 */
extern int32
dal_ssw_filter_flowTbl_add(
    uint32                  unit,
    rtk_filter_id_t         filter_id,
    rtk_filter_flowTbl_t    *pFilter_cfg,
    rtk_filter_action_t     *pAction);

/* Function Name:
 *      dal_ssw_filter_flowTbl_validate
 * Description:
 *      Validate entry without modifying other field of flow entry.
 * Input:
 *      unit        - unit id
 *      filter_id   - filter id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_FILTER_ENTRYIDX        - invalid entry index
 *      RT_ERR_FILTER_FLOWTBL_RULENUM - invalid flow table rulenum
 *      RT_ERR_FILTER_FLOWTBL_TYPE    - entry type isn't an flow table rule
 * Note:
 */
int32
dal_ssw_filter_flowTbl_validate(
    uint32                  unit,
    rtk_filter_id_t         filter_id);

/* Function Name:
 *      dal_ssw_filter_flowTbl_invalidate
 * Description:
 *      Invalidate entry without modifying other field of flow entry.
 * Input:
 *      unit        - unit id
 *      filter_id   - filter id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_FILTER_ENTRYIDX        - invalid entry index
 *      RT_ERR_FILTER_FLOWTBL_RULENUM - invalid flow table rulenum
 *      RT_ERR_FILTER_FLOWTBL_TYPE    - entry type isn't an flow table rule
 * Note:
 */
int32
dal_ssw_filter_flowTbl_invalidate(
    uint32                  unit,
    rtk_filter_id_t         filter_id);

/* Module Name     : filter      */
/* Sub-module Name : ingress acl */

/* Function Name:
 *      dal_ssw_filter_igrAcl_del
 * Description:
 *      Delete one ingress acl entry from the specified device.
 * Input:
 *      unit      - unit id
 *      filter_id - filter id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_FILTER_ENTRYIDX      - invalid entry index
 *      RT_ERR_FILTER_INACL_RULENUM - invalid ACL rulenum
 *      RT_ERR_FILTER_INACL_TYPE    - entry type isn't an ingress ACL rule
 *      RT_ERR_FILTER_INACL_EMPTY   - entry is empty
 * Note:
 *      Valid range of filter id of ingress acl table is 
 *      RTK_MAX_ENTRY_OF_BLOCK*cutline ~ RTK_MAX_NUM_OF_FILTER_ENTRY-1.
 */
extern int32
dal_ssw_filter_igrAcl_del(uint32 unit, rtk_filter_id_t filter_id);


/* Function Name:
 *      dal_ssw_filter_igrAcl_delAll
 * Description:
 *      Delete all ingress acl entries from the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 * Note:
 *      None
 */
extern int32
dal_ssw_filter_igrAcl_delAll(uint32 unit);


/* Function Name:
 *      dal_ssw_filter_igrAcl_get
 * Description:
 *      Get one ingress acl entry from the specified device.
 * Input:
 *      unit         - unit id
 *      filter_id    - filter id
 * Output:
 *      pFilter_cfg - pointer buffer of ingress acl data
 *      pAction     - pointer buffer of ingress acl action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_FILTER_ENTRYIDX      - invalid entry index
 *      RT_ERR_FILTER_INACL_RULENUM - invalid ACL rulenum
 *      RT_ERR_FILTER_INACL_TYPE    - entry type isn't an ingress ACL rule
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      Valid range of filter id of ingress acl table is 
 *      RTK_MAX_ENTRY_OF_BLOCK*cutline ~ RTK_MAX_NUM_OF_FILTER_ENTRY-1.
 */
extern int32
dal_ssw_filter_igrAcl_get(
    uint32              unit,
    rtk_filter_id_t     filter_id,
    rtk_filter_aclCfg_t *pFilter_cfg,
    rtk_filter_action_t *pAction);

/* Function Name:
 *      dal_ssw_filter_igrAcl_set
 * Description:
 *      Set one ingress acl entry to the specified device.
 * Input:
 *      unit         - unit id
 *      filter_id    - filter id
 * Output:
 *      pFilter_cfg - pointer buffer of ingress acl data
 *      pAction     - pointer buffer of ingress acl action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_FILTER_ENTRYIDX      - invalid entry index
 *      RT_ERR_FILTER_INACL_RULENUM - invalid ACL rulenum
 *      RT_ERR_FILTER_INACL_TYPE    - entry type isn't an ingress ACL rule
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      Valid range of filter id of ingress acl table is 
 *      RTK_MAX_ENTRY_OF_BLOCK*cutline ~ RTK_MAX_NUM_OF_FILTER_ENTRY-1.
 */
extern int32
dal_ssw_filter_igrAcl_set(
    uint32              unit,
    rtk_filter_id_t     filter_id,
    rtk_filter_aclCfg_t *pFilter_cfg,
    rtk_filter_action_t *pAction);

/* Function Name:
 *      dal_ssw_filter_igrAcl_add
 * Description:
 *      Add one ingress acl entry to the specified device.
 * Input:
 *      unit         - unit id
 *      filter_id    - filter id
 *      pFilter_cfg - pointer buffer of ingress acl data
 *      pAction     - pointer buffer of ingress acl action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_FILTER_ENTRYIDX      - invalid entry index
 *      RT_ERR_FILTER_INACL_RULENUM - invalid ACL rulenum
 *      RT_ERR_FILTER_INACL_TYPE    - entry type isn't an ingress ACL rule
 *      RT_ERR_FILTER_INACL_EXIST   - ingress ACL entry is already exist
 *      RT_ERR_FILTER_ACTION        - action doesn't consist to entry type
 * Note:
 *      Valid range of filter id of ingress acl table is 
 *      RTK_MAX_ENTRY_OF_BLOCK*cutline ~ RTK_MAX_NUM_OF_FILTER_ENTRY-1.
 *
 *      Caller have to set the valid bit by himself, if who want to write 
 *      a valid entry into PIE.  
 */
extern int32
dal_ssw_filter_igrAcl_add(
    uint32              unit,
    rtk_filter_id_t     filter_id,
    rtk_filter_aclCfg_t *pFilter_cfg,
    rtk_filter_action_t *pAction);

/* Function Name:
 *      dal_ssw_filter_igrAcl_validate
 * Description:
 *      Validate acl entry without modifying other content of acl
 * Input:
 *      unit        - unit id
 *      filter_id   - filter id
 * 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_FILTER_ENTRYIDX      - invalid entry index
 *      RT_ERR_FILTER_INACL_RULENUM - invalid ACL rulenum
 *      RT_ERR_FILTER_INACL_TYPE    - entry type isn't an ingress ACL rule
 * Note:
 */
extern int32
dal_ssw_filter_igrAcl_validate(
    uint32                  unit,
    rtk_filter_id_t         filter_id);
    
/* Function Name:
 *      dal_ssw_filter_igrAcl_invalidate
 * Description:
 *      Invalidate acl entry without modifying other content of acl
 * Input:
 *      unit        - unit id
 *      filter_id   - filter id
 * 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_FILTER_ENTRYIDX      - invalid entry index
 *      RT_ERR_FILTER_INACL_RULENUM - invalid ACL rulenum
 *      RT_ERR_FILTER_INACL_TYPE    - entry type isn't an ingress ACL rule
 * Note:
 */
extern int32
dal_ssw_filter_igrAcl_invalidate(
    uint32                  unit,
    rtk_filter_id_t         filter_id);

/* Function Name:
 *      dal_ssw_filter_igrAclRateLimit_get
 * Description:
 *      Get ratelimit value of the metering entry from the specified device.
 * Input:
 *      unit     - unit id
 *      meter_id - meter id
 * Output:
 *      pRate   - pointer buffer of ingress acl ratelimit
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_FILTER_METER_ID - invalid metering id
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 * Note:
 *      Valid range of meter id is 0 .. RTK_MAX_NUM_OF_METER_ENTRY-1
 */
extern int32
dal_ssw_filter_igrAclRateLimit_get(uint32 unit, rtk_meter_id_t meter_id, uint32 *pRate);


/* Function Name:
 *      dal_ssw_filter_igrAclRateLimit_set
 * Description:
 *      Set ratelimit value of the metering entry to the specified device.
 * Input:
 *      unit     - unit id
 *      meter_id - meter id
 *      rate     - ingress acl ratelimit
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_FILTER_METER_ID - invalid metering id
 *      RT_ERR_RATE            - invalid rate
 * Note:
 *      Valid range of meter id is 0 .. RTK_MAX_NUM_OF_METER_ENTRY-1
 */
extern int32
dal_ssw_filter_igrAclRateLimit_set(uint32 unit, rtk_meter_id_t meter_id, uint32 rate);


/* Module Name     : filter                          */
/* Sub-module Name : metering and statistic counters */

/* Function Name:
 *      dal_ssw_filter_stat_get
 * Description:
 *      Get statistic counter of the log id from the specified device.
 * Input:
 *      unit       - unit id
 *      log_id     - log id
 * Output:
 *      pPkt_cnt  - pointer buffer of packet count
 *      pByte_cnt - pointer buffer of byte count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_FILTER_LOG_ID - invalid log id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      Valid range of log id is 0 .. RTK_MAX_NUM_OF_LOG_ENTRY-1
 */
extern int32
dal_ssw_filter_stat_get(uint32 unit, rtk_log_id_t log_id, uint32 *pPkt_cnt, uint64 *pByte_cnt);


/* Function Name:
 *      dal_ssw_filter_stat_set
 * Description:
 *      Set statistic counter of the log id to the specified device.
 * Input:
 *      unit     - unit id
 *      log_id   - log id
 *      pkt_cnt  - packet count
 *      byte_cnt - byte count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_FILTER_LOG_ID - invalid log id
 * Note:
 *      Valid range of log id is 0 .. RTK_MAX_NUM_OF_LOG_ENTRY-1
 */
extern int32
dal_ssw_filter_stat_set(uint32 unit, rtk_log_id_t log_id, uint32 pkt_cnt, uint64 byte_cnt);


/* Module Name     : filter                         */
/* Sub-module Name : mac-based vlan (by flow table) */

/* Function Name:
 *      dal_ssw_filter_macBasedVlan_add
 * Description:
 *      Add a source mac which associate to the vlan id and priority to the specified device.
 * Input:
 *      unit      - unit id
 *      filter_id - filter id
 *      sa_mac    - source mac address
 *      vid       - vlan id
 *      pri       - priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_MAC                    - invalid mac address
 *      RT_ERR_FILTER_FLOWTBL_EXIST   - flow table entry is already exist
 *      RT_ERR_FILTER_ENTRYIDX        - invalid entry index
 *      RT_ERR_FILTER_FLOWTBL_RULENUM - invalid flow table rulenum
 *      RT_ERR_VLAN_VID               - invalid vid
 *      RT_ERR_OUT_OF_RANGE           - input parameter out of range
 * Note:
 *      1. The incoming packet which match the source mac address will use the
 *         configure vid and priority for ingress pipeline
 *      2. For destination mac or both, please use the flow table API to configure.
 */
extern int32
dal_ssw_filter_macBasedVlan_add(
    uint32          unit,
    rtk_filter_id_t filter_id,
    rtk_mac_t       sa_mac,
    rtk_vlan_t      vid,
    rtk_pri_t       pri);


/* Function Name:
 *      dal_ssw_filter_macBasedVlan_del
 * Description:
 *      Delete a source mac which associate to vlan id and priority from the specified device.
 * Input:
 *      unit      - unit id
 *      filter_id - filter id
 *      sa_mac    - source mac address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_MAC                    - invalid mac address
 *      RT_ERR_FILTER_FLOWTBL_EMPTY   - flow table entry is empty
 *      RT_ERR_FILTER_ENTRYIDX        - invalid entry index
 *      RT_ERR_FILTER_FLOWTBL_RULENUM - invalid flow table rulenum
 *      RT_ERR_FILTER_FLOWTBL_TYPE    - entry type isn't a flow table rule
 * Note:
 *      None
 */
extern int32
dal_ssw_filter_macBasedVlan_del(uint32 unit, rtk_filter_id_t filter_id, rtk_mac_t sa_mac);


/* Function Name:
 *      dal_ssw_filter_macBasedVlan_delAll
 * Description:
 *      Delete all source mac addresses that associate to vlan id and priority from the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 * Note:
 *      None
 */
extern int32
dal_ssw_filter_macBasedVlan_delAll(uint32 unit);



/* Module Name     : filter                                 */
/* Sub-module Name : ingress vlan translate (by flow table) */

/* Function Name:
 *      dal_ssw_filter_igrVlanXlate_add
 * Description:
 *      Add the ingress vlan translate mapping of the port to the specified device.
 * Input:
 *      unit      - unit id
 *      filter_id - filter id
 *      port      - port id
 *      old_vid   - old vlan id
 *      new_vid   - new vlan id
 *      new_pri   - new priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_PORT_ID                - invalid port id
 *      RT_ERR_FILTER_FLOWTBL_EXIST   - flow table entry is already exist
 *      RT_ERR_FILTER_ENTRYIDX        - invalid entry index
 *      RT_ERR_FILTER_FLOWTBL_RULENUM - invalid flow table rulenum
 *      RT_ERR_VLAN_VID               - invalid vid
 *      RT_ERR_OUT_OF_RANGE           - input parameter out of range
 * Note:
 *      1. The incoming packet in the port which match the old_vid will use the
 *         configure new_vid and new_pri for ingress pipeline
 */
extern int32
dal_ssw_filter_igrVlanXlate_add(
    uint32          unit,
    rtk_filter_id_t filter_id,
    rtk_port_t      port,
    rtk_vlan_t      old_vid,
    rtk_vlan_t      new_vid,
    rtk_pri_t       new_pri);


/* Function Name:
 *      dal_ssw_filter_igrVlanXlate_del
 * Description:
 *      Delete the ingress vlan translate mapping of the port from the specified device.
 * Input:
 *      unit      - unit id
 *      filter_id - filter id
 *      port      - port id
 *      old_vid   - old vlan id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_PORT_ID                - invalid port id
 *      RT_ERR_FILTER_FLOWTBL_EMPTY   - flow table entry is empty
 *      RT_ERR_FILTER_ENTRYIDX        - invalid entry index
 *      RT_ERR_FILTER_FLOWTBL_RULENUM - invalid flow table rulenum
 *      RT_ERR_FILTER_FLOWTBL_TYPE    - entry type isn't a flow table rule
 *      RT_ERR_VLAN_VID               - invalid vid
 * Note:
 *      None
 */
extern int32
dal_ssw_filter_igrVlanXlate_del(
    uint32          unit,
    rtk_filter_id_t filter_id,
    rtk_port_t      port,
    rtk_vlan_t      old_vid);


/* Function Name:
 *      dal_ssw_filter_igrVlanXlate_delAll
 * Description:
 *      Delete all ingress vlan translate from the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 * Note:
 *      None
 */
extern int32
dal_ssw_filter_igrVlanXlate_delAll(uint32 unit);



/* Module Name     : filter                             */
/* Sub-module Name : egress vlan translate by acl table */

/* Function Name:
 *      dal_ssw_filter_egrVlanXlate_add
 * Description:
 *      Add the egress vlan translate mapping of the port to the specified device.
 * Input:
 *      unit      - unit id
 *      filter_id - filter id
 *      port      - port id
 *      old_vid   - old vlan id
 *      new_vid   - new vlan id
 *      new_pri   - new priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT           - The module is not initial
 *      RT_ERR_UNIT_ID            - invalid unit id
 *      RT_ERR_PORT_ID            - invalid port id
 *      RT_ERR_FILTER_INACL_EXIST - ACL entry is already exit
 *      RT_ERR_FILTER_INACL_TYPE  - entry type isn't an ingress ACL rule
 *      RT_ERR_FILTER_ENTRYIDX    - invalid entry index
 *      RT_ERR_VLAN_VID           - invalid vid
 *      RT_ERR_OUT_OF_RANGE       - input parameter out of range
 * Note:
 *      1. The incoming packet in the port which match the old_vid will use the
 *         configure new_vid and new_pri for egress pipeline
 */
extern int32
dal_ssw_filter_egrVlanXlate_add(
    uint32          unit,
    rtk_filter_id_t filter_id,
    rtk_port_t      port,
    rtk_vlan_t      old_vid,
    rtk_vlan_t      new_vid,
    rtk_pri_t       new_pri);


/* Function Name:
 *      dal_ssw_filter_egrVlanXlate_del
 * Description:
 *      Delete the egress vlan translate mapping of the port from the specified device.
 * Input:
 *      unit      - unit id
 *      filter_id - filter id
 *      port      - port id
 *      old_vid   - old vlan id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_FILTER_INACL_EMPTY   - ACL entry is empty
 *      RT_ERR_FILTER_ENTRYIDX      - invalid entry index
 *      RT_ERR_FILTER_INACL_RULENUM - invalid ACL rulenum
 *      RT_ERR_FILTER_INACL_TYPE    - entry type isn't an ingress ACL rule
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_OUT_OF_RANGE         - input parameter out of range
 * Note:
 *      None
 */
extern int32
dal_ssw_filter_egrVlanXlate_del(
    uint32          unit,
    rtk_filter_id_t filter_id,
    rtk_port_t      port,
    rtk_vlan_t      old_vid);


/* Function Name:
 *      dal_ssw_filter_egrVlanXlate_delAll
 * Description:
 *      Delete all egress vlan translate from the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 * Note:
 *      None
 */
extern int32
dal_ssw_filter_egrVlanXlate_delAll(uint32 unit);



/* Module Name     : filter                          */
/* Sub-module Name : vlan double tagged by acl table */

/* Function Name:
 *      dal_ssw_filter_stagVlan_add
 * Description:
 *      Add the service-TAG with service vid and priority when match customer
 *      vid to the specified device.
 * Input:
 *      unit         - unit id
 *      filter_id    - filter id
 *      port         - port id
 *      customer_vid - customer (inner) vlan id
 *      service_vid  - service (outer) vlan id
 *      service_pri  - service (outer) priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_FILTER_INACL_EXIST   - ACL entry is already exit
 *      RT_ERR_FILTER_ENTRYIDX      - invalid entry index
 *      RT_ERR_FILTER_INACL_RULENUM - invalid ACL rulenum
 *      RT_ERR_FILTER_INACL_TYPE    - entry type isn't an ingress ACL rule
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_OUT_OF_RANGE         - input parameter out of range
 * Note:
 *      1. The incoming packet in the port which match the customer_vid will add
 *         one service-TAG with service_vid and service_pri for egress pipeline
 */
extern int32
dal_ssw_filter_stagVlan_add(
    uint32          unit,
    rtk_filter_id_t filter_id,
    rtk_port_t      port,
    rtk_vlan_t      customer_vid,
    rtk_vlan_t      service_vid,
    rtk_pri_t       service_pri);


/* Function Name:
 *      dal_ssw_filter_stagVlan_del
 * Description:
 *      Delete the double tagged configure of the port from the specified device.
 * Input:
 *      unit      - unit id
 *      filter_id - filter id
 *      port      - port id
 *      vid       - vlan id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_FILTER_INACL_EMPTY   - ACL entry is empty
 *      RT_ERR_FILTER_ENTRYIDX      - invalid entry index
 *      RT_ERR_FILTER_INACL_RULENUM - invalid ACL rulenum
 *      RT_ERR_VLAN_VID             - invalid vid
 * Note:
 *      None
 */
extern int32
dal_ssw_filter_stagVlan_del(uint32 unit, rtk_filter_id_t filter_id, rtk_port_t port, rtk_vlan_t vid);


/* Function Name:
 *      dal_ssw_filter_stagVlan_delAll
 * Description:
 *      Delete all the service-TAG configures from the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 * Note:
 *      None
 */
extern int32
dal_ssw_filter_stagVlan_delAll(uint32 unit);



/* Module Name     : filter                             */
/* Sub-module Name : ip-subnet-based vlan by flow table */

/* Function Name:
 *      dal_ssw_filter_ipSubnetBasedVlan_add
 * Description:
 *      Add the source ip subnet-based vlan to the specified device.
 * Input:
 *      unit      - unit id
 *      filter_id - filter id
 *      ipaddr    - ip address
 *      netmask   - netmask
 *      vid       - vlan id
 *      pri       - priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_FILTER_FLOWTBL_EXIST   - flow table entry is already exist
 *      RT_ERR_FILTER_ENTRYIDX        - invalid entry index
 *      RT_ERR_FILTER_FLOWTBL_RULENUM - invalid flow table rulenum
 *      RT_ERR_VLAN_VID               - invalid vid
 *      RT_ERR_OUT_OF_RANGE           - input parameter out of range
 * Note:
 *      The incoming packet which match the source ip subnet will use the
 *      configure vid and priority for ingress pipeline
 */
extern int32
dal_ssw_filter_ipSubnetBasedVlan_add(
    uint32          unit,
    rtk_filter_id_t filter_id,
    ipaddr_t        ipaddr,
    ipaddr_t        netmask,
    rtk_vlan_t      vid,
    rtk_pri_t       pri);


/* Function Name:
 *      dal_ssw_filter_ipSubnetBasedVlan_del
 * Description:
 *      Delete the source ip subnet-based vlan from the specified device.
 * Input:
 *      unit      - unit id
 *      filter_id - filter id
 *      ipaddr    - ip address
 *      netmask   - netmask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_FILTER_FLOWTBL_EMPTY   - flow table entry is empty
 *      RT_ERR_FILTER_ENTRYIDX        - invalid entry index
 *      RT_ERR_FILTER_FLOWTBL_RULENUM - invalid flow table rulenum
 *      RT_ERR_FILTER_FLOWTBL_TYPE    - entry type isn't a flow table rule
 * Note:
 *      None
 */
extern int32
dal_ssw_filter_ipSubnetBasedVlan_del(
    uint32          unit,
    rtk_filter_id_t filter_id,
    ipaddr_t        ipaddr,
    ipaddr_t        netmask);


/* Function Name:
 *      dal_ssw_filter_ipSubnetBasedVlan_delAll
 * Description:
 *      Delete all source ip subnet-based vlans from the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 * Note:
 *      None
 */
extern int32
dal_ssw_filter_ipSubnetBasedVlan_delAll(uint32 unit);



/* Module Name     : filter                                     */
/* Sub-module Name : protocol-and-port-based vlan by flow table */

/* Function Name:
 *      dal_ssw_filter_protoAndPortBasedVlan_add
 * Description:
 *      Add the protocol-and-port-based vlan to the specified port of device.
 * Input:
 *      unit            - unit id
 *      filter_id       - filter id
 *      port            - port id
 *      info.proto_type - protocol type
 *      info.frame_type - frame type
 *      info.cvid       - cvlan id
 *      info.cpri       - cvlan priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_PORT_ID                - invalid port id
 *      RT_ERR_FILTER_FLOWTBL_EXIST   - flow table entry is already exist
 *      RT_ERR_FILTER_ENTRYIDX        - invalid entry index
 *      RT_ERR_FILTER_FLOWTBL_RULENUM - invalid flow table rulenum
 *      RT_ERR_INPUT                  - invalid input parameter
 *      RT_ERR_CHIP_NOT_SUPPORTED     - functions not supported by this chip model
 *      RT_ERR_VLAN_VID               - invalid vid
 *      RT_ERR_OUT_OF_RANGE           - input parameter out of range
 * Note:
 *      1. The incoming packet which match the protocol-and-port-based vlan will use 
 *         the configure vid for ingress pipeline
 *      2. The frame type as following:
 *          - FRAME_TYPE_ETHERNET
 *          - FRAME_TYPE_RFC1042
 *          - FRAME_TYPE_SNAP8021H      (not support now)
 *          - FRAME_TYPE_SNAPOTHER      (not support now)
 *          - FRAME_TYPE_LLCOTHER
 */
extern int32
dal_ssw_filter_protoAndPortBasedVlan_add(
    uint32                      unit,
    rtk_filter_id_t             filter_id,
    rtk_port_t                  port,
    rtk_vlan_protoAndPortInfo_t info);


/* Function Name:
 *      dal_ssw_filter_protoAndPortBasedVlan_del
 * Description:
 *      Delete the protocol-and-port-based vlan from the specified port of device.
 * Input:
 *      unit       - unit id
 *      filter_id  - filter id
 *      port       - port id
 *      proto_type - protocol type
 *      frame_type - frame type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_PORT_ID                - invalid port id
 *      RT_ERR_FILTER_FLOWTBL_EMPTY   - flow table entry is empty
 *      RT_ERR_FILTER_ENTRYIDX        - invalid entry index
 *      RT_ERR_FILTER_FLOWTBL_RULENUM - invalid flow table rulenum
 *      RT_ERR_FILTER_FLOWTBL_TYPE    - entry type isn't a flow table rule
 *      RT_ERR_INPUT                  - invalid input parameter
 *      RT_ERR_CHIP_NOT_SUPPORTED     - functions not supported by this chip model
 *      RT_ERR_VLAN_VID               - invalid vid
 * Note:
 *      1. The incoming packet which match the protocol-and-port-based vlan will use
 *         the configure vid for ingress pipeline
 *      2. The frame type as following:
 *          - FRAME_TYPE_ETHERNET
 *          - FRAME_TYPE_RFC1042
 *          - FRAME_TYPE_SNAP8021H      (not support now)
 *          - FRAME_TYPE_SNAPOTHER      (not support now)
 *          - FRAME_TYPE_LLCOTHER
 */
extern int32
dal_ssw_filter_protoAndPortBasedVlan_del(
    uint32                         unit,
    rtk_filter_id_t                filter_id,
    rtk_port_t                     port,
    uint32                         proto_type,
    rtk_vlan_protoVlan_frameType_t frame_type);


/* Function Name:
 *      dal_ssw_filter_protoAndPortBasedVlan_delAll
 * Description:
 *      Delete all protocol-and-port-based vlans from the specified port of device.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 * Note:
 *      1. The incoming packet which match the protocol-and-port-based vlan will use
 *         the configure vid for ingress pipeline
 *      2. Delete all flow table protocol-and-port-based vlan entries.
 */
extern int32
dal_ssw_filter_protoAndPortBasedVlan_delAll(uint32 unit, rtk_port_t port);

#endif /* __DAL_SSW_FILTER_H__ */

