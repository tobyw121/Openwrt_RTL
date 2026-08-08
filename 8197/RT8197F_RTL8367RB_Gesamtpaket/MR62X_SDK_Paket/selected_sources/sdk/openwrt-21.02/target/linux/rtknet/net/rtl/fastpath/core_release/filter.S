	.file	1 "filter.c"
	.section .mdebug.abi32
	.previous
	.nan	legacy
	.module	softfloat
	.module	oddspreg
	.text
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	dos_read_proc
	.type	dos_read_proc, @function
dos_read_proc:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	lui	$2,%hi(dos_read_proc_func)
	lw	$2,%lo(dos_read_proc_func)($2)
	beq	$2,$0,$L7
	nop

	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	jalr	$2
	nop

	move	$2,$0
	lw	$31,20($sp)
	jr	$31
	addiu	$sp,$sp,24

$L7:
	jr	$31
	move	$2,$0

	.set	macro
	.set	reorder
	.end	dos_read_proc
	.size	dos_read_proc, .-dos_read_proc
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	dos_timer_fn
	.type	dos_timer_fn, @function
dos_timer_fn:
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
	lui	$16,%hi(dos_item_global)
	lw	$2,%lo(dos_item_global)($16)
	bne	$2,$0,$L12
	lui	$18,%hi(dosFilter_block)

	lui	$2,%hi(dos_item_ipv6)
	lw	$2,%lo(dos_item_ipv6)($2)
	beq	$2,$0,$L44
	li	$3,1			# 0x1

	lw	$4,%lo(dosFilter_block)($18)
	beq	$4,$3,$L45
	lui	$17,%hi(block_count)

	lui	$2,%hi(dos_timer_ipv6_func)
$L48:
	lw	$2,%lo(dos_timer_ipv6_func)($2)
	beq	$2,$0,$L17
	nop

	jalr	$2
	nop

	b	$L47
	lui	$3,%hi(block_time)

$L12:
	li	$2,1			# 0x1
	lw	$3,%lo(dosFilter_block)($18)
	beq	$3,$2,$L46
	lui	$17,%hi(block_count)

$L27:
	lui	$2,%hi(dos_timer_ipv4_func)
	lw	$2,%lo(dos_timer_ipv4_func)($2)
	beq	$2,$0,$L41
	nop

	jalr	$2
	nop

$L41:
	lui	$2,%hi(dos_item_ipv6)
	lw	$2,%lo(dos_item_ipv6)($2)
$L16:
	bne	$2,$0,$L48
	lui	$2,%hi(dos_timer_ipv6_func)

$L17:
	lui	$3,%hi(block_time)
$L47:
	lw	$2,%lo(block_count)($17)
	lw	$3,%lo(block_time)($3)
	slt	$2,$2,$3
	beq	$2,$0,$L18
	lw	$2,%lo(dos_item_global)($16)

	ext	$2,$2,22,1
	bne	$2,$0,$L49
	lui	$2,%hi(none_dos_drop_pkt_cnt)

$L18:
	sw	$0,%lo(dosFilter_block)($18)
	jal	clear_attack_address
	sw	$0,%lo(block_count)($17)

	lui	$2,%hi(none_dos_drop_pkt_cnt)
$L49:
	lw	$3,%lo(none_dos_drop_pkt_cnt)($2)
	sltu	$4,$3,3001
	bne	$4,$0,$L50
	lui	$4,%hi(dos_item_backup)

	lw	$3,%lo(dos_item_global)($16)
	beq	$3,$0,$L22
	nop

	sw	$0,%lo(dos_item_global)($16)
	sw	$3,%lo(dos_item_backup)($4)
$L22:
	sw	$0,%lo(none_dos_drop_pkt_cnt)($2)
	lui	$2,%hi(jiffies)
	lui	$4,%hi(dos_timer)
	lw	$31,28($sp)
	lw	$18,24($sp)
	lw	$5,%lo(jiffies)($2)
	addiu	$4,$4,%lo(dos_timer)
	lw	$17,20($sp)
	lw	$16,16($sp)
	addiu	$5,$5,100
	j	mod_timer
	addiu	$sp,$sp,32

$L44:
	lui	$2,%hi(none_dos_drop_pkt_cnt)
	lw	$3,%lo(none_dos_drop_pkt_cnt)($2)
	sltu	$4,$3,3001
	beq	$4,$0,$L22
	lui	$4,%hi(dos_item_backup)

$L50:
	lw	$5,%lo(dos_item_backup)($4)
	beq	$5,$0,$L23
	nop

	sw	$5,%lo(dos_item_global)($16)
	sw	$0,%lo(dos_item_backup)($4)
$L23:
	bne	$3,$0,$L22
	lw	$31,28($sp)

	lui	$2,%hi(jiffies)
	lui	$4,%hi(dos_timer)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$5,%lo(jiffies)($2)
	addiu	$4,$4,%lo(dos_timer)
	lw	$16,16($sp)
	addiu	$5,$5,100
	j	mod_timer
	addiu	$sp,$sp,32

$L46:
	lw	$2,%lo(block_count)($17)
	addiu	$2,$2,1
	b	$L27
	sw	$2,%lo(block_count)($17)

$L45:
	lw	$3,%lo(block_count)($17)
	addiu	$3,$3,1
	b	$L16
	sw	$3,%lo(block_count)($17)

	.set	macro
	.set	reorder
	.end	dos_timer_fn
	.size	dos_timer_fn, .-dos_timer_fn
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	rtl_ip_range_filter_fastpath
	.type	rtl_ip_range_filter_fastpath, @function
rtl_ip_range_filter_fastpath:
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$16,16($sp)
	jal	rtl_ip_hdr
	move	$16,$5

	lw	$2,12($2)
	lw	$3,0($16)
	sltu	$3,$2,$3
	bne	$3,$0,$L53
	lw	$31,20($sp)

	lw	$3,4($16)
	lw	$16,16($sp)
	sltu	$2,$3,$2
	xori	$2,$2,0x1
	jr	$31
	addiu	$sp,$sp,24

$L53:
	move	$2,$0
	lw	$16,16($sp)
	jr	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	rtl_ip_range_filter_fastpath
	.size	rtl_ip_range_filter_fastpath, .-rtl_ip_range_filter_fastpath
	.align	2
	.globl	rtl_mac_filter_fastpath
	.set	nomips16
	.set	nomicromips
	.ent	rtl_mac_filter_fastpath
	.type	rtl_mac_filter_fastpath, @function
rtl_mac_filter_fastpath:
	.frame	$sp,40,$31		# vars= 0, regs= 6/0, args= 16, gp= 0
	.mask	0x801f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-40
	sw	$31,36($sp)
	sw	$20,32($sp)
	sw	$19,28($sp)
	sw	$18,24($sp)
	sw	$17,20($sp)
	sw	$16,16($sp)
	li	$16,6			# 0x6
	li	$20,12			# 0xc
	move	$19,$4
	move	$17,$5
	move	$4,$19
$L61:
	jal	rtl_skb_mac_header
	lbu	$18,0($17)

	addiu	$17,$17,1
	addu	$2,$2,$16
	lbu	$2,0($2)
	bne	$18,$2,$L58
	addiu	$16,$16,1

	bne	$16,$20,$L61
	move	$4,$19

	lw	$31,36($sp)
	li	$2,1			# 0x1
	lw	$20,32($sp)
	lw	$19,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	jr	$31
	addiu	$sp,$sp,40

$L58:
	lw	$31,36($sp)
	move	$2,$0
	lw	$20,32($sp)
	lw	$19,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	jr	$31
	addiu	$sp,$sp,40

	.set	macro
	.set	reorder
	.end	rtl_mac_filter_fastpath
	.size	rtl_mac_filter_fastpath, .-rtl_mac_filter_fastpath
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	rtl_schedule_filter_fastpath
	.type	rtl_schedule_filter_fastpath, @function
rtl_schedule_filter_fastpath:
	.frame	$sp,32,$31		# vars= 8, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$16,24($sp)
	addiu	$4,$sp,16
	jal	do_gettimeofday
	move	$16,$5

	li	$3,-1851654144			# 0xffffffff91a20000
	lw	$4,16($sp)
	li	$2,-1037172736			# 0xffffffffc22e0000
	ori	$3,$3,0xb3c5
	lw	$5,8($16)
	addiu	$2,$2,17671
	mult	$4,$3
	li	$6,-2004353024			# 0xffffffff88880000
	sra	$8,$4,31
	ori	$6,$6,0x8889
	mfhi	$3
	mult	$4,$2
	li	$7,715784192			# 0x2aaa0000
	ori	$7,$7,0xaaab
	mfhi	$2
	mult	$4,$6
	addu	$3,$3,$4
	sra	$3,$3,11
	mfhi	$9
	subu	$3,$3,$8
	addu	$2,$2,$4
	mult	$3,$7
	sra	$2,$2,16
	subu	$2,$2,$8
	addu	$9,$9,$4
	li	$4,-1840709632			# 0xffffffff92490000
	addiu	$2,$2,4
	addiu	$4,$4,9363
	mfhi	$7
	sra	$9,$9,5
	mult	$2,$4
	subu	$9,$9,$8
	sra	$8,$3,31
	sra	$10,$2,31
	mfhi	$4
	mult	$9,$6
	sra	$7,$7,2
	subu	$7,$7,$8
	mfhi	$6
	sll	$8,$7,1
	addu	$4,$4,$2
	addu	$7,$8,$7
	sra	$4,$4,2
	sra	$8,$9,31
	subu	$4,$4,$10
	addu	$6,$6,$9
	sll	$7,$7,3
	sra	$6,$6,5
	subu	$7,$3,$7
	subu	$6,$6,$8
	sll	$8,$4,3
	sll	$10,$6,4
	subu	$4,$8,$4
	sll	$3,$7,4
	subu	$8,$2,$4
	subu	$6,$10,$6
	li	$4,1			# 0x1
	subu	$3,$3,$7
	sll	$2,$6,2
	sll	$4,$4,$8
	sll	$3,$3,2
	subu	$2,$9,$2
	ori	$4,$4,0x80
	and	$5,$4,$5
	beq	$5,$0,$L68
	addu	$3,$3,$2

	lw	$2,0($16)
	bne	$2,$0,$L64
	sltu	$2,$3,$2

	lw	$2,4($16)
	beq	$2,$0,$L71
	sltu	$3,$2,$3

	xori	$2,$3,0x1
$L73:
	lw	$31,28($sp)
	lw	$16,24($sp)
	jr	$31
	addiu	$sp,$sp,32

$L64:
	beq	$2,$0,$L72
	nop

$L68:
	lw	$31,28($sp)
	move	$2,$0
	lw	$16,24($sp)
	jr	$31
	addiu	$sp,$sp,32

$L71:
	lw	$31,28($sp)
	li	$2,1			# 0x1
	lw	$16,24($sp)
	jr	$31
	addiu	$sp,$sp,32

$L72:
	lw	$2,4($16)
	sltu	$3,$2,$3
	b	$L73
	xori	$2,$3,0x1

	.set	macro
	.set	reorder
	.end	rtl_schedule_filter_fastpath
	.size	rtl_schedule_filter_fastpath, .-rtl_schedule_filter_fastpath
	.align	2
	.globl	filter_table_single_open
	.set	nomips16
	.set	nomicromips
	.ent	filter_table_single_open
	.type	filter_table_single_open, @function
filter_table_single_open:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	move	$4,$5
	lui	$5,%hi(filter_table_read_proc)
	move	$6,$0
	addiu	$5,$5,%lo(filter_table_read_proc)
	j	single_open
	nop

	.set	macro
	.set	reorder
	.end	filter_table_single_open
	.size	filter_table_single_open, .-filter_table_single_open
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	dos_single_open
	.type	dos_single_open, @function
dos_single_open:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	move	$4,$5
	lui	$5,%hi(dos_read_proc)
	move	$6,$0
	addiu	$5,$5,%lo(dos_read_proc)
	j	single_open
	nop

	.set	macro
	.set	reorder
	.end	dos_single_open
	.size	dos_single_open, .-dos_single_open
	.section	.rodata.str1.4,"aMS",@progbits,1
	.align	2
$LC0:
	.ascii	"pass\000"
	.align	2
$LC1:
	.ascii	"drop\000"
	.align	2
$LC2:
	.ascii	"Mode:%d %s\012\000"
	.align	2
$LC3:
	.ascii	"type=%d num=%d default policy=%s\012\000"
	.align	2
$LC4:
	.ascii	"type=%d num=%d\012\000"
	.align	2
$LC5:
	.ascii	"index=%d\011\000"
	.align	2
$LC6:
	.ascii	"mac=%02x:%02x:%02x:%02x:%02x:%02x\012\000"
	.align	2
$LC7:
	.ascii	"index=%d\011\011\000"
	.align	2
$LC8:
	.ascii	"url=%s    \000"
	.align	2
$LC9:
	.ascii	"user ip=%u.%u.%u.%u\012\000"
	.align	2
$LC10:
	.ascii	"user mac=%s\012\000"
	.align	2
$LC11:
	.ascii	"user=all\012\000"
	.align	2
$LC12:
	.ascii	"start=%x\011\000"
	.align	2
$LC13:
	.ascii	"end=%x\011\000"
	.align	2
$LC14:
	.ascii	"day=%x\012\000"
	.align	2
$LC15:
	.ascii	"unknow table type\012\000"
	.text
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	filter_table_read_proc
	.type	filter_table_read_proc, @function
filter_table_read_proc:
	.frame	$sp,104,$31		# vars= 32, regs= 10/0, args= 32, gp= 0
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
	lui	$16,%hi(kmalloc_caches)
	li	$18,37748736			# 0x2400000
	sw	$4,104($sp)
	addiu	$16,$16,%lo(kmalloc_caches)
	addiu	$5,$18,192
	jal	kmem_cache_alloc
	lw	$4,36($16)

	beq	$2,$0,$L77
	li	$6,512			# 0x200

	move	$5,$0
	move	$4,$2
	jal	memset
	move	$17,$2

	lw	$4,32($16)
	jal	kmem_cache_alloc
	ori	$5,$18,0xc0

	beq	$2,$0,$L111
	move	$16,$2

	li	$6,256			# 0x100
	move	$5,$0
	jal	memset
	move	$4,$2

	lui	$2,%hi(table_list_head)
	addiu	$3,$2,%lo(table_list_head)
	lw	$19,%lo(table_list_head)($2)
	lui	$2,%hi(url_filter_mode)
	sw	$3,36($sp)
	beq	$19,$3,$L97
	sw	$2,44($sp)

	lui	$2,%hi($LC4)
	sw	$2,40($sp)
	lui	$2,%hi(default_policy)
	sw	$2,48($sp)
	lui	$2,%hi($LC0)
	addiu	$2,$2,%lo($LC0)
	sw	$2,52($sp)
	lui	$2,%hi($LC1)
	addiu	$2,$2,%lo($LC1)
	sw	$2,56($sp)
	lui	$2,%hi($LC11)
	addiu	$2,$2,%lo($LC11)
	b	$L96
	sw	$2,32($sp)

$L113:
	lw	$2,44($sp)
	lw	$2,%lo(url_filter_mode)($2)
	beq	$2,$0,$L119
	lw	$2,40($sp)

	lw	$2,48($sp)
	lui	$5,%hi($LC3)
	lw	$4,52($sp)
	li	$6,3			# 0x3
	lw	$3,56($sp)
	addiu	$5,$5,%lo($LC3)
	addiu	$7,$7,-1
	lw	$2,%lo(default_policy)($2)
	movn	$3,$4,$2
	move	$4,$17
	jal	sprintf
	sw	$3,16($sp)

	li	$3,3			# 0x3
	lw	$2,16($19)
	beq	$2,$3,$L84
	li	$3,4			# 0x4

$L121:
	beq	$2,$3,$L85
	li	$3,2			# 0x2

	beq	$2,$3,$L112
	lui	$5,%hi($LC15)

	move	$4,$16
	jal	sprintf
	addiu	$5,$5,%lo($LC15)

	move	$5,$16
	jal	strcat
	move	$4,$17

$L88:
	lw	$19,0($19)
$L117:
	lw	$2,36($sp)
	beq	$19,$2,$L120
	lw	$2,44($sp)

$L96:
	lw	$6,16($19)
	li	$2,3			# 0x3
	beq	$6,$2,$L113
	lw	$7,24($19)

	lw	$2,40($sp)
$L119:
	addiu	$7,$7,-1
	move	$4,$17
	jal	sprintf
	addiu	$5,$2,%lo($LC4)

	li	$3,3			# 0x3
	lw	$2,16($19)
	bne	$2,$3,$L121
	li	$3,4			# 0x4

$L84:
	lw	$fp,8($19)
	addiu	$18,$19,8
	beq	$fp,$18,$L88
	lui	$22,%hi($LC10)

	lui	$21,%hi($LC7)
	lui	$20,%hi($LC8)
	addiu	$22,$22,%lo($LC10)
	b	$L94
	lui	$23,%hi($LC9)

$L93:
	jal	sprintf
	nop

	move	$5,$16
	jal	strcat
	move	$4,$17

	lw	$fp,0($fp)
$L116:
	beq	$fp,$18,$L88
	nop

$L94:
	lw	$6,20($fp)
	addiu	$5,$21,%lo($LC7)
	jal	sprintf
	move	$4,$16

	move	$5,$16
	jal	strcat
	move	$4,$17

	addiu	$6,$fp,48
	addiu	$5,$20,%lo($LC8)
	jal	sprintf
	move	$4,$16

	move	$5,$16
	jal	strcat
	move	$4,$17

	li	$3,73			# 0x49
	lb	$2,28($fp)
	li	$6,77			# 0x4d
	lw	$5,32($sp)
	beq	$2,$3,$L115
	move	$4,$16

	bne	$2,$6,$L93
	addiu	$6,$fp,29

	move	$5,$22
	jal	sprintf
	move	$4,$16

	move	$5,$16
	jal	strcat
	move	$4,$17

	b	$L116
	lw	$fp,0($fp)

$L112:
	lw	$22,8($19)
	addiu	$21,$19,8
	beq	$22,$21,$L88
	lui	$20,%hi($LC5)

	lui	$18,%hi($LC6)
	addiu	$20,$20,%lo($LC5)
	addiu	$18,$18,%lo($LC6)
$L89:
	lw	$6,20($22)
	move	$5,$20
	jal	sprintf
	move	$4,$16

	move	$5,$16
	jal	strcat
	move	$4,$17

	move	$5,$18
	lbu	$2,53($22)
	move	$4,$16
	lbu	$7,49($22)
	lbu	$6,48($22)
	sw	$2,28($sp)
	lbu	$2,52($22)
	sw	$2,24($sp)
	lbu	$2,51($22)
	sw	$2,20($sp)
	lbu	$2,50($22)
	jal	sprintf
	sw	$2,16($sp)

	move	$5,$16
	jal	strcat
	move	$4,$17

	lw	$22,0($22)
	bne	$22,$21,$L89
	lw	$2,36($sp)

	lw	$19,0($19)
	bne	$19,$2,$L96
	nop

$L97:
	lw	$2,44($sp)
$L120:
	lui	$5,%hi($LC2)
	lw	$4,104($sp)
	move	$7,$17
	addiu	$5,$5,%lo($LC2)
	jal	seq_printf
	lw	$6,%lo(url_filter_mode)($2)

	jal	kfree
	move	$4,$17

	jal	kfree
	move	$4,$16

$L77:
	lw	$31,100($sp)
$L118:
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

$L85:
	lw	$18,8($19)
	addiu	$fp,$19,8
	beq	$18,$fp,$L88
	lui	$20,%hi($LC5)

	lui	$23,%hi($LC12)
	lui	$22,%hi($LC13)
	lui	$21,%hi($LC14)
	addiu	$20,$20,%lo($LC5)
	addiu	$23,$23,%lo($LC12)
	addiu	$22,$22,%lo($LC13)
	addiu	$21,$21,%lo($LC14)
$L95:
	lw	$6,20($18)
	move	$5,$20
	jal	sprintf
	move	$4,$16

	move	$5,$16
	jal	strcat
	move	$4,$17

	move	$5,$23
	lw	$6,48($18)
	jal	sprintf
	move	$4,$16

	move	$5,$16
	jal	strcat
	move	$4,$17

	move	$5,$22
	lw	$6,52($18)
	jal	sprintf
	move	$4,$16

	move	$5,$16
	jal	strcat
	move	$4,$17

	move	$5,$21
	lw	$6,56($18)
	jal	sprintf
	move	$4,$16

	move	$5,$16
	jal	strcat
	move	$4,$17

	lw	$18,0($18)
	bne	$18,$fp,$L95
	nop

	b	$L117
	lw	$19,0($19)

$L115:
	lw	$6,44($fp)
	addiu	$5,$23,%lo($LC9)
	ext	$2,$6,8,8
	andi	$3,$6,0xff
	ext	$7,$6,16,8
	sw	$3,20($sp)
	srl	$6,$6,24
	jal	sprintf
	sw	$2,16($sp)

	move	$5,$16
	jal	strcat
	move	$4,$17

	b	$L116
	lw	$fp,0($fp)

$L111:
	jal	kfree
	move	$4,$17

	b	$L118
	lw	$31,100($sp)

	.set	macro
	.set	reorder
	.end	filter_table_read_proc
	.size	filter_table_read_proc, .-filter_table_read_proc
	.section	.rodata.str1.4
	.align	2
$LC16:
	.ascii	"br0\000"
	.align	2
$LC17:
	.ascii	"HTTP/1.1 401 The web site is blocked by administrator\015"
	.ascii	"\012Server: Embedded HTTP Server 2.00\015\012Connection:"
	.ascii	" close\015\012\015\012<HTML><HEAD><TITLE>401 The web sit"
	.ascii	"e is blocked by administrator</TITLE></HEAD>\012<BODY BG"
	.ascii	"COLOR=\"#ffffff\"><H4>401 The web site is blocked by adm"
	.ascii	"inistrator</H4></BODY></HTML>\012\000"
	.align	2
$LC18:
	.ascii	"dos_filter: alloc skb fail!\012\000"
	.text
	.align	2
	.globl	GenerateHTTP401
	.set	nomips16
	.set	nomicromips
	.ent	GenerateHTTP401
	.type	GenerateHTTP401, @function
GenerateHTTP401:
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
	move	$19,$4
	lui	$4,%hi($LC16)
	.set	noreorder
	.set	nomacro
	jal	rtl_get_dev_by_name
	addiu	$4,$4,%lo($LC16)
	.set	macro
	.set	reorder

	move	$4,$19
	.set	noreorder
	.set	nomacro
	jal	rtl_ip_hdr
	move	$16,$2
	.set	macro
	.set	reorder

	addiu	$4,$2,12
	.set	noreorder
	.set	nomacro
	jal	rtl_neigh_lookup
	move	$5,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L153
	move	$4,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_hh_from_neigh
	move	$17,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L170
	lui	$21,%hi(block_message)
	.set	macro
	.set	reorder

	lw	$4,%lo(block_message)($21)
	beq	$4,$0,$L171
