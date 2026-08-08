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
 * $Revision: 57050 $
 * $Date: 2015-03-23 14:36:24 +0800 (Mon, 23 Mar 2015) $
 *
 * Purpose : chip symbol and data type definition in the SDK.
 *
 * Feature : chip symbol and data type definition
 *
 */

/*
 * Include Files
 */
#include <common/rt_autoconf.h>
#include <common/debug/rt_log.h>
#include <ioal/mem32.h>
#if defined(CONFIG_SDK_RTL8389)
#include <drv/swcore/rtl8389.h>
#endif
#if defined(CONFIG_SDK_RTL8390)
#include <drv/swcore/rtl8390.h>
#endif
#if defined(CONFIG_SDK_RTL8380)
#include <drv/swcore/rtl8380.h>
#endif
#if defined(RTK_UNIVERSAL_BSP)
#if defined(CONFIG_RTL8390_SERIES)
#include <drv/swcore/rtl8390.h>
#endif
#if defined(CONFIG_RTL8380_SERIES)
#include <drv/swcore/rtl8380.h>
#endif
#endif
#if defined(RTK_UNIVERSAL_BSP)
#include "chip.h"
#else
#include <drv/swcore/chip.h>
#include <soc/soc.h>
#endif
/*
 * Symbol Definition
 */

#if defined(RTK_UNIVERSAL_BSP)
#define SWCORE_VIRT_BASE    0xBB000000
#endif
/*
 * Data Declaration
 */
typedef int32 (*hal_get_chip_id_f)(uint32 unit, uint32 *pChip_id, uint32 *pChip_rev_id);


#if defined(RTK_UNIVERSAL_BSP)
#if defined(CONFIG_RTL8328_SERIES)
static int32 _bsp_drv_swcore_cid8328_get(uint32 unit, uint32 *pChip_id, uint32 *pChip_rev_id);
#endif
#if defined(CONFIG_RTL8380_SERIES)
static int32 _bsp_drv_swcore_cid8380_get(uint32 unit, uint32 *pChip_id, uint32 *pChip_rev_id);
#endif
#if defined(CONFIG_RTL8390_SERIES)
static int32 _bsp_drv_swcore_cid8390_get(uint32 unit, uint32 *pChip_id, uint32 *pChip_rev_id);
#endif

static hal_get_chip_id_f func[] =
{
#if defined(CONFIG_RTL8328_SERIES)
    _bsp_drv_swcore_cid8328_get,
#endif
#if defined(CONFIG_RTL8380_SERIES)
    _bsp_drv_swcore_cid8380_get,
#endif
#if defined(CONFIG_RTL8390_SERIES)
    _bsp_drv_swcore_cid8390_get,
#endif
};

extern unsigned int bsp_chip_type;

#else  /*SDK part*/
#if defined(CONFIG_SDK_RTL8389)
static int32 _drv_swcore_rtl8377m_det(uint32 unit);
static int32 _drv_swcore_cid8389_get(uint32 unit, uint32 *pChip_id, uint32 *pChip_rev_id);
#endif
#if defined(CONFIG_SDK_RTL8328)
static int32 _drv_swcore_cid8328_get(uint32 unit, uint32 *pChip_id, uint32 *pChip_rev_id);
#endif
#if defined(CONFIG_SDK_RTL8380)
static int32 _drv_swcore_cid8380_get(uint32 unit, uint32 *pChip_id, uint32 *pChip_rev_id);
#endif
#if defined(CONFIG_SDK_RTL8390)
static int32 _drv_swcore_cid8390_get(uint32 unit, uint32 *pChip_id, uint32 *pChip_rev_id);
#endif

static hal_get_chip_id_f func[] =
{
#if defined(CONFIG_SDK_RTL8328)
    _drv_swcore_cid8328_get,
#endif
#if defined(CONFIG_SDK_RTL8389)
    _drv_swcore_cid8389_get,
#endif
#if defined(CONFIG_SDK_RTL8380)
    _drv_swcore_cid8380_get,
#endif
#if defined(CONFIG_SDK_RTL8390)
    _drv_swcore_cid8390_get,
#endif
};

uint32 chip_id_updated_flag = 0;
uint32 drv_chip_id, drv_chip_rev_id, drv_chip_family_id;

#endif
/*
 * Macro Definition
 */
#if defined(RTK_UNIVERSAL_BSP)	

#define RT_ERR_OK 0
#define RT_ERR_FAILED -1
	
#define ioal_mem32_read bsp_mem32_read
#define ioal_mem32_write bsp_mem32_write

#define BSP_CHIP_TYPE_ASSIGN(type)\
do {\
	bsp_chip_type = type;\
} while(0)

#endif

#if !defined(RTK_UNIVERSAL_BSP)
#define BSP_CHIP_TYPE_ASSIGN(type)\
do {\
} while(0)

#endif

#if defined(RTK_UNIVERSAL_BSP)	

/*
 * Function Declaration
 */
	
/* Function Name:
 *		bsp_mem32_read
 * Description:
 *		Get the value from register.
 * Input:
 *		unit - unit id
 *		addr - register address
 * Output:
 *		pVal - pointer buffer of the register value
 * Return:
 *		RT_ERR_OK
 *		RT_ERR_FAILED
 * Note:
 *		1. Support single unit right now and ignore unit
 *		2. When we support the multiple chip in future, we will check the input unit
 */
