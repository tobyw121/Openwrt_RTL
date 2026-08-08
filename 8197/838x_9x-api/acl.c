/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008-2009
 * All rights reserved.
 *
 * $Revision: 41929 $
 * $Date: 2013-08-09 10:57:40 +0800 (Fri, 09 Aug 2013) $
 *
 * Purpose : Definition those public ACL APIs and its data type in the SDK .
 *
 * Feature : The file have include the following module and sub-modules
 *            1) Ingress ACL
 *            2) Egress ACL
 *            3) Field Selector
 *            4) Range Check
 *            5) Meter
 *            6) Counter
 *
 */

/*
 * Include Files
 */
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <dal/dal_mgmt.h>
#include <rtk/default.h>
#include <rtk/acl.h>

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

/* Module Name     : ACL */

/* Function Name:
 *      rtk_acl_init
 * Description:
 *      Initialize ACL module of the specified device.
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
 *      8390, 8380
 * Note:
 *      Must initialize ACL module before calling any ACL APIs.
 */
int32
rtk_acl_init(uint32 unit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_init(unit);
} /* end of rtk_acl_init */

/* Function Name:
 *      rtk_acl_partition_get
 * Description:
 *      Get the acl partition configuration.
 * Input:
 *      unit       - unit id
 * Output:
 *      pPartition - pointer buffer of partition value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      (1) Each block contains 128 ACL rules.
 *      (2) Assumed total block is Y. If partition value is set to X, means block 0~(X-1)
 *          are for ingress ACL while block X~(Y-1) are for egress acl.
 *          Thus, ingress ACL rule is assigned to logical index 0~(X*128-1), egress ACL rule
 *          is assigned to logical index 0~((Y-X)*128-1).
 */
int32
rtk_acl_partition_get(uint32 unit, uint32 *pPartition)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_partition_get(unit, pPartition);
} /* end of rtk_acl_partition_get */

/* Function Name:
 *      rtk_acl_partition_set
 * Description:
 *      Set the acl partition configuration.
 * Input:
 *      unit      - unit id
 *      partition - partition value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - partition value is out of range
 * Applicable:
 *      8390
 * Note:
 *      (1) Each block contains 128 ACL rules.
 *      (2) Assumed total block is Y. If partition value is set to X, means block 0~(X-1)
 *          are for ingress ACL while block X~(Y-1) are for egress acl.
 *          Thus, ingress ACL rule is assigned to logical index 0~(X*128-1), egress ACL rule
 *          is assigned to logical index 0~((Y-X)*128-1).
 */
int32
rtk_acl_partition_set(uint32 unit, uint32 partition)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_partition_set(unit, partition);
} /* end of rtk_acl_partition_set */

/* Function Name:
 *      rtk_acl_blockPwrEnable_get
 * Description:
 *      Get the acl block power state.
 * Input:
 *      unit      - unit id
 *      block_idx - block index
 * Output:
 *      pEnable   - enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_ACL_BLOCK_INDEX - block index is out of range
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      The rule data are cleared if the block power is disabled.
 */
int32
rtk_acl_blockPwrEnable_get(uint32 unit, uint32 block_idx, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_blockPwrEnable_get(unit, block_idx, pEnable);
} /* end of rtk_acl_blockPwrEnable_get */

/* Function Name:
 *      rtk_acl_blockPwrEnable_set
 * Description:
 *      Set the acl block power state.
 * Input:
 *      unit      - unit id
 *      block_idx - block index
 *      enable    - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_ACL_BLOCK_INDEX - block index is out of range
 *      RT_ERR_INPUT           - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      The rule data are cleared if the block power is disabled.
 */
int32
rtk_acl_blockPwrEnable_set(uint32 unit, uint32 block_idx, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_blockPwrEnable_set(unit, block_idx, enable);
} /* end of rtk_acl_blockPwrEnable_set */

/* Function Name:
 *      rtk_acl_blockLookupEnable_get
 * Description:
 *      Get the acl block lookup state.
 * Input:
 *      unit      - unit id
 *      block_idx - block index
 * Output:
 *      pEnable   - enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - block index is out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) The rule data are kept regardless of the lookup status.
 *      (2) The lookup result is always false if the lookup state is disabled.
 */
int32
rtk_acl_blockLookupEnable_get(uint32 unit, uint32 block_idx, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_blockLookupEnable_get(unit, block_idx, pEnable);
} /* end of rtk_acl_blockLookupEnable_get */

/* Function Name:
 *      rtk_acl_blockLookupEnable_set
 * Description:
 *      Set the acl block lookup state.
 * Input:
 *      unit      - unit id
 *      block_idx - block index
 *      enable    - enable status
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
 *      8390, 8380
 * Note:
 *      (1) The rule data are kept regardless of the lookup status.
 *      (2) The lookup result is always false if the lookup state is disabled.
 */
int32
rtk_acl_blockLookupEnable_set(uint32 unit, uint32 block_idx, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_blockLookupEnable_set(unit, block_idx, enable);
} /* end of rtk_acl_blockLookupEnable_set */

/* Function Name:
 *      rtk_acl_portLookupEnable_get
 * Description:
 *      Get the acl per port lookup state.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - enable status
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
int32
rtk_acl_portLookupEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->acl_portLookupEnable_get(unit, phy_port, pEnable);
    }
#else
    return RT_MAPPER(unit)->acl_portLookupEnable_get(unit, port, pEnable);
#endif
} /* end of rtk_acl_portLookupEnable_get */

/* Function Name:
 *      rtk_acl_portLookupEnable_set
 * Description:
 *      Set the acl per port lookup state.
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
 *      8380
 * Note:
 *      None
 */
int32
rtk_acl_portLookupEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->acl_portLookupEnable_set(unit, phy_port, enable);
    }
#else
    return RT_MAPPER(unit)->acl_portLookupEnable_set(unit, port, enable);
#endif
} /* end of rtk_acl_portLookupEnable_set */

/* Function Name:
 *      rtk_acl_lookupMissAct_get
 * Description:
 *      Get the acl per port default action when acl look up miss.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pLmAct - default action of acl look up miss
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
int32
rtk_acl_lookupMissAct_get(uint32 unit, rtk_port_t port, rtk_acl_lookupMissAct_t *pLmAct)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->acl_lookupMissAct_get(unit, phy_port, pLmAct);
    }
#else
    return RT_MAPPER(unit)->acl_lookupMissAct_get(unit, port, pLmAct);
#endif
} /* end of rtk_acl_lookupMissAct_get */

