	.file	1 "fast_pppoe_core.c"
	.section .mdebug.abi32
	.previous
	.nan	legacy
	.module	softfloat
	.module	oddspreg
	.text
	.align	2
	.globl	fastpath_pppoe_single_open
	.set	nomips16
	.set	nomicromips
	.ent	fastpath_pppoe_single_open
	.type	fastpath_pppoe_single_open, @function
fastpath_pppoe_single_open:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	move	$4,$5
	lui	$5,%hi(fast_pppoe_read_proc)
	move	$6,$0
	addiu	$5,$5,%lo(fast_pppoe_read_proc)
	j	single_open
	nop

	.set	macro
	.set	reorder
	.end	fastpath_pppoe_single_open
	.size	fastpath_pppoe_single_open, .-fastpath_pppoe_single_open
	.section	.rodata.str1.4,"aMS",@progbits,1
	.align	2
$LC0:
	.ascii	"fast pppoe enable:%c\012\000"
	.align	2
$LC1:
	.ascii	"[%d] null,null,0\012\000"
	.align	2
$LC2:
	.ascii	"[%d] %s,%s,%d,0x%x:%x:%x:%x:%x:%x\012\000"
	.text
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	fast_pppoe_read_proc
	.type	fast_pppoe_read_proc, @function
fast_pppoe_read_proc:
	.frame	$sp,80,$31		# vars= 0, regs= 7/0, args= 48, gp= 0
	.mask	0x803f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-80
	sw	$31,76($sp)
	sw	$21,72($sp)
	sw	$20,68($sp)
	sw	$19,64($sp)
	sw	$18,60($sp)
	sw	$17,56($sp)
	sw	$16,52($sp)
	lui	$2,%hi(fast_pppoe_fw)
	lui	$5,%hi($LC0)
	lui	$16,%hi(fast_pppoe_info)
	lw	$6,%lo(fast_pppoe_fw)($2)
	addiu	$5,$5,%lo($LC0)
	lui	$21,%hi($LC2)
	lui	$19,%hi($LC1)
	addiu	$6,$6,48
	move	$18,$4
	jal	seq_printf
	addiu	$16,$16,%lo(fast_pppoe_info)

	move	$17,$0
	addiu	$21,$21,%lo($LC2)
	addiu	$19,$19,%lo($LC1)
	b	$L5
	li	$20,16			# 0x10

$L10:
	jal	seq_printf
	addiu	$17,$17,1

	beq	$17,$20,$L9
	addiu	$16,$16,88

$L5:
	lhu	$2,54($16)
	addiu	$7,$16,16
	move	$6,$17
	move	$5,$19
	beq	$2,$0,$L10
	move	$4,$18

	lbu	$12,43($16)
	move	$6,$17
	lbu	$11,42($16)
	addiu	$17,$17,1
	lbu	$10,41($16)
	move	$5,$21
	lbu	$9,40($16)
	move	$4,$18
	lbu	$8,39($16)
	lbu	$3,38($16)
	lhu	$2,52($16)
	sw	$16,16($sp)
	addiu	$16,$16,88
	sw	$12,44($sp)
	sw	$11,40($sp)
	sw	$10,36($sp)
	sw	$9,32($sp)
	sw	$8,28($sp)
	sw	$3,24($sp)
	jal	seq_printf
	sw	$2,20($sp)

	bne	$17,$20,$L5
	nop

$L9:
	lw	$31,76($sp)
	move	$2,$0
	lw	$21,72($sp)
	lw	$20,68($sp)
	lw	$19,64($sp)
	lw	$18,60($sp)
	lw	$17,56($sp)
	lw	$16,52($sp)
	jr	$31
	addiu	$sp,$sp,80

	.set	macro
	.set	reorder
	.end	fast_pppoe_read_proc
	.size	fast_pppoe_read_proc, .-fast_pppoe_read_proc
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	find_pppoe_info
	.type	find_pppoe_info, @function
find_pppoe_info:
	.frame	$sp,64,$31		# vars= 8, regs= 10/0, args= 16, gp= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-64
	sw	$31,60($sp)
	sw	$fp,56($sp)
	sw	$23,52($sp)
	sw	$22,48($sp)
	sw	$21,44($sp)
	sw	$20,40($sp)
	sw	$19,36($sp)
	sw	$18,32($sp)
	sw	$17,28($sp)
	sw	$16,24($sp)
	lui	$fp,%hi(fast_pppoe_info)
	lw	$19,88($sp)
	move	$20,$0
	addiu	$2,$fp,%lo(fast_pppoe_info)
	move	$21,$0
	li	$16,16			# 0x10
	sw	$2,16($sp)
	move	$17,$4
	move	$18,$5
	move	$23,$6
	move	$22,$7
	move	$fp,$2
$L20:
	lhu	$2,54($fp)
	beq	$2,$0,$L12
	.set	noreorder
	.set	nomacro
	beq	$17,$0,$L13
	addiu	$2,$fp,16
	.set	macro
	.set	reorder

	move	$4,$17
 #APP
 # 77 "./arch/mips/include/asm/string.h" 1
	.set	noreorder
	.set	noat
	lbu	$5,($2)
1:	lbu	$1,($4)
	addiu	$2,1
	bne	$1,$5,2f
	addiu	$4,1
	bnez	$5,1b
	lbu	$5,($2)
	move	$5,$1
2:	subu	$5,$1
3:	.set	at
	.set	reorder
 # 0 "" 2
 #NO_APP
	.set	noreorder
	.set	nomacro
	beq	$5,$0,$L51
	lw	$2,16($sp)
	.set	macro
	.set	reorder

