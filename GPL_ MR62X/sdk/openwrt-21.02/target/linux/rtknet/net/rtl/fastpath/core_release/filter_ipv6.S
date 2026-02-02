	.file	1 "filter_ipv6.c"
	.section .mdebug.abi32
	.previous
	.nan	legacy
	.module	softfloat
	.module	oddspreg
	.text
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	_rtl_get_ipv6_skip_exthdr
	.type	_rtl_get_ipv6_skip_exthdr, @function
_rtl_get_ipv6_skip_exthdr:
	.frame	$sp,40,$31		# vars= 8, regs= 4/0, args= 16, gp= 0
	.mask	0x80070000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-40
	sw	$31,36($sp)
	sw	$18,32($sp)
	sw	$17,28($sp)
	sw	$16,24($sp)
	lhu	$2,192($4)
	lw	$16,204($4)
	beq	$4,$0,$L4
	addu	$16,$16,$2

	beq	$16,$0,$L4
	li	$3,96			# 0x60

	lbu	$2,0($16)
	andi	$2,$2,0xf0
	bne	$2,$3,$L4
	move	$17,$4

	lbu	$2,6($16)
	move	$18,$5
	move	$4,$2
	jal	ipv6_ext_hdr
	sb	$2,18($sp)

	bne	$2,$0,$L12
	lbu	$2,18($sp)

	bne	$2,$18,$L4
	li	$2,40			# 0x28

	addu	$2,$16,$2
$L13:
	lw	$31,36($sp)
	lw	$18,32($sp)
	lw	$17,28($sp)
	lw	$16,24($sp)
	jr	$31
	addiu	$sp,$sp,40

$L12:
	addiu	$7,$sp,16
	addiu	$6,$sp,18
	li	$5,40			# 0x28
	jal	ipv6_skip_exthdr
	move	$4,$17

	bltz	$2,$L4
	lbu	$3,18($sp)

	beq	$3,$18,$L13
	addu	$2,$16,$2

$L4:
	lw	$31,36($sp)
	move	$2,$0
	lw	$18,32($sp)
	lw	$17,28($sp)
	lw	$16,24($sp)
	jr	$31
	addiu	$sp,$sp,40

	.set	macro
	.set	reorder
	.end	_rtl_get_ipv6_skip_exthdr
	.size	_rtl_get_ipv6_skip_exthdr, .-_rtl_get_ipv6_skip_exthdr
	.align	2
	.globl	rtl_get_tcpv6_header
	.set	nomips16
	.set	nomicromips
	.ent	rtl_get_tcpv6_header
	.type	rtl_get_tcpv6_header, @function
rtl_get_tcpv6_header:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	li	$5,6			# 0x6
	j	_rtl_get_ipv6_skip_exthdr
	nop

	.set	macro
	.set	reorder
	.end	rtl_get_tcpv6_header
	.size	rtl_get_tcpv6_header, .-rtl_get_tcpv6_header
	.align	2
	.globl	dos_filter_ipv6__action
	.set	nomips16
	.set	nomicromips
	.ent	dos_filter_ipv6__action
	.type	dos_filter_ipv6__action, @function
dos_filter_ipv6__action:
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
	lui	$21,%hi(dos_item_ipv6)
	lw	$2,%lo(dos_item_ipv6)($21)
	andi	$3,$2,0xe
	beq	$3,$0,$L36
	andi	$3,$2,0x2

	bne	$3,$0,$L56
	move	$18,$4

	move	$20,$0
$L17:
	andi	$2,$2,0x8
	bne	$2,$0,$L64
	li	$5,58			# 0x3a

$L60:
	lhu	$2,192($18)
	lw	$16,204($18)
	addu	$16,$16,$2
