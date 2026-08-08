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
 * $Revision: 13326 $
 * $Date: 2010-10-13 18:31:26 +0800 (Wed, 13 Oct 2010) $
 *
 * Purpose : SoC register definition
 *
 * Feature : Define the DRAM/SRAM registers
 *
 */

#ifndef __MEMORY_REG_H__
#define __MEMORY_REG_H__

#include <soc/var/var_memory_reg.h>

#ifndef DRAM_REG_DEFINIED
#define DRAM_REG_DEFINIED

/*
 * Memory Controller - DRAM
 */
#define MC_MCR_BASE     (0xB8001000)
#define MC_MCR          (MC_MCR_BASE+0x000)
#define MC_DCR          (MC_MCR_BASE+0x004)
#define MC_DTCR         (MC_MCR_BASE+0x008)
#define MC_GIAR0        (MC_MCR_BASE+0x010)
#define MC_GIAR1        (MC_MCR_BASE+0x014)
#define MC_LXIAR0       (MC_MCR_BASE+0x018)
#define MC_LXIAR1       (MC_MCR_BASE+0x01C)
#define MC_LXIAR2       (MC_MCR_BASE+0x020)
#define MC_LXIAR3       (MC_MCR_BASE+0x024)
#define MC_MSRR         (MC_MCR_BASE+0x030)
#define MC_MPMR         (MC_MCR_BASE+0x040)
#define MC_DDCR         (MC_MCR_BASE+0x050)

#endif /* end of DRAM_REG_DEFINIED */

#ifndef SRAM_REG_DEFINIED
#define SRAM_REG_DEFINIED

/*
 * SRAM related registers
 */

#define UMSR_BASE       (0xB8001300)
#define UMSAR0          (UMSR_BASE+0x00)
#define UMSAR1          (UMSR_BASE+0x10)
#define UMSAR2          (UMSR_BASE+0x20)
#define UMSAR3          (UMSR_BASE+0x30)
#define UMSSR0          (UMSR_BASE+0x04)
#define UMSSR1          (UMSR_BASE+0x14)
#define UMSSR2          (UMSR_BASE+0x24)
#define UMSSR3          (UMSR_BASE+0x34)

#define SRAMSR_BASE     (0xB8004000)
#define SRAMSAR0        (SRAMSR_BASE+0x00)
#define SRAMSAR1        (SRAMSR_BASE+0x10)
#define SRAMSAR2        (SRAMSR_BASE+0x20)
#define SRAMSAR3        (SRAMSR_BASE+0x30)
#define SRAMSSR0        (SRAMSR_BASE+0x04)
#define SRAMSSR1        (SRAMSR_BASE+0x14)
#define SRAMSSR2        (SRAMSR_BASE+0x24)
#define SRAMSSR3        (SRAMSR_BASE+0x34)
#define SRAMSBR0        (SRAMSR_BASE+0x08)
#define SRAMSBR1        (SRAMSR_BASE+0x18)
#define SRAMSBR2        (SRAMSR_BASE+0x28)
#define SRAMSBR3        (SRAMSR_BASE+0x38)

#define UMSR_OFFSET     (0x1300)
#define UMSAR0_OFFSET   (UMSR_OFFSET+0x00)
#define UMSAR1_OFFSET   (UMSR_OFFSET+0x10)
#define UMSAR2_OFFSET   (UMSR_OFFSET+0x20)
#define UMSAR3_OFFSET   (UMSR_OFFSET+0x30)
#define UMSSR0_OFFSET   (UMSR_OFFSET+0x04)
#define UMSSR1_OFFSET   (UMSR_OFFSET+0x14)
#define UMSSR2_OFFSET   (UMSR_OFFSET+0x24)
#define UMSSR3_OFFSET   (UMSR_OFFSET+0x34)

#define SRAMSR_OFFSET   (0x4000)
#define SRAMSAR0_OFFSET (SRAMSR_OFFSET+0x00)
#define SRAMSAR1_OFFSET (SRAMSR_OFFSET+0x10)
#define SRAMSAR2_OFFSET (SRAMSR_OFFSET+0x20)
#define SRAMSAR3_OFFSET (SRAMSR_OFFSET+0x30)
#define SRAMSSR0_OFFSET (SRAMSR_OFFSET+0x04)
#define SRAMSSR1_OFFSET (SRAMSR_OFFSET+0x14)
#define SRAMSSR2_OFFSET (SRAMSR_OFFSET+0x24)
#define SRAMSSR3_OFFSET (SRAMSR_OFFSET+0x34)
#define SRAMSBR0_OFFSET (SRAMSR_OFFSET+0x08)
#define SRAMSBR1_OFFSET (SRAMSR_OFFSET+0x18)
#define SRAMSBR2_OFFSET (SRAMSR_OFFSET+0x28)
#define SRAMSBR3_OFFSET (SRAMSR_OFFSET+0x38)

#endif /* end of SRAM_REG_DEFINIED */

#endif /* end of __MEMORY_REG_H__ */

