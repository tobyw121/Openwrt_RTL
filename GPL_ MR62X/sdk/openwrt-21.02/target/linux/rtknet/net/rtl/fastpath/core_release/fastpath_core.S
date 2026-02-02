	.file	1 "fastpath_core.c"
	.section .mdebug.abi32
	.previous
	.nan	legacy
	.module	softfloat
	.module	oddspreg
	.text
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	cache_timeout
	.type	cache_timeout, @function
cache_timeout:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	lbu	$2,0($4)
	beq	$2,$0,$L9
	lui	$2,%hi(max_udp_frag_entry)

	lw	$3,8($4)
	lw	$5,4($4)
	lhu	$7,2($4)
	lbu	$6,1($4)
	xor	$5,$5,$3
	lw	$2,%lo(max_udp_frag_entry)($2)
	srl	$3,$5,16
	lw	$8,44($4)
	xor	$5,$7,$5
	sb	$0,0($4)
	xor	$3,$3,$5
	srl	$5,$3,8
	addiu	$2,$2,-1
	xor	$3,$6,$3
	xor	$3,$5,$3
	beq	$8,$0,$L3
	and	$2,$2,$3

	sll	$3,$2,1
	lw	$6,48($4)
	lui	$5,%hi(udp_cache_table)
	addu	$2,$3,$2
	sw	$6,48($8)
	sll	$3,$2,2
$L4:
	lw	$7,48($4)
	lui	$2,%hi(udp_cache_list_free)
	lw	$9,44($4)
	lui	$6,%hi(statistic_udp_frag_free)
	addiu	$2,$2,%lo(udp_cache_list_free)
	addiu	$8,$4,52
	sw	$9,0($7)
	lw	$5,%lo(udp_cache_table)($5)
	lw	$7,4($2)
	lw	$5,0($5)
	addu	$3,$5,$3
	lw	$5,8($3)
	addiu	$5,$5,-1
	sw	$5,8($3)
	sw	$0,52($4)
	sw	$7,56($4)
	sw	$4,0($7)
	lw	$4,8($2)
	lw	$3,%lo(statistic_udp_frag_free)($6)
	sw	$8,4($2)
	addiu	$4,$4,1
	addiu	$3,$3,1
	sw	$4,8($2)
	sw	$3,%lo(statistic_udp_frag_free)($6)
$L9:
	jr	$31
	nop

$L3:
	lui	$5,%hi(udp_cache_table)
	lw	$6,48($4)
	sll	$3,$2,1
	lw	$7,%lo(udp_cache_table)($5)
	addu	$2,$3,$2
	sll	$3,$2,2
	lw	$2,0($7)
	addu	$2,$2,$3
	b	$L4
	sw	$6,4($2)

	.set	macro
	.set	reorder
	.end	cache_timeout
	.size	cache_timeout, .-cache_timeout
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	negative_cache_timeout
	.type	negative_cache_timeout, @function
negative_cache_timeout:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	lw	$3,8($4)
	lui	$2,%hi(max_udp_frag_entry)
	lw	$5,4($4)
	lhu	$7,2($4)
	lbu	$6,1($4)
	xor	$5,$5,$3
	lw	$2,%lo(max_udp_frag_entry)($2)
	srl	$3,$5,16
	lw	$8,40($4)
	xor	$5,$7,$5
	sb	$0,0($4)
	xor	$3,$3,$5
	srl	$5,$3,8
	addiu	$2,$2,-1
	xor	$3,$6,$3
	xor	$3,$5,$3
	beq	$8,$0,$L11
	and	$2,$2,$3

	lw	$6,44($4)
	lui	$5,%hi(negative_cache_table)
	sll	$3,$2,1
	sw	$6,44($8)
$L12:
	lw	$6,44($4)
	addu	$2,$3,$2
	lw	$7,40($4)
	lui	$3,%hi(negative_cache_list_free)
	sll	$2,$2,2
	addiu	$3,$3,%lo(negative_cache_list_free)
	sw	$7,0($6)
	addiu	$7,$4,48
	lw	$5,%lo(negative_cache_table)($5)
	lw	$6,4($3)
	addu	$2,$5,$2
	lw	$5,8($2)
	addiu	$5,$5,-1
	sw	$5,8($2)
	sw	$0,48($4)
	sw	$6,52($4)
	sw	$4,0($6)
	lw	$2,8($3)
	sw	$7,4($3)
	addiu	$2,$2,1
	jr	$31
	sw	$2,8($3)

$L11:
	lui	$5,%hi(negative_cache_table)
	lw	$8,44($4)
	sll	$3,$2,1
	addu	$7,$3,$2
	lw	$6,%lo(negative_cache_table)($5)
	sll	$7,$7,2
	addu	$6,$6,$7
	b	$L12
	sw	$8,4($6)

	.set	macro
	.set	reorder
	.end	negative_cache_timeout
	.size	negative_cache_timeout, .-negative_cache_timeout
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	fastpath_path_single_write
	.type	fastpath_path_single_write, @function
fastpath_path_single_write:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	jr	$31
	move	$2,$6

	.set	macro
	.set	reorder
	.end	fastpath_path_single_write
	.size	fastpath_path_single_write, .-fastpath_path_single_write
	.align	2
	.globl	fastpath_napt_single_open
	.set	nomips16
	.set	nomicromips
	.ent	fastpath_napt_single_open
	.type	fastpath_napt_single_open, @function
fastpath_napt_single_open:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	move	$4,$5
	lui	$5,%hi(fastpath_table_napt)
	move	$6,$0
	addiu	$5,$5,%lo(fastpath_table_napt)
	j	single_open
	nop

	.set	macro
	.set	reorder
	.end	fastpath_napt_single_open
	.size	fastpath_napt_single_open, .-fastpath_napt_single_open
	.align	2
	.globl	fastpath_path_single_open
	.set	nomips16
	.set	nomicromips
	.ent	fastpath_path_single_open
	.type	fastpath_path_single_open, @function
fastpath_path_single_open:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	move	$4,$5
	lui	$5,%hi(fastpath_table_path)
	move	$6,$0
	addiu	$5,$5,%lo(fastpath_table_path)
	j	single_open
	nop

	.set	macro
	.set	reorder
	.end	fastpath_path_single_open
	.size	fastpath_path_single_open, .-fastpath_path_single_open
	.section	.rodata.str1.4,"aMS",@progbits,1
	.align	2
$LC0:
	.ascii	"TCP\000"
	.align	2
$LC1:
	.ascii	"UDP\000"
	.align	2
$LC2:
	.ascii	"~Napt: [%s] int=0x%08X:%-5u ext=0x%08X:%-5u rem=0x%08X:%"
	.ascii	"-5u flags=0x%08X \012\000"
	.text
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	fastpath_table_napt
	.type	fastpath_table_napt, @function
fastpath_table_napt:
	.frame	$sp,64,$31		# vars= 0, regs= 6/0, args= 40, gp= 0
	.mask	0x801f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-64
	sw	$31,60($sp)
	sw	$20,56($sp)
	sw	$19,52($sp)
	sw	$18,48($sp)
	sw	$17,44($sp)
	sw	$16,40($sp)
	lui	$2,%hi(napt_list_inuse)
	lw	$16,%lo(napt_list_inuse)($2)
	bne	$16,$0,$L25
	lui	$18,%hi($LC0)

	lw	$31,60($sp)
$L27:
	move	$2,$0
	lw	$20,56($sp)
	lw	$19,52($sp)
	lw	$18,48($sp)
	lw	$17,44($sp)
	lw	$16,40($sp)
	jr	$31
	addiu	$sp,$sp,64

$L25:
	lui	$17,%hi($LC1)
	lui	$20,%hi($LC2)
	li	$19,2			# 0x2
	addiu	$18,$18,%lo($LC0)
	addiu	$17,$17,%lo($LC1)
	lw	$2,32($16)
$L26:
	move	$3,$17
	lw	$5,4($16)
	addiu	$4,$20,%lo($LC2)
	lw	$7,12($16)
	lw	$6,8($16)
	sw	$2,32($sp)
	xor	$5,$5,$19
	lw	$2,28($16)
	movz	$3,$18,$5
	sw	$2,28($sp)
	lw	$2,24($16)
	move	$5,$3
	sw	$2,24($sp)
	lw	$2,20($16)
	sw	$2,20($sp)
	lw	$2,16($16)
	jal	printk
	sw	$2,16($sp)

	lw	$16,44($16)
	beq	$16,$0,$L27
	lw	$31,60($sp)

	b	$L26
	lw	$2,32($16)

	.set	macro
	.set	reorder
	.end	fastpath_table_napt
	.size	fastpath_table_napt, .-fastpath_table_napt
	.section	.rodata.str1.4
	.align	2
$LC3:
	.ascii	"~Path: [%s] in-S=0x%08X:%-5u in-D=0x%08X:%-5u out-S=0x%0"
	.ascii	"8X:%-5u out-D=0x%08X:%-5u out-ifname=%-5s <%u> {%d}\000"
	.align	2
$LC4:
	.ascii	",[PreMark:%d,PostMark:%d]\000"
	.align	2
$LC5:
	.ascii	",[policy_route_mark:%d]\000"
	.align	2
$LC6:
	.ascii	",[dscp_in:%d,dscp_out:%d]\000"
	.align	2
$LC7:
	.ascii	",[vlanPrio_out:%d]\000"
	.align	2
$LC8:
	.ascii	"\012\000"
	.text
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	fastpath_table_path
	.type	fastpath_table_path, @function
fastpath_table_path:
	.frame	$sp,104,$31		# vars= 8, regs= 10/0, args= 56, gp= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-104
	sw	$31,100($sp)
	sw	$fp,96($sp)
	sw	$23,92($sp)
	sw	$22,88($sp)
	sw	$21,84($sp)
	sw	$20,80($sp)
	sw	$19,76($sp)
	sw	$18,72($sp)
	sw	$17,68($sp)
	sw	$16,64($sp)
	lui	$2,%hi(path_list_inuse)
	lw	$16,%lo(path_list_inuse)($2)
	bne	$16,$0,$L37
	lui	$23,%hi($LC0)

$L35:
	lw	$31,100($sp)
	move	$2,$0
	lw	$fp,96($sp)
	lw	$23,92($sp)
	lw	$22,88($sp)
	lw	$21,84($sp)
	lw	$20,80($sp)
	lw	$19,76($sp)
	lw	$18,72($sp)
	lw	$17,68($sp)
	lw	$16,64($sp)
	jr	$31
	addiu	$sp,$sp,104

$L37:
	lui	$22,%hi($LC1)
	lui	$3,%hi($LC3)
	lui	$21,%hi($LC4)
	lui	$20,%hi($LC5)
	lui	$19,%hi($LC6)
	lui	$18,%hi($LC7)
	lui	$17,%hi($LC8)
	li	$fp,2			# 0x2
	addiu	$23,$23,%lo($LC0)
	addiu	$22,$22,%lo($LC1)
	lbu	$2,1($16)
$L38:
	addiu	$4,$3,%lo($LC3)
	lhu	$7,10($16)
	lw	$5,4($16)
	lw	$6,12($16)
	sw	$2,48($sp)
	lbu	$2,2($16)
	xor	$5,$5,$fp
	sw	$3,56($sp)
	move	$3,$22
	movz	$3,$23,$5
	sw	$2,44($sp)
	lw	$2,40($16)
	move	$5,$3
	sw	$2,40($sp)
	lhu	$2,32($16)
	sw	$2,36($sp)
	lw	$2,28($16)
	sw	$2,32($sp)
	lhu	$2,24($16)
	sw	$2,28($sp)
	lw	$2,20($16)
	sw	$2,24($sp)
	lhu	$2,8($16)
	sw	$2,20($sp)
	lw	$2,16($16)
	jal	printk
	sw	$2,16($sp)

	addiu	$4,$21,%lo($LC4)
	lw	$6,48($16)
	jal	printk
	lw	$5,44($16)

	addiu	$4,$20,%lo($LC5)
	jal	printk
	lw	$5,60($16)

	addiu	$4,$19,%lo($LC6)
	lbu	$6,73($16)
	jal	printk
	lbu	$5,72($16)

	addiu	$4,$18,%lo($LC7)
	jal	printk
	lhu	$5,78($16)

	jal	printk
	addiu	$4,$17,%lo($LC8)

	lw	$16,88($16)
	beq	$16,$0,$L35
	lw	$3,56($sp)

	b	$L38
	lbu	$2,1($16)

	.set	macro
	.set	reorder
	.end	fastpath_table_path
	.size	fastpath_table_path, .-fastpath_table_path
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	fastpath_napt_single_write
	.type	fastpath_napt_single_write, @function
fastpath_napt_single_write:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	jr	$31
	move	$2,$6

	.set	macro
	.set	reorder
	.end	fastpath_napt_single_write
	.size	fastpath_napt_single_write, .-fastpath_napt_single_write
	.align	2
	.globl	rtl_init_bypass_packet
	.set	nomips16
	.set	nomicromips
	.ent	rtl_init_bypass_packet
	.type	rtl_init_bypass_packet, @function
rtl_init_bypass_packet:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	sw	$0,68($4)
	jr	$31
	sw	$0,64($4)

	.set	macro
	.set	reorder
	.end	rtl_init_bypass_packet
	.size	rtl_init_bypass_packet, .-rtl_init_bypass_packet
	.align	2
	.globl	rtl_check_bypass_packet
	.set	nomips16
	.set	nomicromips
	.ent	rtl_check_bypass_packet
	.type	rtl_check_bypass_packet, @function
rtl_check_bypass_packet:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	lw	$2,68($4)
	bne	$2,$0,$L46
	li	$3,1			# 0x1

	lui	$3,%hi(fastpath_packet_bypassed_num)
	lw	$2,64($4)
	lw	$3,%lo(fastpath_packet_bypassed_num)($3)
	addiu	$2,$2,1
	sw	$2,64($4)
	sltu	$2,$3,$2
	beq	$2,$0,$L45
	nop

	li	$3,1			# 0x1
$L46:
	move	$2,$0
	jr	$31
	sw	$3,68($4)

$L45:
	jr	$31
	li	$2,1			# 0x1

	.set	macro
	.set	reorder
	.end	rtl_check_bypass_packet
	.size	rtl_check_bypass_packet, .-rtl_check_bypass_packet
	.align	2
	.globl	udp_fragCache_init
	.set	nomips16
	.set	nomicromips
	.ent	udp_fragCache_init
	.type	udp_fragCache_init, @function
udp_fragCache_init:
	.frame	$sp,48,$31		# vars= 0, regs= 8/0, args= 16, gp= 0
	.mask	0x807f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-48
	sw	$31,44($sp)
	sw	$22,40($sp)
	sw	$21,36($sp)
	sw	$20,32($sp)
	sw	$19,28($sp)
	sw	$18,24($sp)
	sw	$17,20($sp)
	sw	$16,16($sp)
	lui	$19,%hi(kmalloc_caches)
	li	$21,34078720			# 0x2080000
	addiu	$19,$19,%lo(kmalloc_caches)
	move	$16,$4
	addiu	$5,$21,32
	lw	$4,28($19)
	jal	kmem_cache_alloc
	lui	$20,%hi(udp_cache_table)

	beq	$2,$0,$L53
	sw	$2,%lo(udp_cache_table)($20)

	move	$18,$2
	sll	$4,$16,1
	lui	$2,%hi(udp_cache_list_free)
	addu	$4,$4,$16
	addiu	$17,$2,%lo(udp_cache_list_free)
	sw	$0,%lo(udp_cache_list_free)($2)
	lui	$2,%hi(max_udp_frag_entry)
	sll	$4,$4,2
	ori	$5,$21,0x20
	sw	$0,8($17)
	sw	$17,4($17)
	jal	__kmalloc
	sw	$16,%lo(max_udp_frag_entry)($2)

	sw	$2,0($18)
	lw	$4,%lo(udp_cache_table)($20)
	lw	$3,0($4)
	beq	$3,$0,$L47
	li	$2,-1			# 0xffffffffffffffff

	blez	$16,$L50
	move	$2,$0

	b	$L51
	move	$18,$0

$L61:
	lw	$3,0($4)
$L51:
	addu	$3,$3,$2
	addiu	$18,$18,1
	sw	$0,8($3)
	lw	$3,0($4)
	addu	$3,$3,$2
	sw	$0,0($3)
	lw	$4,%lo(udp_cache_table)($20)
	lw	$3,0($4)
	addu	$3,$3,$2
	addiu	$2,$2,12
	bne	$16,$18,$L61
	sw	$3,4($3)

	li	$21,34078720			# 0x2080000
	lui	$22,%hi(cache_timeout)
	move	$20,$0
	addiu	$21,$21,32
	b	$L54
	addiu	$22,$22,%lo(cache_timeout)

$L62:
	addiu	$4,$2,16
	move	$6,$0
	jal	init_timer_key
	move	$5,$0

	addiu	$3,$16,52
	lw	$2,4($17)
	sw	$16,32($16)
	sw	$22,28($16)
	sw	$2,56($16)
	sw	$0,52($16)
	sw	$16,0($2)
	lw	$2,8($17)
	sw	$3,4($17)
	addiu	$2,$2,1
	beq	$18,$20,$L50
	sw	$2,8($17)

$L54:
	lw	$4,28($19)
	move	$5,$21
	jal	kmem_cache_alloc
	addiu	$20,$20,1

	move	$7,$0
	bne	$2,$0,$L62
	move	$16,$2

$L53:
	move	$2,$0
$L47:
	lw	$31,44($sp)
	lw	$22,40($sp)
	lw	$21,36($sp)
	lw	$20,32($sp)
	lw	$19,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	jr	$31
	addiu	$sp,$sp,48

$L50:
	lw	$31,44($sp)
	li	$2,1			# 0x1
	lw	$22,40($sp)
	lw	$21,36($sp)
	lw	$20,32($sp)
	lw	$19,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	jr	$31
	addiu	$sp,$sp,48

	.set	macro
	.set	reorder
	.end	udp_fragCache_init
	.size	udp_fragCache_init, .-udp_fragCache_init
	.align	2
	.globl	negative_fragCache_init
	.set	nomips16
	.set	nomicromips
	.ent	negative_fragCache_init
	.type	negative_fragCache_init, @function
negative_fragCache_init:
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
	lui	$16,%hi(kmalloc_caches)
	li	$5,34078720			# 0x2080000
	addiu	$16,$16,%lo(kmalloc_caches)
	addiu	$5,$5,32
	jal	kmem_cache_alloc
	lw	$4,40($16)

	lui	$7,%hi(negative_cache_table)
	beq	$2,$0,$L67
	sw	$2,%lo(negative_cache_table)($7)

	lui	$3,%hi(negative_cache_list_free)
	move	$5,$0
	addiu	$18,$3,%lo(negative_cache_list_free)
	move	$4,$0
	sw	$0,%lo(negative_cache_list_free)($3)
	li	$8,64			# 0x40
	sw	$0,8($18)
	sw	$18,4($18)
$L66:
	sll	$3,$4,1
	addu	$3,$3,$4
	addiu	$4,$4,1
	sll	$3,$3,2
	addu	$6,$2,$3
	sw	$0,0($6)
	sw	$0,8($6)
	lw	$2,%lo(negative_cache_table)($7)
	addu	$3,$2,$3
	addu	$6,$2,$5
	addiu	$5,$5,12
	bne	$4,$8,$L66
	sw	$6,4($3)

	li	$20,34078720			# 0x2080000
	lui	$21,%hi(negative_cache_timeout)
	li	$19,64			# 0x40
	addiu	$20,$20,32
	b	$L68
	addiu	$21,$21,%lo(negative_cache_timeout)

$L76:
	addiu	$4,$2,12
	move	$6,$0
	jal	init_timer_key
	move	$5,$0

	addiu	$3,$17,48
	lw	$2,4($18)
	sw	$17,28($17)
	sw	$21,24($17)
	sw	$2,52($17)
	sw	$0,48($17)
	sw	$17,0($2)
	lw	$2,8($18)
	sw	$3,4($18)
	addiu	$2,$2,1
	beq	$19,$0,$L75
	sw	$2,8($18)

$L68:
	lw	$4,28($16)
	move	$5,$20
	jal	kmem_cache_alloc
	addiu	$19,$19,-1

	move	$7,$0
	bne	$2,$0,$L76
	move	$17,$2

$L67:
	lw	$31,44($sp)
	move	$2,$0
	lw	$21,40($sp)
	lw	$20,36($sp)
	lw	$19,32($sp)
	lw	$18,28($sp)
	lw	$17,24($sp)
	lw	$16,20($sp)
	jr	$31
	addiu	$sp,$sp,48