$L13:
	.set	noreorder
	.set	nomacro
	beq	$18,$0,$L15
	move	$2,$fp
	.set	macro
	.set	reorder

	move	$4,$18
 #APP
 # 77 "./arch/mips/include/asm/string.h" 1
	.set	noreorder
	.set	noat
	lbu	$5,($2)
1:	lbu	$1,($4)
	addiu	$2,1
	bne	$1,$5,2f
	addiu	$4,1
	bnez	$5,1b
	lbu	$5,($2)
	move	$5,$1
2:	subu	$5,$1
3:	.set	at
	.set	reorder
 # 0 "" 2
 #NO_APP
	.set	noreorder
	.set	nomacro
	beq	$5,$0,$L11
	move	$2,$fp
	.set	macro
	.set	reorder

$L15:
	.set	noreorder
	.set	nomacro
	beq	$19,$0,$L16
	li	$6,6			# 0x6
	.set	macro
	.set	reorder

	addiu	$4,$fp,38
	.set	noreorder
	.set	nomacro
	jal	memcmp
	move	$5,$19
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L51
	lw	$2,16($sp)
	.set	macro
	.set	reorder

$L16:
	lw	$2,84($sp)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L17
	li	$6,6			# 0x6
	.set	macro
	.set	reorder

	addiu	$4,$fp,32
	.set	noreorder
	.set	nomacro
	jal	memcmp
	move	$5,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L49
	lw	$2,16($sp)
	.set	macro
	.set	reorder

$L17:
	lw	$2,80($sp)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L18
	lw	$3,80($sp)
	.set	macro
	.set	reorder

	lw	$2,48($fp)
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L52
	li	$2,88			# 0x58
	.set	macro
	.set	reorder

$L18:
	beq	$22,$0,$L19
	lw	$2,44($fp)
	.set	noreorder
	.set	nomacro
	beq	$22,$2,$L52
	li	$2,88			# 0x58
	.set	macro
	.set	reorder

$L19:
	beq	$23,$0,$L12
	lhu	$2,52($fp)
	.set	noreorder
	.set	nomacro
	beq	$2,$23,$L50
	li	$2,88			# 0x58
	.set	macro
	.set	reorder

$L12:
	addiu	$21,$21,1
	addiu	$fp,$fp,88
	.set	noreorder
	.set	nomacro
	bne	$21,$16,$L20
	addiu	$20,$20,88
	.set	macro
	.set	reorder

	move	$2,$0
$L11:
	lw	$31,60($sp)
	lw	$fp,56($sp)
	lw	$23,52($sp)
	lw	$22,48($sp)
	lw	$21,44($sp)
	lw	$20,40($sp)
	lw	$19,36($sp)
	lw	$18,32($sp)
	lw	$17,28($sp)
	lw	$16,24($sp)
	.set	noreorder
	.set	nomacro
	jr	$31
	addiu	$sp,$sp,64
	.set	macro
	.set	reorder

$L49:
$L51:
	lw	$31,60($sp)
	lw	$fp,56($sp)
	addu	$2,$2,$20
	lw	$23,52($sp)
	lw	$22,48($sp)
	lw	$21,44($sp)
	lw	$20,40($sp)
	lw	$19,36($sp)
	lw	$18,32($sp)
	lw	$17,28($sp)
	lw	$16,24($sp)
	.set	noreorder
	.set	nomacro
	jr	$31
	addiu	$sp,$sp,64
	.set	macro
	.set	reorder

$L50:
$L52:
	lw	$3,16($sp)
	lw	$31,60($sp)
	mul	$4,$21,$2
	lw	$fp,56($sp)
	lw	$23,52($sp)
	lw	$22,48($sp)
	lw	$21,44($sp)
	lw	$20,40($sp)
	lw	$19,36($sp)
	lw	$18,32($sp)
	lw	$17,28($sp)
	addu	$2,$4,$3
	lw	$16,24($sp)
	.set	noreorder
	.set	nomacro
	jr	$31
	addiu	$sp,$sp,64
	.set	macro
	.set	reorder

	.end	find_pppoe_info
	.size	find_pppoe_info, .-find_pppoe_info
	.section	.rodata.str1.4
	.align	2
$LC3:
	.ascii	"net/rtl/fastpath/core/fast_pppoe_core.c\000"
	.text
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	fastpath_pppoe_single_write
	.type	fastpath_pppoe_single_write, @function
fastpath_pppoe_single_write:
	.frame	$sp,40,$31		# vars= 8, regs= 3/0, args= 16, gp= 0
	.mask	0x80030000,-4
	.fmask	0x00000000,0
	sltu	$2,$6,2
	bne	$2,$0,$L63
	beq	$5,$0,$L63
	addiu	$sp,$sp,-40
	sw	$31,36($sp)
	sw	$17,32($sp)
	sw	$16,28($sp)
	addiu	$2,$5,1
	lw	$3,24($28)
	move	$17,$5
	or	$2,$2,$5
	and	$2,$2,$3
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L57
	move	$16,$6
	.set	macro
	.set	reorder

	lui	$4,%hi($LC3)
	li	$5,69			# 0x45
	.set	noreorder
	.set	nomacro
	jal	__might_fault
	addiu	$4,$4,%lo($LC3)
	.set	macro
	.set	reorder

	addiu	$4,$sp,16
	move	$5,$17
	li	$6,1			# 0x1
 #APP
 # 79 "net/rtl/fastpath/core/fast_pppoe_core.c" 1
	.set	noreorder
	jal	__copy_user
	.set	noat
	addu	$1, $5, $6
	.set	at
	.set	reorder
 # 0 "" 2
 #NO_APP
	.set	noreorder
	.set	nomacro
	bne	$6,$0,$L66
	lb	$3,16($sp)
	.set	macro
	.set	reorder

	move	$2,$16
	lui	$4,%hi(fast_pppoe_fw)
	lw	$31,36($sp)
	lw	$17,32($sp)
	lw	$16,28($sp)
	addiu	$3,$3,-48
	sw	$3,%lo(fast_pppoe_fw)($4)
	.set	noreorder
	.set	nomacro
	jr	$31
	addiu	$sp,$sp,40
	.set	macro
	.set	reorder

