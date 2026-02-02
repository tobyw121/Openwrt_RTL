	.file	1 "fast_ipv6_core.c"
	.section .mdebug.abi32
	.previous
	.nan	legacy
	.module	softfloat
	.module	oddspreg
	.text
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	V6_cache_timeout
	.type	V6_cache_timeout, @function
V6_cache_timeout:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	lbu	$2,48($4)
	beq	$2,$0,$L9
	nop

	lw	$2,4($4)
	lw	$10,8($4)
	lw	$14,12($4)
	lw	$12,16($4)
	srl	$15,$2,8
	xor	$3,$2,$10
	lw	$11,20($4)
	lw	$9,24($4)
	xor	$3,$3,$14
	lw	$8,28($4)
	xor	$3,$3,$12
	lw	$7,32($4)
	xor	$3,$3,$11
	lui	$2,%hi(V6_max_udp_frag_entry)
	lhu	$6,36($4)
	xor	$3,$3,$9
	lw	$13,0($4)
	xor	$3,$3,$8
	sb	$0,48($4)
	xor	$3,$3,$7
	lw	$5,%lo(V6_max_udp_frag_entry)($2)
	srl	$2,$10,8
	lw	$10,80($4)
	xor	$3,$3,$15
	srl	$14,$14,8
	xor	$3,$3,$2
	srl	$2,$12,8
	xor	$3,$3,$14
	srl	$11,$11,16
	xor	$2,$3,$2
	srl	$9,$9,16
	xor	$2,$2,$11
	srl	$8,$8,16
	srl	$7,$7,16
	xor	$2,$2,$9
	xor	$2,$2,$8
	xor	$2,$2,$7
	srl	$7,$2,16
	xor	$3,$6,$13
	xor	$3,$3,$2
	addiu	$2,$5,-1
	xor	$3,$7,$3
	beq	$10,$0,$L3
	and	$2,$2,$3

	sll	$3,$2,1
	lw	$6,84($4)
	lui	$5,%hi(V6_udp_cache_table)
	addu	$2,$3,$2
	sw	$6,84($10)
	sll	$3,$2,2
$L4:
	lw	$6,84($4)
	lui	$2,%hi(V6_udp_cache_list_free)
	lw	$8,80($4)
	addiu	$7,$4,88
	addiu	$2,$2,%lo(V6_udp_cache_list_free)
	sw	$8,0($6)
	lw	$5,%lo(V6_udp_cache_table)($5)
	lw	$6,4($2)
	lw	$5,0($5)
	addu	$3,$5,$3
	lw	$5,8($3)
	addiu	$5,$5,-1
	sw	$5,8($3)
	sw	$0,88($4)
	sw	$6,92($4)
	sw	$4,0($6)
	lw	$3,8($2)
	sw	$7,4($2)
	addiu	$3,$3,1
	sw	$3,8($2)
$L9:
	jr	$31
	nop

$L3:
	lui	$5,%hi(V6_udp_cache_table)
	lw	$6,84($4)
	sll	$3,$2,1
	lw	$7,%lo(V6_udp_cache_table)($5)
	addu	$2,$3,$2
	sll	$3,$2,2
	lw	$2,0($7)
	addu	$2,$2,$3
	b	$L4
	sw	$6,4($2)

	.set	macro
	.set	reorder
	.end	V6_cache_timeout
	.size	V6_cache_timeout, .-V6_cache_timeout
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	ipv6_fast_forward_table_single_write
	.type	ipv6_fast_forward_table_single_write, @function
ipv6_fast_forward_table_single_write:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	jr	$31
	move	$2,$6

	.set	macro
	.set	reorder
	.end	ipv6_fast_forward_table_single_write
	.size	ipv6_fast_forward_table_single_write, .-ipv6_fast_forward_table_single_write
	.align	2
	.globl	ipv6_fast_forward_single_open
	.set	nomips16
	.set	nomicromips
	.ent	ipv6_fast_forward_single_open
	.type	ipv6_fast_forward_single_open, @function
ipv6_fast_forward_single_open:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	move	$4,$5
	lui	$5,%hi(ipv6_forward_read_proc)
	move	$6,$0
	addiu	$5,$5,%lo(ipv6_forward_read_proc)
	j	single_open
	nop

	.set	macro
	.set	reorder
	.end	ipv6_fast_forward_single_open
	.size	ipv6_fast_forward_single_open, .-ipv6_fast_forward_single_open
	.align	2
	.globl	ipv6_fast_forward_table_single_open
	.set	nomips16
	.set	nomicromips
	.ent	ipv6_fast_forward_table_single_open
	.type	ipv6_fast_forward_table_single_open, @function
ipv6_fast_forward_table_single_open:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	move	$4,$5
	lui	$5,%hi(ipv6_forward_table_read_proc)
	move	$6,$0
	addiu	$5,$5,%lo(ipv6_forward_table_read_proc)
	j	single_open
	nop

	.set	macro
	.set	reorder
	.end	ipv6_fast_forward_table_single_open
	.size	ipv6_fast_forward_table_single_open, .-ipv6_fast_forward_table_single_open
	.section	.rodata.str1.4,"aMS",@progbits,1
	.align	2
$LC0:
	.ascii	"%c\012\000"
	.text
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	ipv6_forward_read_proc
	.type	ipv6_forward_read_proc, @function
ipv6_forward_read_proc:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	lui	$2,%hi(fast_ipv6_fw)
	lui	$5,%hi($LC0)
	lw	$6,%lo(fast_ipv6_fw)($2)
	addiu	$5,$5,%lo($LC0)
	jal	seq_printf
	addiu	$6,$6,48

	move	$2,$0
	lw	$31,20($sp)
	jr	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	ipv6_forward_read_proc
	.size	ipv6_forward_read_proc, .-ipv6_forward_read_proc
	.section	.rodata.str1.4
	.align	2
$LC1:
	.ascii	"V6 Path --- [protocol %d] in-S=%x:%x:%x:%x:%x:%x:%x:%x ("
	.ascii	"%-5u)  out-D=%x:%x:%x:%x:%x:%x:%x:%x (%-5u), [Mark %d]\012"
	.ascii	"\000"
	.text
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	ipv6_forward_table_read_proc
	.type	ipv6_forward_table_read_proc, @function
ipv6_forward_table_read_proc:
	.frame	$sp,104,$31		# vars= 0, regs= 4/0, args= 88, gp= 0
	.mask	0x80070000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-104
	sw	$31,100($sp)
	sw	$18,96($sp)
	sw	$17,92($sp)
	sw	$16,88($sp)
	lui	$2,%hi(v6_path_list_inuse)
	lw	$16,%lo(v6_path_list_inuse)($2)
	beq	$16,$0,$L20
	lui	$18,%hi($LC1)

	move	$17,$4
	addiu	$18,$18,%lo($LC1)
$L17:
	lw	$3,40($16)
	move	$5,$18
	lw	$8,52($16)
	move	$4,$17
	lw	$2,36($16)
	wsbh	$3,$3
	lw	$6,32($16)
	lhu	$7,0($16)
	andi	$3,$3,0xffff
	sw	$8,84($sp)
	sw	$3,80($sp)
	wsbh	$2,$2
	lhu	$3,30($16)
	andi	$2,$2,0xffff
	sw	$3,76($sp)
	lhu	$3,28($16)
	sw	$3,72($sp)
	lhu	$3,26($16)
	sw	$3,68($sp)
	lhu	$3,24($16)
	sw	$3,64($sp)
	lhu	$3,22($16)
	sw	$3,60($sp)
	lhu	$3,20($16)
	sw	$3,56($sp)
	lhu	$3,18($16)
	sw	$3,52($sp)
	lhu	$3,16($16)
	sw	$2,44($sp)
	sw	$3,48($sp)
	lhu	$2,14($16)
	sw	$2,40($sp)
	lhu	$2,12($16)
	sw	$2,36($sp)
	lhu	$2,10($16)
	sw	$2,32($sp)
	lhu	$2,8($16)
	sw	$2,28($sp)
	lhu	$2,6($16)
	sw	$2,24($sp)
	lhu	$2,4($16)
	sw	$2,20($sp)
	lhu	$2,2($16)
	jal	seq_printf
	sw	$2,16($sp)

	lw	$16,64($16)
	bne	$16,$0,$L17
	nop

$L20:
	lw	$31,100($sp)
	move	$2,$0
	lw	$18,96($sp)
	lw	$17,92($sp)
	lw	$16,88($sp)
	jr	$31
	addiu	$sp,$sp,104

	.set	macro
	.set	reorder
	.end	ipv6_forward_table_read_proc
	.size	ipv6_forward_table_read_proc, .-ipv6_forward_table_read_proc
	.section	.rodata.str1.4
	.align	2
$LC2:
	.ascii	"ipv6_fast_forward\000"
	.section	.init.text,"ax",@progbits
	.align	2
	.globl	ipv6_fast_forward_init
	.set	nomips16
	.set	nomicromips
	.ent	ipv6_fast_forward_init
	.type	ipv6_fast_forward_init, @function
ipv6_fast_forward_init:
	.frame	$sp,32,$31		# vars= 0, regs= 1/0, args= 24, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	lui	$7,%hi(ipv6_fast_forward_proc_fops)
	lui	$6,%hi(proc_root)
	sw	$0,16($sp)
	lui	$4,%hi($LC2)
	addiu	$7,$7,%lo(ipv6_fast_forward_proc_fops)
	addiu	$6,$6,%lo(proc_root)
	move	$5,$0
	jal	proc_create_data
	addiu	$4,$4,%lo($LC2)

	lui	$3,%hi(ipv6_res)
	lw	$31,28($sp)
	sw	$2,%lo(ipv6_res)($3)
	move	$2,$0
	jr	$31
	addiu	$sp,$sp,32

	.set	macro
	.set	reorder
	.end	ipv6_fast_forward_init
	.size	ipv6_fast_forward_init, .-ipv6_fast_forward_init
	.section	.exit.text,"ax",@progbits
	.align	2
	.globl	ipv6_fast_forward_exit
	.set	nomips16
	.set	nomicromips
	.ent	ipv6_fast_forward_exit
	.type	ipv6_fast_forward_exit, @function