int32
bsp_mem32_read(uint32 unit, uint32 addr, uint32 *pVal)
{
	int32 ret = RT_ERR_OK;

	/* Upper layer have check the unit, and don't need to check again */
	
	*pVal = MEM32_READ(SWCORE_VIRT_BASE | addr);

	return ret;
} /* end of bsp_mem32_read */
	
	
/* Function Name:
 *		bsp_mem32_write
 * Description:
 *		Set the value to register.
 * Input:
 *		unit - unit id
 *		addr - register address
 *		val  - the value to write register
 * Output:
 *		None
 * Return:
 *		RT_ERR_OK
 *		RT_ERR_FAILED
 * Note:
 *		1. Support single unit right now and ignore unit
 *		2. When we support the multiple chip in future, we will check the input unit
 */
int32 bsp_mem32_write(uint32 unit, uint32 addr, uint32 val)
{
	int32 ret = RT_ERR_OK;

	MEM32_WRITE(SWCORE_VIRT_BASE | addr, val);
	
	return ret;
} /* end of bsp_mem32_write */
#endif

/*
 * Function Declaration
 */
#if defined(CONFIG_SDK_RTL8389)
/* Function Name:
 *      _drv_swcore_rtl8377m_det
 * Description:
 *      Check the device is RTL8377M or RTL8389M?
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - is RTL8377M device
 *      RT_ERR_FAILED - is RTL8389M device
 * Note:
 *      Due to the chip id is same and need specified method to check out
 *      the device is RTL8377M or RTL8389M?
 *      - for RTL8377M; port 0-7 serdes should link-down.
 *      - for RTL8389M; port 0-7 serdes should link-up.
 */
static int32
_drv_swcore_rtl8377m_det(uint32 unit)
{
    uint32 reg = 0;

    ioal_mem32_read(unit, 0x3260, &reg);
    ioal_mem32_read(unit, 0x3260, &reg);
    if (reg & 1)
        return RT_ERR_FAILED;

    ioal_mem32_read(unit, 0x3264, &reg);
    ioal_mem32_read(unit, 0x3264, &reg);
    if (reg & 1)
        return RT_ERR_FAILED;

    ioal_mem32_read(unit, 0x32E0, &reg);
    ioal_mem32_read(unit, 0x32E0, &reg);
    if (reg & 1)
        return RT_ERR_FAILED;

    ioal_mem32_read(unit, 0x32E4, &reg);
    ioal_mem32_read(unit, 0x32E4, &reg);
    if (reg & 1)
        return RT_ERR_FAILED;

    ioal_mem32_read(unit, 0x3360, &reg);
    ioal_mem32_read(unit, 0x3360, &reg);
    if (reg & 1)
        return RT_ERR_FAILED;

    ioal_mem32_read(unit, 0x3364, &reg);
    ioal_mem32_read(unit, 0x3364, &reg);
    if (reg & 1)
        return RT_ERR_FAILED;

    ioal_mem32_read(unit, 0x33E0, &reg);
    ioal_mem32_read(unit, 0x33E0, &reg);
    if (reg & 1)
        return RT_ERR_FAILED;

    ioal_mem32_read(unit, 0x33E4, &reg);
    ioal_mem32_read(unit, 0x33E4, &reg);
    if (reg & 1)
        return RT_ERR_FAILED;

    return RT_ERR_OK;

} /* end of _drv_swcore_rtl8377m_det */

static int32
_drv_swcore_cid8389_get(uint32 unit, uint32 *pChip_id, uint32 *pChip_rev_id)
{
    uint32 temp = 0;

    /* parameter check */
    RT_INTERNAL_PARAM_CHK((NULL == pChip_id), RT_ERR_NULL_POINTER);
    RT_INTERNAL_PARAM_CHK((NULL == pChip_rev_id), RT_ERR_NULL_POINTER);

#if defined(CONFIG_SDK_RTL8389)
  #if defined(__MODEL_USER__)

    *pChip_id = temp = RTL8329M_CHIP_ID;
    *pChip_rev_id = temp & 0xF;

  #else

    /* Turn on the internal register
     * bit[19:16] internal register access control
     * - 0b1010 mean enabled
     * - 0b1111 mean disabled
     */
    ioal_mem32_read(unit, RTL8389_GLOBAL_MAC_CONTROL0_ADDR, &temp);
    temp &= ~(0xF << 16);
    temp |= (0xA << 16);
    ioal_mem32_write(unit, RTL8389_GLOBAL_MAC_CONTROL0_ADDR, temp);

    /* Read the CHIP_TYP & INT_VER field from GLOBAL_MAC_INTERNAL_STATUS_ADDR
     * bit[7:4] chip type
     * - 0b0000: RTL8389M_CHIP_ID
     * - 0b0001: RTL8389L_CHIP_ID
     * - 0b0010: RTL8329M_CHIP_ID
     * - 0b0011: RTL8329_CHIP_ID
     * - others: reserved
     */
    ioal_mem32_read(unit, RTL8389_GLOBAL_MAC_INTERNAL_STATUS_ADDR, &temp);

    switch ((temp >> 4) & 0xF)
    {
        case 0:
            /* check serdes to know it is 77M or 89M */
            if (RT_ERR_OK == _drv_swcore_rtl8377m_det(unit))
                *pChip_id = RTL8377M_CHIP_ID;
            else
                *pChip_id = RTL8389M_CHIP_ID;
            break;

        case 1:
            *pChip_id = RTL8389L_CHIP_ID;
            break;

        case 2:
            *pChip_id = RTL8329M_CHIP_ID;
            break;

        case 3:
            *pChip_id = RTL8329_CHIP_ID;
            break;

        default:
            return RT_ERR_FAILED;
    }

    /*
     * GLOBAL_MAC_INTERNAL_STATUS_ADDR
     * bit[3:0] chip rev id
     */
    *pChip_rev_id = (temp & 0xF);

    /* Turn off the internal register */
    ioal_mem32_read(unit, RTL8389_GLOBAL_MAC_CONTROL0_ADDR, &temp);
    temp |= (0xF << 16);
    ioal_mem32_write(unit, RTL8389_GLOBAL_MAC_CONTROL0_ADDR, temp);

  #endif /* end of defined(__MODEL_USER__) */

    RT_LOG(LOG_TRACE, MOD_HAL, "chip_id=0x%X, rev_id=0x%X\n", *pChip_id, *pChip_rev_id);

#endif /* end of defined(CONFIG_SDK_RTL8389) */

    return RT_ERR_OK;
} /* end of _drv_swcore_cid8389_get */

