/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008-2009
 * All rights reserved.
 *
 * $Revision: 20135 $
 * $Date: 2011-07-28 17:51:53 +0800 (Thu, 28 Jul 2011) $
 *
 * Purpose : Definition those public PIE APIs and its data type in the SDK .
 *
 * Feature : The file have include the following module and sub-modules
 *           (1) Flow Classification
 *           (2) Ingress ACL
 *           (3) Egress ACL
 *           (4) Egress VID Translation
 *           (5) Range Check
 *           (6) Field Selector
 *           (7) Pattern Match
 *
 */

/*
 * Include Files
 */
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <dal/dal_mgmt.h>
#include <rtk/default.h>
#include <rtk/pie.h>

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

/* Module Name     : PIE */

/* Function Name:
 *      rtk_pie_init
 * Description:
 *      Initialize PIE module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Applicable:
 *      8328
 * Note:
 *      Must initialize PIE module before calling any PIE APIs.
 */
int32
rtk_pie_init(uint32 unit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_init(unit);
} /* end of rtk_pie_init */

/* Sub-module Name :
 *  Flow classification
 *  Ingress ACL
 *  Egress ACL
 *  Egress VID Translation
 */

/* Function Name:
 *      rtk_pie_pieRuleEntryFieldSize_get
 * Description:
 *      Get the field size of PIE entry.
 * Input:
 *      unit        - unit id
 *      type        - type of entry field
 * Output:
 *      pField_size - field size of PIE entry.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_PIE_FIELD_TYPE - invalid entry field type
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The unit of field size is bit.
 */
int32
rtk_pie_pieRuleEntryFieldSize_get(uint32 unit, rtk_pie_fieldType_t type, uint32 *pField_size)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieRuleEntryFieldSize_get(unit, type, pField_size);
} /* end of rtk_pie_pieRuleEntryFieldSize_get */

/* Function Name:
 *      rtk_pie_pieRuleFieldId_get
 * Description:
 *      Get the field ID of field type.
 * Input:
 *      unit        - unit id
 *      type        - type of entry field
 * Output:
 *      pField_size - field size of PIE entry.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_PIE_FIELD_TYPE - invalid entry field type
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The ASIC ID of specified field type.
 */
int32
rtk_pie_pieRuleFieldId_get(uint32 unit, rtk_pie_fieldType_t type, uint32 *pField_id)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieRuleFieldId_get(unit, type, pField_id);
} /* end of rtk_pie_pieRuleFieldId_get */


/* Function Name:
 *      rtk_pie_pieRuleEntrySize_get
 * Description:
 *      Get the rule entry size of PIE.
 * Input:
 *      unit        - unit id
 * Output:
 *      pEntry_size - rule entry size of PIE
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The unit of entry size is bit.
 */
int32
rtk_pie_pieRuleEntrySize_get(uint32 unit, uint32 *pEntry_size)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieRuleEntrySize_get(unit, pEntry_size);
} /* end of  rtk_pie_pieRuleEntrySize_get*/

/* Function Name:
 *      rtk_pie_pieRuleEntryField_get
 * Description:
 *      Get the field data from specified PIE entry buffer.
 * Input:
 *      unit          - unit id
 *      phase         - PIE lookup phase
 *      entry_idx     - PIE entry index
 *      pEntry_buffer - data buffer of PIE entry
 *      type          - field type
 * Output:
 *      pData         - field data
 *      pMask         - field mask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_PIE_PHASE      - invalid PIE phase
 *      RT_ERR_ENTRY_INDEX    - invalid entry index
 *      RT_ERR_PIE_FIELD_TYPE - invalid entry field type
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The entry_idx is for logical view from 0 to (physical block number * physical block size - 1),
 *          driver will transfer it to physical entry index.
 */
int32
rtk_pie_pieRuleEntryField_get(
    uint32              unit,
    rtk_pie_phase_t     phase,
    rtk_pie_id_t        entry_idx,
    uint8               *pEntry_buffer,
    rtk_pie_fieldType_t type,
    uint8               *pData,
    uint8               *pMask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieRuleEntryField_get(unit, phase, entry_idx, pEntry_buffer, type, pData, pMask);
} /* end of rtk_pie_pieRuleEntrySize_get */

/* Function Name:
 *      rtk_pie_pieRuleEntryField_set
 * Description:
 *      Set the field data to specified PIE entry buffer.
 * Input:
 *      unit          - unit id
 *      phase         - PIE lookup phase
 *      entry_idx     - PIE entry index
 *      pEntry_buffer - data buffer of PIE entry
 *      type          - field type
 *      pData         - field data
 *      pMask         - field mask
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_PIE_PHASE      - invalid PIE phase
 *      RT_ERR_ENTRY_INDEX    - invalid entry index
 *      RT_ERR_PIE_FIELD_TYPE - invalid entry field type
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The entry_idx is for logical view from 0 to (physical block number * physical block size - 1),
 *          driver will transfer it to physical entry index.
 */
int32
rtk_pie_pieRuleEntryField_set(
    uint32              unit,
    rtk_pie_phase_t     phase,
    rtk_pie_id_t        entry_idx,
    uint8               *pEntry_buffer,
    rtk_pie_fieldType_t type,
    uint8               *pData,
    uint8               *pMask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieRuleEntryField_set(unit, phase, entry_idx, pEntry_buffer, type, pData, pMask);
} /* end of rtk_pie_pieRuleEntryField_set */

/* Function Name:
 *      rtk_pie_pieRuleEntryField_read
 * Description:
 *      Read the field data from specified PIE entry.
 * Input:
 *      unit      - unit id
 *      phase     - PIE lookup phase
 *      entry_idx - PIE entry index
 *      type      - field type
 * Output:
 *      pData     - field data
 *      pMask     - field mask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_PIE_PHASE      - invalid PIE phase
 *      RT_ERR_ENTRY_INDEX    - invalid entry index
 *      RT_ERR_PIE_FIELD_TYPE - invalid entry field type
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The entry_idx is for logical view from 0 to (physical block number * physical block size - 1),
 *          driver will transfer it to physical entry index.
 */