$L24:
	lw	$2,20($16)
	lui	$17,%hi(dos_pkt)
	addiu	$16,$16,8
	addiu	$22,$17,%lo(dos_pkt)
	andi	$2,$2,0x3ff
	sll	$17,$2,5
	li	$6,16			# 0x10
	addiu	$19,$17,16
	move	$4,$16
	addu	$19,$22,$19
	addu	$23,$22,$17
	jal	memcmp
	move	$5,$19

	beq	$2,$0,$L27
	li	$6,16			# 0x10

	lb	$2,0($23)
	beq	$2,$0,$L57
	move	$5,$16

$L27:
	move	$5,$19
	jal	memcmp
	move	$4,$16

	bne	$2,$0,$L15
	addu	$17,$22,$17

	li	$2,1			# 0x1
	lb	$3,0($17)
	beq	$3,$2,$L58
	lui	$16,%hi(cur_p_pkt)

$L15:
	move	$2,$20
$L65:
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

$L36:
	move	$20,$0
	lw	$31,52($sp)
	lw	$23,48($sp)
	move	$2,$20
	lw	$22,44($sp)
	lw	$21,40($sp)
	lw	$20,36($sp)
	lw	$19,32($sp)
	lw	$18,28($sp)
	lw	$17,24($sp)
	lw	$16,20($sp)
	jr	$31
	addiu	$sp,$sp,56

$L56:
	jal	_rtl_get_ipv6_skip_exthdr
	li	$5,58			# 0x3a

	beq	$2,$0,$L22
	nop

	lbu	$3,0($2)
	li	$2,129			# 0x81
	beq	$3,$2,$L59
	lui	$7,%hi(LogIpv6Flag)

$L22:
	lw	$2,%lo(dos_item_ipv6)($21)
	andi	$2,$2,0x8
	beq	$2,$0,$L60
	move	$20,$0

	li	$5,58			# 0x3a
$L64:
	jal	_rtl_get_ipv6_skip_exthdr
	move	$4,$18

	lhu	$3,192($18)
	lw	$16,204($18)
	beq	$2,$0,$L42
	addu	$16,$16,$3

	lbu	$3,0($2)
	li	$2,135			# 0x87
	beq	$3,$2,$L61
	lui	$7,%hi(LogIpv6Flag)

$L42:
	move	$2,$0
	b	$L24
	or	$20,$20,$2

$L58:
	sw	$23,%lo(cur_p_pkt)($16)
$L28:
	lw	$2,%lo(dos_item_ipv6)($21)
	andi	$2,$2,0x4
	beq	$2,$0,$L65
	move	$2,$20

	li	$5,58			# 0x3a
	jal	_rtl_get_ipv6_skip_exthdr
	move	$4,$18

	beq	$2,$0,$L54
	nop

	lbu	$3,0($2)
	li	$2,2			# 0x2
	beq	$3,$2,$L62
	lui	$19,%hi(dosFilter_block)

$L54:
	move	$2,$0
$L30:
	or	$20,$20,$2
$L63:
	move	$2,$20
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

$L59:
	lw	$8,%lo(LogIpv6Flag)($7)
	andi	$2,$8,0x2
	beq	$2,$0,$L66
	lui	$2,%hi(smurf_pkts)

	lui	$3,%hi(dosFilter_block)
	li	$2,1			# 0x1
	lw	$3,%lo(dosFilter_block)($3)
	bne	$3,$2,$L20
	lw	$2,%lo(dos_item_ipv6)($21)

	b	$L17
	li	$20,-1			# 0xffffffffffffffff

$L20:
	lui	$2,%hi(smurf_pkts)
