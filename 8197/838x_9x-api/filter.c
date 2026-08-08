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
 * $Revision: 20135 $
 * $Date: 2011-07-28 17:51:53 +0800 (Thu, 28 Jul 2011) $
 *
 * Purpose : Definition of Filter API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1)  Global Configuration
 *           (2)  Pattern Match
 *           (3)  Flow Table
 *           (4)  Ingress ACL
 *           (5)  Metering and Statistic Counters
 *           (6)  MAC-based VLAN               [by Flow Table]
 *           (7)  Ingress VLAN Translate       [by Flow Table]
 *           (8)  Egress VLAN Translate        [by ACL Table]
 *           (9)  VLAN Double Tagged           [by ACL Table]
 *           (10) IP Subnet-based VLAN         [by Flow Table]
 *           (11) Protocol-and-port-based VLAN [by Flow Table]
 *
 */

/*
 * Include Files
 */
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <dal/dal_mgmt.h>
#include <rtk/filter.h>
#include <rtk/default.h>


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
 *      rtk_filter_blkCutline_get
 * Description:
 *      Get the cutline value from the specified device.
 * Input:
 *      unit     - unit id
 * Output:
 *      pCutline - pointer buffer of cutline value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389
 * Note:
 *      default cutline value is 4, mean block 0-3 is used by flow table
 *      and 4-7 is used by ingress acl table.
 */
int32
rtk_filter_blkCutline_get(uint32 unit, uint32 *pCutline)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_blkCutline_get(unit, pCutline);
} /* end of  rtk_filter_blkCutline_get */


/* Function Name:
 *      rtk_filter_blkCutline_set
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
 * Applicable:
 *      8389
 * Note:
 *      default cutline value is 4, mean block 0-3 is used by flow table
 *      and 4-7 is used by ingress acl table.
 */
int32
rtk_filter_blkCutline_set(uint32 unit, uint32 cutline)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_blkCutline_set(unit, cutline);
} /* end of rtk_filter_blkCutline_set */

/* Function Name:
 *      rtk_filter_pieEnable_get
 * Description:
 *      Get the PIE enable status from the specified device.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer of enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389
 * Note:
 *      None
 */
int32
rtk_filter_pieEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_pieEnable_get(unit, pEnable);
} /* end of rtk_filter_pieEnable_get */

/* Function Name:
 *      rtk_filter_pieEnable_set
 * Description:
 *      Set the PIE enable status to the specified device.
 * Input:
 *      unit   - unit id
 *      enable - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 * Applicable:
 *      8389
 * Note:
 *      None
 */
int32
rtk_filter_pieEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_pieEnable_set(unit, enable);
} /* end of rtk_filter_pieEnable_set */

/* Function Name:
 *      rtk_filter_init
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
 * Applicable:
 *      8389
 * Note:
 *      Must initialize filter module before calling any filter APIs
 */
int32
rtk_filter_init(uint32 unit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_init(unit);
} /* end of rtk_filter_init */


/* Module Name     : filter        */
/* Sub-module Name : pattern match */

/* Function Name:
 *      rtk_filter_patternMatch_get
 * Description:
 *      Get per port pattern match from the specified device.
 * Input:
 *      unit     - unit id
 *      port     - port id
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
 * Applicable:
 *      8389
 * Note:
 *      The pattern match mode as following:
 *      - PM_4BYTE_MODE       (two 4-bytes mode)
 *      - PM_8BYTE_MODE       (one 8-bytes mode)
 */
int32
rtk_filter_patternMatch_get(
    uint32                          unit,
    rtk_port_t                      port,
    rtk_filter_patternMatch_mode_t  *pMode,
    uint8                           *pPattern,
    uint32                          *pMask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_patternMatch_get(unit, port, pMode, pPattern, pMask);
} /* end of rtk_filter_patternMatch_get */


