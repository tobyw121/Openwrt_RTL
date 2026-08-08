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
 * $Revision: 53488 $
 * $Date: 2014-11-28 18:15:33 +0800 (Fri, 28 Nov 2014) $
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
#include <common/rt_type.h>
#ifdef CONFIG_X86_I2C
#include <rtk_i2c.h>
#include <virtualmac/vmac_target.h>  
#endif
#include <ioal/ioal_init.h>
#include <ioal/mem32.h>
#include <common/debug/rt_log.h>

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
    RT_LOG(LOG_DEBUG, MOD_RTCORE, "real addr=0x%x\n", *addr);
    
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
#ifdef CONFIG_X86_I2C
    enum IC_TYPE ictype;
#endif    

    /* Upper layer have check the unit, and don't need to check again */
#ifdef CONFIG_X86_I2C

    vmac_getTarget(&ictype);
    if (IC_TYPE_REAL == ictype)
    {
        ret = RTL8380_I2C_READ(addr, pVal);
    }
    else
    {
        RT_ERR_CHK(ioal_mem32_check(unit, &addr), ret);

        *pVal = MEM32_READ(addr);
    }     

#else
    RT_ERR_CHK(ioal_mem32_check(unit, &addr), ret);

#if defined (CONFIG_SDK_RTL8380)
    if(ioal_chipId_8380_8330_flag)
    {
        /*8330 & 8380*/
        /*Illegal Reg Range*/
        if(((addr >= 0xbb00d560) && (addr <= 0xbb00d95f)) || \
            ((addr >= 0xbb001200) && (addr <= 0xbb0019ff)))
        {
            *pVal = 0;
        }
        else
        {
            *pVal = MEM32_READ(addr);
        }
    }
    else
#endif        
    {
        *pVal = MEM32_READ(addr);
    }

#endif

#if defined(CONFIG_RTL8380_VERA)
    printf("get_reg_wrapper(16'h%x, data);\n", addr & 0xffff);
    printf("repeat(20) @(system_clk.cb);\n\n");
#endif

    RT_LOG(LOG_INFO, MOD_RTCORE, "register addr=0x%x read val=0x%x\n", addr, *pVal);

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
#ifdef CONFIG_X86_I2C
    enum IC_TYPE ictype;
#endif    
    
#if defined(CONFIG_RTL8380_VERA)
        printf("set_reg_wrapper(16'h%x, 32'h%x);\n", addr & 0xffff, val);
        printf("repeat(20) @(system_clk.cb);\n\n");
#endif

    /* Upper layer have check the unit, and don't need to check again */
#ifdef CONFIG_X86_I2C

    vmac_getTarget(&ictype);

    if (IC_TYPE_REAL == ictype)
    {
        ret = RTL8380_I2C_WRITE(addr, val);
    }
    else
    {
        RT_ERR_CHK(ioal_mem32_check(unit, &addr), ret);

        MEM32_WRITE(addr, val);
    }     
#else
    RT_ERR_CHK(ioal_mem32_check(unit, &addr), ret);

    if(0)
    {
        /*8330 & 8380*/
        /*Illegal Reg Range*/
        if(((addr >= 0xbb00d560) && (addr <= 0xbb00d95f)) || \
            ((addr >= 0xbb001200) && (addr <= 0xbb0019ff)))
        {
            /*Do nothing*/
        }
        else
        {
            MEM32_WRITE(addr, val);
        }
    }
    else
    {
        MEM32_WRITE(addr, val);
    }
#endif

    
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
#ifdef CONFIG_X86_I2C
    enum IC_TYPE ictype;
#endif  

    /* Upper layer have check the unit, and don't need to check again */
#ifdef CONFIG_X86_I2C
    vmac_getTarget(&ictype);
    if (IC_TYPE_REAL == ictype)
    {
        ret = RTL8380_I2C_READ(addr, pVal);
        *pVal = ((*pVal) & mask) >> offset;
    }
    else
    {
        RT_ERR_CHK(ioal_mem32_check(unit, &addr), ret);

        *pVal = (MEM32_READ(addr) & mask) >> offset;
    }     
#else
    RT_ERR_CHK(ioal_mem32_check(unit, &addr), ret);


    if(0)
    {
        /*8330 & 8380*/
        /*Illegal Reg Range*/
        if(((addr >= 0xbb00d560) && (addr <= 0xbb00d95f)) || \
            ((addr >= 0xbb001200) && (addr <= 0xbb0019ff)))
        {
            *pVal = 0;
        }
        else
        {
            *pVal = (MEM32_READ(addr) & mask) >> offset;
        }
    }
    else
    {
        *pVal = (MEM32_READ(addr) & mask) >> offset;
    }
#endif

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
#ifdef CONFIG_X86_I2C
    enum IC_TYPE ictype;
    uint32 value;
#endif  

    /* Upper layer have check the unit, and don't need to check again */
#ifdef CONFIG_X86_I2C
    vmac_getTarget(&ictype);

    if (IC_TYPE_REAL == ictype)
    {
        RTL8380_I2C_READ(addr, &value); 
        ret = RTL8380_I2C_WRITE(addr, (value & ~mask) | (val << offset));
    }
    else
    {
        RT_ERR_CHK(ioal_mem32_check(unit, &addr), ret);

        MEM32_WRITE(addr, (MEM32_READ(addr) & ~mask) | (val << offset));
    }     
#else
    RT_ERR_CHK(ioal_mem32_check(unit, &addr), ret);

    if(0)
    {
        /*8330 & 8380*/
        /*Illegal Reg Range*/
        if(((addr >= 0xbb00d560) && (addr <= 0xbb00d95f)) || \
            ((addr >= 0xbb001200) && (addr <= 0xbb0019ff)))
        {
            /*Do nothing*/
        }
        else
        {
            MEM32_WRITE(addr, (MEM32_READ(addr) & ~mask) | (val << offset));
        }
    }
    else
    {
        MEM32_WRITE(addr, (MEM32_READ(addr) & ~mask) | (val << offset));
    }
#endif

    return ret;
} /* end of ioal_mem32_field_write */
