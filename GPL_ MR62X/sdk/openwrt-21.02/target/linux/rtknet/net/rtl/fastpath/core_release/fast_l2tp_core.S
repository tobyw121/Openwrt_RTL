	.file	1 "fast_l2tp_core.c"
	.section .mdebug.abi32
	.previous
	.nan	legacy
	.module	softfloat
	.module	oddspreg
	.section	.rodata.str1.4,"aMS",@progbits,1
	.align	2
$LC0:
	.ascii	"ppp0\000"
	.text
	.align	2
	.globl	l2tp_id
	.set	nomips16
	.set	nomicromips
	.ent	l2tp_id
	.type	l2tp_id, @function
l2tp_id:
	.frame	$sp,40,$31		# vars= 0, regs= 5/0, args= 16, gp= 0
	.mask	0x800f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-40
	sw	$31,36($sp)
	sw	$19,32($sp)
	sw	$18,28($sp)
	sw	$17,24($sp)
	sw	$16,20($sp)
	move	$17,$4
	move	$18,$5
	jal	rtl_get_skb_data
	move	$16,$6

	move	$4,$17
	jal	rtl_ip_hdr
	move	$19,$2

	beq	$2,$0,$L62
	lw	$31,36($sp)

	lw	$3,0($2)
	andi	$3,$3,0xf
	sll	$3,$3,2
	beq	$16,$0,$L48
	addu	$3,$2,$3

	lhu	$5,0($3)
	li	$4,42246			# 0xa506
	beq	$5,$4,$L54
	nop

$L48:
	beq	$18,$0,$L1
	li	$5,42246			# 0xa506

	lhu	$4,2($3)
	beq	$4,$5,$L55
	nop

	bne	$16,$0,$L50
	nop

$L1:
	lw	$31,36($sp)
$L62:
$L66:
	lw	$19,32($sp)
$L64:
	lw	$18,28($sp)
	lw	$17,24($sp)
	lw	$16,20($sp)
	jr	$31
	addiu	$sp,$sp,40

$L54:
	lhu	$5,8($3)
	andi	$4,$5,0x80
	beq	$4,$0,$L25
	nop

$L24:
	lhu	$4,10($3)
	sltu	$4,$4,5120
	bne	$4,$0,$L1
	addiu	$6,$3,10

	addiu	$7,$3,12
	andi	$4,$5,0x40
	move	$5,$6
	movn	$5,$7,$4
	li	$6,1024			# 0x400
	lhu	$4,14($5)
	beq	$4,$6,$L56
	li	$6,2816			# 0xb00

	beq	$4,$6,$L57
	li	$6,3072			# 0xc00

	bne	$4,$6,$L62
	lw	$31,36($sp)

	lhu	$4,2($5)
	beq	$4,$0,$L62
	lui	$16,%hi(state)

	lw	$4,%lo(state)($16)
	sltu	$4,$4,2
	beq	$4,$0,$L64
	lw	$19,32($sp)

	lui	$17,%hi(l2tpInfo)
	lhu	$4,0($5)
	addiu	$17,$17,%lo(l2tpInfo)
	sh	$4,20($17)
	lhu	$4,2($5)
	sh	$4,22($17)
	lw	$4,12($2)
	sw	$4,16($17)
	lw	$4,16($2)
	sw	$4,12($17)
	lhu	$2,0($3)
	jal	filter_addconnect
	sh	$2,40($17)

	lui	$4,%hi($LC0)
	li	$2,1			# 0x1
	li	$5,4			# 0x4
	addiu	$4,$4,%lo($LC0)
	jal	rtl865x_setNetifType
	sh	$2,38($17)

	li	$2,2			# 0x2
	b	$L1
	sw	$2,%lo(state)($16)

$L55:
	lhu	$5,8($3)
	andi	$6,$5,0x80
	bne	$6,$0,$L24
	nop

	beq	$16,$0,$L65
	li	$2,42246			# 0xa506

$L50:
	lhu	$5,0($3)
	li	$2,42246			# 0xa506
	beq	$5,$2,$L25
	nop

$L65:
	bne	$4,$2,$L62
	lw	$31,36($sp)

$L25:
	lhu	$16,8($3)
	lui	$18,%hi(l2tpInfo)
	addiu	$2,$3,10
	addiu	$17,$18,%lo(l2tpInfo)
	addiu	$3,$3,12
	andi	$16,$16,0x40
	movn	$2,$3,$16
	lw	$4,12($17)
	jal	filter_checkConnect
	move	$16,$2

	bne	$2,$0,$L58
	nop

	lhu	$3,6($16)