/* Function Name:
 *      rtk_acl_lookupMissAct_set
 * Description:
 *     Set the acl per port default action when acl look up miss.
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      lmAct - default action of acl look up miss
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
int32
rtk_acl_lookupMissAct_set(uint32 unit, rtk_port_t port, rtk_acl_lookupMissAct_t lmAct)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->acl_lookupMissAct_set(unit, phy_port, lmAct);
    }
#else
    return RT_MAPPER(unit)->acl_lookupMissAct_set(unit, port, lmAct);
#endif
} /* end of rtk_acl_lookupMissAct_set */

/* Function Name:
 *      rtk_acl_ruleEntryFieldSize_get
 * Description:
 *      Get the field size of ACL entry.
 * Input:
 *      unit        - unit id
 *      type        - type of entry field
 * Output:
 *      pField_size - field size of ACL entry.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_ACL_FIELD_TYPE - invalid entry field type
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      The unit of size is bit.
 */
int32
rtk_acl_ruleEntryFieldSize_get(uint32 unit, rtk_acl_fieldType_t type, uint32 *pField_size)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_ruleEntryFieldSize_get(unit, type, pField_size);
} /* end of rtk_acl_ruleEntryFieldSize_get */

/* Function Name:
 *      rtk_acl_ruleEntrySize_get
 * Description:
 *      Get the rule entry size of ACL.
 * Input:
 *      unit        - unit id
 *      phase       - ACL lookup phase
 * Output:
 *      pEntry_size - rule entry size of ingress ACL
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ACL_PHASE    - invalid ACL phase
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      The unit of size is byte.
 */
int32
rtk_acl_ruleEntrySize_get(uint32 unit, rtk_acl_phase_t phase, uint32 *pEntry_size)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_ruleEntrySize_get(unit, phase, pEntry_size);
} /* end of rtk_acl_ruleEntrySize_get */

/* Function Name:
 *      rtk_acl_ruleValidate_get
 * Description:
 *      Validate ACL rule without modifying the content
 * Input:
 *      unit      - unit id
 *      phase     - ACL lookup phase
 *      entry_idx - entry index
 * Output:
 *      pValid    - pointer buffer of valid state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ACL_PHASE    - invalid ACL phase
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_acl_ruleValidate_get(
    uint32              unit,
    rtk_acl_phase_t     phase,
    rtk_acl_id_t        entry_idx,
    uint32              *pValid)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_ruleValidate_get(unit, phase, entry_idx, pValid);
} /* end of rtk_acl_ruleValidate_get */

/* Function Name:
 *      rtk_acl_ruleValidate_set
 * Description:
 *      Validate ACL rule without modifying the content
 * Input:
 *      unit      - unit id
 *      phase     - ACL lookup phase
 *      entry_idx - entry index
 *      valid     - valid state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_ACL_PHASE   - invalid ACL phase
 *      RT_ERR_ENTRY_INDEX - invalid entry index
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_acl_ruleValidate_set(
    uint32              unit,
    rtk_acl_phase_t     phase,
    rtk_acl_id_t        entry_idx,
    uint32              valid)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_ruleValidate_set(unit, phase, entry_idx, valid);
} /* end of rtk_acl_ruleValidate_set */

/* Function Name:
 *      rtk_acl_ruleEntry_read
 * Description:
 *      Read the entry data from specified ACL entry.
 * Input:
 *      unit          - unit id
 *      phase         - ACL lookup phase
 *      entry_idx     - entry index
 * Output:
 *      pEntry_buffer - data buffer of ACL entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ACL_PHASE    - invalid ACL phase
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      The API reads the rule data from chip to entry buffer. Use rtk_acl_ruleEntryField_get to
 *      get the field data from the entry buffer.
 */
int32
rtk_acl_ruleEntry_read(
    uint32              unit,
    rtk_acl_phase_t     phase,
    rtk_acl_id_t        entry_idx,
    uint8               *pEntry_buffer)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_ruleEntry_read(unit, phase, entry_idx, pEntry_buffer);
} /* end of rtk_acl_ruleEntry_read */

/* Function Name:
 *      rtk_acl_ruleEntry_write
 * Description:
 *      Write the entry data to specified ACL entry.
 * Input:
 *      unit          - unit id
 *      phase         - ACL lookup phase
 *      entry_idx     - entry index
 *      pEntry_buffer - data buffer of ACL entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ACL_PHASE    - invalid ACL phase
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      Use rtk_acl_ruleEntryField_set to fill the entry buffer and then use this API to write the
 *      entry buffer to the chip.
 */
int32
rtk_acl_ruleEntry_write(
    uint32              unit,
    rtk_acl_phase_t     phase,
    rtk_acl_id_t        entry_idx,
    uint8               *pEntry_buffer)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_ruleEntry_write(unit, phase, entry_idx, pEntry_buffer);
} /* end of rtk_acl_ruleEntry_write */

/* Function Name:
 *      rtk_acl_ruleEntryField_get
 * Description:
 *      Get the field data from specified ACL entry buffer.
 * Input:
 *      unit          - unit id
 *      phase         - ACL lookup phase
 *      entry_idx     - ACL entry index
 *      pEntry_buffer - data buffer of ACL entry
 *      type          - field type
 * Output:
 *      pData         - pointer buffer of field data
 *      pMask         - pointer buffer of field mask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_ACL_PHASE      - invalid ACL phase
 *      RT_ERR_ENTRY_INDEX    - invalid entry index
 *      RT_ERR_ACL_FIELD_TYPE - invalid entry field type
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      The API reads the field data/mask from the entry buffer. Use rtk_acl_ruleEntry_read to
 *      read the rule data to the entry buffer.
 */
int32
rtk_acl_ruleEntryField_get(
    uint32              unit,
    rtk_acl_phase_t     phase,
    rtk_acl_id_t        entry_idx,
    uint8               *pEntry_buffer,
    rtk_acl_fieldType_t type,
    uint8               *pData,
    uint8               *pMask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_ruleEntryField_get(unit, phase, entry_idx, pEntry_buffer, type, pData, pMask);
} /* end of rtk_acl_ruleEntryField_get */

/* Function Name:
 *      rtk_acl_ruleEntryField_set
 * Description:
 *      Set the field data to specified ACL entry buffer.
 * Input:
 *      unit          - unit id
 *      phase         - ACL lookup phase
 *      entry_idx     - ACL entry index
 *      pEntry_buffer - data buffer of ACL entry
 *      type          - field type
 *      pData         - pointer buffer of field data
 *      pMask         - pointer buffer of field mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_ACL_PHASE      - invalid ACL phase
 *      RT_ERR_ENTRY_INDEX    - invalid entry index
 *      RT_ERR_ACL_FIELD_TYPE - invalid entry field type
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      The API writes the field data/mask to the entry buffer. After the fields are configured,
 *      use rtk_acl_ruleEntry_write to write the entry buffer to ASIC at once.
 */