$L66:
	lw	$5,204($18)
	lui	$3,%hi(Ipv6_smurf_threshold)
	lhu	$16,192($18)
	addiu	$2,$2,%lo(smurf_pkts)
	lw	$4,%lo(Ipv6_smurf_threshold)($3)
	lw	$3,4($2)
	addiu	$3,$3,1
	slt	$6,$4,$3
	beq	$6,$0,$L22
	sw	$3,4($2)

	blez	$4,$L22
	addu	$16,$5,$16

	lui	$4,%hi(attack_v6_saddr+16)
	lui	$2,%hi(dosFilter_block)
	li	$3,1			# 0x1
	li	$6,16			# 0x10
	addiu	$5,$16,8
	sw	$3,%lo(dosFilter_block)($2)
	addiu	$4,$4,%lo(attack_v6_saddr+16)
	ori	$8,$8,0x2
	sw	$8,%lo(LogIpv6Flag)($7)
	jal	memcpy
	move	$20,$0

	lui	$4,%hi(attack_v6_daddr+16)
	li	$6,16			# 0x10
	addiu	$5,$16,24
	jal	memcpy
	addiu	$4,$4,%lo(attack_v6_daddr+16)

	b	$L17
	lw	$2,%lo(dos_item_ipv6)($21)

$L61:
	lw	$8,%lo(LogIpv6Flag)($7)
	andi	$2,$8,0x8
	beq	$2,$0,$L67
	lui	$2,%hi(ipv6Ns_pkts)

	lui	$3,%hi(dosFilter_block)
	li	$2,1			# 0x1
	lw	$3,%lo(dosFilter_block)($3)
	bne	$3,$2,$L26
	lui	$2,%hi(ipv6Ns_pkts)

	li	$2,-1			# 0xffffffffffffffff
	b	$L24
	or	$20,$20,$2

$L26:
$L67:
	lui	$3,%hi(Ipv6_ns_threshold)
	addiu	$2,$2,%lo(ipv6Ns_pkts)
	lw	$4,%lo(Ipv6_ns_threshold)($3)
	lw	$3,12($2)
	addiu	$3,$3,1
	slt	$5,$4,$3
	beq	$5,$0,$L42
	sw	$3,12($2)

	blez	$4,$L42
	lui	$4,%hi(attack_v6_saddr+48)

	lui	$2,%hi(dosFilter_block)
	li	$3,1			# 0x1
	addiu	$5,$16,8
	li	$6,16			# 0x10
	sw	$3,%lo(dosFilter_block)($2)
	addiu	$4,$4,%lo(attack_v6_saddr+48)
	ori	$8,$8,0x8
	jal	memcpy
	sw	$8,%lo(LogIpv6Flag)($7)

	lui	$4,%hi(attack_v6_daddr+48)
	addiu	$5,$16,24
	li	$6,16			# 0x10
	jal	memcpy
	addiu	$4,$4,%lo(attack_v6_daddr+48)

	move	$2,$0
	lhu	$3,192($18)
	lw	$16,204($18)
	or	$20,$20,$2
	b	$L24
	addu	$16,$16,$3

$L57:
	move	$4,$19
	jal	memcpy
	lui	$16,%hi(cur_p_pkt)

	li	$2,1			# 0x1
	sw	$23,%lo(cur_p_pkt)($16)
	b	$L28
	sb	$2,0($23)

$L62:
	lhu	$5,192($18)
	lw	$2,204($18)
	li	$3,1			# 0x1
	lw	$17,%lo(cur_p_pkt)($16)
	lw	$4,%lo(dosFilter_block)($19)
	beq	$4,$3,$L32
	addu	$16,$2,$5

	lui	$3,%hi(Ipv6_icmp_too_big_threshold)
$L68:
	lw	$2,8($17)
	lw	$3,%lo(Ipv6_icmp_too_big_threshold)($3)
	addiu	$2,$2,1
	sw	$2,8($17)
	slt	$2,$3,$2
	beq	$2,$0,$L54
	nop

	blez	$3,$L54
	lui	$3,%hi(LogIpv6Flag)

	li	$4,1			# 0x1
	li	$6,16			# 0x10
	lw	$2,%lo(LogIpv6Flag)($3)
	addiu	$5,$16,8
	sw	$4,%lo(dosFilter_block)($19)
	lui	$4,%hi(attack_v6_saddr+32)
	addiu	$4,$4,%lo(attack_v6_saddr+32)
	ori	$2,$2,0x4
	jal	memcpy
	sw	$2,%lo(LogIpv6Flag)($3)

	lui	$4,%hi(attack_v6_daddr+32)
	li	$6,16			# 0x10
	addiu	$5,$16,24
	jal	memcpy
	addiu	$4,$4,%lo(attack_v6_daddr+32)

	b	$L30
	move	$2,$0