int32
rtk_pie_pieRuleEntryField_read(
    uint32              unit,
    rtk_pie_phase_t     phase,
    rtk_pie_id_t        entry_idx,
    rtk_pie_fieldType_t type,
    uint8               *pData,
    uint8               *pMask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieRuleEntryField_read(unit, phase, entry_idx, type, pData, pMask);
} /* end of rtk_pie_pieRuleEntryField_read */

/* Function Name:
 *      rtk_pie_pieRuleEntryField_write
 * Description:
 *      Write the field data to specified PIE entry.
 * Input:
 *      unit      - unit id
 *      phase     - PIE lookup phase
 *      entry_idx - PIE entry index
 *      type      - field type
 *      pData     - field data
 *      pMask     - field mask
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_PIE_PHASE      - invalid PIE phase
 *      RT_ERR_ENTRY_INDEX    - invalid entry index
 *      RT_ERR_PIE_FIELD_TYPE - invalid entry field type
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The entry_idx is for logical view from 0 to (physical block number * physical block size - 1),
 *          driver will transfer it to physical entry index.
 */
int32
rtk_pie_pieRuleEntryField_write(
    uint32              unit,
    rtk_pie_phase_t     phase,
    rtk_pie_id_t        entry_idx,
    rtk_pie_fieldType_t type,
    uint8               *pData,
    uint8               *pMask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieRuleEntryField_write(unit, phase, entry_idx, type, pData, pMask);
} /* end of rtk_pie_pieRuleEntryField_write */

/* Function Name:
 *      rtk_pie_piePreDefinedRuleEntry_get
 * Description:
 *      Get the pre-defined entry data from buffer.
 * Input:
 *      unit              - unit id
 *      pEntry_buffer     - data buffer of PIE entry 
 * Output:
 *      pPredefined_entry - pre-defined entry data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None.
 */
int32
rtk_pie_piePreDefinedRuleEntry_get(
    uint32                          unit,
    uint8                           *pEntry_buffer,
    rtk_pie_preDefinedRuleEntry_t   *pPredefined_entry)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_piePreDefinedRuleEntry_get(unit, pEntry_buffer, pPredefined_entry);
} /* end of rtk_pie_piePreDefinedRuleEntry_get */

/* Function Name:
 *      rtk_pie_piePreDefinedRuleEntry_set
 * Description:
 *      Set the pre-defined entry data to buffer.
 * Input:
 *      unit              - unit id
 *      pEntry_buffer     - data buffer of PIE entry  
 *      pPredefined_entry - pre-defined entry data
 * Output:
 *      None.  
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None.
 */
int32
rtk_pie_piePreDefinedRuleEntry_set(
    uint32                          unit,
    uint8                           *pEntry_buffer,
    rtk_pie_preDefinedRuleEntry_t   *pPredefined_entry)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_piePreDefinedRuleEntry_set(unit, pEntry_buffer, pPredefined_entry);
} /* end of rtk_pie_piePreDefinedRuleEntry_set */

/* Function Name:
 *      rtk_pie_pieRuleEntry_read
 * Description:
 *      Read the entry data from specified PIE entry.
 * Input:
 *      unit          - unit id
 *      entry_idx     - PIE entry index
 * Output:
 *      pEntry_buffer - data buffer of PIE entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The entry_idx is for logical view from 0 to (physical block number * physical block size - 1),
 *          driver will transfer it to physical entry index.
 */
int32
rtk_pie_pieRuleEntry_read(
    uint32              unit,
    rtk_pie_id_t        entry_idx,
    uint8               *pEntry_buffer)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieRuleEntry_read(unit, entry_idx, pEntry_buffer);
} /* end of rtk_pie_pieRuleEntry_read */

/* Function Name:
 *      rtk_pie_pieRuleEntry_write
 * Description:
 *      Write the entry data to specified PIE entry.
 * Input:
 *      unit          - unit id
 *      entry_idx     - PIE entry index
 *      pEntry_buffer - data buffer of PIE entry
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The entry_idx is for logical view from 0 to (physical block number * physical block size - 1),
 *          driver will transfer it to physical entry index.
 */
int32
rtk_pie_pieRuleEntry_write(
    uint32              unit,
    rtk_pie_id_t        entry_idx,
    uint8               *pEntry_buffer)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieRuleEntry_write(unit, entry_idx, pEntry_buffer);
} /* end of rtk_pie_pieRuleEntry_write */

/* Function Name:
 *      rtk_pie_pieRuleEntry_del
 * Description:
 *      Delete the specified PIE entry.
 * Input:
 *      unit     - unit id
 *      pContent - PIE entry information.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) Clear the content of specified PIE entry to be 0.
 *
 *      (2) If the content.end_idx is smaller than content.start then clear from content.start to the maximum index of entry
 *          and then return to index 0 to continue to content.end.
 */
int32
rtk_pie_pieRuleEntry_del(uint32 unit, rtk_pie_clearBlockContent_t *pContent)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieRuleEntry_del(unit, pContent);
} /* end of rtk_pie_pieRuleEntry_del */

/* Function Name:
 *      rtk_pie_pieRuleEntry_move
 * Description:
 *      Move the specified PIE entry.
 * Input:
 *      unit     - unit id
 *      pContent - setting for move PIE entry.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      If the moving index of entry is greater than the maximum or smaller than the minimum index of entry,
 *      then ASIC stop action.
 */