#endif /* defined(CONFIG_SDK_RTL8389) */

#if (defined(RTK_UNIVERSAL_BSP) && defined(CONFIG_RTL8328_SERIES))
static int32
_bsp_drv_swcore_cid8328_get(uint32 unit, uint32 *pChip_id, uint32 *pChip_rev_id)
#endif
#if (!defined(RTK_UNIVERSAL_BSP) && defined(CONFIG_SDK_RTL8328))
static int32
_drv_swcore_cid8328_get(uint32 unit, uint32 *pChip_id, uint32 *pChip_rev_id)
#endif
#if (defined(RTK_UNIVERSAL_BSP) && defined(CONFIG_RTL8328_SERIES)) || (!defined(RTK_UNIVERSAL_BSP) && defined(CONFIG_SDK_RTL8328))
{
    uint32  temp = 0, value = 0;

	BSP_CHIP_TYPE_ASSIGN(CHIP_MP_TYPE);

    if (ioal_mem32_read(unit, 0x6FFF8, &temp) != RT_ERR_OK)
        return RT_ERR_FAILED;
    if (((temp>>16)&0xFFFF) != 0x8328)
        return RT_ERR_FAILED;
    else
    {
        /* enable internal read */
        if (ioal_mem32_write(unit, 0x6FFF0, 1) != RT_ERR_OK)
            return RT_ERR_FAILED;
        if (ioal_mem32_read(unit, 0x60128, &value) != RT_ERR_OK)
            return RT_ERR_FAILED;
        if (((value >> 4) & 0x1) == 0x1)
        {
            *pChip_id = RTL8328L_CHIP_ID;
        }
        else
        {
            if (((temp>>11)&0x1F) == 0x9)
                *pChip_id = RTL8328M_CHIP_ID;
            else
                *pChip_id = RTL8328S_CHIP_ID;
        }
        /* disable internal read */
        if (ioal_mem32_write(unit, 0x6FFF0, 0) != RT_ERR_OK)
            return RT_ERR_FAILED;
    }

    /* Detect the chip revision id */
    temp = 0;
    if (ioal_mem32_read(unit, 0x6FFF4, &temp) != RT_ERR_OK)
        return RT_ERR_FAILED;

    if ((temp & 0xFF) == 1)
    {
        /* enable internal read */
        if (ioal_mem32_write(unit, 0x6FFF0, 1) != RT_ERR_OK)
            return RT_ERR_FAILED;
        if (ioal_mem32_read(unit, 0x60128, &value) != RT_ERR_OK)
            return RT_ERR_FAILED;
        if (((value >> 5) & 0x1) == 0x1)
        {
            *pChip_rev_id = CHIP_REV_ID_C;
        }
        else
        {
            *pChip_rev_id = CHIP_REV_ID_B;
        }
        /* disable internal read */
        if (ioal_mem32_write(unit, 0x6FFF0, 0) != RT_ERR_OK)
            return RT_ERR_FAILED;
    }
    else
        *pChip_rev_id = (temp & 0xFF);

    return RT_ERR_OK;
} /* end of _drv_swcore_cid8328_get */
#endif