ipv6_fast_forward_exit:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	lui	$2,%hi(ipv6_res)
	lw	$2,%lo(ipv6_res)($2)
	beq	$2,$0,$L30
	lui	$2,%hi(fast_ipv6_fw)

	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	lui	$5,%hi(proc_root)
	lui	$4,%hi($LC2)
	addiu	$5,$5,%lo(proc_root)
	jal	remove_proc_entry
	addiu	$4,$4,%lo($LC2)

	lui	$2,%hi(fast_ipv6_fw)
	lw	$31,20($sp)
	sw	$0,%lo(fast_ipv6_fw)($2)
	jr	$31
	addiu	$sp,$sp,24

$L30:
	jr	$31
	sw	$0,%lo(fast_ipv6_fw)($2)

	.set	macro
	.set	reorder
	.end	ipv6_fast_forward_exit
	.size	ipv6_fast_forward_exit, .-ipv6_fast_forward_exit
	.section	.rodata.str1.4
	.align	2
$LC3:
	.ascii	"ipv6_fast_forward_table\000"
	.section	.init.text
	.align	2
	.globl	ipv6_fast_forward_table_init
	.set	nomips16
	.set	nomicromips
	.ent	ipv6_fast_forward_table_init
	.type	ipv6_fast_forward_table_init, @function
ipv6_fast_forward_table_init:
	.frame	$sp,32,$31		# vars= 0, regs= 1/0, args= 24, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	lui	$7,%hi(ipv6_fast_forward_table_proc_fops)
	lui	$6,%hi(proc_root)
	sw	$0,16($sp)
	lui	$4,%hi($LC3)
	addiu	$7,$7,%lo(ipv6_fast_forward_table_proc_fops)
	addiu	$6,$6,%lo(proc_root)
	move	$5,$0
	jal	proc_create_data
	addiu	$4,$4,%lo($LC3)

	lui	$3,%hi(ipv6_res_table)
	lw	$31,28($sp)
	sw	$2,%lo(ipv6_res_table)($3)
	move	$2,$0
	jr	$31
	addiu	$sp,$sp,32

	.set	macro
	.set	reorder
	.end	ipv6_fast_forward_table_init
	.size	ipv6_fast_forward_table_init, .-ipv6_fast_forward_table_init
	.text
	.align	2
	.globl	rtk_addV6Connection
	.set	nomips16
	.set	nomicromips
	.ent	rtk_addV6Connection
	.type	rtk_addV6Connection, @function
rtk_addV6Connection:
	.frame	$sp,112,$31		# vars= 56, regs= 10/0, args= 16, gp= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-112
	sw	$31,108($sp)
	sw	$fp,104($sp)
	sw	$23,100($sp)
	sw	$22,96($sp)
	sw	$21,92($sp)
	sw	$20,88($sp)
	sw	$19,84($sp)
	sw	$18,80($sp)
	sw	$17,76($sp)
	sw	$16,72($sp)
	move	$16,$5
	move	$21,$4
	move	$4,$5
	jal	rtl_store_skb_ipv6_dst
	move	$20,$6

	jal	rtl_get_skb_dev
	move	$4,$16

	move	$4,$16
	jal	rtl_ip6_route_input
	sw	$2,48($sp)

	li	$4,-1			# 0xffffffffffffffff
	beq	$2,$4,$L88
	lw	$31,108($sp)

	jal	rtl_get_skb_ipv6_dst
	move	$4,$16

	beq	$2,$0,$L78
	move	$4,$16

	jal	rtl_get_skb_ipv6_dst_dev
	nop

	move	$4,$16
	jal	rtl_dst_release
	sw	$2,68($sp)

	jal	rtl_set_skb_ipv6_dst
	move	$4,$16

$L39:
	lw	$2,0($21)
	lui	$22,%hi(v6_path_table_list_max)
	lw	$5,40($21)
	lui	$19,%hi(v6_table_path)
	lw	$13,4($21)
	lw	$4,8($21)
	srl	$14,$2,8
	lw	$12,12($21)
	xor	$5,$2,$5
	lw	$11,16($21)
	xor	$5,$5,$13
	lw	$10,20($21)
	xor	$5,$5,$4
	lw	$9,24($21)
	xor	$5,$5,$12
	lw	$8,28($21)
	xor	$5,$5,$11
	xor	$5,$5,$10
	lw	$2,36($21)
	xor	$5,$5,$9
	lhu	$7,32($21)
	xor	$5,$5,$8
	lw	$6,%lo(v6_path_table_list_max)($22)
	srl	$13,$13,8
	lw	$15,%lo(v6_table_path)($19)
	xor	$5,$5,$14
	srl	$4,$4,8
	xor	$5,$5,$13
	srl	$12,$12,8
	lw	$13,0($15)
	xor	$4,$5,$4
	srl	$11,$11,16
	xor	$4,$4,$12
	srl	$10,$10,16
	xor	$4,$4,$11
	srl	$9,$9,16
	xor	$4,$4,$10
	srl	$8,$8,16
	xor	$4,$4,$9
	srl	$5,$2,4
	xor	$4,$4,$8
	xor	$4,$4,$5
	xor	$4,$7,$4
	srl	$7,$4,12
	addiu	$6,$6,-1
	xor	$4,$7,$4
	and	$6,$6,$4
	sll	$18,$6,1
	addu	$6,$18,$6
	sll	$18,$6,2
	addu	$13,$13,$18
	lw	$23,0($13)
	beq	$23,$0,$L40
	addiu	$17,$21,16

	b	$L89
	lw	$4,36($23)

$L46:
	lw	$23,56($23)
	beq	$23,$0,$L40
	nop

	lw	$2,36($21)
	lw	$4,36($23)
$L89:
	bne	$4,$2,$L46
	nop

	lw	$4,40($23)
	lw	$2,40($21)
	bne	$4,$2,$L46
	li	$6,16			# 0x10

	move	$4,$23
	jal	memcmp
	move	$5,$21

	bne	$2,$0,$L46
	addiu	$4,$23,16

	li	$6,16			# 0x10
	jal	memcmp
	move	$5,$17

	bne	$2,$0,$L46
	nop

	lw	$6,32($23)
	lw	$2,32($21)
	bne	$6,$2,$L46
	lw	$12,%lo(v6_table_path)($19)

	b	$L90
	lw	$2,16($21)

$L40:
	lui	$17,%hi(v6_path_list_free)
	lw	$fp,%lo(v6_path_list_free)($17)
	beq	$fp,$0,$L79
	nop

	jal	rtl_get_skb_protocol
	move	$4,$16

	move	$4,$16
	wsbh	$2,$2
	andi	$6,$2,0xffff
	jal	rtl_ipv6_hdr
	sw	$6,64($sp)

	move	$4,$16
	jal	rtl_get_skb_mark
	move	$23,$2

	sw	$2,52($sp)
	move	$5,$0
	lhu	$3,40($23)
	move	$4,$16
	lw	$10,28($23)
	lw	$7,32($23)
	lw	$2,36($23)
	lw	$15,8($23)
	lw	$14,12($23)
	lw	$13,16($23)
	lw	$12,20($23)
	lw	$11,24($23)
	sw	$3,56($sp)
	sw	$10,20($sp)
	sw	$7,24($sp)
	sw	$2,28($sp)
	sw	$15,32($sp)
	sw	$14,36($sp)
	sw	$13,40($sp)
	sw	$12,44($sp)
	sw	$11,16($sp)
	lhu	$3,42($23)
	jal	rtl_set_skb_mark
	sw	$3,60($sp)

	li	$2,34525			# 0x86dd
	lw	$10,0($21)
	lw	$7,4($21)
	lw	$5,8($21)
	lw	$4,12($21)
	sw	$10,8($23)
	sw	$7,12($23)
	sw	$5,16($23)
	sw	$4,20($23)
	lw	$4,36($21)
	lw	$6,64($sp)
	sh	$4,40($23)
	lw	$4,28($21)
	lw	$10,16($21)
	lw	$7,20($21)
	lw	$5,24($21)
	sw	$10,24($23)
	sw	$7,28($23)
	sw	$5,32($23)
	sw	$4,36($23)
	lw	$4,40($21)
	beq	$6,$2,$L80
	sh	$4,42($23)

$L48:
	lw	$5,52($sp)
	jal	rtl_set_skb_mark
	move	$4,$16

	lw	$2,32($sp)
	sw	$2,8($23)
	lw	$2,36($sp)
	sw	$2,12($23)
	lw	$2,40($sp)
	sw	$2,16($23)
	lw	$2,44($sp)
	sw	$2,20($23)
	lhu	$2,56($sp)
	sh	$2,40($23)
	lw	$2,16($sp)
	sw	$2,24($23)
	lw	$2,20($sp)
	sw	$2,28($23)
	lw	$2,24($sp)
	sw	$2,32($23)
	lw	$2,28($sp)
	sw	$2,36($23)
	lhu	$2,60($sp)
	sh	$2,42($23)
	lw	$6,0($21)
	lw	$5,4($21)
	lw	$4,8($21)
	lw	$2,12($21)
	sw	$6,0($fp)
	sw	$5,4($fp)
	sw	$4,8($fp)
	sw	$2,12($fp)
	lw	$2,28($21)
	lw	$5,20($21)
	lw	$4,24($21)
	lw	$6,16($21)
	sw	$6,16($fp)
	sw	$5,20($fp)
	sw	$2,28($fp)
	lui	$2,%hi(jiffies)
	sw	$4,24($fp)
	lw	$4,32($21)
	lw	$5,%lo(jiffies)($2)
	lw	$2,64($fp)
	sw	$4,32($fp)
	lw	$4,36($21)
	sw	$4,36($fp)
	lw	$4,40($21)
	sw	$5,44($fp)
	sw	$20,48($fp)
	sw	$4,40($fp)
	beq	$2,$0,$L51
	lw	$4,68($fp)

	sw	$4,68($2)
	lui	$17,%hi(v6_path_list_free)
	lw	$4,68($fp)
	lw	$2,64($fp)
	addiu	$17,$17,%lo(v6_path_list_free)
