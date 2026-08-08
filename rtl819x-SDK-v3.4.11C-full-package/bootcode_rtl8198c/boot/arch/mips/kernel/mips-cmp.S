/*
 * Realtek Semiconductor Corp.
 *
 * mips-cmp.S:
 *	MIPS CMP multicore boot procedure
 *
 * Copyright 2006-2015  PSP Software Group
 */
//#include <config.h>
//
#include <asm/gcmpregs.h>
#include <asm/smp-boot.h>
#include <asm/mipsregs.h>
#include <asm/regdef.h>

	.text

	.globl mips_cmp_copy
	.ent mips_cmp_copy
	.set push
	.set noreorder
mips_cmp_copy:
	la	t0, CMP_SMPBOOT
	la	t1, mips_cmp_secondary
	la	t2, mips_cmp_secondary_end
1:
	lw	a0, 0(t1)
	sw	a0, 0(t0)
	addi	t0, 0x4
	addi	t1, 0x4
	ble	t1, t2, 1b
	 nop
	jr	ra
	 nop
	.set pop
	.end mips_cmp_copy

	.globl mips_cmp_first
	.ent mips_cmp_first
	.set push
	.set noreorder
mips_cmp_first:
	la	t0, CMP_SMPFLAG
	addiu	t1, t0, (32 * NR_CPUS - 4)
1:
	sw	zero, 0(t0)
	bne	t0, t1, 1b
	 add	t0, 4

	/* mark first cpu ready and gone */
	la	t0, CMP_SMPFLAG
	li	t1, SMP_LAUNCH_FREADY|SMP_LAUNCH_FGO|SMP_LAUNCH_FGONE
	sw	t1, SMP_LAUNCH_FLAGS(t0)
	jr	ra
	 nop
	.set pop
	.end mips_cmp_first

	.globl mips_cmp_secondary
	.section ".text.smploop", "ax"
	.ent mips_cmp_secondary
	.set push
	.set noreorder
mips_cmp_secondary:
	la	t0, CMP_SMPFLAG

1:	/* wait until cpu0 goes */
	lw	t1, SMP_LAUNCH_FLAGS(t0)
	andi	t1, SMP_LAUNCH_FGONE
	beqz	t1, 1b
	 nop

	/* each CMP_SMPFLAG struct is 32 byte wide */
	sll	t1, v1, 5
	addu	t0, t1

	/* mark this cpu ready to go */
	lw	t1, SMP_LAUNCH_FLAGS(t0)
	ori	t1, SMP_LAUNCH_FREADY
	sw	t1, SMP_LAUNCH_FLAGS(t0)

	/* enable counter interrupt */
#ifdef CONFIG_CPU_MIPS32
	mfc0	t2, CP0_STATUS, 1
	srl	t2, INTCTLB_IPTI
	addi	t2, STATUSB_IP0
	li	t1, 1
	sllv	t1, t1, t2
	mfc0	t2, CP0_STATUS
	or	t1, t1, t2
	mtc0	t1, CP0_STATUS
#else
	mfc0	t2, CP0_STATUS
	ori	t1, t2, STATUSF_IP7
	mtc0	t1, CP0_STATUS
#endif

1:	/* software wait loop */
	mfc0	t1,CP0_COUNT
	addu	t1,SMP_LAUNCH_PERIOD
	mtc0	t1,CP0_COMPARE

2:
	mfc0	t3, CP0_COUNT
	subu	t3, t1
	bltz	t3, 2b
	 nop

	/* check whether we are signaled to go */
	lw	t1, SMP_LAUNCH_FLAGS(t0)
	andi	t1, SMP_LAUNCH_FGO
	beqz	t1, 1b
	 nop

	/* Reset the counter and interrupts to give naive clients a chance */
	mtc0	t2, CP0_STATUS
	mfc0	t2, CP0_COUNT
	subu	t2, 1
	mtc0	t2, CP0_COMPARE

	/* we're off */
	lw	t2, SMP_LAUNCH_PC(t0)
	lw	gp, SMP_LAUNCH_GP(t0)
	lw	sp, SMP_LAUNCH_SP(t0)
	lw	a0, SMP_LAUNCH_A0(t0)
	move	a1, zero
	move	a2, zero
	move	a3, zero
	ori	t1, SMP_LAUNCH_FGONE
	jr	t2
	 sw	t1, SMP_LAUNCH_FLAGS(t0)
	.set pop
	.globl mips_cmp_secondary_end
mips_cmp_secondary_end:
	.end mips_cmp_secondary