int32
rtk_acl_ruleEntryField_set(
    uint32              unit,
    rtk_acl_phase_t     phase,
    rtk_acl_id_t        entry_idx,
    uint8               *pEntry_buffer,
    rtk_acl_fieldType_t type,
    uint8               *pData,
    uint8               *pMask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_ruleEntryField_set(unit, phase, entry_idx, pEntry_buffer, type, pData, pMask);
} /* end of rtk_acl_ruleEntryField_set */

/* Function Name:
 *      rtk_acl_ruleEntryField_read
 * Description:
 *      Read the field data from specified ACL entry.
 * Input:
 *      unit      - unit id
 *      phase     - ACL lookup phase
 *      entry_idx - ACL entry index
 *      type      - field type
 * Output:
 *      pData     - pointer buffer of field data
 *      pMask     - pointer buffer of field mask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_ACL_PHASE      - invalid ACL phase
 *      RT_ERR_ENTRY_INDEX    - invalid entry index
 *      RT_ERR_ACL_FIELD_TYPE - invalid entry field type
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_acl_ruleEntryField_read(
    uint32              unit,
    rtk_acl_phase_t     phase,
    rtk_acl_id_t        entry_idx,
    rtk_acl_fieldType_t type,
    uint8               *pData,
    uint8               *pMask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_ruleEntryField_read(unit, phase, entry_idx, type, pData, pMask);
} /* end of rtk_acl_ruleEntryField_read */

/* Function Name:
 *      rtk_acl_ruleEntryField_write
 * Description:
 *      Write the field data to specified ACL entry.
 * Input:
 *      unit      - unit id
 *      phase     - ACL lookup phase
 *      entry_idx - ACL entry index
 *      type      - field type
 *      pData     - pointer buffer of field data
 *      pMask     - pointer buffer of field mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_ACL_PHASE      - invalid ACL phase
 *      RT_ERR_ENTRY_INDEX    - invalid entry index
 *      RT_ERR_ACL_FIELD_TYPE - invalid entry field type
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_acl_ruleEntryField_write(
    uint32              unit,
    rtk_acl_phase_t     phase,
    rtk_acl_id_t        entry_idx,
    rtk_acl_fieldType_t type,
    uint8               *pData,
    uint8               *pMask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_ruleEntryField_write(unit, phase, entry_idx, type, pData, pMask);
} /* end of rtk_acl_ruleEntryField_write */

/* Function Name:
 *      rtk_acl_ruleOperation_get
 * Description:
 *      Get ACL rule operation.
 * Input:
 *      unit       - unit id
 *      phase      - ACL lookup phase
 *      entry_idx  - ACL entry index
 * Output:
 *      pOperation - operation configuration
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ACL_PHASE    - invalid ACL phase
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) For reverse operation, valid index is N where N = 0,1,2...
 *      (2) For aggr_1 operation, index must be 2N where N = 0,1,2...
 *      (3) For aggr_2 operation, index must be 2N+256M where N,M = 0,1,2...
 *      (4) For aggregating 4 rules, both aggr_1 and aggr_2 must be enabled.
 */
int32
rtk_acl_ruleOperation_get(
    uint32                  unit,
    rtk_acl_phase_t         phase,
    rtk_acl_id_t            entry_idx,
    rtk_acl_operation_t     *pOperation)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_ruleOperation_get(unit, phase, entry_idx, pOperation);
} /* end of rtk_acl_ruleOperation_get */

/* Function Name:
 *      rtk_acl_ruleOperation_set
 * Description:
 *      Set ACL rule operation.
 * Input:
 *      unit       - unit id
 *      phase      - ACL lookup phase
 *      entry_idx  - ACL entry index
 *      pOperation - operation configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ACL_PHASE    - invalid ACL phase
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) For reverse operation, valid index is N where N = 0,1,2...
 *      (2) For aggr_1 operation, index must be 2N where N = 0,1,2...
 *      (3) For aggr_2 operation, index must be 2N+256M where N,M = 0,1,2...
 *      (4) For aggregating 4 rules, both aggr_1 and aggr_2 must be enabled.
 */
int32
rtk_acl_ruleOperation_set(
    uint32                  unit,
    rtk_acl_phase_t         phase,
    rtk_acl_id_t            entry_idx,
    rtk_acl_operation_t     *pOperation)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_ruleOperation_set(unit, phase, entry_idx, pOperation);
} /* end of rtk_acl_ruleOperation_set */

/* Function Name:
 *      rtk_acl_ruleAction_get
 * Description:
 *      Get the ACL rule action configuration.
 * Input:
 *      unit      - unit id
 *      phase     - ACL lookup phase
 *      entry_idx - ACL entry index
 * Output:
 *      pAction   - action mask and data configuration
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ACL_PHASE    - invalid ACL phase
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      For ingress ACL phase, the data is put to pAction->igr_acl.
 *      For egress ACL phase, the data is put to pAction->egr_acl.
 */
int32
rtk_acl_ruleAction_get(
    uint32               unit,
    rtk_acl_phase_t      phase,
    rtk_acl_id_t         entry_idx,
    rtk_acl_action_t     *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_ruleAction_get(unit, phase, entry_idx, pAction);
} /* end of rtk_acl_ruleAction_get */

/* Function Name:
 *      rtk_acl_ruleAction_set
 * Description:
 *      Set the ACL rule action configuration.
 * Input:
 *      unit      - unit id
 *      phase     - ACL lookup phase
 *      entry_idx - ACL entry index
 *      pAction   - action mask and data configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ACL_PHASE    - invalid ACL phase
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      For ingress ACL phase, pAction->igr_acl is used for programming.
 *      For egress ACL phase, pAction->egr_acl is used for programming.
 */
int32
rtk_acl_ruleAction_set(
    uint32               unit,
    rtk_acl_phase_t      phase,
    rtk_acl_id_t         entry_idx,
    rtk_acl_action_t     *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_ruleAction_set(unit, phase, entry_idx, pAction);
} /* end of rtk_acl_ruleAction_set */