$L52:
	sw	$2,0($4)
	lui	$2,%hi(v6_path_list_inuse)
	lw	$4,8($17)
	addiu	$7,$fp,64
	addiu	$2,$2,%lo(v6_path_list_inuse)
	sw	$0,64($fp)
	addiu	$6,$fp,56
	addiu	$4,$4,-1
	lw	$5,4($2)
	sw	$4,8($17)
	sw	$5,68($fp)
	sw	$fp,0($5)
	lw	$4,8($2)
	lw	$5,%lo(v6_table_path)($19)
	sw	$0,56($fp)
	addiu	$4,$4,1
	sw	$7,4($2)
	sw	$4,8($2)
	lw	$2,0($5)
	addu	$2,$2,$18
	lw	$2,4($2)
	sw	$2,60($fp)
	lw	$2,0($5)
	addu	$2,$2,$18
	lw	$2,4($2)
	sw	$fp,0($2)
	lw	$12,%lo(v6_table_path)($19)
	lw	$2,0($12)
	addu	$2,$2,$18
	sw	$6,4($2)
	lw	$2,0($12)
	addu	$18,$2,$18
	lw	$2,8($18)
	addiu	$2,$2,1
	sw	$2,8($18)
	lw	$6,32($21)
$L47:
	lw	$2,16($21)
$L90:
	andi	$6,$6,0xffff
	lw	$5,36($21)
	lw	$14,20($21)
	lw	$4,24($21)
	srl	$15,$2,8
	lw	$13,28($21)
	xor	$5,$2,$5
	lw	$11,0($21)
	xor	$5,$5,$14
	lw	$10,4($21)
	xor	$5,$5,$4
	lw	$9,8($21)
	xor	$5,$5,$13
	lw	$8,12($21)
	xor	$5,$5,$11
	xor	$5,$5,$10
	lw	$2,40($21)
	xor	$5,$5,$9
	lw	$7,%lo(v6_path_table_list_max)($22)
	xor	$5,$5,$8
	lw	$12,0($12)
	srl	$14,$14,8
	xor	$5,$5,$15
	srl	$4,$4,8
	xor	$5,$5,$14
	srl	$13,$13,8
	xor	$4,$5,$4
	srl	$11,$11,16
	xor	$4,$4,$13
	srl	$10,$10,16
	xor	$4,$4,$11
	srl	$9,$9,16
	xor	$4,$4,$10
	srl	$8,$8,16
	xor	$4,$4,$9
	srl	$5,$2,4
	xor	$4,$4,$8
	xor	$4,$4,$5
	xor	$4,$6,$4
	srl	$6,$4,12
	addiu	$7,$7,-1
	xor	$4,$6,$4
	and	$7,$7,$4
	sll	$18,$7,1
	addu	$7,$18,$7
	sll	$18,$7,2
	addu	$12,$12,$18
	lw	$17,0($12)
	beq	$17,$0,$L53
	addiu	$22,$21,16

	b	$L91
	lw	$4,36($17)

$L59:
	lw	$17,56($17)
	beq	$17,$0,$L53
	nop

	lw	$2,40($21)
	lw	$4,36($17)
$L91:
	bne	$4,$2,$L59
	nop

	lw	$4,40($17)
	lw	$2,36($21)
	bne	$4,$2,$L59
	li	$6,16			# 0x10

	move	$4,$17
	jal	memcmp
	move	$5,$22

	bne	$2,$0,$L59
	addiu	$4,$17,16

	li	$6,16			# 0x10
	jal	memcmp
	move	$5,$21

	bne	$2,$0,$L59
	nop

	lw	$4,32($17)
	lw	$2,32($21)
	bne	$4,$2,$L59
	lw	$31,108($sp)

$L88:
	move	$2,$0
	lw	$fp,104($sp)
	lw	$23,100($sp)
	lw	$22,96($sp)
	lw	$21,92($sp)
	lw	$20,88($sp)
	lw	$19,84($sp)
	lw	$18,80($sp)
	lw	$17,76($sp)
	lw	$16,72($sp)
	jr	$31
	addiu	$sp,$sp,112

$L53:
	lui	$17,%hi(v6_path_list_free)
	lw	$fp,%lo(v6_path_list_free)($17)
	beq	$fp,$0,$L88
	lw	$31,108($sp)

	jal	rtl_get_skb_protocol
	move	$4,$16

	move	$4,$16
	wsbh	$2,$2
	andi	$6,$2,0xffff
	jal	rtl_ipv6_hdr
	sw	$6,60($sp)

	move	$4,$16
	jal	rtl_get_skb_mark
	move	$22,$2

	move	$23,$2
	lhu	$3,40($22)
	move	$5,$0
	lw	$10,28($22)
	move	$4,$16
	lw	$7,32($22)
	lw	$2,36($22)
	lw	$15,8($22)
	lw	$14,12($22)
	lw	$13,16($22)
	lw	$12,20($22)
	lw	$11,24($22)
	sw	$3,52($sp)
	sw	$10,20($sp)
	sw	$7,24($sp)
	sw	$2,28($sp)
	sw	$15,32($sp)
	sw	$14,36($sp)
	sw	$13,40($sp)
	sw	$12,44($sp)
	sw	$11,16($sp)
	lhu	$3,42($22)
	jal	rtl_set_skb_mark
	sw	$3,56($sp)

	li	$2,34525			# 0x86dd
	lw	$10,16($21)
	lw	$7,20($21)
	lw	$5,24($21)
	lw	$4,28($21)
	sw	$10,8($22)
	sw	$7,12($22)
	sw	$5,16($22)
	sw	$4,20($22)
	lw	$4,40($21)
	lw	$6,60($sp)
	sh	$4,40($22)
	lw	$4,12($21)
	lw	$10,0($21)
	lw	$7,4($21)
	lw	$5,8($21)
	sw	$10,24($22)
	sw	$7,28($22)
	sw	$5,32($22)
	sw	$4,36($22)
	lw	$4,36($21)
	beq	$6,$2,$L81
	sh	$4,42($22)

$L60:
	move	$5,$23
	jal	rtl_set_skb_mark
	move	$4,$16

	lw	$2,32($sp)
	sw	$2,8($22)
	lw	$2,36($sp)
	sw	$2,12($22)
	lw	$2,40($sp)
	sw	$2,16($22)
	lw	$2,44($sp)
	sw	$2,20($22)
	lhu	$2,52($sp)
	sh	$2,40($22)
	lw	$2,16($sp)
	sw	$2,24($22)
	lw	$2,20($sp)
	sw	$2,28($22)
	lw	$2,24($sp)
	sw	$2,32($22)
	lw	$2,28($sp)
	sw	$2,36($22)
	lhu	$2,56($sp)
	sh	$2,42($22)
	lw	$6,16($21)
	lw	$5,20($21)
	lw	$4,24($21)
	lw	$2,28($21)
	sw	$6,0($fp)
	sw	$5,4($fp)
	sw	$4,8($fp)
	sw	$2,12($fp)
	lw	$2,12($21)
	lw	$5,4($21)
	lw	$4,8($21)
	lw	$6,0($21)
	sw	$6,16($fp)
	sw	$4,24($fp)
	sw	$2,28($fp)
	lui	$2,%hi(jiffies)
	sw	$5,20($fp)
	lw	$5,32($21)
	lw	$4,%lo(jiffies)($2)
	lw	$2,64($fp)
	sw	$5,32($fp)
	lw	$5,40($21)
	sw	$5,36($fp)
	lw	$3,36($21)
	sw	$4,44($fp)
	sw	$20,48($fp)
	sw	$3,40($fp)
	beq	$2,$0,$L63
	lw	$3,68($fp)

	sw	$3,68($2)
	lui	$17,%hi(v6_path_list_free)
	lw	$3,68($fp)
	lw	$2,64($fp)
	addiu	$17,$17,%lo(v6_path_list_free)
$L64:
	sw	$2,0($3)
	lui	$3,%hi(v6_path_list_inuse)
	lw	$4,8($17)
	addiu	$7,$fp,64
	addiu	$3,$3,%lo(v6_path_list_inuse)
	sw	$0,64($fp)
	addiu	$6,$fp,56
	addiu	$4,$4,-1
	lw	$5,4($3)
	li	$2,1			# 0x1
	sw	$4,8($17)
	sw	$5,68($fp)
	sw	$fp,0($5)
	lw	$4,8($3)
	lw	$5,%lo(v6_table_path)($19)
	sw	$0,56($fp)
	addiu	$4,$4,1
	sw	$7,4($3)
	sw	$4,8($3)
	lw	$3,0($5)
	addu	$3,$3,$18
	lw	$3,4($3)
	sw	$3,60($fp)
	lw	$3,0($5)
	addu	$3,$3,$18
	lw	$3,4($3)
	sw	$fp,0($3)
	lw	$4,%lo(v6_table_path)($19)
	lw	$3,0($4)
	addu	$3,$3,$18
	sw	$6,4($3)
	lw	$3,0($4)
	addu	$18,$3,$18
	lw	$3,8($18)
	addiu	$3,$3,1
	sw	$3,8($18)
	lw	$31,108($sp)
	lw	$fp,104($sp)
	lw	$23,100($sp)
	lw	$22,96($sp)
	lw	$21,92($sp)
	lw	$20,88($sp)
	lw	$19,84($sp)
	lw	$18,80($sp)
	lw	$17,76($sp)
	lw	$16,72($sp)
	jr	$31
	addiu	$sp,$sp,112

$L79:
	lw	$6,32($21)
	b	$L47
	lw	$12,%lo(v6_table_path)($19)

$L78:
	jal	rtl_set_skb_ipv6_dst
	nop

	b	$L39
	sw	$0,68($sp)

$L63:
	addiu	$17,$17,%lo(v6_path_list_free)
	b	$L64
	sw	$3,4($17)

$L81:
	move	$6,$0
	li	$5,10			# 0xa
	jal	rtl_hook_is_empty
	move	$4,$16

	bne	$2,$0,$L82
	lw	$6,68($sp)

	li	$6,4			# 0x4
$L86:
	li	$5,10			# 0xa
	jal	rtl_hook_is_empty
	move	$4,$16

	beq	$2,$0,$L83
	lw	$7,48($sp)

$L62:
	jal	rtl_get_skb_mark
	move	$4,$16

	b	$L60
	sw	$2,52($fp)

$L51:
	addiu	$17,$17,%lo(v6_path_list_free)
	b	$L52
	sw	$4,4($17)