$L57:
	li	$6,1			# 0x1
	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	memset
	addiu	$4,$sp,16
	.set	macro
	.set	reorder

$L66:
	lw	$31,36($sp)
	li	$2,-14			# 0xfffffffffffffff2
	lw	$17,32($sp)
	lw	$16,28($sp)
	.set	noreorder
	.set	nomacro
	jr	$31
	addiu	$sp,$sp,40
	.set	macro
	.set	reorder

$L63:
	.set	noreorder
	.set	nomacro
	jr	$31
	li	$2,-14			# 0xfffffffffffffff2
	.set	macro
	.set	reorder

	.end	fastpath_pppoe_single_write
	.size	fastpath_pppoe_single_write, .-fastpath_pppoe_single_write
	.section	.rodata.str1.4
	.align	2
$LC4:
	.ascii	"fast_pppoe\000"
	.section	.init.text,"ax",@progbits
	.align	2
	.globl	fast_pppoe_init
	.set	nomips16
	.set	nomicromips
	.ent	fast_pppoe_init
	.type	fast_pppoe_init, @function
fast_pppoe_init:
	.frame	$sp,32,$31		# vars= 0, regs= 1/0, args= 24, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	lui	$7,%hi(fastpath_pppoe_proc_fops)
	lui	$6,%hi(proc_root)
	sw	$0,16($sp)
	lui	$4,%hi($LC4)
	addiu	$7,$7,%lo(fastpath_pppoe_proc_fops)
	addiu	$6,$6,%lo(proc_root)
	move	$5,$0
	jal	proc_create_data
	addiu	$4,$4,%lo($LC4)

	lui	$4,%hi(fast_pppoe_info)
	lui	$3,%hi(fast_pppoe_proc)
	li	$6,1408			# 0x580
	move	$5,$0
	addiu	$4,$4,%lo(fast_pppoe_info)
	jal	memset
	sw	$2,%lo(fast_pppoe_proc)($3)

	lui	$3,%hi(fast_pppoe_fw)
	lw	$31,28($sp)
	move	$2,$0
	li	$4,1			# 0x1
	sw	$4,%lo(fast_pppoe_fw)($3)
	jr	$31
	addiu	$sp,$sp,32

	.set	macro
	.set	reorder
	.end	fast_pppoe_init
	.size	fast_pppoe_init, .-fast_pppoe_init
	.section	.exit.text,"ax",@progbits
	.align	2
	.globl	fast_pppoe_exit
	.set	nomips16
	.set	nomicromips
	.ent	fast_pppoe_exit
	.type	fast_pppoe_exit, @function
fast_pppoe_exit:
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$16,16($sp)
	lui	$16,%hi(fast_pppoe_proc)
	lw	$2,%lo(fast_pppoe_proc)($16)
	beq	$2,$0,$L70
	lui	$5,%hi(proc_root)

	lui	$4,%hi($LC4)
	addiu	$5,$5,%lo(proc_root)
	jal	remove_proc_entry
	addiu	$4,$4,%lo($LC4)

	sw	$0,%lo(fast_pppoe_proc)($16)
$L70:
	lw	$31,20($sp)
	move	$2,$0
	lw	$16,16($sp)
	jr	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	fast_pppoe_exit
	.size	fast_pppoe_exit, .-fast_pppoe_exit
	.text
	.align	2
	.globl	clear_pppoe_info
	.set	nomips16
	.set	nomicromips
	.ent	clear_pppoe_info
	.type	clear_pppoe_info, @function
clear_pppoe_info:
	.frame	$sp,48,$31		# vars= 0, regs= 8/0, args= 16, gp= 0
	.mask	0x807f0000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-48
	sw	$31,44($sp)
	sw	$22,40($sp)
	sw	$21,36($sp)
	sw	$20,32($sp)
	sw	$19,28($sp)
	sw	$18,24($sp)
	sw	$17,20($sp)
	sw	$16,16($sp)
	lui	$16,%hi(fast_pppoe_info)
	lui	$17,%hi(fast_pppoe_info+1408)
	lw	$21,68($sp)
	lw	$20,72($sp)
	addiu	$16,$16,%lo(fast_pppoe_info)
	addiu	$17,$17,%lo(fast_pppoe_info+1408)
	move	$19,$4
	move	$18,$5
	.set	noreorder
	.set	nomacro
	b	$L87
	move	$22,$6
	.set	macro
	.set	reorder

$L78:
	.set	noreorder
	.set	nomacro
	beq	$18,$0,$L79
	move	$2,$16
	.set	macro
	.set	reorder

	move	$3,$18
 #APP
 # 77 "./arch/mips/include/asm/string.h" 1
	.set	noreorder
	.set	noat
	lbu	$4,($2)
1:	lbu	$1,($3)
	addiu	$2,1
	bne	$1,$4,2f
	addiu	$3,1
	bnez	$4,1b
	lbu	$4,($2)
	move	$4,$1
2:	subu	$4,$1
3:	.set	at
	.set	reorder
 # 0 "" 2
 #NO_APP
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L96
	li	$6,88			# 0x58
	.set	macro
	.set	reorder

