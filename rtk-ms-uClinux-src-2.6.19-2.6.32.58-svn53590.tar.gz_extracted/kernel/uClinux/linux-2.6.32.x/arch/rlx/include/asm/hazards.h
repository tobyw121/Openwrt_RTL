/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2003, 04, 07 Ralf Baechle <ralf@linux-mips.org>
 * Copyright (C) MIPS Technologies, Inc.
 *   written by Ralf Baechle <ralf@linux-mips.org>
 */
#ifndef _ASM_HAZARDS_H
#define _ASM_HAZARDS_H

#ifdef __ASSEMBLY__
#define ASMMACRO(name, code...) .macro name; code; .endm
#else

#include <asm/cpu-features.h>

#define ASMMACRO(name, code...)						\
__asm__(".macro " #name "; " #code "; .endm");				\
									\
static inline void name(void)						\
{									\
	__asm__ __volatile__ (#name);					\
}

/*
 * MIPS R2 instruction hazard barrier.   Needs to be called as a subroutine.
 */
extern void mips_ihb(void);

#endif

ASMMACRO(_ssnop,
	 sll	$0, $0, 1
	)

ASMMACRO(_ehb,
	 sll	$0, $0, 3
	)

/*
 * TLB hazards
 */
ASMMACRO(mtc0_tlbw_hazard,
	)
ASMMACRO(tlbw_use_hazard,
	)
ASMMACRO(tlb_probe_hazard,
	)
ASMMACRO(irq_enable_hazard,
	)
ASMMACRO(irq_disable_hazard,
	)
ASMMACRO(back_to_back_c0_hazard,
	)
#define instruction_hazard() do { } while (0)

#endif /* _ASM_HAZARDS_H */