$L80:
	move	$6,$0
	li	$5,10			# 0xa
	jal	rtl_hook_is_empty
	move	$4,$16

	beq	$2,$0,$L84
	lw	$6,48($sp)

	li	$6,4			# 0x4
$L87:
	li	$5,10			# 0xa
	jal	rtl_hook_is_empty
	move	$4,$16

	beq	$2,$0,$L85
	lw	$7,68($sp)

$L50:
	jal	rtl_get_skb_mark
	move	$4,$16

	b	$L48
	sw	$2,52($fp)

$L83:
	move	$6,$0
	li	$5,4			# 0x4
	jal	rtl_ipt6_do_table
	move	$4,$16

	b	$L62
	nop

$L82:
	move	$7,$0
	move	$5,$0
	jal	rtl_ipt6_do_table
	move	$4,$16

	b	$L86
	li	$6,4			# 0x4

$L85:
	move	$6,$0
	li	$5,4			# 0x4
	jal	rtl_ipt6_do_table
	move	$4,$16

	b	$L50
	nop

$L84:
	move	$7,$0
	move	$5,$0
	jal	rtl_ipt6_do_table
	move	$4,$16

	b	$L87
	li	$6,4			# 0x4

	.set	macro
	.set	reorder
	.end	rtk_addV6Connection
	.size	rtk_addV6Connection, .-rtk_addV6Connection
	.align	2
	.globl	rtk_delV6Connection
	.set	nomips16
	.set	nomicromips
	.ent	rtk_delV6Connection
	.type	rtk_delV6Connection, @function
rtk_delV6Connection:
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
	lui	$17,%hi(v6_table_path)
	lw	$2,0($4)
	lui	$5,%hi(v6_path_table_list_max)
	lw	$3,40($4)
	lw	$13,4($4)
	lw	$12,8($4)
	srl	$15,$2,8
	lw	$11,12($4)
	xor	$3,$2,$3
	xor	$3,$3,$13
	lw	$2,16($4)
	lw	$9,20($4)
	xor	$3,$3,$12
	lw	$8,24($4)
	xor	$3,$3,$11
	lw	$7,28($4)
	xor	$3,$3,$2
	xor	$3,$3,$9
	lw	$10,%lo(v6_table_path)($17)
	xor	$3,$3,$8
	lw	$6,36($4)
	xor	$3,$3,$7
	lhu	$14,32($4)
	srl	$13,$13,8
	lw	$5,%lo(v6_path_table_list_max)($5)
	xor	$3,$3,$15
	srl	$12,$12,8
	xor	$3,$3,$13
	lw	$13,0($10)
	srl	$11,$11,8
	xor	$3,$3,$12
	srl	$10,$2,16
	xor	$3,$3,$11
	srl	$9,$9,16
	xor	$2,$3,$10
	srl	$8,$8,16
	srl	$7,$7,16
	xor	$2,$2,$9
	srl	$3,$6,4
	xor	$2,$2,$8
	xor	$2,$2,$7
	xor	$2,$2,$3
	xor	$3,$14,$2
	srl	$7,$3,12
	addiu	$2,$5,-1
	xor	$3,$7,$3
	and	$2,$2,$3
	sll	$19,$2,1
	addu	$19,$19,$2
	sll	$19,$19,2
	addu	$2,$13,$19
	lw	$16,0($2)
	beq	$16,$0,$L93
	lui	$20,%hi(v6_path_list_free)

	lui	$21,%hi(v6_path_list_inuse)
	move	$18,$4
	addiu	$22,$4,16
	addiu	$20,$20,%lo(v6_path_list_free)
	b	$L94
	addiu	$21,$21,%lo(v6_path_list_inuse)

$L96:
	lw	$16,56($16)
	beq	$16,$0,$L110
	lw	$31,44($sp)

	lw	$6,36($18)
$L94:
	lw	$2,36($16)
	bne	$2,$6,$L96
	nop

	lw	$3,40($16)
	lw	$2,40($18)
	bne	$3,$2,$L96
	li	$6,16			# 0x10

	move	$5,$18
	jal	memcmp
	move	$4,$16

	bne	$2,$0,$L96
	li	$6,16			# 0x10

	addiu	$4,$16,16
	jal	memcmp
	move	$5,$22

	bne	$2,$0,$L96
	nop

	lw	$3,32($16)
	lw	$2,32($18)
	bne	$3,$2,$L96
	nop

	lw	$2,56($16)
	beq	$2,$0,$L98
	nop

	lw	$3,60($16)
	sw	$3,60($2)
$L99:
	lw	$2,60($16)
	lw	$3,56($16)
	sw	$3,0($2)
	lw	$2,%lo(v6_table_path)($17)
	lw	$2,0($2)
	addu	$2,$2,$19
	lw	$3,8($2)
	addiu	$3,$3,-1
	sw	$3,8($2)
	lw	$2,64($16)
	beq	$2,$0,$L100
	lw	$3,68($16)

	sw	$3,68($2)
	lw	$3,68($16)
	lw	$2,64($16)
$L101:
	sw	$2,0($3)
	addiu	$3,$16,64
	lw	$2,8($21)
	lw	$4,4($20)
	sw	$0,64($16)
	addiu	$2,$2,-1
	sw	$4,68($16)
	sw	$2,8($21)
	sw	$16,0($4)
	lw	$2,8($20)
	sw	$3,4($20)
	addiu	$2,$2,1
	b	$L96
	sw	$2,8($20)

$L93:
	lw	$31,44($sp)
$L110:
	move	$2,$0
	lw	$22,40($sp)
	lw	$21,36($sp)
	lw	$20,32($sp)
	lw	$19,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	jr	$31
	addiu	$sp,$sp,48

$L98:
	lw	$2,%lo(v6_table_path)($17)
	lw	$3,60($16)
	lw	$2,0($2)
	addu	$2,$2,$19
	b	$L99
	sw	$3,4($2)

$L100:
	b	$L101
	sw	$3,4($21)

	.set	macro
	.set	reorder
	.end	rtk_delV6Connection
	.size	rtk_delV6Connection, .-rtk_delV6Connection
	.align	2
	.globl	rtk_flushV6Connection
	.set	nomips16
	.set	nomicromips
	.ent	rtk_flushV6Connection
	.type	rtk_flushV6Connection, @function
rtk_flushV6Connection:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	lui	$2,%hi(v6_path_table_list_max)
	lw	$11,%lo(v6_path_table_list_max)($2)
	beq	$11,$0,$L114
	nop

	lui	$6,%hi(v6_path_list_inuse)
	lui	$4,%hi(v6_path_list_free)
	move	$8,$0
	move	$10,$0
	lui	$9,%hi(v6_table_path)
	addiu	$6,$6,%lo(v6_path_list_inuse)
	addiu	$4,$4,%lo(v6_path_list_free)
$L112:
	lw	$2,%lo(v6_table_path)($9)
	lw	$2,0($2)
	addu	$2,$2,$8
	lw	$2,0($2)
	bne	$2,$0,$L119
	nop

	b	$L127
	addiu	$10,$10,1

$L125:
	lw	$5,60($2)
	sw	$5,60($3)
	lw	$3,60($2)
	lw	$5,56($2)
	sw	$5,0($3)
	lw	$3,%lo(v6_table_path)($9)
	lw	$3,0($3)
	addu	$3,$3,$8
	lw	$5,8($3)
	addiu	$5,$5,-1
	sw	$5,8($3)
	lw	$3,64($2)
	beq	$3,$0,$L117
	lw	$5,68($2)

$L126:
	sw	$5,68($3)
	lw	$5,68($2)
	lw	$3,64($2)
$L118:
	sw	$3,0($5)
	addiu	$7,$2,64
	lw	$3,8($6)
	lw	$5,4($4)
	sw	$0,64($2)
	addiu	$3,$3,-1
	sw	$5,68($2)
	sw	$3,8($6)
	sw	$2,0($5)
	lw	$3,8($4)
	lw	$2,56($2)
	sw	$7,4($4)
	addiu	$3,$3,1
	beq	$2,$0,$L120
	sw	$3,8($4)

$L119:
	lw	$3,56($2)
	bne	$3,$0,$L125
	nop

	lw	$3,%lo(v6_table_path)($9)
	lw	$5,60($2)
	lw	$3,0($3)
	addu	$3,$3,$8
	sw	$5,4($3)
	lw	$3,60($2)
	lw	$5,56($2)
	sw	$5,0($3)
	lw	$3,%lo(v6_table_path)($9)
	lw	$3,0($3)
	addu	$3,$3,$8
	lw	$5,8($3)
	addiu	$5,$5,-1
	sw	$5,8($3)
	lw	$3,64($2)
	bne	$3,$0,$L126
	lw	$5,68($2)

$L117:
	b	$L118
	sw	$5,4($6)

$L120:
	addiu	$10,$10,1
$L127:
	bne	$10,$11,$L112
	addiu	$8,$8,12

$L114:
	jr	$31
	move	$2,$0

	.set	macro
	.set	reorder
	.end	rtk_flushV6Connection
	.size	rtk_flushV6Connection, .-rtk_flushV6Connection
	.section	.rodata.str1.4
	.align	2
$LC4:
	.ascii	"net/rtl/fastpath/core/fast_ipv6_core.c\000"
	.text
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	ipv6_fast_forward_single_write
	.type	ipv6_fast_forward_single_write, @function
ipv6_fast_forward_single_write:
	.frame	$sp,40,$31		# vars= 8, regs= 3/0, args= 16, gp= 0
	.mask	0x80030000,-4
	.fmask	0x00000000,0
	sltu	$2,$6,2
	bne	$2,$0,$L139
	beq	$5,$0,$L139
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
	bne	$3,$0,$L132
	move	$17,$6
	.set	macro
	.set	reorder

	lui	$4,%hi($LC4)
	li	$5,1218			# 0x4c2
	.set	noreorder
	.set	nomacro
	jal	__might_fault
	addiu	$4,$4,%lo($LC4)
	.set	macro
	.set	reorder

	addiu	$4,$sp,16
	move	$5,$16
	li	$6,1			# 0x1
 #APP
 # 1228 "net/rtl/fastpath/core/fast_ipv6_core.c" 1
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
	bne	$6,$0,$L142
	lb	$3,16($sp)
	.set	macro
	.set	reorder

	lui	$2,%hi(fast_ipv6_fw)
	addiu	$3,$3,-48
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L143
	sw	$3,%lo(fast_ipv6_fw)($2)
	.set	macro
	.set	reorder

	move	$2,$17