$L79:
	.set	noreorder
	.set	nomacro
	beq	$20,$0,$L81
	li	$6,6			# 0x6
	.set	macro
	.set	reorder

	addiu	$4,$16,38
	.set	noreorder
	.set	nomacro
	jal	memcmp
	move	$5,$20
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L97
	li	$6,88			# 0x58
	.set	macro
	.set	reorder

$L81:
	.set	noreorder
	.set	nomacro
	beq	$21,$0,$L84
	li	$6,6			# 0x6
	.set	macro
	.set	reorder

	addiu	$4,$16,32
	.set	noreorder
	.set	nomacro
	jal	memcmp
	move	$5,$21
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L98
	li	$6,88			# 0x58
	.set	macro
	.set	reorder

$L84:
	lhu	$2,52($16)
	.set	noreorder
	.set	nomacro
	beq	$2,$22,$L101
	move	$4,$16
	.set	macro
	.set	reorder

$L77:
	addiu	$16,$16,88
	.set	noreorder
	.set	nomacro
	beq	$16,$17,$L102
	lw	$31,44($sp)
	.set	macro
	.set	reorder

$L87:
	lhu	$2,54($16)
	beq	$2,$0,$L77
	.set	noreorder
	.set	nomacro
	beq	$19,$0,$L78
	addiu	$2,$16,16
	.set	macro
	.set	reorder

	move	$3,$19
 #APP
 # 77 "./arch/mips/include/asm/string.h" 1
	.set	noreorder
	.set	noat
	lbu	$4,($2)
1:	lbu	$1,($3)
	addiu	$2,1
	bne	$1,$4,2f
	addiu	$3,1
	bnez	$4,1b
	lbu	$4,($2)
	move	$4,$1
2:	subu	$4,$1
3:	.set	at
	.set	reorder
 # 0 "" 2
 #NO_APP
	.set	noreorder
	.set	nomacro
	bne	$4,$0,$L78
	li	$6,88			# 0x58
	.set	macro
	.set	reorder

	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	memset
	move	$4,$16
	.set	macro
	.set	reorder

	b	$L78
$L96:
	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	memset
	move	$4,$16
	.set	macro
	.set	reorder

	b	$L79
$L97:
	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	memset
	move	$4,$16
	.set	macro
	.set	reorder

	b	$L81
$L98:
	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	memset
	move	$4,$16
	.set	macro
	.set	reorder

	lhu	$2,52($16)
	.set	noreorder
	.set	nomacro
	bne	$2,$22,$L77
	move	$4,$16
	.set	macro
	.set	reorder

$L101:
	li	$6,88			# 0x58
	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	memset
	addiu	$16,$16,88
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$16,$17,$L87
	lw	$31,44($sp)
	.set	macro
	.set	reorder

$L102:
	move	$2,$0
	lw	$22,40($sp)
	lw	$21,36($sp)
	lw	$20,32($sp)
	lw	$19,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	.set	noreorder
	.set	nomacro
	jr	$31
	addiu	$sp,$sp,48
	.set	macro
	.set	reorder

	.end	clear_pppoe_info
	.size	clear_pppoe_info, .-clear_pppoe_info
	.align	2
	.globl	set_pppoe_info
	.set	nomips16
	.set	nomicromips
	.ent	set_pppoe_info
	.type	set_pppoe_info, @function
set_pppoe_info:
	.frame	$sp,72,$31		# vars= 0, regs= 9/0, args= 32, gp= 0
	.mask	0x80ff0000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-72
	sw	$31,68($sp)
	sw	$23,64($sp)
	sw	$22,60($sp)
	sw	$21,56($sp)
	sw	$20,52($sp)
	sw	$19,48($sp)
	sw	$18,44($sp)
	sw	$17,40($sp)
	sw	$16,36($sp)
	move	$19,$5
	lw	$21,88($sp)
	move	$23,$6
	lw	$17,92($sp)
	move	$6,$0
	lw	$18,96($sp)
	move	$5,$0
	sw	$21,16($sp)
	move	$20,$4
	sw	$17,20($sp)
	move	$22,$7
	.set	noreorder
	.set	nomacro
	jal	find_pppoe_info
	sw	$18,24($sp)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L137
	move	$16,$2
	.set	macro
	.set	reorder

$L104:
	beq	$23,$0,$L107
	sh	$23,52($16)
$L107:
	beq	$22,$0,$L108
	sw	$22,44($16)
$L108:
	bne	$21,$0,$L138
$L109:
	.set	noreorder
	.set	nomacro
	beq	$20,$0,$L110
	addiu	$2,$16,16
	.set	macro
	.set	reorder

	move	$4,$20
 #APP
 # 27 "./arch/mips/include/asm/string.h" 1
	.set	noreorder
	.set	noat
1:	lbu	$1,($4)
	addiu	$4,1
	sb	$1,($2)
	bnez	$1,1b
	addiu	$2,1
	.set	at
	.set	reorder
 # 0 "" 2
 #NO_APP
$L110:
	.set	noreorder
	.set	nomacro
	beq	$19,$0,$L111
	move	$2,$16
	.set	macro
	.set	reorder

	move	$5,$19
 #APP
 # 27 "./arch/mips/include/asm/string.h" 1
	.set	noreorder
	.set	noat
1:	lbu	$1,($5)
	addiu	$5,1
	sb	$1,($2)
	bnez	$1,1b
	addiu	$2,1
	.set	at
	.set	reorder
 # 0 "" 2
 #NO_APP
$L111:
	.set	noreorder
	.set	nomacro
	beq	$18,$0,$L112
	li	$6,6			# 0x6
	.set	macro
	.set	reorder

	addiu	$4,$16,38
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$5,$18
	.set	macro
	.set	reorder

