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
 * $Revision: 37036 $
 * $Date: 2013-02-20 15:08:47 +0800 (Wed, 20 Feb 2013) $
 *
 * Purpose : IOAL Layer Init Module
 *
 * Feature : IOAL Init Functions
 *
 */

/*
 * Include Files
 */
#include <ioal/ioal_init.h>

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */
uint32 swcore_base[RTK_MAX_NUM_OF_UNIT];

uint32  ioal_chipId_8380_8330_flag = 0;

/*
 * Macro Declaration
 */


/*
 * Function Declaration
 */

/* Function Name:
 *      ioal_init_memBase_get
 * Description:
 *      Get memory base address
 * Input:
 *      unit      - unit id
 *      mem       - memory region
 * Output:
 *      pBaseAddr - pointer to the base address of memory region
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
ioal_init_memRegion_get(uint32 unit, ioal_memRegion_t mem, uint32 *pBaseAddr)
{
    switch(mem)
    {
        case IOAL_MEM_SWCORE:
            *pBaseAddr = swcore_base[unit];
            break;      

        default:
            return RT_ERR_FAILED;
    }
   
    return RT_ERR_OK;
} /* end of ioal_init_memBase_get */

/* Function Name:
 *      ioal_init
 * Description:
 *      Init SDK IOAL Layer
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
ioal_init(uint32 unit)
{
    uint32 model_info;
    
    swcore_base[unit] = SWCORE_VIRT_BASE;

    /* common IOAL init procedure */
    model_info = MEM32_READ(0xBB0000D4);

    ioal_chipId_8380_8330_flag = 0;
    model_info &= 0xFFFF0000UL;
    model_info = model_info>>16;
    if((model_info == 0x8380) || (model_info == 0x8330))
    {
        ioal_chipId_8380_8330_flag = 1;
    }
    

    return RT_ERR_OK;
} /* end of ioal_init */

