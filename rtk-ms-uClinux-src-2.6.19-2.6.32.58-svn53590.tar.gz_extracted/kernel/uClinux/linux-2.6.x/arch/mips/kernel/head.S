/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1994, 1995 Waldorf Electronics
 * Written by Ralf Baechle and Andreas Busse
 * Copyright (C) 1994 - 99, 2003, 06 Ralf Baechle
 * Copyright (C) 1996 Paul M. Antoine
 * Modified for DECStation and hence R3000 support by Paul M. Antoine
 * Further modifications by David S. Miller and Harald Koerfgen
 * Copyright (C) 1999 Silicon Graphics, Inc.
 * Kevin Kissell, kevink@mips.com and Carsten Langgaard, carstenl@mips.com
 * Copyright (C) 2000 MIPS Technologies, Inc.  All rights reserved.
 */
#include <linux/init.h>
#include <linux/threads.h>

#include <asm/asm.h>
#include <asm/asmmacro.h>
#include <asm/irqflags.h>
#include <asm/regdef.h>
#include <asm/page.h>
#include <asm/mipsregs.h>
#include <asm/stackframe.h>

#include <kernel-entry-init.h>

	.macro	ARC64_TWIDDLE_PC
#if defined(CONFIG_ARC64) || defined(CONFIG_MAPPED_KERNEL)
	/* We get launched at a XKPHYS address but the kernel is linked to
	   run at a KSEG0 address, so jump there.  */
	PTR_LA	t0, \@f
	jr	t0
\@:
#endif
	.endm

	/*
	 * inputs are the text nasid in t1, data nasid in t2.
	 */
	.macro MAPPED_KERNEL_SETUP_TLB
#ifdef CONFIG_MAPPED_KERNEL
	/*
	 * This needs to read the nasid - assume 0 for now.
	 * Drop in 0xffffffffc0000000 in tlbhi, 0+VG in tlblo_0,
	 * 0+DVG in tlblo_1.
	 */
	dli	t0, 0xffffffffc0000000
	dmtc0	t0, CP0_ENTRYHI
	li	t0, 0x1c000		# Offset of text into node memory
	dsll	t1, NASID_SHFT		# Shift text nasid into place
	dsll	t2, NASID_SHFT		# Same for data nasid
	or	t1, t1, t0		# Physical load address of kernel text
	or	t2, t2, t0		# Physical load address of kernel data
	dsrl	t1, 12			# 4K pfn
	dsrl	t2, 12			# 4K pfn
	dsll	t1, 6			# Get pfn into place
	dsll	t2, 6			# Get pfn into place
	li	t0, ((_PAGE_GLOBAL|_PAGE_VALID| _CACHE_CACHABLE_COW) >> 6)
	or	t0, t0, t1
	mtc0	t0, CP0_ENTRYLO0	# physaddr, VG, cach exlwr
	li	t0, ((_PAGE_GLOBAL|_PAGE_VALID| _PAGE_DIRTY|_CACHE_CACHABLE_COW) >> 6)
	or	t0, t0, t2
	mtc0	t0, CP0_ENTRYLO1	# physaddr, DVG, cach exlwr
	li	t0, 0x1ffe000		# MAPPED_KERN_TLBMASK, TLBPGMASK_16M
	mtc0	t0, CP0_PAGEMASK
	li	t0, 0			# KMAP_INX
	mtc0	t0, CP0_INDEX
	li	t0, 1
	mtc0	t0, CP0_WIRED
	tlbwi
#else
	mtc0	zero, CP0_WIRED
#endif
	.endm

	/*
	 * For the moment disable interrupts, mark the kernel mode and
	 * set ST0_KX so that the CPU does not spit fire when using
	 * 64-bit addresses.  A full initialization of the CPU's status
	 * register is done later in per_cpu_trap_init().
	 */
	.macro	setup_c0_status set clr
	.set	push
