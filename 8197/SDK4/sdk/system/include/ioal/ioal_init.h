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
 * $Revision: 55230 $
 * $Date: 2015-01-26 21:56:17 +0800 (Mon, 26 Jan 2015) $
 *
 * Purpose : IOAL Layer Init Module
 *
 * Feature : IOAL Init Functions
 *
 */

/*
 * Include Files
 */
#include <common/rt_autoconf.h>
#include <common/error.h>
#include <soc/type.h>

/*
 * Symbol Definition
 */
#define SWCORE_VIRT_BASE    0xBB000000
#define SOC_VIRT_BASE       0xB8000000
#define RTL8389_SRAM_VIRT_BASE      0xB0000000
#define RTL8328_SRAM_VIRT_BASE      0xB2000000
#define RTL8380_SRAM_VIRT_BASE      0xBF000000
#define RTL8390_SRAM_VIRT_BASE      0xBF000000

#define SWCORE_MEM_SIZE             0x2000000
#define SOC_MEM_SIZE                0x5000
#define SRAM_MEM_SIZE               0x8000
#define SRAM_MEM_SIZE_128M          0x20000

#define RTL8389_DESC_MEM_SIZE       0x100000
#define RTL8389_DMA_MEM_SIZE        0x100000
#define RTL8328_DESC_MEM_SIZE       0x100000
#define RTL8328_DMA_MEM_SIZE        0x100000
#define RTL8380_PYHS_MEM_SIZE       0x08000000
#define RTL8380_DESC_MEM_SIZE       0x100000
#define RTL8380_DMA_MEM_SIZE        0x100000
#define RTL8390_PYHS_MEM_SIZE       0x08000000
#define RTL8390_DESC_MEM_SIZE       0x80000
#define RTL8390_DMA_MEM_SIZE        0x100000
#if defined(CONFIG_SDK_DRIVER_L2NTFY)
#define RTL8390_L2NOTIFY_MEM_SIZE   0x80000
#else
#define RTL8390_L2NOTIFY_MEM_SIZE   0
#endif

#define SWCORE_PHYS_BASE            0x1B000000
#define SOC_PHYS_BASE               0x18000000
#define RTL8389_SRAM_PHYS_BASE      0x10000000
#define RTL8389_DMA_PHYS_BASE       0x01F00000 
#define RTL8389_DESC_PHYS_BASE      0x01D00000 
#define RTL8328_SRAM_PHYS_BASE      0x12000000
#define RTL8328_DMA_PHYS_BASE       0x03F00000 
#define RTL8328_DESC_PHYS_BASE      0x03D00000 
#define RTL8380_SRAM_PHYS_BASE      0x1F000000
#define RTL8380_DMA_PHYS_BASE       0x07F00000 
#define RTL8380_DESC_PHYS_BASE      0x07E00000 
#define RTL8390_SRAM_PHYS_BASE      0x1F000000
#define RTL8390_DESC_PHYS_BASE      (RTL8390_PYHS_MEM_SIZE - RTL8390_L2NOTIFY_MEM_SIZE - RTL8390_DMA_MEM_SIZE - RTL8390_DESC_MEM_SIZE)
#define RTL8390_DMA_PHYS_BASE       (RTL8390_PYHS_MEM_SIZE - RTL8390_L2NOTIFY_MEM_SIZE - RTL8390_DMA_MEM_SIZE)
#define RTL8390_L2NOTIFY_PHYS_BASE  (RTL8390_PYHS_MEM_SIZE - RTL8390_L2NOTIFY_MEM_SIZE)

#if defined(CONFIG_SDK_KERNEL_LINUX_KERNEL_MODE)
#define RTL8390_L2NOTIFY_RING_SIZE  400
#else
#define RTL8390_L2NOTIFY_RING_SIZE  1700
#endif


typedef enum ioal_memRegion_e
{
    IOAL_MEM_SWCORE = 0,
    IOAL_MEM_SOC,
    IOAL_MEM_SRAM,
    IOAL_MEM_DMA,
    IOAL_MEM_L2NOTIFY_DMA,
    IOAL_MEM_DESC,
    IOAL_MEM_END
} ioal_memRegion_t;

/*
 * Data Declaration
 */
#if defined(CONFIG_SDK_MODEL_MODE)  
    #if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
extern int8 *pVirtualSWTable;
  #endif /*end of #if defined(CONFIG_SDK_RTL8390)*/
#endif /*end of #if defined(CONFIG_SDK_MODEL_MODE)*/

extern uint32  ioal_chipId_8380_8330_flag;

/*
 * Macro Declaration
 */
#define SOC_MMAP_BASE(unit) (soc_base[unit])

#define SOC32_READ(unit, addr, val) \
do{ \
    val = MEM32_READ(SOC_MMAP_BASE(unit)+addr); \
}while(0)

#define SOC32_WRITE(unit, addr, val) \
do{ \
    MEM32_WRITE(SOC_MMAP_BASE(unit)+addr, val); \
}while(0)

#define SOC32_FIELD_READ(unit, addr, offset, mask, val) \
do{ \
    val = (MEM32_READ(SOC_MMAP_BASE(unit)+addr) & mask) >> offset; \
}while(0)

#define SOC32_FIELD_WRITE(unit, addr, offset, mask, val) \
do{ \
    MEM32_WRITE(SOC_MMAP_BASE(unit)+addr, (MEM32_READ(SOC_MMAP_BASE(unit)+addr) & ~mask) | (val << offset)); \
}while(0)


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
extern int32
ioal_init_memRegion_get(uint32 unit, ioal_memRegion_t mem, uint32 *pBaseAddr);

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
extern int32
ioal_init(uint32 unit);