$L63:
	li	$2,8640			# 0x21c0
	bne	$3,$2,$L62
	lw	$31,36($sp)

	lbu	$2,8($16)
	li	$3,5			# 0x5
	beq	$2,$3,$L59
	li	$6,44			# 0x2c

$L18:
	li	$3,6			# 0x6
	beq	$2,$3,$L60
	nop

$L19:
	li	$3,1			# 0x1
	bne	$2,$3,$L62
	lw	$31,36($sp)

	lhu	$5,10($16)
	wsbh	$5,$5
	andi	$5,$5,0xffff
	slt	$2,$5,5
	bne	$2,$0,$L66
	addiu	$6,$16,8

	lbu	$4,12($16)
	li	$2,5			# 0x5
	beq	$4,$2,$L28
	lbu	$3,13($16)

	li	$2,4			# 0x4
	b	$L22
	li	$7,5			# 0x5

$L23:
	lbu	$4,0($3)
	beq	$4,$7,$L61
	lbu	$3,1($3)

$L22:
	addu	$2,$2,$3
	slt	$4,$2,$5
	bne	$4,$0,$L23
	addu	$3,$6,$2

	b	$L62
	lw	$31,36($sp)

$L58:
	jal	filter_addconnect
	lw	$4,12($17)

	b	$L63
	lhu	$3,6($16)

$L57:
	lui	$16,%hi(state)
	lw	$2,%lo(state)($16)
	bne	$2,$0,$L62
	lw	$31,36($sp)

	jal	rtl_is_wan_dev
	move	$4,$17

	beq	$2,$0,$L62
	lw	$31,36($sp)

	jal	rtl_get_skb_dev
	move	$4,$17

	lui	$4,%hi(l2tpInfo+24)
	lui	$3,%hi(l2tpInfo)
	li	$6,6			# 0x6
	addiu	$5,$19,-8
	addiu	$4,$4,%lo(l2tpInfo+24)
	jal	memcpy
	sw	$2,%lo(l2tpInfo)($3)

	lui	$4,%hi(l2tpInfo+30)
	li	$6,6			# 0x6
	addiu	$5,$19,-14
	jal	memcpy
	addiu	$4,$4,%lo(l2tpInfo+30)

	lui	$4,%hi(l2tpInfo+36)
	li	$6,2			# 0x2
	addiu	$5,$19,-2
	jal	memcpy
	addiu	$4,$4,%lo(l2tpInfo+36)

	li	$2,1			# 0x1
	b	$L1
	sw	$2,%lo(state)($16)

$L60:
	lhu	$3,20($17)
	lhu	$2,0($16)
	bne	$3,$2,$L62
	lw	$31,36($sp)

	lhu	$3,22($17)
	lhu	$2,2($16)
	bne	$3,$2,$L62
	li	$6,44			# 0x2c

	move	$5,$0
	jal	memset
	move	$4,$17

	lui	$3,%hi(state)
	lbu	$2,8($16)
	b	$L19
	sw	$0,%lo(state)($3)

$L56:
	lui	$4,%hi(l2tpInfo)
	lhu	$2,0($5)
	addiu	$4,$4,%lo(l2tpInfo)
	lhu	$3,20($4)
	bne	$3,$2,$L62
	lw	$31,36($sp)

	li	$6,44			# 0x2c
	jal	memset
	move	$5,$0

	lui	$4,%hi($LC0)
	li	$5,2			# 0x2
	jal	rtl865x_setNetifType
	addiu	$4,$4,%lo($LC0)

	lui	$2,%hi(state)
	b	$L1
	sw	$0,%lo(state)($2)

$L61:
	addiu	$2,$2,2
$L20:
	addu	$2,$6,$2
	lw	$3,0($2)
	lui	$2,%hi(l2tp_ppp_imagic)
	b	$L1
	sw	$3,%lo(l2tp_ppp_imagic)($2)

$L59:
	move	$5,$0
	jal	memset
	addiu	$4,$18,%lo(l2tpInfo)

	lui	$3,%hi(state)
	lbu	$2,8($16)
	b	$L18
	sw	$0,%lo(state)($3)