$L166:
	jal	strlen
	li	$7,34078720			# 0x2080000
	li	$5,96			# 0x60
	addiu	$7,$7,32
	move	$6,$2
	.set	noreorder
	.set	nomacro
	jal	skb_copy_expand
	move	$4,$19
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L172
	move	$16,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_ip_hdr
	move	$4,$2
	.set	macro
	.set	reorder

	move	$4,$16
	lw	$5,0($2)
	move	$20,$2
	andi	$2,$5,0xf
	sll	$23,$2,2
	addu	$23,$20,$23
	lw	$5,12($23)
	ext	$5,$5,4,4
	addu	$5,$5,$2
	.set	noreorder
	.set	nomacro
	jal	skb_trim
	sll	$5,$5,2
	.set	macro
	.set	reorder

	lw	$4,%lo(block_message)($21)
	beq	$4,$0,$L173
$L167:
	jal	strlen
	move	$4,$16
	.set	noreorder
	.set	nomacro
	jal	skb_put
	move	$5,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_hh_from_neigh
	move	$4,$17
	.set	macro
	.set	reorder

	move	$18,$2
$L168:
	lw	$22,4($18)
	andi	$2,$22,0x1
	bne	$2,$0,$L155
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	move	$4,$16
	.set	macro
	.set	reorder

	move	$4,$18
	.set	noreorder
	.set	nomacro
	jal	rtl_get_data_from_hh
	move	$fp,$2
	.set	macro
	.set	reorder

	li	$6,16			# 0x10
	addiu	$4,$fp,-16
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$5,$2
	.set	macro
	.set	reorder

	lw	$2,4($18)
	bne	$2,$22,$L168
	.set	noreorder
	.set	nomacro
	jal	rtl_get_len_from_hh
	move	$4,$18
	.set	macro
	.set	reorder

	move	$4,$16
	.set	noreorder
	.set	nomacro
	jal	skb_push
	move	$5,$2
	.set	macro
	.set	reorder

	addiu	$2,$17,20
 #APP
 # 146 "./arch/mips/include/asm/atomic.h" 1
			

 # 0 "" 2
 #NO_APP
$L139:
 #APP
 # 149 "./arch/mips/include/asm/atomic.h" 1
		.set	mips64r2		
	ll	$4, 0($2)	# atomic_sub_return	
	subu $3, $4, 1			
	sc	$3, 0($2)				
	.set	mips0				

 # 0 "" 2
 #NO_APP
	beq	$3,$0,$L139
 #APP
 # 154 "./arch/mips/include/asm/atomic.h" 1
			

 # 0 "" 2
 #NO_APP
	li	$2,1			# 0x1
	beq	$4,$2,$L174
	move	$5,$0
$L180:
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_nfct
	move	$4,$16
	.set	macro
	.set	reorder

	lw	$18,0($20)
	lw	$3,12($20)
	lw	$2,16($20)
	andi	$18,$18,0xf
	lw	$21,%lo(block_message)($21)
	sll	$18,$18,2
	sw	$3,16($20)
	sw	$2,12($20)
	addu	$18,$20,$18
	lw	$4,12($18)
	lhu	$3,0($18)
	lhu	$2,2($18)
	ext	$4,$4,4,4
	sh	$3,2($18)
	sll	$4,$4,2
	sh	$2,0($18)
	.set	noreorder
	.set	nomacro
	beq	$21,$0,$L141
	addu	$17,$18,$4
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	strlen
	move	$4,$21
	.set	macro
	.set	reorder

	move	$5,$21
	addiu	$6,$2,1
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$4,$17
	.set	macro
	.set	reorder

	lw	$2,8($18)
$L179:
	move	$4,$19
	lw	$17,4($18)
	lui	$19,%hi(url_cache)
	sw	$2,4($18)
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_len
	addiu	$21,$19,%lo(url_cache)
	.set	macro
	.set	reorder

	wsbh	$17,$17
	lw	$5,12($23)
	li	$6,1			# 0x1
	lw	$4,0($20)
	ror	$17,$17,16
	sb	$0,13($18)
	ext	$5,$5,4,4
	lhu	$3,12($18)
	andi	$4,$4,0xf
	sh	$0,18($18)
	addu	$4,$4,$5
	lhu	$5,12($21)
	ins	$3,$0,10,1
	sll	$4,$4,2
	ins	$3,$6,12,1
	sh	$5,14($18)
	subu	$17,$17,$4
	ins	$3,$6,11,1
	addu	$2,$17,$2
	ins	$3,$0,8,1
	wsbh	$2,$2
	move	$4,$16
	sh	$3,12($18)
	ror	$2,$2,16
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_len
	sw	$2,8($18)
	.set	macro
	.set	reorder

	lw	$22,0($20)
	lw	$6,12($23)
	andi	$22,$22,0xf
	ext	$6,$6,4,4
	sll	$22,$22,2
	sll	$6,$6,2
	subu	$22,$2,$22
	slt	$2,$6,21
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L143
	addiu	$22,$22,-14
	.set	macro
	.set	reorder

	li	$17,20			# 0x14
	.set	noreorder
	.set	nomacro
	b	$L147
	li	$5,8			# 0x8
	.set	macro
	.set	reorder

$L144:
	.set	noreorder
	.set	nomacro
	beq	$2,$5,$L175
	lbu	$3,1($3)
	.set	macro
	.set	reorder

	addu	$17,$17,$3
	sltu	$2,$17,$6
	beq	$2,$0,$L143
$L147:
	addu	$3,$18,$17
$L182:
	lbu	$2,0($3)
	sltu	$4,$2,2
	beq	$4,$0,$L144
	addiu	$17,$17,1
	sltu	$2,$17,$6
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L182
	addu	$3,$18,$17
	.set	macro
	.set	reorder

$L143:
	sh	$0,16($18)
$L178:
	move	$5,$22
	lw	$17,16($20)
	andi	$22,$22,0xffff
	move	$6,$0
	lw	$23,12($20)
	addiu	$22,$22,6
	.set	noreorder
	.set	nomacro
	jal	csum_partial
	move	$4,$18
	.set	macro
	.set	reorder

	sll	$22,$22,8
	move	$3,$17
 #APP
 # 167 "./arch/mips/include/asm/checksum.h" 1
		.set	push		# csum_tcpudp_nofold
	.set	noat		
	addu	$3, $23		
	sltu	$1, $3, $23	
	addu	$3, $1		
	addu	$3, $22		
	sltu	$1, $3, $22	
	addu	$3, $1		
	addu	$3, $2		
	sltu	$1, $3, $2	
	addu	$3, $1		
	.set	pop
 # 0 "" 2
 #NO_APP
	sll	$2,$3,16
	move	$4,$16
	addu	$2,$2,$3
	srl	$5,$2,16
	sltu	$3,$2,$3
	addu	$3,$3,$5
	nor	$3,$0,$3
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_len
	sh	$3,16($18)
	.set	macro
	.set	reorder

	addiu	$4,$20,16
	lhu	$3,%lo(url_cache)($19)
	addiu	$2,$2,-14
	lbu	$5,14($21)
	wsbh	$2,$2
	sh	$0,6($20)
	addiu	$3,$3,2
	sh	$0,10($20)
	sh	$2,2($20)
	sh	$3,4($20)
	sb	$5,8($20)
	lw	$3,0($20)
	lw	$8,4($20)
	lw	$7,8($20)
	andi	$5,$3,0xf
	lw	$6,12($20)
	addu	$3,$3,$8
	sll	$5,$5,2
	addu	$2,$3,$7
	sltu	$3,$3,$8
	addu	$3,$2,$3
	addu	$5,$20,$5
	addu	$2,$3,$6
	sltu	$7,$3,$7
	addu	$2,$2,$7
	sltu	$6,$2,$6
	addu	$2,$2,$6
$L148:
	lw	$3,0($4)
	addiu	$4,$4,4
	addu	$2,$3,$2
	sltu	$3,$2,$3
	.set	noreorder
	.set	nomacro
	bne	$5,$4,$L148
	addu	$2,$2,$3
	.set	macro
	.set	reorder

	sll	$3,$2,16
	addu	$3,$3,$2
	srl	$4,$3,16
	sltu	$2,$3,$2
	addu	$2,$2,$4
	move	$4,$16
	nor	$2,$0,$2
	sh	$2,10($20)
	li	$2,1			# 0x1
	lhu	$5,2($18)
	lw	$3,8($18)
	sb	$2,15($21)
	lui	$2,%hi(finStateClientPort)
	sh	$5,%lo(finStateClientPort)($2)
	lui	$2,%hi(finStateClientSeq)
	.set	noreorder
	.set	nomacro
	jal	dev_queue_xmit
	sw	$3,%lo(finStateClientSeq)($2)
	.set	macro
	.set	reorder

$L153:
	lw	$31,60($sp)
$L177:
	move	$2,$0
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

$L170:
	addiu	$2,$17,20
 #APP
 # 146 "./arch/mips/include/asm/atomic.h" 1
			

 # 0 "" 2
 #NO_APP
$L127:
 #APP
 # 149 "./arch/mips/include/asm/atomic.h" 1
		.set	mips64r2		
	ll	$4, 0($2)	# atomic_sub_return	
	subu $3, $4, 1			
	sc	$3, 0($2)				
	.set	mips0				

 # 0 "" 2
 #NO_APP
	beq	$3,$0,$L127
 #APP
 # 154 "./arch/mips/include/asm/atomic.h" 1
			

 # 0 "" 2
 #NO_APP
	li	$2,1			# 0x1
	bne	$4,$2,$L153
	.set	noreorder
	.set	nomacro
	jal	neigh_destroy
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L177
	lw	$31,60($sp)
	.set	macro
	.set	reorder

$L172:
	addiu	$2,$17,20
 #APP
 # 146 "./arch/mips/include/asm/atomic.h" 1
			

 # 0 "" 2
 #NO_APP
$L132:
 #APP
 # 149 "./arch/mips/include/asm/atomic.h" 1
		.set	mips64r2		
	ll	$4, 0($2)	# atomic_sub_return	
	subu $3, $4, 1			
	sc	$3, 0($2)				
	.set	mips0				

 # 0 "" 2
 #NO_APP
	beq	$3,$0,$L132
 #APP
 # 154 "./arch/mips/include/asm/atomic.h" 1
			

 # 0 "" 2
 #NO_APP
	li	$2,1			# 0x1
	beq	$4,$2,$L176
	lui	$4,%hi($LC18)
$L181:
	.set	noreorder
	.set	nomacro
	jal	scrlog_printk
	addiu	$4,$4,%lo($LC18)
	.set	macro
	.set	reorder

	move	$2,$0
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

$L175:
	addiu	$23,$17,2
	addiu	$17,$17,6
	addu	$23,$18,$23
	addiu	$4,$sp,20
	li	$6,4			# 0x4
	move	$5,$23
	.set	noreorder
	.set	nomacro
	jal	memcpy
	addu	$17,$18,$17
	.set	macro
	.set	reorder

	addiu	$4,$sp,16
	li	$6,4			# 0x4
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$5,$17
	.set	macro
	.set	reorder

	addiu	$5,$sp,16
	li	$6,4			# 0x4
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$4,$23
	.set	macro
	.set	reorder

	li	$6,4			# 0x4
	addiu	$5,$sp,20
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L178
	sh	$0,16($18)
	.set	macro
	.set	reorder

$L171:
	lui	$4,%hi($LC17)
	.set	noreorder
	.set	nomacro
	b	$L166
	addiu	$4,$4,%lo($LC17)
	.set	macro
	.set	reorder

$L141:
	lui	$21,%hi($LC17)
	.set	noreorder
	.set	nomacro
	jal	strlen
	addiu	$4,$21,%lo($LC17)
	.set	macro
	.set	reorder

	addiu	$5,$21,%lo($LC17)
	move	$6,$2
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L179
	lw	$2,8($18)
	.set	macro
	.set	reorder

$L173:
	lui	$4,%hi($LC17)
	.set	noreorder
	.set	nomacro
	b	$L167
	addiu	$4,$4,%lo($LC17)
	.set	macro
	.set	reorder

$L174:
	.set	noreorder
	.set	nomacro
	jal	neigh_destroy
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L180
	move	$5,$0
	.set	macro
	.set	reorder

$L155:
	b	$L168
$L176:
	.set	noreorder
	.set	nomacro
	jal	neigh_destroy
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L181
	lui	$4,%hi($LC18)
	.set	macro
	.set	reorder

	.end	GenerateHTTP401
	.size	GenerateHTTP401, .-GenerateHTTP401
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	GenerateTCPENDACK
	.type	GenerateTCPENDACK, @function
GenerateTCPENDACK:
	.frame	$sp,64,$31		# vars= 8, regs= 9/0, args= 16, gp= 0
	.mask	0x80ff0000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-64
	sw	$31,60($sp)
	sw	$23,56($sp)
	sw	$22,52($sp)
	sw	$21,48($sp)
	sw	$20,44($sp)
	sw	$19,40($sp)
	sw	$18,36($sp)
	sw	$17,32($sp)
	sw	$16,28($sp)
	move	$16,$4
	lui	$4,%hi($LC16)
	.set	noreorder
	.set	nomacro
	jal	rtl_get_dev_by_name
	addiu	$4,$4,%lo($LC16)
	.set	macro
	.set	reorder

	move	$4,$16
	.set	noreorder
	.set	nomacro
	jal	rtl_ip_hdr
	move	$17,$2
	.set	macro
	.set	reorder

	lw	$3,0($2)
	lui	$4,%hi(endStateClientPort)
	andi	$3,$3,0xf
	lhu	$4,%lo(endStateClientPort)($4)
	sll	$3,$3,2
	addu	$3,$2,$3
	lhu	$5,0($3)
	beq	$5,$4,$L230
$L211:
	lw	$31,60($sp)
$L236:
	move	$2,$0
	lw	$23,56($sp)
	lw	$22,52($sp)
	lw	$21,48($sp)
	lw	$20,44($sp)
	lw	$19,40($sp)
	lw	$18,36($sp)
	lw	$17,32($sp)
	lw	$16,28($sp)
	.set	noreorder
	.set	nomacro
	jr	$31
	addiu	$sp,$sp,64
	.set	macro
	.set	reorder

$L230:
	lw	$4,4($3)
	lui	$3,%hi(endStateClientSeq)
	lw	$3,%lo(endStateClientSeq)($3)
	.set	noreorder
	.set	nomacro
	bne	$4,$3,$L236
	lw	$31,60($sp)
	.set	macro
	.set	reorder

	addiu	$4,$2,12
	.set	noreorder
	.set	nomacro
	jal	rtl_neigh_lookup
	move	$5,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L211
	move	$18,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_hh_from_neigh
	move	$4,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L231
	li	$7,34078720			# 0x2080000
	.set	macro
	.set	reorder

	move	$6,$0
	addiu	$7,$7,32
	li	$5,96			# 0x60
	.set	noreorder
	.set	nomacro
	jal	skb_copy_expand
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L232
	move	$17,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_ip_hdr
	move	$4,$2
	.set	macro
	.set	reorder

	move	$4,$17
	lw	$5,0($2)
	move	$20,$2
	andi	$2,$5,0xf
	sll	$16,$2,2
	addu	$16,$20,$16
	lw	$5,12($16)
	ext	$5,$5,4,4
	addu	$5,$5,$2
	.set	noreorder
	.set	nomacro
	jal	skb_trim
	sll	$5,$5,2
	.set	macro
	.set	reorder

	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	skb_put
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_hh_from_neigh
	move	$4,$18
	.set	macro
	.set	reorder

	move	$19,$2
$L228:
	lw	$21,4($19)
	andi	$2,$21,0x1
	bne	$2,$0,$L215
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	move	$4,$17
	.set	macro
	.set	reorder

	move	$4,$19
	.set	noreorder
	.set	nomacro
	jal	rtl_get_data_from_hh
	move	$22,$2
	.set	macro
	.set	reorder

	li	$6,16			# 0x10
	addiu	$4,$22,-16
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$5,$2
	.set	macro
	.set	reorder

	lw	$2,4($19)
	bne	$2,$21,$L228
	.set	noreorder
	.set	nomacro
	jal	rtl_get_len_from_hh
	move	$4,$19
	.set	macro
	.set	reorder

	move	$4,$17
	.set	noreorder
	.set	nomacro
	jal	skb_push
	move	$5,$2
	.set	macro
	.set	reorder

	addiu	$2,$18,20
 #APP
 # 146 "./arch/mips/include/asm/atomic.h" 1
			

 # 0 "" 2
 #NO_APP
$L196:
 #APP
 # 149 "./arch/mips/include/asm/atomic.h" 1
		.set	mips64r2		
	ll	$4, 0($2)	# atomic_sub_return	
	subu $3, $4, 1			
	sc	$3, 0($2)				
	.set	mips0				

 # 0 "" 2
 #NO_APP
	beq	$3,$0,$L196
 #APP
 # 154 "./arch/mips/include/asm/atomic.h" 1
			

 # 0 "" 2
 #NO_APP
	li	$2,1			# 0x1
	beq	$4,$2,$L233
	move	$5,$0
$L237:
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_nfct
	move	$4,$17
	.set	macro
	.set	reorder

	lw	$18,0($20)
	lw	$3,12($20)
	lw	$2,16($20)
	andi	$18,$18,0xf
	sll	$18,$18,2
	sw	$3,16($20)
	sw	$2,12($20)
	addu	$18,$20,$18
	lhu	$2,12($18)
	lhu	$5,0($18)
	lhu	$6,2($18)
	lw	$4,8($18)
	andi	$2,$2,0x100
	lw	$3,4($18)
	sh	$6,0($18)
	sh	$5,2($18)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L198
	sw	$4,4($18)
	.set	macro
	.set	reorder

	wsbh	$3,$3
	ror	$3,$3,16
	addiu	$3,$3,1
	wsbh	$3,$3
	ror	$3,$3,16
$L198:
	sb	$0,13($18)
	lui	$19,%hi(url_cache)
	lhu	$2,12($18)
	li	$5,1			# 0x1
	addiu	$21,$19,%lo(url_cache)
	sw	$3,8($18)
	sh	$0,18($18)
	move	$4,$17
	ins	$2,$0,10,1
	lhu	$3,12($21)
	ins	$2,$5,12,1
	ins	$2,$0,8,1
	sh	$3,14($18)
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_len
	sh	$2,12($18)
	.set	macro
	.set	reorder

	lw	$22,0($20)
	lw	$5,12($16)
	andi	$22,$22,0xf
	ext	$5,$5,4,4
	sll	$22,$22,2
	sll	$5,$5,2
	subu	$22,$2,$22
	slt	$2,$5,21
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L199
	addiu	$22,$22,-14
	.set	macro
	.set	reorder

	li	$16,20			# 0x14
	.set	noreorder
	.set	nomacro
	b	$L203
	li	$6,8			# 0x8
	.set	macro
	.set	reorder

$L200:
	.set	noreorder
	.set	nomacro
	beq	$2,$6,$L234
	lbu	$3,1($3)
	.set	macro
	.set	reorder

	addu	$16,$16,$3
$L201:
	sltu	$2,$16,$5
	beq	$2,$0,$L199
$L203:
	addu	$3,$18,$16
	lbu	$2,0($3)
	sltu	$4,$2,2
	beq	$4,$0,$L200
	.set	noreorder
	.set	nomacro
	b	$L201
	addiu	$16,$16,1
	.set	macro
	.set	reorder

$L231:
	addiu	$2,$18,20
 #APP
 # 146 "./arch/mips/include/asm/atomic.h" 1
			

 # 0 "" 2
 #NO_APP
$L189:
 #APP
 # 149 "./arch/mips/include/asm/atomic.h" 1
		.set	mips64r2		
	ll	$4, 0($2)	# atomic_sub_return	
	subu $3, $4, 1			
	sc	$3, 0($2)				
	.set	mips0				

 # 0 "" 2
 #NO_APP
	beq	$3,$0,$L189
 #APP
 # 154 "./arch/mips/include/asm/atomic.h" 1
			

 # 0 "" 2
 #NO_APP
	li	$2,1			# 0x1
	bne	$4,$2,$L211
	.set	noreorder
	.set	nomacro
	jal	neigh_destroy
	move	$4,$18
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L236
	lw	$31,60($sp)
	.set	macro
	.set	reorder

$L234:
	addiu	$23,$16,2
	addiu	$16,$16,6
	addu	$23,$18,$23
	li	$6,4			# 0x4
	addiu	$4,$sp,20
	move	$5,$23
	.set	noreorder
	.set	nomacro
	jal	memcpy
	addu	$16,$18,$16
	.set	macro
	.set	reorder

	li	$6,4			# 0x4
	addiu	$4,$sp,16
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$5,$16
	.set	macro
	.set	reorder

	li	$6,4			# 0x4
	addiu	$5,$sp,16
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$4,$23
	.set	macro
	.set	reorder

	li	$6,4			# 0x4
	addiu	$5,$sp,20
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$4,$16
	.set	macro
	.set	reorder

$L199:
	move	$6,$0
	sh	$0,16($18)
	move	$4,$18
	lw	$23,12($20)
	move	$5,$22
	.set	noreorder
	.set	nomacro
	jal	csum_partial
	lw	$16,16($20)
	.set	macro
	.set	reorder

	andi	$3,$22,0xffff
	addiu	$3,$3,6
	move	$5,$16
	sll	$3,$3,8
 #APP
 # 167 "./arch/mips/include/asm/checksum.h" 1
		.set	push		# csum_tcpudp_nofold
	.set	noat		
	addu	$5, $23		
	sltu	$1, $5, $23	
	addu	$5, $1		
	addu	$5, $3		
	sltu	$1, $5, $3	
	addu	$5, $1		
	addu	$5, $2		
	sltu	$1, $5, $2	
	addu	$5, $1		
	.set	pop
 # 0 "" 2
 #NO_APP
	sll	$2,$5,16
	move	$4,$17
	addu	$2,$2,$5
	srl	$3,$2,16
	sltu	$2,$2,$5
	addu	$2,$2,$3
	nor	$2,$0,$2
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_len
	sh	$2,16($18)
	.set	macro
	.set	reorder

	addiu	$4,$20,16
	lhu	$3,%lo(url_cache)($19)
	addiu	$2,$2,-14
	lbu	$5,14($21)
	wsbh	$2,$2
	sh	$0,6($20)
	addiu	$3,$3,1
	sh	$0,10($20)
	sh	$2,2($20)
	sh	$3,4($20)
	sb	$5,8($20)
	lw	$3,0($20)
	lw	$8,4($20)
	lw	$7,8($20)
	andi	$5,$3,0xf
	lw	$6,12($20)
	addu	$3,$3,$8
	sll	$5,$5,2
	addu	$2,$3,$7
	sltu	$3,$3,$8
	addu	$3,$2,$3
	addu	$5,$20,$5
	addu	$2,$3,$6
	sltu	$7,$3,$7
	addu	$2,$2,$7
	sltu	$6,$2,$6
	addu	$2,$2,$6