int32
rtk_pie_pieRuleEntry_move(uint32 unit, rtk_pie_movePieContent_t *pContent)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieRuleEntry_move(unit, pContent);
} /* end of rtk_pie_pieRuleEntry_move */

/* Function Name:
 *      rtk_pie_pieRuleEntry_swap
 * Description:
 *      Swap the specified PIE entry.
 * Input:
 *      unit     - unit id
 *      pContent - setting for swap PIE entry.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_pie_pieRuleEntry_swap(uint32 unit, rtk_pie_movePieContent_t *pContent)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieRuleEntry_swap(unit, pContent);
} /* end of rtk_pie_pieRuleEntry_swap */

/* Function Name:
 *      rtk_pie_pieRuleEntryAction_del
 * Description:
 *      Delete the specified PIE entry and action.
 * Input:
 *      unit     - unit id
 *      pContent - PIE entry information.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) Clear the content of specified PIE entry and action to be 0.
 *      (2) If the content.end_idx is smaller than content.start then clear from content.start to the maximum index of entry and action
 *          and then return to index 0 to continue to content.end.
 */
int32
rtk_pie_pieRuleEntryAction_del(uint32 unit, rtk_pie_clearBlockContent_t *pContent)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieRuleEntryAction_del(unit, pContent);
} /* end of rtk_pie_pieRuleEntryAction_del */

/* Function Name:
 *      rtk_pie_pieRuleEntryAction_move
 * Description:
 *      Move the specified PIE entry and action.
 * Input:
 *      unit     - unit id
 *      pContent - setting for move PIE entry and action.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      If the moving index of entry is greater than the maximum or smaller than the minimum index of entry and action,
 *      then ASIC stop action.
 */
int32
rtk_pie_pieRuleEntryAction_move(uint32 unit, rtk_pie_movePieContent_t *pContent)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieRuleEntryAction_move(unit, pContent);
} /* end of rtk_pie_pieRuleEntryAction_move */

/* Function Name:
 *      rtk_pie_pieRuleEntryAction_swap
 * Description:
 *      Swap the specified PIE entry and action.
 * Input:
 *      unit     - unit id
 *      pContent - setting for swap PIE entry and action.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_pie_pieRuleEntryAction_swap(uint32 unit, rtk_pie_movePieContent_t *pContent)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieRuleEntryAction_swap(unit, pContent);
} /* end of rtk_pie_pieRuleEntryAction_swap */

/* Function Name:
 *      rtk_pie_pieRuleAction_get
 * Description:
 *      Get the PIE action configuration from specified device.
 * Input:
 *      unit       - unit id
 *      action_idx - PIE action index
 * Output:
 *      pAction    - PIE action configuration.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of action_idx is 0~511 in 8328S/8328L; 0~2047 in 8328M.
 */
int32
rtk_pie_pieRuleAction_get(
    uint32                   unit,
    rtk_pie_id_t             action_idx,
    rtk_pie_actionTable_t    *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieRuleAction_get(unit, action_idx, pAction);
} /* end of rtk_pie_pieRuleAction_get */

/* Function Name:
 *      rtk_pie_pieRuleAction_set
 * Description:
 *      Set the PIE action configuration to specified device.
 * Input:
 *      unit       - unit id
 *      action_idx - PIE action index
 *      pAction    - PIE action configuration.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of action_idx is 0~511 in 8328S/8328L; 0~2047 in 8328M.
 */
int32
rtk_pie_pieRuleAction_set(
    uint32                   unit,
    rtk_pie_id_t             action_idx,
    rtk_pie_actionTable_t    *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieRuleAction_set(unit, action_idx, pAction);
} /* end of rtk_pie_pieRuleAction_set */

/* Function Name:
 *      rtk_pie_pieRuleAction_del
 * Description:
 *      Delete the specified PIE action.
 * Input:
 *      unit     - unit id
 *      pContent - PIE action information.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) Clear the content of specified PIE action to be 0.
 *
 *      (2) If the content.end_idx is smaller than content.start then clear from content.start to the maximum index of entry
 *         and then return to index 0 to continue to content.end.
 */
int32
rtk_pie_pieRuleAction_del(uint32 unit, rtk_pie_clearBlockContent_t *pContent)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieRuleAction_del(unit, pContent);
} /* end of rtk_pie_pieRuleAction_del */

/* Function Name:
 *      rtk_pie_pieRuleAction_move
 * Description:
 *      Move the specified PIE action.
 * Input:
 *      unit     - unit id
 *      pContent - setting for move PIE action.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      If the moving index of entry is greater than the maximum or smaller than the minimum index of entry,
 *      then ASIC stop action.
 */
int32
rtk_pie_pieRuleAction_move(uint32 unit, rtk_pie_movePieContent_t *pContent)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieRuleAction_move(unit, pContent);
} /* end of rtk_pie_pieRuleAction_move */

/* Function Name:
 *      rtk_pie_pieRuleAction_swap
 * Description:
 *      Swap the specified PIE action.
 * Input:
 *      unit     - unit id
 *      pContent - setting for swap PIE action.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_pie_pieRuleAction_swap(uint32 unit, rtk_pie_movePieContent_t *pContent)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieRuleAction_swap(unit, pContent);
} /* end of rtk_pie_pieRuleAction_swap */

/* Function Name:
 *      rtk_pie_pieRulePolicer_get
 * Description:
 *      Get the PIE policer configuration from specified device.
 * Input:
 *      unit        - unit id
 *      policer_idx - PIE policer index
 * Output:
 *      pPolicer    - PIE policer configuration.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of policer_idx is 0~255.
 */
int32
rtk_pie_pieRulePolicer_get(
    uint32                   unit,
    rtk_pie_id_t             policer_idx,
    rtk_pie_policerEntry_t   *pPolicer)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieRulePolicer_get(unit, policer_idx, pPolicer);
} /* end of rtk_pie_pieRulePolicer_get */