/* Function Name:
 *      rtk_acl_ruleHitIndication_get
 * Description:
 *      Get the ACL rule hit indication.
 * Input:
 *      unit        - unit id
 *      phase       - ACL lookup phase
 *      entry_idx   - ACL entry index
 *      reset       - reset the hit status
 * Output:
 *      pIsHit      - pointer to hit status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ACL_PHASE    - invalid ACL phase
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      The hit status can be cleared by configuring para. 'reset' to 1.
 */
int32
rtk_acl_ruleHitIndication_get(
    uint32              unit,
    rtk_acl_phase_t     phase,
    rtk_acl_id_t        entry_idx,
    uint32              reset,
    uint32              *pIsHit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_ruleHitIndication_get(unit, phase, entry_idx, reset, pIsHit);
} /* end of rtk_acl_ruleHitIndication_get */

/* Function Name:
 *      rtk_acl_rule_del
 * Description:
 *      Delete the specified ACL rules.
 * Input:
 *      unit    - unit id
 *      phase   - ACL lookup phase
 *      pClrIdx - rule index to clear
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_ACL_PHASE       - invalid ACL phase
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 *      RT_ERR_ACL_CLEAR_INDEX - end index is lower than start index
 *      RT_ERR_ENTRY_INDEX     - invalid entry index
 * Applicable:
 *      8390, 8380
 * Note:
 *      Entry fields, operations and actions are all cleared.
 */
int32
rtk_acl_rule_del(uint32 unit, rtk_acl_phase_t phase, rtk_acl_clear_t *pClrIdx)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_rule_del(unit, phase, pClrIdx);
} /* end of rtk_acl_rule_del */

/* Function Name:
 *      rtk_acl_rule_move
 * Description:
 *      Move the specified ingress ACL rules.
 * Input:
 *      unit  - unit id
 *      phase - ACL lookup phase
 *      pData - movement info
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ACL_PHASE    - invalid ACL phase
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) Entry fields, operations and actions are all moved.
 *      (2) The vacant entries due to movement are auto cleared to be invalid by H/W.
 *      (3) (move_from + length) and (move_to + length) must <= the number of ACL rule
 */
int32
rtk_acl_rule_move(uint32 unit, rtk_acl_phase_t phase, rtk_acl_move_t *pData)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_rule_move(unit, phase, pData);
} /* end of rtk_acl_rule_move */

/* Function Name:
 *      rtk_acl_templateSelector_get
 * Description:
 *      Get the mapping template of specific block.
 * Input:
 *      unit          - unit id
 *      block_idx     - block index
 * Output:
 *      pTemplate_idx - template index
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_ACL_BLOCK_INDEX - invalid block index
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_acl_templateSelector_get(
    uint32                  unit,
    uint32                  block_idx,
    rtk_acl_templateIdx_t   *pTemplate_idx)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_templateSelector_get(unit, block_idx, pTemplate_idx);
} /* end of rtk_acl_templateSelector_get */

/* Function Name:
 *      rtk_acl_templateSelector_set
 * Description:
 *      Set the mapping template of specific block.
 * Input:
 *      unit         - unit id
 *      block_idx    - block index
 *      template_idx - template index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                   - invalid unit id
 *      RT_ERR_NOT_INIT                  - The module is not initial
 *      RT_ERR_ACL_BLOCK_INDEX           - invalid block index
 *      RT_ERR_ACL_TEMPLATE_INDEX        - invalid template index
 *      RT_ERR_ACL_TEMPLATE_INCOMPATIBLE - try to map a ACL block to an incompatible template
 *      RT_ERR_INPUT                     - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_acl_templateSelector_set(
    uint32                  unit,
    uint32                  block_idx,
    rtk_acl_templateIdx_t   template_idx)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_templateSelector_set(unit, block_idx, template_idx);
} /* end of rtk_acl_templateSelector_set */

/* Function Name:
 *      rtk_acl_template_get
 * Description:
 *      Get the content of specific template.
 * Input:
 *      unit        - unit id
 *      template_id - template ID
 * Output:
 *      pTemplate   - template content
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID            - invalid unit id
 *      RT_ERR_NOT_INIT           - The module is not initial
 *      RT_ERR_ACL_TEMPLATE_INDEX - invalid template index
 *      RT_ERR_NULL_POINTER       - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      ID 0-4 are fixed template, ID 5-7 are user defined template.
 */
int32
rtk_acl_template_get(uint32 unit, uint32 template_id, rtk_acl_template_t *pTemplate)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_template_get(unit, template_id, pTemplate);
} /* end of rtk_acl_template_get */

/* Function Name:
 *      rtk_acl_template_set
 * Description:
 *      Set the content of specific template.
 * Input:
 *      unit        - unit id
 *      template_id - template ID
 *      pTemplate   - template content
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID            - invalid unit id
 *      RT_ERR_NOT_INIT           - The module is not initial
 *      RT_ERR_ACL_TEMPLATE_INDEX - invalid template index
 *      RT_ERR_ACL_FIELD_TYPE     - invalid ACL field type
 *      RT_ERR_ACL_FIELD_LOCATION - invalid field location
 *      RT_ERR_NULL_POINTER       - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) ID 0-4 are fixed template, ID 5-7 are user defined template.
 *      (2) Configure fixed templates is prohibited.
 */
int32
rtk_acl_template_set(uint32 unit, uint32 template_id, rtk_acl_template_t *pTemplate)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_template_set(unit, template_id, pTemplate);
} /* end of rtk_acl_template_set */

/* Function Name:
 *      rtk_acl_templateField_check
 * Description:
 *      Check whether the specified template field type is supported on the chip.
 * Input:
 *      unit  - unit id
 *      phase - ACL lookup phase
 *      type  - template field type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_ACL_PHASE      - invalid ACL phase
 *      RT_ERR_ACL_FIELD_TYPE - invalid ACL field type
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_acl_templateField_check(
    uint32                      unit,
    rtk_acl_phase_t             phase,
    rtk_acl_templateFieldType_t type)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_templateField_check(unit, phase, type);
} /* end of rtk_acl_templateField_check */

/* Function Name:
 *      rtk_acl_blockResultMode_get
 * Description:
 *      Get the acl block result mode.
 * Input:
 *      unit      - unit id
 *      block_idx - block index
 * Output:
 *      pMode     - block result mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_ACL_BLOCK_INDEX - invalid block index
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      If a packet hit multiple rules and the mode is configured to ACL_BLOCK_RESULT_SINGLE, then
 *      the hit result will be the rule with the lowest index.
 */
int32
rtk_acl_blockResultMode_get(uint32 unit, uint32 block_idx, rtk_acl_blockResultMode_t *pMode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_blockResultMode_get(unit, block_idx, pMode);
} /* end of rtk_acl_blockResultMode_get */