$L32:
	lui	$4,%hi(attack_v6_saddr+32)
	li	$6,16			# 0x10
	addiu	$5,$16,8
	jal	memcmp
	addiu	$4,$4,%lo(attack_v6_saddr+32)

	bne	$2,$0,$L68
	lui	$3,%hi(Ipv6_icmp_too_big_threshold)

	li	$2,-1			# 0xffffffffffffffff
	b	$L63
	or	$20,$20,$2

	.set	macro
	.set	reorder
	.end	dos_filter_ipv6__action
	.size	dos_filter_ipv6__action, .-dos_filter_ipv6__action
	.section	.rodata.str1.4,"aMS",@progbits,1
	.align	2
$LC0:
	.ascii	"DoS: ipv6 Smurf Attack source=%04x:%04x:%04x:%04x:%04x:%"
	.ascii	"04x:%04x:%04x destination=%04x:%04x:%04x:%04x:%04x:%04x:"
	.ascii	"%04x:%04x\012\000"
	.align	2
$LC1:
	.ascii	"DoS: ipv6 icmp too big Attack source=%04x:%04x:%04x:%04x"
	.ascii	":%04x:%04x:%04x:%04x destination=%04x:%04x:%04x:%04x:%04"
	.ascii	"x:%04x:%04x:%04x\012\000"
	.align	2
$LC2:
	.ascii	"DoS: ipv6 neighbor solication Attack source=%04x:%04x:%0"
	.ascii	"4x:%04x:%04x:%04x:%04x:%04x destination=%04x:%04x:%04x:%"
	.ascii	"04x:%04x:%04x:%04x:%04x\012\000"
	.text
	.align	2
	.globl	dos_timer_ipv6_handler
	.set	nomips16
	.set	nomicromips
	.ent	dos_timer_ipv6_handler
	.type	dos_timer_ipv6_handler, @function
dos_timer_ipv6_handler:
	.frame	$sp,96,$31		# vars= 0, regs= 5/0, args= 72, gp= 0
	.mask	0x800f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-96
	sw	$31,92($sp)
	sw	$19,88($sp)
	sw	$18,84($sp)
	sw	$17,80($sp)
	sw	$16,76($sp)
	lui	$2,%hi(dos_item_ipv6)
	lw	$11,%lo(dos_item_ipv6)($2)
	andi	$2,$11,0xe
	beq	$2,$0,$L103
	move	$6,$0

	lui	$16,%hi(dos_pkt)
	lui	$4,%hi(dos_pkt+32768)
	move	$7,$0
	addiu	$2,$16,%lo(dos_pkt)
	addiu	$4,$4,%lo(dos_pkt+32768)
	li	$5,1			# 0x1
	andi	$9,$11,0x4
	b	$L70
	andi	$8,$11,0x8

$L72:
	addiu	$2,$2,32
	beq	$2,$4,$L104
	nop

$L70:
	lb	$3,0($2)
	bne	$3,$5,$L72
	nop

	beq	$9,$0,$L73
	nop

	lw	$3,8($2)
	addu	$10,$7,$3
	slt	$3,$0,$3
	movn	$7,$10,$3
$L73:
	beq	$8,$0,$L72
	nop

	lw	$3,12($2)
	addiu	$2,$2,32
	addu	$10,$6,$3
	slt	$3,$0,$3
	bne	$2,$4,$L70
	movn	$6,$10,$3