$L204:
	lw	$3,0($4)
	addiu	$4,$4,4
	addu	$2,$3,$2
	sltu	$3,$2,$3
	.set	noreorder
	.set	nomacro
	bne	$5,$4,$L204
	addu	$2,$2,$3
	.set	macro
	.set	reorder

	sll	$3,$2,16
	sb	$0,16($21)
	move	$4,$17
	addu	$3,$3,$2
	srl	$5,$3,16
	sltu	$2,$3,$2
	addu	$2,$2,$5
	nor	$2,$0,$2
	.set	noreorder
	.set	nomacro
	jal	dev_queue_xmit
	sh	$2,10($20)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L236
	lw	$31,60($sp)
	.set	macro
	.set	reorder

$L232:
	addiu	$2,$18,20
 #APP
 # 146 "./arch/mips/include/asm/atomic.h" 1
			

 # 0 "" 2
 #NO_APP
$L191:
 #APP
 # 149 "./arch/mips/include/asm/atomic.h" 1
		.set	mips64r2		
	ll	$4, 0($2)	# atomic_sub_return	
	subu $3, $4, 1			
	sc	$3, 0($2)				
	.set	mips0				

 # 0 "" 2
 #NO_APP
	beq	$3,$0,$L191
 #APP
 # 154 "./arch/mips/include/asm/atomic.h" 1
			

 # 0 "" 2
 #NO_APP
	li	$2,1			# 0x1
	beq	$4,$2,$L235
	lui	$4,%hi($LC18)
$L238:
	.set	noreorder
	.set	nomacro
	jal	scrlog_printk
	addiu	$4,$4,%lo($LC18)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L236
	lw	$31,60($sp)
	.set	macro
	.set	reorder

$L233:
	.set	noreorder
	.set	nomacro
	jal	neigh_destroy
	move	$4,$18
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L237
	move	$5,$0
	.set	macro
	.set	reorder

$L215:
	b	$L228
$L235:
	.set	noreorder
	.set	nomacro
	jal	neigh_destroy
	move	$4,$18
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L238
	lui	$4,%hi($LC18)
	.set	macro
	.set	reorder

	.end	GenerateTCPENDACK
	.size	GenerateTCPENDACK, .-GenerateTCPENDACK
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	GenerateTCPFINACK
	.type	GenerateTCPFINACK, @function
GenerateTCPFINACK:
	.frame	$sp,64,$31		# vars= 8, regs= 9/0, args= 16, gp= 0
	.mask	0x80ff0000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-64
	sw	$31,60($sp)
	sw	$23,56($sp)
	sw	$22,52($sp)
	sw	$21,48($sp)
	sw	$20,44($sp)
	sw	$19,40($sp)
	sw	$18,36($sp)
	sw	$17,32($sp)
	sw	$16,28($sp)
	move	$16,$4
	lui	$4,%hi($LC16)
	.set	noreorder
	.set	nomacro
	jal	rtl_get_dev_by_name
	addiu	$4,$4,%lo($LC16)
	.set	macro
	.set	reorder

	move	$4,$16
	.set	noreorder
	.set	nomacro
	jal	rtl_ip_hdr
	move	$17,$2
	.set	macro
	.set	reorder

	lw	$3,0($2)
	lui	$4,%hi(finStateClientPort)
	andi	$3,$3,0xf
	lhu	$4,%lo(finStateClientPort)($4)
	sll	$3,$3,2
	addu	$3,$2,$3
	lhu	$5,0($3)
	beq	$5,$4,$L282
$L265:
	lw	$31,60($sp)
$L288:
	move	$2,$0
	lw	$23,56($sp)
	lw	$22,52($sp)
	lw	$21,48($sp)
	lw	$20,44($sp)
	lw	$19,40($sp)
	lw	$18,36($sp)
	lw	$17,32($sp)
	lw	$16,28($sp)
	.set	noreorder
	.set	nomacro
	jr	$31
	addiu	$sp,$sp,64
	.set	macro
	.set	reorder

$L282:
	lw	$4,4($3)
	lui	$3,%hi(finStateClientSeq)
	lw	$3,%lo(finStateClientSeq)($3)
	.set	noreorder
	.set	nomacro
	bne	$4,$3,$L288
	lw	$31,60($sp)
	.set	macro
	.set	reorder

	addiu	$4,$2,12
	.set	noreorder
	.set	nomacro
	jal	rtl_neigh_lookup
	move	$5,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L265
	move	$18,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_hh_from_neigh
	move	$4,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L283
	li	$7,34078720			# 0x2080000
	.set	macro
	.set	reorder

	move	$6,$0
	addiu	$7,$7,32
	li	$5,96			# 0x60
	.set	noreorder
	.set	nomacro
	jal	skb_copy_expand
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L284
	move	$17,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_ip_hdr
	move	$4,$2
	.set	macro
	.set	reorder

	move	$4,$17
	lw	$5,0($2)
	move	$20,$2
	andi	$2,$5,0xf
	sll	$16,$2,2
	addu	$16,$20,$16
	lw	$5,12($16)
	ext	$5,$5,4,4
	addu	$5,$5,$2
	.set	noreorder
	.set	nomacro
	jal	skb_trim
	sll	$5,$5,2
	.set	macro
	.set	reorder

	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	skb_put
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_hh_from_neigh
	move	$4,$18
	.set	macro
	.set	reorder

	move	$19,$2
$L280:
	lw	$21,4($19)
	andi	$2,$21,0x1
	bne	$2,$0,$L268
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	move	$4,$17
	.set	macro
	.set	reorder

	move	$4,$19
	.set	noreorder
	.set	nomacro
	jal	rtl_get_data_from_hh
	move	$22,$2
	.set	macro
	.set	reorder

	li	$6,16			# 0x10
	addiu	$4,$22,-16
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$5,$2
	.set	macro
	.set	reorder

	lw	$2,4($19)
	bne	$2,$21,$L280
	.set	noreorder
	.set	nomacro
	jal	rtl_get_len_from_hh
	move	$4,$19
	.set	macro
	.set	reorder

	move	$4,$17
	.set	noreorder
	.set	nomacro
	jal	skb_push
	move	$5,$2
	.set	macro
	.set	reorder

	addiu	$2,$18,20
 #APP
 # 146 "./arch/mips/include/asm/atomic.h" 1
			

 # 0 "" 2
 #NO_APP
$L252:
 #APP
 # 149 "./arch/mips/include/asm/atomic.h" 1
		.set	mips64r2		
	ll	$4, 0($2)	# atomic_sub_return	
	subu $3, $4, 1			
	sc	$3, 0($2)				
	.set	mips0				

 # 0 "" 2
 #NO_APP
	beq	$3,$0,$L252
 #APP
 # 154 "./arch/mips/include/asm/atomic.h" 1
			

 # 0 "" 2
 #NO_APP
	li	$2,1			# 0x1
	beq	$4,$2,$L285
	move	$5,$0
$L289:
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_nfct
	move	$4,$17
	.set	macro
	.set	reorder

	lui	$19,%hi(url_cache)
	lw	$18,0($20)
	li	$3,1			# 0x1
	lw	$5,12($20)
	addiu	$21,$19,%lo(url_cache)
	lw	$2,16($20)
	move	$4,$17
	andi	$18,$18,0xf
	sll	$18,$18,2
	sw	$5,16($20)
	sw	$2,12($20)
	addu	$18,$20,$18
	lhu	$5,12($21)
	sb	$0,13($18)
	lhu	$2,12($18)
	lhu	$8,0($18)
	lhu	$9,2($18)
	ins	$2,$0,10,1
	lw	$6,4($18)
	lw	$7,8($18)
	ins	$2,$3,12,1
	sh	$5,14($18)
	sh	$9,0($18)
	ins	$2,$3,8,1
	sh	$8,2($18)
	sw	$7,4($18)
	sw	$6,8($18)
	sh	$0,18($18)
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_len
	sh	$2,12($18)
	.set	macro
	.set	reorder

	lw	$22,0($20)
	lw	$5,12($16)
	andi	$22,$22,0xf
	ext	$5,$5,4,4
	sll	$22,$22,2
	sll	$5,$5,2
	subu	$22,$2,$22
	slt	$2,$5,21
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L254
	addiu	$22,$22,-14
	.set	macro
	.set	reorder

	li	$16,20			# 0x14
	.set	noreorder
	.set	nomacro
	b	$L258
	li	$6,8			# 0x8
	.set	macro
	.set	reorder

$L255:
	.set	noreorder
	.set	nomacro
	beq	$2,$6,$L286
	lbu	$3,1($3)
	.set	macro
	.set	reorder

	addu	$16,$16,$3
$L256:
	sltu	$2,$16,$5
	beq	$2,$0,$L254
$L258:
	addu	$3,$18,$16
	lbu	$2,0($3)
	sltu	$4,$2,2
	beq	$4,$0,$L255
	.set	noreorder
	.set	nomacro
	b	$L256
	addiu	$16,$16,1
	.set	macro
	.set	reorder

$L283:
	addiu	$2,$18,20
 #APP
 # 146 "./arch/mips/include/asm/atomic.h" 1
			

 # 0 "" 2
 #NO_APP
$L245:
 #APP
 # 149 "./arch/mips/include/asm/atomic.h" 1
		.set	mips64r2		
	ll	$4, 0($2)	# atomic_sub_return	
	subu $3, $4, 1			
	sc	$3, 0($2)				
	.set	mips0				

 # 0 "" 2
 #NO_APP
	beq	$3,$0,$L245
 #APP
 # 154 "./arch/mips/include/asm/atomic.h" 1
			

 # 0 "" 2
 #NO_APP
	li	$2,1			# 0x1
	bne	$4,$2,$L265
	.set	noreorder
	.set	nomacro
	jal	neigh_destroy
	move	$4,$18
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L288
	lw	$31,60($sp)
	.set	macro
	.set	reorder

$L286:
	addiu	$23,$16,2
	addiu	$16,$16,6
	addu	$23,$18,$23
	li	$6,4			# 0x4
	addiu	$4,$sp,20
	move	$5,$23
	.set	noreorder
	.set	nomacro
	jal	memcpy
	addu	$16,$18,$16
	.set	macro
	.set	reorder

	li	$6,4			# 0x4
	addiu	$4,$sp,16
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$5,$16
	.set	macro
	.set	reorder

	li	$6,4			# 0x4
	addiu	$5,$sp,16
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$4,$23
	.set	macro
	.set	reorder

	li	$6,4			# 0x4
	addiu	$5,$sp,20
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$4,$16
	.set	macro
	.set	reorder

$L254:
	lhu	$5,2($18)
	lui	$2,%hi(endStateClientPort)
	lw	$3,8($18)
	move	$6,$0
	sh	$0,16($18)
	move	$4,$18
	sh	$5,%lo(endStateClientPort)($2)
	lui	$2,%hi(endStateClientSeq)
	move	$5,$22
	lw	$23,12($20)
	lw	$16,16($20)
	.set	noreorder
	.set	nomacro
	jal	csum_partial
	sw	$3,%lo(endStateClientSeq)($2)
	.set	macro
	.set	reorder

	andi	$4,$22,0xffff
	addiu	$4,$4,6
	move	$3,$16
	sll	$4,$4,8
 #APP
 # 167 "./arch/mips/include/asm/checksum.h" 1
		.set	push		# csum_tcpudp_nofold
	.set	noat		
	addu	$3, $23		
	sltu	$1, $3, $23	
	addu	$3, $1		
	addu	$3, $4		
	sltu	$1, $3, $4	
	addu	$3, $1		
	addu	$3, $2		
	sltu	$1, $3, $2	
	addu	$3, $1		
	.set	pop
 # 0 "" 2
 #NO_APP
	sll	$2,$3,16
	move	$4,$17
	addu	$2,$2,$3
	srl	$5,$2,16
	sltu	$2,$2,$3
	addu	$2,$2,$5
	nor	$2,$0,$2
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_len
	sh	$2,16($18)
	.set	macro
	.set	reorder

	addiu	$4,$20,16
	lhu	$3,%lo(url_cache)($19)
	addiu	$2,$2,-14
	lbu	$5,14($21)
	wsbh	$2,$2
	sh	$0,6($20)
	addiu	$3,$3,1
	sh	$0,10($20)
	sh	$2,2($20)
	sh	$3,4($20)
	sb	$5,8($20)
	lw	$3,0($20)
	lw	$6,4($20)
	lw	$8,8($20)
	andi	$5,$3,0xf
	lw	$7,12($20)
	addu	$3,$3,$6
	sll	$5,$5,2
	addu	$2,$3,$8
	sltu	$6,$3,$6
	addu	$3,$6,$2
	addu	$5,$20,$5
	addu	$2,$3,$7
	sltu	$3,$3,$8
	addu	$2,$2,$3
	sltu	$7,$2,$7
	addu	$2,$2,$7
$L259:
	lw	$3,0($4)
	addiu	$4,$4,4
	addu	$2,$3,$2
	sltu	$3,$2,$3
	.set	noreorder
	.set	nomacro
	bne	$5,$4,$L259
	addu	$2,$2,$3
	.set	macro
	.set	reorder

	sll	$3,$2,16
	sb	$0,15($21)
	li	$5,1			# 0x1
	addu	$3,$3,$2
	sb	$5,16($21)
	move	$4,$17
	srl	$5,$3,16
	sltu	$2,$3,$2
	addu	$2,$2,$5
	nor	$2,$0,$2
	.set	noreorder
	.set	nomacro
	jal	dev_queue_xmit
	sh	$2,10($20)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L288
	lw	$31,60($sp)
	.set	macro
	.set	reorder

$L284:
	addiu	$2,$18,20
 #APP
 # 146 "./arch/mips/include/asm/atomic.h" 1
			

 # 0 "" 2
 #NO_APP
$L247:
 #APP
 # 149 "./arch/mips/include/asm/atomic.h" 1
		.set	mips64r2		
	ll	$4, 0($2)	# atomic_sub_return	
	subu $3, $4, 1			
	sc	$3, 0($2)				
	.set	mips0				

 # 0 "" 2
 #NO_APP
	beq	$3,$0,$L247
 #APP
 # 154 "./arch/mips/include/asm/atomic.h" 1
			

 # 0 "" 2
 #NO_APP
	li	$2,1			# 0x1
	beq	$4,$2,$L287
	lui	$4,%hi($LC18)
$L290:
	.set	noreorder
	.set	nomacro
	jal	scrlog_printk
	addiu	$4,$4,%lo($LC18)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L288
	lw	$31,60($sp)
	.set	macro
	.set	reorder

$L285:
	.set	noreorder
	.set	nomacro
	jal	neigh_destroy
	move	$4,$18
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L289
	move	$5,$0
	.set	macro
	.set	reorder

$L268:
	b	$L280
$L287:
	.set	noreorder
	.set	nomacro
	jal	neigh_destroy
	move	$4,$18
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L290
	lui	$4,%hi($LC18)
	.set	macro
	.set	reorder

	.end	GenerateTCPFINACK
	.size	GenerateTCPFINACK, .-GenerateTCPFINACK
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	filter_dos
	.type	filter_dos, @function
filter_dos:
	.frame	$sp,32,$31		# vars= 0, regs= 3/0, args= 16, gp= 0
	.mask	0x80030000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$17,24($sp)
	sw	$16,20($sp)
	jal	rtl_ip_hdr
	move	$16,$4

	move	$4,$16
	jal	filter_wan_check
	move	$17,$2

	beq	$2,$0,$L304
	lw	$31,28($sp)

	lbu	$2,0($17)
	li	$3,96			# 0x60
	andi	$2,$2,0xf0
	beq	$2,$3,$L303
	lui	$2,%hi(dos_item_global)

	lw	$2,%lo(dos_item_global)($2)
	beq	$2,$0,$L305
	move	$2,$0

	lui	$2,%hi(filter_dos_ipv4_handler)
	lw	$2,%lo(filter_dos_ipv4_handler)($2)
	beq	$2,$0,$L306
	nop

$L302:
	jalr	$2
	move	$4,$16

	sltu	$2,$0,$2
	subu	$2,$0,$2
	lw	$31,28($sp)
$L304:
	lw	$17,24($sp)
	lw	$16,20($sp)
	jr	$31
	addiu	$sp,$sp,32

$L303:
	lui	$2,%hi(dos_item_ipv6)
	lw	$2,%lo(dos_item_ipv6)($2)
	beq	$2,$0,$L306
	lw	$31,28($sp)

	lui	$2,%hi(filter_dos_ipv6_handler)
	lw	$2,%lo(filter_dos_ipv6_handler)($2)
	bne	$2,$0,$L302
	nop

$L306:
	move	$2,$0
$L305:
	lw	$17,24($sp)
	lw	$16,20($sp)
	jr	$31
	addiu	$sp,$sp,32

	.set	macro
	.set	reorder
	.end	filter_dos
	.size	filter_dos, .-filter_dos
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	find_pattern.constprop.6
	.type	find_pattern.constprop.6, @function
find_pattern.constprop.6:
	.frame	$sp,64,$31		# vars= 8, regs= 10/0, args= 16, gp= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
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
	subu	$2,$5,$7
	move	$fp,$0
	sw	$2,16($sp)
	move	$17,$0
	lw	$2,84($sp)
	li	$21,13			# 0xd
	li	$23,10			# 0xa
	move	$19,$5
	move	$18,$7
	sw	$0,0($2)
	move	$16,$4
	lw	$2,80($sp)
	move	$22,$6
	move	$20,$4
	sw	$0,0($2)
$L313:
	lb	$2,0($20)
	beq	$2,$21,$L325
	nop

	beq	$2,$23,$L326
	andi	$2,$fp,0x1

$L320:
	move	$fp,$0
	move	$6,$18
$L329:
	move	$5,$22
	jal	memcmp
	move	$4,$20

	beq	$2,$0,$L312
	addu	$7,$18,$17

	lw	$2,16($sp)
	addiu	$17,$17,1
	sltu	$2,$2,$17
	beq	$2,$0,$L313
	addiu	$20,$20,1

$L314:
	move	$2,$0
$L327:
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

$L312:
	lw	$4,80($sp)
	li	$2,13			# 0xd
	addu	$3,$16,$7
	sw	$7,0($4)
	lb	$4,0($3)
	beq	$4,$2,$L321
	sltu	$2,$19,$7

	bne	$2,$0,$L314
	li	$6,13			# 0xd

	b	$L316
	move	$4,$0

$L317:
	sltu	$2,$19,$2
	bne	$2,$0,$L314
	nop

$L316:
	lb	$5,1($3)
	addiu	$4,$4,1
	addiu	$3,$3,1
	bne	$5,$6,$L317
	addu	$2,$7,$4

$L315:
	lw	$3,84($sp)
	li	$2,1			# 0x1
	sw	$4,0($3)
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

$L325:
	andi	$2,$fp,0x1
	bne	$2,$0,$L320
	addiu	$fp,$fp,1

	slt	$2,$fp,4
	bne	$2,$0,$L329
	move	$6,$18

	b	$L327
	move	$2,$0

$L326:
	beq	$2,$0,$L320
	addiu	$fp,$fp,1

	slt	$2,$fp,4
	bne	$2,$0,$L329
	move	$6,$18

	b	$L327
	move	$2,$0

$L321:
	b	$L315
	move	$4,$0

	.set	macro
	.set	reorder
	.end	find_pattern.constprop.6
	.size	find_pattern.constprop.6, .-find_pattern.constprop.6
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	scan_sch_url_tbl.part.0.constprop.7
	.type	scan_sch_url_tbl.part.0.constprop.7, @function
scan_sch_url_tbl.part.0.constprop.7:
	.frame	$sp,32,$31		# vars= 0, regs= 4/0, args= 16, gp= 0
	.mask	0x80070000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	lui	$2,%hi(url_filter_haystack-1)
	addiu	$2,$2,%lo(url_filter_haystack-1)
	addu	$2,$5,$2
	lb	$2,0($2)
	beq	$2,$0,$L342
	nop

	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$18,24($sp)
	sw	$17,20($sp)
	sw	$16,16($sp)
	lui	$16,%hi(url_filter_haystack)
	move	$18,$4
	addiu	$16,$16,%lo(url_filter_haystack)
	move	$17,$5
	addu	$16,$16,$5
	subu	$4,$16,$17
$L343:
	move	$6,$17
	move	$5,$18
	jal	memcmp
	addiu	$16,$16,1

	beq	$2,$0,$L335
	lw	$31,28($sp)

	lb	$2,-1($16)
	bne	$2,$0,$L343
	subu	$4,$16,$17

	lw	$31,28($sp)
	move	$2,$0
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	jr	$31
	addiu	$sp,$sp,32

$L335:
	li	$2,1			# 0x1
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	jr	$31
	addiu	$sp,$sp,32

$L342:
	jr	$31
	move	$2,$0

	.set	macro
	.set	reorder
	.end	scan_sch_url_tbl.part.0.constprop.7
	.size	scan_sch_url_tbl.part.0.constprop.7, .-scan_sch_url_tbl.part.0.constprop.7
	.section	.rodata.str1.4
	.align	2
$LC19:
	.ascii	"GET \000"
	.align	2
$LC20:
	.ascii	"Host: \000"
	.align	2
$LC21:
	.ascii	"%02X%02X%02X%02X%02X%02X\000"
	.align	2
$LC22:
	.ascii	"%s\000"
	.text
	.align	2
	.globl	rtl_url_filter_fastpath
	.set	nomips16
	.set	nomicromips
	.ent	rtl_url_filter_fastpath
	.type	rtl_url_filter_fastpath, @function