$L28:
	b	$L20
	li	$2,6			# 0x6

	.set	macro
	.set	reorder
	.end	l2tp_id
	.size	l2tp_id, .-l2tp_id
	.align	2
	.globl	fastpath_l2tp_single_open
	.set	nomips16
	.set	nomicromips
	.ent	fastpath_l2tp_single_open
	.type	fastpath_l2tp_single_open, @function
fastpath_l2tp_single_open:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	move	$4,$5
	lui	$5,%hi(l2tp_read_proc)
	move	$6,$0
	addiu	$5,$5,%lo(l2tp_read_proc)
	j	single_open
	nop

	.set	macro
	.set	reorder
	.end	fastpath_l2tp_single_open
	.size	fastpath_l2tp_single_open, .-fastpath_l2tp_single_open
	.section	.rodata.str1.4
	.align	2
$LC1:
	.ascii	"%c\012\000"
	.text
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	l2tp_read_proc
	.type	l2tp_read_proc, @function
l2tp_read_proc:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	lui	$2,%hi(fast_l2tp_fw)
	lui	$5,%hi($LC1)
	lw	$6,%lo(fast_l2tp_fw)($2)
	addiu	$5,$5,%lo($LC1)
	jal	seq_printf
	addiu	$6,$6,48

	move	$2,$0
	lw	$31,20($sp)
	jr	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	l2tp_read_proc
	.size	l2tp_read_proc, .-l2tp_read_proc
	.section	.rodata.str1.4
	.align	2
$LC2:
	.ascii	"net/rtl/fastpath/core/fast_l2tp_core.c\000"
	.text
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	fastpath_l2tp_single_write
	.type	fastpath_l2tp_single_write, @function
fastpath_l2tp_single_write:
	.frame	$sp,40,$31		# vars= 8, regs= 3/0, args= 16, gp= 0
	.mask	0x80030000,-4
	.fmask	0x00000000,0
	sltu	$2,$6,2
	bne	$2,$0,$L82
	beq	$5,$0,$L82
	addiu	$sp,$sp,-40
	sw	$31,36($sp)
	sw	$17,32($sp)
	sw	$16,28($sp)
	addiu	$3,$5,1
	lw	$2,24($28)
	move	$16,$5
	or	$3,$3,$5
	and	$3,$3,$2
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L74
	move	$17,$6
	.set	macro
	.set	reorder

	lui	$4,%hi($LC2)
	li	$5,738			# 0x2e2
	.set	noreorder
	.set	nomacro
	jal	__might_fault
	addiu	$4,$4,%lo($LC2)
	.set	macro
	.set	reorder

	addiu	$4,$sp,16
	move	$5,$16
	li	$6,1			# 0x1
 #APP
 # 748 "net/rtl/fastpath/core/fast_l2tp_core.c" 1
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
	bne	$6,$0,$L85
	lb	$3,16($sp)
	.set	macro
	.set	reorder

	lui	$4,%hi(fast_l2tp_fw)
	li	$2,1			# 0x1
	addiu	$3,$3,-48
	sw	$3,%lo(fast_l2tp_fw)($4)
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L86
	lui	$4,%hi(l2tpInfo)
	.set	macro
	.set	reorder

	addiu	$2,$4,%lo(l2tpInfo)
	lui	$3,%hi(l2tp_tx_id_hook)
	sh	$0,20($2)
	sw	$0,%lo(l2tp_tx_id_hook)($3)
$L77:
	li	$6,44			# 0x2c
	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	memset
	addiu	$4,$4,%lo(l2tpInfo)
	.set	macro
	.set	reorder

	move	$2,$17
	lui	$3,%hi(state)
	lw	$31,36($sp)
	lw	$17,32($sp)
	lw	$16,28($sp)
	sw	$0,%lo(state)($3)
	.set	noreorder
	.set	nomacro
	jr	$31
	addiu	$sp,$sp,40
	.set	macro
	.set	reorder

$L86:
	lui	$2,%hi(l2tp_id)
	lui	$3,%hi(l2tp_tx_id_hook)
	addiu	$2,$2,%lo(l2tp_id)
	.set	noreorder
	.set	nomacro
	b	$L77
	sw	$2,%lo(l2tp_tx_id_hook)($3)
	.set	macro
	.set	reorder

$L74:
	li	$6,1			# 0x1
	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	memset
	addiu	$4,$sp,16
	.set	macro
	.set	reorder

$L85:
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