$L75:
	lw	$31,44($sp)
	li	$2,1			# 0x1
	lw	$21,40($sp)
	lw	$20,36($sp)
	lw	$19,32($sp)
	lw	$18,28($sp)
	lw	$17,24($sp)
	lw	$16,20($sp)
	jr	$31
	addiu	$sp,$sp,48

	.set	macro
	.set	reorder
	.end	negative_fragCache_init
	.size	negative_fragCache_init, .-negative_fragCache_init
	.align	2
	.globl	rtk_addArp
	.set	nomips16
	.set	nomicromips
	.ent	rtk_addArp
	.type	rtk_addArp, @function
rtk_addArp:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	jr	$31
	move	$2,$0

	.set	macro
	.set	reorder
	.end	rtk_addArp
	.size	rtk_addArp, .-rtk_addArp
	.align	2
	.globl	rtk_modifyArp
	.set	nomips16
	.set	nomicromips
	.ent	rtk_modifyArp
	.type	rtk_modifyArp, @function
rtk_modifyArp:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	jr	$31
	move	$2,$0

	.set	macro
	.set	reorder
	.end	rtk_modifyArp
	.size	rtk_modifyArp, .-rtk_modifyArp
	.align	2
	.globl	rtk_delArp
	.set	nomips16
	.set	nomicromips
	.ent	rtk_delArp
	.type	rtk_delArp, @function
rtk_delArp:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	jr	$31
	li	$2,-4			# 0xfffffffffffffffc

	.set	macro
	.set	reorder
	.end	rtk_delArp
	.size	rtk_delArp, .-rtk_delArp
	.align	2
	.globl	rtk_addRoute
	.set	nomips16
	.set	nomicromips
	.ent	rtk_addRoute
	.type	rtk_addRoute, @function
rtk_addRoute:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	jr	$31
	move	$2,$0

	.set	macro
	.set	reorder
	.end	rtk_addRoute
	.size	rtk_addRoute, .-rtk_addRoute
	.align	2
	.globl	rtk_modifyRoute
	.set	nomips16
	.set	nomicromips
	.ent	rtk_modifyRoute
	.type	rtk_modifyRoute, @function
rtk_modifyRoute:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	jr	$31
	move	$2,$0

	.set	macro
	.set	reorder
	.end	rtk_modifyRoute
	.size	rtk_modifyRoute, .-rtk_modifyRoute
	.align	2
	.globl	rtk_delRoute
	.set	nomips16
	.set	nomicromips
	.ent	rtk_delRoute
	.type	rtk_delRoute, @function
rtk_delRoute:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	jr	$31
	li	$2,-4			# 0xfffffffffffffffc

	.set	macro
	.set	reorder
	.end	rtk_delRoute
	.size	rtk_delRoute, .-rtk_delRoute
	.align	2
	.globl	rtk_addSession
	.set	nomips16
	.set	nomicromips
	.ent	rtk_addSession
	.type	rtk_addSession, @function
rtk_addSession:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	jr	$31
	move	$2,$0

	.set	macro
	.set	reorder
	.end	rtk_addSession
	.size	rtk_addSession, .-rtk_addSession
	.align	2
	.globl	rtk_delSession
	.set	nomips16
	.set	nomicromips
	.ent	rtk_delSession
	.type	rtk_delSession, @function
rtk_delSession:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	jr	$31
	move	$2,$0

	.set	macro
	.set	reorder
	.end	rtk_delSession
	.size	rtk_delSession, .-rtk_delSession
	.align	2
	.globl	rtk_addNaptConnection
	.set	nomips16
	.set	nomicromips
	.ent	rtk_addNaptConnection
	.type	rtk_addNaptConnection, @function
rtk_addNaptConnection:
	.frame	$sp,160,$31		# vars= 104, regs= 10/0, args= 16, gp= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	beq	$5,$0,$L200
	addiu	$sp,$sp,-160
	sw	$31,156($sp)
	sw	$fp,152($sp)
	sw	$23,148($sp)
	sw	$22,144($sp)
	sw	$21,140($sp)
	sw	$20,136($sp)
	sw	$19,132($sp)
	sw	$18,128($sp)
	sw	$17,124($sp)
	sw	$16,120($sp)
	move	$23,$4
	move	$4,$5
	move	$17,$7
	move	$20,$6
	.set	noreorder
	.set	nomacro
	jal	rtl_eth_hdr
	move	$16,$5
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L199
	li	$3,17			# 0x11
	.set	macro
	.set	reorder

	lw	$19,0($23)
	li	$2,6			# 0x6
	lw	$4,20($23)
	xori	$19,$19,0x2
	movn	$2,$3,$19
	.set	noreorder
	.set	nomacro
	jal	filter_addconnect
	sw	$2,56($sp)
	.set	macro
	.set	reorder

 #APP
 # 40 "./arch/mips/include/asm/irqflags.h" 1
		.set	push						
	.set	reorder						
	.set	noat						
	di	$2					
	andi	$2, 1					
	sll $0, $0, 3			
	.set	pop						

 # 0 "" 2
 #NO_APP
	lw	$6,4($23)
	lui	$3,%hi(napt_table_list_max)
	lw	$9,8($23)
	lui	$7,%hi(table_napt)
	lw	$10,20($23)
	lw	$11,24($23)
	srl	$13,$6,16
	lw	$4,0($23)
	lw	$3,%lo(napt_table_list_max)($3)
	srl	$5,$10,16
	sw	$2,72($sp)
	xor	$2,$6,$9
	xor	$2,$2,$10
	lw	$12,%lo(table_napt)($7)
	xor	$2,$2,$11
	xor	$2,$2,$13
	xor	$2,$2,$5
	andi	$5,$4,0xffff
	lw	$12,0($12)
	xor	$5,$5,$2
	srl	$13,$5,12
	addiu	$2,$3,-1
	xor	$5,$13,$5
	and	$2,$2,$5
	sll	$5,$2,1
	addu	$5,$5,$2
	sll	$5,$5,2
	addu	$2,$12,$5
	lw	$2,0($2)
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L92
	lui	$3,%hi(napt_list_free)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L230
	lw	$2,%lo(napt_list_free)($3)
	.set	macro
	.set	reorder

$L91:
	lw	$2,36($2)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L90
	lui	$3,%hi(napt_list_free)
	.set	macro
	.set	reorder

$L92:
	lw	$3,4($2)
	bne	$4,$3,$L91
	lw	$3,8($2)
	bne	$6,$3,$L91
	lw	$3,12($2)
	bne	$9,$3,$L91
	lw	$8,16($2)
	lw	$3,12($23)
	bne	$8,$3,$L91
	lw	$8,20($2)
	lw	$3,16($23)
	bne	$8,$3,$L91
	lw	$3,24($2)
	bne	$10,$3,$L91
	lw	$3,28($2)
	bne	$11,$3,$L91
	lw	$5,56($sp)
	move	$6,$20
	.set	noreorder
	.set	nomacro
	jal	rtl_conntrack_drop_check_hook
	move	$4,$20
	.set	macro
	.set	reorder

	lw	$2,72($sp)
 #APP
 # 59 "./arch/mips/include/asm/irqflags.h" 1
		.set	push						
	.set	noreorder					
	.set	noat						
	beqz	$2, 1f					
	di							
	ei							
1:								
	sll $0, $0, 3			
	.set	pop						

 # 0 "" 2
 #NO_APP
	.set	noreorder
	.set	nomacro
	b	$L85
	move	$2,$0
	.set	macro
	.set	reorder

$L90:
	lw	$2,%lo(napt_list_free)($3)
$L230:
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L93
	li	$9,-1			# 0xffffffffffffffff
	.set	macro
	.set	reorder

	sw	$4,4($2)
	lw	$4,4($23)
	lw	$6,44($2)
	sw	$4,8($2)
	lw	$4,8($23)
	sw	$4,12($2)
	lw	$4,12($23)
	sw	$4,16($2)
	lw	$4,16($23)
	sw	$4,20($2)
	lw	$4,20($23)
	sw	$4,24($2)
	lw	$4,24($23)
	sb	$9,0($2)
	sw	$17,32($2)
	sw	$4,28($2)
	addiu	$4,$3,%lo(napt_list_free)
	.set	noreorder
	.set	nomacro
	beq	$6,$0,$L94
	lw	$9,48($2)
	.set	macro
	.set	reorder

	sw	$9,48($6)
	lw	$9,48($2)
	lw	$6,44($2)
$L95:
	lui	$3,%hi(napt_list_inuse)
	sw	$6,0($9)
	lw	$6,8($4)
	addiu	$11,$2,44
	addiu	$3,$3,%lo(napt_list_inuse)
	sw	$0,44($2)
	addiu	$10,$2,36
	addiu	$6,$6,-1
	lw	$9,4($3)
	lui	$fp,%hi(path_table_list_max)
	sw	$6,8($4)
	sw	$9,48($2)
	sw	$2,0($9)
	lw	$4,8($3)
	lw	$6,%lo(table_napt)($7)
	sw	$11,4($3)
	addiu	$4,$4,1
	sw	$0,36($2)
	sw	$4,8($3)
	lw	$3,0($6)
	addu	$3,$3,$5
	lw	$3,4($3)
	sw	$3,40($2)
	lw	$3,0($6)
	addu	$3,$3,$5
	lw	$3,4($3)
	sw	$2,0($3)
	lw	$3,%lo(table_napt)($7)
	lw	$2,0($3)
	addu	$2,$2,$5
	sw	$10,4($2)
	lw	$2,0($3)
	li	$3,33			# 0x21
	sb	$0,48($sp)
	sb	$3,49($sp)
	addu	$5,$2,$5
	lw	$2,8($5)
	addiu	$2,$2,1
	.set	noreorder
	.set	nomacro
	jal	rtl865x_getLanDev
	sw	$2,8($5)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl865x_getWanDev
	sw	$2,60($sp)
	.set	macro
	.set	reorder

	move	$4,$16
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_protocol
	move	$22,$2
	.set	macro
	.set	reorder

	wsbh	$2,$2
	move	$4,$16
	andi	$2,$2,0xffff
	.set	noreorder
	.set	nomacro
	jal	rtl_ip_hdr
	sw	$2,108($sp)
	.set	macro
	.set	reorder

	move	$4,$16
	lw	$18,0($2)
	move	$17,$2
	andi	$18,$18,0xf
	sll	$18,$18,2
	.set	noreorder
	.set	nomacro
	jal	rtl_eth_hdr
	addu	$18,$2,$18
	.set	macro
	.set	reorder

	addiu	$5,$2,6
	li	$6,6			# 0x6
	.set	noreorder
	.set	nomacro
	jal	memcpy
	addiu	$4,$sp,40
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_eth_hdr
	move	$4,$16
	.set	macro
	.set	reorder

	li	$6,6			# 0x6
	addiu	$4,$sp,32
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$5,$2
	.set	macro
	.set	reorder

	move	$4,$16
	lbu	$2,1($17)
	move	$19,$2
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_orig_dscp
	sw	$2,76($sp)
	.set	macro
	.set	reorder

	move	$4,$16
	sll	$3,$2,2
	andi	$2,$19,0x3
	or	$2,$2,$3
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_remark_vlanpri
	sb	$2,1($17)
	.set	macro
	.set	reorder

	move	$4,$16
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_ori_vlanpri
	sw	$2,80($sp)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_phy_port
	move	$4,$16
	.set	macro
	.set	reorder

	move	$4,$16
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_mark
	sw	$2,64($sp)
	.set	macro
	.set	reorder

	move	$4,$16
	sw	$2,84($sp)
	lw	$2,12($17)
	sw	$2,88($sp)
	lhu	$2,0($18)
	sw	$2,92($sp)
	lw	$2,16($17)
	sw	$2,96($sp)
	lhu	$2,2($18)
	.set	noreorder
	.set	nomacro
	jal	rtl_store_skb_dst
	sw	$2,100($sp)
	.set	macro
	.set	reorder

	lui	$2,%hi(path_list_free)
	lw	$3,4($23)
	lw	$7,20($23)
	lw	$5,24($23)
	lw	$9,8($23)
	srl	$10,$3,8
	xor	$4,$3,$7
	lw	$19,%lo(path_list_free)($2)
	lw	$6,%lo(path_table_list_max)($fp)
	xor	$4,$4,$5
	sw	$2,68($sp)
	srl	$2,$7,16
	xor	$4,$4,$10
	srl	$9,$9,4
	xor	$4,$4,$2
	lw	$2,56($sp)
	xor	$4,$4,$9
	addiu	$6,$6,-1
	xor	$4,$2,$4
	srl	$9,$4,12
	xor	$4,$9,$4
	and	$2,$4,$6
	.set	noreorder
	.set	nomacro
	beq	$19,$0,$L96
	sw	$2,104($sp)
	.set	macro
	.set	reorder

	lw	$2,0($23)
	move	$14,$0
	sw	$2,4($19)
	lui	$2,%hi(route_list_inuse)
	lw	$13,4($23)
	lw	$2,%lo(route_list_inuse)($2)
	sw	$13,12($19)
	lhu	$11,8($23)
	sh	$11,10($19)
	lw	$4,20($23)
	sw	$4,16($19)
	lw	$4,24($23)
	sh	$4,8($19)
	lw	$12,12($23)
	sw	$12,20($19)
	lhu	$10,16($23)
	sh	$10,24($19)
	lw	$9,20($23)
	sw	$9,28($19)
	lw	$4,24($23)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L97
	sh	$4,32($19)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L99
	move	$7,$0
	.set	macro
	.set	reorder

$L98:
	lw	$2,44($2)
	beq	$2,$0,$L97
$L99:
	lw	$3,8($2)
	sltu	$4,$3,$7
	.set	noreorder
	.set	nomacro
	bne	$4,$0,$L98
	and	$5,$9,$3
	.set	macro
	.set	reorder

	lw	$4,4($2)
	bne	$5,$4,$L98
	addiu	$14,$2,16
	lw	$2,44($2)
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L99
	move	$7,$3
	.set	macro
	.set	reorder

$L97:
	li	$2,1			# 0x1
	sb	$0,1($19)
	sw	$14,40($19)
	sb	$2,2($19)
	li	$2,-1			# 0xffffffffffffffff
	sw	$20,52($19)
	sb	$2,0($19)
	lui	$2,%hi(rtl_init_bypass_packet_hook)
	lw	$2,%lo(rtl_init_bypass_packet_hook)($2)
	beq	$2,$0,$L100
	.set	noreorder
	.set	nomacro
	jalr	$2
	move	$4,$19
	.set	macro
	.set	reorder

	lw	$13,12($19)
	lw	$12,20($19)
	lhu	$11,10($19)
	lhu	$10,24($19)
$L100:
	beq	$13,$12,$L101
	lbu	$2,1($19)
	ori	$2,$2,0x1
	sb	$2,1($19)
$L101:
	.set	noreorder
	.set	nomacro
	beq	$11,$10,$L227
	li	$2,255			# 0xff
	.set	macro
	.set	reorder

	lbu	$2,1($19)
	ori	$2,$2,0x2
	sb	$2,1($19)
	li	$2,255			# 0xff
$L227:
	sw	$0,44($19)
	sw	$0,48($19)
	addiu	$21,$sp,16
	sw	$0,60($19)
	move	$5,$0
	sb	$0,72($19)
	li	$6,14			# 0xe
	sb	$0,73($19)
	move	$4,$21
	sh	$2,78($19)
	lw	$2,4($23)
	sw	$2,12($17)
	lw	$2,8($23)
	sh	$2,0($18)
	lw	$2,20($23)
	sw	$2,16($17)
	lw	$2,24($23)
	.set	noreorder
	.set	nomacro
	jal	memset
	sh	$2,2($18)
	.set	macro
	.set	reorder

	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_mark
	move	$4,$16
	.set	macro
	.set	reorder

	lw	$2,60($sp)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L104
	move	$6,$21
	.set	macro
	.set	reorder

	lw	$4,4($23)
	.set	noreorder
	.set	nomacro
	jal	rtl_arp_req_get_ha
	move	$5,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L201
	addiu	$5,$sp,48
	.set	macro
	.set	reorder

$L104:
	lw	$2,12($23)
	move	$5,$0
	li	$6,14			# 0xe
	move	$4,$21
	sw	$2,12($17)
	lw	$2,16($23)
	sh	$2,0($18)
	lw	$2,20($23)
	sw	$2,16($17)
	lw	$2,24($23)
	.set	noreorder
	.set	nomacro
	jal	memset
	sh	$2,2($18)
	.set	macro
	.set	reorder

	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_mark
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$22,$0,$L116
	move	$6,$21
	.set	macro
	.set	reorder

	lw	$4,20($23)
	.set	noreorder
	.set	nomacro
	jal	rtl_arp_req_get_ha
	move	$5,$22
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L202
	addiu	$5,$sp,48
	.set	macro
	.set	reorder

$L116:
	move	$5,$16
$L229:
	.set	noreorder
	.set	nomacro
	jal	rtl_set_entry_dscp_out
	move	$4,$19
	.set	macro
	.set	reorder

	move	$5,$16
	.set	noreorder
	.set	nomacro
	jal	rtl_set_entry_vlanpri
	move	$4,$19
	.set	macro
	.set	reorder

	move	$5,$16
	.set	noreorder
	.set	nomacro
	jal	rtl_set_entry_vlanid_out
	move	$4,$19
	.set	macro
	.set	reorder

	li	$2,6			# 0x6
	lw	$3,56($sp)
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L203
	li	$4,1			# 0x1
	.set	macro
	.set	reorder

	li	$6,1			# 0x1
	.set	noreorder
	.set	nomacro
	jal	udp_get_timeouts_by_state
	move	$5,$20
	.set	macro
	.set	reorder

	lw	$4,88($19)
	sw	$2,56($19)
	lui	$2,%hi(jiffies)
	sw	$20,52($19)
	lw	$5,92($19)
	lw	$2,%lo(jiffies)($2)
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L127
	sw	$2,36($19)
	.set	macro
	.set	reorder

$L207:
	sw	$5,92($4)
	lui	$2,%hi(path_list_free)
	lw	$5,92($19)
	lw	$4,88($19)
	addiu	$2,$2,%lo(path_list_free)
$L128:
	lw	$3,104($sp)
	lui	$9,%hi(table_path)
	sw	$4,0($5)
	lui	$4,%hi(path_list_inuse)
	lw	$7,8($2)
	addiu	$11,$19,88
	addiu	$4,$4,%lo(path_list_inuse)
	sw	$0,88($19)
	sll	$5,$3,1
	lw	$fp,%lo(path_table_list_max)($fp)
	addiu	$7,$7,-1
	lw	$10,4($4)
	sw	$7,8($2)
	addu	$2,$5,$3
	addiu	$7,$19,80
	sw	$10,92($19)
	sll	$5,$2,2
	sw	$19,0($10)
	addiu	$6,$fp,-1
	lw	$2,8($4)
	lw	$10,%lo(table_path)($9)
	sw	$0,80($19)
	addiu	$2,$2,1
	sw	$11,4($4)
	sw	$2,8($4)
	lw	$2,0($10)
	addu	$2,$2,$5
	lw	$2,4($2)
	sw	$2,84($19)
	lw	$2,0($10)
	addu	$2,$2,$5
	lw	$2,4($2)
	sw	$19,0($2)
	lw	$4,%lo(table_path)($9)
	lw	$2,68($sp)
	lw	$19,%lo(path_list_free)($2)
	lw	$2,0($4)
	addu	$2,$2,$5
	sw	$7,4($2)
	lw	$2,0($4)
	addu	$2,$2,$5
	lw	$4,8($2)
	addiu	$4,$4,1
	sw	$4,8($2)
	lw	$7,20($23)
	lw	$5,24($23)
$L96:
	lw	$4,12($23)
	srl	$9,$7,8
	lw	$2,16($23)
	srl	$5,$5,4
	lw	$3,56($sp)
	srl	$10,$4,16
	xor	$2,$4,$2
	xor	$2,$2,$10
	xor	$2,$2,$3
	xor	$2,$2,$7
	xor	$2,$2,$9
	xor	$2,$2,$5
	srl	$fp,$2,12
	xor	$fp,$fp,$2
	.set	noreorder
	.set	nomacro
	beq	$19,$0,$L129
	and	$fp,$fp,$6
	.set	macro
	.set	reorder

	lw	$2,0($23)
	move	$13,$0
	sw	$2,4($19)
	lui	$2,%hi(route_list_inuse)
	lw	$4,20($23)
	lw	$2,%lo(route_list_inuse)($2)
	sw	$4,12($19)
	lw	$4,24($23)
	sh	$4,10($19)
	lw	$12,12($23)
	sw	$12,16($19)
	lhu	$11,16($23)
	sh	$11,8($19)
	lw	$4,20($23)
	sw	$4,20($19)
	lw	$4,24($23)
	sh	$4,24($19)
	lw	$7,4($23)
	sw	$7,28($19)
	lhu	$10,8($23)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L130
	sh	$10,32($19)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L132
	move	$9,$0
	.set	macro
	.set	reorder