$L144:
	lw	$31,36($sp)
	lw	$17,32($sp)
	lw	$16,28($sp)
	.set	noreorder
	.set	nomacro
	jr	$31
	addiu	$sp,$sp,40
	.set	macro
	.set	reorder

$L143:
	jal	rtk_flushV6Connection
	.set	noreorder
	.set	nomacro
	b	$L144
	move	$2,$17
	.set	macro
	.set	reorder

$L132:
	li	$6,1			# 0x1
	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	memset
	addiu	$4,$sp,16
	.set	macro
	.set	reorder

$L142:
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

$L139:
	.set	noreorder
	.set	nomacro
	jr	$31
	li	$2,-14			# 0xfffffffffffffff2
	.set	macro
	.set	reorder

	.end	ipv6_fast_forward_single_write
	.size	ipv6_fast_forward_single_write, .-ipv6_fast_forward_single_write
	.align	2
	.globl	rtk_idleV6Connection
	.set	nomips16
	.set	nomicromips
	.ent	rtk_idleV6Connection
	.type	rtk_idleV6Connection, @function
rtk_idleV6Connection:
	.frame	$sp,56,$31		# vars= 0, regs= 9/0, args= 16, gp= 0
	.mask	0x80ff0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-56
	sw	$31,52($sp)
	sw	$23,48($sp)
	sw	$22,44($sp)
	sw	$21,40($sp)
	sw	$20,36($sp)
	sw	$19,32($sp)
	sw	$18,28($sp)
	sw	$17,24($sp)
	sw	$16,20($sp)
	move	$17,$4
	lw	$12,0($4)
	move	$20,$5
	lw	$7,40($4)
	lui	$22,%hi(v6_path_table_list_max)
	lw	$11,4($4)
	lui	$23,%hi(v6_table_path)
	lw	$10,8($4)
	srl	$9,$12,8
	lw	$5,12($4)
	xor	$4,$7,$12
	lw	$15,16($17)
	xor	$4,$4,$11
	lw	$14,20($17)
	xor	$4,$4,$10
	lw	$3,24($17)
	xor	$4,$4,$5
	lw	$13,28($17)
	xor	$4,$4,$15
	xor	$4,$4,$14
	lw	$2,36($17)
	xor	$4,$4,$3
	lhu	$24,32($17)
	srl	$16,$11,8
	lw	$8,%lo(v6_path_table_list_max)($22)
	xor	$4,$4,$13
	lw	$19,%lo(v6_table_path)($23)
	xor	$4,$4,$9
	srl	$25,$10,8
	xor	$4,$4,$16
	srl	$16,$5,8
	xor	$4,$4,$25
	srl	$25,$15,16
	xor	$4,$4,$16
	srl	$16,$14,16
	xor	$4,$4,$25
	srl	$25,$3,16
	xor	$4,$4,$16
	srl	$16,$13,16
	xor	$4,$4,$25
	srl	$25,$2,4
	xor	$4,$4,$16
	xor	$4,$4,$25
	xor	$4,$4,$24
	srl	$25,$4,12
	addiu	$8,$8,-1
	xor	$4,$25,$4
	and	$4,$4,$8
	lui	$9,%hi(jiffies)
	sll	$25,$4,1
	addu	$4,$25,$4
	lw	$18,%lo(jiffies)($9)
	lw	$9,0($19)
	sll	$4,$4,2
	addu	$4,$9,$4
	lw	$16,0($4)
	beq	$16,$0,$L146
	move	$19,$6

	b	$L153
	addiu	$21,$17,16

$L147:
	lw	$16,56($16)
	beq	$16,$0,$L172
	lw	$4,%lo(v6_table_path)($23)

$L153:
	lw	$3,36($16)
	bne	$3,$2,$L147
	nop

	lw	$3,40($16)
	bne	$3,$7,$L147
	li	$6,16			# 0x10

	move	$5,$17
	jal	memcmp
	move	$4,$16

	beq	$2,$0,$L171
	li	$6,16			# 0x10

$L150:
	lw	$16,56($16)
	lw	$7,40($17)
	bne	$16,$0,$L153
	lw	$2,36($17)

	lw	$4,%lo(v6_table_path)($23)
$L172:
	lw	$8,%lo(v6_path_table_list_max)($22)
	lw	$15,16($17)
	lw	$3,24($17)
	lw	$9,0($4)
	addiu	$8,$8,-1
	lw	$12,0($17)
	lw	$11,4($17)
	lw	$10,8($17)
	lw	$5,12($17)
	lw	$14,20($17)
	lw	$13,28($17)
	lhu	$24,32($17)
$L146:
	xor	$4,$2,$15
	xor	$4,$4,$3
	xor	$4,$4,$12
	xor	$4,$4,$11
	xor	$4,$4,$10
	xor	$4,$4,$5
	xor	$4,$4,$14
	xor	$4,$4,$13
	srl	$15,$15,8
	xor	$4,$4,$24
	srl	$14,$14,8
	xor	$4,$4,$15
	srl	$3,$3,8
	xor	$4,$4,$14
	srl	$13,$13,8
	xor	$3,$4,$3
	srl	$12,$12,16
	xor	$3,$3,$13
	srl	$11,$11,16
	xor	$3,$3,$12
	srl	$10,$10,16
	xor	$3,$3,$11
	srl	$5,$5,16
	xor	$3,$3,$10
	srl	$7,$7,4
	xor	$3,$3,$5
	xor	$3,$3,$7
	srl	$4,$3,12
	xor	$3,$4,$3
	and	$8,$3,$8
	sll	$3,$8,1
	addu	$8,$3,$8
	sll	$8,$8,2
	addu	$8,$9,$8
	lw	$16,0($8)
	beq	$16,$0,$L154
	addiu	$21,$17,16

	b	$L173
	lw	$3,36($16)

$L156:
	lw	$16,56($16)
	beq	$16,$0,$L145
	move	$2,$0

	lw	$2,36($17)
	lw	$3,36($16)
$L173:
	bne	$3,$2,$L156
	nop

	lw	$3,40($16)
	lw	$2,40($17)
	bne	$3,$2,$L156
	li	$6,16			# 0x10

	move	$4,$16
	jal	memcmp
	move	$5,$17

	bne	$2,$0,$L156
	addiu	$4,$16,16

	li	$6,16			# 0x10
	jal	memcmp
	move	$5,$21

	bne	$2,$0,$L156
	nop

	lw	$3,32($16)
	lw	$2,32($17)
	bne	$3,$2,$L156
	subu	$2,$18,$20

	lw	$3,44($16)
	subu	$2,$2,$3
	bgez	$2,$L154
	nop

$L169:
	sw	$3,0($19)
	b	$L145
	li	$2,-1			# 0xffffffffffffffff

$L154:
	move	$2,$0
$L145:
	lw	$31,52($sp)
	lw	$23,48($sp)
	lw	$22,44($sp)
	lw	$21,40($sp)
	lw	$20,36($sp)
	lw	$19,32($sp)
	lw	$18,28($sp)
	lw	$17,24($sp)
	lw	$16,20($sp)
	jr	$31
	addiu	$sp,$sp,56

$L171:
	addiu	$4,$16,16
	jal	memcmp
	move	$5,$21

	bne	$2,$0,$L150
	nop

	lw	$4,32($16)
	lw	$2,32($17)
	bne	$4,$2,$L150
	nop

	lw	$3,44($16)
	subu	$2,$18,$20
	subu	$2,$2,$3
	bltz	$2,$L169
	andi	$24,$4,0xffff

	lw	$3,%lo(v6_table_path)($23)
	lw	$8,%lo(v6_path_table_list_max)($22)
	lw	$7,40($17)
	lw	$2,36($17)
	lw	$9,0($3)
	addiu	$8,$8,-1
	lw	$15,16($17)
	lw	$3,24($17)
	lw	$12,0($17)
	lw	$11,4($17)
	lw	$10,8($17)
	lw	$5,12($17)
	lw	$14,20($17)
	b	$L146
	lw	$13,28($17)

	.set	macro
	.set	reorder
	.end	rtk_idleV6Connection
	.size	rtk_idleV6Connection, .-rtk_idleV6Connection
	.align	2
	.globl	rtl_V6_Cache_Timer_update
	.set	nomips16
	.set	nomicromips
	.ent	rtl_V6_Cache_Timer_update
	.type	rtl_V6_Cache_Timer_update, @function
rtl_V6_Cache_Timer_update:
	.frame	$sp,80,$31		# vars= 48, regs= 4/0, args= 16, gp= 0
	.mask	0x80070000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-80
	sw	$31,76($sp)
	sw	$18,72($sp)
	sw	$17,68($sp)
	sw	$16,64($sp)
	lui	$2,%hi(jiffies)
	move	$5,$0
	move	$16,$4
	lw	$18,%lo(jiffies)($2)
	jal	rtl_get_ct_protonum
	nop

	li	$3,17			# 0x11
	beq	$2,$3,$L183
	move	$5,$0

	jal	rtl_get_ct_protonum
	move	$4,$16

	li	$3,6			# 0x6
	bne	$2,$3,$L185
	lw	$31,76($sp)

	jal	rtl_get_ct_tcp_state
	move	$4,$16

	sltu	$2,$2,6
	beq	$2,$0,$L185
	lw	$31,76($sp)

	jal	rtl_tcp_get_timeouts
	move	$4,$16

	move	$17,$2