#ifdef CONFIG_MIPS_MT_SMTC
	/*
	 * For SMTC, we need to set privilege and disable interrupts only for
	 * the current TC, using the TCStatus register.
	 */
	mfc0	t0, CP0_TCSTATUS
	/* Fortunately CU 0 is in the same place in both registers */
	/* Set TCU0, TMX, TKSU (for later inversion) and IXMT */
	li	t1, ST0_CU0 | 0x08001c00
	or	t0, t1
	/* Clear TKSU, leave IXMT */
	xori	t0, 0x00001800
	mtc0	t0, CP0_TCSTATUS
	_ehb
	/* We need to leave the global IE bit set, but clear EXL...*/
	mfc0	t0, CP0_STATUS
	or	t0, ST0_CU0 | ST0_EXL | ST0_ERL | \set | \clr
	xor	t0, ST0_EXL | ST0_ERL | \clr
	mtc0	t0, CP0_STATUS
#else
	mfc0	t0, CP0_STATUS
	or	t0, ST0_CU0|\set|0x1f|\clr
	xor	t0, 0x1f|\clr
	mtc0	t0, CP0_STATUS
	.set	noreorder
	sll	zero,3				# ehb
#endif
	.set	pop
	.endm

	.macro	setup_c0_status_pri
#ifdef CONFIG_64BIT
	setup_c0_status ST0_KX 0
#else
	setup_c0_status 0 0
#endif
	.endm

	.macro	setup_c0_status_sec
#ifdef CONFIG_64BIT
	setup_c0_status ST0_KX ST0_BEV
#else
	setup_c0_status 0 ST0_BEV
#endif
	.endm

	/*
	 * Reserved space for exception handlers.
	 * Necessary for machines which link their kernels at KSEG0.
	 */
#ifdef CONFIG_AUTO_JUMP_KERNEL_ENTRY
	jal	kernel_entry
	.balign	0x400
#else
	.fill	0x400
#endif

EXPORT(stext)					# used for profiling
EXPORT(_stext)

#if defined(CONFIG_QEMU) || defined(CONFIG_MIPS_SIM)
	/*
	 * Give us a fighting chance of running if execution beings at the
	 * kernel load address.  This is needed because this platform does
	 * not have a ELF loader yet.
	 */
	j	kernel_entry
#endif
	__INIT

NESTED(kernel_entry, 16, sp)			# kernel entry point

	kernel_entry_setup			# cpu specific setup

	setup_c0_status_pri

	ARC64_TWIDDLE_PC

#ifdef CONFIG_MIPS_MT_SMTC
	/*
	 * In SMTC kernel, "CLI" is thread-specific, in TCStatus.
	 * We still need to enable interrupts globally in Status,
	 * and clear EXL/ERL.
	 *
	 * TCContext is used to track interrupt levels under
	 * service in SMTC kernel. Clear for boot TC before
	 * allowing any interrupts.
	 */
	mtc0	zero, CP0_TCCONTEXT

	mfc0	t0, CP0_STATUS
	ori	t0, t0, 0xff1f
	xori	t0, t0, 0x001e
	mtc0	t0, CP0_STATUS
#endif /* CONFIG_MIPS_MT_SMTC */

.set nomips16

#ifdef CONFIG_KERNEL_DECRY
//  .comm key, 24
//  .comm iv, 8
//  .comm prepsrc, 48
//  .comm prepdst, 32
#define SRC_DESCRIPTOR 0xA1a00000
#define DST_DESCRIPTOR 0xA1a20000
#define KEY_ARRAY  0xA1a40000
#define IV_ARRAY   0xA1a50000
#define EFSSR 0xb8003A00
#define EFKEYR0 (EFSSR+0x04)
#define EFKEYR1 (EFSSR+0x08)
#define EFKEYR2 (EFSSR+0x0C)
#define EFKEYR3 (EFSSR+0x10)
#define EFKEYR4 (EFSSR+0x14)
#define EFKEYR5 (EFSSR+0x18)
#define	SRAMSAR0	0xB8004000
#define	SRAMSSR0	0xB8004004
#define	SRAMSBR0	0xB8004008
#define UMSAR0 (0xB8001300)
#define UMSSR0 (UMSAR0+0x4)

#define TMP_ADDR 0xA1160000