$L131:
	lw	$2,44($2)
	beq	$2,$0,$L130
$L132:
	lw	$3,8($2)
	sltu	$4,$3,$9
	.set	noreorder
	.set	nomacro
	bne	$4,$0,$L131
	and	$5,$7,$3
	.set	macro
	.set	reorder

	lw	$4,4($2)
	bne	$5,$4,$L131
	addiu	$13,$2,16
	lw	$2,44($2)
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L132
	move	$9,$3
	.set	macro
	.set	reorder

$L130:
	li	$2,2			# 0x2
	sb	$0,1($19)
	sw	$13,40($19)
	sb	$2,2($19)
	li	$2,-1			# 0xffffffffffffffff
	sw	$20,52($19)
	sb	$2,0($19)
	lui	$2,%hi(rtl_init_bypass_packet_hook)
	lw	$2,%lo(rtl_init_bypass_packet_hook)($2)
	beq	$2,$0,$L133
	.set	noreorder
	.set	nomacro
	jalr	$2
	move	$4,$19
	.set	macro
	.set	reorder

	lw	$12,16($19)
	lw	$7,28($19)
	lhu	$11,8($19)
	lhu	$10,32($19)
$L133:
	beq	$12,$7,$L134
	lbu	$2,1($19)
	ori	$2,$2,0x4
	sb	$2,1($19)
$L134:
	beq	$11,$10,$L135
	lbu	$2,1($19)
	ori	$2,$2,0x8
	sb	$2,1($19)
$L135:
	lbu	$5,1($17)
	move	$4,$16
	addiu	$21,$sp,16
	andi	$5,$5,0x3
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_orig_dscp
	sw	$5,104($sp)
	.set	macro
	.set	reorder

	li	$6,14			# 0xe
	lw	$5,104($sp)
	sll	$2,$2,2
	move	$4,$21
	or	$5,$5,$2
	li	$2,255			# 0xff
	sb	$5,1($17)
	sw	$0,44($19)
	move	$5,$0
	sh	$2,78($19)
	sw	$0,48($19)
	sw	$0,60($19)
	sb	$0,72($19)
	sb	$0,73($19)
	lw	$2,20($23)
	sw	$2,12($17)
	lw	$2,24($23)
	sh	$2,0($18)
	lw	$2,12($23)
	sw	$2,16($17)
	lw	$2,16($23)
	.set	noreorder
	.set	nomacro
	jal	memset
	sh	$2,2($18)
	.set	macro
	.set	reorder

	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_mark
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_ori_vlanpri
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$22,$0,$L137
	move	$6,$21
	.set	macro
	.set	reorder

	lw	$4,20($23)
	.set	noreorder
	.set	nomacro
	jal	rtl_arp_req_get_ha
	move	$5,$22
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L204
	addiu	$5,$sp,48
	.set	macro
	.set	reorder

$L137:
	lw	$2,20($23)
	move	$5,$0
	li	$6,14			# 0xe
	move	$4,$21
	sw	$2,12($17)
	lw	$2,24($23)
	sh	$2,0($18)
	lw	$2,4($23)
	sw	$2,16($17)
	lw	$2,8($23)
	.set	noreorder
	.set	nomacro
	jal	memset
	sh	$2,2($18)
	.set	macro
	.set	reorder

	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_mark
	move	$4,$16
	.set	macro
	.set	reorder

	lw	$2,60($sp)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L148
	move	$6,$21
	.set	macro
	.set	reorder

	lw	$4,4($23)
	.set	noreorder
	.set	nomacro
	jal	rtl_arp_req_get_ha
	move	$5,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L205
	addiu	$5,$sp,48
	.set	macro
	.set	reorder

$L148:
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_mark
	move	$4,$16
	.set	macro
	.set	reorder

	move	$5,$16
	sw	$2,48($19)
	.set	noreorder
	.set	nomacro
	jal	rtl_set_entry_dscp_in
	move	$4,$19
	.set	macro
	.set	reorder

	move	$5,$16
	.set	noreorder
	.set	nomacro
	jal	rtl_set_entry_vlanid_in
	move	$4,$19
	.set	macro
	.set	reorder

	li	$2,6			# 0x6
	lw	$3,56($sp)
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L206
	li	$4,1			# 0x1
	.set	macro
	.set	reorder

	li	$6,1			# 0x1
	.set	noreorder
	.set	nomacro
	jal	udp_get_timeouts_by_state
	move	$5,$20
	.set	macro
	.set	reorder

	lw	$4,88($19)
	sw	$2,56($19)
	lui	$2,%hi(jiffies)
	sw	$20,52($19)
	lw	$5,92($19)
	lw	$2,%lo(jiffies)($2)
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L156
	sw	$2,36($19)
	.set	macro
	.set	reorder

$L208:
	sw	$5,92($4)
	lui	$2,%hi(path_list_free)
	lw	$5,92($19)
	lw	$4,88($19)
	addiu	$2,$2,%lo(path_list_free)
$L157:
	sw	$4,0($5)
	lui	$4,%hi(path_list_inuse)
	lw	$6,8($2)
	lui	$7,%hi(table_path)
	addiu	$4,$4,%lo(path_list_inuse)
	sw	$0,88($19)
	sll	$5,$fp,1
	addiu	$6,$6,-1
	lw	$8,4($4)
	addu	$fp,$5,$fp
	sw	$6,8($2)
	addiu	$9,$19,88
	sll	$fp,$fp,2
	sw	$8,92($19)
	addiu	$6,$19,80
	sw	$19,0($8)
	lw	$2,8($4)
	lw	$5,%lo(table_path)($7)
	sw	$0,80($19)
	addiu	$2,$2,1
	sw	$9,4($4)
	sw	$2,8($4)
	lw	$2,0($5)
	addu	$2,$2,$fp
	lw	$2,4($2)
	sw	$2,84($19)
	lw	$2,0($5)
	addu	$2,$2,$fp
	lw	$2,4($2)
	sw	$19,0($2)
	lw	$3,%lo(table_path)($7)
	lw	$2,0($3)
	addu	$2,$2,$fp
	sw	$6,4($2)
	lw	$2,0($3)
	addu	$fp,$2,$fp
	lw	$2,8($fp)
	addiu	$2,$2,1
	sw	$2,8($fp)