$L177:
	li	$6,44			# 0x2c
	move	$5,$0
	jal	memset
	addiu	$4,$sp,16

	move	$5,$0
	jal	rtl_get_ct_protonum_ipv6
	move	$4,$16

	move	$6,$0
	move	$5,$0
	sw	$2,48($sp)
	addiu	$7,$sp,16
	jal	rtl_get_ct_v6_ip_by_dir
	move	$4,$16

	addiu	$7,$sp,32
	li	$6,1			# 0x1
	move	$5,$0
	jal	rtl_get_ct_v6_ip_by_dir
	move	$4,$16

	move	$6,$0
	move	$5,$0
	jal	rtl_get_ct_port_by_dir
	move	$4,$16

	li	$6,1			# 0x1
	move	$5,$0
	sw	$2,52($sp)
	jal	rtl_get_ct_port_by_dir
	move	$4,$16

	addiu	$6,$sp,60
	addiu	$4,$sp,16
	sw	$2,56($sp)
	jal	rtk_idleV6Connection
	move	$5,$17

	bne	$2,$0,$L184
	lw	$31,76($sp)

$L185:
	li	$2,-1			# 0xffffffffffffffff
	lw	$18,72($sp)
	lw	$17,68($sp)
	lw	$16,64($sp)
	jr	$31
	addiu	$sp,$sp,80

$L184:
	lw	$5,60($sp)
	move	$4,$16
	jal	rtl_set_ct_timeout_expires
	addu	$5,$17,$5

	addu	$5,$17,$18
	jal	rtl_check_for_acc
	move	$4,$16

	move	$2,$0
	lw	$31,76($sp)
	lw	$18,72($sp)
	lw	$17,68($sp)
	lw	$16,64($sp)
	jr	$31
	addiu	$sp,$sp,80

$L183:
	jal	rtl_get_ct_udp_status
	move	$4,$16

	andi	$2,$2,0x2
	beq	$2,$0,$L176
	li	$6,1			# 0x1

	li	$4,1			# 0x1
	jal	udp_get_timeouts_by_state
	move	$5,$16

	b	$L177
	move	$17,$2

$L176:
	move	$4,$0
	jal	udp_get_timeouts_by_state
	move	$5,$16

	b	$L177
	move	$17,$2

	.set	macro
	.set	reorder
	.end	rtl_V6_Cache_Timer_update
	.size	rtl_V6_Cache_Timer_update, .-rtl_V6_Cache_Timer_update
	.align	2
	.globl	V6_udp_fragCache_init
	.set	nomips16
	.set	nomicromips
	.ent	V6_udp_fragCache_init
	.type	V6_udp_fragCache_init, @function
V6_udp_fragCache_init:
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
	lui	$18,%hi(kmalloc_caches)
	move	$16,$4
	addiu	$18,$18,%lo(kmalloc_caches)
	li	$22,34078720			# 0x2080000
	lui	$20,%hi(V6_udp_cache_table)
	lw	$4,28($18)
	jal	kmem_cache_alloc
	ori	$5,$22,0x8020

	beq	$2,$0,$L193
	sw	$2,%lo(V6_udp_cache_table)($20)

	sll	$4,$16,1
	move	$21,$2
	lui	$2,%hi(V6_udp_cache_list_free)
	addu	$4,$4,$16
	addiu	$17,$2,%lo(V6_udp_cache_list_free)
	lui	$19,%hi(V6_max_udp_frag_entry)
	sw	$0,%lo(V6_udp_cache_list_free)($2)
	sll	$4,$4,2
	addiu	$5,$22,32
	sw	$0,8($17)
	sw	$17,4($17)
	jal	__kmalloc
	sw	$16,%lo(V6_max_udp_frag_entry)($19)

	sw	$2,0($21)
	lw	$4,%lo(V6_udp_cache_table)($20)
	lw	$3,0($4)
	beq	$3,$0,$L186
	li	$2,-1			# 0xffffffffffffffff

	blez	$16,$L189
	move	$2,$0

	b	$L190
	move	$5,$0

$L202:
	lw	$3,0($4)
$L190:
	addu	$3,$3,$2
	addiu	$5,$5,1
	sw	$0,8($3)
	lw	$3,0($4)
	addu	$3,$3,$2
	sw	$0,0($3)
	lw	$4,%lo(V6_udp_cache_table)($20)
	lw	$3,0($4)
	addu	$3,$3,$2
	addiu	$2,$2,12
	bne	$16,$5,$L202
	sw	$3,4($3)

$L189:
	lw	$2,%lo(V6_max_udp_frag_entry)($19)
	blez	$2,$L192
	move	$20,$0

	li	$21,34078720			# 0x2080000
	lui	$22,%hi(V6_cache_timeout)
	addiu	$21,$21,32
	b	$L198
	addiu	$22,$22,%lo(V6_cache_timeout)

$L203:
	addiu	$4,$2,52
	move	$6,$0
	jal	init_timer_key
	move	$5,$0

	addiu	$4,$16,88
	lw	$3,4($17)
	lw	$2,%lo(V6_max_udp_frag_entry)($19)
	sw	$16,68($16)
	sw	$3,92($16)
	sw	$22,64($16)
	slt	$2,$20,$2
	sw	$0,88($16)
	sw	$16,0($3)
	lw	$3,8($17)
	sw	$4,4($17)
	addiu	$3,$3,1
	beq	$2,$0,$L192
	sw	$3,8($17)

$L198:
	lw	$4,28($18)
	move	$5,$21
	jal	kmem_cache_alloc
	addiu	$20,$20,1

	move	$7,$0
	bne	$2,$0,$L203
	move	$16,$2

$L193:
	move	$2,$0
$L186:
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

$L192:
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
	.end	V6_udp_fragCache_init
	.size	V6_udp_fragCache_init, .-V6_udp_fragCache_init
	.align	2
	.globl	ipv6_fast_enter
	.set	nomips16
	.set	nomicromips
	.ent	ipv6_fast_enter
	.type	ipv6_fast_enter, @function
ipv6_fast_enter:
	.frame	$sp,136,$31		# vars= 80, regs= 10/0, args= 16, gp= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-136
	sw	$31,132($sp)
	sw	$fp,128($sp)
	sw	$23,124($sp)
	sw	$22,120($sp)
	sw	$21,116($sp)
	sw	$20,112($sp)
	sw	$19,108($sp)
	sw	$18,104($sp)
	sw	$17,100($sp)
	sw	$16,96($sp)
	jal	rtl_skb_mac_header
	move	$17,$4

	move	$4,$17
	jal	rtl_ipv6_hdr
	move	$16,$2

	move	$4,$17
	jal	rtl_get_skb_pppoe_flag
	move	$18,$2

	bne	$2,$0,$L205
	li	$3,56710			# 0xdd86

	lbu	$2,13($16)
	lbu	$4,12($16)
	sll	$2,$2,8
	or	$2,$2,$4
	beq	$2,$3,$L205
	nop

$L207:
	move	$3,$0
$L204:
	lw	$31,132($sp)
$L267:
	move	$2,$3
	lw	$fp,128($sp)
	lw	$23,124($sp)
	lw	$22,120($sp)
	lw	$21,116($sp)
	lw	$20,112($sp)
	lw	$19,108($sp)
	lw	$18,104($sp)
	lw	$17,100($sp)
	lw	$16,96($sp)
	jr	$31
	addiu	$sp,$sp,136

$L205:
	jal	__ipv6_addr_type
	addiu	$4,$18,8

	andi	$2,$2,0x20
	bne	$2,$0,$L204
	move	$3,$0

	jal	__ipv6_addr_type
	addiu	$4,$18,24

	andi	$2,$2,0x22
	bne	$2,$0,$L207
	li	$3,6			# 0x6

	lbu	$2,6($18)
	beq	$2,$3,$L208
	li	$3,17			# 0x11

	beq	$2,$3,$L209
	li	$3,44			# 0x2c

	bne	$2,$3,$L204
	move	$3,$0

$L209:
	lw	$9,20($18)
	li	$2,44			# 0x2c
	lw	$4,32($18)
	lw	$8,8($18)
	lw	$7,12($18)
	lw	$6,16($18)
	lw	$5,28($18)
	lw	$3,36($18)
	sw	$9,76($sp)
	sw	$4,56($sp)
	sw	$8,64($sp)
	sw	$7,68($sp)
	sw	$6,72($sp)
	sw	$5,52($sp)
	lw	$4,24($18)
	lhu	$23,40($18)
	lhu	$fp,42($18)
	sw	$4,48($sp)
	sw	$3,60($sp)
	lbu	$19,6($18)
	beq	$19,$2,$L264
	move	$20,$0

	sw	$0,80($sp)
	move	$22,$0
	move	$21,$0
$L211:
	lw	$3,64($sp)
	srl	$2,$23,4
	lw	$13,68($sp)
	lw	$9,72($sp)
	lw	$12,76($sp)
	srl	$14,$3,8
	xor	$4,$3,$13
	lw	$11,48($sp)
	lw	$10,52($sp)
	xor	$4,$4,$9
	lui	$3,%hi(v6_path_table_list_max)
	lw	$7,56($sp)
	xor	$4,$4,$12
	lw	$6,60($sp)
	xor	$4,$4,$11
	xor	$4,$4,$10
	lw	$5,%lo(v6_path_table_list_max)($3)
	lui	$3,%hi(v6_table_path)
	xor	$4,$4,$7
	xor	$4,$4,$6
	srl	$13,$13,8
	lw	$15,%lo(v6_table_path)($3)
	xor	$4,$4,$14
	srl	$3,$9,8
	xor	$4,$4,$13
	srl	$12,$12,8
	lw	$9,0($15)
	xor	$3,$4,$3
	srl	$11,$11,16
	xor	$3,$3,$12
	srl	$10,$10,16
	xor	$3,$3,$11
	srl	$7,$7,16
	xor	$3,$3,$10
	srl	$6,$6,16
	xor	$3,$3,$7
	xor	$3,$3,$6
	xor	$3,$3,$fp
	xor	$3,$3,$19
	xor	$2,$3,$2
	srl	$3,$2,12
	addiu	$5,$5,-1
	xor	$2,$3,$2
	and	$2,$5,$2
	sll	$3,$2,1
	addu	$2,$3,$2
	sll	$2,$2,2
	addu	$2,$9,$2
	lw	$16,0($2)
	bne	$16,$0,$L234
	move	$3,$0

	b	$L267
	lw	$31,132($sp)

$L218:
	lw	$16,56($16)
	beq	$16,$0,$L204
	move	$3,$0