/* Function Name:
 *      rtk_filter_patternMatch_set
 * Description:
 *      Set per port pattern match to the specified device.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      mode     - pattern match mode
 *      pPattern - pointer buffer of pattern to be matched.
 *      mask     - pattrn character care mask.
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
 * Applicable:
 *      8389
 * Note:
 *      (1) The pattern match mode as following:
 *          - PM_4BYTE_MODE       (two 4-bytes mode)
 *          - PM_8BYTE_MODE       (one 8-bytes mode)
 *
 *      (2) The API can set ether in 4-Byte mode or 8-Byte mode.
 *          In 4-Byte mode, pattern of set0 should place in pattern[0~3] and
 *          that of set1 should place in pattern[4~7].
 *          In 4-Byte mode, the first byte of set0 and set1 must care even
 *          if you don't speciy set1.
 *          This is due to ASIC limitation.
 *          If you really want not set1 to be compared, just set the care-bit 
 *          of "PatternMatch" field in ACL to 2 ('10' in binary).
 *          
 *          In 8-Byte mode, the first byte must care.
 *          
 *          Here are some 4-Byte mode examples:
 *            - Example 1. To compare "abcd" and "efgh"
 *            -     input parameter:
 *            -     mode        = 0
 *            -     pattern[]   = "abcdefgh"
 *            -     mask        = 0xff
 *            - Example 2. To compare "abcd" and "efg"
 *            -     input parameter:
 *            -     mode        = 0
 *            -     pattern[]   = "abcdefg "            [Note: Leave 1 blank]
 *            -     mask        = 0xfe
 *            - Example 3. To compare "abc" and "efgh"
 *            -     input parameter:
 *            -     mode        = 0
 *            -     pattern[]   = "abc efgh"
 *            -     mask        = 0xef
 *            - Example 4. To compare "abc" and "efg"
 *            -     input parameter:
 *            -     mode        = 0
 *            -     pattern[]   = "abc efg "
 *            -     mask        = 0xee          
 *            - Example 5. To compare "abcd"
 *            -     input parameter:
 *            -     mode        = 0
 *            -     pattern[]   = "abcd    "            [Note: Leave 4 blank]
 *            -     mask        = 0xf1
 *            - Example 6. To compare "abc"
 *            -     input parameter:
 *            -     mode        = 0
 *            -     pattern[]   = "abc     "            [Note: Leave 5 blank]
 *            -     mask        = 0xe1
 *            - Example 7. To compare "a*cd"
 *            -     input parameter:
 *            -     mode        = 0
 *            -     pattern[]   = "a cd    "            [Note: There are 5 blank]
 *            -     mask        = 0xb1
 *          
 *          In above case 5,6 and 7, although set1 is not specified, we still
 *          care its first byte.          
 *          
 *          Here are some 8-Byte mode examples:
 *            - Example 1. To compare "abcdefgh"
 *            -     input parameter:
 *            -     mode        = 1
 *            -     pattern[]   = "abcdefgh"
 *            -     mask        = 0xff
 *            - Example 2. To compare "abcde"
 *            -     input parameter:
 *            -     mode        = 1
 *            -     pattern[]   = "abcde   "
 *            -     mask        = 0xf8
 *            - Example 3. To compare "abc*e"
 *            -     input parameter:
 *            -     mode        = 1
 *            -     pattern[]   = "abcde   "
 *            -     mask        = 0xe8
 */
int32
rtk_filter_patternMatch_set(
    uint32                          unit,
    rtk_port_t                      port,
    rtk_filter_patternMatch_mode_t  mode,
    uint8                           *pPattern,
    uint32                          mask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_patternMatch_set(unit, port, mode, pPattern, mask);
} /* end of rtk_filter_patternMatch_set */


/* Module Name     : filter     */
/* Sub-module Name : flow table */

/* Function Name:
 *      rtk_filter_flowTbl_del
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
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid filter_id range of flow table is 0 .. HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline -1. 
 */
int32
rtk_filter_flowTbl_del(uint32 unit, rtk_filter_id_t filter_id)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_flowTbl_del(unit, filter_id);
} /* end of rtk_filter_flowTbl_del */


/* Function Name:
 *      rtk_filter_flowTbl_delAll
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
 * Applicable:
 *      8389
 * Note:
 *      None
 */
int32
rtk_filter_flowTbl_delAll(uint32 unit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_flowTbl_delAll(unit);
} /* end of rtk_filter_flowTbl_delAll */