$L112:
	.set	noreorder
	.set	nomacro
	beq	$17,$0,$L113
	li	$6,6			# 0x6
	.set	macro
	.set	reorder

	addiu	$4,$16,32
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$5,$17
	.set	macro
	.set	reorder

$L113:
	li	$3,1			# 0x1
	move	$2,$0
	sh	$3,54($16)
	lw	$31,68($sp)
	lw	$23,64($sp)
	lw	$22,60($sp)
	lw	$21,56($sp)
	lw	$20,52($sp)
	lw	$19,48($sp)
	lw	$18,44($sp)
	lw	$17,40($sp)
	lw	$16,36($sp)
	.set	noreorder
	.set	nomacro
	jr	$31
	addiu	$sp,$sp,72
	.set	macro
	.set	reorder

$L138:
	.set	noreorder
	.set	nomacro
	b	$L109
	sw	$21,48($16)
	.set	macro
	.set	reorder

$L137:
	lui	$2,%hi(fast_pppoe_info+54)
	li	$4,16			# 0x10
	addiu	$2,$2,%lo(fast_pppoe_info+54)
$L106:
	lhu	$3,0($2)
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L139
	addiu	$2,$2,88
	.set	macro
	.set	reorder

	addiu	$16,$16,1
	.set	noreorder
	.set	nomacro
	bne	$16,$4,$L106
	li	$6,1408			# 0x580
	.set	macro
	.set	reorder

	lui	$16,%hi(fast_pppoe_info)
	addiu	$4,$16,%lo(fast_pppoe_info)
	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	memset
	move	$16,$4
	.set	macro
	.set	reorder

	b	$L104
$L139:
	li	$3,88			# 0x58
	lui	$2,%hi(fast_pppoe_info)
	mul	$4,$16,$3
	addiu	$2,$2,%lo(fast_pppoe_info)
	.set	noreorder
	.set	nomacro
	b	$L104
	addu	$16,$4,$2
	.set	macro
	.set	reorder

	.end	set_pppoe_info
	.size	set_pppoe_info, .-set_pppoe_info
	.align	2
	.globl	get_pppoe_last_rx_tx
	.set	nomips16
	.set	nomicromips
	.ent	get_pppoe_last_rx_tx
	.type	get_pppoe_last_rx_tx, @function
get_pppoe_last_rx_tx:
	.frame	$sp,40,$31		# vars= 0, regs= 1/0, args= 32, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-40
	sw	$31,36($sp)
	lw	$2,64($sp)
	sw	$2,24($sp)
	lw	$2,60($sp)
	sw	$2,20($sp)
	lw	$2,56($sp)
	jal	find_pppoe_info
	sw	$2,16($sp)

	beq	$2,$0,$L141
	lw	$3,68($sp)

	lw	$4,56($2)
	sw	$4,0($3)
	lw	$3,60($2)
	lw	$2,72($sp)
	sw	$3,0($2)
$L141:
	lw	$31,36($sp)
	move	$2,$0
	jr	$31
	addiu	$sp,$sp,40

	.set	macro
	.set	reorder
	.end	get_pppoe_last_rx_tx
	.size	get_pppoe_last_rx_tx, .-get_pppoe_last_rx_tx
	.align	2
	.globl	check_and_pull_pppoe_hdr
	.set	nomips16
	.set	nomicromips
	.ent	check_and_pull_pppoe_hdr
	.type	check_and_pull_pppoe_hdr, @function
check_and_pull_pppoe_hdr:
	.frame	$sp,64,$31		# vars= 8, regs= 10/0, args= 16, gp= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	lui	$2,%hi(fast_pppoe_fw)
	lw	$2,%lo(fast_pppoe_fw)($2)
	bne	$2,$0,$L169
	move	$2,$0

	jr	$31
	nop

$L169:
	addiu	$sp,$sp,-64
	sw	$31,60($sp)
	sw	$fp,56($sp)
	sw	$23,52($sp)
	sw	$22,48($sp)
	sw	$21,44($sp)
	sw	$20,40($sp)
	sw	$19,36($sp)
	sw	$18,32($sp)
	sw	$17,28($sp)
	sw	$16,24($sp)
	move	$5,$0
	jal	rtl_set_skb_pppoe_flag
	move	$19,$4

	jal	rtl_skb_mac_header
	move	$4,$19

	beq	$2,$0,$L149
	move	$18,$2

	lhu	$3,12($2)
	li	$2,25736			# 0x6488
	beq	$3,$2,$L170
	li	$3,8448			# 0x2100

$L149:
	move	$2,$0
$L146:
	lw	$31,60($sp)
	lw	$fp,56($sp)
	lw	$23,52($sp)
	lw	$22,48($sp)
	lw	$21,44($sp)
	lw	$20,40($sp)
	lw	$19,36($sp)
	lw	$18,32($sp)
	lw	$17,28($sp)
	lw	$16,24($sp)
	jr	$31
	addiu	$sp,$sp,64

$L170:
	lhu	$2,20($18)
	beq	$2,$3,$L150
	li	$3,22272			# 0x5700

	bne	$2,$3,$L146
	move	$2,$0

$L150:
	lhu	$3,16($18)
	lui	$17,%hi(fast_pppoe_info)
	lui	$16,%hi(fast_pppoe_info+38)
	addiu	$20,$17,%lo(fast_pppoe_info)
	wsbh	$3,$3
	li	$23,-38			# 0xffffffffffffffda
	addiu	$22,$18,6
	addiu	$16,$16,%lo(fast_pppoe_info+38)
	move	$17,$0
	subu	$23,$23,$20
	li	$21,16			# 0x10
	b	$L153
	andi	$fp,$3,0xffff