#if (defined(RTK_UNIVERSAL_BSP) && defined(CONFIG_RTL8380_SERIES))
static int32
_bsp_drv_swcore_cid8380_get(uint32 unit, uint32 *pChip_id, uint32 *pChip_rev_id)
#endif
#if (!defined(RTK_UNIVERSAL_BSP) && defined(CONFIG_SDK_RTL8380))
static int32
_drv_swcore_cid8380_get(uint32 unit, uint32 *pChip_id, uint32 *pChip_rev_id)
#endif
#if (defined(RTK_UNIVERSAL_BSP) && defined(CONFIG_RTL8380_SERIES)) || (!defined(RTK_UNIVERSAL_BSP) && defined(CONFIG_SDK_RTL8380))
{
    uint32 original_data_intRd;
    uint32 original_data_chipRd;
    uint32 temp = 0, temp1 = 0;

    /* parameter check */
    RT_INTERNAL_PARAM_CHK((NULL == pChip_id), RT_ERR_NULL_POINTER);
    RT_INTERNAL_PARAM_CHK((NULL == pChip_rev_id), RT_ERR_NULL_POINTER);

#if defined(__MODEL_USER__) || defined(CONFIG_X86_I2C)
    *pChip_id = RTL8382M_CHIP_ID; /* orignal: RTL8382M_CHIP_ID, change to 8380 for testing zero-based port view */
    *pChip_rev_id = CHIP_REV_ID_A;
#elif defined(CONFIG_SDK_FPGA_PLATFORM) || defined(CONFIG_SDK_MODEL_MODE)
    /* Check the part can used dynamic probe or not?
     * If Yes, remove the code and used the probe #else
     */
    {
    uint32  temp_chip_info = 0;

    ioal_mem32_read(unit, RTL8380_INT_RW_CTRL_ADDR, &temp);
    original_data_intRd = temp;
    temp &= ~(0x3);
    temp |= (0x3);
    ioal_mem32_write(unit, RTL8380_INT_RW_CTRL_ADDR, temp);

    ioal_mem32_read(unit, 0xD0, &temp1);
    temp1 &= 0x1f;

    ioal_mem32_read(unit, RTL8380_CHIP_INFO_ADDR, &temp);
    original_data_chipRd = temp;
    temp &= ~(RTL8380_CHIP_INFO_CHIP_INFO_EN_MASK);
    temp |= (0xA << RTL8380_CHIP_INFO_CHIP_INFO_EN_OFFSET);
    ioal_mem32_write(unit, RTL8380_CHIP_INFO_ADDR, temp);

    ioal_mem32_read(unit, RTL8380_MODEL_NAME_INFO_ADDR, &temp);

	BSP_CHIP_TYPE_ASSIGN(CHIP_TYPE_NOTFOUND);

    switch (temp & 0xfffff800)
    {
        case RTL8330M_CHIP_ID & 0xfffff800:
            if(temp1 == 2)    
            {
                *pChip_id = RTL8330M_CHIP_ID;
                *pChip_rev_id = 1;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MP_TYPE);
            }
            break;
        case RTL8332M_CHIP_ID & 0xfffff800:
            if(temp1 == 2)    
            {
                *pChip_id = RTL8332M_CHIP_ID;
                *pChip_rev_id = 1;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MP_TYPE);
            }
            break;
        case RTL8380M_CHIP_ID & 0xfffff800:
            if(temp1 == 2)   
            {
                *pChip_id = RTL8380M_CHIP_ID;
                *pChip_rev_id = 1;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MP_TYPE);
            }  
            break;
        case RTL8382M_CHIP_ID & 0xfffff800:
            if(temp1 == 2)  
            {
                *pChip_id = RTL8382M_CHIP_ID;
                *pChip_rev_id = 1;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MP_TYPE);
            }
            break;
        default:
            return RT_ERR_FAILED;
    }

    if(temp1 == 2)
    {
        ioal_mem32_write(unit, RTL8380_CHIP_INFO_ADDR, original_data_chipRd);
        ioal_mem32_write(unit, RTL8380_INT_RW_CTRL_ADDR, original_data_intRd);
        return RT_ERR_OK;
    }
    else
    {
        ioal_mem32_read(unit, RTL8380_CHIP_INFO_ADDR, &temp_chip_info);
        ioal_mem32_write(unit, RTL8380_CHIP_INFO_ADDR, original_data_chipRd);
        ioal_mem32_write(unit, RTL8380_INT_RW_CTRL_ADDR, original_data_intRd);
    }
    
    switch (temp & 0xfffff800)
    {
        case RTL8330M_CHIP_ID & 0xfffff800:
            if ((temp_chip_info & RTL8380_CHIP_INFO_RL_ID_MASK) == 0x0477)
            {
                *pChip_id = RTL8330MES_CHIP_ID;
                *pChip_rev_id = (temp_chip_info & RTL8380_CHIP_INFO_CHIP_VER_MASK) >> RTL8380_CHIP_INFO_CHIP_VER_OFFSET;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MES_TYPE);
            }
            else
            {
                *pChip_id = RTL8330M_CHIP_ID;
                *pChip_rev_id = ((temp_chip_info & RTL8380_CHIP_INFO_CHIP_VER_MASK) >> RTL8380_CHIP_INFO_CHIP_VER_OFFSET) - 1;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MP_TYPE);
            }
            break;
        case RTL8332M_CHIP_ID & 0xfffff800:
            if ((temp_chip_info & RTL8380_CHIP_INFO_RL_ID_MASK) == 0x0477)
            {
                *pChip_id = RTL8332MES_CHIP_ID;
                *pChip_rev_id = (temp_chip_info & RTL8380_CHIP_INFO_CHIP_VER_MASK) >> RTL8380_CHIP_INFO_CHIP_VER_OFFSET;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MES_TYPE);
            }
            else
            {
                *pChip_id = RTL8332M_CHIP_ID;
                *pChip_rev_id = ((temp_chip_info & RTL8380_CHIP_INFO_CHIP_VER_MASK) >> RTL8380_CHIP_INFO_CHIP_VER_OFFSET) - 1;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MP_TYPE);
            }
            break;
        case RTL8380M_CHIP_ID & 0xfffff800:
            if ((temp_chip_info & RTL8380_CHIP_INFO_RL_ID_MASK) == 0x0477)
            {
                *pChip_id = RTL8380MES_CHIP_ID;
                *pChip_rev_id = (temp_chip_info & RTL8380_CHIP_INFO_CHIP_VER_MASK) >> RTL8380_CHIP_INFO_CHIP_VER_OFFSET;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MES_TYPE);
            }
            else
            {
                *pChip_id = RTL8380M_CHIP_ID;
                *pChip_rev_id = ((temp_chip_info & RTL8380_CHIP_INFO_CHIP_VER_MASK) >> RTL8380_CHIP_INFO_CHIP_VER_OFFSET) - 1;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MP_TYPE);
            }
            break;
        case RTL8382M_CHIP_ID & 0xfffff800:
            if ((temp_chip_info & RTL8380_CHIP_INFO_RL_ID_MASK) == 0x0477)
            {
                *pChip_id = RTL8382MES_CHIP_ID;
                *pChip_rev_id = (temp_chip_info & RTL8380_CHIP_INFO_CHIP_VER_MASK) >> RTL8380_CHIP_INFO_CHIP_VER_OFFSET;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MES_TYPE);
            }
            else
            {
                *pChip_id = RTL8382M_CHIP_ID;
                *pChip_rev_id = ((temp_chip_info & RTL8380_CHIP_INFO_CHIP_VER_MASK) >> RTL8380_CHIP_INFO_CHIP_VER_OFFSET) - 1;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MP_TYPE);
            }
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    }
#else
    {
    uint32  temp_chip_info = 0;

    if (ioal_mem32_read(unit, RTL8380_MODEL_NAME_INFO_ADDR, &temp) != RT_ERR_OK)
        return RT_ERR_FAILED;
    if ((((temp>>16)&0xFFFF) != 0x8330) && (((temp>>16)&0xFFFF) != 0x8332) &&
        (((temp>>16)&0xFFFF) != 0x8380) && (((temp>>16)&0xFFFF) != 0x8382))
        return RT_ERR_FAILED;

    ioal_mem32_read(unit, RTL8380_INT_RW_CTRL_ADDR, &temp);
    original_data_intRd = temp;
    temp &= ~(0x3);
    temp |= (0x3);
    ioal_mem32_write(unit, RTL8380_INT_RW_CTRL_ADDR, temp);

    ioal_mem32_read(unit, 0xD0, &temp1);
    temp1 &= 0x1f;

    ioal_mem32_read(unit, RTL8380_CHIP_INFO_ADDR, &temp);
    original_data_chipRd = temp;
    temp &= ~(RTL8380_CHIP_INFO_CHIP_INFO_EN_MASK);
    temp |= (0xA << RTL8380_CHIP_INFO_CHIP_INFO_EN_OFFSET);
    ioal_mem32_write(unit, RTL8380_CHIP_INFO_ADDR, temp);

    ioal_mem32_read(unit, RTL8380_MODEL_NAME_INFO_ADDR, &temp);

	BSP_CHIP_TYPE_ASSIGN(CHIP_TYPE_NOTFOUND);

    switch (temp & 0xfffff800)
    {
        case RTL8330M_CHIP_ID & 0xfffff800:
            if(temp1 == 2)    
            {
                *pChip_id = RTL8330M_CHIP_ID;
                *pChip_rev_id = 1;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MP_TYPE);
            }
            break;
        case RTL8332M_CHIP_ID & 0xfffff800:
            if(temp1 == 2)    
            {
                *pChip_id = RTL8332M_CHIP_ID;
                *pChip_rev_id = 1;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MP_TYPE);
            }
            break;
        case RTL8380M_CHIP_ID & 0xfffff800:
            if(temp1 == 2)   
            {
                *pChip_id = RTL8380M_CHIP_ID;
                *pChip_rev_id = 1;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MP_TYPE);
            }  
            break;
        case RTL8382M_CHIP_ID & 0xfffff800:
            if(temp1 == 2)  
            {
                *pChip_id = RTL8382M_CHIP_ID;
                *pChip_rev_id = 1;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MP_TYPE);
            }
            break;
        default:
            return RT_ERR_FAILED;
    }

    if(temp1 == 2)
    {
        ioal_mem32_write(unit, RTL8380_CHIP_INFO_ADDR, original_data_chipRd);
        ioal_mem32_write(unit, RTL8380_INT_RW_CTRL_ADDR, original_data_intRd);
        return RT_ERR_OK;
    }
    else
    {
        ioal_mem32_read(unit, RTL8380_CHIP_INFO_ADDR, &temp_chip_info);
        ioal_mem32_write(unit, RTL8380_CHIP_INFO_ADDR, original_data_chipRd);
        ioal_mem32_write(unit, RTL8380_INT_RW_CTRL_ADDR, original_data_intRd);
    }
    
    switch (temp & 0xfffff800)
    {
        case RTL8330M_CHIP_ID & 0xfffff800:
            if ((temp_chip_info & RTL8380_CHIP_INFO_RL_ID_MASK) == 0x0477)
            {
                *pChip_id = RTL8330MES_CHIP_ID;
                *pChip_rev_id = (temp_chip_info & RTL8380_CHIP_INFO_CHIP_VER_MASK) >> RTL8380_CHIP_INFO_CHIP_VER_OFFSET;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MES_TYPE);
            }
            else
            {
                *pChip_id = RTL8330M_CHIP_ID;
                *pChip_rev_id = ((temp_chip_info & RTL8380_CHIP_INFO_CHIP_VER_MASK) >> RTL8380_CHIP_INFO_CHIP_VER_OFFSET) - 1;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MP_TYPE);
            }
            break;
        case RTL8332M_CHIP_ID & 0xfffff800:
            if ((temp_chip_info & RTL8380_CHIP_INFO_RL_ID_MASK) == 0x0477)
            {
                *pChip_id = RTL8332MES_CHIP_ID;
                *pChip_rev_id = (temp_chip_info & RTL8380_CHIP_INFO_CHIP_VER_MASK) >> RTL8380_CHIP_INFO_CHIP_VER_OFFSET;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MES_TYPE);
            }
            else
            {
                *pChip_id = RTL8332M_CHIP_ID;
                *pChip_rev_id = ((temp_chip_info & RTL8380_CHIP_INFO_CHIP_VER_MASK) >> RTL8380_CHIP_INFO_CHIP_VER_OFFSET) - 1;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MP_TYPE);
            }
            break;
        case RTL8380M_CHIP_ID & 0xfffff800:
            if ((temp_chip_info & RTL8380_CHIP_INFO_RL_ID_MASK) == 0x0477)
            {
                *pChip_id = RTL8380MES_CHIP_ID;
                *pChip_rev_id = (temp_chip_info & RTL8380_CHIP_INFO_CHIP_VER_MASK) >> RTL8380_CHIP_INFO_CHIP_VER_OFFSET;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MES_TYPE);
            }
            else
            {
                *pChip_id = RTL8380M_CHIP_ID;
                *pChip_rev_id = ((temp_chip_info & RTL8380_CHIP_INFO_CHIP_VER_MASK) >> RTL8380_CHIP_INFO_CHIP_VER_OFFSET) - 1;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MP_TYPE);
            }
            break;
        case RTL8382M_CHIP_ID & 0xfffff800:
            if ((temp_chip_info & RTL8380_CHIP_INFO_RL_ID_MASK) == 0x0477)
            {
                *pChip_id = RTL8382MES_CHIP_ID;
                *pChip_rev_id = (temp_chip_info & RTL8380_CHIP_INFO_CHIP_VER_MASK) >> RTL8380_CHIP_INFO_CHIP_VER_OFFSET;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MES_TYPE);
            }
            else
            {
                *pChip_id = RTL8382M_CHIP_ID;
                *pChip_rev_id = ((temp_chip_info & RTL8380_CHIP_INFO_CHIP_VER_MASK) >> RTL8380_CHIP_INFO_CHIP_VER_OFFSET) - 1;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MP_TYPE);
            }
            break;
        default:
            return RT_ERR_FAILED;
    }    
    }