/* Function Name:
 *      rtk_filter_flowTbl_get
 * Description:
 *      Get one flow table entry with action from the specified device.
 * Input:
 *      unit        - unit id
 *      filter_id   - filter id
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
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid filter_id range of flow table is 0 .. HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline -1.  
 */
int32
rtk_filter_flowTbl_get(
    uint32                  unit,
    rtk_filter_id_t         filter_id,
    rtk_filter_flowTbl_t    *pFilter_cfg,
    rtk_filter_action_t     *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_flowTbl_get(unit, filter_id, pFilter_cfg, pAction);
} /* end of rtk_filter_flowTbl_get */

/* Function Name:
 *      rtk_filter_flowTbl_set
 * Description:
 *      Set flow table entry with action to the specified device.
 * Input:
 *      unit        - unit id
 *      filter_id   - filter id
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
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid filter_id range of flow table is 0 .. HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline -1.  
 */
int32
rtk_filter_flowTbl_set(
    uint32                  unit,
    rtk_filter_id_t         filter_id,
    rtk_filter_flowTbl_t    *pFilter_cfg,
    rtk_filter_action_t     *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_flowTbl_set(unit, filter_id, pFilter_cfg, pAction);
} /* end of rtk_filter_flowTbl_set */


/* Function Name:
 *      rtk_filter_flowTbl_add
 * Description:
 *      Add one flow table entry with action to the specified device.
 * Input:
 *      unit        - unit id
 *      filter_id   - filter id
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
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid filter_id range of flow table is 0 .. HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline -1.  
 *      (2) Caller have to set the valid bit by himself, if who want to write a valid entry into PIE.
 */
int32
rtk_filter_flowTbl_add(
    uint32                  unit,
    rtk_filter_id_t         filter_id,
    rtk_filter_flowTbl_t    *pFilter_cfg,
    rtk_filter_action_t     *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_flowTbl_add(unit, filter_id, pFilter_cfg, pAction);
} /* end of rtk_filter_flowTbl_add */

/* Function Name:
 *      rtk_filter_flowTbl_validate
 * Description:
 *      Validate entry without modifying other field of flow entry.
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
 *      RT_ERR_FILTER_FLOWTBL_TYPE    - entry type isn't an flow table rule
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid filter_id range of flow table is 0 .. HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline -1.  
 */
int32
rtk_filter_flowTbl_validate(
    uint32                  unit,
    rtk_filter_id_t         filter_id)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_flowTbl_validate(unit, filter_id);
} /* end of rtk_filter_flowTbl_validate */

/* Function Name:
 *      rtk_filter_flowTbl_invalidate
 * Description:
 *      Invalidate entry without modifying other field of flow entry.
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
 *      RT_ERR_FILTER_FLOWTBL_TYPE    - entry type isn't an flow table rule
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid filter_id range of flow table is 0 .. HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline -1.  
 */
int32
rtk_filter_flowTbl_invalidate(
    uint32                  unit,
    rtk_filter_id_t         filter_id)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_flowTbl_invalidate(unit, filter_id);
} /* end of rtk_filter_flowTbl_invalidate */

/* Module Name     : filter      */
/* Sub-module Name : ingress acl */

/* Function Name:
 *      rtk_filter_igrAcl_del
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
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid filter_id range of ingress acl table is 
 *          HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline ~ HAL_PIE_FILTER_ID_MAX.
 */
int32
rtk_filter_igrAcl_del(uint32 unit, rtk_filter_id_t filter_id)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_igrAcl_del(unit, filter_id);
} /* end of rtk_filter_igrAcl_del */


/* Function Name:
 *      rtk_filter_igrAcl_delAll
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
 * Applicable:
 *      8389
 * Note:
 *      None
 */
int32
rtk_filter_igrAcl_delAll(uint32 unit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_igrAcl_delAll(unit);
} /* end of rtk_filter_igrAcl_delAll */


/* Function Name:
 *      rtk_filter_igrAcl_get
 * Description:
 *      Get one ingress acl entry with action from the specified device.
 * Input:
 *      unit        - unit id
 *      filter_id   - filter id
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
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid filter_id range of ingress acl table is 
 *          HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline ~ HAL_PIE_FILTER_ID_MAX.
 */