/* Function Name:
 *      rtk_acl_blockResultMode_set
 * Description:
 *      Set the acl block result mode.
 * Input:
 *      unit      - unit id
 *      block_idx - block index
 *      mode      - block result mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_ACL_BLOCK_INDEX - invalid block index
 *      RT_ERR_INPUT           - invalid input parameter
 * Applicable:
 *      8390
 * Note:
 *      If a packet hit multiple rules and the mode is configured to ACL_BLOCK_RESULT_SINGLE, then
 *      the hit result will be the rule with the lowest index.
 */
int32
rtk_acl_blockResultMode_set(uint32 unit, uint32 block_idx, rtk_acl_blockResultMode_t mode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_blockResultMode_set(unit, block_idx, mode);
} /* end of rtk_acl_blockResultMode_set */

/* Function Name:
 *      rtk_acl_blockGroupEnable_get
 * Description:
 *      Set the block grouping.
 * Input:
 *      unit       - unit id
 *      block_idx  - block index
 *      group_type - grouping type
 * Output:
 *      pEnable    - enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_ACL_BLOCK_INDEX - invalid block index
 *      RT_ERR_INPUT           - invalid input parameter
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) If multiple physical blocks are grouped to a logical block,
 *          it only outputs a single hit result and the hit result will be
 *          the entry with lowest index.
 *      (2) Group ingress ACL block with egress ACL block is forbidden.
 *      (3) For ACL_BLOCK_GROUP_2, valid index is 2N where N = 0,1...
 *      (4) For ACL_BLOCK_GROUP_4, valid index is 4N where N = 0,1...
 *      (5) For ACL_BLOCK_GROUP_8, valid index is 8N where N = 0,1...
 *      (6) For ACL_BLOCK_GROUP_ALL, valid index is 0.
 *      (7) If multiple grouping types are applied to the same block index, then
 *          the priority will be ACL_BLOCK_GROUP_ALL > ACL_BLOCK_GROUP_8 >
 *          ACL_BLOCK_GROUP_4 > ACL_BLOCK_GROUP_2.
 *      (8) ACL_BLOCK_GROUP_1 is applicable to 8380 only.
 */
int32
rtk_acl_blockGroupEnable_get(
    uint32                     unit,
    uint32                     block_idx,
    rtk_acl_blockGroup_t       group_type,
    rtk_enable_t               *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_blockGroupEnable_get(unit, block_idx, group_type, pEnable);
} /* end of rtk_acl_blockGroupEnable_get */

/* Function Name:
 *      rtk_acl_blockGroupEnable_set
 * Description:
 *      Set the block grouping.
 * Input:
 *      unit       - unit id
 *      block_idx  - block index
 *      group_type - grouping type
 *      enable     - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_ACL_BLOCK_INDEX - invalid block index
 *      RT_ERR_INPUT           - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) If multiple physical blocks are grouped to a logical block,
 *          it only outputs a single hit result and the hit result will be
 *          the entry with lowest index.
 *      (2) Group ingress ACL block with egress ACL block is forbidden.
 *      (3) For ACL_BLOCK_GROUP_2, valid index is 2N where N = 0,1...
 *      (4) For ACL_BLOCK_GROUP_4, valid index is 4N where N = 0,1...
 *      (5) For ACL_BLOCK_GROUP_8, valid index is 8N where N = 0,1...
 *      (6) For ACL_BLOCK_GROUP_ALL, valid index is 0.
 *      (7) If multiple grouping types are applied to the same block index, then
 *          the priority will be ACL_BLOCK_GROUP_ALL > ACL_BLOCK_GROUP_8 >
 *          ACL_BLOCK_GROUP_4 > ACL_BLOCK_GROUP_2.
 *      (8) ACL_BLOCK_GROUP_1 is applicable to 8380 only.
 */
int32
rtk_acl_blockGroupEnable_set(
    uint32                     unit,
    uint32                     block_idx,
    rtk_acl_blockGroup_t       group_type,
    rtk_enable_t               enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_blockGroupEnable_set(unit, block_idx, group_type, enable);
} /* end of rtk_acl_blockGroupEnable_set */

/* Function Name:
 *      rtk_acl_statPktCnt_get
 * Description:
 *      Get packet-based statistic counter of the log id.
 * Input:
 *      unit     - unit id
 *      log_id   - log id
 * Output:
 *      pPkt_cnt - pointer buffer of packet count
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
int32
rtk_acl_statPktCnt_get(uint32 unit, uint32 log_id, uint32 *pPkt_cnt)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_statPktCnt_get(unit, log_id, pPkt_cnt);
} /* end of rtk_acl_statPktCnt_get */

/* Function Name:
 *      rtk_acl_statPktCnt_clear
 * Description:
 *      Set packet-based statistic counter of the log id.
 * Input:
 *      unit   - unit id
 *      log_id - log id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_ENTRY_INDEX - invalid entry index
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_acl_statPktCnt_clear(uint32 unit, uint32 log_id)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_statPktCnt_clear(unit, log_id);
} /* end of rtk_acl_statPktCnt_clear */

/* Function Name:
 *      rtk_acl_statByteCnt_get
 * Description:
 *      Get byte-based statistic counter of the log id.
 * Input:
 *      unit      - unit id
 *      log_id    - log id
 * Output:
 *      pByte_cnt - byte count
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
int32
rtk_acl_statByteCnt_get(uint32 unit, uint32 log_id, uint64 *pByte_cnt)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_statByteCnt_get(unit, log_id, pByte_cnt);
} /* end of rtk_acl_statByteCnt_get */

/* Function Name:
 *      rtk_acl_statByteCnt_clear
 * Description:
 *      Reset byte-based statistic counter of the log id.
 * Input:
 *      unit   - unit id
 *      log_id - log id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_ENTRY_INDEX - invalid entry index
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_acl_statByteCnt_clear(uint32 unit, uint32 log_id)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_statByteCnt_clear(unit, log_id);
} /* end of rtk_acl_statByteCnt_clear */

/* Function Name:
 *      rtk_acl_stat_clearAll
 * Description:
 *      Clear all statistic counter for the specified device.
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
 *      8390, 8380
 * Note:
 *      All the packet counters and byte counters are cleared.
 */
int32
rtk_acl_stat_clearAll(uint32 unit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_stat_clearAll(unit);
} /* end of rtk_acl_stat_clearAll */

/* Sub-module Name :
 *  Range Check
 */