/*copy .imem code to TMP_ADDR, since the code is for SRAM*/
/* t0: src
    t1:dst
    t4:src bottom */
la 	t0, __imem_start
	nop
la 	t1, __imem_end
	nop
li	t2, TMP_ADDR
1:
	lw	t3, 0(t0)
	addiu t0, 4
	sw	t3, 0(t2)
	addiu t2, 4
	bne t0, t1, 1b
	nop
		
	la 	t0, __imem_start
	nop
	la 	t1, __imem_end
	nop
  	sub t1, t0 /*t1 len*/

  	beqz		t1, 1f
	nop
	srl	t1, 7
	
	li	t2, 0
2:	addi	t2, 1
	srl	t1, 1
	bgtz t1, 2b
	nop
	
	sub	t2,1
	li	t1, SRAMSBR0
	li	t3, 0
	sw	t3, 0(t1)

	li	t1, SRAMSSR0
	sw	t2, 0(t1)
	li	t1, UMSSR0
	sw	t2, 0(t1)
	
	li	t1, SRAMSAR0
	and	 t0, 0x1FFFFFFF
	or	t0, t0, 0x1
	sw	t0, 0(t1)
	li	t1, UMSAR0
	sw	t0, 0(t1)
	


#if 1
/*enable IPSEC */
1: li	t0, 0xb8003a00
  li t3, 0x1100
  sw t3, 0(t0)
 #endif
/* Software Reset */

  li t0, 0xb800c008
  lw t1, 0(t0)
  or t1, t1, 0x1
  sw t1, 0(t0)
  
/* We must delay a while after software reset. */
	li	t0, 5000
1:	sub	t0, 1
	bgez	t0, 1b
nop

  li t0, 0xb800c00c
  li t1, 0x40
  sw t1, 0(t0)

	
/*	WRITE_MEM32( IPSCSR, READ_MEM32(IPSCSR)|IPS_SDUEIP|IPS_SDLEIP|IPS_DDUEIP|IPS_DDOKIP );  write 1 to clear */
  li t0, 0xb800c008
  lw t1, 0(t0)
  or t1, t1, 0x0000f000
  sw t1, 0(t0)
  
	 li	t3, 10
1:	sub	t3, 1
	bgez	t3, 1b
	nop

	/* write prepared descriptors into ASIC , if in flash, write dst addr in sdram first, then src addr*/
	
/*init prepsrc, prepdst, key, iv, data*/
	 li t1, SRC_DESCRIPTOR
	 li  t2, 0
	 li  t3, 12	
1: sw t2, 0(t1)
 	 addi	t1, 4
	 sub t3,1
	 bgez t3, 1b
	 nop
  
   li t1, DST_DESCRIPTOR
    li t2, 0
    li t3, 8
 1:	sw t2, 0(t1)
	  addi	t1, 4
	  sub t3,1
	  bgez t3, 1b
	  nop
	
	
	 li  t1, KEY_ARRAY
	 li  t2, 0
	 li  t3, 6
1: sw t2, 0(t1)
   addi	t1, 4
   sub  t3, 1
   bgez t3, 1b
   nop

   li t1, IV_ARRAY
   li t2, 0
   li t3, 2
1: sw t2, 0(t1)
   addi t1, 4
   sub  t3, 1
   bgez t3, 1b
   nop

#if 1
	li	t1, EFKEYR0
	li	t2, 0x89abcdef
	sw	t2, 0(t1)
	li	t1, EFKEYR1
	li	t2, 0x1234567
	sw	t2, 0(t1)
	li	t1, EFKEYR2
	li	t2, 0x76543210
	sw	t2, 0(t1)
	li	t1, EFKEYR3
	li	t2, 0xfedcba98
	sw	t2, 0(t1)
	li	t1, EFKEYR4
	li	t2, 0x89abcdef
	sw	t2, 0(t1)
	li	t1, EFKEYR5
	li	t2, 0x1234567
	sw	t2, 0(t1)