rtl_url_filter_fastpath:
	.frame	$sp,104,$31		# vars= 32, regs= 9/0, args= 32, gp= 0
	.mask	0x80ff0000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-104
	sw	$31,100($sp)
	sw	$23,96($sp)
	sw	$22,92($sp)
	sw	$21,88($sp)
	sw	$20,84($sp)
	sw	$19,80($sp)
	sw	$18,76($sp)
	sw	$17,72($sp)
	sw	$16,68($sp)
	move	$16,$4
	move	$4,$5
	sw	$0,32($sp)
	move	$19,$5
	sw	$0,36($sp)
	sw	$0,40($sp)
	.set	noreorder
	.set	nomacro
	jal	strlen
	sb	$0,44($sp)
	.set	macro
	.set	reorder

	sltu	$2,$2,2
	bne	$2,$0,$L347
	.set	noreorder
	.set	nomacro
	jal	rtl_ip_hdr
	move	$4,$16
	.set	macro
	.set	reorder

	move	$4,$16
	.set	noreorder
	.set	nomacro
	jal	rtl_get_tcp_header
	move	$20,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L347
	move	$17,$2
	.set	macro
	.set	reorder

	lw	$2,12($2)
	move	$4,$16
	.set	noreorder
	.set	nomacro
	jal	get_payload_len
	ext	$18,$2,4,4
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L432
	move	$21,$2
	.set	macro
	.set	reorder

	lhu	$3,2($17)
	sll	$2,$18,2
	li	$4,80			# 0x50
	addu	$17,$17,$2
	wsbh	$2,$3
	andi	$2,$2,0xffff
	.set	noreorder
	.set	nomacro
	beq	$2,$4,$L433
	li	$3,443			# 0x1bb
	.set	macro
	.set	reorder

	bne	$2,$3,$L432
	.set	noreorder
	.set	nomacro
	beq	$17,$0,$L432
	sltu	$2,$21,4
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L432
	li	$2,22			# 0x16
	.set	macro
	.set	reorder

	lbu	$3,0($17)
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L432
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

	lbu	$3,5($17)
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L432
	addiu	$21,$21,-20
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$21,$0,$L432
	addiu	$17,$17,11
	.set	macro
	.set	reorder

	li	$4,1			# 0x1
	.set	noreorder
	.set	nomacro
	b	$L359
	move	$2,$17
	.set	macro
	.set	reorder

$L385:
	sltu	$3,$4,$21
$L442:
	addiu	$2,$2,1
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L432
	addiu	$4,$4,1
	.set	macro
	.set	reorder

$L359:
	lbu	$3,0($2)
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L442
	sltu	$3,$4,$21
	.set	macro
	.set	reorder

	lbu	$3,1($2)
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L442
	sltu	$3,$4,$21
	.set	macro
	.set	reorder

	lbu	$3,2($2)
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L357
	lbu	$6,3($2)
	.set	macro
	.set	reorder

	beq	$6,$0,$L385
$L357:
	lbu	$5,4($2)
	sll	$3,$3,8
	lbu	$7,5($2)
	or	$3,$3,$6
	sll	$5,$5,8
	addiu	$3,$3,-2
	or	$5,$5,$7
	.set	noreorder
	.set	nomacro
	bne	$5,$3,$L442
	sltu	$3,$4,$21
	.set	macro
	.set	reorder

	lbu	$5,7($2)
	lbu	$23,8($2)
	sll	$5,$5,8
	or	$5,$5,$23
	beq	$5,$0,$L442
	sltu	$3,$21,$5
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L442
	sltu	$3,$4,$21
	.set	macro
	.set	reorder

	addiu	$3,$4,8
	addu	$6,$5,$3
	sltu	$7,$6,$21
	.set	noreorder
	.set	nomacro
	beq	$7,$0,$L385
	lui	$18,%hi(url_filter_haystack)
	.set	macro
	.set	reorder

	addu	$2,$17,$3
	addiu	$23,$18,%lo(url_filter_haystack)
	addu	$17,$17,$6
	move	$3,$23
$L358:
	lbu	$4,0($2)
	addiu	$3,$3,1
	addiu	$2,$2,1
	.set	noreorder
	.set	nomacro
	bne	$17,$2,$L358
	sb	$4,-1($3)
	.set	macro
	.set	reorder

	addu	$23,$23,$5
	.set	noreorder
	.set	nomacro
	b	$L353
	sb	$0,0($23)
	.set	macro
	.set	reorder

$L347:
	move	$16,$0
	lw	$31,100($sp)
	lw	$23,96($sp)
	move	$2,$16
	lw	$22,92($sp)
	lw	$21,88($sp)
	lw	$20,84($sp)
	lw	$19,80($sp)
	lw	$18,76($sp)
	lw	$17,72($sp)
	lw	$16,68($sp)
	.set	noreorder
	.set	nomacro
	jr	$31
	addiu	$sp,$sp,104
	.set	macro
	.set	reorder

$L433:
	sw	$0,56($sp)
	sw	$0,52($sp)
	.set	noreorder
	.set	nomacro
	beq	$17,$0,$L432
	sw	$0,48($sp)
	.set	macro
	.set	reorder

	lui	$22,%hi($LC19)
	li	$6,4			# 0x4
	addiu	$5,$22,%lo($LC19)
	.set	noreorder
	.set	nomacro
	jal	memcmp
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L434
	addiu	$2,$sp,52
	.set	macro
	.set	reorder

$L432:
	lui	$18,%hi(url_filter_haystack)
$L351:
	move	$16,$0
$L349:
	addiu	$4,$18,%lo(url_filter_haystack)
$L444:
	li	$6,2048			# 0x800
	.set	noreorder
	.set	nomacro
	jal	memset
	move	$5,$0
	.set	macro
	.set	reorder

	move	$2,$16
	lw	$31,100($sp)
	lw	$23,96($sp)
	lw	$22,92($sp)
	lw	$21,88($sp)
	lw	$20,84($sp)
	lw	$19,80($sp)
	lw	$18,76($sp)
	lw	$17,72($sp)
	lw	$16,68($sp)
	.set	noreorder
	.set	nomacro
	jr	$31
	addiu	$sp,$sp,104
	.set	macro
	.set	reorder

$L434:
	lui	$6,%hi($LC20)
	sw	$2,20($sp)
	addiu	$2,$sp,56
	li	$7,6			# 0x6
	addiu	$6,$6,%lo($LC20)
	sw	$2,16($sp)
	move	$5,$21
	.set	noreorder
	.set	nomacro
	jal	find_pattern.constprop.6
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L432
	lw	$2,52($sp)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L378
	lui	$18,%hi(url_filter_haystack)
	.set	macro
	.set	reorder

	lw	$3,56($sp)
	addiu	$23,$18,%lo(url_filter_haystack)
	move	$4,$23
	addu	$3,$17,$3
 #APP
 # 52 "./arch/mips/include/asm/string.h" 1
	.set	noreorder
	.set	noat
1:	lbu	$1,($3)
	subu	$2,1
	sb	$1,($4)
	beqz	$1,2f
	addiu	$4,1
	bnez	$2,1b
	addiu	$3,1
2:
	.set	at
	.set	reorder
 # 0 "" 2
 #NO_APP
	lw	$2,52($sp)
$L352:
	addiu	$3,$sp,56
	addu	$2,$23,$2
	sw	$3,16($sp)
	addiu	$3,$sp,48
	li	$7,4			# 0x4
	addiu	$6,$22,%lo($LC19)
	sw	$3,20($sp)
	move	$5,$21
	sb	$0,0($2)
	.set	noreorder
	.set	nomacro
	jal	find_pattern.constprop.6
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L351
	lw	$2,48($sp)
	.set	macro
	.set	reorder

	addiu	$2,$2,-9
	.set	noreorder
	.set	nomacro
	blez	$2,$L351
	sw	$2,48($sp)
	.set	macro
	.set	reorder

	lw	$3,52($sp)
	lw	$4,56($sp)
	addu	$3,$23,$3
	addu	$17,$17,$4
 #APP
 # 52 "./arch/mips/include/asm/string.h" 1
	.set	noreorder
	.set	noat
1:	lbu	$1,($17)
	subu	$2,1
	sb	$1,($3)
	beqz	$1,2f
	addiu	$3,1
	bnez	$2,1b
	addiu	$17,1
2:
	.set	at
	.set	reorder
 # 0 "" 2
 #NO_APP
	lw	$3,48($sp)
	lw	$2,52($sp)
	addu	$2,$2,$3
	addu	$23,$23,$2
	sb	$0,0($23)
$L353:
	.set	noreorder
	.set	nomacro
	jal	rtl_skb_mac_header
	move	$4,$16
	.set	macro
	.set	reorder

	lui	$5,%hi($LC21)
	lbu	$3,11($2)
	lui	$16,%hi(url_filter_mode)
	lbu	$7,7($2)
	addiu	$5,$5,%lo($LC21)
	lbu	$6,6($2)
	addiu	$4,$sp,32
	sw	$3,28($sp)
	lbu	$3,10($2)
	sw	$3,24($sp)
	lbu	$3,9($2)
	sw	$3,20($sp)
	lbu	$2,8($2)
	.set	noreorder
	.set	nomacro
	jal	sprintf
	sw	$2,16($sp)
	.set	macro
	.set	reorder

	lw	$2,%lo(url_filter_mode)($16)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L360
	lui	$2,%hi(table_list_head)
	.set	macro
	.set	reorder

	lw	$7,12($20)
	addiu	$9,$2,%lo(table_list_head)
	lw	$8,%lo(table_list_head)($2)
	wsbh	$7,$7
	.set	noreorder
	.set	nomacro
	beq	$8,$9,$L361
	ror	$7,$7,16
	.set	macro
	.set	reorder

	li	$10,3			# 0x3
	li	$5,73			# 0x49
	.set	noreorder
	.set	nomacro
	b	$L365
	move	$6,$8
	.set	macro
	.set	reorder

$L362:
	lw	$6,0($6)
$L438:
	beq	$6,$9,$L435
$L365:
	lw	$2,16($6)
	.set	noreorder
	.set	nomacro
	bne	$2,$10,$L362
	addiu	$4,$6,8
	.set	macro
	.set	reorder

	lw	$2,8($6)
	bne	$2,$4,$L364
	.set	noreorder
	.set	nomacro
	b	$L438
	lw	$6,0($6)
	.set	macro
	.set	reorder

$L363:
	lw	$2,0($2)
	beq	$2,$4,$L362
$L364:
	lb	$3,28($2)
	bne	$3,$5,$L363
	lw	$3,44($2)
	bne	$7,$3,$L363
$L360:
	lb	$2,-20($19)
$L441:
	li	$3,73			# 0x49
	.set	noreorder
	.set	nomacro
	beq	$2,$3,$L436
	li	$3,77			# 0x4d
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$3,$L373
	addiu	$2,$19,-19
	.set	macro
	.set	reorder

	addiu	$3,$sp,32
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
	beq	$4,$0,$L373
	lui	$6,%hi($LC22)
	.set	macro
	.set	reorder

$L443:
	lui	$4,%hi(log_info)
	addiu	$7,$18,%lo(url_filter_haystack)
	addiu	$6,$6,%lo($LC22)
	li	$5,32			# 0x20
	.set	noreorder
	.set	nomacro
	jal	snprintf
	addiu	$4,$4,%lo(log_info)
	.set	macro
	.set	reorder

	li	$2,1			# 0x1
	lw	$3,%lo(url_filter_mode)($16)
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L351
	lui	$2,%hi(url_filter_flag)
	.set	macro
	.set	reorder

$L439:
	li	$3,1			# 0x1
	move	$16,$0
	.set	noreorder
	.set	nomacro
	b	$L349
	sw	$3,%lo(url_filter_flag)($2)
	.set	macro
	.set	reorder

$L436:
	lw	$2,12($20)
	lw	$3,-4($19)
	wsbh	$2,$2
	ror	$2,$2,16
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L443
	lui	$6,%hi($LC22)
	.set	macro
	.set	reorder

$L373:
	.set	noreorder
	.set	nomacro
	jal	strlen
	addiu	$4,$18,%lo(url_filter_haystack)
	.set	macro
	.set	reorder

	move	$4,$19
	.set	noreorder
	.set	nomacro
	jal	strlen
	move	$17,$2
	.set	macro
	.set	reorder

	sltu	$17,$17,$2
	.set	noreorder
	.set	nomacro
	bne	$17,$0,$L443
	lui	$6,%hi($LC22)
	.set	macro
	.set	reorder

	move	$5,$2
	.set	noreorder
	.set	nomacro
	jal	scan_sch_url_tbl.part.0.constprop.7
	move	$4,$19
	.set	macro
	.set	reorder

	lui	$6,%hi($LC22)
	lui	$4,%hi(log_info)
	addiu	$7,$18,%lo(url_filter_haystack)
	addiu	$6,$6,%lo($LC22)
	li	$5,32			# 0x20
	addiu	$4,$4,%lo(log_info)
	.set	noreorder
	.set	nomacro
	jal	snprintf
	move	$17,$2
	.set	macro
	.set	reorder

	li	$2,1			# 0x1
	lw	$16,%lo(url_filter_mode)($16)
	bne	$16,$2,$L437
	.set	noreorder
	.set	nomacro
	bne	$17,$0,$L444
	addiu	$4,$18,%lo(url_filter_haystack)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L439
	lui	$2,%hi(url_filter_flag)
	.set	macro
	.set	reorder

$L435:
	li	$10,3			# 0x3
	.set	noreorder
	.set	nomacro
	b	$L369
	li	$6,77			# 0x4d
	.set	macro
	.set	reorder

$L366:
	lw	$8,0($8)
$L440:
	.set	noreorder
	.set	nomacro
	beq	$8,$9,$L445
	lui	$2,%hi(defaultRuleChanged)
	.set	macro
	.set	reorder

$L369:
	lw	$2,16($8)
	.set	noreorder
	.set	nomacro
	bne	$2,$10,$L366
	addiu	$5,$8,8
	.set	macro
	.set	reorder

	lw	$2,8($8)
	bne	$5,$2,$L368
	.set	noreorder
	.set	nomacro
	b	$L440
	lw	$8,0($8)
	.set	macro
	.set	reorder

$L367:
	lw	$2,0($2)
	beq	$5,$2,$L366
$L368:
	lb	$3,28($2)
	.set	noreorder
	.set	nomacro
	bne	$3,$6,$L367
	addiu	$3,$2,29
	.set	macro
	.set	reorder

	addiu	$4,$sp,32
 #APP
 # 77 "./arch/mips/include/asm/string.h" 1
	.set	noreorder
	.set	noat
	lbu	$7,($3)
1:	lbu	$1,($4)
	addiu	$3,1
	bne	$1,$7,2f
	addiu	$4,1
	bnez	$7,1b
	lbu	$7,($3)
	move	$7,$1
2:	subu	$7,$1
3:	.set	at
	.set	reorder
 # 0 "" 2
 #NO_APP
	bne	$7,$0,$L367
	.set	noreorder
	.set	nomacro
	b	$L441
	lb	$2,-20($19)
	.set	macro
	.set	reorder

$L378:
	move	$2,$0
	.set	noreorder
	.set	nomacro
	b	$L352
	addiu	$23,$18,%lo(url_filter_haystack)
	.set	macro
	.set	reorder

$L361:
	lui	$2,%hi(defaultRuleChanged)
$L445:
	lw	$2,%lo(defaultRuleChanged)($2)
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L360
	lui	$2,%hi(default_policy)
	.set	macro
	.set	reorder

	lw	$2,%lo(default_policy)($2)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L439
	lui	$2,%hi(url_filter_flag)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L349
	li	$16,1			# 0x1
	.set	macro
	.set	reorder

$L437:
	.set	noreorder
	.set	nomacro
	b	$L349
	sltu	$16,$0,$17
	.set	macro
	.set	reorder

	.end	rtl_url_filter_fastpath
	.size	rtl_url_filter_fastpath, .-rtl_url_filter_fastpath
	.section	.rodata.str1.4
	.align	2
$LC23:
	.ascii	"enable_dos\000"
	.align	2
$LC24:
	.ascii	"filter_table\000"
	.section	.exit.text,"ax",@progbits
	.align	2
	.globl	filter_exit
	.set	nomips16
	.set	nomicromips
	.ent	filter_exit
	.type	filter_exit, @function
filter_exit:
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$16,16($sp)
	lui	$16,%hi(res3)
	lw	$5,%lo(res3)($16)
	beq	$5,$0,$L447
	nop

	lui	$4,%hi($LC23)
	jal	remove_proc_entry
	addiu	$4,$4,%lo($LC23)

	sw	$0,%lo(res3)($16)
$L447:
	lui	$4,%hi(dos_timer)
	lui	$16,%hi(res12)
	jal	del_timer
	addiu	$4,$4,%lo(dos_timer)

	lw	$2,%lo(res12)($16)
	beq	$2,$0,$L446
	lui	$5,%hi(proc_root)

	lui	$4,%hi($LC24)
	addiu	$5,$5,%lo(proc_root)
	jal	remove_proc_entry
	addiu	$4,$4,%lo($LC24)

	sw	$0,%lo(res12)($16)
$L446:
	lw	$31,20($sp)
	lw	$16,16($sp)
	jr	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	filter_exit
	.size	filter_exit, .-filter_exit
	.section	.rodata.str1.4
	.align	2
$LC25:
	.ascii	"ppp0\000"
	.align	2
$LC26:
	.ascii	"wlan0\000"
	.text
	.align	2
	.globl	need_filter
	.set	nomips16
	.set	nomicromips
	.ent	need_filter
	.type	need_filter, @function