#endif
    return RT_ERR_OK;
} /* end of _drv_swcore_cid8380_get */
#endif


#if (defined(RTK_UNIVERSAL_BSP) && defined(CONFIG_RTL8390_SERIES))
static int32
_bsp_drv_swcore_cid8390_get(uint32 unit, uint32 *pChip_id, uint32 *pChip_rev_id)
#endif
#if (!defined(RTK_UNIVERSAL_BSP) && defined(CONFIG_SDK_RTL8390))
static int32
_drv_swcore_cid8390_get(uint32 unit, uint32 *pChip_id, uint32 *pChip_rev_id)
#endif
#if (defined(RTK_UNIVERSAL_BSP) && defined(CONFIG_RTL8390_SERIES)) || (!defined(RTK_UNIVERSAL_BSP) && defined(CONFIG_SDK_RTL8390))
{
    uint32 model_info = 0;
#if !defined(__MODEL_USER__)
    uint32 tmp = 0;
#endif

    /* parameter check */
    RT_INTERNAL_PARAM_CHK((NULL == pChip_id), RT_ERR_NULL_POINTER);
    RT_INTERNAL_PARAM_CHK((NULL == pChip_rev_id), RT_ERR_NULL_POINTER);

#if defined(__MODEL_USER__)
    *pChip_id = model_info = RTL8393M_CHIP_ID;
    *pChip_rev_id = CHIP_REV_ID_A;
#else
    ioal_mem32_read(unit, RTL8390_MODEL_NAME_INFO_ADDR, &model_info);
    tmp = (model_info & 0x3E) >> 1;
    model_info &= 0xFFFFFFC0;

	BSP_CHIP_TYPE_ASSIGN(CHIP_TYPE_NOTFOUND);

    switch (model_info)
    {
        case RTL8391M_CHIP_ID:
            *pChip_id = RTL8391M_CHIP_ID;
			BSP_CHIP_TYPE_ASSIGN(CHIP_MP_TYPE);
            break;

        case RTL8392M_CHIP_ID:
            if (tmp == 0)
            {
                *pChip_id = RTL8392MES_CHIP_ID;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MES_TYPE);
            }
            else
            {
                *pChip_id = RTL8392M_CHIP_ID;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MP_TYPE);
            }
            break;
        case RTL8393M_CHIP_ID:
            if (tmp == 0)
            {
                *pChip_id = RTL8393MES_CHIP_ID;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MES_TYPE);
            }
            else
            {
                *pChip_id = RTL8393M_CHIP_ID;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MP_TYPE);
            }
            break;
        case RTL8396M_CHIP_ID:
            if (tmp == 0)
            {
                *pChip_id = RTL8396MES_CHIP_ID;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MES_TYPE);
            }
            else
            {
                *pChip_id = RTL8396M_CHIP_ID;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MP_TYPE);
            }
            break;
        case RTL8352M_CHIP_ID:
            if (tmp == 0)
            {
                *pChip_id = RTL8352MES_CHIP_ID;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MES_TYPE);
            }
            else
            {
                *pChip_id = RTL8352M_CHIP_ID;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MP_TYPE);
            }
            break;
        case RTL8353M_CHIP_ID:
            if (tmp == 0)
            {
                *pChip_id = RTL8353MES_CHIP_ID;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MES_TYPE);
            }
            else
            {
                *pChip_id = RTL8353M_CHIP_ID;
				BSP_CHIP_TYPE_ASSIGN(CHIP_MP_TYPE);
            }
            break;
        default:
            printk("%s %x\n", __func__, model_info);
            return RT_ERR_FAILED;
    }

    *pChip_rev_id = tmp;