$L104:
	andi	$2,$11,0x2
	bne	$2,$0,$L74
	lui	$17,%hi(smurf_pkts)

	lui	$19,%hi(LogIpv6Flag)
$L108:
	lw	$18,%lo(LogIpv6Flag)($19)
$L75:
	lui	$2,%hi(Ipv6_icmp_too_big_threshold)
	lw	$2,%lo(Ipv6_icmp_too_big_threshold)($2)
	slt	$7,$2,$7
	beq	$7,$0,$L77
	andi	$3,$11,0x4

	beq	$3,$0,$L77
	nop

	blez	$2,$L105
	lui	$2,%hi(Ipv6_ns_threshold)

	ori	$18,$18,0x4
	sw	$18,%lo(LogIpv6Flag)($19)
$L77:
	lui	$2,%hi(Ipv6_ns_threshold)
$L105:
	lw	$2,%lo(Ipv6_ns_threshold)($2)
	slt	$6,$2,$6
	beq	$6,$0,$L71
	andi	$11,$11,0x8

	beq	$11,$0,$L71
	nop

	blez	$2,$L106
	andi	$2,$18,0x2

	ori	$18,$18,0x8
	sw	$18,%lo(LogIpv6Flag)($19)
$L71:
	andi	$2,$18,0x2
$L106:
	beq	$2,$0,$L107
	andi	$2,$18,0x4

	lui	$3,%hi(attack_v6_saddr)
	lui	$2,%hi(attack_v6_daddr)
	addiu	$3,$3,%lo(attack_v6_saddr)
	addiu	$2,$2,%lo(attack_v6_daddr)
	lui	$4,%hi($LC0)
	lhu	$31,22($3)
	lhu	$25,24($3)
	addiu	$4,$4,%lo($LC0)
	lhu	$24,26($3)
	lhu	$5,16($3)
	wsbh	$31,$31
	lhu	$6,18($3)
	wsbh	$25,$25
	lhu	$7,20($3)
	wsbh	$24,$24
	lhu	$15,28($3)
	wsbh	$5,$5
	lhu	$14,30($3)
	wsbh	$6,$6
	lhu	$13,16($2)
	wsbh	$7,$7
	lhu	$12,18($2)
	wsbh	$15,$15
	lhu	$11,20($2)
	wsbh	$14,$14
	lhu	$10,22($2)
	wsbh	$13,$13
	lhu	$9,24($2)
	wsbh	$12,$12
	lhu	$8,26($2)
	wsbh	$11,$11
	lhu	$3,28($2)
	wsbh	$10,$10
	lhu	$2,30($2)
	wsbh	$9,$9
	wsbh	$8,$8
	wsbh	$3,$3
	wsbh	$2,$2
	andi	$31,$31,0xffff
	andi	$25,$25,0xffff
	sw	$31,16($sp)
	andi	$24,$24,0xffff
	sw	$25,20($sp)
	andi	$15,$15,0xffff
	sw	$24,24($sp)
	andi	$14,$14,0xffff
	sw	$15,28($sp)
	andi	$13,$13,0xffff
	sw	$14,32($sp)
	andi	$12,$12,0xffff
	sw	$13,36($sp)
	andi	$11,$11,0xffff
	sw	$12,40($sp)
	andi	$10,$10,0xffff
	sw	$11,44($sp)
	andi	$9,$9,0xffff
	sw	$10,48($sp)
	andi	$8,$8,0xffff
	sw	$9,52($sp)
	andi	$3,$3,0xffff
	sw	$8,56($sp)
	andi	$2,$2,0xffff
	sw	$3,60($sp)
	andi	$7,$7,0xffff
	sw	$2,64($sp)
	andi	$6,$6,0xffff
	jal	scrlog_printk
	andi	$5,$5,0xffff

	andi	$2,$18,0x4