$L234:
	lw	$2,36($16)
	bne	$23,$2,$L218
	nop

	lw	$2,40($16)
	bne	$fp,$2,$L218
	li	$6,16			# 0x10

	addiu	$5,$sp,64
	jal	memcmp
	move	$4,$16

	bne	$2,$0,$L218
	addiu	$4,$16,16

	li	$6,16			# 0x10
	jal	memcmp
	addiu	$5,$sp,48

	bne	$2,$0,$L218
	move	$3,$2

	lw	$2,32($16)
	bne	$19,$2,$L218
	lui	$2,%hi(ipv6_rtcache)

	sw	$3,84($sp)
	lw	$2,%lo(ipv6_rtcache)($2)
	beq	$2,$0,$L219
	move	$6,$0

	lw	$7,48($16)
	li	$4,10			# 0xa
	jal	rtl_ip_route_input
	move	$5,$17

	bne	$2,$0,$L207
	lw	$3,84($sp)

$L221:
	move	$4,$17
	jal	rtl_get_skb_ipv6_dst
	sw	$3,84($sp)

	beq	$2,$0,$L207
	lw	$3,84($sp)

	move	$4,$17
	jal	rtl_skb_ipv6_dst_check
	sw	$3,84($sp)

	li	$5,-1			# 0xffffffffffffffff
	beq	$2,$5,$L223
	lw	$3,84($sp)

	move	$4,$17
	sw	$5,88($sp)
	jal	rtl_check_dst_input
	sw	$3,84($sp)

	lw	$5,88($sp)
	beq	$2,$5,$L223
	lw	$3,84($sp)

	lw	$2,80($sp)
	beq	$2,$0,$L224
	lw	$14,64($sp)

	lui	$4,%hi(V6_max_udp_frag_entry)
	lw	$13,68($sp)
	lui	$6,%hi(V6_udp_cache_list_free)
	lw	$12,72($sp)
	lw	$11,76($sp)
	srl	$24,$14,8
	lw	$10,48($sp)
	xor	$2,$14,$13
	lw	$9,52($sp)
	xor	$2,$2,$12
	lw	$8,56($sp)
	xor	$2,$2,$11
	lw	$7,60($sp)
	xor	$2,$2,$10
	xor	$2,$2,$9
	lw	$4,%lo(V6_max_udp_frag_entry)($4)
	xor	$2,$2,$8
	lw	$15,4($22)
	xor	$2,$2,$7
	lw	$5,%lo(V6_udp_cache_list_free)($6)
	srl	$20,$13,8
	xor	$2,$2,$24
	srl	$25,$12,8
	srl	$24,$11,8
	xor	$2,$2,$20
	xor	$2,$2,$25
	srl	$25,$10,16
	xor	$2,$2,$24
	srl	$24,$9,16
	xor	$2,$2,$25
	srl	$25,$8,16
	xor	$2,$2,$24
	srl	$24,$7,16
	xor	$2,$2,$25
	xor	$2,$2,$24
	srl	$24,$2,16
	addiu	$4,$4,-1
	xor	$2,$24,$2
	xor	$2,$2,$19
	xor	$2,$15,$2
	beq	$5,$0,$L223
	and	$2,$4,$2

	lw	$3,88($5)
	li	$4,1			# 0x1
	andi	$23,$23,0xffff
	sw	$15,0($5)
	andi	$fp,$fp,0xffff
	sw	$14,4($5)
	sw	$13,8($5)
	sw	$12,12($5)
	sw	$11,16($5)
	sw	$10,20($5)
	sw	$9,24($5)
	sw	$8,28($5)
	sw	$7,32($5)
	sw	$19,36($5)
	sw	$23,40($5)
	sw	$fp,44($5)
	beq	$3,$0,$L225
	sb	$4,48($5)

	lw	$4,92($5)
	addiu	$6,$6,%lo(V6_udp_cache_list_free)
	sw	$4,92($3)
	lw	$3,88($5)
	lw	$4,92($5)
$L226:
	sw	$3,0($4)
	lui	$7,%hi(V6_udp_cache_table)
	lw	$3,8($6)
	li	$4,12			# 0xc
	sw	$0,80($5)
	addiu	$9,$5,80
	mul	$2,$2,$4
	lw	$8,%lo(V6_udp_cache_table)($7)
	addiu	$3,$3,-1
	addiu	$4,$5,52
	sw	$3,8($6)
	lw	$3,0($8)
	addu	$3,$3,$2
	lw	$3,4($3)
	sw	$3,84($5)
	lw	$3,0($8)
	addu	$3,$3,$2
	lw	$3,4($3)
	sw	$5,0($3)
	lui	$3,%hi(jiffies)
	lw	$7,%lo(V6_udp_cache_table)($7)
	lw	$6,%lo(jiffies)($3)
	lw	$3,0($7)
	addiu	$6,$6,1000
	addu	$3,$3,$2
	sw	$9,4($3)
	lw	$3,0($7)
	addu	$2,$3,$2
	lw	$3,8($2)
	addiu	$3,$3,1
	sw	$3,8($2)
	jal	add_timer
	sw	$6,60($5)

$L227:
	move	$5,$0
$L268:
	jal	rtl_set_skb_dev
	move	$4,$17

	lbu	$2,7($18)
	sltu	$3,$2,2
	bne	$3,$0,$L230
	addiu	$2,$2,-1

	move	$4,$17
	sb	$2,7($18)
	jal	rtl_set_skb_mark
	lw	$5,52($16)

	jal	rtl_get_skb_len
	move	$4,$17

	move	$4,$17
	jal	rtl_ip6_skb_dst_mtu
	move	$18,$2

	sltu	$2,$2,$18
	beq	$2,$0,$L233
	nop

	jal	rtl_skb_is_gso
	move	$4,$17

	beq	$2,$0,$L230
	nop

$L233:
	jal	rtl_dst_allfrag
	move	$4,$17

	beq	$2,$0,$L265
	lui	$2,%hi(jiffies)

$L230:
	jal	kfree_skb
	move	$4,$17

	b	$L204
	li	$3,1			# 0x1

$L208:
	lw	$2,52($18)
	andi	$2,$2,0x700
	beq	$2,$0,$L209
	move	$3,$0

	b	$L267
	lw	$31,132($sp)

$L264:
	lbu	$3,40($18)
	li	$2,17			# 0x11
	beq	$3,$2,$L266
	addiu	$22,$18,40

	move	$20,$0
	sw	$0,80($sp)
	move	$21,$0
	b	$L211
	li	$19,44			# 0x2c

$L266:
	wsbh	$2,$fp
	li	$3,1			# 0x1
	andi	$2,$2,0xffff
	andi	$4,$2,0xfff9
	beq	$4,$3,$L212
	andi	$3,$2,0xfff8

	beq	$3,$0,$L213
	move	$21,$0

	nor	$21,$0,$2
	andi	$21,$21,0x1
$L213:
	lw	$11,68($sp)
	lui	$3,%hi(V6_max_udp_frag_entry)
	lw	$12,64($sp)
	lui	$9,%hi(V6_udp_cache_table)
	lw	$10,72($sp)
	lw	$8,76($sp)
	srl	$13,$11,8
	lw	$7,48($sp)
	xor	$2,$12,$11
	lw	$6,52($sp)
	xor	$2,$2,$10
	lw	$5,56($sp)
	xor	$2,$2,$8
	lw	$4,60($sp)
	xor	$2,$2,$7
	xor	$2,$2,$6
	sw	$11,36($sp)
	xor	$2,$2,$5
	sw	$8,44($sp)
	srl	$14,$12,8
	sw	$7,16($sp)
	xor	$2,$2,$4
	sw	$6,20($sp)
	xor	$2,$2,$14
	sw	$5,24($sp)
	srl	$11,$10,8
	sw	$4,28($sp)
	srl	$8,$8,8
	lw	$3,%lo(V6_max_udp_frag_entry)($3)
	xor	$2,$2,$13
	lw	$16,44($18)
	xor	$2,$2,$11
	lw	$9,%lo(V6_udp_cache_table)($9)
	srl	$7,$7,16
	sw	$12,32($sp)
	xor	$2,$2,$8
	sw	$10,40($sp)
	srl	$6,$6,16
	xor	$2,$2,$7
	lw	$8,0($9)
	srl	$5,$5,16
	srl	$4,$4,16
	xor	$2,$2,$6
	xor	$2,$2,$5
	xor	$2,$2,$4
	srl	$4,$2,16
	xori	$2,$2,0x11
	addiu	$3,$3,-1
	xor	$2,$2,$4
	xor	$2,$2,$16
	and	$2,$3,$2
	sll	$3,$2,1
	addu	$2,$3,$2
	sll	$2,$2,2
	addu	$2,$8,$2
	lw	$20,0($2)
	bne	$20,$0,$L250
	li	$19,17			# 0x11

	b	$L204
	move	$3,$0

$L216:
	lw	$20,80($20)
	beq	$20,$0,$L204
	move	$3,$0

$L250:
	lw	$2,0($20)
	bne	$16,$2,$L216
	addiu	$4,$20,4

	li	$6,16			# 0x10
	jal	memcmp
	addiu	$5,$sp,32

	bne	$2,$0,$L216
	addiu	$4,$20,20

	li	$6,16			# 0x10
	jal	memcmp
	addiu	$5,$sp,16

	bne	$2,$0,$L216
	sw	$2,80($sp)

	lw	$2,36($20)
	bne	$2,$19,$L216
	nop

	lw	$23,40($20)
	li	$19,17			# 0x11
	b	$L211
	lw	$fp,44($20)

$L212:
	li	$2,1			# 0x1
	lhu	$23,48($18)
	move	$20,$0
	lhu	$fp,50($18)
	move	$21,$0
	sw	$2,80($sp)
	b	$L211
	li	$19,17			# 0x11

$L219:
	jal	rtl_ip6_route_input
	move	$4,$17

	li	$4,-1			# 0xffffffffffffffff
	bne	$2,$4,$L221
	lw	$3,84($sp)

	b	$L204
	move	$3,$0

$L265:
	move	$4,$17
	lw	$2,%lo(jiffies)($2)
	jal	fast_path_post_process_xmit_check6
	sw	$2,44($16)

	move	$3,$2
	li	$2,1			# 0x1
	beq	$3,$2,$L267
	lw	$31,132($sp)

	jal	ip_finish_output6
	move	$4,$17

	lui	$4,%hi(statistic_ipv6_fp)
	li	$3,1			# 0x1
	lw	$2,%lo(statistic_ipv6_fp)($4)
	addiu	$2,$2,1
	b	$L204
	sw	$2,%lo(statistic_ipv6_fp)($4)