$L82:
	.set	noreorder
	.set	nomacro
	jr	$31
	li	$2,-14			# 0xfffffffffffffff2
	.set	macro
	.set	reorder

	.end	fastpath_l2tp_single_write
	.size	fastpath_l2tp_single_write, .-fastpath_l2tp_single_write
	.section	.rodata.str1.4
	.align	2
$LC3:
	.ascii	"fast_l2tp\000"
	.section	.init.text,"ax",@progbits
	.align	2
	.globl	fast_l2tp_init
	.set	nomips16
	.set	nomicromips
	.ent	fast_l2tp_init
	.type	fast_l2tp_init, @function
fast_l2tp_init:
	.frame	$sp,32,$31		# vars= 0, regs= 1/0, args= 24, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	lui	$7,%hi(fastpath_l2tp_proc_fops)
	lui	$6,%hi(proc_root)
	sw	$0,16($sp)
	lui	$4,%hi($LC3)
	addiu	$7,$7,%lo(fastpath_l2tp_proc_fops)
	addiu	$6,$6,%lo(proc_root)
	move	$5,$0
	jal	proc_create_data
	addiu	$4,$4,%lo($LC3)

	lui	$3,%hi(res1)
	lui	$4,%hi(l2tpInfo)
	li	$6,44			# 0x2c
	sw	$2,%lo(res1)($3)
	lui	$2,%hi(l2tp_tx_id_hook)
	move	$5,$0
	addiu	$4,$4,%lo(l2tpInfo)
	sw	$0,%lo(l2tp_tx_id_hook)($2)
	lui	$2,%hi(fast_l2tp_fw)
	sw	$0,%lo(fast_l2tp_fw)($2)
	lui	$2,%hi(state)
	jal	memset
	sw	$0,%lo(state)($2)

	move	$2,$0
	lw	$31,28($sp)
	jr	$31
	addiu	$sp,$sp,32

	.set	macro
	.set	reorder
	.end	fast_l2tp_init
	.size	fast_l2tp_init, .-fast_l2tp_init
	.section	.exit.text,"ax",@progbits
	.align	2
	.globl	fast_l2tp_exit
	.set	nomips16
	.set	nomicromips
	.ent	fast_l2tp_exit
	.type	fast_l2tp_exit, @function
fast_l2tp_exit:
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$16,16($sp)
	lui	$16,%hi(res1)
	lw	$2,%lo(res1)($16)
	beq	$2,$0,$L89
	lui	$5,%hi(proc_root)

	lui	$4,%hi($LC3)
	addiu	$5,$5,%lo(proc_root)
	jal	remove_proc_entry
	addiu	$4,$4,%lo($LC3)

	sw	$0,%lo(res1)($16)
$L89:
	lw	$31,20($sp)
	lw	$16,16($sp)
	jr	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	fast_l2tp_exit
	.size	fast_l2tp_exit, .-fast_l2tp_exit
	.text
	.align	2
	.globl	is_l2tp_device
	.set	nomips16
	.set	nomicromips
	.ent	is_l2tp_device
	.type	is_l2tp_device, @function
is_l2tp_device:
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	lui	$2,%hi(l2tpInfo+4)
	lw	$2,%lo(l2tpInfo+4)($2)
	beq	$2,$0,$L97
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$16,16($sp)
	move	$16,$4
	.set	noreorder
	.set	nomacro
	jal	rtl_get_ppp_dev_name
	move	$4,$2
	.set	macro
	.set	reorder

	move	$4,$16
 #APP
 # 77 "./arch/mips/include/asm/string.h" 1
	.set	noreorder
	.set	noat
	lbu	$3,($2)
1:	lbu	$1,($4)
	addiu	$2,1
	bne	$1,$3,2f
	addiu	$4,1
	bnez	$3,1b
	lbu	$3,($2)
	move	$3,$1
2:	subu	$3,$1
3:	.set	at
	.set	reorder
 # 0 "" 2
 #NO_APP
	lw	$31,20($sp)
	sltu	$2,$3,1
	lw	$16,16($sp)
	.set	noreorder
	.set	nomacro
	jr	$31
	addiu	$sp,$sp,24
	.set	macro
	.set	reorder

$L97:
	.set	noreorder
	.set	nomacro
	jr	$31
	move	$2,$0
	.set	macro
	.set	reorder

	.end	is_l2tp_device
	.size	is_l2tp_device, .-is_l2tp_device
	.align	2
	.globl	set_l2tp_device
	.set	nomips16
	.set	nomicromips
	.ent	set_l2tp_device
	.type	set_l2tp_device, @function