/* Function Name:
 *      rtk_pie_pieRulePolicer_set
 * Description:
 *      Set the PIE policer configuration to specified device.
 * Input:
 *      unit        - unit id
 *      policer_idx - PIE policer index
 *      pPolicer    - PIE policer configuration.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of policer_idx is 0~255.
 */
int32
rtk_pie_pieRulePolicer_set(
    uint32                   unit,
    rtk_pie_id_t             policer_idx,
    rtk_pie_policerEntry_t   *pPolicer)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieRulePolicer_set(unit, policer_idx, pPolicer);
} /* end of rtk_pie_pieRulePolicer_set */

/* Function Name:
 *      rtk_pie_pieHitIndication_get
 * Description:
 *      Get the hit indication of PIE rule from specified device.
 * Input:
 *      unit       - unit id
 *      lblock_idx - logical block index
 * Output:
 *      pStatus    - hit indication status for 128 rules on a logical block.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of lblock_idx is 0~3 in 8328S/8328L; 0~15 in 8328M
 *      (2) Read and clear indication bits (128 bits/one time)
 */
int32
rtk_pie_pieHitIndication_get(
    uint32                       unit,
    rtk_pie_id_t                 lblock_idx,
    rtk_pie_hitIndicationEntry_t *pStatus)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieHitIndication_get(unit, lblock_idx, pStatus);
} /* end of rtk_pie_pieHitIndication_get */

/* Function Name:
 *      rtk_pie_pieStat_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of log_id is 0~511 in 8328S/8328L; 0~1023 in 8328M
 */
int32
rtk_pie_pieStat_get(uint32 unit, rtk_pie_id_t log_id, uint32 *pPkt_cnt, uint64 *pByte_cnt)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieStat_get(unit, log_id, pPkt_cnt, pByte_cnt);
} /* end of rtk_pie_pieStat_get */

/* Function Name:
 *      rtk_pie_pieStat_set
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
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_ENTRY_INDEX - invalid entry index
 *      RT_ERR_INPUT       - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of log_id is 0~511 in 8328S/8328L; 0~1023 in 8328M
 */
int32
rtk_pie_pieStat_set(uint32 unit, rtk_pie_id_t log_id, uint32 pkt_cnt, uint64 byte_cnt)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieStat_set(unit, log_id, pkt_cnt, byte_cnt);
} /* end of rtk_pie_pieStat_set */

/* Function Name:
 *      rtk_pie_pieStat_clearAll
 * Description:
 *      Clear all statistic counter for the specified device.
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
 *      8328
 * Note:
 *      None
 */
int32
rtk_pie_pieStat_clearAll(uint32 unit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieStat_clearAll(unit);
} /* end of rtk_pie_pieStat_clearAll */

/* Function Name:
 *      rtk_pie_pieTemplateSelector_get
 * Description:
 *      Get the template index of specific physical block and lookup phase.
 * Input:
 *      unit          - unit id
 *      pblock_idx    - physical block index
 *      phase         - lookup phase
 * Output:
 *      pTemplate_idx - template index
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_PIE_PHASE    - invalid PIE phase
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of pblock_idx is 0~31
 *      (2) The valid range of phase is 0~3
 */
int32
rtk_pie_pieTemplateSelector_get(
    uint32                  unit,
    rtk_pie_id_t            pblock_idx,
    rtk_pie_phase_t         phase,
    rtk_pie_id_t            *pTemplate_idx)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieTemplateSelector_get(unit, pblock_idx, phase, pTemplate_idx);
} /* end of rtk_pie_pieTemplateSelector_get */

/* Function Name:
 *      rtk_pie_pieTemplateSelector_set
 * Description:
 *      Set the template index of specific physical block and lookup phase.
 * Input:
 *      unit         - unit id
 *      pblock_idx   - physical block index
 *      phase        - lookup phase
 *      template_idx - template index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_ENTRY_INDEX - invalid entry index
 *      RT_ERR_PIE_PHASE   - invalid PIE phase
 *      RT_ERR_INPUT       - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of pblock_idx is 0~31
 *      (2) The valid range of phase is 0~3
 *      (3) The valid range of template_idx is 0~15
 */
int32
rtk_pie_pieTemplateSelector_set(
    uint32                  unit,
    rtk_pie_id_t            pblock_idx,
    rtk_pie_phase_t         phase,
    rtk_pie_id_t            template_idx)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieTemplateSelector_set(unit, pblock_idx, phase, template_idx);
} /* end of rtk_pie_pieTemplateSelector_set */

/* template_idx 1-15 (0 for pre-defined), field_idx 0-8 */
/* some field type must be in specific field index, driver need to check it */

/* Function Name:
 *      rtk_pie_pieUserTemplate_get
 * Description:
 *      Get the template content of specific template index.
 * Input:
 *      unit         - unit id
 *      template_idx - template index
 * Output:
 *      pTemplate    - template content
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of template_idx is 0~15
 */
int32
rtk_pie_pieUserTemplate_get(uint32 unit, rtk_pie_id_t template_idx, rtk_pie_template_t *pTemplate)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieUserTemplate_get(unit, template_idx, pTemplate);
} /* end of rtk_pie_pieUserTemplate_get */

/* Function Name:
 *      rtk_pie_pieUserTemplate_set
 * Description:
 *      Set the template content of specific template index.
 * Input:
 *      unit         - unit id
 *      template_idx - template index
 *      pTemplate    - template content
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of template_idx is 0~15
 */
int32
rtk_pie_pieUserTemplate_set(uint32 unit, rtk_pie_id_t template_idx, rtk_pie_template_t *pTemplate)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieUserTemplate_set(unit, template_idx, pTemplate);
} /* end of rtk_pie_pieUserTemplate_set */