need_filter:
	.frame	$sp,32,$31		# vars= 0, regs= 3/0, args= 16, gp= 0
	.mask	0x80030000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$17,24($sp)
	sw	$16,20($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_ip_hdr
	lui	$17,%hi($LC16)
	.set	macro
	.set	reorder

	addiu	$4,$17,%lo($LC16)
	.set	noreorder
	.set	nomacro
	jal	rtl_get_dev_by_name
	move	$16,$2
	.set	macro
	.set	reorder

	beq	$2,$0,$L457
	lw	$2,440($2)
	beq	$2,$0,$L457
	lw	$4,12($2)
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L457
	addiu	$6,$17,%lo($LC16)
	.set	macro
	.set	reorder

	addiu	$2,$4,48
	move	$3,$6
 #APP
 # 77 "./arch/mips/include/asm/string.h" 1
	.set	noreorder
	.set	noat
	lbu	$5,($3)
1:	lbu	$1,($2)
	addiu	$3,1
	bne	$1,$5,2f
	addiu	$2,1
	bnez	$5,1b
	lbu	$5,($3)
	move	$5,$1
2:	subu	$5,$1
3:	.set	at
	.set	reorder
 # 0 "" 2
 #NO_APP
	bne	$5,$0,$L460
	.set	noreorder
	.set	nomacro
	b	$L528
	lw	$3,16($16)
	.set	macro
	.set	reorder

$L461:
 #APP
 # 77 "./arch/mips/include/asm/string.h" 1
	.set	noreorder
	.set	noat
	lbu	$5,($3)
1:	lbu	$1,($2)
	addiu	$3,1
	bne	$1,$5,2f
	addiu	$2,1
	bnez	$5,1b
	lbu	$5,($3)
	move	$5,$1
2:	subu	$5,$1
3:	.set	at
	.set	reorder
 # 0 "" 2
 #NO_APP
	beq	$5,$0,$L458
$L460:
	lw	$4,8($4)
	move	$3,$6
	.set	noreorder
	.set	nomacro
	bne	$4,$0,$L461
	addiu	$2,$4,48
	.set	macro
	.set	reorder

$L457:
	lui	$17,%hi($LC25)
$L531:
	.set	noreorder
	.set	nomacro
	jal	rtl_get_dev_by_name
	addiu	$4,$17,%lo($LC25)
	.set	macro
	.set	reorder

	beq	$2,$0,$L462
	lw	$2,440($2)
	beq	$2,$0,$L465
	lw	$4,12($2)
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L465
	addiu	$2,$4,48
	.set	macro
	.set	reorder

	addiu	$6,$17,%lo($LC25)
	move	$3,$6
 #APP
 # 77 "./arch/mips/include/asm/string.h" 1
	.set	noreorder
	.set	noat
	lbu	$5,($3)
1:	lbu	$1,($2)
	addiu	$3,1
	bne	$1,$5,2f
	addiu	$2,1
	bnez	$5,1b
	lbu	$5,($3)
	move	$5,$1
2:	subu	$5,$1
3:	.set	at
	.set	reorder
 # 0 "" 2
 #NO_APP
	bne	$5,$0,$L468
	.set	noreorder
	.set	nomacro
	b	$L529
	lw	$2,16($16)
	.set	macro
	.set	reorder

$L469:
 #APP
 # 77 "./arch/mips/include/asm/string.h" 1
	.set	noreorder
	.set	noat
	lbu	$5,($3)
1:	lbu	$1,($2)
	addiu	$3,1
	bne	$1,$5,2f
	addiu	$2,1
	bnez	$5,1b
	lbu	$5,($3)
	move	$5,$1
2:	subu	$5,$1
3:	.set	at
	.set	reorder
 # 0 "" 2
 #NO_APP
	beq	$5,$0,$L466
$L468:
	lw	$4,8($4)
	move	$3,$6
	.set	noreorder
	.set	nomacro
	bne	$4,$0,$L469
	addiu	$2,$4,48
	.set	macro
	.set	reorder

$L465:
	li	$2,1			# 0x1
$L527:
	lw	$31,28($sp)
$L532:
	lw	$17,24($sp)
	lw	$16,20($sp)
	.set	noreorder
	.set	nomacro
	jr	$31
	addiu	$sp,$sp,32
	.set	macro
	.set	reorder

$L458:
	lw	$3,16($16)
$L528:
	lw	$2,28($4)
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L531
	lui	$17,%hi($LC25)
	.set	macro
	.set	reorder

	lw	$31,28($sp)
	move	$2,$0
	lw	$17,24($sp)
	lw	$16,20($sp)
	.set	noreorder
	.set	nomacro
	jr	$31
	addiu	$sp,$sp,32
	.set	macro
	.set	reorder

$L466:
	lw	$2,16($16)
$L529:
	lw	$3,24($4)
	xor	$2,$2,$3
	sltu	$2,$0,$2
$L456:
	lw	$31,28($sp)
	lw	$17,24($sp)
	lw	$16,20($sp)
	.set	noreorder
	.set	nomacro
	jr	$31
	addiu	$sp,$sp,32
	.set	macro
	.set	reorder

$L462:
	jal	rtl_get_wan_dev
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L470
	lui	$17,%hi($LC26)
	.set	macro
	.set	reorder

	lw	$2,440($2)
	beq	$2,$0,$L465
	lw	$17,12($2)
	.set	noreorder
	.set	nomacro
	bne	$17,$0,$L472
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L532
	lw	$31,28($sp)
	.set	macro
	.set	reorder

$L526:
	lw	$17,8($17)
	.set	noreorder
	.set	nomacro
	beq	$17,$0,$L527
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

$L472:
	.set	noreorder
	.set	nomacro
	jal	rtl_strcmp_wan_dev_name
	addiu	$4,$17,48
	.set	macro
	.set	reorder

	bne	$2,$0,$L526
	lw	$2,16($16)
	lw	$3,28($17)
	xor	$2,$2,$3
	.set	noreorder
	.set	nomacro
	b	$L456
	sltu	$2,$0,$2
	.set	macro
	.set	reorder

$L470:
	.set	noreorder
	.set	nomacro
	jal	rtl_get_dev_by_name
	addiu	$4,$17,%lo($LC26)
	.set	macro
	.set	reorder

	beq	$2,$0,$L465
	lw	$2,440($2)
	beq	$2,$0,$L465
	lw	$3,12($2)
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L465
	addiu	$5,$17,%lo($LC26)
	.set	macro
	.set	reorder

	addiu	$2,$3,48
	move	$4,$5
 #APP
 # 77 "./arch/mips/include/asm/string.h" 1
	.set	noreorder
	.set	noat
	lbu	$6,($4)
1:	lbu	$1,($2)
	addiu	$4,1
	bne	$1,$6,2f
	addiu	$2,1
	bnez	$6,1b
	lbu	$6,($4)
	move	$6,$1
2:	subu	$6,$1
3:	.set	at
	.set	reorder
 # 0 "" 2
 #NO_APP
	bne	$6,$0,$L475
	.set	noreorder
	.set	nomacro
	b	$L530
	lw	$2,16($16)
	.set	macro
	.set	reorder

$L476:
 #APP
 # 77 "./arch/mips/include/asm/string.h" 1
	.set	noreorder
	.set	noat
	lbu	$6,($4)
1:	lbu	$1,($2)
	addiu	$4,1
	bne	$1,$6,2f
	addiu	$2,1
	bnez	$6,1b
	lbu	$6,($4)
	move	$6,$1
2:	subu	$6,$1
3:	.set	at
	.set	reorder
 # 0 "" 2
 #NO_APP
	beq	$6,$0,$L473
$L475:
	lw	$3,8($3)
	move	$4,$5
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L476
	addiu	$2,$3,48
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L527
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

$L473:
	lw	$2,16($16)
$L530:
	lw	$3,28($3)
	xor	$2,$2,$3
	.set	noreorder
	.set	nomacro
	b	$L456
	sltu	$2,$0,$2
	.set	macro
	.set	reorder

	.end	need_filter
	.size	need_filter, .-need_filter
	.align	2
	.globl	str2hexnum
	.set	nomips16
	.set	nomicromips
	.ent	str2hexnum
	.type	str2hexnum, @function
str2hexnum:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	lbu	$5,0($4)
	bne	$5,$0,$L537
	move	$2,$0

	b	$L543
	nop

$L542:
	addiu	$6,$5,-87
$L535:
	addiu	$4,$4,1
	lbu	$5,0($4)
	sll	$2,$2,4
	beq	$5,$0,$L543
	addu	$2,$6,$2

$L537:
	addiu	$6,$5,-48
	addiu	$3,$5,-97
	sltu	$7,$6,10
	bne	$7,$0,$L535
	sltu	$3,$3,6

	bne	$3,$0,$L542
	addiu	$7,$5,-65

	sltu	$7,$7,6
	bne	$7,$0,$L535
	addiu	$6,$5,-55

	jr	$31
	li	$2,-1			# 0xffffffffffffffff

$L543:
	jr	$31
	nop

	.set	macro
	.set	reorder
	.end	str2hexnum
	.size	str2hexnum, .-str2hexnum
	.align	2
	.globl	filter_table_head_init
	.set	nomips16
	.set	nomicromips
	.ent	filter_table_head_init
	.type	filter_table_head_init, @function
filter_table_head_init:
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$16,16($sp)
	lui	$16,%hi(table_list_head)
	li	$6,32			# 0x20
	addiu	$4,$16,%lo(table_list_head)
	jal	memset
	move	$5,$0

	lui	$4,%hi(table_list_head+8)
	addiu	$3,$16,%lo(table_list_head)
	lw	$31,20($sp)
	addiu	$4,$4,%lo(table_list_head+8)
	sw	$3,%lo(table_list_head)($16)
	move	$2,$0
	sw	$4,8($3)
	sw	$4,12($3)
	li	$4,1			# 0x1
	sw	$3,4($3)
	sw	$0,16($3)
	sw	$0,20($3)
	sw	$0,28($3)
	sw	$4,24($3)
	lw	$16,16($sp)
	jr	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	filter_table_head_init
	.size	filter_table_head_init, .-filter_table_head_init
	.align	2
	.globl	filter_table_regist
	.set	nomips16
	.set	nomicromips
	.ent	filter_table_regist
	.type	filter_table_regist, @function
filter_table_regist:
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
	lui	$17,%hi(table_list_head)
	move	$19,$4
	addiu	$16,$17,%lo(table_list_head)
	lw	$2,24($16)
	bne	$2,$0,$L547
	move	$18,$5

	jal	filter_table_head_init
	nop

$L547:
	lui	$2,%hi(kmalloc_caches+28)
	li	$5,37748736			# 0x2400000
	lw	$4,%lo(kmalloc_caches+28)($2)
	jal	kmem_cache_alloc
	addiu	$5,$5,192

	li	$5,1			# 0x1
	move	$3,$2
	addiu	$4,$2,8
	sw	$2,0($3)
	sw	$19,16($2)
	move	$2,$0
	sw	$4,8($3)
	sw	$4,12($3)
	sw	$0,20($3)
	sw	$5,24($3)
	sw	$18,28($3)
	lw	$4,%lo(table_list_head)($17)
	sw	$3,4($4)
	sw	$4,0($3)
	sw	$16,4($3)
	sw	$3,%lo(table_list_head)($17)
	lw	$4,24($16)
	lw	$31,36($sp)
	lw	$19,32($sp)
	addiu	$3,$4,1
	lw	$18,28($sp)
	lw	$17,24($sp)
	sw	$3,24($16)
	lw	$16,20($sp)
	jr	$31
	addiu	$sp,$sp,40

	.set	macro
	.set	reorder
	.end	filter_table_regist
	.size	filter_table_regist, .-filter_table_regist
	.align	2
	.globl	filter_table_flush
	.set	nomips16
	.set	nomicromips
	.ent	filter_table_flush
	.type	filter_table_flush, @function
filter_table_flush:
	.frame	$sp,64,$31		# vars= 8, regs= 10/0, args= 16, gp= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
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
	lui	$2,%hi(url_filter_mode)
	sw	$2,16($sp)
	lw	$2,%lo(url_filter_mode)($2)
	beq	$2,$0,$L569
	lui	$2,%hi(table_list_head)

	lui	$2,%hi(defaultRuleChanged)
	sw	$0,%lo(defaultRuleChanged)($2)
	lui	$2,%hi(table_list_head)
$L569:
	addiu	$20,$2,%lo(table_list_head)
	lw	$5,%lo(table_list_head)($2)
	beq	$5,$20,$L558
	lw	$19,0($5)

	li	$18,256			# 0x100
	li	$17,512			# 0x200
	li	$21,3			# 0x3
	li	$22,1			# 0x1
	move	$23,$5
$L557:
	lw	$2,8($23)
	addiu	$16,$23,8
	lw	$3,0($2)
	beq	$2,$16,$L556
	move	$fp,$3

$L555:
	lw	$6,4($2)
	move	$4,$2
	sw	$6,4($3)
	sw	$3,0($6)
	sw	$18,0($2)
	jal	kfree
	sw	$17,4($2)

	move	$4,$fp
	lw	$3,24($23)
	move	$2,$fp
	addiu	$3,$3,-1
	sw	$3,24($23)
	lw	$fp,0($fp)
	bne	$4,$16,$L555
	move	$3,$fp

$L556:
	lw	$2,16($23)
	beq	$2,$21,$L567
	nop

$L554:
	lw	$2,4($23)
$L568:
	move	$4,$23
	lw	$3,0($23)
	sw	$2,4($3)
	sw	$3,0($2)
	sw	$18,0($23)
	sw	$17,4($23)
	jal	kfree
	move	$23,$19

	lw	$2,24($20)
	addiu	$2,$2,-1
	sw	$2,24($20)
	bne	$23,$20,$L557
	lw	$19,0($19)

$L558:
	lw	$2,16($sp)
	li	$3,1			# 0x1
	lw	$31,60($sp)
	lw	$fp,56($sp)
	lw	$23,52($sp)
	sw	$0,%lo(url_filter_mode)($2)
	lui	$2,%hi(log_enable)
	lw	$22,48($sp)
	lw	$21,44($sp)
	lw	$20,40($sp)
	lw	$19,36($sp)
	lw	$18,32($sp)
	lw	$17,28($sp)
	lw	$16,24($sp)
	sw	$0,%lo(log_enable)($2)
	lui	$2,%hi(filter_rule_num)
	sw	$3,%lo(filter_rule_num)($2)
	move	$2,$0
	jr	$31
	addiu	$sp,$sp,64

$L567:
	lw	$2,24($23)
	bne	$2,$22,$L554
	nop

	jal	Del_Pattern_ACL_For_ContentFilter
	nop

	b	$L568
	lw	$2,4($23)

	.set	macro
	.set	reorder
	.end	filter_table_flush
	.size	filter_table_flush, .-filter_table_flush
	.align	2
	.globl	filter_item_regist
	.set	nomips16
	.set	nomicromips
	.ent	filter_item_regist
	.type	filter_item_regist, @function
filter_item_regist:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	lui	$2,%hi(table_list_head)
	addiu	$6,$2,%lo(table_list_head)
	lw	$2,%lo(table_list_head)($2)
	bne	$2,$6,$L575
	nop

	jr	$31
	li	$2,-1			# 0xffffffffffffffff

$L574:
	lw	$2,0($2)
	beq	$2,$6,$L576
	nop

$L575:
	lw	$3,16($2)
	bne	$3,$4,$L574
	addiu	$7,$2,8

	lw	$3,8($2)
	li	$6,3			# 0x3
	sw	$5,4($3)
	sw	$3,0($5)
	sw	$7,4($5)
	lw	$3,24($2)
	sw	$5,8($2)
	addiu	$3,$3,1
	beq	$4,$6,$L585
	sw	$3,24($2)

$L578:
	jr	$31
	move	$2,$0

$L576:
	li	$2,-1			# 0xffffffffffffffff
	jr	$31
	nop

$L585:
	li	$2,2			# 0x2
	bne	$3,$2,$L578
	nop

	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	jal	Add_Pattern_ACL_For_ContentFilter
	nop

	move	$2,$0
	lw	$31,20($sp)
	jr	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	filter_item_regist
	.size	filter_item_regist, .-filter_item_regist
	.align	2
	.globl	check_esc_end
	.set	nomips16
	.set	nomicromips
	.ent	check_esc_end
	.type	check_esc_end, @function
check_esc_end:
	.frame	$sp,32,$31		# vars= 0, regs= 3/0, args= 16, gp= 0
	.mask	0x80030000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$17,24($sp)
	sw	$16,20($sp)
	move	$16,$4
	jal	strlen
	move	$17,$5

	addiu	$3,$2,-1
	bltz	$3,$L594
	nop

	addu	$3,$16,$3
	lb	$3,0($3)
	bne	$3,$17,$L594
	nop

	addu	$4,$16,$2
	b	$L590
	move	$3,$0

$L592:
	lb	$6,-1($4)
	bne	$6,$17,$L597
	lw	$31,28($sp)

$L590:
	addiu	$3,$3,1
	bne	$2,$3,$L592
	addiu	$4,$4,-1

	andi	$2,$2,0x1
$L587:
	lw	$31,28($sp)
	lw	$17,24($sp)
	lw	$16,20($sp)
	jr	$31
	addiu	$sp,$sp,32

$L597:
	andi	$2,$3,0x1
	lw	$17,24($sp)
	lw	$16,20($sp)
	jr	$31
	addiu	$sp,$sp,32

$L594:
	b	$L587
	move	$2,$0

	.set	macro
	.set	reorder
	.end	check_esc_end
	.size	check_esc_end, .-check_esc_end
	.section	.rodata.str1.4
	.align	2
$LC27:
	.ascii	"net/rtl/fastpath/core/filter_v2.c\000"
	.align	2
$LC28:
	.ascii	"init\000"
	.align	2
$LC29:
	.ascii	" \000"
	.align	2
$LC30:
	.ascii	"flush\000"
	.align	2
$LC31:
	.ascii	"enable_log\000"
	.align	2
$LC32:
	.ascii	"white\000"
	.align	2
$LC33:
	.ascii	"black\000"
	.align	2
$LC34:
	.ascii	"add:\000"
	.align	2
$LC35:
	.ascii	"#\000"
	.align	2
$LC36:
	.ascii	";\000"
	.text
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	filter_table_single_write
	.type	filter_table_single_write, @function
filter_table_single_write:
	.frame	$sp,96,$31		# vars= 40, regs= 10/0, args= 16, gp= 0
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
	lui	$20,%hi(kmalloc_caches)
	move	$18,$5
	addiu	$20,$20,%lo(kmalloc_caches)
	li	$5,37748736			# 0x2400000
	move	$16,$6
	lw	$4,40($20)
	.set	noreorder
	.set	nomacro
	jal	kmem_cache_alloc
	addiu	$5,$5,192
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L658
	move	$5,$0
	.set	macro
	.set	reorder

	li	$6,1024			# 0x400
	lui	$17,%hi(get_info_from_usr_space)
	move	$4,$2
	.set	noreorder
	.set	nomacro
	jal	memset
	move	$19,$2
	.set	macro
	.set	reorder

	addiu	$4,$17,%lo(get_info_from_usr_space)
	.set	noreorder
	.set	nomacro
	jal	strlen
	sw	$19,16($sp)
	.set	macro
	.set	reorder

	move	$5,$0
	addiu	$4,$17,%lo(get_info_from_usr_space)
	.set	noreorder
	.set	nomacro
	jal	memset
	move	$6,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$18,$0,$L600
	addu	$2,$16,$18
	.set	macro
	.set	reorder

	lw	$3,24($28)
	or	$4,$16,$18
	or	$2,$2,$4
	and	$2,$2,$3
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L601
	lui	$4,%hi($LC27)
	.set	macro
	.set	reorder

	li	$5,820			# 0x334
	.set	noreorder
	.set	nomacro
	jal	__might_fault
	addiu	$4,$4,%lo($LC27)
	.set	macro
	.set	reorder

	addiu	$4,$17,%lo(get_info_from_usr_space)
	move	$5,$18
	move	$6,$16
 #APP
 # 830 "net/rtl/fastpath/core/filter_v2.c" 1
	.set	noreorder
	jal	__copy_user
	.set	noat
	addu	$1, $5, $6
	.set	at
	.set	reorder
 # 0 "" 2
 #NO_APP
$L602:
	bne	$6,$0,$L600
	.set	noreorder
	.set	nomacro
	bne	$16,$0,$L714
	addiu	$2,$17,%lo(get_info_from_usr_space)
	.set	macro
	.set	reorder

	addiu	$4,$17,%lo(get_info_from_usr_space)
$L722:
	.set	noreorder
	.set	nomacro
	jal	strlen
	lui	$17,%hi($LC28)
	.set	macro
	.set	reorder

	addu	$2,$19,$2
	addiu	$4,$17,%lo($LC28)
	sb	$0,-1($2)
	.set	noreorder
	.set	nomacro
	jal	strlen
	lw	$18,16($sp)
	.set	macro
	.set	reorder

	addiu	$5,$17,%lo($LC28)
	move	$6,$2
	.set	noreorder
	.set	nomacro
	jal	memcmp
	move	$4,$18
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L604
	lui	$17,%hi($LC30)
	.set	macro
	.set	reorder

	lui	$17,%hi($LC29)
	addiu	$18,$sp,16
	addiu	$5,$17,%lo($LC29)
	.set	noreorder
	.set	nomacro
	jal	strsep
	move	$4,$18
	.set	macro
	.set	reorder

	lw	$2,16($sp)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L600
	lui	$20,%hi(filter_tlb)
	.set	macro
	.set	reorder

	addiu	$17,$17,%lo($LC29)
	addiu	$20,$20,%lo(filter_tlb)
	move	$5,$17
$L723:
	.set	noreorder
	.set	nomacro
	jal	strsep
	move	$4,$18
	.set	macro
	.set	reorder

	lbu	$5,0($2)
	.set	noreorder
	.set	nomacro
	bne	$5,$0,$L608
	move	$4,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L721
	sll	$2,$4,3
	.set	macro
	.set	reorder

$L715:
	addiu	$6,$5,-87
$L606:
	addiu	$2,$2,1
	lbu	$5,0($2)
	sll	$4,$4,4
	.set	noreorder
	.set	nomacro
	beq	$5,$0,$L605
	addu	$4,$4,$6
	.set	macro
	.set	reorder

$L608:
	addiu	$6,$5,-48
	addiu	$3,$5,-97
	sltu	$7,$6,10
	.set	noreorder
	.set	nomacro
	bne	$7,$0,$L606
	sltu	$3,$3,6
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L715
	addiu	$7,$5,-65
	.set	macro
	.set	reorder

	sltu	$7,$7,6
	.set	noreorder
	.set	nomacro
	bne	$7,$0,$L606
	addiu	$6,$5,-55
	.set	macro
	.set	reorder

	li	$4,-1			# 0xffffffffffffffff
$L605:
	sll	$2,$4,3
$L721:
	addu	$2,$20,$2
	.set	noreorder
	.set	nomacro
	jal	filter_table_regist
	lw	$5,4($2)
	.set	macro
	.set	reorder

	lw	$2,16($sp)
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L723
	move	$5,$17
	.set	macro
	.set	reorder

	b	$L610
$L604:
	lw	$18,16($sp)
	.set	noreorder
	.set	nomacro
	jal	strlen
	addiu	$4,$17,%lo($LC30)
	.set	macro
	.set	reorder

	addiu	$5,$17,%lo($LC30)
	move	$6,$2
	.set	noreorder
	.set	nomacro
	jal	memcmp
	move	$4,$18
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L611
	lui	$17,%hi($LC31)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	filter_table_flush
	move	$4,$0
	.set	macro
	.set	reorder

$L610:
	.set	noreorder
	.set	nomacro
	jal	kfree
	move	$4,$19
	.set	macro
	.set	reorder

	move	$2,$16
$L598:
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

$L714:
	move	$3,$19
	move	$4,$16
 #APP
 # 52 "./arch/mips/include/asm/string.h" 1
	.set	noreorder
	.set	noat
1:	lbu	$1,($2)
	subu	$4,1
	sb	$1,($3)
	beqz	$1,2f
	addiu	$3,1
	bnez	$4,1b
	addiu	$2,1
2:
	.set	at
	.set	reorder
 # 0 "" 2
 #NO_APP
	.set	noreorder
	.set	nomacro
	b	$L722
	addiu	$4,$17,%lo(get_info_from_usr_space)
	.set	macro
	.set	reorder

$L620:
	addiu	$6,$3,-55
	sltu	$3,$7,6
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L724
	addiu	$2,$2,1
	.set	macro
	.set	reorder

$L600:
	.set	noreorder
	.set	nomacro
	jal	kfree
	move	$4,$19
	.set	macro
	.set	reorder

	li	$2,-14			# 0xfffffffffffffff2
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

$L611:
	lw	$18,16($sp)
	.set	noreorder
	.set	nomacro
	jal	strlen
	addiu	$4,$17,%lo($LC31)
	.set	macro
	.set	reorder

	addiu	$5,$17,%lo($LC31)
	move	$6,$2
	.set	noreorder
	.set	nomacro
	jal	memcmp
	move	$4,$18
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L612
	lui	$2,%hi(log_enable)
	.set	macro
	.set	reorder

	li	$3,1			# 0x1
	.set	noreorder
	.set	nomacro
	b	$L610
	sw	$3,%lo(log_enable)($2)
	.set	macro
	.set	reorder

$L612:
	lui	$17,%hi($LC32)
	lw	$18,16($sp)
	.set	noreorder
	.set	nomacro
	jal	strlen
	addiu	$4,$17,%lo($LC32)
	.set	macro
	.set	reorder

	addiu	$5,$17,%lo($LC32)
	move	$6,$2
	.set	noreorder
	.set	nomacro
	jal	memcmp
	move	$4,$18
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L613
	lui	$2,%hi(url_filter_mode)
	.set	macro
	.set	reorder

	li	$3,1			# 0x1
	.set	noreorder
	.set	nomacro
	b	$L610
	sw	$3,%lo(url_filter_mode)($2)
	.set	macro
	.set	reorder

$L601:
	move	$5,$0
	addiu	$4,$17,%lo(get_info_from_usr_space)
	.set	noreorder
	.set	nomacro
	jal	memset
	move	$6,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L602
	move	$6,$16
	.set	macro
	.set	reorder

$L613:
	lui	$17,%hi($LC33)
	lw	$18,16($sp)
	.set	noreorder
	.set	nomacro
	jal	strlen
	addiu	$4,$17,%lo($LC33)
	.set	macro
	.set	reorder

	addiu	$5,$17,%lo($LC33)
	move	$6,$2
	.set	noreorder
	.set	nomacro
	jal	memcmp
	move	$4,$18
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L614
	lui	$2,%hi(url_filter_mode)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L610
	sw	$0,%lo(url_filter_mode)($2)
	.set	macro
	.set	reorder

$L658:
	.set	noreorder
	.set	nomacro
	b	$L598
	li	$2,-14			# 0xfffffffffffffff2
	.set	macro
	.set	reorder

$L614:
	lui	$17,%hi($LC1)
	lw	$18,16($sp)
	.set	noreorder
	.set	nomacro
	jal	strlen
	addiu	$4,$17,%lo($LC1)
	.set	macro
	.set	reorder

	addiu	$5,$17,%lo($LC1)
	move	$6,$2
	.set	noreorder
	.set	nomacro
	jal	memcmp
	move	$4,$18
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L615
	lui	$2,%hi(default_policy)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L610
	sw	$0,%lo(default_policy)($2)
	.set	macro
	.set	reorder

$L615:
	lui	$17,%hi($LC0)
	lw	$18,16($sp)
	.set	noreorder
	.set	nomacro
	jal	strlen
	addiu	$4,$17,%lo($LC0)
	.set	macro
	.set	reorder

	addiu	$5,$17,%lo($LC0)
	move	$6,$2
	.set	noreorder
	.set	nomacro
	jal	memcmp
	move	$4,$18
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L616
	lui	$2,%hi(default_policy)
	.set	macro
	.set	reorder

	li	$3,1			# 0x1
	.set	noreorder
	.set	nomacro
	b	$L610
	sw	$3,%lo(default_policy)($2)
	.set	macro
	.set	reorder

$L616:
	lui	$17,%hi($LC34)
	lw	$18,16($sp)
	.set	noreorder
	.set	nomacro
	jal	strlen
	addiu	$4,$17,%lo($LC34)
	.set	macro
	.set	reorder

	addiu	$5,$17,%lo($LC34)
	move	$6,$2
	.set	noreorder
	.set	nomacro
	jal	memcmp
	move	$4,$18
	.set	macro
	.set	reorder

	bne	$2,$0,$L610
	.set	noreorder
	.set	nomacro
	jal	strlen
	addiu	$4,$17,%lo($LC34)
	.set	macro
	.set	reorder

	lw	$3,16($sp)
	addu	$2,$3,$2
	sw	$2,16($sp)
	lb	$2,0($2)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L600
	lui	$5,%hi($LC35)
	.set	macro
	.set	reorder

	addiu	$18,$sp,16
	addiu	$5,$5,%lo($LC35)
	.set	noreorder
	.set	nomacro
	jal	strsep
	move	$4,$18
	.set	macro
	.set	reorder

	lw	$3,16($sp)
	beq	$3,$0,$L600
	lbu	$3,0($2)
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L618
	move	$4,$0
	.set	macro
	.set	reorder

$L621:
	addiu	$6,$3,-48
	addiu	$5,$3,-97
	sltu	$7,$6,10
	.set	noreorder
	.set	nomacro
	bne	$7,$0,$L619
	sltu	$5,$5,6
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$5,$0,$L620
	addiu	$7,$3,-65
	.set	macro
	.set	reorder

	addiu	$6,$3,-87
$L619:
	addiu	$2,$2,1
$L724:
	lbu	$3,0($2)
	sll	$4,$4,4
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L621
	addu	$4,$4,$6
	.set	macro
	.set	reorder

	li	$2,-1			# 0xffffffffffffffff
	beq	$4,$2,$L600
$L618:
	lui	$17,%hi($LC29)
	move	$4,$18
	.set	noreorder
	.set	nomacro
	jal	strsep
	addiu	$5,$17,%lo($LC29)
	.set	macro
	.set	reorder

	lw	$3,16($sp)
	beq	$3,$0,$L600
	lbu	$3,0($2)
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L622
	move	$4,$0
	.set	macro
	.set	reorder

$L625:
	addiu	$6,$3,-48
	addiu	$5,$3,-97
	sltu	$7,$6,10
	.set	noreorder
	.set	nomacro
	bne	$7,$0,$L623
	sltu	$5,$5,6
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$5,$0,$L624
	addiu	$7,$3,-65
	.set	macro
	.set	reorder

	addiu	$6,$3,-87
$L623:
	addiu	$2,$2,1
$L726:
	lbu	$3,0($2)
	sll	$4,$4,4
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L625
	addu	$4,$4,$6
	.set	macro
	.set	reorder

	li	$2,-1			# 0xffffffffffffffff
	beq	$4,$2,$L600
$L622:
	addiu	$5,$17,%lo($LC29)
	.set	noreorder
	.set	nomacro
	jal	strsep
	move	$4,$18
	.set	macro
	.set	reorder

	lw	$3,16($sp)
	beq	$3,$0,$L600
	lbu	$3,0($2)
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L626
	move	$22,$0
	.set	macro
	.set	reorder

$L629:
	addiu	$5,$3,-48
	addiu	$4,$3,-97
	sltu	$6,$5,10
	.set	noreorder
	.set	nomacro
	bne	$6,$0,$L627
	sltu	$4,$4,6
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L628
	addiu	$6,$3,-65
	.set	macro
	.set	reorder

	addiu	$5,$3,-87
$L627:
	addiu	$2,$2,1
$L725:
	lbu	$3,0($2)
	sll	$22,$22,4
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L629
	addu	$22,$22,$5
	.set	macro
	.set	reorder

	li	$2,-1			# 0xffffffffffffffff
	beq	$22,$2,$L600
$L626:
	lui	$2,%hi($LC36)
	lui	$fp,%hi(filter_rule_num)
	addiu	$2,$2,%lo($LC36)
	sw	$2,28($sp)
	lui	$2,%hi(defaultRuleChanged)
	sw	$2,36($sp)
	lw	$5,28($sp)
$L728:
	move	$4,$18
	.set	noreorder
	.set	nomacro
	jal	strsep
	li	$21,59			# 0x3b
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L630
	sw	$2,20($sp)
	.set	macro
	.set	reorder

$L633:
	.set	noreorder
	.set	nomacro
	jal	strlen
	move	$4,$23
	.set	macro
	.set	reorder

	move	$4,$18
	addu	$2,$23,$2
	lw	$5,28($sp)
	.set	noreorder
	.set	nomacro
	jal	strsep
	sb	$21,0($2)
	.set	macro
	.set	reorder

	lw	$2,16($sp)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L634
	lw	$2,20($sp)
	.set	macro
	.set	reorder

$L630:
	li	$5,92			# 0x5c
	.set	noreorder
	.set	nomacro
	jal	check_esc_end
	move	$4,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L633
	lw	$23,20($sp)
	.set	macro
	.set	reorder

$L634:
	addiu	$5,$17,%lo($LC29)
	.set	noreorder
	.set	nomacro
	jal	strsep
	addiu	$4,$sp,20
	.set	macro
	.set	reorder

	move	$21,$2
	lw	$2,20($sp)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L600
	addiu	$2,$17,%lo($LC29)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L632
	sw	$2,32($sp)
	.set	macro
	.set	reorder

$L635:
	jal	strlen
	li	$3,32			# 0x20
	addu	$2,$21,$2
	lw	$5,32($sp)
	addiu	$4,$sp,20
	.set	noreorder
	.set	nomacro
	jal	strsep
	sb	$3,0($2)
	.set	macro
	.set	reorder

$L632:
	li	$5,92			# 0x5c
	.set	noreorder
	.set	nomacro
	jal	check_esc_end
	move	$4,$21
	.set	macro
	.set	reorder

	move	$4,$21
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L635
	move	$23,$2
	.set	macro
	.set	reorder

	lb	$4,0($21)
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L642
	move	$3,$0
	.set	macro
	.set	reorder

	li	$8,92			# 0x5c
	.set	noreorder
	.set	nomacro
	b	$L641
	move	$2,$21
	.set	macro
	.set	reorder

$L638:
	addiu	$2,$2,1
	lb	$4,0($2)
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L642
	addiu	$3,$3,1
	.set	macro
	.set	reorder

$L641:
	bne	$4,$8,$L638
	lb	$6,1($2)
	.set	noreorder
	.set	nomacro
	beq	$6,$0,$L639
	move	$4,$3
	.set	macro
	.set	reorder

	move	$5,$2
	move	$7,$3
$L640:
	sb	$6,0($5)
	addiu	$5,$5,1
	lb	$6,1($5)
	addiu	$7,$7,1
	.set	noreorder
	.set	nomacro
	bne	$6,$0,$L640
	move	$4,$7
	.set	macro
	.set	reorder

$L639:
	addu	$4,$21,$4
	.set	noreorder
	.set	nomacro
	b	$L638
	sb	$0,0($4)
	.set	macro
	.set	reorder

$L628:
	addiu	$5,$3,-55
	sltu	$3,$6,6
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L725
	addiu	$2,$2,1
	.set	macro
	.set	reorder

	b	$L600
$L624:
	addiu	$6,$3,-55
	sltu	$3,$7,6
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L726
	addiu	$2,$2,1
	.set	macro
	.set	reorder

	b	$L600
$L642:
	addiu	$5,$17,%lo($LC29)
	.set	noreorder
	.set	nomacro
	jal	strsep
	addiu	$4,$sp,20
	.set	macro
	.set	reorder

	lw	$3,20($sp)
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L600
	li	$4,73			# 0x49
	.set	macro
	.set	reorder

	lb	$6,0($2)
	.set	noreorder
	.set	nomacro
	bne	$6,$4,$L643
	addiu	$3,$2,1
	.set	macro
	.set	reorder

	lbu	$2,1($2)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L662
	move	$4,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L646
	move	$8,$3
	.set	macro
	.set	reorder

$L717:
	addiu	$7,$2,-87
$L644:
	addiu	$8,$8,1
$L727:
	lbu	$2,0($8)
	sll	$4,$4,4
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L716
	addu	$4,$4,$7
	.set	macro
	.set	reorder

$L646:
	addiu	$7,$2,-48
	addiu	$5,$2,-97
	sltu	$9,$7,10
	.set	noreorder
	.set	nomacro
	bne	$9,$0,$L644
	sltu	$5,$5,6
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$5,$0,$L717
	addiu	$9,$2,-65
	.set	macro
	.set	reorder

	addiu	$7,$2,-55
	sltu	$2,$9,6
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L727
	addiu	$8,$8,1
	.set	macro
	.set	reorder

	b	$L600
$L716:
	li	$2,-1			# 0xffffffffffffffff
	beq	$4,$2,$L600
	sw	$4,24($sp)
$L643:
	li	$5,37748736			# 0x2400000
	lw	$4,28($20)
	sw	$3,40($sp)
	addiu	$5,$5,192
	.set	noreorder
	.set	nomacro
	jal	kmem_cache_alloc
	sw	$6,32($sp)
	.set	macro
	.set	reorder

	move	$5,$2
	lw	$6,32($sp)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L610
	lw	$3,40($sp)
	.set	macro
	.set	reorder

	lw	$7,%lo(filter_rule_num)($fp)
	addiu	$2,$2,8
	li	$4,73			# 0x49
	sw	$5,0($5)
	sw	$5,4($5)
	sw	$2,8($5)
	sw	$2,12($5)
	sw	$0,24($5)
	sw	$7,20($5)
	.set	noreorder
	.set	nomacro
	beq	$6,$4,$L718
	sb	$6,28($5)
	.set	macro
	.set	reorder

	li	$2,77			# 0x4d
	.set	noreorder
	.set	nomacro
	beq	$6,$2,$L719
	lw	$2,36($sp)
	.set	macro
	.set	reorder

	li	$3,1			# 0x1
	sw	$3,%lo(defaultRuleChanged)($2)
$L649:
	sw	$22,16($5)
	move	$4,$21
	.set	noreorder
	.set	nomacro
	jal	strlen
	sw	$5,32($sp)
	.set	macro
	.set	reorder

	sltu	$2,$2,41
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L720
	lw	$5,32($sp)
	.set	macro
	.set	reorder

	addiu	$3,$5,48
	li	$2,39			# 0x27
	li	$4,39			# 0x27
$L657:
 #APP
 # 52 "./arch/mips/include/asm/string.h" 1
	.set	noreorder
	.set	noat
1:	lbu	$1,($21)
	subu	$2,1
	sb	$1,($3)
	beqz	$1,2f
	addiu	$3,1
	bnez	$2,1b
	addiu	$21,1
2:
	.set	at
	.set	reorder
 # 0 "" 2
 #NO_APP
	move	$23,$4
$L655:
	addu	$23,$5,$23
	li	$4,3			# 0x3
	.set	noreorder
	.set	nomacro
	jal	filter_item_regist
	sb	$0,48($23)
	.set	macro
	.set	reorder

	lw	$2,%lo(filter_rule_num)($fp)
	lw	$3,16($sp)
	addiu	$2,$2,1
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L610
	sw	$2,%lo(filter_rule_num)($fp)
	.set	macro
	.set	reorder

	lb	$2,0($3)
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L728
	lw	$5,28($sp)
	.set	macro
	.set	reorder

	b	$L610
$L662:
	.set	noreorder
	.set	nomacro
	b	$L643
	sw	$0,24($sp)
	.set	macro
	.set	reorder

$L720:
	.set	noreorder
	.set	nomacro
	jal	strlen
	move	$4,$21
	.set	macro
	.set	reorder

	lw	$5,32($sp)
	move	$4,$2
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L655
	addiu	$3,$5,48
	.set	macro
	.set	reorder

	b	$L657
$L719:
	addiu	$6,$5,29
	move	$2,$3
	move	$4,$6
	li	$3,12			# 0xc
 #APP
 # 52 "./arch/mips/include/asm/string.h" 1
	.set	noreorder
	.set	noat
1:	lbu	$1,($2)
	subu	$3,1
	sb	$1,($4)
	beqz	$1,2f
	addiu	$4,1
	bnez	$3,1b
	addiu	$2,1
2:
	.set	at
	.set	reorder
 # 0 "" 2
 #NO_APP
	move	$7,$0
	sb	$0,41($5)
	move	$3,$6
$L651:
	move	$4,$6
	sw	$3,48($sp)
	sw	$7,44($sp)
	sw	$5,40($sp)
	.set	noreorder
	.set	nomacro
	jal	strlen
	sw	$6,32($sp)
	.set	macro
	.set	reorder

	lw	$7,44($sp)
	lw	$6,32($sp)
	lw	$5,40($sp)
	sltu	$2,$7,$2
	lw	$3,48($sp)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L649
	addiu	$7,$7,1
	.set	macro
	.set	reorder

	lbu	$2,0($3)
	addiu	$4,$2,-97
	sltu	$4,$4,26
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L652
	addiu	$2,$2,-32
	.set	macro
	.set	reorder

	sb	$2,0($3)
$L652:
	.set	noreorder
	.set	nomacro
	b	$L651
	addiu	$3,$3,1
	.set	macro
	.set	reorder

$L718:
	lw	$2,24($sp)
	.set	noreorder
	.set	nomacro
	b	$L649
	sw	$2,44($5)
	.set	macro
	.set	reorder

	.end	filter_table_single_write
	.size	filter_table_single_write, .-filter_table_single_write
	.align	2
	.globl	scan_table
	.set	nomips16
	.set	nomicromips
	.ent	scan_table
	.type	scan_table, @function
scan_table:
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
	lui	$22,%hi(url_filter_flag)
	beq	$6,$0,$L730
	sw	$0,%lo(url_filter_flag)($22)

	lw	$2,28($6)
	beq	$2,$0,$L729
	li	$2,-1			# 0xffffffffffffffff

	lw	$2,24($6)
	sltu	$2,$2,2
	bne	$2,$0,$L733
	addiu	$21,$6,8

	lw	$16,8($6)
	move	$20,$7
	move	$19,$5
	move	$18,$4
	beq	$21,$16,$L733
	move	$17,$6

	lw	$2,16($16)
	andi	$3,$2,0x1
	bne	$3,$0,$L791
	andi	$3,$2,0x200

$L787:
	lw	$3,20($16)
	beq	$3,$19,$L791
	andi	$3,$2,0x200

$L735:
	lw	$2,0($16)
	beq	$21,$2,$L786
	nop

	move	$16,$2
	lw	$2,16($16)
	andi	$3,$2,0x1
	beq	$3,$0,$L787
	andi	$3,$2,0x200

$L791:
	beq	$3,$0,$L736
	nop

	andi	$2,$2,0x2
	beq	$2,$0,$L735
	nop

	lw	$7,0($20)
	beq	$7,$0,$L730
	move	$6,$7

	lw	$5,20($16)
	jal	scan_table
	move	$4,$18

	beq	$2,$0,$L735
	nop

$L740:
	li	$2,1			# 0x1
$L729:
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

$L736:
	andi	$3,$2,0x100
	beq	$3,$0,$L788
	addiu	$5,$16,48

$L738:
	andi	$3,$2,0x2
	beq	$3,$0,$L789
	andi	$3,$2,0x1

	lw	$7,0($20)
	beq	$7,$0,$L730
	move	$6,$7

	lw	$5,20($16)
	jal	scan_table
	move	$4,$18

	beq	$2,$0,$L735
	nop

	lw	$2,16($16)
	andi	$3,$2,0x1
	beq	$3,$0,$L741
	lw	$31,44($sp)

	lui	$3,%hi(url_filter_mode)
	lw	$3,%lo(url_filter_mode)($3)
	beq	$3,$0,$L742
	nop

	ext	$2,$2,3,1
$L790:
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

$L733:
	lui	$2,%hi(url_filter_mode)
	lw	$2,%lo(url_filter_mode)($2)
	beq	$2,$0,$L729
	li	$2,-1			# 0xffffffffffffffff

	lui	$2,%hi(default_policy)
	lw	$2,%lo(default_policy)($2)
	beq	$2,$0,$L740
	nop

$L730:
	b	$L729
	li	$2,-1			# 0xffffffffffffffff

$L788:
	lw	$2,28($17)
	jalr	$2
	move	$4,$18

	beq	$2,$0,$L735
	nop

	b	$L738
	lw	$2,16($16)

$L741:
	lui	$2,%hi(url_filter_mode)
	lw	$22,40($sp)
	lw	$21,36($sp)
	lw	$2,%lo(url_filter_mode)($2)
	lw	$20,32($sp)
	lw	$19,28($sp)
	lw	$18,24($sp)
	sltu	$2,$2,1
	lw	$17,20($sp)
	lw	$16,16($sp)
	jr	$31
	addiu	$sp,$sp,48

$L789:
	beq	$3,$0,$L740
	nop

$L742:
	xori	$2,$2,0x8
	b	$L790
	ext	$2,$2,3,1

$L786:
$L744 = .
	beq	$16,$0,$L733
	lw	$2,%lo(url_filter_flag)($22)

	bne	$2,$0,$L740
	lw	$31,44($sp)

	lw	$3,16($16)
	lw	$22,40($sp)
	ext	$4,$3,3,1
	lw	$21,36($sp)
	andi	$3,$3,0x1
	lw	$20,32($sp)
	movn	$2,$4,$3
	lw	$19,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	jr	$31
	addiu	$sp,$sp,48

	.set	macro
	.set	reorder
	.end	scan_table
	.size	scan_table, .-scan_table
	.section	.rodata.str1.4
	.align	2
$LC37:
	.ascii	"[ip_range_filter]%d.%d.%d.%d ===> %d.%d.%d.%d  [Blocked]"
	.ascii	"\012\000"
	.align	2
$LC38:
	.ascii	"[mac_filter]%d.%d.%d.%d(%02x:%02x:%02x:%02x:%02x:%02x) ="
	.ascii	"==> %d.%d.%d.%d [Blocked]\012\000"
	.align	2
$LC39:
	.ascii	"[url_key_filter]%d.%d.%d.%d ==> %d.%d.%d.%d(%s)  [Blocke"
	.ascii	"d]\012\000"
	.align	2
$LC40:
	.ascii	"[schedule_filter]%d.%d.%d.%d ===> %d.%d.%d.%d  [Blocked]"
	.ascii	"\012\000"
	.align	2
$LC41:
	.ascii	"Unkown Type [Blocked]\012\000"
	.text
	.align	2
	.globl	log_fastFilter
	.set	nomips16
	.set	nomicromips
	.ent	log_fastFilter
	.type	log_fastFilter, @function
log_fastFilter:
	.frame	$sp,184,$31		# vars= 8, regs= 9/0, args= 136, gp= 0
	.mask	0x80ff0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-184
	sw	$31,180($sp)
	sw	$23,176($sp)
	sw	$22,172($sp)
	sw	$21,168($sp)
	sw	$20,164($sp)
	sw	$19,160($sp)
	sw	$18,156($sp)
	sw	$17,152($sp)
	sw	$16,148($sp)
	move	$16,$4
	addiu	$4,$sp,136
	jal	do_gettimeofday
	move	$17,$5

	jal	rtl_ip_hdr
	move	$4,$17

	move	$4,$17
	jal	rtl_eth_hdr
	move	$18,$2

	li	$3,2			# 0x2
	beq	$16,$3,$L794
	nop

	slt	$2,$16,3
	bne	$2,$0,$L817
	li	$2,3			# 0x3

	beq	$16,$2,$L797
	li	$2,4			# 0x4

	bne	$16,$2,$L818
	lui	$4,%hi($LC41)

	beq	$18,$0,$L799
	lui	$4,%hi($LC40)

	lw	$2,16($18)
	lw	$5,12($18)
	addiu	$4,$4,%lo($LC40)
	ext	$7,$2,8,8
	ext	$3,$2,16,8
	srl	$6,$2,24
	sw	$7,28($sp)
	andi	$2,$2,0xff
	sw	$3,24($sp)
	ext	$7,$5,8,8
	sw	$6,20($sp)
	andi	$3,$5,0xff
	sw	$2,32($sp)
	ext	$6,$5,16,8
	sw	$3,16($sp)
	jal	scrlog_printk
	srl	$5,$5,24

$L799:
	lw	$31,180($sp)
$L819:
	move	$2,$0
	lw	$23,176($sp)
	lw	$22,172($sp)
	lw	$21,168($sp)
	lw	$20,164($sp)
	lw	$19,160($sp)
	lw	$18,156($sp)
	lw	$17,152($sp)
	lw	$16,148($sp)
	jr	$31
	addiu	$sp,$sp,184

$L797:
	beq	$18,$0,$L799
	lui	$3,%hi(log_info)

	lw	$2,16($18)
	lw	$5,12($18)
	addiu	$3,$3,%lo(log_info)
	srl	$4,$2,24
	ext	$7,$2,8,8
	sw	$3,36($sp)
	ext	$6,$2,16,8
	sw	$4,20($sp)
	lui	$4,%hi($LC39)
	sw	$7,28($sp)
	andi	$3,$5,0xff
	sw	$6,24($sp)
	ext	$7,$5,8,8
	sw	$3,16($sp)
	ext	$6,$5,16,8
	andi	$2,$2,0xff
	srl	$5,$5,24
	sw	$2,32($sp)
	jal	scrlog_printk
	addiu	$4,$4,%lo($LC39)

	move	$2,$0
	lw	$31,180($sp)
	lw	$23,176($sp)
	lw	$22,172($sp)
	lw	$21,168($sp)
	lw	$20,164($sp)
	lw	$19,160($sp)
	lw	$18,156($sp)
	lw	$17,152($sp)
	lw	$16,148($sp)
	jr	$31
	addiu	$sp,$sp,184

$L817:
	li	$2,1			# 0x1
	bne	$16,$2,$L793
	lui	$4,%hi($LC41)

	beq	$18,$0,$L799
	lui	$4,%hi($LC37)

	lw	$2,16($18)
	lw	$5,12($18)
	addiu	$4,$4,%lo($LC37)
	ext	$7,$2,8,8
	ext	$3,$2,16,8
	srl	$6,$2,24
	sw	$7,28($sp)
	sw	$3,24($sp)
	ext	$7,$5,8,8
	andi	$3,$5,0xff
	sw	$6,20($sp)
	andi	$2,$2,0xff
	sw	$3,16($sp)
	ext	$6,$5,16,8
	sw	$2,32($sp)
	jal	scrlog_printk
	srl	$5,$5,24

	move	$2,$0
	lw	$31,180($sp)
	lw	$23,176($sp)
	lw	$22,172($sp)
	lw	$21,168($sp)
	lw	$20,164($sp)
	lw	$19,160($sp)
	lw	$18,156($sp)
	lw	$17,152($sp)
	lw	$16,148($sp)
	jr	$31
	addiu	$sp,$sp,184

$L793:
$L818:
	jal	printk
	addiu	$4,$4,%lo($LC41)

	move	$2,$0
	lw	$31,180($sp)
	lw	$23,176($sp)
	lw	$22,172($sp)
	lw	$21,168($sp)
	lw	$20,164($sp)
	lw	$19,160($sp)
	lw	$18,156($sp)
	lw	$17,152($sp)
	lw	$16,148($sp)
	jr	$31
	addiu	$sp,$sp,184

$L794:
	beq	$2,$0,$L819
	lw	$31,180($sp)

	beq	$18,$0,$L819
	move	$4,$17

	jal	rtl_eth_hdr
	lw	$23,12($18)

	move	$4,$17
	jal	rtl_eth_hdr
	move	$16,$2

	move	$4,$17
	jal	rtl_eth_hdr
	move	$19,$2

	move	$4,$17
	jal	rtl_eth_hdr
	move	$20,$2

	move	$4,$17
	jal	rtl_eth_hdr
	move	$21,$2

	move	$4,$17
	jal	rtl_eth_hdr
	move	$22,$2

	lw	$3,16($18)
	andi	$9,$23,0xff
	ext	$7,$23,8,8
	ext	$6,$3,8,8
	ext	$5,$3,16,8
	srl	$4,$3,24
	sw	$6,124($sp)
	andi	$3,$3,0xff
	sw	$5,120($sp)
	sw	$4,116($sp)
	lui	$4,%hi($LC38)
	sw	$3,128($sp)
	ext	$6,$23,16,8
	lwl	$10,157($2)
	srl	$5,$23,24
	lwl	$8,161($2)
	addiu	$4,$4,%lo($LC38)
	lwl	$3,165($2)
	lwr	$10,154($2)
	lwr	$8,158($2)
	lwr	$3,162($2)
	sw	$10,100($sp)
	sw	$8,104($sp)
	sw	$3,108($sp)
	lbu	$3,166($2)
	sb	$3,112($sp)
	lbu	$2,167($2)
	sb	$2,113($sp)
	lwl	$8,143($22)
	lwl	$3,147($22)
	lwr	$8,140($22)
	lwr	$3,144($22)
	lwl	$2,151($22)
	lwr	$2,148($22)
	sw	$8,84($sp)
	sw	$3,88($sp)
	sw	$2,92($sp)
	lbu	$2,152($22)
	sb	$2,96($sp)
	lbu	$2,153($22)
	sb	$2,97($sp)
	lwl	$8,129($21)
	lwl	$3,133($21)
	lwl	$2,137($21)
	lwr	$8,126($21)
	lwr	$3,130($21)
	lwr	$2,134($21)
	sw	$8,68($sp)
	sw	$3,72($sp)
	sw	$2,76($sp)
	lbu	$2,138($21)
	sb	$2,80($sp)
	lbu	$2,139($21)
	sb	$2,81($sp)
	lwl	$8,115($20)
	lwr	$8,112($20)
	lwl	$3,119($20)
	lwl	$2,123($20)
	lwr	$3,116($20)
	lwr	$2,120($20)
	sw	$8,52($sp)
	sw	$3,56($sp)
	sw	$2,60($sp)
	lbu	$2,124($20)
	sb	$2,64($sp)
	lbu	$2,125($20)
	sb	$2,65($sp)
	lwl	$8,101($19)
	lwl	$3,105($19)
	lwl	$2,109($19)
	lwr	$8,98($19)
	lwr	$3,102($19)
	lwr	$2,106($19)
	sw	$8,36($sp)
	sw	$3,40($sp)
	sw	$2,44($sp)
	lbu	$2,110($19)
	sb	$2,48($sp)
	lbu	$2,111($19)
	sb	$2,49($sp)
	lwl	$8,87($16)
	lwl	$3,91($16)
	lwl	$2,95($16)
	lwr	$8,84($16)
	lwr	$3,88($16)
	lwr	$2,92($16)
	sw	$8,20($sp)
	sw	$3,24($sp)
	sw	$2,28($sp)
	lbu	$2,96($16)
	sb	$2,32($sp)
	lbu	$2,97($16)
	sw	$9,16($sp)
	jal	scrlog_printk
	sb	$2,33($sp)

	move	$2,$0
	lw	$31,180($sp)
	lw	$23,176($sp)
	lw	$22,172($sp)
	lw	$21,168($sp)
	lw	$20,164($sp)
	lw	$19,160($sp)
	lw	$18,156($sp)
	lw	$17,152($sp)
	lw	$16,148($sp)
	jr	$31
	addiu	$sp,$sp,184

	.set	macro
	.set	reorder
	.end	log_fastFilter
	.size	log_fastFilter, .-log_fastFilter
	.align	2
	.globl	do_filter
	.set	nomips16
	.set	nomicromips
	.ent	do_filter
	.type	do_filter, @function
do_filter:
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
	lui	$2,%hi(table_list_head)
	addiu	$19,$2,%lo(table_list_head)
	lw	$16,%lo(table_list_head)($2)
	beq	$16,$19,$L823
	move	$18,$4

	li	$20,-1			# 0xffffffffffffffff
	b	$L828
	li	$21,1			# 0x1

$L833:
	beq	$2,$21,$L832
	lui	$2,%hi(log_enable)

	lw	$16,0($16)
	beq	$16,$19,$L823
	nop

$L828:
	move	$5,$0
	move	$7,$16
	move	$6,$16
	jal	scan_table
	move	$4,$18

	bne	$2,$20,$L833
	move	$17,$2

$L823:
	move	$17,$0
$L820:
	move	$2,$17
$L834:
	lw	$31,44($sp)
	lw	$21,40($sp)
	lw	$20,36($sp)
	lw	$19,32($sp)
	lw	$18,28($sp)
	lw	$17,24($sp)
	lw	$16,20($sp)
	jr	$31
	addiu	$sp,$sp,48

$L832:
	lw	$2,%lo(log_enable)($2)
	beq	$2,$0,$L820
	nop

	lw	$4,16($16)
	jal	log_fastFilter
	move	$5,$18

	b	$L834
	move	$2,$17

	.set	macro
	.set	reorder
	.end	do_filter
	.size	do_filter, .-do_filter
	.align	2
	.globl	FilterWeb_v2
	.set	nomips16
	.set	nomicromips
	.ent	FilterWeb_v2
	.type	FilterWeb_v2, @function
FilterWeb_v2:
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

	jal	need_filter
	move	$4,$16

	bne	$2,$0,$L838
	lw	$31,20($sp)

	lw	$16,16($sp)
	jr	$31
	addiu	$sp,$sp,24

$L838:
	move	$4,$16
	lw	$16,16($sp)
	j	do_filter
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	FilterWeb_v2
	.size	FilterWeb_v2, .-FilterWeb_v2
	.align	2
	.globl	filter_table_init
	.set	nomips16
	.set	nomicromips
	.ent	filter_table_init
	.type	filter_table_init, @function
filter_table_init:
	.frame	$sp,32,$31		# vars= 0, regs= 1/0, args= 24, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	jal	filter_table_head_init
	nop

	lui	$7,%hi(filter_table_proc_fops)
	lui	$6,%hi(proc_root)
	sw	$0,16($sp)
	lui	$4,%hi($LC24)
	addiu	$7,$7,%lo(filter_table_proc_fops)
	addiu	$6,$6,%lo(proc_root)
	move	$5,$0
	jal	proc_create_data
	addiu	$4,$4,%lo($LC24)

	lui	$3,%hi(res12)
	lw	$31,28($sp)
	sw	$2,%lo(res12)($3)
	move	$2,$0
	jr	$31
	addiu	$sp,$sp,32

	.set	macro
	.set	reorder
	.end	filter_table_init
	.size	filter_table_init, .-filter_table_init
	.section	.init.text,"ax",@progbits
	.align	2
	.globl	filter_init
	.set	nomips16
	.set	nomicromips
	.ent	filter_init
	.type	filter_init, @function
filter_init:
	.frame	$sp,32,$31		# vars= 0, regs= 2/0, args= 24, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$16,24($sp)
	lui	$2,%hi(dos_item_global)
	lui	$16,%hi(dos_timer)
	sw	$0,%lo(dos_item_global)($2)
	lui	$2,%hi(dos_item_ipv6)
	sw	$0,%lo(dos_item_ipv6)($2)
	lui	$2,%hi(none_dos_drop_pkt_cnt)
	jal	dos_func_init
	sw	$0,%lo(none_dos_drop_pkt_cnt)($2)

	lui	$7,%hi(dos_proc_fops)
	lui	$6,%hi(proc_root)
	sw	$0,16($sp)
	lui	$4,%hi($LC23)
	addiu	$7,$7,%lo(dos_proc_fops)
	addiu	$6,$6,%lo(proc_root)
	move	$5,$0
	jal	proc_create_data
	addiu	$4,$4,%lo($LC23)

	lui	$3,%hi(res3)
	move	$7,$0
	addiu	$4,$16,%lo(dos_timer)
	move	$6,$0
	sw	$2,%lo(res3)($3)
	move	$5,$0
	jal	init_timer_key
	addiu	$16,$16,%lo(dos_timer)

	lui	$4,%hi(jiffies)
	lui	$6,%hi(dos_timer_fn)
	sw	$0,16($16)
	lw	$3,%lo(jiffies)($4)
	addiu	$6,$6,%lo(dos_timer_fn)
	lw	$5,%lo(jiffies)($4)
	move	$4,$16
	sw	$6,12($16)
	addiu	$3,$3,100
	addiu	$5,$5,100
	jal	mod_timer
	sw	$3,8($16)

	lui	$2,%hi(filter_rule_num)
	li	$3,1			# 0x1
	jal	filter_table_init
	sw	$3,%lo(filter_rule_num)($2)

	jal	igmp_delete_init_netlink
	nop

	move	$2,$0
	lw	$31,28($sp)
	lw	$16,24($sp)
	jr	$31
	addiu	$sp,$sp,32

	.set	macro
	.set	reorder
	.end	filter_init
	.size	filter_init, .-filter_init
	.text
	.align	2
	.globl	filter_init_fastpath
	.set	nomips16
	.set	nomicromips
	.ent	filter_init_fastpath
	.type	filter_init_fastpath, @function
filter_init_fastpath:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	lui	$2,%hi(filter_rule_num)
	li	$3,1			# 0x1
	jal	filter_table_init
	sw	$3,%lo(filter_rule_num)($2)

	move	$2,$0
	lw	$31,20($sp)
	jr	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	filter_init_fastpath
	.size	filter_init_fastpath, .-filter_init_fastpath
	.align	2
	.globl	filter_exit_fastpath
	.set	nomips16
	.set	nomicromips
	.ent	filter_exit_fastpath
	.type	filter_exit_fastpath, @function
filter_exit_fastpath:
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$16,16($sp)
	lui	$16,%hi(res12)
	lw	$2,%lo(res12)($16)
	beq	$2,$0,$L846
	lui	$5,%hi(proc_root)

	lui	$4,%hi($LC24)
	addiu	$5,$5,%lo(proc_root)
	jal	remove_proc_entry
	addiu	$4,$4,%lo($LC24)

	sw	$0,%lo(res12)($16)
$L846:
	lw	$31,20($sp)
	move	$2,$0
	lw	$16,16($sp)
	jr	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	filter_exit_fastpath
	.size	filter_exit_fastpath, .-filter_exit_fastpath
	.align	2
	.globl	strtoint
	.set	nomips16
	.set	nomicromips
	.ent	strtoint
	.type	strtoint, @function
strtoint:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	lb	$6,0($4)
	beq	$6,$0,$L856
	nop

	addiu	$2,$6,-48
	sltu	$2,$2,10
	bne	$2,$0,$L857
	move	$3,$0

	jr	$31
	move	$2,$3

$L855:
	beq	$5,$0,$L859
	move	$2,$3

$L857:
	sll	$2,$3,2
	lb	$7,1($4)
	addiu	$4,$4,1
	addu	$2,$2,$3
	addiu	$5,$7,-48
	addiu	$3,$6,-48
	sll	$2,$2,1
	move	$6,$7
	sltu	$5,$5,10
	bne	$7,$0,$L855
	addu	$3,$3,$2

$L851:
	move	$2,$3
$L859:
	jr	$31
	nop

$L856:
	b	$L851
	move	$3,$0

	.set	macro
	.set	reorder
	.end	strtoint
	.size	strtoint, .-strtoint
	.align	2
	.globl	strtok
	.set	nomips16
	.set	nomicromips
	.ent	strtok
	.type	strtok, @function
strtok:
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
	beq	$4,$0,$L873
	move	$17,$5

	lui	$18,%hi(___strtok)
	move	$16,$4
$L861:
	move	$4,$16
	jal	strspn
	move	$5,$17

	addu	$16,$16,$2
	lb	$2,0($16)
	beq	$2,$0,$L875
	move	$5,$17

	jal	strpbrk
	move	$4,$16

	beq	$2,$0,$L864
	nop

	lb	$3,0($2)
	bne	$3,$0,$L876
	nop

$L864:
	sw	$2,%lo(___strtok)($18)
$L860:
	move	$2,$16
	lw	$31,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	jr	$31
	addiu	$sp,$sp,32

$L876:
	sb	$0,0($2)
	b	$L864
	addiu	$2,$2,1

$L875:
	move	$16,$0
	lw	$31,28($sp)
	lw	$17,20($sp)
	move	$2,$16
	lw	$16,16($sp)
	sw	$0,%lo(___strtok)($18)
	lw	$18,24($sp)
	jr	$31
	addiu	$sp,$sp,32

$L873:
	lui	$18,%hi(___strtok)
	lw	$16,%lo(___strtok)($18)
	bne	$16,$0,$L861
	nop

	b	$L860
	move	$16,$0

	.set	macro
	.set	reorder
	.end	strtok
	.size	strtok, .-strtok
	.section	.rodata.str1.4
	.align	2
$LC42:
	.ascii	"net/rtl/fastpath/core/filter.c\000"
	.text
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	dos_single_write
	.type	dos_single_write, @function
dos_single_write:
	.frame	$sp,144,$31		# vars= 88, regs= 10/0, args= 16, gp= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	sltu	$2,$6,2
	bne	$2,$0,$L909
	beq	$5,$0,$L909
	addiu	$sp,$sp,-144
	sw	$31,140($sp)
	sw	$fp,136($sp)
	sw	$23,132($sp)
	sw	$22,128($sp)
	sw	$21,124($sp)
	sw	$20,120($sp)
	sw	$19,116($sp)
	sw	$18,112($sp)
	sw	$17,108($sp)
	sw	$16,104($sp)
	lui	$16,%hi(dos_flag)
	move	$18,$5
	move	$17,$6
	move	$5,$0
	li	$6,80			# 0x50
	.set	noreorder
	.set	nomacro
	jal	memset
	addiu	$4,$16,%lo(dos_flag)
	.set	macro
	.set	reorder

	addiu	$4,$sp,16
	li	$6,80			# 0x50
	.set	noreorder
	.set	nomacro
	jal	memset
	move	$5,$0
	.set	macro
	.set	reorder

	addu	$2,$18,$17
	lw	$3,24($28)
	or	$4,$18,$17
	or	$2,$2,$4
	and	$2,$2,$3
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L881
	lui	$4,%hi($LC42)
	.set	macro
	.set	reorder

	li	$5,2473			# 0x9a9
	.set	noreorder
	.set	nomacro
	jal	__might_fault
	addiu	$4,$4,%lo($LC42)
	.set	macro
	.set	reorder

	addiu	$4,$16,%lo(dos_flag)
	move	$5,$18
	move	$6,$17
 #APP
 # 2483 "net/rtl/fastpath/core/filter.c" 1
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
	bne	$6,$0,$L912
	addiu	$2,$16,%lo(dos_flag)
	.set	macro
	.set	reorder

	addiu	$3,$sp,16
	addu	$5,$2,$17
	move	$4,$17
	sb	$0,0($5)
 #APP
 # 52 "./arch/mips/include/asm/string.h" 1
	.set	noreorder
	.set	noat
1:	lbu	$1,($2)
	subu	$4,1
	sb	$1,($3)
	beqz	$1,2f
	addiu	$3,1
	bnez	$4,1b
	addiu	$2,1
2:
	.set	at
	.set	reorder
 # 0 "" 2
 #NO_APP
	lui	$2,%hi(dos_item_global_0)
	lui	$19,%hi($LC29)
	sw	$2,96($sp)
	addiu	$4,$sp,16
	lui	$2,%hi(block_time)
	addiu	$5,$19,%lo($LC29)
	lui	$18,%hi($L885)
	.set	noreorder
	.set	nomacro
	jal	strtok
	sw	$2,100($sp)
	.set	macro
	.set	reorder

	li	$16,1			# 0x1
	addiu	$18,$18,%lo($L885)
	lui	$fp,%hi(Ipv6_ns_threshold)
	lui	$23,%hi(Ipv6_icmp_too_big_threshold)
	lui	$22,%hi(Ipv6_smurf_threshold)
	lui	$21,%hi(dos_item_ipv6)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L913
	move	$20,$2
	.set	macro
	.set	reorder

$L903:
	move	$6,$0
	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	simple_strtol
	move	$4,$20
	.set	macro
	.set	reorder

	sltu	$3,$16,19
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L883
	sll	$3,$16,2
	.set	macro
	.set	reorder

	addu	$3,$18,$3
	lw	$3,0($3)
	jr	$3
	.rdata
	.align	2
	.align	2
$L885:
	.word	$L883
	.word	$L884
	.word	$L886
	.word	$L887
	.word	$L888
	.word	$L889
	.word	$L890
	.word	$L891
	.word	$L892
	.word	$L893
	.word	$L894
	.word	$L895
	.word	$L896
	.word	$L897
	.word	$L898
	.word	$L899
	.word	$L900
	.word	$L901
	.word	$L902
	.text
$L881:
	move	$5,$0
	addiu	$4,$16,%lo(dos_flag)
	.set	noreorder
	.set	nomacro
	jal	memset
	move	$6,$17
	.set	macro
	.set	reorder

$L912:
	lw	$31,140($sp)
	li	$2,-14			# 0xfffffffffffffff2
	lw	$fp,136($sp)
	lw	$23,132($sp)
	lw	$22,128($sp)
	lw	$21,124($sp)
	lw	$20,120($sp)
	lw	$19,116($sp)
	lw	$18,112($sp)
	lw	$17,108($sp)
	lw	$16,104($sp)
	.set	noreorder
	.set	nomacro
	jr	$31
	addiu	$sp,$sp,144
	.set	macro
	.set	reorder

$L900:
	sw	$2,%lo(Ipv6_smurf_threshold)($22)
$L883:
	addiu	$16,$16,1
	move	$4,$0
	andi	$16,$16,0x00ff
$L914:
	.set	noreorder
	.set	nomacro
	jal	strtok
	addiu	$5,$19,%lo($LC29)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L903
	move	$20,$2
	.set	macro
	.set	reorder

$L913:
	move	$2,$17
	lw	$31,140($sp)
	lw	$fp,136($sp)
	lw	$23,132($sp)
	lw	$22,128($sp)
	lw	$21,124($sp)
	lw	$20,120($sp)
	lw	$19,116($sp)
	lw	$18,112($sp)
	lw	$17,108($sp)
	lw	$16,104($sp)
	.set	noreorder
	.set	nomacro
	jr	$31
	addiu	$sp,$sp,144
	.set	macro
	.set	reorder

$L902:
	addiu	$16,$16,1
	sw	$2,%lo(Ipv6_ns_threshold)($fp)
	move	$4,$0
	.set	noreorder
	.set	nomacro
	b	$L914
	andi	$16,$16,0x00ff
	.set	macro
	.set	reorder

$L901:
	addiu	$16,$16,1
	sw	$2,%lo(Ipv6_icmp_too_big_threshold)($23)
	move	$4,$0
	.set	noreorder
	.set	nomacro
	b	$L914
	andi	$16,$16,0x00ff
	.set	macro
	.set	reorder

$L899:
	addiu	$16,$16,1
	sw	$2,%lo(dos_item_ipv6)($21)
	move	$4,$0
	.set	noreorder
	.set	nomacro
	b	$L914
	andi	$16,$16,0x00ff
	.set	macro
	.set	reorder

$L898:
	lw	$3,96($sp)
	addiu	$16,$16,1
	move	$4,$0
	andi	$16,$16,0x00ff
	.set	noreorder
	.set	nomacro
	b	$L914
	sw	$2,%lo(dos_item_global_0)($3)
	.set	macro
	.set	reorder

$L897:
	lw	$3,100($sp)
	addiu	$16,$16,1
	move	$4,$0
	andi	$16,$16,0x00ff
	.set	noreorder
	.set	nomacro
	b	$L914
	sw	$2,%lo(block_time)($3)
	.set	macro
	.set	reorder

$L896:
	lui	$3,%hi(per_icmp_threshold)
	addiu	$16,$16,1
	move	$4,$0
	sw	$2,%lo(per_icmp_threshold)($3)
	.set	noreorder
	.set	nomacro
	b	$L914
	andi	$16,$16,0x00ff
	.set	macro
	.set	reorder

$L895:
	lui	$3,%hi(per_udp_threshold)
	addiu	$16,$16,1
	move	$4,$0
	sw	$2,%lo(per_udp_threshold)($3)
	.set	noreorder
	.set	nomacro
	b	$L914
	andi	$16,$16,0x00ff
	.set	macro
	.set	reorder

$L894:
	lui	$3,%hi(per_fin_threshold)
	addiu	$16,$16,1
	move	$4,$0
	sw	$2,%lo(per_fin_threshold)($3)
	.set	noreorder
	.set	nomacro
	b	$L914
	andi	$16,$16,0x00ff
	.set	macro
	.set	reorder

$L893:
	lui	$3,%hi(per_syn_threshold)
	addiu	$16,$16,1
	move	$4,$0
	sw	$2,%lo(per_syn_threshold)($3)
	.set	noreorder
	.set	nomacro
	b	$L914
	andi	$16,$16,0x00ff
	.set	macro
	.set	reorder

$L892:
	lui	$3,%hi(whole_icmp_threshold)
	addiu	$16,$16,1
	move	$4,$0
	sw	$2,%lo(whole_icmp_threshold)($3)
	.set	noreorder
	.set	nomacro
	b	$L914
	andi	$16,$16,0x00ff
	.set	macro
	.set	reorder

$L891:
	lui	$3,%hi(whole_udp_threshold)
	addiu	$16,$16,1
	move	$4,$0
	sw	$2,%lo(whole_udp_threshold)($3)
	.set	noreorder
	.set	nomacro
	b	$L914
	andi	$16,$16,0x00ff
	.set	macro
	.set	reorder

$L890:
	lui	$3,%hi(whole_fin_threshold)
	addiu	$16,$16,1
	move	$4,$0
	sw	$2,%lo(whole_fin_threshold)($3)
	.set	noreorder
	.set	nomacro
	b	$L914
	andi	$16,$16,0x00ff
	.set	macro
	.set	reorder

$L889:
	lui	$3,%hi(whole_syn_threshold)
	addiu	$16,$16,1
	move	$4,$0
	sw	$2,%lo(whole_syn_threshold)($3)
	.set	noreorder
	.set	nomacro
	b	$L914
	andi	$16,$16,0x00ff
	.set	macro
	.set	reorder

$L888:
	lui	$3,%hi(dos_item_global)
	addiu	$16,$16,1
	move	$4,$0
	sw	$2,%lo(dos_item_global)($3)
	lui	$2,%hi(dos_item_backup)
	andi	$16,$16,0x00ff
	.set	noreorder
	.set	nomacro
	b	$L914
	sw	$0,%lo(dos_item_backup)($2)
	.set	macro
	.set	reorder

$L887:
	li	$6,16			# 0x10
	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	simple_strtol
	move	$4,$20
	.set	macro
	.set	reorder

	lui	$3,%hi(dos_lan_mask)
	addiu	$16,$16,1
	move	$4,$0
	sw	$2,%lo(dos_lan_mask)($3)
	.set	noreorder
	.set	nomacro
	b	$L914
	andi	$16,$16,0x00ff
	.set	macro
	.set	reorder

$L886:
	li	$6,16			# 0x10
	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	simple_strtol
	move	$4,$20
	.set	macro
	.set	reorder

	lui	$3,%hi(dos_lan_addr)
	addiu	$16,$16,1
	move	$4,$0
	sw	$2,%lo(dos_lan_addr)($3)
	.set	noreorder
	.set	nomacro
	b	$L914
	andi	$16,$16,0x00ff
	.set	macro
	.set	reorder

$L884:
	lui	$3,%hi(dos_op_mode)
	addiu	$16,$16,1
	move	$4,$0
	sb	$2,%lo(dos_op_mode)($3)
	.set	noreorder
	.set	nomacro
	b	$L914
	andi	$16,$16,0x00ff
	.set	macro
	.set	reorder

$L909:
	.set	noreorder
	.set	nomacro
	jr	$31
	li	$2,-14			# 0xfffffffffffffff2
	.set	macro
	.set	reorder

	.end	dos_single_write
	.size	dos_single_write, .-dos_single_write
	.align	2
	.globl	filter_addconnect
	.set	nomips16
	.set	nomicromips
	.ent	filter_addconnect
	.type	filter_addconnect, @function
filter_addconnect:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	lui	$3,%hi(ConnectedIp)
	andi	$2,$4,0xff
	sll	$2,$2,2
	addiu	$3,$3,%lo(ConnectedIp)
	addu	$2,$2,$3
	jr	$31
	sw	$4,0($2)

	.set	macro
	.set	reorder
	.end	filter_addconnect
	.size	filter_addconnect, .-filter_addconnect
	.align	2
	.globl	filter_delconnect
	.set	nomips16
	.set	nomicromips
	.ent	filter_delconnect
	.type	filter_delconnect, @function
filter_delconnect:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	lui	$2,%hi(ConnectedIp)
	andi	$4,$4,0x00ff
	sll	$4,$4,2
	addiu	$2,$2,%lo(ConnectedIp)
	addu	$4,$4,$2
	jr	$31
	sw	$0,0($4)

	.set	macro
	.set	reorder
	.end	filter_delconnect
	.size	filter_delconnect, .-filter_delconnect
	.align	2
	.globl	filter_checkConnect
	.set	nomips16
	.set	nomicromips
	.ent	filter_checkConnect
	.type	filter_checkConnect, @function
filter_checkConnect:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	lui	$2,%hi(ConnectedIp)
	andi	$3,$4,0xff
	addiu	$2,$2,%lo(ConnectedIp)
	sll	$3,$3,2
	addu	$3,$3,$2
	lw	$2,0($3)
	beq	$2,$0,$L919
	nop

	xor	$4,$4,$2
	jr	$31
	sltu	$2,$0,$4

$L919:
	jr	$31
	li	$2,1			# 0x1

	.set	macro
	.set	reorder
	.end	filter_checkConnect
	.size	filter_checkConnect, .-filter_checkConnect
	.align	2
	.globl	register_customRspStr
	.set	nomips16
	.set	nomicromips
	.ent	register_customRspStr
	.type	register_customRspStr, @function
register_customRspStr:
	.frame	$sp,32,$31		# vars= 0, regs= 3/0, args= 16, gp= 0
	.mask	0x80030000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$17,24($sp)
	sw	$16,20($sp)
	lui	$16,%hi(block_message)
	move	$17,$4
	lw	$4,%lo(block_message)($16)
	beq	$4,$0,$L921
	nop

	jal	kfree
	nop

	sw	$0,%lo(block_message)($16)
$L921:
	beq	$17,$0,$L922
	nop

	jal	strlen
	move	$4,$17

	li	$5,37748736			# 0x2400000
	addiu	$4,$2,1
	jal	__kmalloc
	addiu	$5,$5,192

	beq	$2,$0,$L920
	sw	$2,%lo(block_message)($16)

	lui	$5,%hi($LC22)
	lw	$31,28($sp)
	move	$6,$17
	lw	$16,20($sp)
	lw	$17,24($sp)
	addiu	$5,$5,%lo($LC22)
	move	$4,$2
	j	sprintf
	addiu	$sp,$sp,32

$L920:
	lw	$31,28($sp)
	lw	$17,24($sp)
	lw	$16,20($sp)
	jr	$31
	addiu	$sp,$sp,32

$L922:
	lui	$17,%hi($LC17)
	jal	strlen
	addiu	$4,$17,%lo($LC17)

	li	$5,37748736			# 0x2400000
	move	$4,$2
	jal	__kmalloc
	addiu	$5,$5,192

	beq	$2,$0,$L920
	sw	$2,%lo(block_message)($16)

	lui	$5,%hi($LC22)
	lw	$31,28($sp)
	addiu	$6,$17,%lo($LC17)
	lw	$16,20($sp)
	lw	$17,24($sp)
	addiu	$5,$5,%lo($LC22)
	move	$4,$2
	j	sprintf
	addiu	$sp,$sp,32

	.set	macro
	.set	reorder
	.end	register_customRspStr
	.size	register_customRspStr, .-register_customRspStr
	.align	2
	.globl	unregister_customRspStr
	.set	nomips16
	.set	nomicromips
	.ent	unregister_customRspStr
	.type	unregister_customRspStr, @function
unregister_customRspStr:
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$16,16($sp)
	lui	$16,%hi(block_message)
	lw	$4,%lo(block_message)($16)
	beq	$4,$0,$L940
	lw	$31,20($sp)

	jal	kfree
	nop

	sw	$0,%lo(block_message)($16)
	lw	$31,20($sp)
$L940:
	lw	$16,16($sp)
	jr	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	unregister_customRspStr
	.size	unregister_customRspStr, .-unregister_customRspStr
	.align	2
	.globl	register_customRspHook
	.set	nomips16
	.set	nomicromips
	.ent	register_customRspHook
	.type	register_customRspHook, @function
register_customRspHook:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L942
	lui	$2,%hi(fast_path_cusRsp401_func)

	sw	$4,%lo(fast_path_cusRsp401_func)($2)
$L943:
	beq	$5,$0,$L944
	lui	$2,%hi(fast_path_cusRspTCPFinAck_func)

	sw	$5,%lo(fast_path_cusRspTCPFinAck_func)($2)
$L945:
	beq	$6,$0,$L946
	lui	$2,%hi(fast_path_cusRspTCPEndAck_func)

	jr	$31
	sw	$6,%lo(fast_path_cusRspTCPEndAck_func)($2)

$L946:
	lui	$2,%hi(GenerateTCPENDACK)
	lui	$3,%hi(fast_path_cusRspTCPEndAck_func)
	addiu	$2,$2,%lo(GenerateTCPENDACK)
	jr	$31
	sw	$2,%lo(fast_path_cusRspTCPEndAck_func)($3)

$L944:
	lui	$2,%hi(GenerateTCPFINACK)
	lui	$3,%hi(fast_path_cusRspTCPFinAck_func)
	addiu	$2,$2,%lo(GenerateTCPFINACK)
	b	$L945
	sw	$2,%lo(fast_path_cusRspTCPFinAck_func)($3)

$L942:
	lui	$2,%hi(GenerateHTTP401)
	lui	$3,%hi(fast_path_cusRsp401_func)
	addiu	$2,$2,%lo(GenerateHTTP401)
	b	$L943
	sw	$2,%lo(fast_path_cusRsp401_func)($3)

	.set	macro
	.set	reorder
	.end	register_customRspHook
	.size	register_customRspHook, .-register_customRspHook
	.align	2
	.globl	unregister_customRspHook
	.set	nomips16
	.set	nomicromips
	.ent	unregister_customRspHook
	.type	unregister_customRspHook, @function
unregister_customRspHook:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	lui	$2,%hi(fast_path_cusRsp401_func)
	sw	$0,%lo(fast_path_cusRsp401_func)($2)
	lui	$2,%hi(fast_path_cusRspTCPFinAck_func)
	sw	$0,%lo(fast_path_cusRspTCPFinAck_func)($2)
	lui	$2,%hi(fast_path_cusRspTCPEndAck_func)
	jr	$31
	sw	$0,%lo(fast_path_cusRspTCPEndAck_func)($2)

	.set	macro
	.set	reorder
	.end	unregister_customRspHook
	.size	unregister_customRspHook, .-unregister_customRspHook
	.align	2
	.globl	filter_enter
	.set	nomips16
	.set	nomicromips
	.ent	filter_enter
	.type	filter_enter, @function
filter_enter:
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
	lui	$18,%hi(dos_item_global)
	lw	$2,%lo(dos_item_global)($18)
	bne	$2,$0,$L950
	lui	$2,%hi(dos_item_ipv6)

	lw	$2,%lo(dos_item_ipv6)($2)
	bne	$2,$0,$L950
	lui	$2,%hi(table_list_head+24)

	lw	$2,%lo(table_list_head+24)($2)
	sltu	$2,$2,2
	bne	$2,$0,$L993
	lui	$3,%hi(none_dos_drop_pkt_cnt)

$L950:
	jal	rtl_get_skb_type
	move	$16,$4

	li	$3,2			# 0x2
	beq	$2,$3,$L989
	lui	$2,%hi(dos_item_ipv6)

	jal	rtl_get_skb_type
	move	$4,$16

	beq	$2,$0,$L990
	nop

$L953:
	li	$17,4			# 0x4
$L949:
	move	$2,$17
$L992:
	lw	$31,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	jr	$31
	addiu	$sp,$sp,32

$L989:
	lw	$2,%lo(dos_item_ipv6)($2)
	andi	$2,$2,0x1
	beq	$2,$0,$L949
	li	$17,4			# 0x4

	jal	filter_dos
	move	$4,$16

	beq	$2,$0,$L953
	nop

$L988:
	b	$L949
	move	$17,$0

$L990:
	jal	rtl_get_skb_protocol
	move	$4,$16

	li	$3,8			# 0x8
	beq	$2,$3,$L957
	lui	$2,%hi(dos_item_ipv6)

	lw	$2,%lo(dos_item_ipv6)($2)
	beq	$2,$0,$L949
	li	$17,4			# 0x4

	jal	rtl_get_skb_protocol
	move	$4,$16

	li	$3,56710			# 0xdd86
	bne	$2,$3,$L949
	li	$17,4			# 0x4

$L957:
	lui	$2,%hi(table_list_head+24)
	lw	$2,%lo(table_list_head+24)($2)
	sltu	$2,$2,2
	bne	$2,$0,$L956
	nop

	jal	FilterWeb_v2
	move	$4,$16

	move	$17,$2
	lui	$2,%hi(fast_path_cusRsp401_func)
	beq	$17,$0,$L958
	lw	$2,%lo(fast_path_cusRsp401_func)($2)

	beq	$2,$0,$L988
	nop

	jalr	$2
	move	$4,$16

	b	$L949
	move	$17,$0

$L956:
	lw	$2,%lo(dos_item_global)($18)
$L994:
	andi	$2,$2,0x1
	bne	$2,$0,$L961
	lui	$2,%hi(dos_item_ipv6)

	lw	$2,%lo(dos_item_ipv6)($2)
	andi	$2,$2,0x1
	beq	$2,$0,$L963
	nop

$L961:
	jal	filter_dos
	move	$4,$16

	bne	$2,$0,$L949
	move	$17,$0

$L963:
	jal	rtl_ip_hdr
	move	$4,$16

	li	$3,1			# 0x1
	lbu	$2,9($2)
	beq	$2,$3,$L953
	lui	$3,%hi(none_dos_drop_pkt_cnt)

$L993:
	li	$17,1			# 0x1
	lw	$2,%lo(none_dos_drop_pkt_cnt)($3)
	addiu	$2,$2,1
	b	$L949
	sw	$2,%lo(none_dos_drop_pkt_cnt)($3)

$L958:
	beq	$2,$0,$L956
	lui	$2,%hi(url_cache)

	li	$3,1			# 0x1
	addiu	$2,$2,%lo(url_cache)
	lbu	$4,15($2)
	beq	$4,$3,$L991
	nop

	lbu	$2,16($2)
	bne	$2,$3,$L994
	lw	$2,%lo(dos_item_global)($18)

	lui	$2,%hi(fast_path_cusRspTCPEndAck_func)
	lw	$2,%lo(fast_path_cusRspTCPEndAck_func)($2)
	beq	$2,$0,$L988
	nop

$L987:
	jalr	$2
	move	$4,$16

	b	$L992
	move	$2,$17

$L991:
	lui	$2,%hi(fast_path_cusRspTCPFinAck_func)
	lw	$2,%lo(fast_path_cusRspTCPFinAck_func)($2)
	bne	$2,$0,$L987
	nop

	b	$L949
	move	$17,$0

	.set	macro
	.set	reorder
	.end	filter_enter
	.size	filter_enter, .-filter_enter
	.local	res3
	.comm	res3,4,4
	.globl	fast_path_cusRspTCPEndAck_func
	.section	.bss,"aw",@nobits
	.align	2
	.type	fast_path_cusRspTCPEndAck_func, @object
	.size	fast_path_cusRspTCPEndAck_func, 4
fast_path_cusRspTCPEndAck_func:
	.space	4
	.globl	fast_path_cusRspTCPFinAck_func
	.align	2
	.type	fast_path_cusRspTCPFinAck_func, @object
	.size	fast_path_cusRspTCPFinAck_func, 4
fast_path_cusRspTCPFinAck_func:
	.space	4
	.globl	fast_path_cusRsp401_func
	.align	2
	.type	fast_path_cusRsp401_func, @object
	.size	fast_path_cusRsp401_func, 4
fast_path_cusRsp401_func:
	.space	4
	.globl	dos_proc_fops
	.data
	.align	2
	.type	dos_proc_fops, @object
	.size	dos_proc_fops, 108
dos_proc_fops:
	.space	4
	.word	seq_lseek
	.word	seq_read
	.word	dos_single_write
	.space	28
	.word	dos_single_open
	.space	4
	.word	single_release
	.space	52
	.globl	___strtok
	.section	.bss
	.align	2
	.type	___strtok, @object
	.size	___strtok, 4
___strtok:
	.space	4
	.local	endStateClientSeq
	.comm	endStateClientSeq,4,4
	.local	endStateClientPort
	.comm	endStateClientPort,2,2
	.local	finStateClientSeq
	.comm	finStateClientSeq,4,4
	.local	finStateClientPort
	.comm	finStateClientPort,2,2
	.local	dos_item_backup
	.comm	dos_item_backup,4,4
	.local	none_dos_drop_pkt_cnt
	.comm	none_dos_drop_pkt_cnt,4,4
	.globl	dos_flag
	.align	2
	.type	dos_flag, @object
	.size	dos_flag, 80
dos_flag:
	.space	80
	.globl	dos_lan_mask
	.align	2
	.type	dos_lan_mask, @object
	.size	dos_lan_mask, 4
dos_lan_mask:
	.space	4
	.globl	dos_lan_addr
	.align	2
	.type	dos_lan_addr, @object
	.size	dos_lan_addr, 4
dos_lan_addr:
	.space	4
	.globl	dos_item_ipv6
	.align	2
	.type	dos_item_ipv6, @object
	.size	dos_item_ipv6, 4
dos_item_ipv6:
	.space	4
	.globl	dos_item_global_0
	.align	2
	.type	dos_item_global_0, @object
	.size	dos_item_global_0, 4
dos_item_global_0:
	.space	4
	.globl	dos_item_global
	.align	2
	.type	dos_item_global, @object
	.size	dos_item_global, 4
dos_item_global:
	.space	4
	.globl	Ipv6_ns_threshold
	.align	2
	.type	Ipv6_ns_threshold, @object
	.size	Ipv6_ns_threshold, 4
Ipv6_ns_threshold:
	.space	4
	.globl	Ipv6_icmp_too_big_threshold
	.align	2
	.type	Ipv6_icmp_too_big_threshold, @object
	.size	Ipv6_icmp_too_big_threshold, 4
Ipv6_icmp_too_big_threshold:
	.space	4
	.globl	Ipv6_smurf_threshold
	.align	2
	.type	Ipv6_smurf_threshold, @object
	.size	Ipv6_smurf_threshold, 4
Ipv6_smurf_threshold:
	.space	4
	.local	block_count
	.comm	block_count,4,4
	.local	block_time
	.comm	block_time,4,4
	.globl	dosFilter_block
	.align	2
	.type	dosFilter_block, @object
	.size	dosFilter_block, 4
dosFilter_block:
	.space	4
	.globl	per_ping_threshold
	.align	2
	.type	per_ping_threshold, @object
	.size	per_ping_threshold, 4
per_ping_threshold:
	.space	4
	.globl	per_icmp_threshold
	.align	2
	.type	per_icmp_threshold, @object
	.size	per_icmp_threshold, 4
per_icmp_threshold:
	.space	4
	.globl	per_udp_threshold
	.align	2
	.type	per_udp_threshold, @object
	.size	per_udp_threshold, 4
per_udp_threshold:
	.space	4
	.globl	per_fin_threshold
	.align	2
	.type	per_fin_threshold, @object
	.size	per_fin_threshold, 4
per_fin_threshold:
	.space	4
	.globl	per_syn_threshold
	.align	2
	.type	per_syn_threshold, @object
	.size	per_syn_threshold, 4
per_syn_threshold:
	.space	4
	.globl	whole_icmp_threshold
	.align	2
	.type	whole_icmp_threshold, @object
	.size	whole_icmp_threshold, 4
whole_icmp_threshold:
	.space	4
	.globl	whole_udp_threshold
	.align	2
	.type	whole_udp_threshold, @object
	.size	whole_udp_threshold, 4
whole_udp_threshold:
	.space	4
	.globl	whole_fin_threshold
	.align	2
	.type	whole_fin_threshold, @object
	.size	whole_fin_threshold, 4
whole_fin_threshold:
	.space	4
	.globl	whole_syn_threshold
	.align	2
	.type	whole_syn_threshold, @object
	.size	whole_syn_threshold, 4
whole_syn_threshold:
	.space	4
	.globl	dos_op_mode
	.type	dos_op_mode, @object
	.size	dos_op_mode, 1
dos_op_mode:
	.space	1
	.local	dos_timer
	.comm	dos_timer,28,4
	.globl	ConnectedIp
	.align	2
	.type	ConnectedIp, @object
	.size	ConnectedIp, 1024
ConnectedIp:
	.space	1024
	.local	url_cache
	.comm	url_cache,20,4
	.local	block_message
	.comm	block_message,4,4
	.globl	dos_read_proc_func
	.align	2
	.type	dos_read_proc_func, @object
	.size	dos_read_proc_func, 4
dos_read_proc_func:
	.space	4
	.globl	dos_timer_ipv6_func
	.align	2
	.type	dos_timer_ipv6_func, @object
	.size	dos_timer_ipv6_func, 4
dos_timer_ipv6_func:
	.space	4
	.globl	dos_timer_ipv4_func
	.align	2
	.type	dos_timer_ipv4_func, @object
	.size	dos_timer_ipv4_func, 4
dos_timer_ipv4_func:
	.space	4
	.globl	filter_dos_ipv6_handler
	.align	2
	.type	filter_dos_ipv6_handler, @object
	.size	filter_dos_ipv6_handler, 4
filter_dos_ipv6_handler:
	.space	4
	.globl	filter_dos_ipv4_handler
	.align	2
	.type	filter_dos_ipv4_handler, @object
	.size	filter_dos_ipv4_handler, 4
filter_dos_ipv4_handler:
	.space	4
	.local	res12
	.comm	res12,4,4
	.globl	filter_table_proc_fops
	.data
	.align	2
	.type	filter_table_proc_fops, @object
	.size	filter_table_proc_fops, 108
filter_table_proc_fops:
	.space	4
	.word	seq_lseek
	.word	seq_read
	.word	filter_table_single_write
	.space	28
	.word	filter_table_single_open
	.space	4
	.word	single_release
	.space	52
	.globl	table_list_head
	.section	.bss
	.align	2
	.type	table_list_head, @object
	.size	table_list_head, 32
table_list_head:
	.space	32
	.local	filter_rule_num
	.comm	filter_rule_num,4,4
	.globl	filter_tlb
	.data
	.align	2
	.type	filter_tlb, @object
	.size	filter_tlb, 40
filter_tlb:
	.word	0
	.word	0
	.word	1
	.word	rtl_ip_range_filter_fastpath
	.word	2
	.word	rtl_mac_filter_fastpath
	.word	3
	.word	rtl_url_filter_fastpath
	.word	4
	.word	rtl_schedule_filter_fastpath
	.local	url_filter_haystack
	.comm	url_filter_haystack,2048,4
	.align	2
	.type	default_policy, @object
	.size	default_policy, 4
default_policy:
	.word	1
	.local	defaultRuleChanged
	.comm	defaultRuleChanged,4,4
	.local	url_filter_flag
	.comm	url_filter_flag,4,4
	.local	url_filter_mode
	.comm	url_filter_mode,4,4
	.local	log_enable
	.comm	log_enable,4,4
	.local	log_info
	.comm	log_info,32,4
	.local	get_info_from_usr_space
	.comm	get_info_from_usr_space,1024,4
	.ident	"GCC: (Realtek MSDK-6.4.1 Build 3055) 6.4.1 20180425"