$L107:
	beq	$2,$0,$L79
	lui	$3,%hi(attack_v6_saddr)

	lui	$2,%hi(attack_v6_daddr)
	addiu	$3,$3,%lo(attack_v6_saddr)
	addiu	$2,$2,%lo(attack_v6_daddr)
	lui	$4,%hi($LC1)
	lhu	$31,38($3)
	lhu	$25,40($3)
	addiu	$4,$4,%lo($LC1)
	lhu	$24,42($3)
	lhu	$5,32($3)
	wsbh	$31,$31
	lhu	$6,34($3)
	wsbh	$25,$25
	lhu	$7,36($3)
	wsbh	$24,$24
	lhu	$15,44($3)
	wsbh	$5,$5
	lhu	$14,46($3)
	wsbh	$6,$6
	lhu	$13,32($2)
	wsbh	$7,$7
	lhu	$12,34($2)
	wsbh	$15,$15
	lhu	$11,36($2)
	wsbh	$14,$14
	lhu	$10,38($2)
	wsbh	$13,$13
	lhu	$9,40($2)
	wsbh	$12,$12
	lhu	$8,42($2)
	wsbh	$11,$11
	lhu	$3,44($2)
	wsbh	$10,$10
	lhu	$2,46($2)
	wsbh	$9,$9
	wsbh	$8,$8
	wsbh	$3,$3
	wsbh	$2,$2
	andi	$31,$31,0xffff
	andi	$25,$25,0xffff
	sw	$31,16($sp)
	andi	$24,$24,0xffff
	sw	$25,20($sp)
	andi	$15,$15,0xffff
	sw	$24,24($sp)
	andi	$14,$14,0xffff
	sw	$15,28($sp)
	andi	$13,$13,0xffff
	sw	$14,32($sp)
	andi	$12,$12,0xffff
	sw	$13,36($sp)
	andi	$11,$11,0xffff
	sw	$12,40($sp)
	andi	$10,$10,0xffff
	sw	$11,44($sp)
	andi	$9,$9,0xffff
	sw	$10,48($sp)
	andi	$8,$8,0xffff
	sw	$9,52($sp)
	andi	$3,$3,0xffff
	sw	$8,56($sp)
	andi	$2,$2,0xffff
	sw	$3,60($sp)
	andi	$7,$7,0xffff
	sw	$2,64($sp)
	andi	$6,$6,0xffff
	jal	scrlog_printk
	andi	$5,$5,0xffff

$L79:
	andi	$18,$18,0x8
	beq	$18,$0,$L80
	lui	$3,%hi(attack_v6_saddr)

	lui	$2,%hi(attack_v6_daddr)
	addiu	$3,$3,%lo(attack_v6_saddr)
	addiu	$2,$2,%lo(attack_v6_daddr)
	lui	$4,%hi($LC2)
	lhu	$18,54($3)
	lhu	$25,56($3)
	addiu	$4,$4,%lo($LC2)
	lhu	$24,58($3)
	lhu	$5,48($3)
	wsbh	$18,$18
	lhu	$6,50($3)
	wsbh	$25,$25
	lhu	$7,52($3)
	wsbh	$24,$24
	lhu	$15,60($3)
	wsbh	$5,$5
	lhu	$14,62($3)
	wsbh	$6,$6
	lhu	$13,48($2)
	wsbh	$7,$7
	lhu	$12,50($2)
	wsbh	$15,$15
	lhu	$11,52($2)
	wsbh	$14,$14
	lhu	$10,54($2)
	wsbh	$13,$13
	lhu	$9,56($2)
	wsbh	$12,$12
	lhu	$8,58($2)
	wsbh	$11,$11
	lhu	$3,60($2)
	wsbh	$10,$10
	lhu	$2,62($2)
	wsbh	$9,$9
	wsbh	$8,$8
	wsbh	$3,$3
	wsbh	$2,$2
	andi	$18,$18,0xffff
	andi	$25,$25,0xffff
	sw	$18,16($sp)
	andi	$24,$24,0xffff
	sw	$25,20($sp)
	andi	$15,$15,0xffff
	sw	$24,24($sp)
	andi	$14,$14,0xffff
	sw	$15,28($sp)
	andi	$13,$13,0xffff
	sw	$14,32($sp)
	andi	$12,$12,0xffff
	sw	$13,36($sp)
	andi	$11,$11,0xffff
	sw	$12,40($sp)
	andi	$10,$10,0xffff
	sw	$11,44($sp)
	andi	$9,$9,0xffff
	sw	$10,48($sp)
	andi	$8,$8,0xffff
	sw	$9,52($sp)
	andi	$3,$3,0xffff
	sw	$8,56($sp)
	andi	$2,$2,0xffff
	sw	$3,60($sp)
	andi	$7,$7,0xffff
	sw	$2,64($sp)
	andi	$6,$6,0xffff
	jal	scrlog_printk
	andi	$5,$5,0xffff