#endif

    return RT_ERR_OK;
} /* end of _drv_swcore_cid8390_get */
#endif

/* Function Name:
 *      drv_swcore_cid_get
 * Description:
 *      Get chip id and chip revision id.
 * Input:
 *      unit           - unit id
 * Output:
 *      pChip_id       - pointer buffer of chip id
 *      pChip_rev_id   - pointer buffer of chip revision id
 * Return:
 *      RT_ERR_OK      - OK
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32
drv_swcore_cid_get(uint32 unit, uint32 *pChip_id, uint32 *pChip_rev_id)
{
    uint32  i;
    hal_get_chip_id_f f;

    /* parameter check */
    RT_PARAM_CHK((NULL == pChip_id), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pChip_rev_id), RT_ERR_NULL_POINTER);

#if (defined(RTK_UNIVERSAL_BSP))
    for (i = 0; i < (sizeof(func)/sizeof(hal_get_chip_id_f)); i++)
    {
        f = (hal_get_chip_id_f) func[i];
        if (RT_ERR_OK == ((f)(unit, pChip_id, pChip_rev_id)))
            return RT_ERR_OK;
    }

    return RT_ERR_FAILED;
#endif

#if (!defined(RTK_UNIVERSAL_BSP))
	if(chip_id_updated_flag == 0){
	    for (i = 0; i < (sizeof(func)/sizeof(hal_get_chip_id_f)); i++)
    	{
        	f = (hal_get_chip_id_f) func[i];
	        if (RT_ERR_OK == ((f)(unit, pChip_id, pChip_rev_id)))
	        {
	        	drv_chip_id = *pChip_id;
				drv_chip_rev_id = *pChip_rev_id;
				chip_id_updated_flag = 1;
    	        return RT_ERR_OK;
	        }	
	    }
	
    	return RT_ERR_FAILED;
	}else{
		*pChip_id = drv_chip_id;
		*pChip_rev_id = drv_chip_rev_id;		
		return RT_ERR_OK;
	}
	
	return RT_ERR_FAILED;