set_l2tp_device:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	lui	$2,%hi(fast_l2tp_fw)
	lw	$2,%lo(fast_l2tp_fw)($2)
	bne	$2,$0,$L112
	nop

	jr	$31
	nop

$L112:
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	jal	rtl_get_dev_by_name
	nop

	lui	$3,%hi(l2tpInfo+4)
	lw	$31,20($sp)
	sw	$2,%lo(l2tpInfo+4)($3)
	jr	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	set_l2tp_device
	.size	set_l2tp_device, .-set_l2tp_device
	.align	2
	.globl	event_ppp_dev_down
	.set	nomips16
	.set	nomicromips
	.ent	event_ppp_dev_down
	.type	event_ppp_dev_down, @function
event_ppp_dev_down:
	.frame	$sp,32,$31		# vars= 0, regs= 3/0, args= 16, gp= 0
	.mask	0x80030000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$17,24($sp)
	sw	$16,20($sp)
	lui	$17,%hi(l2tpInfo)
	addiu	$17,$17,%lo(l2tpInfo)
	lhu	$2,38($17)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L122
	lw	$31,28($sp)
	.set	macro
	.set	reorder

	lw	$2,4($17)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L122
	move	$16,$4
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_ppp_dev_name
	move	$4,$2
	.set	macro
	.set	reorder

	move	$4,$16
 #APP
 # 77 "./arch/mips/include/asm/string.h" 1
	.set	noreorder
	.set	noat
	lbu	$3,($2)
1:	lbu	$1,($4)
	addiu	$2,1
	bne	$1,$3,2f
	addiu	$4,1
	bnez	$3,1b
	lbu	$3,($2)
	move	$3,$1
2:	subu	$3,$1
3:	.set	at
	.set	reorder
 # 0 "" 2
 #NO_APP
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L121
	li	$6,44			# 0x2c
	.set	macro
	.set	reorder

	lw	$31,28($sp)
$L122:
	lw	$17,24($sp)
	lw	$16,20($sp)
	.set	noreorder
	.set	nomacro
	jr	$31
	addiu	$sp,$sp,32
	.set	macro
	.set	reorder

$L121:
	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	memset
	move	$4,$17
	.set	macro
	.set	reorder

	lui	$2,%hi(state)
	lw	$31,28($sp)
	lw	$17,24($sp)
	lw	$16,20($sp)
	sw	$0,%lo(state)($2)
	.set	noreorder
	.set	nomacro
	jr	$31
	addiu	$sp,$sp,32
	.set	macro
	.set	reorder

	.end	event_ppp_dev_down
	.size	event_ppp_dev_down, .-event_ppp_dev_down
	.align	2
	.globl	fast_l2tp_rx
	.set	nomips16
	.set	nomicromips
	.ent	fast_l2tp_rx
	.type	fast_l2tp_rx, @function
fast_l2tp_rx:
	.frame	$sp,48,$31		# vars= 0, regs= 7/0, args= 16, gp= 0
	.mask	0x803f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-48
	sw	$31,44($sp)
	sw	$21,40($sp)
	sw	$20,36($sp)
	sw	$19,32($sp)
	sw	$18,28($sp)
	sw	$17,24($sp)
	sw	$16,20($sp)
	jal	rtl_ip_hdr
	move	$17,$4

	move	$4,$17
	move	$18,$2
	lw	$2,0($2)
	jal	rtl_get_skb_data
	andi	$16,$2,0xf

	move	$4,$17
	jal	rtl_get_skb_len
	move	$19,$2

	sltu	$2,$2,40
	bne	$2,$0,$L154
	sll	$2,$16,2

	li	$3,42246			# 0xa506
	addu	$2,$18,$2
	lhu	$2,0($2)
	beq	$2,$3,$L155
	lui	$20,%hi(l2tpInfo)

$L154:
	move	$2,$0
$L123:
	lw	$31,44($sp)
	lw	$21,40($sp)
	lw	$20,36($sp)
	lw	$19,32($sp)
	lw	$18,28($sp)
	lw	$17,24($sp)
	lw	$16,20($sp)
	jr	$31
	addiu	$sp,$sp,48

$L155:
	lw	$2,%lo(l2tpInfo)($20)
	beq	$2,$0,$L156
	li	$6,1			# 0x1

	lbu	$2,28($19)