/* Function Name:
 *      rtk_acl_rangeCheckL4Port_get
 * Description:
 *      Get the configuration of L4 port range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 * Output:
 *      pData - configuration of L4 port
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
int32
rtk_acl_rangeCheckL4Port_get(uint32 unit, uint32 index, rtk_acl_rangeCheck_l4Port_t *pData)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_rangeCheckL4Port_get(unit, index, pData);
} /* end of rtk_acl_rangeCheckL4Port_get */

/* Function Name:
 *      rtk_acl_rangeCheckL4Port_set
 * Description:
 *      Set the configuration of L4 port range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 *      pData - configuration of L4 port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_acl_rangeCheckL4Port_set(uint32 unit, uint32 index, rtk_acl_rangeCheck_l4Port_t *pData)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_rangeCheckL4Port_set(unit, index, pData);
} /* end of rtk_acl_rangeCheckL4Port_set */

/* Function Name:
 *      rtk_acl_rangeCheckVid_get
 * Description:
 *      Get the configuration of VID range check.
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
 *      For untag packet, P-VID is used.
 */
int32
rtk_acl_rangeCheckVid_get(uint32 unit, uint32 index, rtk_acl_rangeCheck_vid_t *pData)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_rangeCheckVid_get(unit, index, pData);
} /* end of rtk_acl_rangeCheckVid_get */

/* Function Name:
 *      rtk_acl_rangeCheckVid_set
 * Description:
 *      Set the configuration of VID range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 *      pData - configuration of VID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      For untag packet, P-VID is used.
 */
int32
rtk_acl_rangeCheckVid_set(uint32 unit, uint32 index, rtk_acl_rangeCheck_vid_t *pData)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_rangeCheckVid_set(unit, index, pData);
} /* end of rtk_acl_rangeCheckVid_set */

/* Function Name:
 *      rtk_acl_rangeCheckIp_get
 * Description:
 *      Get the configuration of IP range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 * Output:
 *      pData - configuration of IP
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
 *      (1) For IPv6 range check, index 0/4 means IP6[31:0], index 1/5 means IP6[63:32],
 *          index 2/6 means IP6[95:64], index 3/7 means IP6[127:96]. Index 0~3/4~7 must
 *          be used together in order to filter a full IPv6 address.
 *      (2) For IPv6 suffix range check, index 0/2/4/6 means IP6[31:0], index 1/3/5/7 means IP6[63:32],
 *          Index 0&1/2&3/4&5/6&7 must be used together in order to filter a IPv6 suffix address.
 */
int32
rtk_acl_rangeCheckIp_get(uint32 unit, uint32 index, rtk_acl_rangeCheck_ip_t *pData)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_rangeCheckIp_get(unit, index, pData);
} /* end of rtk_acl_rangeCheckIp_get */

/* Function Name:
 *      rtk_acl_rangeCheckIp_set
 * Description:
 *      Set the configuration of IP range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 *      pData - configuration of IP
 * Output:
 *      None
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
 *      (1) For IPv6 range check, index 0/4 means IP6[31:0], index 1/5 means IP6[63:32],
 *          index 2/6 means IP6[95:64], index 3/7 means IP6[127:96]. Index 0~3/4~7 must
 *          be used together in order to filter a full IPv6 address.
 *      (2) For IPv6 suffix range check, index 0/2/4/6 means IP6[31:0], index 1/3/5/7 means IP6[63:32],
 *          Index 0&1/2&3/4&5/6&7 must be used together in order to filter a IPv6 suffix address.
 */
int32
rtk_acl_rangeCheckIp_set(uint32 unit, uint32 index, rtk_acl_rangeCheck_ip_t *pData)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_rangeCheckIp_set(unit, index, pData);
} /* end of rtk_acl_rangeCheckIp_set */

/* Function Name:
 *      rtk_acl_rangeCheckSrcPort_get
 * Description:
 *      Get the configuration of source port range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 * Output:
 *      pData - configuration of source port
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
int32
rtk_acl_rangeCheckSrcPort_get(uint32 unit, uint32 index, rtk_acl_rangeCheck_portMask_t *pData)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32   ret;
    rtk_acl_rangeCheck_portMask_t  temp_data;

    if ((ret = RT_MAPPER(unit)->acl_rangeCheckSrcPort_get(unit, index, &temp_data)) != RT_ERR_OK)
        return ret;
    *pData = temp_data;
    (*pData).port_mask = PHYSICAL_PORTMASK_TO_RTK_PORTMASK(unit, temp_data.port_mask);
    return ret;
    }
#else
    return RT_MAPPER(unit)->acl_rangeCheckSrcPort_get(unit, index, pData);
#endif
} /* end of rtk_acl_rangeCheckSrcPort_get */

/* Function Name:
 *      rtk_acl_rangeCheckSrcPort_set
 * Description:
 *      Set the configuration of source port range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 *      pData - configuration of source port
 * Output:
 *      None
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
int32
rtk_acl_rangeCheckSrcPort_set(uint32 unit, uint32 index, rtk_acl_rangeCheck_portMask_t *pData)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    rtk_acl_rangeCheck_portMask_t  temp_data;

    temp_data = *pData;
    temp_data.port_mask = RTK_PORTMASK_TO_PHYSICAL_PORTMASK(unit, (*pData).port_mask);
    return RT_MAPPER(unit)->acl_rangeCheckSrcPort_set(unit, index, &temp_data);
    }
#else
    return RT_MAPPER(unit)->acl_rangeCheckSrcPort_set(unit, index, pData);
#endif
} /* end of rtk_acl_rangeCheckSrcPort_set */

/* Function Name:
 *      rtk_acl_rangeCheckDstPort_get
 * Description:
 *      Get the configuration of destination port range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 * Output:
 *      pData - configuration of destination port
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
rtk_acl_rangeCheckDstPort_get(uint32 unit, uint32 index, rtk_acl_rangeCheck_portMask_t *pData)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_rangeCheckDstPort_get(unit, index, pData);
} /* end of rtk_acl_rangeCheckDstPort_get */

/* Function Name:
 *      rtk_acl_rangeCheckDstPort_set
 * Description:
 *      Set the configuration of destination port range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 *      pData - configuration of destination port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
rtk_acl_rangeCheckDstPort_set(uint32 unit, uint32 index, rtk_acl_rangeCheck_portMask_t *pData)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_rangeCheckDstPort_set(unit, index, pData);
} /* end of rtk_acl_rangeCheckDstPort_set */