int32
rtk_filter_igrAcl_get(
    uint32              unit,
    rtk_filter_id_t     filter_id,
    rtk_filter_aclCfg_t *pFilter_cfg,
    rtk_filter_action_t *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_igrAcl_get(unit, filter_id, pFilter_cfg, pAction);
} /* end of rtk_filter_igrAcl_get */

/* Function Name:
 *      rtk_filter_igrAcl_set
 * Description:
 *      Set one ingress acl entry with action to the specified device.
 * Input:
 *      unit        - unit id
 *      filter_id   - filter id
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
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid filter_id range of ingress acl table is 
 *          HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline ~ HAL_PIE_FILTER_ID_MAX.
 */
int32
rtk_filter_igrAcl_set(
    uint32              unit,
    rtk_filter_id_t     filter_id,
    rtk_filter_aclCfg_t *pFilter_cfg,
    rtk_filter_action_t *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_igrAcl_set(unit, filter_id, pFilter_cfg, pAction);
} /* end of rtk_filter_igrAcl_set */

/* Function Name:
 *      rtk_filter_igrAcl_add
 * Description:
 *      Add one ingress acl entry with action to the specified device.
 * Input:
 *      unit        - unit id
 *      filter_id   - filter id
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
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid filter_id range of ingress acl table is 
 *          HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline ~ HAL_PIE_FILTER_ID_MAX.
 *      (2) Caller have to set the valid bit by himself, if who want to write a valid entry into PIE.
 */
int32
rtk_filter_igrAcl_add(
    uint32              unit,
    rtk_filter_id_t     filter_id,
    rtk_filter_aclCfg_t *pFilter_cfg,
    rtk_filter_action_t *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_igrAcl_add(unit, filter_id, pFilter_cfg, pAction);
} /* end of rtk_filter_igrAcl_add */

/* Function Name:
 *      rtk_filter_igrAcl_validate
 * Description:
 *      Validate acl entry without modifying other content of acl
 * Input:
 *      unit      - unit id
 *      filter_id - filter id
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
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid filter_id range of ingress acl table is 
 *          HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline ~ HAL_PIE_FILTER_ID_MAX.
 */
int32
rtk_filter_igrAcl_validate(
    uint32                  unit,
    rtk_filter_id_t         filter_id)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_igrAcl_validate(unit, filter_id);
} /* end of rtk_filter_igrAcl_validate */
    
/* Function Name:
 *      rtk_filter_igrAcl_invalidate
 * Description:
 *      Invalidate acl entry without modifying other content of acl
 * Input:
 *      unit      - unit id
 *      filter_id - filter id
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
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid filter_id range of ingress acl table is 
 *          HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline ~ HAL_PIE_FILTER_ID_MAX.
 */
int32
rtk_filter_igrAcl_invalidate(
    uint32                  unit,
    rtk_filter_id_t         filter_id)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_igrAcl_invalidate(unit, filter_id);
} /* end of rtk_filter_igrAcl_invalidate */

/* Function Name:
 *      rtk_filter_igrAclRateLimit_get
 * Description:
 *      Get ratelimit value of the metering entry from the specified device.
 * Input:
 *      unit     - unit id
 *      meter_id - meter id
 * Output:
 *      pRate    - pointer buffer of ingress acl ratelimit
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_FILTER_METER_ID - invalid metering id
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid range of meter_id is 0 ~ 127
 *      (2) The unit of granularity is 16Kbps and valid range of rate is 0 ~ 0xFFFF
 */
int32
rtk_filter_igrAclRateLimit_get(
    uint32         unit,
    rtk_meter_id_t meter_id,
    uint32         *pRate)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_igrAclRateLimit_get(unit, meter_id, pRate);
} /* end of rtk_filter_igrAclRateLimit_get */


/* Function Name:
 *      rtk_filter_igrAclRateLimit_set
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
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid range of meter_id is 0 ~ 127
 *      (2) The unit of granularity is 16Kbps and valid range of rate is 0 ~ 0xFFFF
 */
int32
rtk_filter_igrAclRateLimit_set(uint32 unit, rtk_meter_id_t meter_id, uint32 rate)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_igrAclRateLimit_set(unit, meter_id, rate);
} /* end of rtk_filter_igrAclRateLimit_set */