#endif
} /* end of drv_swcore_cid_get */

/* Function Name:
 *      drv_swcore_cid_cmp
 * Description:
 *      Compare cmp_id with the chip id of unit
 * Input:
 *      unit           - unit id
 * Output:
 *      pMacAddr       - mac address
 * Return:
 *      0              - identical
 *      RT_ERR_FAILED  - not identical
 * Note:
 *      None
 */
int32
drv_swcore_cid_cmp(uint32 unit, uint32 cmp_id)
{
    uint32 cid, crid;

    if ( RT_ERR_OK != drv_swcore_cid_get(unit, &cid, &crid))
        return RT_ERR_FAILED;

    if (cmp_id == cid)
        return 0;
    else
        return RT_ERR_FAILED;

} /* end of drv_swcore_cid_cmp */

#if !defined(RTK_UNIVERSAL_BSP)
/* Function Name:
 *      drv_swcore_family_cid_get
 * Description:
 *      Get Chip Family ID
 * Input:
 *      unit           - unit id
 * Output:
 *      pFamily_id       - Family_id
 * Return:
 *      RT_ERR_OK        - OK
 *      RT_ERR_FAILED   - Failed 
 * Note:
 *      None
 */
int32
drv_swcore_family_cid_get(uint32 unit, uint32 * pFamily_id)
{
    uint32 cid, crid;

    if ( RT_ERR_OK != drv_swcore_cid_get(unit, &cid, &crid))
        return RT_ERR_FAILED;

#if defined(CONFIG_SDK_RTL8390)
	if((cid & FAMILY_ID_MASK) == RTL8390_FAMILY_ID)
		drv_chip_family_id = RTL8390_FAMILY_ID;
	if((cid & FAMILY_ID_MASK) == RTL8350_FAMILY_ID)
		drv_chip_family_id = RTL8350_FAMILY_ID;
#endif
#if defined(CONFIG_SDK_RTL8380)
	if((cid & FAMILY_ID_MASK) == RTL8380_FAMILY_ID)
		drv_chip_family_id = RTL8380_FAMILY_ID;
	if((cid & FAMILY_ID_MASK) == RTL8330_FAMILY_ID)
		drv_chip_family_id = RTL8330_FAMILY_ID;
#endif
#if defined(CONFIG_SDK_RTL8328)
	if((cid & RTL8328_FAMILY_ID) == RTL8328_FAMILY_ID)
		drv_chip_family_id = RTL8328_FAMILY_ID;
#endif

	*pFamily_id = drv_chip_family_id;	

    return RT_ERR_OK;

} /* end of drv_swcore_family_cid_get */

/* Function Name:
 *      drv_swcore_CPU_freq_get
 * Description:
 *      Get Chip CPU Frequency
 * Input:
 *      unit           - unit id
 * Output:
 *      pCPU_freq       - CPU_freq
 * Return:
 *      RT_ERR_OK        - OK
 *      RT_ERR_FAILED   - Failed 
 *      RT_ERR_CHIP_NOT_SUPPORTED - Not support by this API
 * Note:
 *      None
 */