/* Function Name:
 *      rtk_acl_rangeCheckPacketLen_get
 * Description:
 *      Get the configuration of packet length range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 * Output:
 *      pData - configuration of packet length
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
 *      Packet length includes CRC(4Byte)
 */
int32
rtk_acl_rangeCheckPacketLen_get(uint32 unit, uint32 index, rtk_acl_rangeCheck_packetLen_t *pData)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_rangeCheckPacketLen_get(unit, index, pData);
} /* end of rtk_acl_rangeCheckPacketLen_get */

/* Function Name:
 *      rtk_acl_rangeCheckPacketLen_set
 * Description:
 *      Set the configuration of packet length range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 *      pData - configuration of packet length
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      Packet length includes CRC(4Byte)
 */
int32
rtk_acl_rangeCheckPacketLen_set(uint32 unit, uint32 index, rtk_acl_rangeCheck_packetLen_t *pData)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_rangeCheckPacketLen_set(unit, index, pData);
} /* end of rtk_acl_rangeCheckPacketLen_set */

/* Function Name:
 *      rtk_acl_rangeCheckFieldSelector_get
 * Description:
 *      Get the configuration of field selector range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 * Output:
 *      pData - configuration of field selector
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      None
 */
int32
rtk_acl_rangeCheckFieldSelector_get(uint32 unit, uint32 index, rtk_acl_rangeCheck_fieldSelector_t *pData)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_rangeCheckFieldSelector_get(unit, index, pData);
} /* end of rtk_acl_rangeCheckFieldSelector_get */

/* Function Name:
 *      rtk_acl_rangeCheckFieldSelector_set
 * Description:
 *      Set the configuration of field selector range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 *      pData - configuration of field selector
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8380
 * Note:
 *      None
 */
int32
rtk_acl_rangeCheckFieldSelector_set(uint32 unit, uint32 index, rtk_acl_rangeCheck_fieldSelector_t *pData)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_rangeCheckFieldSelector_set(unit, index, pData);
} /* end of rtk_acl_rangeCheckFieldSelector_set */

/* Sub-module Name :
 *  Field Selector
 */

/* Function Name:
 *      rtk_acl_fieldSelector_get
 * Description:
 *      Get the configuration of field selector.
 * Input:
 *      unit   - unit id
 *      fs_idx - field selector index
 * Output:
 *      pFs    - configuration of field selector.
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
 *      For 8390, only the first 180B(since DA) of packet can be inspected by field selector.
 *      Thus, (start position + offest) must less than 164B(since DA) in order to grab a
 *      complete 16-bit user define field.
 */
int32
rtk_acl_fieldSelector_get(
    uint32                       unit,
    uint32                       fs_idx,
    rtk_acl_fieldSelector_data_t *pFs)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_fieldSelector_get(unit, fs_idx, pFs);
} /* end of rtk_acl_fieldSelector_get */

/* Function Name:
 *      rtk_acl_fieldSelector_set
 * Description:
 *      Set the configuration of field selector.
 * Input:
 *      unit   - unit id
 *      fs_idx - field selector index
 *      pFs    - configuration of field selector.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      For 8390, only the first 180B(since DA) of packet can be inspected by field selector.
 *      Thus, (start position + offest) must less than 164B(since DA) in order to grab a
 *      complete 16-bit user define field.
 */
int32
rtk_acl_fieldSelector_set(
    uint32                       unit,
    uint32                       fs_idx,
    rtk_acl_fieldSelector_data_t *pFs)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_fieldSelector_set(unit, fs_idx, pFs);
} /* end of rtk_acl_fieldSelector_set */

/* Sub-module Name :
 *  Field Selector
 */

/* Function Name:
 *      rtk_acl_portFieldSelector_get
 * Description:
 *      Get the per-port configuration of field selector .
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      fs_idx - field selector index
 * Output:
 *      pFs    - configuration of field selector.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      For 8380, filed selector can be configured by port.
 */
int32
rtk_acl_portFieldSelector_get(
    uint32                       unit,
    rtk_port_t                   port,
    uint32                       fs_idx,
    rtk_acl_fieldSelector_data_t *pFs)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->acl_portFieldSelector_get(unit, phy_port, fs_idx, pFs);
    }
#else
    return RT_MAPPER(unit)->acl_portFieldSelector_get(unit, port, fs_idx, pFs);
#endif
} /* end of rtk_acl_portFieldSelector_get */

/* Function Name:
 *      rtk_acl_portFieldSelector_set
 * Description:
 *      Set the configuration of field selector.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      fs_idx - field selector index
 *      pFs    - configuration of field selector.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8380
 * Note:
 *      For 8380, filed selector can be configured by port.
 */
int32
rtk_acl_portFieldSelector_set(
    uint32                       unit,
    rtk_port_t                   port,
    uint32                       fs_idx,
    rtk_acl_fieldSelector_data_t *pFs)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->acl_portFieldSelector_set(unit, phy_port, fs_idx, pFs);
    }
#else
    return RT_MAPPER(unit)->acl_portFieldSelector_set(unit, port, fs_idx, pFs);
#endif
} /* end of rtk_acl_portFieldSelector_set */

/* Function Name:
 *      rtk_acl_meterMode_get
 * Description:
 *      Get the meter mode of a specific meter block.
 * Input:
 *      unit       - unit id
 *      idx        - meter block ID for 8390, meter entry ID for 8380
 * Output:
 *      pMeterMode - meter mode:byte based or packet based
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_acl_meterMode_get(
    uint32              unit,
    uint32              idx,
    rtk_acl_meterMode_t *pMeterMode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_meterMode_get(unit, idx, pMeterMode);
} /* end of rtk_acl_meterMode_get */

/* Function Name:
 *      rtk_acl_meterMode_set
 * Description:
 *      Set the meter mode.
 * Input:
 *      unit      - unit id
 *      idx       - meter block ID for 8390, meter entry ID for 8380
 *      meterMode - meter mode (byte based or packet based)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_ENTRY_INDEX - invalid entry index
 *      RT_ERR_INPUT       - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_acl_meterMode_set(
    uint32              unit,
    uint32              idx,
    rtk_acl_meterMode_t meterMode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_meterMode_set(unit, idx, meterMode);
} /* end of rtk_acl_meterMode_set */

/* Function Name:
 *      rtk_acl_meterIncludeIfg_get
 * Description:
 *      Get IFG including status for packet length calculation of meter module.
 * Input:
 *      unit         - unit id
 * Output:
 *      pIfg_include - pointer to enable status of includes IFG
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
int32
rtk_acl_meterIncludeIfg_get(uint32 unit, rtk_enable_t *pIfg_include)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_meterIncludeIfg_get(unit, pIfg_include);
} /* end of rtk_acl_meterIncludeIfg_get */