/* Module Name     : filter                          */
/* Sub-module Name : metering and statistic counters */

/* Function Name:
 *      rtk_filter_stat_get
 * Description:
 *      Get statistic counter of the log id from the specified device.
 * Input:
 *      unit      - unit id
 *      log_id    - log id
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
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid range of log_id is 0 ~ 127
 */
int32
rtk_filter_stat_get(uint32 unit, rtk_log_id_t log_id, uint32 *pPkt_cnt, uint64 *pByte_cnt)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_stat_get(unit, log_id, pPkt_cnt, pByte_cnt);
} /* end of rtk_filter_stat_get */


/* Function Name:
 *      rtk_filter_stat_set
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
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid range of log_id is 0 ~ 127
 */
int32
rtk_filter_stat_set(uint32 unit, rtk_log_id_t log_id, uint32 pkt_cnt, uint64 byte_cnt)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_stat_set(unit, log_id, pkt_cnt, byte_cnt);
} /* end of rtk_filter_stat_set */


/* Module Name     : filter                         */
/* Sub-module Name : mac-based vlan (by flow table) */

/* Function Name:
 *      rtk_filter_macBasedVlan_add
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
 * Applicable:
 *      8389
 * Note:
 *      (1) The incoming packet which match the source mac address will use the
 *          configure vid and priority for ingress pipeline
 *      (2) For destination mac or both, please use the flow table API to configure.
 *      (3) The valid filter_id range of flow table is 0 .. HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline -1.  
 */
int32
rtk_filter_macBasedVlan_add(
    uint32          unit,
    rtk_filter_id_t filter_id,
    rtk_mac_t       sa_mac,
    rtk_vlan_t      vid,
    rtk_pri_t       pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_macBasedVlan_add(unit, filter_id, sa_mac, vid, pri);
} /* end of rtk_filter_macBasedVlan_add */


/* Function Name:
 *      rtk_filter_macBasedVlan_del
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
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid filter_id range of flow table is 0 .. HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline -1.  
 */
int32
rtk_filter_macBasedVlan_del(uint32 unit, rtk_filter_id_t filter_id, rtk_mac_t sa_mac)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_macBasedVlan_del(unit, filter_id, sa_mac);
} /* end of rtk_filter_macBasedVlan_del */


/* Function Name:
 *      rtk_filter_macBasedVlan_delAll
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
 * Applicable:
 *      8389
 * Note:
 *      None
 */
int32
rtk_filter_macBasedVlan_delAll(uint32 unit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_macBasedVlan_delAll(unit);
} /* end of rtk_filter_macBasedVlan_delAll */


/* Module Name     : filter                                 */
/* Sub-module Name : ingress vlan translate (by flow table) */

/* Function Name:
 *      rtk_filter_igrVlanXlate_add
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
 * Applicable:
 *      8389
 * Note:
 *      (1) The incoming packet in the port which match the old_vid will use the
 *          configure new_vid and new_pri for ingress pipeline
 *      (2) The valid filter_id range of flow table is 0 .. HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline -1.
 */
int32
rtk_filter_igrVlanXlate_add(
    uint32          unit,
    rtk_filter_id_t filter_id,
    rtk_port_t      port,
    rtk_vlan_t      old_vid,
    rtk_vlan_t      new_vid,
    rtk_pri_t       new_pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_igrVlanXlate_add(unit, filter_id, port, old_vid, new_vid, new_pri);
} /* end of rtk_filter_igrVlanXlate_add */


/* Function Name:
 *      rtk_filter_igrVlanXlate_del
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
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid filter_id range of flow table is 0 .. HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline -1.
 */
int32
rtk_filter_igrVlanXlate_del(
    uint32          unit,
    rtk_filter_id_t filter_id,
    rtk_port_t      port,
    rtk_vlan_t      old_vid)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_igrVlanXlate_del(unit, filter_id, port, old_vid);
} /* end of rtk_filter_igrVlanXlate_del */


/* Function Name:
 *      rtk_filter_igrVlanXlate_delAll
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
 * Applicable:
 *      8389
 * Note:
 *      None
 */