#endif
  	
        li 		t2, 0xb800c000
	 li		 t3, SRC_DESCRIPTOR
	 and		 t3, 0x1FFFFFFF
	 sw 		t3, 0(t2)

	 li t3, DST_DESCRIPTOR
 	 and		 t3,	 0x1FFFFFFF
	 sw 		 t3, 4(t2)
/* if in flash, calc len*/
	la 	t0, __imem_start
	nop
	nop
	la 	t4, __imem_end
	nop
	nop
  	sub t4, t0 /*t4 len*/
  	and	t4, 0xFFFFFFF8
/*  	addi t4, 0x8*/
	
/*	t0: dst addr
	t5: src addr
	t4: len
	t1:16368
*/
	li    t1, 16368 
/*	la 	t5, __imem_start // __crypto_start*/
/*	or	t5, t5, 0xa0000000*/
	li	t5, TMP_ADDR
	
	ble t1, t4, 1f
	nop
	ble	t4, t1, 4f
	nop
	beqz		t4, 5f
 
1:  	  li t2, SRC_DESCRIPTOR

        /*key descriptor*/
        /*  0xa2900018: 3des-ebc-encrypto
 	 *  0xa1100018: 3des-ebc-decrypto
	 *  0xa2980018: 3des-cbc-encrypto
	 *  0xa1180018: 3des-cbc-decrypto 
	 *  0xa3800008:  des-ebc-encrypto
	 *  0xa0000008:  des-ebc-decrypto
	 *  0xa3880008:  des-cbc-encrypto
	 *  0xa0080008:  des-cbc-decrypto*/
	  li	t3, 0xa1100018
	  sw    t3, 0(t2)
  	  sw    t1, 4(t2)
  	  li 	 t3, KEY_ARRAY
	  and 	t3, 0x1FFFFFFF
	  sw 	t3, 12(t2)
  
	  /*iv descriptor*/
	  li 		t3, 0x81000008
	  sw 	t3, 16(t2)
	  sw 	t1, 20(t2)
	  li t3, IV_ARRAY
	  and	 t3, 0x1FFFFFFF
	  sw 	t3, 28(t2)
  
	  /*data descriptor*/
	  li 		t3, 0xc1000000
	  or 		t3, t3, t1
	  sw 	t3, 32(t2)
	  sw 	t1, 36(t2)
	  and	 t5, 0x1FFFFFFF
	  sw 	 t5, 44(t2)

 	 /* if in flash, prepare dst prep */
	  li 		t2, DST_DESCRIPTOR
	  li		 t3, 0xc0000000
	  or		 t3, t3, t1
	  sw 	t3, 0(t2)
	  and 	t0, 0x1FFFFFFF
	  sw 	t0, 4(t2)
  

	/* Clear OK flag and kick off engine*/

	 li 		t2, 0xb800c008
	 lw		 t3, 0(t2)
	 nop
	 nop
	 or 		t3, t3, 0x0000fffe
	 sw		 t3, 0(t2)

	nop
3:	lw	t3, 0(t2)
	li	t6, 0x1000
	and	t3, t3, t6
	beqz t3, 3b
	nop


	 sub 	t4, t1
	 add 	t5, 16368
	 add		t0, 16368
	 nop
	 ble		t1, t4, 1b
	 nop

 4: 	  li t2, SRC_DESCRIPTOR
        /*key descriptor*/
  	  li 	   t3, 0xa1100018  /*decrypto*/
	  sw    t3, 0(t2)
  	  sw    t4, 4(t2)
  	  li 	 t3, KEY_ARRAY
 	  and 	t3, 0x1FFFFFFF
	  sw 	t3, 12(t2)
  
	  /*iv descriptor*/
	  li 		t3, 0x81000008
	  sw 	t3, 16(t2)
	  sw 	t4, 20(t2)
	  li t3, IV_ARRAY
	  and	 t3, 0x1FFFFFFF
	  sw 	t3, 28(t2)
  
	  /*data descriptor*/
	  li 		t3, 0xc1000000
	  or 		t3, t3, t4
	  sw 	t3, 32(t2)
	  sw 	t4, 36(t2)
	  and	 t5, 0x1FFFFFFF
	  sw 	 t5, 44(t2)

 	 /* if in flash, prepare dst prep */
	  li 		t2, DST_DESCRIPTOR
	  li		 t3, 0xc0000000
	  or		 t3, t3, t4
	  sw 	t3, 0(t2)
	  and 	t0, 0x1FFFFFFF
	  sw 	t0, 4(t2)
  