$L129:
	.set	noreorder
	.set	nomacro
	jal	rtl_eth_hdr
	move	$4,$16
	.set	macro
	.set	reorder

	li	$6,6			# 0x6
	addiu	$5,$sp,40
	.set	noreorder
	.set	nomacro
	jal	memcpy
	addiu	$4,$2,6
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_eth_hdr
	move	$4,$16
	.set	macro
	.set	reorder

	li	$6,6			# 0x6
	addiu	$5,$sp,32
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$4,$2
	.set	macro
	.set	reorder

	move	$4,$16
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_mark
	lw	$5,84($sp)
	.set	macro
	.set	reorder

	move	$4,$16
	lw	$2,88($sp)
	sw	$2,12($17)
	lhu	$2,92($sp)
	sh	$2,0($18)
	lw	$2,96($sp)
	sw	$2,16($17)
	lhu	$2,100($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_dst
	sh	$2,2($18)
	.set	macro
	.set	reorder

	move	$4,$16
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_phy_port
	lw	$5,64($sp)
	.set	macro
	.set	reorder

	move	$4,$16
	lbu	$2,76($sp)
	lw	$5,80($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_restore_skb_vlanpri
	sb	$2,1($17)
	.set	macro
	.set	reorder

	lw	$2,60($sp)
	beq	$2,$0,$L158
	.set	noreorder
	.set	nomacro
	jal	rtl_dev_put
	move	$4,$2
	.set	macro
	.set	reorder

$L158:
	.set	noreorder
	.set	nomacro
	beq	$22,$0,$L228
	lw	$2,72($sp)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_dev_put
	move	$4,$22
	.set	macro
	.set	reorder

	lw	$2,72($sp)
$L228:
 #APP
 # 59 "./arch/mips/include/asm/irqflags.h" 1
		.set	push						
	.set	noreorder					
	.set	noat						
	beqz	$2, 1f					
	di							
	ei							
1:								
	sll $0, $0, 3			
	.set	pop						

 # 0 "" 2
 #NO_APP
	lw	$5,56($sp)
	move	$6,$20
	.set	noreorder
	.set	nomacro
	jal	rtl_conntrack_drop_check_hook
	move	$4,$20
	.set	macro
	.set	reorder

	move	$2,$0
$L85:
	lw	$31,156($sp)
	lw	$fp,152($sp)
	lw	$23,148($sp)
	lw	$22,144($sp)
	lw	$21,140($sp)
	lw	$20,136($sp)
	lw	$19,132($sp)
	lw	$18,128($sp)
	lw	$17,124($sp)
	lw	$16,120($sp)
	.set	noreorder
	.set	nomacro
	jr	$31
	addiu	$sp,$sp,160
	.set	macro
	.set	reorder

$L203:
	.set	noreorder
	.set	nomacro
	jal	rtl_tcp_get_timeouts
	move	$4,$20
	.set	macro
	.set	reorder

	lw	$4,88($19)
	sw	$2,56($19)
	lui	$2,%hi(jiffies)
	sw	$20,52($19)
	lw	$5,92($19)
	lw	$2,%lo(jiffies)($2)
	.set	noreorder
	.set	nomacro
	bne	$4,$0,$L207
	sw	$2,36($19)
	.set	macro
	.set	reorder

$L127:
	lw	$2,68($sp)
	addiu	$2,$2,%lo(path_list_free)
	.set	noreorder
	.set	nomacro
	b	$L128
	sw	$5,4($2)
	.set	macro
	.set	reorder

$L206:
	.set	noreorder
	.set	nomacro
	jal	rtl_tcp_get_timeouts
	move	$4,$20
	.set	macro
	.set	reorder

	lw	$4,88($19)
	sw	$2,56($19)
	lui	$2,%hi(jiffies)
	sw	$20,52($19)
	lw	$5,92($19)
	lw	$2,%lo(jiffies)($2)
	.set	noreorder
	.set	nomacro
	bne	$4,$0,$L208
	sw	$2,36($19)
	.set	macro
	.set	reorder

$L156:
	lw	$2,68($sp)
	addiu	$2,$2,%lo(path_list_free)
	.set	noreorder
	.set	nomacro
	b	$L157
	sw	$5,4($2)
	.set	macro
	.set	reorder

$L205:
	.set	noreorder
	.set	nomacro
	jal	rtl_ppp_proto_check
	move	$4,$16
	.set	macro
	.set	reorder

	move	$5,$2
	li	$2,1			# 0x1
	.set	noreorder
	.set	nomacro
	beq	$5,$2,$L209
	move	$4,$16
	.set	macro
	.set	reorder

$L151:
	.set	noreorder
	.set	nomacro
	jal	rtl_eth_hdr
	move	$4,$16
	.set	macro
	.set	reorder

	li	$6,6			# 0x6
	move	$5,$21
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$4,$2
	.set	macro
	.set	reorder

	move	$4,$16
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_smac
	lw	$5,60($sp)
	.set	macro
	.set	reorder

	li	$2,2048			# 0x800
	lw	$3,108($sp)
	bne	$3,$2,$L148
	lw	$5,44($19)
	bne	$5,$0,$L210
	li	$6,4			# 0x4
$L225:
	li	$5,2			# 0x2
	.set	noreorder
	.set	nomacro
	jal	rtl_hook_is_empty
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L148
	lw	$7,60($sp)
	.set	macro
	.set	reorder

	li	$5,4			# 0x4
	move	$6,$22
	.set	noreorder
	.set	nomacro
	jal	rtl_ipt_do_table
	move	$4,$16
	.set	macro
	.set	reorder

	b	$L148
$L201:
	.set	noreorder
	.set	nomacro
	jal	rtl_ppp_proto_check
	move	$4,$16
	.set	macro
	.set	reorder

	move	$5,$2
	li	$2,1			# 0x1
	beq	$5,$2,$L211
$L107:
	.set	noreorder
	.set	nomacro
	jal	rtl_eth_hdr
	move	$4,$16
	.set	macro
	.set	reorder

	li	$6,6			# 0x6
	addiu	$4,$2,6
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$5,$21
	.set	macro
	.set	reorder

	move	$4,$16
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_dmac
	lw	$5,60($sp)
	.set	macro
	.set	reorder

	move	$6,$0
	move	$5,$21
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_src_phy_port
	move	$4,$16
	.set	macro
	.set	reorder

	li	$2,2048			# 0x800
	lw	$3,108($sp)
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L104
	move	$6,$0
	.set	macro
	.set	reorder

	li	$5,2			# 0x2
	.set	noreorder
	.set	nomacro
	jal	rtl_hook_is_empty
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L212
	lw	$6,60($sp)
	.set	macro
	.set	reorder

$L109:
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_mark
	move	$4,$16
	.set	macro
	.set	reorder

	bne	$2,$0,$L213
	lw	$2,44($19)
	move	$4,$16
$L226:
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_mark
	sw	$2,44($19)
	.set	macro
	.set	reorder

	li	$6,4			# 0x4
	li	$5,2			# 0x2
	sw	$2,60($19)
	.set	noreorder
	.set	nomacro
	jal	rtl_hook_is_empty
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L214
	lw	$6,60($sp)
	.set	macro
	.set	reorder

$L112:
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_mark
	move	$4,$16
	.set	macro
	.set	reorder

	bne	$2,$0,$L215
	lw	$2,48($19)
	.set	noreorder
	.set	nomacro
	b	$L104
	sw	$2,48($19)
	.set	macro
	.set	reorder

$L202:
	.set	noreorder
	.set	nomacro
	jal	rtl_ppp_proto_check
	move	$4,$16
	.set	macro
	.set	reorder

	move	$5,$2
	li	$2,1			# 0x1
	.set	noreorder
	.set	nomacro
	beq	$5,$2,$L216
	move	$4,$16
	.set	macro
	.set	reorder

$L119:
	.set	noreorder
	.set	nomacro
	jal	rtl_eth_hdr
	move	$4,$16
	.set	macro
	.set	reorder

	li	$6,6			# 0x6
	move	$5,$21
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$4,$2
	.set	macro
	.set	reorder

	move	$5,$22
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_smac
	move	$4,$16
	.set	macro
	.set	reorder

	li	$2,2048			# 0x800
	lw	$3,108($sp)
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L229
	move	$5,$16
	.set	macro
	.set	reorder

	lw	$5,44($19)
	bne	$5,$0,$L217
	li	$6,4			# 0x4
$L224:
	li	$5,2			# 0x2
	.set	noreorder
	.set	nomacro
	jal	rtl_hook_is_empty
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L218
	lw	$6,60($sp)
	.set	macro
	.set	reorder

$L122:
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_mark
	move	$4,$16
	.set	macro
	.set	reorder

	bne	$2,$0,$L219
	lw	$2,48($19)
	.set	noreorder
	.set	nomacro
	b	$L116
	sw	$2,48($19)
	.set	macro
	.set	reorder

$L204:
	.set	noreorder
	.set	nomacro
	jal	rtl_ppp_proto_check
	move	$4,$16
	.set	macro
	.set	reorder

	move	$5,$2
	li	$2,1			# 0x1
	beq	$5,$2,$L220
$L140:
	.set	noreorder
	.set	nomacro
	jal	rtl_eth_hdr
	move	$4,$16
	.set	macro
	.set	reorder

	li	$6,6			# 0x6
	addiu	$4,$2,6
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$5,$21
	.set	macro
	.set	reorder

	move	$5,$22
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_dmac
	move	$4,$16
	.set	macro
	.set	reorder

	move	$4,$16
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_phy_port
	lw	$5,64($sp)
	.set	macro
	.set	reorder

	li	$6,1			# 0x1
	move	$5,$21
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_src_phy_port
	move	$4,$16
	.set	macro
	.set	reorder

	li	$2,2048			# 0x800
	lw	$3,108($sp)
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L221
	move	$6,$0
	.set	macro
	.set	reorder

$L143:
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_mark
	move	$4,$16
	.set	macro
	.set	reorder

	bne	$2,$0,$L222
	lw	$2,44($19)
	.set	noreorder
	.set	nomacro
	b	$L137
	sw	$2,44($19)
	.set	macro
	.set	reorder

$L94:
	.set	noreorder
	.set	nomacro
	b	$L95
	sw	$9,4($4)
	.set	macro
	.set	reorder

$L222:
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_mark
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L137
	sw	$2,44($19)
	.set	macro
	.set	reorder

$L93:
	lw	$2,72($sp)
 #APP
 # 59 "./arch/mips/include/asm/irqflags.h" 1
		.set	push						
	.set	noreorder					
	.set	noat						
	beqz	$2, 1f					
	di							
	ei							
1:								
	sll $0, $0, 3			
	.set	pop						

 # 0 "" 2
 #NO_APP
$L199:
	.set	noreorder
	.set	nomacro
	b	$L85
	li	$2,-1			# 0xffffffffffffffff
	.set	macro
	.set	reorder

$L209:
	.set	noreorder
	.set	nomacro
	jal	rtl_skb_mac_header_was_set
	sw	$5,104($sp)
	.set	macro
	.set	reorder

	lw	$5,104($sp)
	bne	$2,$5,$L151
	.set	noreorder
	.set	nomacro
	jal	rtl_eth_hdr
	move	$4,$16
	.set	macro
	.set	reorder

	li	$5,8			# 0x8
	move	$4,$2
	lbu	$2,13($2)
	lbu	$4,12($4)
	sll	$2,$2,8
	or	$2,$2,$4
	beq	$2,$5,$L151
	li	$5,-22			# 0xffffffffffffffea
	.set	noreorder
	.set	nomacro
	jal	rtl_skb_set_mac_header
	move	$4,$16
	.set	macro
	.set	reorder

	b	$L151
$L221:
	li	$5,2			# 0x2
	.set	noreorder
	.set	nomacro
	jal	rtl_hook_is_empty
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L143
	lw	$7,60($sp)
	.set	macro
	.set	reorder

	move	$5,$0
	move	$6,$22
	.set	noreorder
	.set	nomacro
	jal	rtl_ipt_do_table
	move	$4,$16
	.set	macro
	.set	reorder

	b	$L143
$L220:
	move	$4,$16
	.set	noreorder
	.set	nomacro
	jal	rtl_skb_mac_header_was_set
	sw	$5,104($sp)
	.set	macro
	.set	reorder

	lw	$5,104($sp)
	bne	$2,$5,$L140
	.set	noreorder
	.set	nomacro
	jal	rtl_eth_hdr
	move	$4,$16
	.set	macro
	.set	reorder

	li	$5,8			# 0x8
	move	$4,$2
	lbu	$2,13($2)
	lbu	$4,12($4)
	sll	$2,$2,8
	or	$2,$2,$4
	beq	$2,$5,$L140
	li	$5,-22			# 0xffffffffffffffea
	.set	noreorder
	.set	nomacro
	jal	rtl_skb_set_mac_header
	move	$4,$16
	.set	macro
	.set	reorder

	b	$L140
$L216:
	.set	noreorder
	.set	nomacro
	jal	rtl_skb_mac_header_was_set
	sw	$5,112($sp)
	.set	macro
	.set	reorder

	lw	$5,112($sp)
	bne	$2,$5,$L119
	.set	noreorder
	.set	nomacro
	jal	rtl_eth_hdr
	move	$4,$16
	.set	macro
	.set	reorder

	li	$5,8			# 0x8
	move	$4,$2
	lbu	$2,13($2)
	lbu	$4,12($4)
	sll	$2,$2,8
	or	$2,$2,$4
	.set	noreorder
	.set	nomacro
	beq	$2,$5,$L119
	li	$5,-22			# 0xffffffffffffffea
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_skb_set_mac_header
	move	$4,$16
	.set	macro
	.set	reorder

	b	$L119
$L211:
	move	$4,$16
	.set	noreorder
	.set	nomacro
	jal	rtl_skb_mac_header_was_set
	sw	$5,112($sp)
	.set	macro
	.set	reorder

	lw	$5,112($sp)
	bne	$2,$5,$L107
	.set	noreorder
	.set	nomacro
	jal	rtl_eth_hdr
	move	$4,$16
	.set	macro
	.set	reorder

	li	$5,8			# 0x8
	move	$4,$2
	lbu	$2,13($2)
	lbu	$4,12($4)
	sll	$2,$2,8
	or	$2,$2,$4
	.set	noreorder
	.set	nomacro
	beq	$2,$5,$L107
	li	$5,-22			# 0xffffffffffffffea
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_skb_set_mac_header
	move	$4,$16
	.set	macro
	.set	reorder

	b	$L107
$L219:
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_mark
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L116
	sw	$2,48($19)
	.set	macro
	.set	reorder

$L218:
	li	$5,4			# 0x4
	move	$7,$22
	.set	noreorder
	.set	nomacro
	jal	rtl_ipt_do_table
	move	$4,$16
	.set	macro
	.set	reorder

	b	$L122
$L217:
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_mark
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L224
	li	$6,4			# 0x4
	.set	macro
	.set	reorder

$L210:
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_mark
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L225
	li	$6,4			# 0x4
	.set	macro
	.set	reorder

$L214:
	li	$5,4			# 0x4
	move	$7,$22
	.set	noreorder
	.set	nomacro
	jal	rtl_ipt_do_table
	move	$4,$16
	.set	macro
	.set	reorder

	b	$L112
$L213:
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_mark
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L226
	move	$4,$16
	.set	macro
	.set	reorder

$L212:
	move	$5,$0
	move	$7,$22
	.set	noreorder
	.set	nomacro
	jal	rtl_ipt_do_table
	move	$4,$16
	.set	macro
	.set	reorder

	b	$L109
$L215:
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_mark
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L104
	sw	$2,48($19)
	.set	macro
	.set	reorder

$L200:
	.set	noreorder
	.set	nomacro
	jr	$31
	li	$2,-1			# 0xffffffffffffffff
	.set	macro
	.set	reorder

	.end	rtk_addNaptConnection
	.size	rtk_addNaptConnection, .-rtk_addNaptConnection
	.align	2
	.globl	rtk_delNaptConnection
	.set	nomips16
	.set	nomicromips
	.ent	rtk_delNaptConnection
	.type	rtk_delNaptConnection, @function
rtk_delNaptConnection:
	.frame	$sp,32,$31		# vars= 0, regs= 4/0, args= 16, gp= 0
	.mask	0x80070000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$18,24($sp)
	sw	$17,20($sp)
	sw	$16,16($sp)
	li	$3,17			# 0x11
	lw	$16,0($4)
	li	$2,6			# 0x6
	move	$17,$4
	xori	$16,$16,0x2
	movn	$2,$3,$16
	move	$16,$2
 #APP
 # 40 "./arch/mips/include/asm/irqflags.h" 1
		.set	push						
	.set	reorder						
	.set	noat						
	di	$18					
	andi	$18, 1					
	sll $0, $0, 3			
	.set	pop						

 # 0 "" 2
 #NO_APP
	.set	noreorder
	.set	nomacro
	jal	filter_delconnect
	lw	$4,20($4)
	.set	macro
	.set	reorder

	lui	$3,%hi(napt_table_list_max)
	lw	$5,4($17)
	lui	$10,%hi(table_napt)
	lw	$8,8($17)
	lw	$9,20($17)
	lw	$11,24($17)
	srl	$12,$5,16
	xor	$2,$5,$8
	lw	$4,0($17)
	xor	$2,$2,$9
	lw	$3,%lo(napt_table_list_max)($3)
	srl	$6,$9,16
	lw	$7,%lo(table_napt)($10)
	xor	$2,$2,$11
	xor	$2,$2,$12
	xor	$2,$2,$6
	andi	$6,$4,0xffff
	lw	$7,0($7)
	xor	$6,$6,$2
	srl	$12,$6,12
	addiu	$2,$3,-1
	xor	$6,$12,$6
	and	$2,$2,$6
	sll	$6,$2,1
	addu	$2,$6,$2
	sll	$6,$2,2
	addu	$2,$7,$6
	lw	$2,0($2)
	bne	$2,$0,$L255
	.set	noreorder
	.set	nomacro
	b	$L268
	move	$2,$18
	.set	macro
	.set	reorder

$L234:
	lw	$2,36($2)
	beq	$2,$0,$L233
$L255:
	lw	$3,4($2)
	bne	$4,$3,$L234
	lw	$3,8($2)
	bne	$5,$3,$L234
	lw	$3,12($2)
	bne	$8,$3,$L234
	lw	$7,16($2)
	lw	$3,12($17)
	bne	$7,$3,$L234
	lw	$7,20($2)
	lw	$3,16($17)
	bne	$7,$3,$L234
	lw	$3,24($2)
	bne	$9,$3,$L234
	lw	$3,28($2)
	bne	$11,$3,$L234
	lw	$3,36($2)
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L235
	sb	$0,0($2)
	.set	macro
	.set	reorder

	lw	$4,40($2)
	sw	$4,40($3)
$L236:
	lw	$3,40($2)
	lw	$4,36($2)
	sw	$4,0($3)
	lw	$3,%lo(table_napt)($10)
	lw	$3,0($3)
	addu	$6,$3,$6
	lw	$3,8($6)
	addiu	$3,$3,-1
	sw	$3,8($6)
	lui	$3,%hi(napt_list_inuse)
	lw	$4,44($2)
	lw	$5,48($2)
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L237
	addiu	$3,$3,%lo(napt_list_inuse)
	.set	macro
	.set	reorder

	sw	$5,48($4)
	lw	$5,48($2)
	lw	$4,44($2)
$L238:
	lui	$6,%hi(napt_list_free)
	sw	$4,0($5)
	lw	$4,8($3)
	addiu	$9,$2,44
	addiu	$6,$6,%lo(napt_list_free)
	sw	$0,44($2)
	lui	$5,%hi(table_path)
	addiu	$4,$4,-1
	lw	$7,4($6)
	sw	$4,8($3)
	lui	$3,%hi(path_table_list_max)
	sw	$7,48($2)
	sw	$2,0($7)
	lw	$2,8($6)
	lw	$8,%lo(path_table_list_max)($3)
	lw	$3,%lo(table_path)($5)
	addiu	$2,$2,1
	lw	$4,20($17)
	lw	$7,24($17)
	addiu	$8,$8,-1
	sw	$2,8($6)
	sw	$9,4($6)
	srl	$9,$4,16
	lw	$12,4($17)
	lw	$13,8($17)
	lw	$6,0($3)
	xor	$2,$12,$4
	srl	$3,$12,8
	xor	$2,$2,$7
	xor	$2,$2,$3
	srl	$3,$13,4
	xor	$2,$2,$9
	xor	$2,$2,$3
	xor	$2,$2,$16
	srl	$3,$2,12
	xor	$2,$3,$2
	and	$2,$2,$8
	sll	$11,$2,1
	addu	$2,$11,$2
	sll	$11,$2,2
	addu	$2,$6,$11
	lw	$2,0($2)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L267
	li	$3,1			# 0x1
	.set	macro
	.set	reorder

	lw	$14,0($17)
	lw	$9,12($17)
	.set	noreorder
	.set	nomacro
	b	$L246
	lw	$10,16($17)
	.set	macro
	.set	reorder

$L241:
	lw	$2,80($2)
	beq	$2,$0,$L240
$L246:
	lw	$15,4($2)
	bne	$15,$14,$L241
	lbu	$15,2($2)
	bne	$15,$3,$L241
	lw	$15,12($2)
	bne	$12,$15,$L241
	lhu	$15,10($2)
	bne	$13,$15,$L241
	lw	$15,20($2)
	bne	$15,$9,$L241
	lhu	$15,24($2)
	bne	$15,$10,$L241
	lw	$15,16($2)
	bne	$4,$15,$L241
	lhu	$15,8($2)
	bne	$7,$15,$L241
	lw	$3,80($2)
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L242
	sb	$0,0($2)
	.set	macro
	.set	reorder

	lw	$4,84($2)
	sw	$4,84($3)
$L243:
	lw	$3,84($2)
	lw	$4,80($2)
	sw	$4,0($3)
	lw	$3,%lo(table_path)($5)
	lw	$3,0($3)
	addu	$3,$3,$11
	lw	$4,8($3)
	addiu	$4,$4,-1
	sw	$4,8($3)
	lw	$4,88($2)
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L244
	lui	$3,%hi(path_list_inuse)
	.set	macro
	.set	reorder

	lw	$6,92($2)
	addiu	$3,$3,%lo(path_list_inuse)
	sw	$6,92($4)
	lw	$6,92($2)
	lw	$4,88($2)
$L245:
	sw	$4,0($6)
	lui	$6,%hi(path_list_free)
	lw	$4,8($3)
	addiu	$11,$2,88
	addiu	$6,$6,%lo(path_list_free)
	sw	$0,88($2)
	addiu	$4,$4,-1
	lw	$7,4($6)
	sw	$4,8($3)
	sw	$7,92($2)
	sw	$2,0($7)
	lw	$2,8($6)
	lw	$3,%lo(table_path)($5)
	lw	$4,20($17)
	addiu	$2,$2,1
	lw	$7,24($17)
	lw	$9,12($17)
	lw	$10,16($17)
	sw	$11,4($6)
	sw	$2,8($6)
	lw	$6,0($3)
$L240:
	xor	$2,$4,$9
	srl	$3,$4,8
	xor	$2,$2,$16
	xor	$2,$2,$10
	srl	$11,$9,16
	xor	$2,$2,$3
	srl	$3,$7,4
	xor	$2,$2,$11
	xor	$2,$2,$3
	srl	$3,$2,12
	xor	$2,$3,$2
	and	$2,$2,$8
	sll	$3,$2,1
	addu	$2,$3,$2
	sll	$3,$2,2
	addu	$6,$6,$3
	lw	$2,0($6)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L247
	li	$6,2			# 0x2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L253
	lw	$8,0($17)
	.set	macro
	.set	reorder

$L248:
	lw	$2,80($2)
	beq	$2,$0,$L247
$L253:
	lw	$11,4($2)
	bne	$11,$8,$L248
	lbu	$11,2($2)
	bne	$11,$6,$L248
	lw	$11,16($2)
	bne	$11,$9,$L248
	lhu	$11,8($2)
	bne	$11,$10,$L248
	lw	$11,20($2)
	bne	$11,$4,$L248
	lhu	$11,24($2)
	bne	$11,$7,$L248
	lw	$12,28($2)
	lw	$11,4($17)
	bne	$12,$11,$L248
	lhu	$12,32($2)
	lw	$11,8($17)
	bne	$12,$11,$L248
	lw	$4,80($2)
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L249
	sb	$0,0($2)
	.set	macro
	.set	reorder

	lw	$6,84($2)
	sw	$6,84($4)
$L250:
	lw	$4,84($2)
	lw	$6,80($2)
	sw	$6,0($4)
	lw	$4,%lo(table_path)($5)
	lw	$4,0($4)
	addu	$3,$4,$3
	lw	$4,8($3)
	addiu	$4,$4,-1
	sw	$4,8($3)
	lw	$4,88($2)
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L251
	lui	$3,%hi(path_list_inuse)
	.set	macro
	.set	reorder

	lw	$5,92($2)
	addiu	$3,$3,%lo(path_list_inuse)
	sw	$5,92($4)
	lw	$5,92($2)
	lw	$4,88($2)
$L252:
	sw	$4,0($5)
	lui	$4,%hi(path_list_free)
	lw	$5,8($3)
	addiu	$7,$2,88
	addiu	$4,$4,%lo(path_list_free)
	sw	$0,88($2)
	addiu	$5,$5,-1
	lw	$6,4($4)
	sw	$5,8($3)
	sw	$6,92($2)
	sw	$2,0($6)
	lw	$2,8($4)
	sw	$7,4($4)
	addiu	$2,$2,1
	sw	$2,8($4)
$L247:
	move	$2,$18
 #APP
 # 59 "./arch/mips/include/asm/irqflags.h" 1
		.set	push						
	.set	noreorder					
	.set	noat						
	beqz	$2, 1f					
	di							
	ei							
1:								
	sll $0, $0, 3			
	.set	pop						

 # 0 "" 2
 #NO_APP
	.set	noreorder
	.set	nomacro
	b	$L231
	move	$2,$0
	.set	macro
	.set	reorder

$L233:
	move	$2,$18
$L268:
 #APP
 # 59 "./arch/mips/include/asm/irqflags.h" 1
		.set	push						
	.set	noreorder					
	.set	noat						
	beqz	$2, 1f					
	di							
	ei							
1:								
	sll $0, $0, 3			
	.set	pop						

 # 0 "" 2
 #NO_APP
	li	$2,-4			# 0xfffffffffffffffc
$L231:
	lw	$31,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	.set	noreorder
	.set	nomacro
	jr	$31
	addiu	$sp,$sp,32
	.set	macro
	.set	reorder

$L251:
	lw	$5,92($2)
	addiu	$3,$3,%lo(path_list_inuse)
	.set	noreorder
	.set	nomacro
	b	$L252
	sw	$5,4($3)
	.set	macro
	.set	reorder

$L249:
	lw	$4,%lo(table_path)($5)
	lw	$6,84($2)
	lw	$4,0($4)
	addu	$4,$4,$3
	.set	noreorder
	.set	nomacro
	b	$L250
	sw	$6,4($4)
	.set	macro
	.set	reorder

$L244:
	lw	$6,92($2)
	addiu	$3,$3,%lo(path_list_inuse)
	.set	noreorder
	.set	nomacro
	b	$L245
	sw	$6,4($3)
	.set	macro
	.set	reorder

$L242:
	lw	$3,%lo(table_path)($5)
	lw	$4,84($2)
	lw	$3,0($3)
	addu	$3,$3,$11
	.set	noreorder
	.set	nomacro
	b	$L243
	sw	$4,4($3)
	.set	macro
	.set	reorder

$L267:
	lw	$9,12($17)
	.set	noreorder
	.set	nomacro
	b	$L240
	lw	$10,16($17)
	.set	macro
	.set	reorder

$L235:
	lw	$3,%lo(table_napt)($10)
	lw	$4,40($2)
	lw	$3,0($3)
	addu	$3,$3,$6
	.set	noreorder
	.set	nomacro
	b	$L236
	sw	$4,4($3)
	.set	macro
	.set	reorder

$L237:
	.set	noreorder
	.set	nomacro
	b	$L238
	sw	$5,4($3)
	.set	macro
	.set	reorder

	.end	rtk_delNaptConnection
	.size	rtk_delNaptConnection, .-rtk_delNaptConnection
	.align	2
	.globl	rtk_idleNaptConnection
	.set	nomips16
	.set	nomicromips
	.ent	rtk_idleNaptConnection
	.type	rtk_idleNaptConnection, @function
rtk_idleNaptConnection:
	.frame	$sp,8,$31		# vars= 0, regs= 1/0, args= 0, gp= 0
	.mask	0x00010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-8
	sw	$16,4($sp)
	lui	$9,%hi(path_table_list_max)
	lw	$10,4($4)
	li	$13,17			# 0x11
	lw	$8,20($4)
	lw	$3,0($4)
	lw	$11,24($4)
	srl	$7,$10,8
	lw	$24,%lo(path_table_list_max)($9)
	li	$9,6			# 0x6
	xori	$25,$3,0x2
	lw	$12,8($4)
	xor	$2,$10,$8
	movn	$9,$13,$25
	xor	$2,$2,$11
	xor	$2,$2,$7
	lui	$7,%hi(table_path)
	srl	$13,$8,16
	move	$25,$9
	lw	$7,%lo(table_path)($7)
	srl	$9,$12,4
	xor	$2,$2,$13
	lw	$13,16($4)
	xor	$2,$2,$9
	lw	$9,12($4)
	xor	$2,$2,$25
	lw	$15,0($7)
	srl	$7,$2,12
	addiu	$24,$24,-1
	xor	$2,$7,$2
	and	$7,$2,$24
	sll	$2,$7,1
	addu	$2,$2,$7
	lui	$7,%hi(jiffies)
	sll	$2,$2,2
	addu	$2,$15,$2
	lw	$14,%lo(jiffies)($7)
	lw	$2,0($2)
	beq	$2,$0,$L272
	li	$16,16711680			# 0xff0000

	li	$7,65536			# 0x10000
	addiu	$16,$16,255
	b	$L275
	addiu	$7,$7,255

$L273:
	lw	$2,80($2)
	beq	$2,$0,$L286
	srl	$4,$8,8

$L275:
	lw	$4,4($2)
	bne	$3,$4,$L273
	nop

	lw	$4,0($2)
	and	$4,$4,$16
	bne	$4,$7,$L273
	nop

	lw	$4,12($2)
	bne	$10,$4,$L273
	nop

	lhu	$4,10($2)
	bne	$12,$4,$L273
	nop

	lw	$4,20($2)
	bne	$4,$9,$L273
	nop

	lhu	$4,24($2)
	bne	$4,$13,$L273
	nop

	lw	$4,16($2)
	bne	$8,$4,$L273
	nop

	lhu	$4,8($2)
	bne	$11,$4,$L273
	nop

	lw	$4,36($2)
	subu	$2,$14,$5
	subu	$2,$2,$4
	bltz	$2,$L287
	li	$2,-1			# 0xffffffffffffffff

$L272:
	srl	$4,$8,8
$L286:
	srl	$16,$11,4
	move	$7,$4
	xor	$4,$7,$16
	xor	$4,$4,$8
	xor	$4,$4,$25
	srl	$2,$9,16
	xor	$4,$4,$9
	xor	$4,$4,$13
	xor	$2,$4,$2
	srl	$4,$2,12
	xor	$2,$4,$2
	and	$24,$2,$24
	sll	$2,$24,1
	addu	$24,$2,$24
	sll	$24,$24,2
	addu	$15,$15,$24
	lw	$2,0($15)
	beq	$2,$0,$L280
	nop

	li	$15,16711680			# 0xff0000
	li	$7,131072			# 0x20000
	addiu	$15,$15,255
	b	$L277
	addiu	$7,$7,255

$L276:
	lw	$2,80($2)
	beq	$2,$0,$L288
	lw	$16,4($sp)

$L277:
	lw	$4,4($2)
	bne	$3,$4,$L276
	nop

	lw	$4,0($2)
	and	$4,$4,$15
	bne	$4,$7,$L276
	nop

	lw	$4,16($2)
	bne	$4,$9,$L276
	nop

	lhu	$4,8($2)
	bne	$4,$13,$L276
	nop

	lw	$4,20($2)
	bne	$8,$4,$L276
	nop

	lhu	$4,24($2)
	bne	$11,$4,$L276
	nop

	lw	$4,28($2)
	bne	$10,$4,$L276
	nop

	lhu	$4,32($2)
	bne	$12,$4,$L276
	nop

	lw	$4,36($2)
	subu	$3,$14,$5
	subu	$3,$3,$4
	bgez	$3,$L269
	move	$2,$0

	li	$2,-1			# 0xffffffffffffffff
$L287:
	sw	$4,0($6)
$L269:
	lw	$16,4($sp)
$L288:
	jr	$31
	addiu	$sp,$sp,8

$L280:
	b	$L269
	move	$2,$0

	.set	macro
	.set	reorder
	.end	rtk_idleNaptConnection
	.size	rtk_idleNaptConnection, .-rtk_idleNaptConnection
	.align	2
	.globl	rtl_fpTimer_update
	.set	nomips16
	.set	nomicromips
	.ent	rtl_fpTimer_update
	.type	rtl_fpTimer_update, @function
rtl_fpTimer_update:
	.frame	$sp,72,$31		# vars= 32, regs= 6/0, args= 16, gp= 0
	.mask	0x801f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-72
	sw	$31,68($sp)
	sw	$20,64($sp)
	sw	$19,60($sp)
	sw	$18,56($sp)
	sw	$17,52($sp)
	sw	$16,48($sp)
	lui	$2,%hi(jiffies)
	move	$5,$0
	move	$16,$4
	lw	$18,%lo(jiffies)($2)
	jal	rtl_get_ct_protonum
	nop

	li	$3,17			# 0x11
	beq	$2,$3,$L307
	move	$5,$0

	jal	rtl_get_ct_protonum
	move	$4,$16

	li	$3,6			# 0x6
	bne	$2,$3,$L289
	li	$2,-1			# 0xffffffffffffffff

	jal	rtl_get_ct_tcp_state
	move	$4,$16

	sltu	$2,$2,6
	beq	$2,$0,$L289
	li	$2,-1			# 0xffffffffffffffff

	jal	rtl_tcp_get_timeouts
	move	$4,$16

	li	$20,2			# 0x2
	move	$17,$2
$L292:
	move	$6,$0
	move	$5,$0
	jal	rtl_get_ct_ip_by_dir
	move	$4,$16

	li	$6,1			# 0x1
	li	$5,1			# 0x1
	move	$4,$16
	jal	rtl_get_ct_ip_by_dir
	move	$19,$2

	beq	$19,$2,$L310
	move	$6,$0

	li	$6,1			# 0x1
$L311:
	move	$5,$0
	jal	rtl_get_ct_ip_by_dir
	move	$4,$16

	move	$6,$0
	li	$5,1			# 0x1
	move	$4,$16
	jal	rtl_get_ct_ip_by_dir
	move	$19,$2

	beq	$19,$2,$L309
	move	$6,$0

$L293:
	li	$2,-1			# 0xffffffffffffffff
$L289:
	lw	$31,68($sp)
	lw	$20,64($sp)
	lw	$19,60($sp)
	lw	$18,56($sp)
	lw	$17,52($sp)
	lw	$16,48($sp)
	jr	$31
	addiu	$sp,$sp,72

$L307:
	jal	rtl_get_ct_udp_status
	move	$4,$16

	andi	$2,$2,0x2
	beq	$2,$0,$L291
	li	$6,1			# 0x1

	li	$4,1			# 0x1
	jal	udp_get_timeouts_by_state
	move	$5,$16

	move	$6,$0
	move	$5,$0
	move	$4,$16
	jal	rtl_get_ct_ip_by_dir
	move	$17,$2

	li	$6,1			# 0x1
	li	$5,1			# 0x1
	move	$4,$16
	move	$19,$2
	jal	rtl_get_ct_ip_by_dir
	li	$20,1			# 0x1

	bne	$19,$2,$L311
	li	$6,1			# 0x1

	move	$6,$0
$L310:
	sw	$20,16($sp)
	li	$5,1			# 0x1
	jal	rtl_get_ct_ip_by_dir
	move	$4,$16

	move	$6,$0
	li	$5,1			# 0x1
	sw	$2,20($sp)
	jal	rtl_get_ct_port_by_dir
	move	$4,$16

	li	$6,1			# 0x1
	move	$5,$0
	sw	$2,24($sp)
	jal	rtl_get_ct_ip_by_dir
	move	$4,$16

	li	$6,1			# 0x1
	move	$5,$0
	sw	$2,28($sp)
	jal	rtl_get_ct_port_by_dir
	move	$4,$16

	move	$6,$0
	move	$5,$0
	sw	$2,32($sp)
	jal	rtl_get_ct_ip_by_dir
	move	$4,$16

	move	$6,$0
	move	$5,$0
$L306:
	move	$4,$16
	jal	rtl_get_ct_port_by_dir
	sw	$2,36($sp)

	addiu	$6,$sp,44
	addiu	$4,$sp,16
	sw	$2,40($sp)
	jal	rtk_idleNaptConnection
	move	$5,$17

	beq	$2,$0,$L293
	lw	$5,44($sp)

	move	$4,$16
	jal	rtl_set_ct_timeout_expires
	addu	$5,$17,$5

	addu	$5,$17,$18
	jal	rtl_check_for_acc
	move	$4,$16

	b	$L289
	move	$2,$0

$L291:
	move	$4,$0
	jal	udp_get_timeouts_by_state
	move	$5,$16

	li	$20,1			# 0x1
	b	$L292
	move	$17,$2

$L309:
	sw	$20,16($sp)
	move	$5,$0
	jal	rtl_get_ct_ip_by_dir
	move	$4,$16

	move	$6,$0
	move	$5,$0
	sw	$2,20($sp)
	jal	rtl_get_ct_port_by_dir
	move	$4,$16

	li	$6,1			# 0x1
	li	$5,1			# 0x1
	sw	$2,24($sp)
	jal	rtl_get_ct_ip_by_dir
	move	$4,$16

	li	$6,1			# 0x1
	li	$5,1			# 0x1
	sw	$2,28($sp)
	jal	rtl_get_ct_port_by_dir
	move	$4,$16

	move	$6,$0
	li	$5,1			# 0x1
	sw	$2,32($sp)
	jal	rtl_get_ct_ip_by_dir
	move	$4,$16

	move	$6,$0
	b	$L306
	li	$5,1			# 0x1

	.set	macro
	.set	reorder
	.end	rtl_fpTimer_update
	.size	rtl_fpTimer_update, .-rtl_fpTimer_update
	.align	2
	.globl	rtl_br_fdb_time_update
	.set	nomips16
	.set	nomicromips
	.ent	rtl_br_fdb_time_update
	.type	rtl_br_fdb_time_update, @function
rtl_br_fdb_time_update:
	.frame	$sp,32,$31		# vars= 0, regs= 4/0, args= 16, gp= 0
	.mask	0x80070000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$18,24($sp)
	sw	$17,20($sp)
	sw	$16,16($sp)
	move	$2,$6
	move	$18,$4
	li	$6,2			# 0x2
	move	$4,$0
	move	$17,$5
	jal	rtl_get_hw_fdb_age
	move	$5,$2

	li	$3,300			# 0x12c
	beq	$2,$3,$L314
	li	$3,450			# 0x1c2

	beq	$2,$3,$L315
	li	$3,150			# 0x96

	beq	$2,$3,$L318
	lui	$16,%hi(jiffies)

	move	$4,$17
$L319:
	lw	$16,%lo(jiffies)($16)
	jal	rtl_get_fdb_aging
	nop

	move	$4,$18
	jal	rtl_hold_time
	move	$17,$2

	addu	$2,$17,$2
	lw	$31,28($sp)
	lw	$18,24($sp)
	subu	$2,$16,$2
	lw	$17,20($sp)
	lw	$16,16($sp)
	nor	$2,$0,$2
	sra	$2,$2,31
	jr	$31
	addiu	$sp,$sp,32

$L318:
	lw	$5,%lo(jiffies)($16)
	move	$4,$17
	jal	rtl_set_fdb_aging
	addiu	$5,$5,-30000

	b	$L319
	move	$4,$17

$L315:
	lui	$16,%hi(jiffies)
	move	$4,$17
	lw	$5,%lo(jiffies)($16)
	jal	rtl_set_fdb_aging
	nop

	b	$L319
	move	$4,$17

$L314:
	lui	$16,%hi(jiffies)
	move	$4,$17
	lw	$5,%lo(jiffies)($16)
	jal	rtl_set_fdb_aging
	addiu	$5,$5,-15000

	b	$L319
	move	$4,$17

	.set	macro
	.set	reorder
	.end	rtl_br_fdb_time_update
	.size	rtl_br_fdb_time_update, .-rtl_br_fdb_time_update
	.align	2
	.globl	FastPath_Route
	.set	nomips16
	.set	nomicromips
	.ent	FastPath_Route
	.type	FastPath_Route, @function
FastPath_Route:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	lui	$2,%hi(route_list_inuse)
	lw	$3,%lo(route_list_inuse)($2)
	beq	$3,$0,$L324
	nop

	move	$8,$0
	b	$L323
	move	$2,$0

$L322:
	lw	$3,44($3)
	beq	$3,$0,$L327
	nop

$L323:
	lw	$5,8($3)
	sltu	$6,$5,$8
	bne	$6,$0,$L322
	and	$7,$5,$4

	lw	$6,4($3)
	bne	$7,$6,$L322
	nop

	addiu	$2,$3,16
	lw	$3,44($3)
	bne	$3,$0,$L323
	move	$8,$5

$L327:
	jr	$31
	nop

$L324:
	jr	$31
	move	$2,$0

	.set	macro
	.set	reorder
	.end	FastPath_Route
	.size	FastPath_Route, .-FastPath_Route
	.align	2
	.globl	mark_path_invalid
	.set	nomips16
	.set	nomicromips
	.ent	mark_path_invalid
	.type	mark_path_invalid, @function
mark_path_invalid:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	lhu	$8,16($sp)
	srl	$2,$6,16
	srl	$3,$4,8
	xor	$2,$2,$4
	xor	$11,$6,$8
	lui	$9,%hi(path_table_list_max)
	xor	$2,$2,$11
	xor	$2,$2,$3
	srl	$3,$5,4
	lw	$9,%lo(path_table_list_max)($9)
	xor	$2,$2,$7
	xor	$2,$2,$3
	srl	$3,$2,12
	lui	$10,%hi(table_path)
	addiu	$9,$9,-1
	xor	$2,$3,$2
	and	$2,$2,$9
	lw	$12,%lo(table_path)($10)
	sll	$3,$2,1
	addu	$2,$3,$2
	lw	$3,0($12)
	sll	$2,$2,2
	addu	$2,$3,$2
	lw	$2,0($2)
	bne	$2,$0,$L345
	li	$13,255			# 0xff

	b	$L358
	nop

$L330:
	lw	$2,80($2)
	beq	$2,$0,$L358
	nop

$L345:
	lhu	$3,10($2)
	bne	$3,$5,$L330
	nop

	lhu	$3,8($2)
	bne	$3,$7,$L330
	nop

	lw	$3,12($2)
	bne	$4,$3,$L330
	nop

	lw	$3,16($2)
	bne	$6,$3,$L330
	nop

	lbu	$3,0($2)
	bne	$3,$13,$L330
	li	$3,1			# 0x1

	lbu	$12,2($2)
	beq	$12,$3,$L354
	sb	$0,0($2)

	lw	$11,28($2)
	beq	$11,$0,$L358
	lhu	$3,32($2)

	srl	$12,$4,16
	xor	$2,$4,$8
	xor	$2,$2,$12
	xor	$2,$2,$5
	srl	$8,$11,8
	srl	$3,$3,4
	xor	$2,$2,$11
	xor	$2,$2,$8
	xor	$2,$2,$3
	srl	$3,$2,12
	xor	$2,$3,$2
	and	$9,$2,$9
$L338:
	lw	$3,%lo(table_path)($10)
	sll	$2,$9,1
	addu	$9,$2,$9
	lw	$2,0($3)
	sll	$9,$9,2
	addu	$9,$2,$9
	lw	$2,0($9)
	beq	$2,$0,$L358
	nop

	b	$L337
	li	$8,255			# 0xff

$L336:
	lw	$2,80($2)
	beq	$2,$0,$L358
	nop

$L337:
	lw	$3,20($2)
	bne	$6,$3,$L336
	nop

	lhu	$3,24($2)
	bne	$3,$7,$L336
	nop

	lw	$3,28($2)
	bne	$4,$3,$L336
	nop

	lhu	$3,32($2)
	bne	$3,$5,$L336
	nop

	lbu	$3,0($2)
	bne	$3,$8,$L336
	nop

	b	$L336
	sb	$0,0($2)

$L354:
	lw	$3,20($2)
	beq	$3,$0,$L358
	lhu	$12,24($2)

	srl	$2,$6,8
	srl	$8,$7,4
	xor	$2,$2,$11
	xor	$2,$2,$8
	xor	$2,$2,$3
	srl	$3,$3,16
	xor	$2,$2,$12
	xor	$2,$2,$3
	srl	$3,$2,12
	xor	$2,$3,$2
	b	$L338
	and	$9,$2,$9

$L358:
	jr	$31
	nop

	.set	macro
	.set	reorder
	.end	mark_path_invalid
	.size	mark_path_invalid, .-mark_path_invalid
	.section	.rodata.str1.4
	.align	2
$LC9:
	.ascii	"lo\000"
	.text
	.align	2
	.globl	enter_fast_path
	.set	nomips16
	.set	nomicromips
	.ent	enter_fast_path
	.type	enter_fast_path, @function
enter_fast_path:
	.frame	$sp,96,$31		# vars= 32, regs= 10/0, args= 24, gp= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-96
	sw	$31,92($sp)
	sw	$fp,88($sp)
	sw	$23,84($sp)
	sw	$22,80($sp)
	sw	$21,76($sp)
	sw	$20,72($sp)
	sw	$19,68($sp)
	sw	$18,64($sp)
	sw	$17,60($sp)
	sw	$16,56($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_dev_name
	move	$20,$4
	.set	macro
	.set	reorder

	move	$4,$20
	.set	noreorder
	.set	nomacro
	jal	rtl_ip_hdr
	move	$16,$2
	.set	macro
	.set	reorder

	move	$17,$2
	lbu	$2,8($2)
	sltu	$2,$2,2
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L547
	li	$2,6			# 0x6
	.set	macro
	.set	reorder

	lbu	$21,9($17)
	.set	noreorder
	.set	nomacro
	beq	$21,$2,$L362
	li	$2,17			# 0x11
	.set	macro
	.set	reorder

	beq	$21,$2,$L362
$L547:
	move	$19,$0
$L359:
	move	$2,$19
$L585:
	lw	$31,92($sp)
	lw	$fp,88($sp)
	lw	$23,84($sp)
	lw	$22,80($sp)
	lw	$21,76($sp)
	lw	$20,72($sp)
	lw	$19,68($sp)
	lw	$18,64($sp)
	lw	$17,60($sp)
	lw	$16,56($sp)
	.set	noreorder
	.set	nomacro
	jr	$31
	addiu	$sp,$sp,96
	.set	macro
	.set	reorder

$L362:
	lw	$18,0($17)
	move	$6,$20
	move	$4,$17
	andi	$18,$18,0xf
	sll	$18,$18,2
	addu	$18,$17,$18
	.set	noreorder
	.set	nomacro
	jal	fast_path_pre_process_check
	move	$5,$18
	.set	macro
	.set	reorder

	move	$19,$2
	li	$2,6			# 0x6
	.set	noreorder
	.set	nomacro
	bne	$19,$2,$L585
	move	$2,$19
	.set	macro
	.set	reorder

	lui	$2,%hi($LC9)
	addiu	$2,$2,%lo($LC9)
 #APP
 # 77 "./arch/mips/include/asm/string.h" 1
	.set	noreorder
	.set	noat
	lbu	$3,($16)
1:	lbu	$1,($2)
	addiu	$16,1
	bne	$1,$3,2f
	addiu	$2,1
	bnez	$3,1b
	lbu	$3,($16)
	move	$3,$1
2:	subu	$3,$1
3:	.set	at
	.set	reorder
 # 0 "" 2
 #NO_APP
	beq	$3,$0,$L547
	lhu	$2,6($17)
	lw	$8,12($17)
	lw	$10,16($17)
	wsbh	$2,$2
	lhu	$22,0($18)
	lhu	$9,2($18)
	andi	$2,$2,0xffff
	.set	noreorder
	.set	nomacro
	beq	$21,$19,$L555
	andi	$3,$2,0x3fff
	.set	macro
	.set	reorder

$L365:
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L368
	move	$23,$0
	.set	macro
	.set	reorder

	move	$fp,$0
	move	$12,$0
	li	$19,1			# 0x1
	xor	$11,$8,$10
$L369:
	srl	$2,$8,8
	srl	$3,$10,16
	lui	$4,%hi(path_table_list_max)
	xor	$2,$2,$21
	xor	$3,$2,$3
	lw	$4,%lo(path_table_list_max)($4)
	xor	$3,$3,$9
	srl	$2,$22,4
	xor	$3,$3,$11
	xor	$2,$3,$2
	srl	$5,$2,12
	addiu	$3,$4,-1
	lui	$4,%hi(table_path)
	xor	$2,$5,$2
	and	$2,$3,$2
	lw	$4,%lo(table_path)($4)
	sll	$3,$2,1
	addu	$2,$3,$2
	lw	$3,0($4)
	sll	$2,$2,2
	addu	$2,$3,$2
	lw	$16,0($2)
	.set	noreorder
	.set	nomacro
	beq	$16,$0,$L380
	li	$3,255			# 0xff
	.set	macro
	.set	reorder

	li	$7,2			# 0x2
	li	$6,1			# 0x1
	li	$5,17			# 0x11
	.set	noreorder
	.set	nomacro
	b	$L457
	li	$4,6			# 0x6
	.set	macro
	.set	reorder

$L381:
	lw	$16,80($16)
	beq	$16,$0,$L380
$L457:
	lhu	$2,10($16)
	bne	$22,$2,$L381
	lhu	$2,8($16)
	bne	$9,$2,$L381
	lw	$2,12($16)
	bne	$8,$2,$L381
	lw	$2,16($16)
	bne	$10,$2,$L381
	lbu	$2,0($16)
	bne	$2,$3,$L381
	lw	$2,4($16)
	beq	$2,$7,$L556
	bne	$2,$6,$L381
	.set	noreorder
	.set	nomacro
	bne	$21,$5,$L381
	lui	$2,%hi(rtl_check_bypass_packet_hook)
	.set	macro
	.set	reorder

	lw	$2,%lo(rtl_check_bypass_packet_hook)($2)
$L591:
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L587
	addiu	$5,$16,60
	.set	macro
	.set	reorder

	move	$4,$16
	sw	$10,48($sp)
	sw	$8,44($sp)
	sw	$11,40($sp)
	sw	$12,36($sp)
	.set	noreorder
	.set	nomacro
	jalr	$2
	sw	$9,32($sp)
	.set	macro
	.set	reorder

	lw	$9,32($sp)
	lw	$12,36($sp)
	lw	$11,40($sp)
	lw	$8,44($sp)
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L557
	lw	$10,48($sp)
	.set	macro
	.set	reorder

	addiu	$5,$16,60
$L587:
	sw	$10,48($sp)
	move	$4,$20
	sw	$8,44($sp)
	sw	$11,40($sp)
	sw	$12,36($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_recheck_fp_policy_route_mark
	sw	$9,32($sp)
	.set	macro
	.set	reorder

	addiu	$6,$sp,24
	lw	$5,60($16)
	.set	noreorder
	.set	nomacro
	jal	rtl_skb_policy_route_mark_set
	move	$4,$20
	.set	macro
	.set	reorder

	li	$4,2			# 0x2
	lw	$7,52($16)
	move	$5,$20
	.set	noreorder
	.set	nomacro
	jal	rtl_ip_route_input
	lw	$6,28($16)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L558
	lw	$11,40($sp)
	.set	macro
	.set	reorder

	lw	$5,24($sp)
	move	$6,$0
	.set	noreorder
	.set	nomacro
	jal	rtl_skb_policy_route_mark_set
	move	$4,$20
	.set	macro
	.set	reorder

	move	$7,$0
$L387:
	.set	noreorder
	.set	nomacro
	b	$L359
	move	$19,$7
	.set	macro
	.set	reorder

$L368:
	lui	$4,%hi(statistic_udp_frag_rx)
	li	$5,8192			# 0x2000
	lw	$3,%lo(statistic_udp_frag_rx)($4)
	addiu	$3,$3,1
	.set	noreorder
	.set	nomacro
	beq	$2,$5,$L463
	sw	$3,%lo(statistic_udp_frag_rx)($4)
	.set	macro
	.set	reorder

$L462:
	andi	$3,$2,0x2000
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L464
	move	$fp,$0
	.set	macro
	.set	reorder

	andi	$2,$2,0x1fff
	move	$19,$0
	sltu	$fp,$0,$2
$L370:
	lhu	$4,4($17)
	lui	$5,%hi(max_udp_frag_entry)
	xor	$11,$8,$10
	lbu	$7,9($17)
	srl	$3,$11,16
	xor	$2,$4,$11
	lw	$5,%lo(max_udp_frag_entry)($5)
	xor	$2,$2,$3
	srl	$3,$2,8
	addiu	$5,$5,-1
	xor	$2,$7,$2
	xor	$2,$2,$3
	and	$5,$5,$2
	lui	$12,%hi(negative_cache_table)
	sll	$6,$5,1
	addu	$3,$6,$5
	lw	$2,%lo(negative_cache_table)($12)
	sll	$3,$3,2
	addu	$2,$2,$3
	lw	$2,0($2)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L371
	li	$13,3			# 0x3
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L590
	lhu	$3,2($2)
	.set	macro
	.set	reorder

$L372:
	lw	$2,40($2)
	beq	$2,$0,$L371
	lhu	$3,2($2)
$L590:
	bne	$3,$4,$L372
	lw	$3,4($2)
	bne	$8,$3,$L372
	lw	$3,8($2)
	bne	$10,$3,$L372
	lbu	$3,1($2)
	bne	$3,$7,$L372
	lbu	$3,0($2)
	bne	$3,$13,$L372
	.set	noreorder
	.set	nomacro
	b	$L359
	move	$19,$0
	.set	macro
	.set	reorder

$L380:
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_inDev
	move	$4,$20
	.set	macro
	.set	reorder

	move	$6,$20
	move	$5,$18
	.set	noreorder
	.set	nomacro
	jal	fast_path_post_process_return_check
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L359
	move	$19,$2
	.set	macro
	.set	reorder

$L371:
	lui	$2,%hi(udp_cache_table)
	addu	$6,$6,$5
	lw	$2,%lo(udp_cache_table)($2)
	sll	$6,$6,2
	lw	$2,0($2)
	addu	$2,$2,$6
	lw	$23,0($2)
	.set	noreorder
	.set	nomacro
	bne	$23,$0,$L512
	li	$3,1			# 0x1
	.set	macro
	.set	reorder

	b	$L377
$L374:
	lw	$23,44($23)
	beq	$23,$0,$L377
$L512:
	lhu	$2,2($23)
	bne	$2,$4,$L374
	lw	$2,4($23)
	bne	$8,$2,$L374
	lw	$2,8($23)
	bne	$10,$2,$L374
	lbu	$2,1($23)
	bne	$2,$7,$L374
	lbu	$2,0($23)
	bne	$2,$3,$L374
	lhu	$22,12($23)
	move	$12,$19
	.set	noreorder
	.set	nomacro
	b	$L369
	lhu	$9,14($23)
	.set	macro
	.set	reorder

$L555:
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L366
	li	$4,8192			# 0x2000
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$4,$L366
	lui	$4,%hi(statistic_udp_frag_rx)
	.set	macro
	.set	reorder

	lw	$3,%lo(statistic_udp_frag_rx)($4)
	addiu	$3,$3,1
	.set	noreorder
	.set	nomacro
	b	$L462
	sw	$3,%lo(statistic_udp_frag_rx)($4)
	.set	macro
	.set	reorder

$L366:
	lhu	$4,12($18)
	andi	$4,$4,0x100
	.set	noreorder
	.set	nomacro
	bne	$4,$0,$L559
	move	$7,$9
	.set	macro
	.set	reorder

	lw	$4,12($18)
	andi	$4,$4,0x600
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L365
	move	$19,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L585
	move	$2,$19
	.set	macro
	.set	reorder

$L464:
	.set	noreorder
	.set	nomacro
	b	$L370
	move	$19,$0
	.set	macro
	.set	reorder

$L463:
	move	$fp,$0
	.set	noreorder
	.set	nomacro
	b	$L370
	li	$19,1			# 0x1
	.set	macro
	.set	reorder

$L377:
	.set	noreorder
	.set	nomacro
	bne	$19,$0,$L560
	lui	$3,%hi(negative_cache_list_free)
	.set	macro
	.set	reorder

	lui	$11,%hi(statistic_udp_frag_ps)
	lw	$9,%lo(statistic_udp_frag_ps)($11)
	lw	$2,%lo(negative_cache_list_free)($3)
	addiu	$9,$9,1
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L547
	sw	$9,%lo(statistic_udp_frag_ps)($11)
	.set	macro
	.set	reorder

	lw	$9,48($2)
	sh	$4,2($2)
	li	$4,3			# 0x3
	sw	$8,4($2)
	sw	$10,8($2)
	sb	$7,1($2)
	.set	noreorder
	.set	nomacro
	beq	$9,$0,$L378
	sb	$4,0($2)
	.set	macro
	.set	reorder

	lw	$4,52($2)
	addiu	$3,$3,%lo(negative_cache_list_free)
	sw	$4,52($9)
	lw	$9,48($2)
	lw	$4,52($2)
$L379:
	sw	$9,0($4)
	addiu	$9,$2,40
	lw	$8,8($3)
	addiu	$4,$2,12
	lw	$7,%lo(negative_cache_table)($12)
	sw	$0,40($2)
	addiu	$8,$8,-1
	addu	$6,$7,$6
	sw	$8,8($3)
	lw	$3,4($6)
	sw	$3,44($2)
	lw	$3,4($6)
	sw	$2,0($3)
	lui	$3,%hi(jiffies)
	lw	$7,%lo(negative_cache_table)($12)
	lw	$6,%lo(jiffies)($3)
	li	$3,12			# 0xc
	mul	$8,$5,$3
	addiu	$3,$6,1000
	addu	$5,$8,$7
	lw	$6,8($5)
	sw	$9,4($5)
	addiu	$6,$6,1
	sw	$6,8($5)
	.set	noreorder
	.set	nomacro
	jal	add_timer
	sw	$3,20($2)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L585
	move	$2,$19
	.set	macro
	.set	reorder

$L560:
	move	$12,$19
	.set	noreorder
	.set	nomacro
	b	$L369
	move	$23,$0
	.set	macro
	.set	reorder

$L559:
	li	$2,6			# 0x6
	sw	$2,16($sp)
	move	$6,$10
	move	$5,$22
	.set	noreorder
	.set	nomacro
	jal	mark_path_invalid
	move	$4,$8
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L359
	move	$19,$0
	.set	macro
	.set	reorder

$L556:
	.set	noreorder
	.set	nomacro
	bne	$21,$4,$L381
	lui	$2,%hi(rtl_check_bypass_packet_hook)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L591
	lw	$2,%lo(rtl_check_bypass_packet_hook)($2)
	.set	macro
	.set	reorder

$L378:
	addiu	$3,$3,%lo(negative_cache_list_free)
	lw	$4,52($2)
	.set	noreorder
	.set	nomacro
	b	$L379
	sw	$4,4($3)
	.set	macro
	.set	reorder

$L558:
	move	$6,$0
	lw	$8,44($sp)
	move	$4,$20
	lw	$10,48($sp)
	lw	$5,24($sp)
	sw	$8,48($sp)
	sw	$10,52($sp)
	sw	$11,44($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_skb_policy_route_mark_set
	sw	$2,40($sp)
	.set	macro
	.set	reorder

	move	$6,$17
	lw	$5,28($16)
	.set	noreorder
	.set	nomacro
	jal	rtl_skb_dst_check
	move	$4,$20
	.set	macro
	.set	reorder

	li	$3,-1			# 0xffffffffffffffff
	lw	$9,32($sp)
	lw	$12,36($sp)
	lw	$7,40($sp)
	lw	$11,44($sp)
	lw	$8,48($sp)
	.set	noreorder
	.set	nomacro
	beq	$2,$3,$L561
	lw	$10,52($sp)
	.set	macro
	.set	reorder

	li	$2,6			# 0x6
	.set	noreorder
	.set	nomacro
	beq	$21,$2,$L562
	addiu	$2,$18,6
	.set	macro
	.set	reorder

	lhu	$3,6($18)
	movz	$2,$0,$3
	move	$3,$2
$L391:
	.set	noreorder
	.set	nomacro
	beq	$12,$0,$L392
	lui	$6,%hi(max_udp_frag_entry)
	.set	macro
	.set	reorder

	lhu	$12,4($17)
	srl	$2,$11,16
	lui	$4,%hi(udp_cache_list_free)
	xor	$5,$12,$11
	lw	$6,%lo(max_udp_frag_entry)($6)
	xor	$5,$5,$2
	srl	$11,$5,8
	lw	$2,%lo(udp_cache_list_free)($4)
	xor	$5,$21,$5
	addiu	$6,$6,-1
	xor	$5,$5,$11
	andi	$22,$22,0xffff
	andi	$9,$9,0xffff
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L563
	and	$5,$6,$5
	.set	macro
	.set	reorder

	lw	$6,52($2)
	li	$7,1			# 0x1
	sh	$12,2($2)
	sw	$8,4($2)
	sw	$10,8($2)
	sb	$21,1($2)
	sh	$22,12($2)
	sh	$9,14($2)
	.set	noreorder
	.set	nomacro
	beq	$6,$0,$L394
	sb	$7,0($2)
	.set	macro
	.set	reorder

	lw	$7,56($2)
	addiu	$4,$4,%lo(udp_cache_list_free)
	sw	$7,56($6)
	lw	$6,52($2)
	lw	$7,56($2)
$L395:
	sw	$6,0($7)
	lui	$7,%hi(udp_cache_table)
	lw	$6,8($4)
	li	$9,12			# 0xc
	sw	$0,44($2)
	addiu	$11,$2,44
	mul	$5,$5,$9
	lw	$9,%lo(udp_cache_table)($7)
	addiu	$6,$6,-1
	sw	$10,40($sp)
	sw	$8,36($sp)
	sw	$6,8($4)
	addiu	$4,$2,16
	lw	$6,0($9)
	sw	$3,32($sp)
	addu	$6,$6,$5
	lw	$6,4($6)
	sw	$6,48($2)
	lw	$6,0($9)
	addu	$6,$6,$5
	lw	$6,4($6)
	sw	$2,0($6)
	lui	$6,%hi(jiffies)
	lw	$9,%lo(udp_cache_table)($7)
	lw	$7,%lo(jiffies)($6)
	lw	$6,0($9)
	addiu	$7,$7,1000
	addu	$6,$6,$5
	sw	$11,4($6)
	lw	$6,0($9)
	addu	$5,$6,$5
	lw	$6,8($5)
	addiu	$6,$6,1
	sw	$6,8($5)
	.set	noreorder
	.set	nomacro
	jal	add_timer
	sw	$7,24($2)
	.set	macro
	.set	reorder

	lui	$4,%hi(statistic_udp_frag_add)
	lw	$3,32($sp)
	lw	$8,36($sp)
	lw	$2,%lo(statistic_udp_frag_add)($4)
	lw	$10,40($sp)
	addiu	$2,$2,1
	sw	$2,%lo(statistic_udp_frag_add)($4)
$L396:
	move	$4,$20
	sw	$10,40($sp)
	move	$6,$21
	sw	$8,36($sp)
	move	$5,$17
	.set	noreorder
	.set	nomacro
	jal	fast_path_before_nat_check
	sw	$3,32($sp)
	.set	macro
	.set	reorder

	li	$4,1			# 0x1
	.set	noreorder
	.set	nomacro
	beq	$2,$4,$L467
	li	$2,6			# 0x6
	.set	macro
	.set	reorder

	lw	$3,32($sp)
	lw	$8,36($sp)
	.set	noreorder
	.set	nomacro
	beq	$21,$2,$L564
	lw	$10,40($sp)
	.set	macro
	.set	reorder

$L399:
	lbu	$2,1($16)
	sltu	$4,$2,13
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L400
	sll	$4,$2,2
	.set	macro
	.set	reorder

	lui	$2,%hi($L402)
	addiu	$2,$2,%lo($L402)
	addu	$2,$2,$4
	lw	$2,0($2)
	jr	$2
	.rdata
	.align	2
	.align	2
$L402:
	.word	$L401
	.word	$L403
	.word	$L404
	.word	$L404
	.word	$L405
	.word	$L400
	.word	$L400
	.word	$L400
	.word	$L406
	.word	$L400
	.word	$L400
	.word	$L400
	.word	$L406
	.text
$L557:
	.set	noreorder
	.set	nomacro
	b	$L387
	move	$7,$0
	.set	macro
	.set	reorder

$L406:
	wsbh	$10,$10
	lw	$4,28($16)
	lhu	$2,10($17)
	ror	$10,$10,16
	wsbh	$4,$4
	srl	$5,$10,16
	ror	$4,$4,16
	andi	$10,$10,0xffff
	addu	$5,$5,$10
	andi	$6,$4,0xffff
	subu	$6,$5,$6
	srl	$4,$4,16
	wsbh	$2,$2
	subu	$4,$6,$4
	andi	$2,$2,0xffff
	addu	$2,$2,$4
	.set	noreorder
	.set	nomacro
	bltz	$2,$L565
	sra	$4,$2,16
	.set	macro
	.set	reorder

	andi	$2,$2,0xffff
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$2,$4
	wsbh	$2,$2
	sh	$2,10($17)
$L433:
	.set	noreorder
	.set	nomacro
	beq	$19,$0,$L546
	li	$2,17			# 0x11
	.set	macro
	.set	reorder

	beq	$21,$2,$L566
	lw	$6,28($16)
	lhu	$4,2($18)
	lhu	$7,32($16)
	wsbh	$6,$6
	lhu	$2,0($3)
	wsbh	$4,$4
	ror	$6,$6,16
	andi	$4,$4,0xffff
	andi	$8,$6,0xffff
	subu	$5,$5,$8
	srl	$6,$6,16
	wsbh	$7,$7
	subu	$6,$5,$6
	wsbh	$2,$2
	addu	$4,$4,$6
	andi	$5,$7,0xffff
	subu	$4,$4,$5
	andi	$2,$2,0xffff
	addu	$2,$2,$4
	.set	noreorder
	.set	nomacro
	bltz	$2,$L544
	sra	$4,$2,16
	.set	macro
	.set	reorder

$L589:
	andi	$2,$2,0xffff
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$2,$4
	wsbh	$2,$2
	sh	$2,0($3)
$L436:
	lhu	$2,32($16)
	sh	$2,2($18)
$L546:
	lw	$2,28($16)
	sw	$2,16($17)
$L401:
	lbu	$3,8($17)
	lbu	$6,9($17)
	lhu	$2,10($17)
	addiu	$4,$3,-1
	sll	$3,$3,8
	andi	$4,$4,0x00ff
	sll	$5,$4,8
	sb	$4,8($17)
	or	$3,$3,$6
	or	$4,$5,$6
	wsbh	$2,$2
	subu	$3,$3,$4
	andi	$2,$2,0xffff
	addu	$2,$2,$3
	.set	noreorder
	.set	nomacro
	bltz	$2,$L567
	sra	$3,$2,16
	.set	macro
	.set	reorder

	andi	$2,$2,0xffff
	addu	$2,$3,$2
	sra	$3,$2,16
	addu	$2,$2,$3
	wsbh	$2,$2
	sh	$2,10($17)
$L453:
	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_ip_summed
	move	$4,$20
	.set	macro
	.set	reorder

	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_dev
	move	$4,$20
	.set	macro
	.set	reorder

	move	$4,$20
	lw	$6,48($16)
	.set	noreorder
	.set	nomacro
	jal	fastpath_set_qos_mark
	lw	$5,44($16)
	.set	macro
	.set	reorder

	move	$5,$17
	.set	noreorder
	.set	nomacro
	jal	rtl_set_iph_dscp
	move	$4,$16
	.set	macro
	.set	reorder

	move	$5,$20
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_remark_vlanpri
	move	$4,$16
	.set	macro
	.set	reorder

	li	$2,65535			# 0xffff
	lhu	$5,74($16)
	.set	noreorder
	.set	nomacro
	beq	$5,$2,$L454
	li	$2,2			# 0x2
	.set	macro
	.set	reorder

	lbu	$3,2($16)
	bne	$3,$2,$L454
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_vlanId
	move	$4,$20
	.set	macro
	.set	reorder

$L454:
	lhu	$5,76($16)
	li	$2,65535			# 0xffff
	.set	noreorder
	.set	nomacro
	beq	$5,$2,$L455
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

	lbu	$3,2($16)
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L588
	lui	$2,%hi(jiffies)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_vlanId
	move	$4,$20
	.set	macro
	.set	reorder

$L455:
	lui	$2,%hi(jiffies)
$L588:
	move	$6,$20
	move	$5,$18
	lw	$2,%lo(jiffies)($2)
	move	$4,$17
	.set	noreorder
	.set	nomacro
	jal	fast_path_post_process_xmit_check
	sw	$2,36($16)
	.set	macro
	.set	reorder

	move	$7,$2
	li	$2,1			# 0x1
	.set	noreorder
	.set	nomacro
	bne	$7,$2,$L456
	lui	$3,%hi(statistic_fp)
	.set	macro
	.set	reorder

	lw	$2,%lo(statistic_fp)($3)
	addiu	$2,$2,1
	.set	noreorder
	.set	nomacro
	b	$L387
	sw	$2,%lo(statistic_fp)($3)
	.set	macro
	.set	reorder

$L567:
	addiu	$2,$2,-1
	wsbh	$2,$2
	.set	noreorder
	.set	nomacro
	b	$L453
	sh	$2,10($17)
	.set	macro
	.set	reorder

$L400:
	wsbh	$8,$8
	lw	$4,20($16)
	lhu	$2,10($17)
	ror	$8,$8,16
	wsbh	$4,$4
	srl	$5,$8,16
	ror	$4,$4,16
	andi	$8,$8,0xffff
	addu	$5,$5,$8
	andi	$6,$4,0xffff
	subu	$6,$5,$6
	srl	$4,$4,16
	wsbh	$2,$2
	subu	$4,$6,$4
	andi	$2,$2,0xffff
	addu	$2,$2,$4
	.set	noreorder
	.set	nomacro
	bltz	$2,$L568
	sra	$4,$2,16
	.set	macro
	.set	reorder

	andi	$2,$2,0xffff
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$2,$4
	wsbh	$2,$2
	sh	$2,10($17)
$L440:
	wsbh	$10,$10
	lw	$6,28($16)
	lhu	$2,10($17)
	ror	$10,$10,16
	wsbh	$6,$6
	srl	$4,$10,16
	ror	$6,$6,16
	andi	$10,$10,0xffff
	addu	$4,$4,$10
	andi	$7,$6,0xffff
	subu	$7,$4,$7
	srl	$6,$6,16
	wsbh	$2,$2
	subu	$6,$7,$6
	andi	$2,$2,0xffff
	addu	$2,$2,$6
	.set	noreorder
	.set	nomacro
	bltz	$2,$L569
	sra	$6,$2,16
	.set	macro
	.set	reorder

	andi	$2,$2,0xffff
	addu	$2,$6,$2
	sra	$6,$2,16
	addu	$2,$2,$6
	wsbh	$2,$2
	sh	$2,10($17)
$L442:
	.set	noreorder
	.set	nomacro
	beq	$19,$0,$L443
	li	$2,17			# 0x11
	.set	macro
	.set	reorder

	beq	$21,$2,$L570
	lw	$7,20($16)
	lhu	$6,0($18)
	lhu	$8,24($16)
	wsbh	$7,$7
	lhu	$2,0($3)
	wsbh	$6,$6
	ror	$7,$7,16
	andi	$6,$6,0xffff
	andi	$9,$7,0xffff
	subu	$5,$5,$9
	srl	$7,$7,16
	wsbh	$8,$8
	subu	$5,$5,$7
	wsbh	$2,$2
	addu	$5,$6,$5
	andi	$7,$8,0xffff
	subu	$5,$5,$7
	andi	$2,$2,0xffff
	addu	$2,$2,$5
	.set	noreorder
	.set	nomacro
	bltz	$2,$L571
	sra	$5,$2,16
	.set	macro
	.set	reorder

	andi	$2,$2,0xffff
	addu	$2,$5,$2
	sra	$5,$2,16
	addu	$2,$2,$5
	wsbh	$2,$2
	sh	$2,0($3)
$L450:
	lw	$6,28($16)
	lhu	$5,2($18)
	lhu	$7,32($16)
	wsbh	$6,$6
	lhu	$2,0($3)
	wsbh	$5,$5
	ror	$6,$6,16
	andi	$5,$5,0xffff
	andi	$8,$6,0xffff
	subu	$4,$4,$8
	srl	$6,$6,16
	wsbh	$7,$7
	subu	$4,$4,$6
	andi	$6,$7,0xffff
	addu	$4,$5,$4
$L551:
	wsbh	$2,$2
	subu	$4,$4,$6
	andi	$2,$2,0xffff
	addu	$2,$2,$4
	.set	noreorder
	.set	nomacro
	bltz	$2,$L572
	sra	$4,$2,16
	.set	macro
	.set	reorder

	andi	$2,$2,0xffff
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$2,$4
	wsbh	$2,$2
	sh	$2,0($3)
$L445:
	lhu	$2,24($16)
	sh	$2,0($18)
	lhu	$2,32($16)
	sh	$2,2($18)
$L443:
	lw	$2,20($16)
	.set	noreorder
	.set	nomacro
	b	$L546
	sw	$2,12($17)
	.set	macro
	.set	reorder

$L569:
	subu	$2,$0,$2
	sra	$6,$2,16
	andi	$2,$2,0xffff
	addu	$2,$6,$2
	sra	$6,$2,16
	addu	$2,$2,$6
	nor	$2,$0,$2
	wsbh	$2,$2
	.set	noreorder
	.set	nomacro
	b	$L442
	sh	$2,10($17)
	.set	macro
	.set	reorder

$L568:
	subu	$2,$0,$2
	sra	$4,$2,16
	andi	$2,$2,0xffff
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$2,$4
	nor	$2,$0,$2
	wsbh	$2,$2
	.set	noreorder
	.set	nomacro
	b	$L440
	sh	$2,10($17)
	.set	macro
	.set	reorder

$L467:
	.set	noreorder
	.set	nomacro
	b	$L387
	li	$7,1			# 0x1
	.set	macro
	.set	reorder

$L394:
	addiu	$4,$4,%lo(udp_cache_list_free)
	lw	$7,56($2)
	.set	noreorder
	.set	nomacro
	b	$L395
	sw	$7,4($4)
	.set	macro
	.set	reorder

$L405:
	wsbh	$10,$10
	lw	$4,28($16)
	lhu	$2,10($17)
	ror	$10,$10,16
	wsbh	$4,$4
	srl	$5,$10,16
	ror	$4,$4,16
	andi	$10,$10,0xffff
	addu	$5,$5,$10
	andi	$6,$4,0xffff
	subu	$6,$5,$6
	srl	$4,$4,16
	wsbh	$2,$2
	subu	$4,$6,$4
	andi	$2,$2,0xffff
	addu	$2,$2,$4
	.set	noreorder
	.set	nomacro
	bltz	$2,$L573
	sra	$4,$2,16
	.set	macro
	.set	reorder

	andi	$2,$2,0xffff
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$2,$4
	wsbh	$2,$2
	sh	$2,10($17)
$L427:
	.set	noreorder
	.set	nomacro
	beq	$19,$0,$L546
	li	$2,17			# 0x11
	.set	macro
	.set	reorder

	beq	$21,$2,$L574
	lw	$4,28($16)
	lhu	$2,0($3)
	wsbh	$4,$4
	wsbh	$2,$2
	ror	$4,$4,16
	andi	$2,$2,0xffff
$L549:
	andi	$6,$4,0xffff
	subu	$5,$5,$6
	srl	$4,$4,16
	subu	$4,$5,$4
	addu	$2,$2,$4
	.set	noreorder
	.set	nomacro
	bltz	$2,$L575
	sra	$4,$2,16
	.set	macro
	.set	reorder

	andi	$2,$2,0xffff
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$2,$4
	wsbh	$2,$2
	.set	noreorder
	.set	nomacro
	b	$L546
	sh	$2,0($3)
	.set	macro
	.set	reorder

$L403:
	lw	$4,20($16)
	wsbh	$8,$8
	lhu	$2,10($17)
	ror	$8,$8,16
	wsbh	$4,$4
	srl	$5,$8,16
	ror	$4,$4,16
	andi	$8,$8,0xffff
	srl	$6,$4,16
	andi	$4,$4,0xffff
	addu	$4,$4,$6
	wsbh	$2,$2
	subu	$4,$5,$4
	andi	$2,$2,0xffff
	addu	$8,$4,$8
	addu	$2,$2,$8
	.set	noreorder
	.set	nomacro
	bltz	$2,$L576
	sra	$4,$2,16
	.set	macro
	.set	reorder

	andi	$2,$2,0xffff
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$2,$4
	wsbh	$2,$2
	sh	$2,10($17)
$L408:
	.set	noreorder
	.set	nomacro
	beq	$19,$0,$L418
	li	$2,17			# 0x11
	.set	macro
	.set	reorder

	beq	$21,$2,$L577
	lw	$4,20($16)
	lw	$5,12($17)
	lhu	$2,0($3)
	wsbh	$4,$4
	wsbh	$5,$5
	ror	$4,$4,16
	ror	$5,$5,16
	srl	$7,$4,16
	andi	$4,$4,0xffff
	srl	$6,$5,16
	addu	$4,$4,$7
	andi	$5,$5,0xffff
	subu	$4,$6,$4
	wsbh	$2,$2
	addu	$5,$4,$5
	andi	$2,$2,0xffff
	addu	$2,$2,$5
	.set	noreorder
	.set	nomacro
	bltz	$2,$L578
	sra	$4,$2,16
	.set	macro
	.set	reorder

	andi	$2,$2,0xffff
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$2,$4
	wsbh	$2,$2
	sh	$2,0($3)
$L415:
	lhu	$2,12($18)
	andi	$2,$2,0x1000
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L418
	li	$2,10240			# 0x2800
	.set	macro
	.set	reorder

	lhu	$3,2($17)
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L418
	li	$5,10240			# 0x2800
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	skb_trim
	move	$4,$20
	.set	macro
	.set	reorder

$L418:
	lw	$2,20($16)
	.set	noreorder
	.set	nomacro
	b	$L401
	sw	$2,12($17)
	.set	macro
	.set	reorder

$L576:
	subu	$2,$0,$2
	sra	$4,$2,16
	andi	$2,$2,0xffff
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$2,$4
	nor	$2,$0,$2
	wsbh	$2,$2
	.set	noreorder
	.set	nomacro
	b	$L408
	sh	$2,10($17)
	.set	macro
	.set	reorder

$L578:
	subu	$2,$0,$2
	sra	$4,$2,16
	andi	$2,$2,0xffff
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$2,$4
	nor	$2,$0,$2
	wsbh	$2,$2
	.set	noreorder
	.set	nomacro
	b	$L415
	sh	$2,0($3)
	.set	macro
	.set	reorder

$L577:
	beq	$3,$0,$L418
	lhu	$2,0($3)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L418
	wsbh	$2,$2
	.set	macro
	.set	reorder

	lw	$4,20($16)
	lw	$5,12($17)
	andi	$2,$2,0xffff
	wsbh	$4,$4
	wsbh	$5,$5
	ror	$4,$4,16
	ror	$5,$5,16
	srl	$7,$4,16
	andi	$4,$4,0xffff
	srl	$6,$5,16
	addu	$4,$4,$7
	andi	$5,$5,0xffff
	subu	$4,$6,$4
	addu	$5,$4,$5
	addu	$2,$2,$5
	.set	noreorder
	.set	nomacro
	bltz	$2,$L579
	sra	$4,$2,16
	.set	macro
	.set	reorder

	andi	$2,$2,0xffff
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$2,$4
	wsbh	$2,$2
	.set	noreorder
	.set	nomacro
	b	$L418
	sh	$2,0($3)
	.set	macro
	.set	reorder

$L404:
	lw	$4,20($16)
	wsbh	$8,$8
	lhu	$2,10($17)
	ror	$8,$8,16
	wsbh	$4,$4
	srl	$5,$8,16
	ror	$4,$4,16
	andi	$8,$8,0xffff
	srl	$6,$4,16
	andi	$4,$4,0xffff
	addu	$4,$4,$6
	wsbh	$2,$2
	subu	$4,$5,$4
	andi	$2,$2,0xffff
	addu	$8,$4,$8
	addu	$2,$2,$8
	.set	noreorder
	.set	nomacro
	bltz	$2,$L580
	sra	$4,$2,16
	.set	macro
	.set	reorder

	andi	$2,$2,0xffff
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$2,$4
	wsbh	$2,$2
	sh	$2,10($17)
$L417:
	.set	noreorder
	.set	nomacro
	beq	$19,$0,$L418
	li	$2,17			# 0x11
	.set	macro
	.set	reorder

	beq	$21,$2,$L581
	lw	$4,20($16)
	lw	$6,12($17)
	lhu	$5,0($18)
	wsbh	$4,$4
	lhu	$7,24($16)
	wsbh	$6,$6
	lhu	$2,0($3)
	ror	$4,$4,16
	ror	$6,$6,16
	srl	$9,$4,16
	andi	$4,$4,0xffff
	srl	$8,$6,16
	addu	$4,$4,$9
	andi	$6,$6,0xffff
	subu	$8,$8,$4
	wsbh	$4,$5
	addu	$6,$8,$6
	andi	$4,$4,0xffff
	wsbh	$5,$7
	addu	$4,$4,$6
	andi	$5,$5,0xffff
	wsbh	$2,$2
	subu	$4,$4,$5
	andi	$2,$2,0xffff
	addu	$2,$2,$4
	.set	noreorder
	.set	nomacro
	bltz	$2,$L582
	sra	$4,$2,16
	.set	macro
	.set	reorder

	andi	$2,$2,0xffff
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$2,$4
	wsbh	$2,$2
	sh	$2,0($3)
$L425:
	lhu	$2,12($18)
	andi	$2,$2,0x1000
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L421
	li	$2,10240			# 0x2800
	.set	macro
	.set	reorder

	lhu	$3,2($17)
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L421
	li	$5,10240			# 0x2800
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	skb_trim
	move	$4,$20
	.set	macro
	.set	reorder

$L421:
	lhu	$2,24($16)
	.set	noreorder
	.set	nomacro
	b	$L418
	sh	$2,0($18)
	.set	macro
	.set	reorder

$L582:
	subu	$2,$0,$2
	sra	$4,$2,16
	andi	$2,$2,0xffff
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$2,$4
	nor	$2,$0,$2
	wsbh	$2,$2
	.set	noreorder
	.set	nomacro
	b	$L425
	sh	$2,0($3)
	.set	macro
	.set	reorder

$L581:
	beq	$3,$0,$L421
	lhu	$2,0($3)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L421
	wsbh	$2,$2
	.set	macro
	.set	reorder

	lw	$4,20($16)
	lw	$6,12($17)
	lhu	$5,0($18)
	andi	$2,$2,0xffff
	wsbh	$4,$4
	lhu	$7,24($16)
	wsbh	$6,$6
	ror	$4,$4,16
	ror	$6,$6,16
	srl	$9,$4,16
	andi	$4,$4,0xffff
	srl	$8,$6,16
	addu	$4,$4,$9
	andi	$6,$6,0xffff
	subu	$8,$8,$4
	wsbh	$4,$5
	addu	$6,$8,$6
	andi	$4,$4,0xffff
	wsbh	$5,$7
	addu	$4,$4,$6
	andi	$5,$5,0xffff
	subu	$4,$4,$5
	addu	$2,$2,$4
	.set	noreorder
	.set	nomacro
	bltz	$2,$L583
	sra	$4,$2,16
	.set	macro
	.set	reorder

	andi	$2,$2,0xffff
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$2,$4
	wsbh	$2,$2
	.set	noreorder
	.set	nomacro
	b	$L421
	sh	$2,0($3)
	.set	macro
	.set	reorder

$L580:
	subu	$2,$0,$2
	sra	$4,$2,16
	andi	$2,$2,0xffff
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$2,$4
	nor	$2,$0,$2
	wsbh	$2,$2
	.set	noreorder
	.set	nomacro
	b	$L417
	sh	$2,10($17)
	.set	macro
	.set	reorder

$L583:
	subu	$2,$0,$2
	sra	$4,$2,16
	andi	$2,$2,0xffff
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$2,$4
	nor	$2,$0,$2
	wsbh	$2,$2
	.set	noreorder
	.set	nomacro
	b	$L421
	sh	$2,0($3)
	.set	macro
	.set	reorder

$L563:
	lui	$3,%hi(statistic_udp_frag_tbl_full)
	sw	$7,32($sp)
	move	$4,$20
	lw	$2,%lo(statistic_udp_frag_tbl_full)($3)
	addiu	$2,$2,1
	.set	noreorder
	.set	nomacro
	jal	rtl_dst_release_ipv4
	sw	$2,%lo(statistic_udp_frag_tbl_full)($3)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L387
	lw	$7,32($sp)
	.set	macro
	.set	reorder

$L392:
	.set	noreorder
	.set	nomacro
	beq	$fp,$0,$L396
	addiu	$4,$23,16
	.set	macro
	.set	reorder

	sw	$10,40($sp)
	sw	$8,36($sp)
	.set	noreorder
	.set	nomacro
	jal	del_timer
	sw	$3,32($sp)
	.set	macro
	.set	reorder

	lw	$4,8($23)
	lw	$5,4($23)
	lhu	$7,2($23)
	lbu	$2,1($23)
	xor	$5,$5,$4
	lw	$9,44($23)
	lui	$4,%hi(max_udp_frag_entry)
	lw	$3,32($sp)
	lw	$8,36($sp)
	lw	$10,40($sp)
	lw	$6,%lo(max_udp_frag_entry)($4)
	srl	$4,$5,16
	xor	$5,$7,$5
	sb	$0,0($23)
	xor	$4,$4,$5
	srl	$5,$4,8
	xor	$2,$2,$4
	addiu	$4,$6,-1
	xor	$2,$2,$5
	.set	noreorder
	.set	nomacro
	beq	$9,$0,$L397
	and	$4,$4,$2
	.set	macro
	.set	reorder

	lw	$5,48($23)
	lui	$2,%hi(udp_cache_table)
	sw	$5,48($9)
$L398:
	lw	$7,48($23)
	li	$11,12			# 0xc
	lw	$9,44($23)
	lui	$5,%hi(udp_cache_list_free)
	mul	$12,$4,$11
	addiu	$5,$5,%lo(udp_cache_list_free)
	sw	$9,0($7)
	lui	$6,%hi(statistic_udp_frag_free)
	lw	$2,%lo(udp_cache_table)($2)
	addiu	$9,$23,52
	lw	$7,4($5)
	lw	$2,0($2)
	addu	$2,$12,$2
	lw	$4,8($2)
	addiu	$4,$4,-1
	sw	$4,8($2)
	sw	$0,52($23)
	sw	$7,56($23)
	sw	$23,0($7)
	lw	$4,8($5)
	lw	$2,%lo(statistic_udp_frag_free)($6)
	sw	$9,4($5)
	addiu	$4,$4,1
	addiu	$2,$2,1
	sw	$4,8($5)
	.set	noreorder
	.set	nomacro
	b	$L396
	sw	$2,%lo(statistic_udp_frag_free)($6)
	.set	macro
	.set	reorder

$L562:
	.set	noreorder
	.set	nomacro
	b	$L391
	addiu	$3,$18,16
	.set	macro
	.set	reorder

$L561:
	move	$4,$20
	.set	noreorder
	.set	nomacro
	jal	rtl_dst_release_ipv4
	sw	$7,32($sp)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L387
	lw	$7,32($sp)
	.set	macro
	.set	reorder

$L397:
	lui	$2,%hi(udp_cache_table)
	lw	$6,48($23)
	lw	$5,%lo(udp_cache_table)($2)
	lw	$7,0($5)
	li	$5,12			# 0xc
	mul	$9,$4,$5
	addu	$5,$9,$7
	.set	noreorder
	.set	nomacro
	b	$L398
	sw	$6,4($5)
	.set	macro
	.set	reorder

$L564:
	lhu	$2,12($18)
	andi	$2,$2,0x1000
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L399
	li	$2,40			# 0x28
	.set	macro
	.set	reorder

	lhu	$4,2($17)
	.set	noreorder
	.set	nomacro
	bne	$4,$2,$L399
	li	$5,40			# 0x28
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	skb_trim
	move	$4,$20
	.set	macro
	.set	reorder

	lw	$10,40($sp)
	lw	$8,36($sp)
	.set	noreorder
	.set	nomacro
	b	$L399
	lw	$3,32($sp)
	.set	macro
	.set	reorder

$L572:
	subu	$2,$0,$2
	sra	$4,$2,16
	andi	$2,$2,0xffff
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$2,$4
	nor	$2,$0,$2
	wsbh	$2,$2
	.set	noreorder
	.set	nomacro
	b	$L445
	sh	$2,0($3)
	.set	macro
	.set	reorder

$L571:
	subu	$2,$0,$2
	sra	$5,$2,16
	andi	$2,$2,0xffff
	addu	$2,$5,$2
	sra	$5,$2,16
	addu	$2,$2,$5
	nor	$2,$0,$2
	wsbh	$2,$2
	.set	noreorder
	.set	nomacro
	b	$L450
	sh	$2,0($3)
	.set	macro
	.set	reorder

$L570:
	beq	$3,$0,$L445
	lhu	$2,0($3)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L445
	wsbh	$2,$2
	.set	macro
	.set	reorder

	lw	$7,20($16)
	lhu	$6,0($18)
	lhu	$8,24($16)
	andi	$2,$2,0xffff
	wsbh	$7,$7
	wsbh	$6,$6
	ror	$7,$7,16
	andi	$6,$6,0xffff
	andi	$9,$7,0xffff
	subu	$9,$5,$9
	srl	$7,$7,16
	wsbh	$5,$8
	subu	$8,$9,$7
	andi	$7,$5,0xffff
	addu	$5,$6,$8
	subu	$5,$5,$7
	addu	$2,$2,$5
	.set	noreorder
	.set	nomacro
	bltz	$2,$L584
	sra	$5,$2,16
	.set	macro
	.set	reorder

	andi	$2,$2,0xffff
	addu	$2,$5,$2
	sra	$5,$2,16
	addu	$2,$2,$5
	wsbh	$2,$2
	sh	$2,0($3)
$L447:
	lw	$6,28($16)
	lhu	$5,2($18)
	lhu	$7,32($16)
	wsbh	$6,$6
	lhu	$2,0($3)
	wsbh	$5,$5
	ror	$6,$6,16
	andi	$5,$5,0xffff
	andi	$8,$6,0xffff
	subu	$8,$4,$8
	srl	$6,$6,16
	wsbh	$4,$7
	subu	$7,$8,$6
	andi	$6,$4,0xffff
	.set	noreorder
	.set	nomacro
	b	$L551
	addu	$4,$5,$7
	.set	macro
	.set	reorder

$L566:
	beq	$3,$0,$L436
	lhu	$2,0($3)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L436
	wsbh	$2,$2
	.set	macro
	.set	reorder

	lw	$6,28($16)
	lhu	$4,2($18)
	lhu	$7,32($16)
	andi	$2,$2,0xffff
	wsbh	$6,$6
	wsbh	$4,$4
	ror	$6,$6,16
	andi	$4,$4,0xffff
	andi	$8,$6,0xffff
	subu	$8,$5,$8
	srl	$6,$6,16
	wsbh	$5,$7
	subu	$6,$8,$6
	andi	$5,$5,0xffff
	addu	$4,$4,$6
	subu	$4,$4,$5
	addu	$2,$2,$4
	.set	noreorder
	.set	nomacro
	bgez	$2,$L589
	sra	$4,$2,16
	.set	macro
	.set	reorder

$L544:
	subu	$2,$0,$2
	sra	$4,$2,16
	andi	$2,$2,0xffff
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$2,$4
	nor	$2,$0,$2
	wsbh	$2,$2
	.set	noreorder
	.set	nomacro
	b	$L436
	sh	$2,0($3)
	.set	macro
	.set	reorder

$L574:
	beq	$3,$0,$L546
	lhu	$2,0($3)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L546
	wsbh	$2,$2
	.set	macro
	.set	reorder

	lw	$4,28($16)
	andi	$2,$2,0xffff
	wsbh	$4,$4
	.set	noreorder
	.set	nomacro
	b	$L549
	ror	$4,$4,16
	.set	macro
	.set	reorder

$L573:
	subu	$2,$0,$2
	sra	$4,$2,16
	andi	$2,$2,0xffff
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$2,$4
	nor	$2,$0,$2
	wsbh	$2,$2
	.set	noreorder
	.set	nomacro
	b	$L427
	sh	$2,10($17)
	.set	macro
	.set	reorder

$L575:
	subu	$2,$0,$2
	sra	$4,$2,16
	andi	$2,$2,0xffff
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$2,$4
	nor	$2,$0,$2
	wsbh	$2,$2
	.set	noreorder
	.set	nomacro
	b	$L546
	sh	$2,0($3)
	.set	macro
	.set	reorder

$L579:
	subu	$2,$0,$2
	sra	$4,$2,16
	andi	$2,$2,0xffff
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$2,$4
	nor	$2,$0,$2
	wsbh	$2,$2
	.set	noreorder
	.set	nomacro
	b	$L418
	sh	$2,0($3)
	.set	macro
	.set	reorder

$L565:
	subu	$2,$0,$2
	sra	$4,$2,16
	andi	$2,$2,0xffff
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$2,$4
	nor	$2,$0,$2
	wsbh	$2,$2
	.set	noreorder
	.set	nomacro
	b	$L433
	sh	$2,10($17)
	.set	macro
	.set	reorder

$L456:
	.set	noreorder
	.set	nomacro
	jal	ip_finish_output3
	move	$4,$20
	.set	macro
	.set	reorder

	lui	$3,%hi(statistic_fp)
	li	$7,1			# 0x1
	lw	$2,%lo(statistic_fp)($3)
	addiu	$2,$2,1
	.set	noreorder
	.set	nomacro
	b	$L387
	sw	$2,%lo(statistic_fp)($3)
	.set	macro
	.set	reorder

$L584:
	subu	$2,$0,$2
	sra	$5,$2,16
	andi	$2,$2,0xffff
	addu	$2,$5,$2
	sra	$5,$2,16
	addu	$2,$2,$5
	nor	$2,$0,$2
	wsbh	$2,$2
	.set	noreorder
	.set	nomacro
	b	$L447
	sh	$2,0($3)
	.set	macro
	.set	reorder

	.end	enter_fast_path
	.size	enter_fast_path, .-enter_fast_path
	.section	.rodata.str1.4
	.align	2
$LC10:
	.ascii	"napt num: %d, path num: %d.\012\000"
	.text
	.align	2
	.globl	fastpath_dump_napt_entry_num
	.set	nomips16
	.set	nomicromips
	.ent	fastpath_dump_napt_entry_num
	.type	fastpath_dump_napt_entry_num, @function
fastpath_dump_napt_entry_num:
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$16,16($sp)
	lui	$2,%hi(path_list_inuse)
	move	$16,$5
	lw	$2,%lo(path_list_inuse)($2)
	beq	$2,$0,$L593
	move	$7,$0

$L594:
	lw	$2,88($2)
	bne	$2,$0,$L594
	addiu	$7,$7,1

$L593:
	lui	$2,%hi(napt_list_inuse)
	lw	$2,%lo(napt_list_inuse)($2)
	beq	$2,$0,$L595
	move	$6,$0

$L596:
	lw	$2,44($2)
	bne	$2,$0,$L596
	addiu	$6,$6,1

$L595:
	lui	$5,%hi($LC10)
	addu	$4,$4,$16
	jal	sprintf
	addiu	$5,$5,%lo($LC10)

	addu	$2,$16,$2
	lw	$31,20($sp)
	lw	$16,16($sp)
	jr	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	fastpath_dump_napt_entry_num
	.size	fastpath_dump_napt_entry_num, .-fastpath_dump_napt_entry_num
	.section	.rodata.str1.4
	.align	2
$LC11:
	.ascii	"\012udp frag cache entry:\012\000"
	.align	2
$LC12:
	.ascii	"src_ip: 0x%08x src_port:0x%04x dst_ip: 0x%08x dst_port:0"
	.ascii	"x%04x frag_id:0x%04x protocol:0x%x status:0x%x \012\000"
	.align	2
$LC13:
	.ascii	"total num: %d \012\000"
	.text
	.align	2
	.globl	rtl_readUdpFragEntry
	.set	nomips16
	.set	nomicromips
	.ent	rtl_readUdpFragEntry
	.type	rtl_readUdpFragEntry, @function
rtl_readUdpFragEntry:
	.frame	$sp,64,$31		# vars= 0, regs= 7/0, args= 32, gp= 0
	.mask	0x803f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-64
	sw	$31,60($sp)
	sw	$21,56($sp)
	sw	$20,52($sp)
	sw	$19,48($sp)
	sw	$18,44($sp)
	sw	$17,40($sp)
	sw	$16,36($sp)
	lui	$4,%hi($LC11)
	move	$16,$0
	addiu	$4,$4,%lo($LC11)
	jal	printk
	move	$21,$0

	lui	$2,%hi($LC12)
	lui	$18,%hi(udp_cache_table)
	addiu	$20,$2,%lo($LC12)
	li	$19,768			# 0x300
	lw	$2,%lo(udp_cache_table)($18)
$L613:
	lw	$2,0($2)
	addu	$2,$2,$16
	lw	$17,0($2)
	bne	$17,$0,$L604
	nop

	addiu	$16,$16,12
$L614:
	bne	$16,$19,$L613
	lw	$2,%lo(udp_cache_table)($18)

	lui	$4,%hi($LC13)
	lw	$31,60($sp)
	move	$5,$21
	lw	$20,52($sp)
	lw	$21,56($sp)
	addiu	$4,$4,%lo($LC13)
	lw	$19,48($sp)
	lw	$18,44($sp)
	lw	$17,40($sp)
	lw	$16,36($sp)
	j	printk
	addiu	$sp,$sp,64

$L604:
	lbu	$2,0($17)
$L612:
	move	$4,$20
	lw	$7,8($17)
	addiu	$21,$21,1
	lhu	$6,12($17)
	lw	$5,4($17)
	sw	$2,28($sp)
	lbu	$2,1($17)
	sw	$2,24($sp)
	lhu	$2,2($17)
	sw	$2,20($sp)
	lhu	$2,14($17)
	jal	printk
	sw	$2,16($sp)

	lw	$17,44($17)
	beq	$17,$0,$L614
	addiu	$16,$16,12

	addiu	$16,$16,-12
	b	$L612
	lbu	$2,0($17)

	.set	macro
	.set	reorder
	.end	rtl_readUdpFragEntry
	.size	rtl_readUdpFragEntry, .-rtl_readUdpFragEntry
	.section	.rodata.str1.4
	.align	2
$LC14:
	.ascii	"fp_napt\000"
	.align	2
$LC15:
	.ascii	"fp_path\000"
	.text
	.align	2
	.globl	init_fastpath_debug_proc
	.set	nomips16
	.set	nomicromips
	.ent	init_fastpath_debug_proc
	.type	init_fastpath_debug_proc, @function
init_fastpath_debug_proc:
	.frame	$sp,32,$31		# vars= 0, regs= 2/0, args= 24, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$16,24($sp)
	lui	$16,%hi(proc_root)
	lui	$7,%hi(fastpath_napt_proc_fops)
	sw	$0,16($sp)
	lui	$4,%hi($LC14)
	addiu	$6,$16,%lo(proc_root)
	addiu	$7,$7,%lo(fastpath_napt_proc_fops)
	move	$5,$0
	jal	proc_create_data
	addiu	$4,$4,%lo($LC14)

	lui	$7,%hi(fastpath_path_proc_fops)
	lui	$4,%hi($LC15)
	sw	$0,16($sp)
	lui	$3,%hi(fp_napt)
	addiu	$6,$16,%lo(proc_root)
	addiu	$7,$7,%lo(fastpath_path_proc_fops)
	move	$5,$0
	sw	$2,%lo(fp_napt)($3)
	jal	proc_create_data
	addiu	$4,$4,%lo($LC15)

	lui	$3,%hi(fp_path)
	lw	$31,28($sp)
	lw	$16,24($sp)
	sw	$2,%lo(fp_path)($3)
	jr	$31
	addiu	$sp,$sp,32

	.set	macro
	.set	reorder
	.end	init_fastpath_debug_proc
	.size	init_fastpath_debug_proc, .-init_fastpath_debug_proc
	.align	2
	.globl	remove_fastpath_debug_proc
	.set	nomips16
	.set	nomicromips
	.ent	remove_fastpath_debug_proc
	.type	remove_fastpath_debug_proc, @function
remove_fastpath_debug_proc:
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$16,16($sp)
	lui	$16,%hi(fp_napt)
	lw	$2,%lo(fp_napt)($16)
	beq	$2,$0,$L618
	lui	$5,%hi(proc_root)

	lui	$4,%hi($LC14)
	addiu	$5,$5,%lo(proc_root)
	jal	remove_proc_entry
	addiu	$4,$4,%lo($LC14)

	sw	$0,%lo(fp_napt)($16)
$L618:
	lui	$16,%hi(fp_path)
	lw	$2,%lo(fp_path)($16)
	beq	$2,$0,$L617
	lui	$5,%hi(proc_root)

	lui	$4,%hi($LC15)
	addiu	$5,$5,%lo(proc_root)
	jal	remove_proc_entry
	addiu	$4,$4,%lo($LC15)

	sw	$0,%lo(fp_path)($16)
$L617:
	lw	$31,20($sp)
	lw	$16,16($sp)
	jr	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	remove_fastpath_debug_proc
	.size	remove_fastpath_debug_proc, .-remove_fastpath_debug_proc
	.align	2
	.globl	init_table_napt
	.set	nomips16
	.set	nomicromips
	.ent	init_table_napt
	.type	init_table_napt, @function
init_table_napt:
	.frame	$sp,48,$31		# vars= 0, regs= 8/0, args= 16, gp= 0
	.mask	0x807f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-48
	sw	$31,44($sp)
	sw	$22,40($sp)
	sw	$21,36($sp)
	sw	$20,32($sp)
	sw	$19,28($sp)
	sw	$18,24($sp)
	sw	$17,20($sp)
	sw	$16,16($sp)
	lui	$17,%hi(kmalloc_caches)
	li	$22,34078720			# 0x2080000
	addiu	$17,$17,%lo(kmalloc_caches)
	move	$19,$4
	move	$18,$5
	lw	$4,28($17)
	addiu	$5,$22,32
	jal	kmem_cache_alloc
	lui	$20,%hi(table_napt)

	beq	$2,$0,$L630
	sw	$2,%lo(table_napt)($20)

	lui	$5,%hi(napt_list_inuse)
	move	$21,$2
	sll	$4,$19,1
	addiu	$2,$5,%lo(napt_list_inuse)
	sw	$0,%lo(napt_list_inuse)($5)
	lui	$3,%hi(napt_list_free)
	addu	$4,$4,$19
	addiu	$16,$3,%lo(napt_list_free)
	sw	$0,8($2)
	sw	$2,4($2)
	lui	$2,%hi(napt_table_list_max)
	sll	$4,$4,2
	sw	$0,%lo(napt_list_free)($3)
	ori	$5,$22,0x20
	sw	$0,8($16)
	sw	$16,4($16)
	jal	__kmalloc
	sw	$19,%lo(napt_table_list_max)($2)

	sw	$2,0($21)
	lw	$4,%lo(table_napt)($20)
	lw	$3,0($4)
	beq	$3,$0,$L627
	li	$2,-1			# 0xffffffffffffffff

	blez	$19,$L631
	move	$2,$0

	b	$L632
	move	$5,$0

$L643:
	lw	$3,0($4)
$L632:
	addu	$3,$3,$2
	addiu	$5,$5,1
	sw	$0,8($3)
	lw	$3,0($4)
	addu	$3,$3,$2
	sw	$0,0($3)
	lw	$4,%lo(table_napt)($20)
	lw	$3,0($4)
	addu	$3,$3,$2
	addiu	$2,$2,12
	bne	$19,$5,$L643
	sw	$3,4($3)

$L631:
	blez	$18,$L634
	move	$19,$0

	li	$20,34078720			# 0x2080000
	b	$L639
	addiu	$20,$20,32

$L644:
	lw	$3,4($16)
	sw	$0,44($2)
	sw	$3,48($2)
	sw	$2,0($3)
	lw	$2,8($16)
	sw	$4,4($16)
	addiu	$2,$2,1
	beq	$18,$19,$L634
	sw	$2,8($16)

$L639:
	lw	$4,28($17)
	move	$5,$20
	jal	kmem_cache_alloc
	addiu	$19,$19,1

	bne	$2,$0,$L644
	addiu	$4,$2,44

	lw	$31,44($sp)
	li	$2,-2			# 0xfffffffffffffffe
	lw	$22,40($sp)
	lw	$21,36($sp)
	lw	$20,32($sp)
	lw	$19,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	jr	$31
	addiu	$sp,$sp,48

$L634:
	move	$2,$0
$L627:
	lw	$31,44($sp)
	lw	$22,40($sp)
	lw	$21,36($sp)
	lw	$20,32($sp)
	lw	$19,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	jr	$31
	addiu	$sp,$sp,48

$L630:
	b	$L627
	li	$2,-1			# 0xffffffffffffffff

	.set	macro
	.set	reorder
	.end	init_table_napt
	.size	init_table_napt, .-init_table_napt
	.align	2
	.globl	init_table_path
	.set	nomips16
	.set	nomicromips
	.ent	init_table_path
	.type	init_table_path, @function
init_table_path:
	.frame	$sp,48,$31		# vars= 0, regs= 8/0, args= 16, gp= 0
	.mask	0x807f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-48
	sw	$31,44($sp)
	sw	$22,40($sp)
	sw	$21,36($sp)
	sw	$20,32($sp)
	sw	$19,28($sp)
	sw	$18,24($sp)
	sw	$17,20($sp)
	sw	$16,16($sp)
	lui	$17,%hi(kmalloc_caches)
	li	$22,34078720			# 0x2080000
	addiu	$17,$17,%lo(kmalloc_caches)
	move	$19,$4
	move	$18,$5
	lw	$4,28($17)
	addiu	$5,$22,32
	jal	kmem_cache_alloc
	lui	$20,%hi(table_path)

	beq	$2,$0,$L648
	sw	$2,%lo(table_path)($20)

	lui	$5,%hi(path_list_inuse)
	move	$21,$2
	sll	$4,$19,1
	addiu	$2,$5,%lo(path_list_inuse)
	sw	$0,%lo(path_list_inuse)($5)
	lui	$3,%hi(path_list_free)
	addu	$4,$4,$19
	addiu	$16,$3,%lo(path_list_free)
	sw	$0,8($2)
	sw	$2,4($2)
	lui	$2,%hi(path_table_list_max)
	sll	$4,$4,2
	sw	$0,%lo(path_list_free)($3)
	ori	$5,$22,0x20
	sw	$0,8($16)
	sw	$16,4($16)
	jal	__kmalloc
	sw	$19,%lo(path_table_list_max)($2)

	sw	$2,0($21)
	lw	$4,%lo(table_path)($20)
	lw	$3,0($4)
	beq	$3,$0,$L645
	li	$2,-1			# 0xffffffffffffffff

	blez	$19,$L649
	move	$2,$0

	b	$L650
	move	$5,$0

$L661:
	lw	$3,0($4)
$L650:
	addu	$3,$3,$2
	addiu	$5,$5,1
	sw	$0,8($3)
	lw	$3,0($4)
	addu	$3,$3,$2
	sw	$0,0($3)
	lw	$4,%lo(table_path)($20)
	lw	$3,0($4)
	addu	$3,$3,$2
	addiu	$2,$2,12
	bne	$19,$5,$L661
	sw	$3,4($3)

$L649:
	blez	$18,$L652
	move	$19,$0

	li	$20,34078720			# 0x2080000
	b	$L657
	addiu	$20,$20,32

$L662:
	lw	$3,4($16)
	sw	$0,88($2)
	sw	$3,92($2)
	sw	$2,0($3)
	lw	$2,8($16)
	sw	$4,4($16)
	addiu	$2,$2,1
	beq	$18,$19,$L652
	sw	$2,8($16)

$L657:
	lw	$4,28($17)
	move	$5,$20
	jal	kmem_cache_alloc
	addiu	$19,$19,1

	bne	$2,$0,$L662
	addiu	$4,$2,88

	lw	$31,44($sp)
	li	$2,-2			# 0xfffffffffffffffe
	lw	$22,40($sp)
	lw	$21,36($sp)
	lw	$20,32($sp)
	lw	$19,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	jr	$31
	addiu	$sp,$sp,48

$L652:
	move	$2,$0
$L645:
	lw	$31,44($sp)
	lw	$22,40($sp)
	lw	$21,36($sp)
	lw	$20,32($sp)
	lw	$19,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	jr	$31
	addiu	$sp,$sp,48

$L648:
	b	$L645
	li	$2,-1			# 0xffffffffffffffff

	.set	macro
	.set	reorder
	.end	init_table_path
	.size	init_table_path, .-init_table_path
	.section	.rodata.str1.4
	.align	2
$LC16:
	.ascii	"v1.03\000"
	.align	2
$LC17:
	.ascii	"Realtek FastPath\000"
	.align	2
$LC18:
	.ascii	"%s:%s\012\000"
	.text
	.align	2
	.globl	get_fastpath_module_info
	.set	nomips16
	.set	nomicromips
	.ent	get_fastpath_module_info
	.type	get_fastpath_module_info, @function
get_fastpath_module_info:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L665
	nop

	lui	$7,%hi($LC16)
	lui	$6,%hi($LC17)
	lui	$5,%hi($LC18)
	addiu	$7,$7,%lo($LC16)
	addiu	$6,$6,%lo($LC17)
	addiu	$5,$5,%lo($LC18)
	j	sprintf
	nop

$L665:
	jr	$31
	nop

	.set	macro
	.set	reorder
	.end	get_fastpath_module_info
	.size	get_fastpath_module_info, .-get_fastpath_module_info
	.globl	fastpath_path_proc_fops
	.data
	.align	2
	.type	fastpath_path_proc_fops, @object
	.size	fastpath_path_proc_fops, 108
fastpath_path_proc_fops:
	.space	4
	.word	seq_lseek
	.word	seq_read
	.word	fastpath_path_single_write
	.space	28
	.word	fastpath_path_single_open
	.space	4
	.word	single_release
	.space	52
	.globl	fastpath_napt_proc_fops
	.align	2
	.type	fastpath_napt_proc_fops, @object
	.size	fastpath_napt_proc_fops, 108
fastpath_napt_proc_fops:
	.space	4
	.word	seq_lseek
	.word	seq_read
	.word	fastpath_napt_single_write
	.space	28
	.word	fastpath_napt_single_open
	.space	4
	.word	single_release
	.space	52
	.globl	rtl_check_bypass_packet_hook
	.section	.bss,"aw",@nobits
	.align	2
	.type	rtl_check_bypass_packet_hook, @object
	.size	rtl_check_bypass_packet_hook, 4
rtl_check_bypass_packet_hook:
	.space	4
	.globl	rtl_init_bypass_packet_hook
	.align	2
	.type	rtl_init_bypass_packet_hook, @object
	.size	rtl_init_bypass_packet_hook, 4
rtl_init_bypass_packet_hook:
	.space	4
	.globl	fastpath_packet_bypassed_num
	.data
	.align	2
	.type	fastpath_packet_bypassed_num, @object
	.size	fastpath_packet_bypassed_num, 4
fastpath_packet_bypassed_num:
	.word	10
	.globl	negative_cache_table
	.section	.bss
	.align	2
	.type	negative_cache_table, @object
	.size	negative_cache_table, 4
negative_cache_table:
	.space	4
	.globl	negative_cache_list_free
	.align	2
	.type	negative_cache_list_free, @object
	.size	negative_cache_list_free, 12
negative_cache_list_free:
	.space	12
	.local	max_udp_frag_entry
	.comm	max_udp_frag_entry,4,4
	.globl	udp_cache_table
	.align	2
	.type	udp_cache_table, @object
	.size	udp_cache_table, 4
udp_cache_table:
	.space	4
	.globl	udp_cache_list_free
	.align	2
	.type	udp_cache_list_free, @object
	.size	udp_cache_list_free, 12
udp_cache_list_free:
	.space	12
	.globl	fp_napt
	.align	2
	.type	fp_napt, @object
	.size	fp_napt, 4
fp_napt:
	.space	4
	.globl	fp_route
	.align	2
	.type	fp_route, @object
	.size	fp_route, 4
fp_route:
	.space	4
	.globl	fp_path
	.align	2
	.type	fp_path, @object
	.size	fp_path, 4
fp_path:
	.space	4
	.globl	fp_arp
	.align	2
	.type	fp_arp, @object
	.size	fp_arp, 4
fp_arp:
	.space	4
	.local	path_table_list_max
	.comm	path_table_list_max,4,4
	.globl	table_path
	.align	2
	.type	table_path, @object
	.size	table_path, 4
table_path:
	.space	4
	.globl	path_list_free
	.align	2
	.type	path_list_free, @object
	.size	path_list_free, 12
path_list_free:
	.space	12
	.globl	path_list_inuse
	.align	2
	.type	path_list_inuse, @object
	.size	path_list_inuse, 12
path_list_inuse:
	.space	12
	.local	napt_table_list_max
	.comm	napt_table_list_max,4,4
	.globl	table_napt
	.align	2
	.type	table_napt, @object
	.size	table_napt, 4
table_napt:
	.space	4
	.globl	napt_list_free
	.align	2
	.type	napt_list_free, @object
	.size	napt_list_free, 12
napt_list_free:
	.space	12
	.globl	napt_list_inuse
	.align	2
	.type	napt_list_inuse, @object
	.size	napt_list_inuse, 12
napt_list_inuse:
	.space	12
	.globl	route_list_free
	.align	2
	.type	route_list_free, @object
	.size	route_list_free, 12
route_list_free:
	.space	12
	.globl	route_list_inuse
	.align	2
	.type	route_list_inuse, @object
	.size	route_list_inuse, 12
route_list_inuse:
	.space	12
	.ident	"GCC: (Realtek MSDK-6.4.1 Build 3055) 6.4.1 20180425"