/* Function Name:
 *      rtk_pie_pieL34ChecksumErr_get
 * Description:
 *      Get the operation for L3 and L4 packets checksum error.
 * Input:
 *      unit       - unit id
 * Output:
 *      pOperation - operation for L3 and L4 packets checksum error
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The operation value as following:
 *      - CHECKSUM_ERR_DOWNGRADE
 *      - CHECKSUM_ERR_PARSE_ANYWAY
 */
int32
rtk_pie_pieL34ChecksumErr_get(uint32 unit, rtk_pie_l34ChecksumErrOper_t *pOperation)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieL34ChecksumErr_get(unit, pOperation);
} /* end of rtk_pie_pieL34ChecksumErr_get */

/* Function Name:
 *      rtk_pie_pieL34ChecksumErr_set
 * Description:
 *      Set the operation for L3 and L4 packets checksum error.
 * Input:
 *      unit      - unit id
 *      operation - operation for L3 and L4 packets checksum error
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      The operation value as following:
 *      - CHECKSUM_ERR_DOWNGRADE
 *      - CHECKSUM_ERR_PARSE_ANYWAY
 */
int32
rtk_pie_pieL34ChecksumErr_set(uint32 unit, rtk_pie_l34ChecksumErrOper_t operation)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieL34ChecksumErr_set(unit, operation);
} /* end of rtk_pie_pieL34ChecksumErr_set */

/* Function Name:
 *      rtk_pie_pieUserTemplatePayloadOffset_get
 * Description:
 *      Get the payload offset for template field that bound in secific physical block.
 * Input:
 *      unit       - unit id
 *      pblock_idx - physical block index
 *      offset_idx - offset index
 * Output:
 *      pOffset    - payload offset
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of pblock_idx is 0~31
 *      (2) The valid range of offset_idx is 0~1
 */
int32
rtk_pie_pieUserTemplatePayloadOffset_get(
    uint32        unit,
    rtk_pie_id_t  pblock_idx,
    rtk_pie_id_t  offset_idx,
    uint32        *pOffset)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieUserTemplatePayloadOffset_get(unit, pblock_idx, offset_idx, pOffset);
} /* end of rtk_pie_pieUserTemplatePayloadOffset_get */

/* Function Name:
 *      rtk_pie_pieUserTemplatePayloadOffset_set
 * Description:
 *      Set the payload offset for template field that bound in secific physical block.
 * Input:
 *      unit       - unit id
 *      pblock_idx - physical block index
 *      offset_idx - offset index
 *      offset     - payload offset
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of pblock_idx is 0~31
 *      (2) The valid range of offset_idx is 0~1
 */
int32
rtk_pie_pieUserTemplatePayloadOffset_set(
    uint32        unit,
    rtk_pie_id_t  pblock_idx,
    rtk_pie_id_t  offset_idx,
    uint32        offset)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieUserTemplatePayloadOffset_set(unit, pblock_idx, offset_idx, offset);
} /* end of rtk_pie_pieUserTemplatePayloadOffset_set */

/* Function Name:
 *      rtk_pie_pieResultReverse_get
 * Description:
 *      Get the operation of reverse for lookup result.
 * Input:
 *      unit       - unit id
 *      entry_idx  - entry index
 * Output:
 *      pOperation - result reverse
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The operation value as following:
 *      - RESULT_KEEP
 *      - RESULT_REVERSE
 */
int32
rtk_pie_pieResultReverse_get(
    uint32                       unit,
    rtk_pie_id_t                 entry_idx,
    rtk_pie_resultReverseOper_t  *pOperation)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieResultReverse_get(unit, entry_idx, pOperation);
} /* end of rtk_pie_pieResultReverse_get */

/* Function Name:
 *      rtk_pie_pieResultReverse_set
 * Description:
 *      Set the operation of reverse for lookup result.
 * Input:
 *      unit       - unit id
 *      entry_idx  - entry index
 *      operation  - result reverse
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_UNIT_ID    - invalid unit id
 *      RT_ERR_ENTRY_INDE - invalid entry index
 *      RT_ERR_INPUT      - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      The operation value as following:
 *      - RESULT_KEEP
 *      - RESULT_REVERSE
 */
int32
rtk_pie_pieResultReverse_set(
    uint32                       unit,
    rtk_pie_id_t                 entry_idx,
    rtk_pie_resultReverseOper_t  operation)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieResultReverse_set(unit, entry_idx, operation);
} /* end of rtk_pie_pieResultReverse_set */

/* Function Name:
 *      rtk_pie_pieResultAggregator_get
 * Description:
 *      Get the type of lookup result aggregation.
 * Input:
 *      unit            - unit id
 *      pblockRange_idx - index of physical block range
 *      entry_idx       - physical block entry index
 * Output:
 *      pType           - type of result aggregator
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The range index: 0 for physical block 0~3, 1 for physical block 4~7, ..., 7 for physical block 28~31.
 */
int32
rtk_pie_pieResultAggregator_get(
    uint32                           unit,
    rtk_pie_resultAggregatorRange_t  pblockRange_idx,
    rtk_pie_id_t                     entry_idx,
    rtk_pie_resultAggregatorType_t   *pType)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieResultAggregator_get(unit, pblockRange_idx, entry_idx, pType);
} /* end of rtk_pie_pieResultAggregator_get */

/* Function Name:
 *      rtk_pie_pieResultAggregator_set
 * Description:
 *      Set the type of lookup result aggregation.
 * Input:
 *      unit            - unit id
 *      pblockRange_idx - index of physical block range
 *      entry_idx       - physical block entry index
 *      type            - type of result aggregator
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      The range index: 0 for physical block 0~3, 1 for physical block 4~7, ..., 7 for physical block 28~31.
 */