/* li 		t2, 0xb800c000
	 li		 t3, SRC_DESCRIPTOR
	 and		 t3, 0x1FFFFFFF
	 sw 		t3, 0(t2)

	 li t3, DST_DESCRIPTOR
	 
	 and		 t3,	 0x1FFFFFFF
	 sw 		 t3, 4(t2)
*/
	/* Clear OK flag and kick off engine*/

	 li 		t2, 0xb800c008
	 lw		 t3, 0(t2)
	 nop
	 nop
	 or 		t3, t3, 0x0000fffe
	 sw		 t3, 0(t2)

	nop
3:	lw	t3, 0(t2)
	li	t6, 0x1000
	and	t3, t3, t6
	beqz t3, 3b
	nop

#endif


5:	PTR_LA		t0, __bss_start		# clear .bss
	LONG_S		zero, (t0)
	PTR_LA		t1, __bss_stop - LONGSIZE
1:
	PTR_ADDIU	t0, LONGSIZE
	LONG_S		zero, (t0)
	bne		t0, t1, 1b

	LONG_S		a0, fw_arg0		# firmware arguments
	LONG_S		a1, fw_arg1
	LONG_S		a2, fw_arg2
	LONG_S		a3, fw_arg3

	MTC0		zero, CP0_CONTEXT	# clear context register
	PTR_LA		$28, init_thread_union
	PTR_LI		sp, _THREAD_SIZE - 32
	PTR_ADDU	sp, $28
	set_saved_sp	sp, t0, t1
	PTR_SUBU	sp, 4 * SZREG		# init stack pointer

	j		start_kernel
	END(kernel_entry)

#ifdef CONFIG_QEMU
	__INIT
#endif

#ifdef CONFIG_SMP
/*
 * SMP slave cpus entry point.  Board specific code for bootstrap calls this
 * function after setting up the stack and gp registers.
 */
NESTED(smp_bootstrap, 16, sp)
#ifdef CONFIG_MIPS_MT_SMTC
	/*
	 * Read-modify-writes of Status must be atomic, and this
	 * is one case where CLI is invoked without EXL being
	 * necessarily set. The CLI and setup_c0_status will
	 * in fact be redundant for all but the first TC of
	 * each VPE being booted.
	 */
	DMT	10	# dmt t2 /* t0, t1 are used by CLI and setup_c0_status() */
	jal	mips_ihb
#endif /* CONFIG_MIPS_MT_SMTC */
	setup_c0_status_sec
	smp_slave_setup
#ifdef CONFIG_MIPS_MT_SMTC
	andi	t2, t2, VPECONTROL_TE
	beqz	t2, 2f
	EMT		# emt
2:
#endif /* CONFIG_MIPS_MT_SMTC */
	j	start_secondary
	END(smp_bootstrap)
#endif /* CONFIG_SMP */

	__FINIT

	.comm	kernelsp,    NR_CPUS * 8, 8
	.comm	pgd_current, NR_CPUS * 8, 8

	.comm	fw_arg0, SZREG, SZREG		# firmware arguments
	.comm	fw_arg1, SZREG, SZREG
	.comm	fw_arg2, SZREG, SZREG
	.comm	fw_arg3, SZREG, SZREG

	.macro page name, order
	.comm	\name, (_PAGE_SIZE << \order), (_PAGE_SIZE << \order)
	.endm

	/*
	 * On 64-bit we've got three-level pagetables with a slightly
	 * different layout ...
	 */
	page	swapper_pg_dir, _PGD_ORDER
#ifdef CONFIG_64BIT
	page	invalid_pmd_table, _PMD_ORDER
#endif
	page	invalid_pte_table, _PTE_ORDER