int32
rtk_filter_igrVlanXlate_delAll(uint32 unit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_igrVlanXlate_delAll(unit);
} /* end of rtk_filter_igrVlanXlate_delAll */


/* Module Name     : filter                             */
/* Sub-module Name : egress vlan translate by acl table */

/* Function Name:
 *      rtk_filter_egrVlanXlate_add
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
 * Applicable:
 *      8389
 * Note:
 *      (1) The incoming packet in the port which match the old_vid will use the
 *          configure new_vid and new_pri for egress pipeline
 *      (2) The valid filter_id range of ingress acl table is 
 *          HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline ~ HAL_PIE_FILTER_ID_MAX.
 */
int32
rtk_filter_egrVlanXlate_add(
    uint32          unit,
    rtk_filter_id_t filter_id,
    rtk_port_t      port,
    rtk_vlan_t      old_vid,
    rtk_vlan_t      new_vid,
    rtk_pri_t       new_pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_egrVlanXlate_add(unit, filter_id, port, old_vid, new_vid, new_pri);
} /* end of rtk_filter_egrVlanXlate_add */


/* Function Name:
 *      rtk_filter_egrVlanXlate_del
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
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid filter_id range of ingress acl table is 
 *          HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline ~ HAL_PIE_FILTER_ID_MAX.
 */
int32
rtk_filter_egrVlanXlate_del(
    uint32          unit,
    rtk_filter_id_t filter_id,
    rtk_port_t      port,
    rtk_vlan_t      old_vid)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_egrVlanXlate_del(unit, filter_id, port, old_vid);
} /* end of rtk_filter_egrVlanXlate_del */


/* Function Name:
 *      rtk_filter_egrVlanXlate_delAll
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
 * Applicable:
 *      8389
 * Note:
 *      None
 */
int32
rtk_filter_egrVlanXlate_delAll(uint32 unit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_egrVlanXlate_delAll(unit);
} /* end of rtk_filter_egrVlanXlate_delAll */


/* Module Name     : filter                          */
/* Sub-module Name : vlan double tagged by acl table */

/* Function Name:
 *      rtk_filter_stagVlan_add
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
 * Applicable:
 *      8389
 * Note:
 *      (1) The incoming packet in the port which match the customer_vid will add
 *          one service-TAG with service_vid and service_pri for egress pipeline
 *      (2) The valid filter_id range of ingress acl table is 
 *          HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline ~ HAL_PIE_FILTER_ID_MAX.
 */
int32
rtk_filter_stagVlan_add(
    uint32          unit,
    rtk_filter_id_t filter_id,
    rtk_port_t      port,
    rtk_vlan_t      customer_vid,
    rtk_vlan_t      service_vid,
    rtk_pri_t       service_pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_stagVlan_add(unit, filter_id, port, customer_vid, service_vid, service_pri);
} /* end of rtk_filter_stagVlan_add */


/* Function Name:
 *      rtk_filter_stagVlan_del
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
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid filter_id range of ingress acl table is 
 *          HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline ~ HAL_PIE_FILTER_ID_MAX.
 */
int32
rtk_filter_stagVlan_del(uint32 unit, rtk_filter_id_t filter_id, rtk_port_t port, rtk_vlan_t vid)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_stagVlan_del(unit, filter_id, port, vid);
} /* end of rtk_filter_stagVlan_del */


/* Function Name:
 *      rtk_filter_stagVlan_delAll
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
 * Applicable:
 *      8389
 * Note:
 *      None
 */
int32
rtk_filter_stagVlan_delAll(uint32 unit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_stagVlan_delAll(unit);
} /* end of rtk_filter_stagVlan_delAll */


/* Module Name     : filter                             */
/* Sub-module Name : ip-subnet-based vlan by flow table */

/* Function Name:
 *      rtk_filter_ipSubnetBasedVlan_add
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
 * Applicable:
 *      8389
 * Note:
 *      (1) The incoming packet which match the source ip subnet will use the
 *          configure vid and priority for ingress pipeline
 *      (2) The valid filter_id range of flow table is 0 .. HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline -1.
 */