int32
rtk_pie_pieResultAggregator_set(
    uint32                           unit,
    rtk_pie_resultAggregatorRange_t  pblockRange_idx,
    rtk_pie_id_t                     entry_idx,
    rtk_pie_resultAggregatorType_t   type)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieResultAggregator_set(unit, pblockRange_idx, entry_idx, type);
} /* end of rtk_pie_pieResultAggregator_set */

/* Function Name:
 *      rtk_pie_pieBlockPriority_get
 * Description:
 *      Get the priority of PIE block.
 * Input:
 *      unit       - unit id
 *      lblock_idx - logical block index
 * Output:
 *      pPriority  - block priority
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of lblock_idx is 0~3 in 8328S/8328L; 0~15 in 8328M
 */
int32
rtk_pie_pieBlockPriority_get(uint32 unit, rtk_pie_id_t lblock_idx, uint32 *pPriority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieBlockPriority_get(unit, lblock_idx, pPriority);
} /* end of rtk_pie_pieBlockPriority_get */

/* Function Name:
 *      rtk_pie_pieBlockPriority_set
 * Description:
 *      Set the priority of PIE block.
 * Input:
 *      unit       - unit id
 *      lblock_idx - logical block index
 *      priority   - block priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_ENTRY_INDEX - invalid entry index
 *      RT_ERR_INPUT       - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of lblock_idx is 0~3 in 8328S/8328L; 0~15 in 8328M
 */
int32
rtk_pie_pieBlockPriority_set(uint32 unit, rtk_pie_id_t lblock_idx, uint32 priority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieBlockPriority_set(unit, lblock_idx, priority);
} /* end of rtk_pie_pieBlockPriority_set */

/* Function Name:
 *      rtk_pie_pieGroupCtrl_get
 * Description:
 *      Get the group operation of logical block.
 * Input:
 *      unit            - unit id
 *      lblockRange_idx - index of logical block range
 * Output:
 *      pOperation      - group operation
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The range index: 0 for logical block index 0~3, 1 for logical block index 4~7, ..., 3 for logical block index 12~15
 */
int32
rtk_pie_pieGroupCtrl_get(
    uint32                      unit,
    rtk_pie_groupCtrlRange_t    lblockRange_idx,
    rtk_pie_groupCtrl_t         *pOperation)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieGroupCtrl_get(unit, lblockRange_idx, pOperation);
} /* end of rtk_pie_pieGroupCtrl_get */

/* Function Name:
 *      rtk_pie_pieGroupCtrl_set
 * Description:
 *      Set the group operation of logical block.
 * Input:
 *      unit            - unit id
 *      lblockRange_idx - index of logical block range
 *      operation       - group operation
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_ENTRY_INDEX - invalid entry index
 *      RT_ERR_INPUT       - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      The range index: 0 for logical block index 0~3, 1 for logical block index 4~7, ..., 3 for logical block index 12~15
 */
int32
rtk_pie_pieGroupCtrl_set(
    uint32                      unit,
    rtk_pie_groupCtrlRange_t    lblockRange_idx,
    rtk_pie_groupCtrl_t         operation)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieGroupCtrl_set(unit, lblockRange_idx, operation);
} /* end of rtk_pie_pieGroupCtrl_set */

/* Function Name:
 *      rtk_pie_pieEgrAclLookupCtrl_get
 * Description:
 *      Get the control configuration of egress ACL lookup.
 * Input:
 *      unit     - unit id
 * Output:
 *      pControl - control configuration
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_pie_pieEgrAclLookupCtrl_get(uint32 unit, rtk_pie_egrAclLookupCtrl_t *pControl)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieEgrAclLookupCtrl_get(unit, pControl);
} /* end of rtk_pie_pieEgrAclLookupCtrl_get */

/* Function Name:
 *      rtk_pie_pieEgrAclLookupCtrl_set
 * Description:
 *      Set the control configuration of egress ACL lookup.
 * Input:
 *      unit     - unit id
 *      pControl - control configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_pie_pieEgrAclLookupCtrl_set(uint32 unit, rtk_pie_egrAclLookupCtrl_t *pControl)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieEgrAclLookupCtrl_set(unit, pControl);
} /* end of rtk_pie_pieEgrAclLookupCtrl_set */

/* Function Name:
 *      rtk_pie_piePortLookupPhaseEnable_get
 * Description:
 *      Get the enable status of specific lookup phase on a port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      phase   - lookup_phase
 * Output:
 *      pEnable - enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PIE_PHASE    - invalid PIE phase
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of phase is 0~3
 */
int32
rtk_pie_piePortLookupPhaseEnable_get(
    uint32           unit,
    rtk_port_t       port,
    rtk_pie_phase_t  phase,
    rtk_enable_t     *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_piePortLookupPhaseEnable_get(unit, port, phase, pEnable);
} /* end of rtk_pie_piePortLookupPhaseEnable_get */

/* Function Name:
 *      rtk_pie_piePortLookupPhaseEnable_set
 * Description:
 *      Set the enable status of specific lookup phase on a port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      phase  - lookup_phase
 *      enable - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT  - The module is not initial
 *      RT_ERR_UNIT_ID   - invalid unit id
 *      RT_ERR_PORT_ID   - invalid port id
 *      RT_ERR_PIE_PHASE - invalid PIE phase 
 *      RT_ERR_INPUT     - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of phase is 0~3
 */
int32
rtk_pie_piePortLookupPhaseEnable_set(
    uint32           unit,
    rtk_port_t       port,
    rtk_pie_phase_t  phase,
    rtk_enable_t     enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_piePortLookupPhaseEnable_set(unit, port, phase, enable);
} /* end of rtk_pie_piePortLookupPhaseEnable_set */