int32
drv_swcore_CPU_freq_get(uint32 unit, uint32 * pCPU_freq)
{
    uint32 cFamily_id;
	uint32 pll_value0, pll_value1;
	
	if ( RT_ERR_OK != drv_swcore_family_cid_get(unit, &cFamily_id))
        return RT_ERR_FAILED;

#if defined(CONFIG_SDK_RTL8390)
	if((cFamily_id == RTL8390_FAMILY_ID) || (cFamily_id == RTL8350_FAMILY_ID))
	{
		ioal_mem32_read(unit, RTL839x5x_CPU_PLL0, &pll_value0);
		ioal_mem32_read(unit, RTL839x5x_CPU_PLL1, &pll_value1);
		*pCPU_freq = 0;
		if((pll_value0 == RTL839x5x_CPU_750M_PLL0) && (pll_value1 == RTL839x5x_CPU_750M_PLL1))
			*pCPU_freq = 750;
		if((pll_value0 == RTL839x5x_CPU_700M_PLL0) && (pll_value1 == RTL839x5x_CPU_700M_PLL1))
			*pCPU_freq = 700;
		if((pll_value0 == RTL839x5x_CPU_650M_PLL0) && (pll_value1 == RTL839x5x_CPU_650M_PLL1))
			*pCPU_freq = 650;
		
		if(*pCPU_freq != 0)
			return RT_ERR_OK;
		else
			return RT_ERR_FAILED;		
	}
#endif


#if defined(CONFIG_SDK_RTL8380)
	if((cFamily_id == RTL8380_FAMILY_ID) || (cFamily_id == RTL8330_FAMILY_ID))
	{
		uint32	enable_value, pll_value2;
		enable_value = 0x3;
		ioal_mem32_write(unit, RTL838x3x_CPU_READ_CTL, enable_value);
		ioal_mem32_read(unit, RTL838x3x_CPU_CTL0, &pll_value0);
		ioal_mem32_read(unit, RTL838x3x_CPU_CTL1, &pll_value1);
		ioal_mem32_read(unit, RTL838x3x_CPU_MISC, &pll_value2);
		enable_value = 0x0;
		ioal_mem32_write(unit, RTL838x3x_CPU_READ_CTL, enable_value);
		*pCPU_freq = 0;
		if((pll_value0 == RTL838x3x_CPU_500M_CTL0) && (pll_value1 == RTL838x3x_CPU_500M_CTL1)  && (pll_value2 == RTL838x3x_CPU_500M_MISC))
			*pCPU_freq = 500;
		if((pll_value0 == RTL838x3x_CPU_300M_CTL0) && (pll_value1 == RTL838x3x_CPU_300M_CTL1)  && (pll_value2 == RTL838x3x_CPU_300M_MISC))
			*pCPU_freq = 300;
		
		if(*pCPU_freq != 0)
			return RT_ERR_OK;
		else
			return RT_ERR_FAILED;
	}
#endif

    return RT_ERR_CHIP_NOT_SUPPORTED;

} /* end of drv_swcore_CPU_freq_get */

#define MAX_REG_IDX	64
/* Function Name:
 *      drv_swcore_register_dump
 * Description:
 *      Dump Chip peripher registers
 * Input:
 *      unit           - unit id
 * Output:
 *      N/A
 * Return:
 *      RT_ERR_OK        - OK
 *      RT_ERR_FAILED   - Failed
 * Note:
 *      None
 */
int32
drv_swcore_register_dump(uint32 unit)
{
    uint32 base_address, index, offset, reg_value;
	int ret;

	ret = 0;
	osal_printf("\nUART0 register dump:\n");
	base_address = UART0_BASE;
	for(index = 0; index < 64; index++)
	{
		offset = (index*4);
		reg_value = MEM32_READ(base_address+offset);
		osal_printf("(%08x) : %08x\n",(base_address+offset),reg_value);
	}

	osal_printf("\nUART1 register dump:\n");
	base_address = UART1_BASE;
	for(index = 0; index < 64; index++)
	{
		offset = (index*4);
		reg_value = MEM32_READ(base_address+offset);
		osal_printf("(%08x) : %08x\n",(base_address+offset),reg_value);
	}

	osal_printf("\nInterrupt register dump:\n");
	base_address = GIMR;
	for(index = 0; index < 64; index++)
	{
		offset = (index*4);
		reg_value = MEM32_READ(base_address+offset);
		osal_printf("(%08x) : %08x\n",(base_address+offset),reg_value);
	}

	osal_printf("\nTimer/Counyter register dump:\n");
	base_address = TC_BASE;
	for(index = 0; index < 64; index++)
	{
		offset = (index*4);
		reg_value = MEM32_READ(base_address+offset);
		osal_printf("(%08x) : %08x\n",(base_address+offset),reg_value);
	}

	osal_printf("\nGPIO register dump:\n");
	base_address = GPIO_BASE;
	for(index = 0; index < 64; index++)
	{
		offset = (index*4);
		reg_value = MEM32_READ(base_address+offset);
		osal_printf("(%08x) : %08x\n",(base_address+offset),reg_value);
	}

    return RT_ERR_OK;

} /* end of drv_swcore_register_dump */

#endif

#if defined(RTK_UNIVERSAL_BSP)	
/* Function Name:
 *      bsp_drv_swcore_cid_get
 * Description:
 *      Get chip id and chip revision id.
 * Input:
 *      unit           - unit id
 * Output:
 *      pChip_id       - pointer buffer of chip id
 *      pChip_rev_id   - pointer buffer of chip revision id
 * Return:
 *      RT_ERR_OK      - OK
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int
bsp_drv_swcore_cid_get(unsigned int unit, unsigned int *pCid, unsigned int *pCrevid)
{
	int ret;
	
	ret = (int)drv_swcore_cid_get((uint32)0, (uint32 *)pCid, (uint32 *)pCrevid);

	return ret;

} /* end of bsp_drv_swcore_cid_get */
#endif