/* Function Name:
 *      rtk_acl_meterIncludeIfg_set
 * Description:
 *      Set IFG including status for packet length calculation of meter module.
 * Input:
 *      unit        - unit id
 *      ifg_include - enable status of includes IFG
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
int32
rtk_acl_meterIncludeIfg_set(uint32 unit, rtk_enable_t ifg_include)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_meterIncludeIfg_set(unit, ifg_include);
} /* end of rtk_acl_meterIncludeIfg_set */

/* Function Name:
 *      rtk_acl_meterBurstSize_get
 * Description:
 *      Get the meter burst sizes of a specific meter mode.
 * Input:
 *      unit       - unit id
 *      meterMode  - meter mode
 * Output:
 *      pBurstSize - pointer to burst sizes
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      double leaky packet/ single rate three color/ two rate three color type
 *      have separated burst size settings.
 */
int32
rtk_acl_meterBurstSize_get(
    uint32                      unit,
    rtk_acl_meterMode_t         meterMode,
    rtk_acl_meterBurstSize_t    *pBurstSize)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_meterBurstSize_get(unit, meterMode, pBurstSize);
} /* end of rtk_acl_meterBurstSize_get */

/* Function Name:
 *      rtk_acl_meterBurstSize_set
 * Description:
 *      Set the meter burst sizes of a specific meter mode.
 * Input:
 *      unit       - unit id
 *      meterMode  - meter mode (byte based or packet based)
 *      pBurstSize - pointer to burst sizes
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) Double leaky packet/ single rate three color/ two rate three color type
 *          have separated burst size settings.
 *      (2) In meterMode = METER_MODE_BYTE of 8390, the (minimum, maximum) of pBurstSize->dlb_lb0bs and 
 *          pBurstSize->dlb_lb1bs setting range is (17, 65535). 
 */
int32
rtk_acl_meterBurstSize_set(
    uint32                      unit,
    rtk_acl_meterMode_t         meterMode,
    rtk_acl_meterBurstSize_t    *pBurstSize)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_meterBurstSize_set(unit, meterMode, pBurstSize);
} /* end of rtk_acl_meterBurstSize_set */

/* Function Name:
 *      rtk_acl_meterExceed_get
 * Description:
 *      Get the meter exceed flag of a meter entry.
 * Input:
 *      unit      - unit id
 *      meterIdx  - meter entry index
 * Output:
 *      pIsExceed - pointer to exceed flag
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
int32
rtk_acl_meterExceed_get(
    uint32  unit,
    uint32  meterIdx,
    uint32  *pIsExceed)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_meterExceed_get(unit, meterIdx, pIsExceed);
} /* end of rtk_acl_meterExceed_get */


/* Function Name:
 *      rtk_acl_meterExceedAggregation_get
 * Description:
 *      Get the meter exceed flag mask of meter entry exceed aggregated result every 16 entries.
 * Input:
 *      unit      - unit id
 * Output:
 *      pExceedMask - pointer to aggregated exceed flag mask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
rtk_acl_meterExceedAggregation_get(
    uint32  unit,
    uint32  *pExceedMask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_meterExceedAggregation_get(unit, pExceedMask);
} /* end of rtk_acl_meterExceedAggregation_get */

/* Function Name:
 *      rtk_acl_meterEntry_get
 * Description:
 *      Get the content of a meter entry.
 * Input:
 *      unit        - unit id
 *      meterIdx    - meter entry index
 * Output:
 *      pMeterEntry - pointer to a meter entry
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
int32
rtk_acl_meterEntry_get(
    uint32                  unit,
    uint32                  meterIdx,
    rtk_acl_meterEntry_t    *pMeterEntry)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_meterEntry_get(unit, meterIdx, pMeterEntry);
} /* end of rtk_acl_meterEntry_get */

/* Function Name:
 *      rtk_acl_meterEntry_set
 * Description:
 *      Set a meter entry.
 * Input:
 *      unit        - unit id
 *      meterIdx    - meter entry index
 *      pMeterEntry - pointer to meter entry
 * Output:
 *      None
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
int32
rtk_acl_meterEntry_set(
    uint32                  unit,
    uint32                  meterIdx,
    rtk_acl_meterEntry_t    *pMeterEntry)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_meterEntry_set(unit, meterIdx, pMeterEntry);
} /* end of rtk_acl_meterEntry_set */

/* Function Name:
 *      rtk_acl_ruleEntryField_check
 * Description:
 *      Check whether the specified field type is supported on the chip.
 * Input:
 *      unit  - unit id
 *      phase - ACL lookup phase
 *      type  - field type to be checked
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_ACL_PHASE      - invalid ACL phase
 *      RT_ERR_ACL_FIELD_TYPE - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_acl_ruleEntryField_check(
    uint32              unit,
    rtk_acl_phase_t     phase,
    rtk_acl_fieldType_t type)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->acl_ruleEntryField_check(unit, phase, type);
} /* end of rtk_acl_ruleEntryField_check */

/* Function Name:
 *      rtk_acl_templateFieldIntentVlanTag_get
 * Description:
 *      Get the acl template field VLAN tag status
 * Input:
 *      unit     - unit id
 * Output:
 *      tagType  - template field VLAN tag status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
rtk_acl_templateFieldIntentVlanTag_get(uint32 unit,
    rtk_vlan_tagType_t *tagType)
{
    /* function body */
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    if (NULL == RT_MAPPER(unit)->acl_templateFieldIntentVlanTag_get)
        return RT_ERR_DRIVER_NOT_FOUND;
    return RT_MAPPER(unit)->acl_templateFieldIntentVlanTag_get(unit, tagType);
}   /* end of rtk_acl_templateFieldIntentVlanTag_get */

/* Function Name:
 *      rtk_acl_templateFieldIntentVlanTag_set
 * Description:
 *      Set the acl template field VLAN tag status
 * Input:
 *      unit     - unit id
 *      tagType  - template field VLAN tag status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
rtk_acl_templateFieldIntentVlanTag_set(uint32 unit,
    rtk_vlan_tagType_t tagType)
{
    /* function body */
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    if (NULL == RT_MAPPER(unit)->acl_templateFieldIntentVlanTag_set)
        return RT_ERR_DRIVER_NOT_FOUND;
    return RT_MAPPER(unit)->acl_templateFieldIntentVlanTag_set(unit, tagType);
}   /* end of rtk_acl_templateFieldIntentVlanTag_set */
