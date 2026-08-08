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
 * $Revision: 6569 $
 * $Date: 2009-10-26 11:57:06 +0800 (Mon, 26 Oct 2009) $
 *
 * Purpose : I/O read/write APIs in the SDK.
 *
 * Feature : I/O read/write APIs
 *
 */

/*
 * Include Files
 */
#include <common/error.h>
#include <ioal/ioal_init.h>
#include <ioal/mem32.h>

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

/* Function Name:
 *      ioal_mem32_check
 * Description:
 *      Check the register address is valid or not for the specified chip.
 * Input:
 *      unit - unit id
 *      addr - register address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - valid
 *      RT_ERR_FAILED - invalid
 * Note:
 *  1. The addr value should be offset address from the chip base in multiple chip.
 *  2. For some single chip solution, it maybe input physical address value.
 */
int32
ioal_mem32_check(uint32 unit, uint32 *addr)
{
    uint32 base;
    int32 ret = RT_ERR_FAILED;

    /* Upper layer have check the unit, and don't need to check again */

    RT_ERR_CHK(ioal_init_memRegion_get(unit, IOAL_MEM_SWCORE, &base), ret);   
    *addr += base;  
    
    return RT_ERR_OK;
} /* end of ioal_mem32_check */


/* Function Name:
 *      ioal_mem32_read
 * Description:
 *      Get the value from register.
 * Input:
 *      unit - unit id
 *      addr - register address
 * Output:
 *      pVal - pointer buffer of the register value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      1. Support single unit right now and ignore unit
 *      2. When we support the multiple chip in future, we will check the input unit
 */
int32
ioal_mem32_read(uint32 unit, uint32 addr, uint32 *pVal)
{
    int32 ret = RT_ERR_FAILED;

    /* Upper layer have check the unit, and don't need to check again */

    RT_ERR_CHK(ioal_mem32_check(unit, &addr), ret);
    
    *pVal = MEM32_READ(addr);

    return ret;
} /* end of ioal_mem32_read */


/* Function Name:
 *      ioal_mem32_write
 * Description:
 *      Set the value to register.
 * Input:
 *      unit - unit id
 *      addr - register address
 *      val  - the value to write register
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      1. Support single unit right now and ignore unit
 *      2. When we support the multiple chip in future, we will check the input unit
 */
int32 ioal_mem32_write(uint32 unit, uint32 addr, uint32 val)
{
    int32 ret = RT_ERR_FAILED;

    /* Upper layer have check the unit, and don't need to check again */

    RT_ERR_CHK(ioal_mem32_check(unit, &addr), ret);

    MEM32_WRITE(addr, val);
    
    return ret;
} /* end of ioal_mem32_write */

/* Function Name:
 *      ioal_mem32_field_read
 * Description:
 *      Read the value from the field of register.
 * Input:
 *      unit   - unit id
 *      addr   - register address
 *      offset - field offset
 *      mask   - field mask
 * Output:
 *      pVal - pointer buffer of the register field value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
ioal_mem32_field_read(uint32 unit, uint32 addr, uint32 offset, uint32 mask, uint32 *pVal)
{
    int32 ret = RT_ERR_FAILED;

    /* Upper layer have check the unit, and don't need to check again */

    RT_ERR_CHK(ioal_mem32_check(unit, &addr), ret);

    *pVal = (MEM32_READ(addr) & mask) >> offset;

    return ret;
} /* end of ioal_mem32_field_read */

/* Function Name:
 *      ioal_mem32_field_write
 * Description:
 *      Write the value to the field of register.
 * Input:
 *      unit   - unit id
 *      addr   - register address
 *      offset - field offset
 *      mask   - field mask
 *      val    - the value to write register
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
ioal_mem32_field_write(uint32 unit, uint32 addr, uint32 offset, uint32 mask, uint32 val)
{
    int32 ret = RT_ERR_FAILED;

    /* Upper layer have check the unit, and don't need to check again */

    RT_ERR_CHK(ioal_mem32_check(unit, &addr), ret);

    MEM32_WRITE(addr, (MEM32_READ(addr) & ~mask) | (val << offset));

    return ret;
} /* end of ioal_mem32_read */