/* Function Name:
 *      rtk_pie_piePortLookupPhaseMiss_get
 * Description:
 *      Get the lookup miss action of specific lookup phase on a port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      phase   - lookup_phase
 * Output:
 *      pAction - lookup miss action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PIE_PHASE    - invalid PIE phase
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of phase is 0~3
 *      (2) The action value as following:
 *          - LOOKUP_MISS_PERMIT
 *          - LOOKUP_MISS_DROP
 */
int32
rtk_pie_piePortLookupPhaseMiss_get(
    uint32                       unit,
    rtk_port_t                   port,
    rtk_pie_phase_t              phase,
    rtk_pie_lookupMissAction_t   *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_piePortLookupPhaseMiss_get(unit, port, phase, pAction);
} /* end of rtk_pie_piePortLookupPhaseMiss_get */

/* Function Name:
 *      rtk_pie_piePortLookupPhaseMiss_set
 * Description:
 *      Set the lookup miss action of specific lookup phase on a port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      phase  - lookup_phase
 *      action - lookup miss action
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT  - The module is not initial
 *      RT_ERR_UNIT_ID   - invalid unit id
 *      RT_ERR_PORT_ID   - invalid port id
 *      RT_ERR_PIE_PHASE - invalid PIE phase
 *      RT_ERR_INPUT     - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of phase is 0~3
 *      (2) The action value as following:
 *          - LOOKUP_MISS_PERMIT
 *          - LOOKUP_MISS_DROP
 */
int32
rtk_pie_piePortLookupPhaseMiss_set(
    uint32                       unit,
    rtk_port_t                   port,
    rtk_pie_phase_t              phase,
    rtk_pie_lookupMissAction_t   action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_piePortLookupPhaseMiss_set(unit, port, phase, action);
} /* end of rtk_pie_piePortLookupPhaseMiss_set */

/* Function Name:
 *      rtk_pie_pieCounterIndicationMode_get
 * Description:
 *      Get the mode of ACL counter and hit indication.
 * Input:
 *      unit       - unit id
 *      lblock_idx - logical block index
 * Output:
 *      pMode      - counter and hit indication mode.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of lblock_idx is 0~3 in 8328S/8328L; 0~15 in 8328M
 *      (2) The mode value as following:
 *          - ACTION_EXECUTION
 *          - RULE_MATCH
 */
int32
rtk_pie_pieCounterIndicationMode_get(
    uint32                           unit,
    rtk_pie_id_t                     lblock_idx,
    rtk_pie_counterIndicationMode_t  *pMode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieCounterIndicationMode_get(unit, lblock_idx, pMode);
} /* end of rtk_pie_pieCounterIndicationMode_get */

/* Function Name:
 *      rtk_pie_pieCounterIndicationMode_set
 * Description:
 *      Set the mode of ACL counter and hit indication.
 * Input:
 *      unit       - unit id
 *      lblock_idx - logical block index
 *      mode       - counter and hit indication mode.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_ENTRY_INDEX - invalid entry index
 *      RT_ERR_INPUT       - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of lblock_idx is 0~3 in 8328S/8328L; 0~15 in 8328M
 *      (2) The mode value as following:
 *          - ACTION_EXECUTION
 *          - RULE_MATCH
 */
int32
rtk_pie_pieCounterIndicationMode_set(
    uint32                           unit,
    rtk_pie_id_t                     lblock_idx,
    rtk_pie_counterIndicationMode_t  mode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_pieCounterIndicationMode_set(unit, lblock_idx, mode);
} /* end of rtk_pie_pieCounterIndicationMode_set */

/* Function Name:
 *      rtk_pie_piePolicerCtrl_get
 * Description:
 *      Get the control configuration of policer.
 * Input:
 *      unit     - unit id
 * Output:
 *      pControl - control configuration
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_pie_piePolicerCtrl_get(uint32 unit, rtk_pie_policerCtrl_t *pControl)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_piePolicerCtrl_get(unit, pControl);
} /* end of rtk_pie_piePolicerCtrl_get */

/* Function Name:
 *      rtk_pie_piePolicerCtrl_set
 * Description:
 *      Set the control configuration of policer.
 * Input:
 *      unit     - unit id
 *      pControl - control configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      Configure the preamble IFG or not?
 */
int32
rtk_pie_piePolicerCtrl_set(uint32 unit, rtk_pie_policerCtrl_t *pControl)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_piePolicerCtrl_set(unit, pControl);
} /* end of rtk_pie_piePolicerCtrl_set */

/* Sub-module Name : 
 *  Range Check
 */

/* Function Name:
 *      rtk_pie_rangeCheckL4Port_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of index is 0~15 in 8328.
 */
int32
rtk_pie_rangeCheckL4Port_get(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_l4Port_t *pData)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_rangeCheckL4Port_get(unit, index, pData);
} /* end of rtk_pie_rangeCheckL4Port_get */

/* Function Name:
 *      rtk_pie_rangeCheckL4Port_set
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of index is 0~15 in 8328.
 */
int32
rtk_pie_rangeCheckL4Port_set(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_l4Port_t *pData)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_rangeCheckL4Port_set(unit, index, pData);
} /* end of rtk_pie_rangeCheckL4Port_set */

/* Function Name:
 *      rtk_pie_rangeCheckVid_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of index is 0~31 in 8328.
 */
int32
rtk_pie_rangeCheckVid_get(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_vid_t *pData)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_rangeCheckVid_get(unit, index, pData);
} /* end of rtk_pie_rangeCheckVid_get */

/* Function Name:
 *      rtk_pie_rangeCheckVid_set
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of index is 0~31 in 8328.
 */
int32
rtk_pie_rangeCheckVid_set(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_vid_t *pData)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_rangeCheckVid_set(unit, index, pData);
} /* end of rtk_pie_rangeCheckVid_set */