$L158:
	move	$6,$17
	move	$4,$19
	ext	$16,$2,6,1
	andi	$2,$2,0x2
	sll	$16,$16,1
	addu	$16,$16,$2
	jal	fast_l2tp_tx_lcp_echo_reply
	move	$5,$16

	li	$3,1			# 0x1
	beq	$2,$3,$L123
	addiu	$21,$20,%lo(l2tpInfo)

	lhu	$2,20($21)
	beq	$2,$0,$L123
	move	$2,$0

	lhu	$2,22($21)
	beq	$2,$0,$L154
	addu	$2,$19,$16

	lbu	$3,36($2)
	bne	$3,$0,$L154
	nop

	lbu	$3,37($2)
	li	$2,33			# 0x21
	bne	$3,$2,$L123
	move	$2,$0

	lw	$2,%lo(l2tpInfo)($20)
	beq	$2,$0,$L123
	move	$2,$0

	lw	$3,16($21)
	lw	$2,16($18)
	bne	$3,$2,$L123
	move	$2,$0

	lw	$5,4($21)
	beq	$5,$0,$L157
	lui	$4,%hi($LC0)

$L130:
	addiu	$16,$16,38
	li	$3,6			# 0x6
	addu	$19,$19,$16
	lbu	$2,9($19)
	beq	$2,$3,$L131
	li	$3,1			# 0x1

	andi	$4,$2,0xef
	beq	$4,$3,$L131
	li	$3,47			# 0x2f

	bne	$2,$3,$L123
	move	$2,$0

$L131:
	jal	rtl_set_skb_dev
	move	$4,$17

	move	$5,$16
	jal	skb_pull
	move	$4,$17

	jal	rtl_skb_reset_network_header
	move	$4,$17

	jal	rtl_skb_reset_transport_header
	move	$4,$17

	lw	$4,4($21)
	beq	$4,$0,$L123
	move	$2,$0

	jal	rtl_get_ppp_dev_priv
	nop

	beq	$2,$0,$L154
	nop

	jal	rtl_get_ppp_dev_priv
	lw	$4,4($21)

	move	$4,$17
	jal	rtl_get_skb_len
	move	$16,$2

	move	$5,$0
	move	$6,$2
	jal	rtl_inc_ppp_stats
	move	$4,$16

	b	$L123
	move	$2,$0

$L156:
	move	$5,$0
	jal	l2tp_id
	move	$4,$17

	b	$L158
	lbu	$2,28($19)

$L157:
	jal	rtl_get_dev_by_name
	addiu	$4,$4,%lo($LC0)

	beq	$2,$0,$L154
	sw	$2,4($21)

	b	$L130
	move	$5,$2

	.set	macro
	.set	reorder
	.end	fast_l2tp_rx
	.size	fast_l2tp_rx, .-fast_l2tp_rx
	.align	2
	.globl	get_fast_l2tp_lastxmit
	.set	nomips16
	.set	nomicromips
	.ent	get_fast_l2tp_lastxmit
	.type	get_fast_l2tp_lastxmit, @function
get_fast_l2tp_lastxmit:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	lui	$2,%hi(l2tpInfo)
	li	$3,1			# 0x1
	addiu	$2,$2,%lo(l2tpInfo)
	lhu	$4,38($2)
	beq	$4,$3,$L162
	nop

	jr	$31
	move	$2,$0

$L162:
	jr	$31
	lw	$2,8($2)

	.set	macro
	.set	reorder
	.end	get_fast_l2tp_lastxmit
	.size	get_fast_l2tp_lastxmit, .-get_fast_l2tp_lastxmit
	.align	2
	.globl	check_for_fast_l2tp_to_wan
	.set	nomips16
	.set	nomicromips
	.ent	check_for_fast_l2tp_to_wan
	.type	check_for_fast_l2tp_to_wan, @function
check_for_fast_l2tp_to_wan:
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$16,16($sp)
	jal	rtl_ip_hdr
	move	$16,$4

	jal	rtl_ip_hdr
	move	$4,$16

	li	$3,-4259840			# 0xffffffffffbf0000
	lw	$2,12($2)
	ori	$3,$3,0xbff6
	lw	$31,20($sp)
	lw	$16,16($sp)
	ext	$2,$2,0,24
	addu	$2,$2,$3
	sltu	$2,$0,$2
	jr	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	check_for_fast_l2tp_to_wan
	.size	check_for_fast_l2tp_to_wan, .-check_for_fast_l2tp_to_wan
	.align	2
	.globl	fast_l2tp_to_wan
	.set	nomips16
	.set	nomicromips
	.ent	fast_l2tp_to_wan
	.type	fast_l2tp_to_wan, @function