$L80:
	addiu	$4,$16,%lo(dos_pkt)
	sw	$0,%lo(LogIpv6Flag)($19)
	li	$6,32768			# 0x8000
	jal	memset
	move	$5,$0

	addiu	$4,$17,%lo(smurf_pkts)
	lw	$31,92($sp)
	li	$6,32			# 0x20
	lw	$19,88($sp)
	move	$5,$0
	lw	$18,84($sp)
	lw	$17,80($sp)
	lw	$16,76($sp)
	j	memset
	addiu	$sp,$sp,96

$L74:
	addiu	$2,$17,%lo(smurf_pkts)
	lui	$3,%hi(Ipv6_smurf_threshold)
	lw	$4,%lo(Ipv6_smurf_threshold)($3)
	lw	$3,4($2)
	slt	$2,$3,0
	movn	$3,$0,$2
	slt	$2,$4,$3
	beq	$2,$0,$L108
	lui	$19,%hi(LogIpv6Flag)

	blez	$4,$L108
	lw	$18,%lo(LogIpv6Flag)($19)

	ori	$18,$18,0x2
	b	$L75
	sw	$18,%lo(LogIpv6Flag)($19)

$L103:
	lui	$19,%hi(LogIpv6Flag)
	lui	$16,%hi(dos_pkt)
	lui	$17,%hi(smurf_pkts)
	b	$L71
	lw	$18,%lo(LogIpv6Flag)($19)

	.set	macro
	.set	reorder
	.end	dos_timer_ipv6_handler
	.size	dos_timer_ipv6_handler, .-dos_timer_ipv6_handler
	.align	2
	.globl	clear_attack_address_ipv6
	.set	nomips16
	.set	nomicromips
	.ent	clear_attack_address_ipv6
	.type	clear_attack_address_ipv6, @function
clear_attack_address_ipv6:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	lui	$4,%hi(attack_v6_saddr)
	li	$6,64			# 0x40
	move	$5,$0
	jal	memset
	addiu	$4,$4,%lo(attack_v6_saddr)

	lui	$4,%hi(attack_v6_daddr)
	lw	$31,20($sp)
	li	$6,64			# 0x40
	move	$5,$0
	addiu	$4,$4,%lo(attack_v6_daddr)
	j	memset
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	clear_attack_address_ipv6
	.size	clear_attack_address_ipv6, .-clear_attack_address_ipv6
	.local	ipv6Ns_pkts
	.comm	ipv6Ns_pkts,32,4
	.local	smurf_pkts
	.comm	smurf_pkts,32,4
	.local	LogIpv6Flag
	.comm	LogIpv6Flag,4,4
	.local	attack_v6_daddr
	.comm	attack_v6_daddr,64,4
	.local	attack_v6_saddr
	.comm	attack_v6_saddr,64,4
	.local	cur_p_pkt
	.comm	cur_p_pkt,4,4
	.local	dos_pkt
	.comm	dos_pkt,32768,4
	.ident	"GCC: (Realtek MSDK-6.4.1 Build 3055) 6.4.1 20180425"