/* Function Name:
 *      rtk_pie_rangeCheckIp_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of index is 0~15 in 8328.
 */
int32
rtk_pie_rangeCheckIp_get(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_ip_t *pData)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_rangeCheckIp_get(unit, index, pData);
} /* end of rtk_pie_rangeCheckIp_get */

/* Function Name:
 *      rtk_pie_rangeCheckIp_set
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of index is 0~15 in 8328.
 */
int32
rtk_pie_rangeCheckIp_set(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_ip_t *pData)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_rangeCheckIp_set(unit, index, pData);
} /* end of rtk_pie_rangeCheckIp_set */

/* Function Name:
 *      rtk_pie_rangeCheckSrcPort_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of index is 0~15 in 8328.
 */
int32
rtk_pie_rangeCheckSrcPort_get(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_srcPortMask_t *pData)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_rangeCheckSrcPort_get(unit, index, pData);
} /* end of rtk_pie_rangeCheckSrcPort_get */

/* Function Name:
 *      rtk_pie_rangeCheckSrcPort_set
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of index is 0~15 in 8328.
 */
int32
rtk_pie_rangeCheckSrcPort_set(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_srcPortMask_t *pData)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_rangeCheckSrcPort_set(unit, index, pData);
} /* end of rtk_pie_rangeCheckSrcPort_set */

/* Sub-module Name :
 *  Field Selector
 */

/* Function Name:
 *      rtk_pie_fieldSelectorEnable_get
 * Description:
 *      Get the enable status of field selector on specific port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      fs_idx  - field selector index
 * Output:
 *      pEnable - enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of fs_idx is 0~1 in 8328.
 */
int32
rtk_pie_fieldSelectorEnable_get(uint32 unit, rtk_port_t port, rtk_pie_id_t fs_idx, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_fieldSelectorEnable_get(unit, port, fs_idx, pEnable);
} /* end of rtk_pie_fieldSelectorEnable_get */

/* Function Name:
 *      rtk_pie_fieldSelectorEnable_set
 * Description:
 *      Set the enable status of field selector on specific port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      fs_idx - field selector index
 *      enable - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_PORT_ID     - invalid port id
 *      RT_ERR_ENTRY_INDEX - invalid entry index
 *      RT_ERR_INPUT       - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of fs_idx is 0~1 in 8328.
 */
int32
rtk_pie_fieldSelectorEnable_set(uint32 unit, rtk_port_t port, rtk_pie_id_t fs_idx, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_fieldSelectorEnable_set(unit, port, fs_idx, enable);
} /* end of rtk_pie_fieldSelectorEnable_set */

/* Function Name:
 *      rtk_pie_fieldSelectorContent_get
 * Description:
 *      Get the content of field selector on specific port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      fs_idx - field selector index
 * Output:
 *      pFs    - configuration of field selector.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of fs_idx is 0~1 in 8328.
 */
int32
rtk_pie_fieldSelectorContent_get(
    uint32                       unit,
    rtk_port_t                   port,
    rtk_pie_id_t                 fs_idx,
    rtk_pie_fieldSelector_data_t *pFs)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_fieldSelectorContent_get(unit, port, fs_idx, pFs);
} /* end of rtk_pie_fieldSelectorContent_get */

/* Function Name:
 *      rtk_pie_fieldSelectorContent_set
 * Description:
 *      Set the content of field selector on specific port.
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of fs_idx is 0~1 in 8328.
 */
int32
rtk_pie_fieldSelectorContent_set(
    uint32                       unit,
    rtk_port_t                   port,
    rtk_pie_id_t                 fs_idx,
    rtk_pie_fieldSelector_data_t *pFs)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_fieldSelectorContent_set(unit, port, fs_idx, pFs);
} /* end of rtk_pie_fieldSelectorContent_set */

/* Sub-module Name :
 *  Pattern Match
 */

/*Currently, Pattern Match Function only implement on giga port (port24-27)*/
/* Function Name:
 *      rtk_pie_patternMatchEnable_get
 * Description:
 *      Get the enable status of pattern match on specific port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_pie_patternMatchEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_patternMatchEnable_get(unit, port, pEnable);
} /* end of rtk_pie_patternMatchEnable_get */

/* Function Name:
 *      rtk_pie_patternMatchEnable_set
 * Description:
 *      Set the enable status of pattern match on specific port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_pie_patternMatchEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_patternMatchEnable_set(unit, port, enable);
} /* end of rtk_pie_patternMatchEnable_set */

/* Function Name:
 *      rtk_pie_patternMatchContent_get
 * Description:
 *      Get the content of pattern match on specific port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pContent - content of pattern match.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      If you want to check pattern "integer" need to:
 *      put 'r' into data[0], 'e' into data[1], 'g' into data[2] and so on
 *      turn on last_bit of data[0], turn on start_bit of data[6], turn on care_bit of data[0]~data[6].
 */
int32
rtk_pie_patternMatchContent_get(
    uint32                           unit,
    rtk_port_t                       port,
    rtk_pie_patternMatch_content_t   *pContent)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_patternMatchContent_get(unit, port, pContent);
} /* end of rtk_pie_patternMatchContent_get */

/* Function Name:
 *      rtk_pie_patternMatchContent_set
 * Description:
 *      Set the content of pattern match on specific port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      pContent - content of pattern match.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      If you want to check pattern "integer" need to:
 *      put 'r' into data[0], 'e' into data[1], 'g' into data[2] and so on
 *      turn on last_bit of data[0], turn on start_bit of data[6], turn on care_bit of data[0]~data[6].
 */
int32
rtk_pie_patternMatchContent_set(
    uint32                           unit,
    rtk_port_t                       port,
    rtk_pie_patternMatch_content_t   *pContent)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->pie_patternMatchContent_set(unit, port, pContent);
} /* end of rtk_pie_patternMatchContent_set */