fast_l2tp_to_wan:
	.frame	$sp,88,$31		# vars= 40, regs= 7/0, args= 16, gp= 0
	.mask	0x803f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	lui	$2,%hi(fast_l2tp_fw)
	lw	$2,%lo(fast_l2tp_fw)($2)
	beq	$2,$0,$L200
	nop

	addiu	$sp,$sp,-88
	sw	$31,84($sp)
	sw	$21,80($sp)
	sw	$20,76($sp)
	sw	$19,72($sp)
	sw	$18,68($sp)
	sw	$17,64($sp)
	sw	$16,60($sp)
	lui	$19,%hi(l2tpInfo)
	addiu	$18,$19,%lo(l2tpInfo)
	lhu	$2,20($18)
	beq	$2,$0,$L165
	move	$2,$0

	lhu	$2,22($18)
	beq	$2,$0,$L168
	lw	$2,%lo(l2tpInfo)($19)

	beq	$2,$0,$L168
	li	$2,1			# 0x1

	lhu	$5,38($18)
	beq	$5,$2,$L201
	nop

$L168:
	move	$2,$0
$L165:
	lw	$31,84($sp)
	lw	$21,80($sp)
	lw	$20,76($sp)
	lw	$19,72($sp)
	lw	$18,68($sp)
	lw	$17,64($sp)
	lw	$16,60($sp)
	jr	$31
	addiu	$sp,$sp,88

$L200:
	jr	$31
	move	$2,$0

$L201:
	jal	rtl_ip_hdr
	move	$20,$4

	lhu	$17,2($2)
	move	$4,$20
	jal	rtl_skb_headroom
	move	$21,$2

	sltu	$2,$2,52
	wsbh	$17,$17
	bne	$2,$0,$L171
	andi	$17,$17,0xffff

	jal	rtl_skb_cloned
	move	$4,$20

	beq	$2,$0,$L202
	nop

$L171:
	li	$5,52			# 0x34
$L205:
	jal	skb_realloc_headroom
	move	$4,$20

	beq	$2,$0,$L168
	move	$16,$2

	jal	consume_skb
	move	$4,$20

	li	$5,52			# 0x34
$L206:
	lbu	$20,1($21)
	jal	skb_push
	move	$4,$16

	lui	$5,%hi(l2tpInfo+24)
	li	$6,14			# 0xe
	addiu	$5,$5,%lo(l2tpInfo+24)
	jal	memcpy
	move	$4,$2

	li	$5,4			# 0x4
	lbu	$2,16($sp)
	li	$7,17			# 0x11
	lw	$4,12($18)
	li	$3,64			# 0x40
	lw	$6,16($18)
	ins	$2,$5,4,4
	sb	$7,25($sp)
	li	$7,5			# 0x5
	sw	$4,32($sp)
	move	$5,$0
	sh	$3,22($sp)
	ins	$2,$7,0,4
	sw	$6,28($sp)
	move	$4,$16
	sb	$3,24($sp)
	sb	$2,16($sp)
	jal	rtl_set_skb_ip_summed
	sb	$20,17($sp)

	jal	rtl_get_skb_len
	move	$4,$16

	addiu	$4,$sp,32
	addiu	$2,$2,-14
	sh	$0,20($sp)
	sh	$0,26($sp)
	wsbh	$3,$2
	lw	$2,20($sp)
	lw	$7,24($sp)
	sh	$3,18($sp)
	lw	$3,16($sp)
	lw	$6,28($sp)
	addu	$8,$3,$2
	andi	$5,$3,0xf
	addu	$3,$8,$7
	sltu	$2,$8,$2
	addu	$3,$3,$2
	sll	$5,$5,2
	addu	$2,$3,$6
	sltu	$7,$3,$7
	addu	$2,$2,$7
	addiu	$3,$sp,16
	sltu	$6,$2,$6
	addu	$5,$3,$5
	addu	$2,$2,$6
