/*
 * Realtek Semiconductor Corp.
 *
 * mips-cm.S:
 *	MIPS coherence manager setup
 *
 * Copyright 2006-2015  PSP Software Group
 */
//#include <config.h>

#include <asm/gcmpregs.h>
#include <asm/smp-boot.h>
#include <asm/mipsregs.h>
#include <asm/regdef.h>

	.text

	.globl mips_cm_setup
	.ent mips_cm_setup
	.set push
	.set noreorder
mips_cm_setup:
	/* Rewrite the GCMPB register to initialise CM_DEFAULT_TARGET */
#define	GCMP	t0
	la	GCMP, GCMP_BASE

	/* Check the CPU number */
	mfc0	v1, CP0_PRID, 1
	andi	v1, 0x3ff
	bnez	v1, 1f
	 nop

	/* Core 0: Initialise CM */
	lw	t1, GCMPGCBOFS(GCMPB)(GCMP)
	or	t1, GCMP_GCB_GCMPB_CMDEFTGT_MEM<<GCMP_GCB_GCMPB_CMDEFTGT_SHF
	sw	t1, GCMPGCBOFS(GCMPB)(GCMP)

	/* Disable the CM regions */
	lui	t1, 0xffff
	sw	t1, GCMP_GCB_CMxBASE_OFS(0)(GCMP)
	sw	t1, GCMP_GCB_CMxMASK_OFS(0)(GCMP)
	sw	t1, GCMP_GCB_CMxBASE_OFS(1)(GCMP)
	sw	t1, GCMP_GCB_CMxMASK_OFS(1)(GCMP)
	sw	t1, GCMP_GCB_CMxBASE_OFS(2)(GCMP)
	sw	t1, GCMP_GCB_CMxMASK_OFS(2)(GCMP)
	sw	t1, GCMP_GCB_CMxBASE_OFS(3)(GCMP)
	sw	t1, GCMP_GCB_CMxMASK_OFS(3)(GCMP)
1:
	/* Core n: Allow interventions from all other cores (including self) */
	li	t1, 0xff
	sw	t1, GCMPCLCBOFS(COHCTL)(GCMP)
	sll	zero, zero, 3	/* ehb */

	jr	ra
	 nop
	.end mips_cm_setup