$L223:
	move	$4,$17
	jal	rtl_dst_release_ipv6
	sw	$3,80($sp)

	b	$L204
	lw	$3,80($sp)

$L225:
	addiu	$6,$6,%lo(V6_udp_cache_list_free)
	lw	$4,92($5)
	b	$L226
	sw	$4,4($6)

$L224:
	beq	$21,$0,$L268
	move	$5,$0

	beq	$20,$0,$L268
	nop

	jal	del_timer
	addiu	$4,$20,52

	lui	$3,%hi(V6_max_udp_frag_entry)
	lw	$10,8($20)
	lw	$2,4($20)
	lw	$13,12($20)
	lw	$12,16($20)
	srl	$14,$10,8
	srl	$15,$2,8
	lw	$9,20($20)
	xor	$2,$2,$10
	lw	$7,24($20)
	xor	$2,$2,$13
	lw	$6,28($20)
	xor	$2,$2,$12
	lw	$5,32($20)
	xor	$2,$2,$9
	lhu	$4,36($20)
	xor	$2,$2,$7
	lw	$11,0($20)
	xor	$2,$2,$6
	lw	$3,%lo(V6_max_udp_frag_entry)($3)
	xor	$2,$2,$5
	lw	$10,80($20)
	xor	$2,$2,$15
	sb	$0,48($20)
	srl	$13,$13,8
	xor	$2,$2,$14
	srl	$12,$12,8
	xor	$2,$2,$13
	srl	$9,$9,16
	xor	$2,$2,$12
	srl	$7,$7,16
	xor	$2,$2,$9
	srl	$6,$6,16
	srl	$5,$5,16
	xor	$2,$2,$7
	xor	$2,$2,$6
	xor	$2,$2,$5
	srl	$5,$2,16
	xor	$4,$4,$11
	xor	$4,$4,$2
	addiu	$2,$3,-1
	xor	$4,$5,$4
	beq	$10,$0,$L228
	and	$2,$2,$4

	lw	$3,84($20)
	lui	$9,%hi(V6_udp_cache_table)
	sw	$3,84($10)
$L229:
	lw	$4,84($20)
	li	$7,12			# 0xc
	lw	$5,80($20)
	lui	$3,%hi(V6_udp_cache_list_free)
	mul	$8,$2,$7
	addiu	$3,$3,%lo(V6_udp_cache_list_free)
	sw	$5,0($4)
	addiu	$6,$20,88
	lw	$4,%lo(V6_udp_cache_table)($9)
	lw	$5,4($3)
	lw	$4,0($4)
	addu	$2,$8,$4
	lw	$4,8($2)
	addiu	$4,$4,-1
	sw	$4,8($2)
	sw	$0,88($20)
	sw	$5,92($20)
	sw	$20,0($5)
	lw	$2,8($3)
	sw	$6,4($3)
	addiu	$2,$2,1
	b	$L227
	sw	$2,8($3)

$L228:
	lui	$9,%hi(V6_udp_cache_table)
	lw	$4,84($20)
	lw	$3,%lo(V6_udp_cache_table)($9)
	lw	$5,0($3)
	li	$3,12			# 0xc
	mul	$6,$2,$3
	addu	$3,$6,$5
	b	$L229
	sw	$4,4($3)

	.set	macro
	.set	reorder
	.end	ipv6_fast_enter
	.size	ipv6_fast_enter, .-ipv6_fast_enter
	.align	2
	.globl	init_V6_table_path
	.set	nomips16
	.set	nomicromips
	.ent	init_V6_table_path
	.type	init_V6_table_path, @function
init_V6_table_path:
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
	move	$19,$4
	addiu	$17,$17,%lo(kmalloc_caches)
	li	$22,34078720			# 0x2080000
	lui	$20,%hi(v6_table_path)
	lw	$4,28($17)
	move	$18,$5
	jal	kmem_cache_alloc
	ori	$5,$22,0x8020

	beq	$2,$0,$L272
	sw	$2,%lo(v6_table_path)($20)

	lui	$5,%hi(v6_path_list_inuse)
	move	$21,$2
	sll	$4,$19,1
	addiu	$2,$5,%lo(v6_path_list_inuse)
	sw	$0,%lo(v6_path_list_inuse)($5)
	lui	$3,%hi(v6_path_list_free)
	addu	$4,$4,$19
	addiu	$16,$3,%lo(v6_path_list_free)
	sw	$0,8($2)
	sw	$2,4($2)
	lui	$2,%hi(v6_path_table_list_max)
	sll	$4,$4,2
	sw	$0,%lo(v6_path_list_free)($3)
	addiu	$5,$22,32
	sw	$0,8($16)
	sw	$16,4($16)
	jal	__kmalloc
	sw	$19,%lo(v6_path_table_list_max)($2)

	sw	$2,0($21)
	lw	$4,%lo(v6_table_path)($20)
	lw	$3,0($4)
	beq	$3,$0,$L269
	li	$2,-1			# 0xffffffffffffffff

	blez	$19,$L273
	move	$2,$0

	b	$L274
	move	$5,$0

$L285:
	lw	$3,0($4)
$L274:
	addu	$3,$3,$2
	addiu	$5,$5,1
	sw	$0,8($3)
	lw	$3,0($4)
	addu	$3,$3,$2
	sw	$0,0($3)
	lw	$4,%lo(v6_table_path)($20)
	lw	$3,0($4)
	addu	$3,$3,$2
	addiu	$2,$2,12
	bne	$19,$5,$L285
	sw	$3,4($3)

$L273:
	blez	$18,$L276
	move	$19,$0

	li	$20,34078720			# 0x2080000
	b	$L281
	addiu	$20,$20,32

$L286:
	lw	$3,4($16)
	sw	$0,64($2)
	sw	$3,68($2)
	sw	$2,0($3)
	lw	$2,8($16)
	sw	$4,4($16)
	addiu	$2,$2,1
	beq	$18,$19,$L276
	sw	$2,8($16)

$L281:
	lw	$4,28($17)
	move	$5,$20
	jal	kmem_cache_alloc
	addiu	$19,$19,1

	bne	$2,$0,$L286
	addiu	$4,$2,64

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

$L276:
	move	$2,$0
$L269:
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

$L272:
	b	$L269
	li	$2,-1			# 0xffffffffffffffff

	.set	macro
	.set	reorder
	.end	init_V6_table_path
	.size	init_V6_table_path, .-init_V6_table_path
	.section	.exit.text
	.align	2
	.globl	ipv6_fast_forward_table_exit
	.set	nomips16
	.set	nomicromips
	.ent	ipv6_fast_forward_table_exit
	.type	ipv6_fast_forward_table_exit, @function
ipv6_fast_forward_table_exit:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	lui	$2,%hi(ipv6_res_table)
	lw	$2,%lo(ipv6_res_table)($2)
	beq	$2,$0,$L289
	nop

	lui	$5,%hi(proc_root)
	lui	$4,%hi($LC3)
	addiu	$5,$5,%lo(proc_root)
	addiu	$4,$4,%lo($LC3)
	j	remove_proc_entry
	nop

$L289:
	jr	$31
	nop

	.set	macro
	.set	reorder
	.end	ipv6_fast_forward_table_exit
	.size	ipv6_fast_forward_table_exit, .-ipv6_fast_forward_table_exit
	.local	ipv6_res_table
	.comm	ipv6_res_table,4,4
	.globl	ipv6_fast_forward_table_proc_fops
	.data
	.align	2
	.type	ipv6_fast_forward_table_proc_fops, @object
	.size	ipv6_fast_forward_table_proc_fops, 108
ipv6_fast_forward_table_proc_fops:
	.space	4
	.word	seq_lseek
	.word	seq_read
	.word	ipv6_fast_forward_table_single_write
	.space	28
	.word	ipv6_fast_forward_table_single_open
	.space	4
	.word	single_release
	.space	52
	.local	ipv6_res
	.comm	ipv6_res,4,4
	.globl	ipv6_fast_forward_proc_fops
	.align	2
	.type	ipv6_fast_forward_proc_fops, @object
	.size	ipv6_fast_forward_proc_fops, 108
ipv6_fast_forward_proc_fops:
	.space	4
	.word	seq_lseek
	.word	seq_read
	.word	ipv6_fast_forward_single_write
	.space	28
	.word	ipv6_fast_forward_single_open
	.space	4
	.word	single_release
	.space	52
	.local	V6_max_udp_frag_entry
	.comm	V6_max_udp_frag_entry,4,4
	.globl	V6_udp_cache_table
	.section	.bss,"aw",@nobits
	.align	2
	.type	V6_udp_cache_table, @object
	.size	V6_udp_cache_table, 4
V6_udp_cache_table:
	.space	4
	.globl	V6_udp_cache_list_free
	.align	2
	.type	V6_udp_cache_list_free, @object
	.size	V6_udp_cache_list_free, 12
V6_udp_cache_list_free:
	.space	12
	.local	v6_path_table_list_max
	.comm	v6_path_table_list_max,4,4
	.globl	v6_table_path
	.align	2
	.type	v6_table_path, @object
	.size	v6_table_path, 4
v6_table_path:
	.space	4
	.globl	v6_path_list_free
	.align	2
	.type	v6_path_list_free, @object
	.size	v6_path_list_free, 12
v6_path_list_free:
	.space	12
	.globl	v6_path_list_inuse
	.align	2
	.type	v6_path_list_inuse, @object
	.size	v6_path_list_inuse, 12
v6_path_list_inuse:
	.space	12
	.globl	ipv6_rtcache
	.data
	.align	2
	.type	ipv6_rtcache, @object
	.size	ipv6_rtcache, 4
ipv6_rtcache:
	.word	1
	.globl	fast_ipv6_fw
	.section	.bss
	.align	2
	.type	fast_ipv6_fw, @object
	.size	fast_ipv6_fw, 4
fast_ipv6_fw:
	.space	4
	.ident	"GCC: (Realtek MSDK-6.4.1 Build 3055) 6.4.1 20180425"