int32
rtk_filter_ipSubnetBasedVlan_add(
    uint32          unit,
    rtk_filter_id_t filter_id,
    ipaddr_t        ipaddr,
    ipaddr_t        netmask,
    rtk_vlan_t      vid,
    rtk_pri_t       pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_ipSubnetBasedVlan_add(unit, filter_id, ipaddr, netmask, vid, pri);
} /* end of rtk_filter_ipSubnetBasedVlan_add */


/* Function Name:
 *      rtk_filter_ipSubnetBasedVlan_del
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
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid filter_id range of flow table is 0 .. HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline -1.
 */
int32
rtk_filter_ipSubnetBasedVlan_del(
    uint32          unit,
    rtk_filter_id_t filter_id,
    ipaddr_t        ipaddr,
    ipaddr_t        netmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_ipSubnetBasedVlan_del(unit, filter_id, ipaddr, netmask);
} /* end of rtk_filter_ipSubnetBasedVlan_del */


/* Function Name:
 *      rtk_filter_ipSubnetBasedVlan_delAll
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
 * Applicable:
 *      8389
 * Note:
 *      None
 */
int32
rtk_filter_ipSubnetBasedVlan_delAll(uint32 unit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_ipSubnetBasedVlan_delAll(unit);
} /* end of rtk_filter_ipSubnetBasedVlan_delAll */


/* Module Name     : filter                                     */
/* Sub-module Name : protocol-and-port-based vlan by flow table */

/* Function Name:
 *      rtk_filter_protoAndPortBasedVlan_add
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
 * Applicable:
 *      8389
 * Note:
 *      (1) The incoming packet which match the protocol-and-port-based vlan will use 
 *          the configure vid for ingress pipeline
 *      (2) The valid filter_id range of flow table is 0 .. HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline -1.
 *      (3) The frame type as following:
 *          - FRAME_TYPE_ETHERNET
 *          - FRAME_TYPE_RFC1042
 *          - FRAME_TYPE_SNAP8021H      (not support now)
 *          - FRAME_TYPE_SNAPOTHER      (not support now)
 *          - FRAME_TYPE_LLCOTHER
 */
int32
rtk_filter_protoAndPortBasedVlan_add(
    uint32                      unit,
    rtk_filter_id_t             filter_id,
    rtk_port_t                  port,
    rtk_vlan_protoAndPortInfo_t info)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_protoAndPortBasedVlan_add(unit, filter_id, port, info);
} /* end of rtk_filter_protoAndPortBasedVlan_add */


/* Function Name:
 *      rtk_filter_protoAndPortBasedVlan_del
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
 * Applicable:
 *      8389
 * Note:
 *      (1) The incoming packet which match the protocol-and-port-based vlan will use
 *          the configure vid for ingress pipeline
 *      (2) The valid filter_id range of flow table is 0 .. HAL_MAX_NUM_OF_PIE_BLOCKSIZE*cutline -1.
 *      (3) The frame type as following:
 *          - FRAME_TYPE_ETHERNET
 *          - FRAME_TYPE_RFC1042
 *          - FRAME_TYPE_SNAP8021H      (not support now)
 *          - FRAME_TYPE_SNAPOTHER      (not support now)
 *          - FRAME_TYPE_LLCOTHER
 */
int32
rtk_filter_protoAndPortBasedVlan_del(
    uint32                         unit,
    rtk_filter_id_t                filter_id,
    rtk_port_t                     port,
    uint32                         proto_type,
    rtk_vlan_protoVlan_frameType_t frame_type)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_protoAndPortBasedVlan_del(unit, filter_id, port, proto_type, frame_type);
} /* end of rtk_filter_protoAndPortBasedVlan_del */


/* Function Name:
 *      rtk_filter_protoAndPortBasedVlan_delAll
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
 * Applicable:
 *      8389
 * Note:
 *      (1) The incoming packet which match the protocol-and-port-based vlan will use
 *          the configure vid for ingress pipeline
 *      (2) Delete all flow table protocol-and-port-based vlan entries.
 */
int32
rtk_filter_protoAndPortBasedVlan_delAll(
    uint32 unit,
    rtk_port_t port)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->filter_protoAndPortBasedVlan_delAll(unit, port);
} /* end of rtk_filter_protoAndPortBasedVlan_delAll */
