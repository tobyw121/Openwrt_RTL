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
 * $Revision: 51953 $
 * $Date: 2014-10-07 11:28:04 +0800 (Tue, 07 Oct 2014) $
 *
 * Purpose : IOAL Layer Init Module
 *
 * Feature : IOAL Init Functions
 *
 */

/*
 * Include Files
 */
#include <osal/memory.h>
#include <ioal/ioal_init.h>
#include <drv/swcore/chip.h>

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */
uint32 swcore_base[RTK_MAX_NUM_OF_UNIT];
uint32 soc_base[RTK_MAX_NUM_OF_UNIT];
uint32 sram_base[RTK_MAX_NUM_OF_UNIT];
uint32 dma_base[RTK_MAX_NUM_OF_UNIT];
uint32 l2Notify_dma_base[RTK_MAX_NUM_OF_UNIT];
uint32 desc_base[RTK_MAX_NUM_OF_UNIT];

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

        case IOAL_MEM_SOC:
            *pBaseAddr = soc_base[unit];
            break;

        case IOAL_MEM_SRAM:
            *pBaseAddr = sram_base[unit];
            break;

        case IOAL_MEM_DMA:
            *pBaseAddr = dma_base[unit];
            break;

        case IOAL_MEM_L2NOTIFY_DMA:
            *pBaseAddr = l2Notify_dma_base[unit];
            break;

        case IOAL_MEM_DESC:
            *pBaseAddr = desc_base[unit];
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
    uint32  chip_id, rev_id;
    int32   ret = RT_ERR_FAILED;
    void    *pUserReg;

    pUserReg = osal_mmap(MEM_DEV_NAME, SWCORE_PHYS_BASE, SWCORE_MEM_SIZE);
    if(RT_ERR_FAILED == (uint32)pUserReg)
        return RT_ERR_FAILED;
    swcore_base[unit] = (uint32)pUserReg;

    pUserReg = osal_mmap(MEM_DEV_NAME, SOC_PHYS_BASE, SOC_MEM_SIZE);
    if(RT_ERR_FAILED == (uint32)pUserReg)
        return RT_ERR_FAILED;
    soc_base[unit] = (uint32)pUserReg;

    if ((ret = drv_swcore_cid_get(unit, &chip_id, &rev_id)) != RT_ERR_OK)
    {
        return RT_ERR_FAILED;
    }

    if ((RTL8389M_CHIP_ID == chip_id) || (RTL8329M_CHIP_ID == chip_id) ||
        (RTL8377M_CHIP_ID == chip_id) || (RTL8389L_CHIP_ID == chip_id))
    {
        pUserReg = osal_mmap(MEM_DEV_NAME, RTL8389_SRAM_PHYS_BASE, SRAM_MEM_SIZE);
    }
    else if ((RTL8328M_CHIP_ID == chip_id) || (RTL8328S_CHIP_ID == chip_id) ||
             (RTL8328L_CHIP_ID == chip_id))
    {
        pUserReg = osal_mmap(MEM_DEV_NAME, RTL8328_SRAM_PHYS_BASE, SRAM_MEM_SIZE);
    }
    else 
    {
        pUserReg = osal_mmap(MEM_DEV_NAME, RTL8380_SRAM_PHYS_BASE, SRAM_MEM_SIZE_128M);
    }

    if(RT_ERR_FAILED == (uint32)pUserReg)
        return RT_ERR_FAILED;
    sram_base[unit] = (uint32)pUserReg;

    if ((RTL8389M_CHIP_ID == chip_id) || (RTL8329M_CHIP_ID == chip_id) ||
        (RTL8377M_CHIP_ID == chip_id) || (RTL8389L_CHIP_ID == chip_id))
    {
        pUserReg = osal_mmap(MEM_DEV_NAME, RTL8389_DMA_PHYS_BASE, RTL8389_DMA_MEM_SIZE);
    }
    else if ((RTL8328M_CHIP_ID == chip_id) || (RTL8328S_CHIP_ID == chip_id) ||
             (RTL8328L_CHIP_ID == chip_id))
    {
        pUserReg = osal_mmap(MEM_DEV_NAME, RTL8328_DMA_PHYS_BASE, RTL8328_DMA_MEM_SIZE);
    }
    else if (CHIP_FAMILY_IS_RTL8330(chip_id) ||
             CHIP_FAMILY_IS_RTL8380(chip_id))
    {
        pUserReg = osal_mmap(MEM_DEV_NAME, RTL8380_DMA_PHYS_BASE, RTL8380_DMA_MEM_SIZE);
    }
    else if (CHIP_FAMILY_IS_RTL8350(chip_id) ||
             CHIP_FAMILY_IS_RTL8390(chip_id))
    {
        pUserReg = osal_mmap(MEM_DEV_NAME, RTL8390_DMA_PHYS_BASE, RTL8390_DMA_MEM_SIZE);
    }        
    else
    {
        pUserReg = osal_mmap(MEM_DEV_NAME, RTL8380_DMA_PHYS_BASE, RTL8380_DMA_MEM_SIZE);
    }
    if(RT_ERR_FAILED == (uint32)pUserReg)
        return RT_ERR_FAILED;
    dma_base[unit] = (uint32)pUserReg;


    if (CHIP_FAMILY_IS_RTL8350(chip_id) ||
        CHIP_FAMILY_IS_RTL8390(chip_id))
    {
        pUserReg = osal_mmap(MEM_DEV_NAME, RTL8390_L2NOTIFY_PHYS_BASE, RTL8390_L2NOTIFY_MEM_SIZE);
        l2Notify_dma_base[unit] = (uint32)pUserReg;
    }


    if (CHIP_FAMILY_IS_RTL8389(chip_id))
    {
        pUserReg = osal_mmap(MEM_DEV_NAME, RTL8389_DESC_PHYS_BASE, RTL8389_DESC_MEM_SIZE);
    }
    else if (CHIP_FAMILY_IS_RTL8328(chip_id))
    {
        pUserReg = osal_mmap(MEM_DEV_NAME, RTL8328_DESC_PHYS_BASE, RTL8328_DESC_MEM_SIZE);
    }
    else if (CHIP_FAMILY_IS_RTL8330(chip_id) ||
             CHIP_FAMILY_IS_RTL8380(chip_id))
    {
        pUserReg = osal_mmap(MEM_DEV_NAME, RTL8380_DESC_PHYS_BASE, RTL8380_DESC_MEM_SIZE);
    }
    else if (CHIP_FAMILY_IS_RTL8350(chip_id) ||
             CHIP_FAMILY_IS_RTL8390(chip_id))
    {
        pUserReg = osal_mmap(MEM_DEV_NAME, RTL8390_DESC_PHYS_BASE, RTL8390_DESC_MEM_SIZE);
    }        
    else
    {
        pUserReg = osal_mmap(MEM_DEV_NAME, RTL8380_DESC_PHYS_BASE, RTL8380_DESC_MEM_SIZE);
    }
    if(RT_ERR_FAILED == (uint32)pUserReg)
        return RT_ERR_FAILED;
    desc_base[unit] = (uint32)pUserReg;

    return ((void *)RT_ERR_FAILED != pUserReg) ? RT_ERR_OK : RT_ERR_FAILED;   
} /* end of ioal_init */