$L151:
	addiu	$17,$17,1
$L171:
	beq	$17,$21,$L149
	addiu	$16,$16,88

$L153:
	lhu	$2,16($16)
	beq	$2,$0,$L151
	nop

	lhu	$2,14($16)
	bne	$2,$fp,$L151
	addu	$2,$23,$16

	li	$6,6			# 0x6
	move	$4,$16
	sw	$2,16($sp)
	jal	memcmp
	move	$5,$22

	bne	$2,$0,$L171
	addiu	$17,$17,1

	addiu	$17,$17,-1
	lw	$2,16($sp)
	addiu	$4,$2,16
	jal	rtl_get_dev_by_name
	addu	$4,$20,$4

	beq	$2,$0,$L149
	move	$16,$2

	addiu	$18,$18,22
	move	$4,$19
	jal	rtl_set_skb_network_header
	move	$5,$18

	move	$5,$18
	jal	rtl_set_skb_transport_header
	move	$4,$19

	li	$5,8			# 0x8
	jal	rtl_skb_pull
	move	$4,$19

	jal	rtl_get_skb_dev
	move	$4,$19

	move	$4,$19
	jal	rtl_set_skb_rx_dev
	move	$5,$2

	li	$5,8			# 0x8
	jal	rtl_set_skb_protocol
	move	$4,$19

	move	$5,$16
	jal	rtl_set_skb_dev
	move	$4,$19

	jal	rtl_get_skb_dev
	move	$4,$19

	move	$4,$19
	lw	$3,192($2)
	addiu	$3,$3,1
	jal	rtl_get_skb_len
	sw	$3,192($2)

	move	$4,$19
	jal	rtl_get_skb_dev
	move	$16,$2

	lw	$3,200($2)
	li	$5,1			# 0x1
	move	$4,$19
	addu	$3,$3,$16
	jal	rtl_set_skb_pppoe_flag
	sw	$3,200($2)

	li	$3,88			# 0x58
	li	$2,1			# 0x1
	mul	$4,$17,$3
	lui	$3,%hi(jiffies)
	lw	$3,%lo(jiffies)($3)
	addu	$17,$4,$20
	b	$L146
	sw	$3,56($17)

	.set	macro
	.set	reorder
	.end	check_and_pull_pppoe_hdr
	.size	check_and_pull_pppoe_hdr, .-check_and_pull_pppoe_hdr
	.align	2
	.globl	check_and_restore_pppoe_hdr
	.set	nomips16
	.set	nomicromips
	.ent	check_and_restore_pppoe_hdr
	.type	check_and_restore_pppoe_hdr, @function
check_and_restore_pppoe_hdr:
	.frame	$sp,32,$31		# vars= 0, regs= 3/0, args= 16, gp= 0
	.mask	0x80030000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	lui	$2,%hi(fast_pppoe_fw)
	lw	$2,%lo(fast_pppoe_fw)($2)
	bne	$2,$0,$L186
	nop

	jr	$31
	nop

$L186:
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$17,24($sp)
	sw	$16,20($sp)
	jal	rtl_get_skb_pppoe_flag
	move	$16,$4

	bne	$2,$0,$L187
	lw	$31,28($sp)

	lw	$17,24($sp)
	lw	$16,20($sp)
	jr	$31
	addiu	$sp,$sp,32

$L187:
	li	$5,8			# 0x8
	jal	rtl_skb_push
	move	$4,$16

	jal	rtl_get_skb_data
	move	$4,$16

	move	$4,$16
	jal	rtl_set_skb_network_header
	move	$5,$2

	jal	rtl_get_skb_data
	move	$4,$16

	move	$4,$16
	jal	rtl_set_skb_transport_header
	move	$5,$2

	jal	rtl_get_skb_dev
	move	$4,$16

	move	$4,$16
	lw	$3,192($2)
	addiu	$3,$3,-1
	jal	rtl_get_skb_len
	sw	$3,192($2)

	move	$4,$16
	jal	rtl_get_skb_dev
	move	$17,$2

	lw	$3,200($2)
	move	$4,$16
	subu	$3,$3,$17
	jal	rtl_get_skb_rx_dev
	sw	$3,200($2)

	move	$4,$16
	jal	rtl_set_skb_dev
	move	$5,$2

	li	$5,25736			# 0x6488
	jal	rtl_set_skb_protocol
	move	$4,$16

	move	$4,$16
	lw	$31,28($sp)
	move	$5,$0
	lw	$17,24($sp)
	lw	$16,20($sp)
	j	rtl_set_skb_pppoe_flag
	addiu	$sp,$sp,32

	.set	macro
	.set	reorder
	.end	check_and_restore_pppoe_hdr
	.size	check_and_restore_pppoe_hdr, .-check_and_restore_pppoe_hdr
	.section	.rodata.str1.4
	.align	2
$LC5:
	.ascii	"ppp\000"
	.text
	.align	2
	.globl	fast_pppoe_xmit
	.set	nomips16
	.set	nomicromips
	.ent	fast_pppoe_xmit
	.type	fast_pppoe_xmit, @function
fast_pppoe_xmit:
	.frame	$sp,56,$31		# vars= 0, regs= 6/0, args= 32, gp= 0
	.mask	0x801f0000,-4
	.fmask	0x00000000,0
	lui	$2,%hi(fast_pppoe_fw)
	lw	$2,%lo(fast_pppoe_fw)($2)
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L214
	move	$2,$0
	.set	macro
	.set	reorder

	jr	$31