$L173:
	lw	$3,0($4)
	addiu	$4,$4,4
	addu	$2,$3,$2
	sltu	$3,$2,$3
	bne	$5,$4,$L173
	addu	$2,$2,$3

	sll	$3,$2,16
	move	$4,$16
	addu	$3,$3,$2
	addiu	$17,$17,18
	srl	$5,$3,16
	sltu	$2,$3,$2
	addu	$2,$2,$5
	wsbh	$17,$17
	nor	$2,$0,$2
	jal	rtl_get_skb_data
	sh	$2,26($sp)

	li	$6,20			# 0x14
	addiu	$4,$2,14
	jal	memcpy
	addiu	$5,$sp,16

	li	$5,512			# 0x200
	lhu	$2,22($18)
	lhu	$4,40($18)
	lhu	$3,20($18)
	sh	$2,48($sp)
	li	$2,1023			# 0x3ff
	sh	$4,36($sp)
	li	$4,-23290			# 0xffffffffffffa506
	sh	$2,50($sp)
	li	$2,8448			# 0x2100
	sh	$4,38($sp)
	move	$4,$16
	sh	$3,46($sp)
	sh	$5,44($sp)
	sh	$17,40($sp)
	sh	$2,52($sp)
	jal	rtl_get_skb_data
	sh	$0,42($sp)

	li	$6,18			# 0x12
	addiu	$5,$sp,36
	jal	memcpy
	addiu	$4,$2,34

	move	$4,$16
	jal	rtl_set_skb_dev
	lw	$5,%lo(l2tpInfo)($19)

	jal	rtl_get_skb_data
	move	$4,$16

	li	$5,52			# 0x34
	jal	timeoutCheck_skipp_pkt
	move	$4,$16

	li	$3,1			# 0x1
	beq	$2,$3,$L174
	lui	$2,%hi(jiffies)

	lw	$2,%lo(jiffies)($2)
	sw	$2,8($18)
$L174:
	lw	$4,4($18)
	beq	$4,$0,$L204
	lui	$2,%hi(gQosEnabled)

	jal	rtl_get_ppp_dev_priv
	nop

	beq	$2,$0,$L204
	lui	$2,%hi(gQosEnabled)

	jal	rtl_get_ppp_dev_priv
	lw	$4,4($18)

	move	$4,$16
	jal	rtl_get_skb_len
	move	$17,$2

	li	$5,1			# 0x1
	move	$6,$2
	jal	rtl_inc_ppp_stats
	move	$4,$17

	lui	$2,%hi(gQosEnabled)
$L204:
	lw	$2,%lo(gQosEnabled)($2)
	bne	$2,$0,$L203
	move	$4,$16

	jal	rtl_call_skb_ndo_start_xmit
	nop

	b	$L165
	li	$2,1			# 0x1

$L203:
	jal	dev_queue_xmit
	nop

	b	$L165
	li	$2,1			# 0x1

$L202:
	jal	rtl_skb_shared
	move	$4,$20

	bne	$2,$0,$L205
	li	$5,52			# 0x34

	b	$L206
	move	$16,$20

	.set	macro
	.set	reorder
	.end	fast_l2tp_to_wan
	.size	fast_l2tp_to_wan, .-fast_l2tp_to_wan
	.local	res1
	.comm	res1,4,4
	.globl	fastpath_l2tp_proc_fops
	.data
	.align	2
	.type	fastpath_l2tp_proc_fops, @object
	.size	fastpath_l2tp_proc_fops, 108
fastpath_l2tp_proc_fops:
	.space	4
	.word	seq_lseek
	.word	seq_read
	.word	fastpath_l2tp_single_write
	.space	28
	.word	fastpath_l2tp_single_open
	.space	4
	.word	single_release
	.space	52
	.globl	l2tp_ppp_imagic
	.section	.bss,"aw",@nobits
	.align	2
	.type	l2tp_ppp_imagic, @object
	.size	l2tp_ppp_imagic, 4
l2tp_ppp_imagic:
	.space	4
	.globl	state
	.align	2
	.type	state, @object
	.size	state, 4
state:
	.space	4
	.globl	l2tpInfo
	.align	2
	.type	l2tpInfo, @object
	.size	l2tpInfo, 44
l2tpInfo:
	.space	44
	.globl	fast_l2tp_fw
	.align	2
	.type	fast_l2tp_fw, @object
	.size	fast_l2tp_fw, 4
fast_l2tp_fw:
	.space	4
	.globl	l2tp_tx_id_hook
	.align	2
	.type	l2tp_tx_id_hook, @object
	.size	l2tp_tx_id_hook, 4
l2tp_tx_id_hook:
	.space	4
	.ident	"GCC: (Realtek MSDK-6.4.1 Build 3055) 6.4.1 20180425"