$L214:
	addiu	$sp,$sp,-56
	sw	$31,52($sp)
	sw	$20,48($sp)
	sw	$19,44($sp)
	sw	$18,40($sp)
	sw	$17,36($sp)
	sw	$16,32($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_dev
	move	$16,$4
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L188
	move	$2,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_dev
	move	$4,$16
	.set	macro
	.set	reorder

	lui	$3,%hi($LC5)
	li	$5,3			# 0x3
	addiu	$3,$3,%lo($LC5)
 #APP
 # 108 "./arch/mips/include/asm/string.h" 1
	.set	noreorder
	.set	noat
1:	lbu	$4,($2)
	beqz	$5,2f
	lbu	$1,($3)
	subu	$5,1
	bne	$1,$4,3f
	addiu	$2,1
	bnez	$4,1b
	addiu	$3,1
2:
	move	$4,$1
3:	subu	$4,$1
	.set	at
	.set	reorder
 # 0 "" 2
 #NO_APP
	beq	$4,$0,$L215
$L191:
	move	$2,$0
$L188:
	lw	$31,52($sp)
	lw	$20,48($sp)
	lw	$19,44($sp)
	lw	$18,40($sp)
	lw	$17,36($sp)
	lw	$16,32($sp)
	.set	noreorder
	.set	nomacro
	jr	$31
	addiu	$sp,$sp,56
	.set	macro
	.set	reorder

$L215:
	.set	noreorder
	.set	nomacro
	jal	rtl_skb_headroom
	move	$4,$16
	.set	macro
	.set	reorder

	sltu	$2,$2,22
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L188
	move	$2,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_skb_cloned
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L188
	move	$2,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_skb_shared
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L188
	move	$2,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_dev
	move	$4,$16
	.set	macro
	.set	reorder

	move	$7,$0
	move	$6,$0
	sw	$0,24($sp)
	move	$5,$0
	sw	$0,20($sp)
	sw	$0,16($sp)
	.set	noreorder
	.set	nomacro
	jal	find_pppoe_info
	move	$4,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L191
	move	$17,$2
	.set	macro
	.set	reorder

	lb	$2,16($2)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L188
	move	$2,$0
	.set	macro
	.set	reorder

	lb	$2,0($17)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L188
	move	$2,$0
	.set	macro
	.set	reorder

	lhu	$2,52($17)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L188
	move	$2,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_dev
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_len
	move	$4,$16
	.set	macro
	.set	reorder

	move	$4,$17
	.set	noreorder
	.set	nomacro
	jal	rtl_get_dev_by_name
	move	$18,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L191
	move	$19,$2
	.set	macro
	.set	reorder

	li	$5,22			# 0x16
	.set	noreorder
	.set	nomacro
	jal	rtl_skb_push
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	move	$4,$16
	.set	macro
	.set	reorder

	li	$6,6			# 0x6
	addiu	$5,$17,38
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$4,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_dev_addr
	move	$4,$19
	.set	macro
	.set	reorder

	move	$4,$16
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	move	$20,$2
	.set	macro
	.set	reorder

	addiu	$4,$2,6
	li	$6,6			# 0x6
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$5,$20
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	move	$4,$16
	.set	macro
	.set	reorder

	li	$3,25736			# 0x6488
	move	$4,$16
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	sh	$3,12($2)
	.set	macro
	.set	reorder

	li	$6,1			# 0x1
	lbu	$4,14($2)
	addiu	$3,$18,2
	sb	$0,15($2)
	move	$5,$19
	wsbh	$3,$3
	ins	$4,$6,4,4
	andi	$3,$3,0xffff
	ins	$4,$6,0,4
	srl	$7,$3,8
	sb	$4,14($2)
	move	$4,$16
	lhu	$6,52($17)
	sb	$3,18($2)
	sb	$7,19($2)
	wsbh	$3,$6
	andi	$3,$3,0xffff
	srl	$6,$3,8
	sb	$3,16($2)
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_dev
	sb	$6,17($2)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	move	$4,$16
	.set	macro
	.set	reorder

	li	$3,8448			# 0x2100
	move	$4,$16
	.set	noreorder
	.set	nomacro
	jal	ip_finish_output4
	sh	$3,20($2)
	.set	macro
	.set	reorder

	lui	$3,%hi(jiffies)
	li	$2,1			# 0x1
	lw	$3,%lo(jiffies)($3)
	.set	noreorder
	.set	nomacro
	b	$L188
	sw	$3,60($17)
	.set	macro
	.set	reorder

	.end	fast_pppoe_xmit
	.size	fast_pppoe_xmit, .-fast_pppoe_xmit
	.align	2
	.globl	fast_pppoe_xmit6
	.set	nomips16
	.set	nomicromips
	.ent	fast_pppoe_xmit6
	.type	fast_pppoe_xmit6, @function
fast_pppoe_xmit6:
	.frame	$sp,56,$31		# vars= 0, regs= 6/0, args= 32, gp= 0
	.mask	0x801f0000,-4
	.fmask	0x00000000,0
	lui	$2,%hi(fast_pppoe_fw)
	lw	$2,%lo(fast_pppoe_fw)($2)
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L244
	move	$2,$0
	.set	macro
	.set	reorder

	jr	$31
$L244:
	addiu	$sp,$sp,-56
	sw	$31,52($sp)
	sw	$20,48($sp)
	sw	$19,44($sp)
	sw	$18,40($sp)
	sw	$17,36($sp)
	sw	$16,32($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_dev
	move	$16,$4
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L218
	move	$2,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_dev
	move	$4,$16
	.set	macro
	.set	reorder

	lui	$3,%hi($LC5)
	li	$5,3			# 0x3
	addiu	$3,$3,%lo($LC5)
 #APP
 # 108 "./arch/mips/include/asm/string.h" 1
	.set	noreorder
	.set	noat
1:	lbu	$4,($2)
	beqz	$5,2f
	lbu	$1,($3)
	subu	$5,1
	bne	$1,$4,3f
	addiu	$2,1
	bnez	$4,1b
	addiu	$3,1
2:
	move	$4,$1
3:	subu	$4,$1
	.set	at
	.set	reorder
 # 0 "" 2
 #NO_APP
	beq	$4,$0,$L245
$L221:
	move	$2,$0
$L218:
	lw	$31,52($sp)
	lw	$20,48($sp)
	lw	$19,44($sp)
	lw	$18,40($sp)
	lw	$17,36($sp)
	lw	$16,32($sp)
	.set	noreorder
	.set	nomacro
	jr	$31
	addiu	$sp,$sp,56
	.set	macro
	.set	reorder

$L245:
	.set	noreorder
	.set	nomacro
	jal	rtl_skb_headroom
	move	$4,$16
	.set	macro
	.set	reorder

	sltu	$2,$2,22
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L218
	move	$2,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_skb_cloned
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L218
	move	$2,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_skb_shared
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L218
	move	$2,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_dev
	move	$4,$16
	.set	macro
	.set	reorder

	move	$7,$0
	move	$6,$0
	sw	$0,24($sp)
	move	$5,$0
	sw	$0,20($sp)
	sw	$0,16($sp)
	.set	noreorder
	.set	nomacro
	jal	find_pppoe_info
	move	$4,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L221
	move	$17,$2
	.set	macro
	.set	reorder

	lb	$2,16($2)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L218
	move	$2,$0
	.set	macro
	.set	reorder

	lb	$2,0($17)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L218
	move	$2,$0
	.set	macro
	.set	reorder

	lhu	$2,52($17)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L218
	move	$2,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_dev
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_len
	move	$4,$16
	.set	macro
	.set	reorder

	move	$4,$17
	.set	noreorder
	.set	nomacro
	jal	rtl_get_dev_by_name
	move	$18,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L221
	move	$19,$2
	.set	macro
	.set	reorder

	li	$5,22			# 0x16
	.set	noreorder
	.set	nomacro
	jal	rtl_skb_push
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	move	$4,$16
	.set	macro
	.set	reorder

	li	$6,6			# 0x6
	addiu	$5,$17,38
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$4,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_dev_addr
	move	$4,$19
	.set	macro
	.set	reorder

	move	$4,$16
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	move	$20,$2
	.set	macro
	.set	reorder

	addiu	$4,$2,6
	li	$6,6			# 0x6
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$5,$20
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	move	$4,$16
	.set	macro
	.set	reorder

	li	$3,25736			# 0x6488
	move	$4,$16
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	sh	$3,12($2)
	.set	macro
	.set	reorder

	li	$6,1			# 0x1
	lbu	$4,14($2)
	addiu	$3,$18,2
	sb	$0,15($2)
	move	$5,$19
	wsbh	$3,$3
	ins	$4,$6,4,4
	andi	$3,$3,0xffff
	ins	$4,$6,0,4
	srl	$7,$3,8
	sb	$4,14($2)
	move	$4,$16
	lhu	$6,52($17)
	sb	$3,18($2)
	sb	$7,19($2)
	wsbh	$3,$6
	andi	$3,$3,0xffff
	srl	$6,$3,8
	sb	$3,16($2)
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_dev
	sb	$6,17($2)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	move	$4,$16
	.set	macro
	.set	reorder

	li	$3,22272			# 0x5700
	move	$4,$16
	.set	noreorder
	.set	nomacro
	jal	ip_finish_output4
	sh	$3,20($2)
	.set	macro
	.set	reorder

	lui	$4,%hi(statistic_ipv6_pppoe_xmit)
	lui	$5,%hi(jiffies)
	li	$2,1			# 0x1
	lw	$3,%lo(statistic_ipv6_pppoe_xmit)($4)
	lw	$5,%lo(jiffies)($5)
	addiu	$3,$3,1
	sw	$5,60($17)
	.set	noreorder
	.set	nomacro
	b	$L218
	sw	$3,%lo(statistic_ipv6_pppoe_xmit)($4)
	.set	macro
	.set	reorder

	.end	fast_pppoe_xmit6
	.size	fast_pppoe_xmit6, .-fast_pppoe_xmit6
	.globl	fastpath_pppoe_proc_fops
	.data
	.align	2
	.type	fastpath_pppoe_proc_fops, @object
	.size	fastpath_pppoe_proc_fops, 108
fastpath_pppoe_proc_fops:
	.space	4
	.word	seq_lseek
	.word	seq_read
	.word	fastpath_pppoe_single_write
	.space	28
	.word	fastpath_pppoe_single_open
	.space	4
	.word	single_release
	.space	52
	.globl	fast_pppoe_proc
	.section	.bss,"aw",@nobits
	.align	2
	.type	fast_pppoe_proc, @object
	.size	fast_pppoe_proc, 4
fast_pppoe_proc:
	.space	4
	.globl	fast_pppoe_info
	.align	2
	.type	fast_pppoe_info, @object
	.size	fast_pppoe_info, 1408
fast_pppoe_info:
	.space	1408
	.globl	fast_pppoe_fw
	.align	2
	.type	fast_pppoe_fw, @object
	.size	fast_pppoe_fw, 4
fast_pppoe_fw:
	.space	4
	.ident	"GCC: (Realtek MSDK-6.4.1 Build 3055) 6.4.1 20180425"
