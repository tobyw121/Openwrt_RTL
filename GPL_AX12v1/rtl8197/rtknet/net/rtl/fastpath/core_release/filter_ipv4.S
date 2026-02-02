	.file	1 "filter_ipv4.c"
	.section .mdebug.abi32
	.previous
	.nan	legacy
	.module	softfloat
	.module	oddspreg
	.section	.rodata.str1.4,"aMS",@progbits,1
	.align	2
$LC0:
	.ascii	"%u.%u.%u.%u\000"
	.text
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	IP2NAME
	.type	IP2NAME, @function
IP2NAME:
	.frame	$sp,32,$31		# vars= 0, regs= 2/0, args= 24, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$16,24($sp)
	li	$6,16			# 0x10
	move	$16,$5
	sw	$4,32($sp)
	move	$5,$0
	jal	memset
	move	$4,$16

	lui	$5,%hi($LC0)
	lbu	$3,35($sp)
	move	$4,$16
	lbu	$2,34($sp)
	addiu	$5,$5,%lo($LC0)
	lbu	$7,33($sp)
	lbu	$6,32($sp)
	sw	$3,20($sp)
	jal	sprintf
	sw	$2,16($sp)

	move	$2,$16
	lw	$31,28($sp)
	lw	$16,24($sp)
	jr	$31
	addiu	$sp,$sp,32

	.set	macro
	.set	reorder
	.end	IP2NAME
	.size	IP2NAME, .-IP2NAME
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	dos_pkt_locate.isra.0
	.type	dos_pkt_locate.isra.0, @function
dos_pkt_locate.isra.0:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	lui	$3,%hi(ConnectedIp)
	andi	$6,$4,0xff
	addiu	$3,$3,%lo(ConnectedIp)
	sll	$6,$6,2
	andi	$7,$4,0x3ff
	addu	$6,$6,$3
	sll	$5,$7,2
	lui	$2,%hi(dos_pkt)
	addu	$3,$5,$7
	lw	$6,0($6)
	addiu	$2,$2,%lo(dos_pkt)
	sll	$3,$3,3
	beq	$6,$4,$L11
	addu	$3,$2,$3

	lw	$6,28($3)
	beq	$4,$6,$L7
	nop

$L5:
	addu	$5,$5,$7
	sll	$5,$5,3
	addu	$5,$2,$5
	lb	$2,0($5)
	beq	$2,$0,$L12
	nop

$L9:
	jr	$31
	move	$2,$0

$L12:
	sw	$4,28($5)
	li	$4,1			# 0x1
	li	$2,1			# 0x1
	sb	$4,0($5)
	lui	$4,%hi(cur_p_pkt)
	jr	$31
	sw	$3,%lo(cur_p_pkt)($4)

$L7:
	lb	$4,0($3)
	li	$2,1			# 0x1
	bne	$4,$2,$L9
	nop

	lui	$4,%hi(cur_p_pkt)
	jr	$31
	sw	$3,%lo(cur_p_pkt)($4)

$L11:
	lw	$8,28($3)
	bne	$8,$6,$L5
	nop

	sb	$0,0($3)
	jr	$31
	move	$2,$0

	.set	macro
	.set	reorder
	.end	dos_pkt_locate.isra.0
	.size	dos_pkt_locate.isra.0, .-dos_pkt_locate.isra.0
	.section	.rodata.str1.4
	.align	2
$LC1:
	.ascii	"%s\012\000"
	.align	2
$LC2:
	.ascii	"connectip = %s\012\000"
	.align	2
$LC3:
	.ascii	"pkt---IP:%s, syn_cnt:%d, fin_cnt:%d, udp_nct:%d, icmp_cn"
	.ascii	"t:%d, scan_cnt:%d\012\000"
	.align	2
$LC4:
	.ascii	"attack_saddr[%s]=%s,attack_daddr[%s]=%s\012\000"
	.align	2
$LC5:
	.ascii	"op_mode = %d, lan_addr = %s, lan_mask = %s, dos_item_glo"
	.ascii	"bal = 0x%x,dos_item_global_0 = 0x%x\012\000"
	.text
	.align	2
	.globl	dos_read_proc_handler
	.set	nomips16
	.set	nomicromips
	.ent	dos_read_proc_handler
	.type	dos_read_proc_handler, @function
dos_read_proc_handler:
	.frame	$sp,88,$31		# vars= 16, regs= 10/0, args= 32, gp= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-88
	sw	$31,84($sp)
	sw	$fp,80($sp)
	sw	$23,76($sp)
	sw	$22,72($sp)
	sw	$21,68($sp)
	sw	$20,64($sp)
	sw	$19,60($sp)
	sw	$18,56($sp)
	sw	$17,52($sp)
	sw	$16,48($sp)
	lui	$6,%hi(dos_flag)
	lui	$5,%hi($LC1)
	addiu	$6,$6,%lo(dos_flag)
	addiu	$5,$5,%lo($LC1)
	move	$17,$4
	.set	noreorder
	.set	nomacro
	jal	seq_printf
	lui	$16,%hi(ConnectedIp)
	.set	macro
	.set	reorder

	lui	$19,%hi(ConnectedIp+1024)
	lui	$2,%hi(ip_name0)
	lui	$20,%hi($LC2)
	sw	$2,36($sp)
	addiu	$16,$16,%lo(ConnectedIp)
	addiu	$19,$19,%lo(ConnectedIp+1024)
	addiu	$21,$2,%lo(ip_name0)
	.set	noreorder
	.set	nomacro
	b	$L15
	addiu	$20,$20,%lo($LC2)
	.set	macro
	.set	reorder

$L14:
	addiu	$16,$16,4
	beq	$16,$19,$L16
$L15:
	lw	$4,0($16)
	beq	$4,$0,$L14
	.set	noreorder
	.set	nomacro
	jal	IP2NAME
	move	$5,$21
	.set	macro
	.set	reorder

	addiu	$16,$16,4
	move	$6,$2
	move	$5,$20
	.set	noreorder
	.set	nomacro
	jal	seq_printf
	move	$4,$17
	.set	macro
	.set	reorder

	bne	$16,$19,$L15
$L16:
	lw	$2,16($28)
	addiu	$2,$2,512
	sw	$2,16($28)
	lw	$2,36($sp)
	lui	$16,%hi(dos_pkt)
	lui	$19,%hi(dos_pkt+40960)
	lui	$21,%hi($LC3)
	addiu	$16,$16,%lo(dos_pkt)
	addiu	$19,$19,%lo(dos_pkt+40960)
	li	$20,1			# 0x1
	addiu	$22,$2,%lo(ip_name0)
	.set	noreorder
	.set	nomacro
	b	$L18
	addiu	$21,$21,%lo($LC3)
	.set	macro
	.set	reorder

$L17:
	addiu	$16,$16,40
	.set	noreorder
	.set	nomacro
	beq	$16,$19,$L35
	lui	$3,%hi(rtl_dos_name)
	.set	macro
	.set	reorder

$L18:
	lb	$2,0($16)
	.set	noreorder
	.set	nomacro
	bne	$2,$20,$L17
	move	$5,$22
	.set	macro
	.set	reorder

	lw	$4,28($16)
	.set	noreorder
	.set	nomacro
	jal	IP2NAME
	addiu	$16,$16,40
	.set	macro
	.set	reorder

	move	$5,$21
	lw	$9,-16($16)
	move	$6,$2
	lw	$8,-24($16)
	move	$4,$17
	lw	$3,-28($16)
	lw	$2,-32($16)
	lw	$7,-36($16)
	sw	$9,28($sp)
	sw	$8,24($sp)
	sw	$3,20($sp)
	.set	noreorder
	.set	nomacro
	jal	seq_printf
	sw	$2,16($sp)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$16,$19,$L18
	lui	$3,%hi(rtl_dos_name)
	.set	macro
	.set	reorder

$L35:
	lui	$2,%hi(ip_name1)
	addiu	$22,$3,%lo(rtl_dos_name)
	lw	$3,36($sp)
	lui	$16,%hi(attack_daddr)
	sw	$2,40($sp)
	lui	$19,%hi(attack_saddr)
	lui	$20,%hi(attack_daddr+108)
	lui	$21,%hi($LC4)
	addiu	$16,$16,%lo(attack_daddr)
	addiu	$19,$19,%lo(attack_saddr)
	addiu	$20,$20,%lo(attack_daddr+108)
	addiu	$fp,$3,%lo(ip_name0)
	addiu	$23,$2,%lo(ip_name1)
	addiu	$21,$21,%lo($LC4)
$L22:
	lw	$2,0($16)
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L20
	lw	$4,0($19)
	.set	macro
	.set	reorder

	beq	$4,$0,$L21
$L20:
	.set	noreorder
	.set	nomacro
	jal	IP2NAME
	move	$5,$fp
	.set	macro
	.set	reorder

	move	$5,$23
	lw	$4,0($16)
	lw	$18,0($22)
	.set	noreorder
	.set	nomacro
	jal	IP2NAME
	sw	$2,32($sp)
	.set	macro
	.set	reorder

	move	$5,$21
	lw	$7,32($sp)
	move	$6,$18
	sw	$2,20($sp)
	move	$4,$17
	.set	noreorder
	.set	nomacro
	jal	seq_printf
	sw	$18,16($sp)
	.set	macro
	.set	reorder

$L21:
	addiu	$16,$16,4
	addiu	$19,$19,4
	.set	noreorder
	.set	nomacro
	bne	$16,$20,$L22
	addiu	$22,$22,4
	.set	macro
	.set	reorder

$L23:
	lui	$4,%hi($L23)
	li	$5,512			# 0x200
	.set	noreorder
	.set	nomacro
	jal	__local_bh_enable_ip
	addiu	$4,$4,%lo($L23)
	.set	macro
	.set	reorder

	lw	$2,36($sp)
	addiu	$5,$2,%lo(ip_name0)
	lui	$2,%hi(dos_lan_addr)
	lw	$4,%lo(dos_lan_addr)($2)
	lui	$2,%hi(dos_op_mode)
	.set	noreorder
	.set	nomacro
	jal	IP2NAME
	lb	$16,%lo(dos_op_mode)($2)
	.set	macro
	.set	reorder

	move	$18,$2
	lw	$2,40($sp)
	addiu	$5,$2,%lo(ip_name1)
	lui	$2,%hi(dos_lan_mask)
	.set	noreorder
	.set	nomacro
	jal	IP2NAME
	lw	$4,%lo(dos_lan_mask)($2)
	.set	macro
	.set	reorder

	lui	$5,%hi($LC5)
	sw	$2,16($sp)
	lui	$2,%hi(dos_item_global_0)
	addiu	$5,$5,%lo($LC5)
	move	$7,$18
	lw	$3,%lo(dos_item_global_0)($2)
	lui	$2,%hi(dos_item_global)
	move	$6,$16
	move	$4,$17
	lw	$2,%lo(dos_item_global)($2)
	sw	$3,24($sp)
	.set	noreorder
	.set	nomacro
	jal	seq_printf
	sw	$2,20($sp)
	.set	macro
	.set	reorder

	lw	$31,84($sp)
	lw	$fp,80($sp)
	lw	$23,76($sp)
	lw	$22,72($sp)
	lw	$21,68($sp)
	lw	$20,64($sp)
	lw	$19,60($sp)
	lw	$18,56($sp)
	lw	$17,52($sp)
	lw	$16,48($sp)
	.set	noreorder
	.set	nomacro
	jr	$31
	addiu	$sp,$sp,88
	.set	macro
	.set	reorder

	.end	dos_read_proc_handler
	.size	dos_read_proc_handler, .-dos_read_proc_handler
	.align	2
	.globl	clear_attack_address_ipv4
	.set	nomips16
	.set	nomicromips
	.ent	clear_attack_address_ipv4
	.type	clear_attack_address_ipv4, @function
clear_attack_address_ipv4:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	lui	$2,%hi(attack_saddr)
	lui	$3,%hi(attack_daddr)
	lui	$4,%hi(attack_saddr+108)
	addiu	$2,$2,%lo(attack_saddr)
	addiu	$3,$3,%lo(attack_daddr)
	addiu	$4,$4,%lo(attack_saddr+108)
$L37:
	sw	$0,0($2)
	addiu	$2,$2,4
	sw	$0,0($3)
	bne	$2,$4,$L37
	addiu	$3,$3,4

	jr	$31
	nop

	.set	macro
	.set	reorder
	.end	clear_attack_address_ipv4
	.size	clear_attack_address_ipv4, .-clear_attack_address_ipv4
	.align	2
	.globl	filter_out
	.set	nomips16
	.set	nomicromips
	.ent	filter_out
	.type	filter_out, @function
filter_out:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	jal	rtl_ip_hdr
	nop

	li	$3,1			# 0x1
	lbu	$4,9($2)
	beq	$4,$3,$L42
	li	$5,8			# 0x8

	lw	$31,20($sp)
$L43:
	move	$2,$0
	jr	$31
	addiu	$sp,$sp,24

$L42:
	lw	$3,0($2)
	andi	$3,$3,0xf
	sll	$3,$3,2
	addu	$2,$2,$3
	lbu	$2,0($2)
	bne	$2,$5,$L43
	lw	$31,20($sp)

	lui	$2,%hi(jiffies)
	lw	$3,%lo(jiffies)($2)
	lui	$2,%hi(icmp_echo_request)
	sw	$4,%lo(icmp_echo_request)($2)
	lui	$2,%hi(icmp_echo_request_time)
	sw	$3,%lo(icmp_echo_request_time)($2)
	move	$2,$0
	jr	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	filter_out
	.size	filter_out, .-filter_out
	.align	2
	.globl	dos_filter_ipv4__action
	.set	nomips16
	.set	nomicromips
	.ent	dos_filter_ipv4__action
	.type	dos_filter_ipv4__action, @function
dos_filter_ipv4__action:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	jal	rtl_ip_hdr
	nop

	lui	$12,%hi(dos_item_global)
	lw	$10,0($2)
	move	$9,$2
	lw	$2,%lo(dos_item_global)($12)
	andi	$10,$10,0xf
	sll	$10,$10,2
	andi	$3,$2,0x3fe
	bne	$3,$0,$L274
	addu	$10,$9,$10

	move	$11,$0
$L45:
	lui	$3,%hi(dos_item_global_0)
	lw	$3,%lo(dos_item_global_0)($3)
	andi	$4,$3,0x1
	bne	$4,$0,$L275
	li	$4,6			# 0x6

$L84:
	andi	$3,$3,0x2
$L312:
	bne	$3,$0,$L276
	li	$4,17			# 0x11

$L98:
	li	$3,536870912			# 0x20000000
$L318:
	and	$4,$2,$3
	bne	$4,$0,$L277
	li	$5,1			# 0x1

$L102:
	li	$3,1073741824			# 0x40000000
$L319:
	and	$4,$2,$3
	beq	$4,$0,$L107
	li	$4,1			# 0x1

	lbu	$5,9($9)
	beq	$5,$4,$L278
	li	$5,8			# 0x8

$L107:
	bltz	$2,$L279
	li	$3,1			# 0x1

$L109:
	andi	$3,$2,0x8000
$L326:
	beq	$3,$0,$L313
	li	$3,65536			# 0x10000

	lbu	$4,9($9)
	li	$3,6			# 0x6
	beq	$4,$3,$L280
	lui	$4,%hi(_tcpDosScanBitmap)

$L111:
	li	$3,65536			# 0x10000
$L313:
	and	$4,$2,$3
	bne	$4,$0,$L281
	li	$4,6			# 0x6

$L113:
	andi	$3,$2,0x800
	beq	$3,$0,$L314
	ext	$3,$2,18,1

	lw	$3,12($9)
	lw	$4,16($9)
	beq	$3,$4,$L282
	lui	$5,%hi(LogFlag)

$L116:
	ext	$3,$2,18,1
$L314:
	bne	$3,$0,$L283
	li	$3,17			# 0x11

$L118:
	li	$3,131072			# 0x20000
$L325:
	and	$4,$2,$3
	bne	$4,$0,$L284
	li	$4,17			# 0x11

$L121:
	andi	$3,$2,0x4000
	bne	$3,$0,$L285
	nop

$L123:
	andi	$3,$2,0x400
$L324:
	bne	$3,$0,$L286
	li	$3,1			# 0x1

$L125:
	li	$3,524288			# 0x80000
$L323:
	and	$4,$2,$3
	bne	$4,$0,$L287
	li	$4,17			# 0x11

$L127:
	ext	$3,$2,24,1
	bne	$3,$0,$L288
	li	$3,1			# 0x1

$L129:
	ext	$3,$2,25,1
$L322:
	beq	$3,$0,$L315
	ext	$3,$2,26,1

	lbu	$4,9($9)
	li	$3,17			# 0x11
	beq	$4,$3,$L289
	li	$4,1792			# 0x700

$L132:
	ext	$3,$2,26,1
$L315:
	beq	$3,$0,$L316
	li	$3,134217728			# 0x8000000

	lbu	$4,9($9)
	li	$3,6			# 0x6
	beq	$4,$3,$L290
	nop

$L135:
	li	$3,134217728			# 0x8000000
$L316:
	and	$4,$2,$3
	beq	$4,$0,$L138
	li	$4,6			# 0x6

	lbu	$5,9($9)
	beq	$5,$4,$L291
	nop

$L138:
	ext	$3,$2,28,1
	bne	$3,$0,$L292
	li	$3,224			# 0xe0

$L140:
	andi	$3,$2,0x1000
$L320:
	bne	$3,$0,$L293
	lui	$5,%hi(dos_lan_addr)

$L143:
	andi	$2,$2,0x2000
	bne	$2,$0,$L294
	nop

$L44:
	lw	$31,20($sp)
$L317:
	move	$2,$11
	jr	$31
	addiu	$sp,$sp,24

$L294:
	jal	dos_pkt_locate.isra.0
	lw	$4,12($9)

	beq	$2,$0,$L44
	lui	$2,%hi(cur_p_pkt)

	lhu	$3,4($9)
	lw	$4,%lo(cur_p_pkt)($2)
	wsbh	$3,$3
	andi	$5,$3,0xffff
	lhu	$2,32($4)
	beq	$5,$2,$L147
	nop

	bne	$2,$0,$L317
	lw	$31,20($sp)

$L147:
	lhu	$2,6($9)
	wsbh	$2,$2
	andi	$2,$2,0xffff
	andi	$5,$2,0x3fff
	beq	$5,$0,$L44
	sll	$5,$2,3

	lhu	$6,34($4)
	andi	$5,$5,0xffff
	slt	$5,$5,$6
	bne	$5,$0,$L148
	andi	$2,$2,0x2000

	beq	$2,$0,$L295
	nop

	sh	$3,32($4)
	lw	$2,0($9)
	lhu	$3,2($9)
	sll	$2,$2,2
	wsbh	$3,$3
	andi	$2,$2,0x3c
	subu	$6,$6,$2
	addu	$6,$6,$3
	b	$L44
	sh	$6,34($4)

$L276:
	lbu	$3,9($9)
	beq	$3,$4,$L99
	li	$4,6			# 0x6

	bne	$3,$4,$L318
	li	$3,536870912			# 0x20000000

$L99:
	lhu	$3,2($10)
	li	$4,1792			# 0x700
	beq	$3,$4,$L101
	li	$6,-513			# 0xfffffffffffffdff

	li	$5,4352			# 0x1100
	and	$3,$3,$6
	beq	$3,$5,$L101
	nop

	lhu	$3,0($10)
	beq	$3,$4,$L101
	and	$6,$3,$6

	bne	$6,$5,$L318
	li	$3,536870912			# 0x20000000

$L101:
	lw	$3,12($9)
	lw	$4,16($9)
	bne	$3,$4,$L98
	lui	$4,%hi(attack_saddr+104)

	li	$11,-1			# 0xffffffffffffffff
	sw	$3,%lo(attack_saddr+104)($4)
	lui	$3,%hi(attack_daddr+104)
	lw	$4,16($9)
	b	$L98
	sw	$4,%lo(attack_daddr+104)($3)

$L275:
	lbu	$5,9($9)
	bne	$5,$4,$L84
	nop

	lw	$5,0($9)
	andi	$5,$5,0xf
	sll	$5,$5,2
	addu	$5,$9,$5
	lbu	$6,12($5)
	sra	$6,$6,2
	andi	$6,$6,0x3c
	sltu	$4,$6,21
	bne	$4,$0,$L84
	addiu	$5,$5,20

	lui	$12,%hi($L88)
	addiu	$6,$6,-20
	move	$7,$0
	addiu	$12,$12,%lo($L88)
	li	$15,10			# 0xa
	li	$14,3			# 0x3
	li	$13,4			# 0x4
$L97:
	lbu	$4,0($5)
	sltu	$8,$4,9
	beq	$8,$0,$L86
	sll	$4,$4,2

	addu	$4,$12,$4
	lw	$4,0($4)
	jr	$4
	nop

	.rdata
	.align	2
	.align	2
$L88:
	.word	$L89
	.word	$L89
	.word	$L90
	.word	$L91
	.word	$L86
	.word	$L86
	.word	$L86
	.word	$L86
	.word	$L92
	.text
$L277:
	lbu	$4,9($9)
	beq	$4,$5,$L296
	nop

	li	$3,17			# 0x11
	beq	$4,$3,$L105
	nop

	li	$3,6			# 0x6
	bne	$4,$3,$L102
	li	$4,20			# 0x14

	lhu	$3,2($9)
$L271:
	lw	$5,0($9)
	wsbh	$3,$3
	andi	$3,$3,0xffff
	andi	$5,$5,0xf
	sll	$5,$5,2
	subu	$3,$3,$5
	slt	$3,$3,$4
	beq	$3,$0,$L319
	li	$3,1073741824			# 0x40000000

	lui	$4,%hi(LogFlag)
	lw	$6,12($9)
	lui	$5,%hi(attack_saddr+88)
	li	$11,-1			# 0xffffffffffffffff
	lw	$3,%lo(LogFlag)($4)
	sw	$6,%lo(attack_saddr+88)($5)
	li	$6,536870912			# 0x20000000
	lw	$5,16($9)
	or	$3,$3,$6
	sw	$3,%lo(LogFlag)($4)
	lui	$3,%hi(attack_daddr+88)
	b	$L102
	sw	$5,%lo(attack_daddr+88)($3)

$L274:
	jal	dos_pkt_locate.isra.0
	lw	$4,12($9)

	beq	$2,$0,$L297
	move	$11,$2

	lw	$3,%lo(dos_item_global)($12)
	andi	$2,$3,0x22
	bne	$2,$0,$L298
	li	$2,6			# 0x6

$L48:
	andi	$2,$3,0x44
$L334:
	bne	$2,$0,$L299
	li	$2,6			# 0x6

$L162:
	move	$11,$0
$L55:
	andi	$2,$3,0x88
	bne	$2,$0,$L300
	li	$2,17			# 0x11

$L59:
	andi	$2,$3,0x110
$L329:
	bne	$2,$0,$L301
	nop

$L64:
	li	$2,3145728			# 0x300000
	and	$2,$3,$2
	bne	$2,$0,$L302
	li	$2,1			# 0x1

$L69:
	andi	$2,$3,0x200
	bne	$2,$0,$L303
	move	$2,$3

	ext	$3,$3,22,1
	b	$L45
	movz	$11,$0,$3

$L293:
	lw	$4,12($9)
	lw	$6,%lo(dos_lan_addr)($5)
	lui	$5,%hi(dos_lan_mask)
	wsbh	$3,$4
	lw	$5,%lo(dos_lan_mask)($5)
	ror	$3,$3,16
	xor	$3,$3,$6
	and	$3,$3,$5
	bne	$3,$0,$L143
	li	$3,1			# 0x1

	lbu	$5,9($9)
	beq	$5,$3,$L143
	lui	$5,%hi(LogFlag)

	lui	$6,%hi(attack_saddr+36)
	li	$11,-1			# 0xffffffffffffffff
	lw	$3,%lo(LogFlag)($5)
	sw	$4,%lo(attack_saddr+36)($6)
	lw	$4,16($9)
	ori	$3,$3,0x1000
	sw	$3,%lo(LogFlag)($5)
	lui	$3,%hi(attack_daddr+36)
	b	$L143
	sw	$4,%lo(attack_daddr+36)($3)

$L292:
	lw	$4,12($9)
	andi	$5,$4,0xf0
	beq	$5,$3,$L141
	li	$3,-1			# 0xffffffffffffffff

	beq	$4,$3,$L141
	lui	$5,%hi(dos_lan_mask)

	wsbh	$3,$4
	lw	$5,%lo(dos_lan_mask)($5)
	ror	$3,$3,16
	nor	$3,$3,$5
	bne	$3,$0,$L320
	andi	$3,$2,0x1000

$L141:
	lui	$5,%hi(LogFlag)
	lui	$6,%hi(attack_saddr+84)
	li	$11,-1			# 0xffffffffffffffff
	lw	$3,%lo(LogFlag)($5)
	sw	$4,%lo(attack_saddr+84)($6)
	li	$6,268435456			# 0x10000000
	lw	$4,16($9)
	or	$3,$3,$6
	sw	$3,%lo(LogFlag)($5)
	lui	$3,%hi(attack_daddr+84)
	b	$L140
	sw	$4,%lo(attack_daddr+84)($3)

$L281:
	lbu	$5,9($9)
	bne	$5,$4,$L113
	li	$5,2			# 0x2

	lbu	$4,13($10)
	andi	$4,$4,0x3f
	bne	$4,$5,$L113
	nop

	lhu	$4,2($9)
	lw	$6,0($9)
	lw	$5,12($10)
	wsbh	$4,$4
	andi	$6,$6,0xf
	sll	$6,$6,2
	ext	$5,$5,4,4
	andi	$4,$4,0xffff
	subu	$4,$4,$6
	sll	$5,$5,2
	bne	$4,$5,$L321
	lui	$5,%hi(LogFlag)

	lhu	$4,6($9)
	wsbh	$4,$4
	andi	$4,$4,0x2000
	beq	$4,$0,$L113
	nop

$L321:
	lw	$7,12($9)
	lui	$6,%hi(attack_saddr+52)
	li	$11,-1			# 0xffffffffffffffff
	lw	$4,%lo(LogFlag)($5)
	sw	$7,%lo(attack_saddr+52)($6)
	lw	$6,16($9)
	or	$3,$4,$3
	sw	$3,%lo(LogFlag)($5)
	lui	$3,%hi(attack_daddr+52)
	b	$L113
	sw	$6,%lo(attack_daddr+52)($3)

$L288:
	lbu	$4,9($9)
	bne	$4,$3,$L322
	ext	$3,$2,25,1

	lw	$3,0($9)
	andi	$3,$3,0xf
	sll	$3,$3,2
	addu	$3,$9,$3
	lbu	$3,0($3)
	andi	$3,$3,0xf7
	bne	$3,$0,$L129
	li	$3,224			# 0xe0

	lw	$4,12($9)
	andi	$5,$4,0xf0
	beq	$5,$3,$L131
	li	$3,-1			# 0xffffffffffffffff

	beq	$4,$3,$L131
	lui	$5,%hi(dos_lan_mask)

	wsbh	$3,$4
	lw	$5,%lo(dos_lan_mask)($5)
	ror	$3,$3,16
	nor	$3,$3,$5
	bne	$3,$0,$L322
	ext	$3,$2,25,1

$L131:
	lui	$5,%hi(LogFlag)
	lui	$6,%hi(attack_saddr+68)
	li	$11,-1			# 0xffffffffffffffff
	lw	$3,%lo(LogFlag)($5)
	sw	$4,%lo(attack_saddr+68)($6)
	li	$6,16777216			# 0x1000000
	lw	$4,16($9)
	or	$3,$3,$6
	sw	$3,%lo(LogFlag)($5)
	lui	$3,%hi(attack_daddr+68)
	b	$L129
	sw	$4,%lo(attack_daddr+68)($3)

$L287:
	lbu	$5,9($9)
	bne	$5,$4,$L127
	lui	$6,%hi(dos_lan_addr)

	lw	$5,12($9)
	lw	$7,%lo(dos_lan_addr)($6)
	lui	$6,%hi(dos_lan_mask)
	wsbh	$4,$5
	lw	$6,%lo(dos_lan_mask)($6)
	ror	$4,$4,16
	xor	$4,$4,$7
	and	$4,$4,$6
	bne	$4,$0,$L127
	lui	$6,%hi(LogFlag)

	lui	$7,%hi(attack_saddr+64)
	li	$11,-1			# 0xffffffffffffffff
	lw	$4,%lo(LogFlag)($6)
	sw	$5,%lo(attack_saddr+64)($7)
	lw	$5,16($9)
	or	$3,$4,$3
	sw	$3,%lo(LogFlag)($6)
	lui	$3,%hi(attack_daddr+64)
	b	$L127
	sw	$5,%lo(attack_daddr+64)($3)

$L286:
	lbu	$4,9($9)
	bne	$4,$3,$L323
	li	$3,524288			# 0x80000

	lw	$3,0($9)
	li	$4,8			# 0x8
	andi	$3,$3,0xf
	sll	$3,$3,2
	addu	$3,$9,$3
	lbu	$3,0($3)
	bne	$3,$4,$L323
	li	$3,524288			# 0x80000

	lui	$5,%hi(dos_lan_addr)
	lw	$4,12($9)
	lw	$6,%lo(dos_lan_addr)($5)
	lui	$5,%hi(dos_lan_mask)
	wsbh	$3,$4
	lw	$5,%lo(dos_lan_mask)($5)
	ror	$3,$3,16
	xor	$3,$3,$6
	and	$3,$3,$5
	bne	$3,$0,$L323
	li	$3,524288			# 0x80000

	lui	$5,%hi(LogFlag)
	lui	$6,%hi(attack_saddr+28)
	li	$11,-1			# 0xffffffffffffffff
	lw	$3,%lo(LogFlag)($5)
	sw	$4,%lo(attack_saddr+28)($6)
	lw	$4,16($9)
	ori	$3,$3,0x400
	sw	$3,%lo(LogFlag)($5)
	lui	$3,%hi(attack_daddr+28)
	b	$L125
	sw	$4,%lo(attack_daddr+28)($3)

$L285:
	lhu	$3,6($9)
	wsbh	$3,$3
	andi	$3,$3,0xffff
	andi	$4,$3,0x2000
	bne	$4,$0,$L123
	andi	$3,$3,0x1fff

	beq	$3,$0,$L123
	sll	$3,$3,3

	lhu	$5,2($9)
	lw	$4,0($9)
	wsbh	$5,$5
	andi	$4,$4,0xf
	andi	$5,$5,0xffff
	sll	$4,$4,2
	addu	$3,$3,$5
	subu	$3,$3,$4
	li	$4,65536			# 0x10000
	sltu	$3,$3,$4
	bne	$3,$0,$L324
	andi	$3,$2,0x400

	lui	$4,%hi(LogFlag)
	lw	$6,12($9)
	lui	$5,%hi(attack_saddr+44)
	li	$11,-1			# 0xffffffffffffffff
	lw	$3,%lo(LogFlag)($4)
	sw	$6,%lo(attack_saddr+44)($5)
	lw	$5,16($9)
	ori	$3,$3,0x4000
	sw	$3,%lo(LogFlag)($4)
	lui	$3,%hi(attack_daddr+44)
	b	$L123
	sw	$5,%lo(attack_daddr+44)($3)

$L284:
	lbu	$5,9($9)
	bne	$5,$4,$L121
	nop

	lhu	$4,6($9)
	wsbh	$4,$4
	andi	$4,$4,0x3fff
	bne	$4,$0,$L121
	nop

	lhu	$4,2($9)
	lw	$6,0($9)
	lhu	$5,4($10)
	wsbh	$4,$4
	andi	$6,$6,0xf
	sll	$6,$6,2
	andi	$4,$4,0xffff
	wsbh	$5,$5
	subu	$4,$4,$6
	andi	$5,$5,0xffff
	slt	$4,$5,$4
	beq	$4,$0,$L121
	lui	$5,%hi(LogFlag)

	lw	$7,12($9)
	lui	$6,%hi(attack_saddr+56)
	li	$11,-1			# 0xffffffffffffffff
	lw	$4,%lo(LogFlag)($5)
	sw	$7,%lo(attack_saddr+56)($6)
	lw	$6,16($9)
	or	$3,$4,$3
	sw	$3,%lo(LogFlag)($5)
	lui	$3,%hi(attack_daddr+56)
	b	$L121
	sw	$6,%lo(attack_daddr+56)($3)

$L283:
	lbu	$4,9($9)
	bne	$4,$3,$L325
	li	$3,131072			# 0x20000

	lhu	$3,2($10)
	li	$4,1792			# 0x700
	beq	$3,$4,$L120
	li	$6,-513			# 0xfffffffffffffdff

	li	$5,4352			# 0x1100
	and	$3,$3,$6
	beq	$3,$5,$L120
	nop

	lhu	$3,0($10)
	beq	$3,$4,$L120
	and	$3,$3,$6

	bne	$3,$5,$L325
	li	$3,131072			# 0x20000

$L120:
	lui	$4,%hi(LogFlag)
	lw	$6,12($9)
	lui	$5,%hi(attack_saddr+60)
	li	$11,-1			# 0xffffffffffffffff
	lw	$3,%lo(LogFlag)($4)
	sw	$6,%lo(attack_saddr+60)($5)
	li	$6,262144			# 0x40000
	lw	$5,16($9)
	or	$3,$3,$6
	sw	$3,%lo(LogFlag)($4)
	lui	$3,%hi(attack_daddr+60)
	b	$L118
	sw	$5,%lo(attack_daddr+60)($3)

$L279:
	lbu	$4,9($9)
	bne	$4,$3,$L326
	andi	$3,$2,0x8000

	lw	$3,0($9)
	andi	$3,$3,0xf
	sll	$3,$3,2
	addu	$3,$9,$3
	lbu	$3,0($3)
	bne	$3,$0,$L326
	andi	$3,$2,0x8000

	lui	$3,%hi(icmp_echo_request)
	lw	$3,%lo(icmp_echo_request)($3)
	bne	$3,$0,$L326
	andi	$3,$2,0x8000

	lui	$4,%hi(LogFlag)
	lw	$6,12($9)
	lui	$5,%hi(attack_saddr+96)
	li	$11,-1			# 0xffffffffffffffff
	lw	$3,%lo(LogFlag)($4)
	sw	$6,%lo(attack_saddr+96)($5)
	li	$6,-2147483648			# 0xffffffff80000000
	lw	$5,16($9)
	or	$3,$3,$6
	sw	$3,%lo(LogFlag)($4)
	lui	$3,%hi(attack_daddr+96)
	b	$L109
	sw	$5,%lo(attack_daddr+96)($3)

$L297:
	b	$L45
	lw	$2,%lo(dos_item_global)($12)

$L278:
	lw	$4,0($9)
	andi	$4,$4,0xf
	sll	$4,$4,2
	addu	$4,$9,$4
	lbu	$4,0($4)
	bne	$4,$5,$L107
	lui	$5,%hi(LogFlag)

	lw	$7,12($9)
	lui	$6,%hi(attack_saddr+92)
	li	$11,-1			# 0xffffffffffffffff
	lw	$4,%lo(LogFlag)($5)
	sw	$7,%lo(attack_saddr+92)($6)
	lw	$6,16($9)
	or	$3,$4,$3
	sw	$3,%lo(LogFlag)($5)
	lui	$3,%hi(attack_daddr+92)
	b	$L107
	sw	$6,%lo(attack_daddr+92)($3)

$L280:
	lbu	$3,13($10)
	addiu	$4,$4,%lo(_tcpDosScanBitmap)
	andi	$3,$3,0x3f
	addu	$3,$3,$4
	lb	$3,0($3)
	beq	$3,$0,$L313
	li	$3,65536			# 0x10000

	lui	$4,%hi(LogFlag)
	lw	$6,12($9)
	lui	$5,%hi(attack_saddr+48)
	li	$11,-1			# 0xffffffffffffffff
	lw	$3,%lo(LogFlag)($4)
	sw	$6,%lo(attack_saddr+48)($5)
	lw	$5,16($9)
	ori	$3,$3,0x8000
	sw	$3,%lo(LogFlag)($4)
	lui	$3,%hi(attack_daddr+48)
	b	$L111
	sw	$5,%lo(attack_daddr+48)($3)

$L282:
	lui	$6,%hi(attack_saddr+32)
	li	$11,-1			# 0xffffffffffffffff
	lw	$4,%lo(LogFlag)($5)
	sw	$3,%lo(attack_saddr+32)($6)
	lw	$6,16($9)
	ori	$3,$4,0x800
	sw	$3,%lo(LogFlag)($5)
	lui	$3,%hi(attack_daddr+32)
	b	$L116
	sw	$6,%lo(attack_daddr+32)($3)

$L289:
	lhu	$3,0($10)
	andi	$5,$3,0x7fff
	beq	$5,$4,$L134
	li	$4,4864			# 0x1300

	bne	$3,$4,$L315
	ext	$3,$2,26,1

$L134:
	lhu	$4,2($10)
	li	$3,34560			# 0x8700
	bne	$4,$3,$L315
	ext	$3,$2,26,1

	lui	$4,%hi(LogFlag)
	lw	$6,12($9)
	lui	$5,%hi(attack_saddr+72)
	li	$11,-1			# 0xffffffffffffffff
	lw	$3,%lo(LogFlag)($4)
	sw	$6,%lo(attack_saddr+72)($5)
	li	$6,33554432			# 0x2000000
	lw	$5,16($9)
	or	$3,$3,$6
	sw	$3,%lo(LogFlag)($4)
	lui	$3,%hi(attack_daddr+72)
	b	$L132
	sw	$5,%lo(attack_daddr+72)($3)

$L290:
	lbu	$3,13($10)
	andi	$4,$3,0x17
	beq	$4,$0,$L327
	lui	$4,%hi(LogFlag)

	li	$4,3			# 0x3
	andi	$5,$3,0x3
	beq	$5,$4,$L137
	li	$4,1			# 0x1

	andi	$3,$3,0x11
	bne	$3,$4,$L316
	li	$3,134217728			# 0x8000000

$L137:
	lui	$4,%hi(LogFlag)
$L327:
	lw	$6,12($9)
	lui	$5,%hi(attack_saddr+76)
	li	$11,-1			# 0xffffffffffffffff
	lw	$3,%lo(LogFlag)($4)
	sw	$6,%lo(attack_saddr+76)($5)
	li	$6,67108864			# 0x4000000
	lw	$5,16($9)
	or	$3,$3,$6
	sw	$3,%lo(LogFlag)($4)
	lui	$3,%hi(attack_daddr+76)
	b	$L135
	sw	$5,%lo(attack_daddr+76)($3)

$L291:
	lbu	$4,13($10)
	andi	$4,$4,0x4
	beq	$4,$0,$L138
	lui	$5,%hi(LogFlag)

	lw	$7,12($9)
	lui	$6,%hi(attack_saddr+80)
	li	$11,-1			# 0xffffffffffffffff
	lw	$4,%lo(LogFlag)($5)
	sw	$7,%lo(attack_saddr+80)($6)
	lw	$6,16($9)
	or	$3,$4,$3
	sw	$3,%lo(LogFlag)($5)
	lui	$3,%hi(attack_daddr+80)
	b	$L138
	sw	$6,%lo(attack_daddr+80)($3)

$L89:
	addiu	$5,$5,1
	addiu	$7,$7,1
$L93:
	sltu	$4,$7,$6
	bne	$4,$0,$L97
	nop

	b	$L312
	andi	$3,$3,0x2

$L105:
	lhu	$3,2($9)
	b	$L271
	li	$4,8			# 0x8

$L91:
	lbu	$4,1($5)
	bne	$4,$14,$L86
	addiu	$5,$5,3

	b	$L93
	addiu	$7,$7,3

$L90:
	lbu	$4,1($5)
	bne	$4,$13,$L86
	addiu	$5,$5,4

	b	$L93
	addiu	$7,$7,4

$L92:
	lbu	$4,1($5)
	bne	$4,$15,$L86
	addiu	$5,$5,10

	b	$L93
	addiu	$7,$7,10

$L86:
	lw	$5,12($9)
	lui	$4,%hi(attack_saddr+100)
	li	$11,-1			# 0xffffffffffffffff
	sw	$5,%lo(attack_saddr+100)($4)
	lui	$4,%hi(attack_daddr+100)
	lw	$5,16($9)
	b	$L84
	sw	$5,%lo(attack_daddr+100)($4)

$L296:
	lhu	$4,6($9)
	wsbh	$4,$4
	andi	$4,$4,0x3fff
	bne	$4,$0,$L102
	nop

	lhu	$4,2($9)
	lw	$5,0($9)
	wsbh	$4,$4
	andi	$5,$5,0xf
	sll	$5,$5,2
	andi	$4,$4,0xffff
	subu	$4,$4,$5
	slt	$4,$4,4
	beq	$4,$0,$L102
	lui	$5,%hi(LogFlag)

	lw	$7,12($9)
	lui	$6,%hi(attack_saddr+88)
	li	$11,-1			# 0xffffffffffffffff
	lw	$4,%lo(LogFlag)($5)
	sw	$7,%lo(attack_saddr+88)($6)
	lw	$6,16($9)
	or	$3,$4,$3
	sw	$3,%lo(LogFlag)($5)
	lui	$3,%hi(attack_daddr+88)
	b	$L102
	sw	$6,%lo(attack_daddr+88)($3)

$L299:
	lbu	$4,9($9)
	bne	$4,$2,$L55
	move	$11,$0

	lui	$4,%hi(cur_p_pkt)
	lbu	$2,13($10)
	lw	$5,%lo(cur_p_pkt)($4)
	andi	$2,$2,0x3f
	li	$4,1			# 0x1
$L328:
	bne	$2,$4,$L55
	move	$11,$0

	lui	$4,%hi(dosFilter_block)
	lw	$6,%lo(dosFilter_block)($4)
	beq	$6,$2,$L304
	lui	$6,%hi(attack_saddr+8)

	lui	$6,%hi(per_fin_threshold)
$L338:
	lw	$2,8($5)
	lw	$6,%lo(per_fin_threshold)($6)
	addiu	$2,$2,1
	sw	$2,8($5)
	slt	$2,$6,$2
	beq	$2,$0,$L162
	andi	$2,$3,0x40

	beq	$2,$0,$L55
	move	$11,$0

	blez	$6,$L55
	lui	$5,%hi(LogFlag)

	lw	$7,12($9)
	lui	$6,%hi(attack_saddr+8)
	move	$11,$0
	lw	$2,%lo(LogFlag)($5)
	sw	$7,%lo(attack_saddr+8)($6)
	li	$6,1			# 0x1
	ori	$2,$2,0x40
	sw	$6,%lo(dosFilter_block)($4)
	sw	$2,%lo(LogFlag)($5)
	lui	$2,%hi(attack_daddr+8)
	lw	$4,16($9)
	b	$L55
	sw	$4,%lo(attack_daddr+8)($2)

$L298:
	lbu	$4,9($9)
	bne	$4,$2,$L55
	move	$11,$0

	lbu	$2,13($10)
	lui	$5,%hi(cur_p_pkt)
	li	$4,2			# 0x2
	andi	$2,$2,0x3f
	beq	$2,$4,$L305
	lw	$5,%lo(cur_p_pkt)($5)

	andi	$4,$3,0x44
	bne	$4,$0,$L328
	li	$4,1			# 0x1

$L153:
	andi	$2,$3,0x88
	beq	$2,$0,$L329
	andi	$2,$3,0x110

$L63:
	move	$2,$0
$L333:
	b	$L59
	or	$11,$11,$2

$L303:
	lui	$4,%hi(cur_p_pkt)
	lbu	$2,9($9)
	li	$5,6			# 0x6
	beq	$2,$5,$L306
	lw	$4,%lo(cur_p_pkt)($4)

$L75:
	li	$5,17			# 0x11
	beq	$2,$5,$L80
	nop

$L270:
	lw	$2,24($4)
$L78:
	ext	$4,$3,23,1
	beq	$4,$0,$L82
	nop

	slt	$2,$2,11
	bne	$2,$0,$L151
	lui	$5,%hi(LogFlag)

$L330:
	lw	$7,12($9)
	lui	$6,%hi(attack_saddr+24)
	lw	$2,%lo(dos_item_global)($12)
	ext	$3,$3,22,1
	lw	$4,%lo(LogFlag)($5)
	sw	$7,%lo(attack_saddr+24)($6)
	lw	$6,16($9)
	ori	$4,$4,0x200
	movz	$11,$0,$3
	sw	$4,%lo(LogFlag)($5)
	lui	$4,%hi(attack_daddr+24)
	b	$L45
	sw	$6,%lo(attack_daddr+24)($4)

$L82:
	slt	$2,$2,201
	beq	$2,$0,$L330
	lui	$5,%hi(LogFlag)

$L151:
	ext	$3,$3,22,1
	lw	$2,%lo(dos_item_global)($12)
	b	$L45
	movz	$11,$0,$3

$L80:
	lhu	$5,36($4)
	bne	$5,$0,$L307
	nop

	lhu	$5,2($10)
	lw	$2,24($4)
	b	$L78
	sh	$5,36($4)

$L302:
	lbu	$5,9($9)
	beq	$5,$2,$L70
	nop

$L72:
	move	$2,$0
$L331:
	b	$L69
	or	$11,$11,$2

$L70:
	lw	$2,0($9)
	andi	$2,$2,0xf
	sll	$2,$2,2
	addu	$2,$9,$2
	lbu	$2,0($2)
	andi	$2,$2,0xf7
	bne	$2,$0,$L331
	move	$2,$0

	lui	$4,%hi(dosFilter_block)
	lw	$2,%lo(dosFilter_block)($4)
	beq	$2,$5,$L308
	lui	$5,%hi(attack_saddr+20)

	lui	$2,%hi(cur_p_pkt)
$L337:
	lw	$5,%lo(cur_p_pkt)($2)
	lui	$2,%hi(per_ping_threshold)
	lw	$6,%lo(per_ping_threshold)($2)
	lw	$2,20($5)
	addiu	$2,$2,1
	slt	$7,$6,$2
	beq	$7,$0,$L72
	sw	$2,20($5)

	li	$5,2097152			# 0x200000
	and	$2,$3,$5
	beq	$2,$0,$L331
	move	$2,$0

	blez	$6,$L72
	lui	$2,%hi(attack_saddr+20)

	lw	$8,12($9)
	lui	$7,%hi(LogFlag)
	sw	$8,%lo(attack_saddr+20)($2)
	li	$8,1			# 0x1
	lw	$6,%lo(LogFlag)($7)
	move	$2,$0
	sw	$8,%lo(dosFilter_block)($4)
	lui	$4,%hi(attack_daddr+20)
	lw	$8,16($9)
	or	$11,$11,$2
	or	$5,$6,$5
	sw	$5,%lo(LogFlag)($7)
	b	$L69
	sw	$8,%lo(attack_daddr+20)($4)

$L301:
	lbu	$2,9($9)
	li	$4,1			# 0x1
	beq	$2,$4,$L65
	nop

$L68:
	move	$2,$0
$L332:
	b	$L64
	or	$11,$11,$2

$L65:
	lui	$4,%hi(dosFilter_block)
	lw	$5,%lo(dosFilter_block)($4)
	beq	$5,$2,$L309
	lui	$5,%hi(attack_saddr+16)

	lui	$2,%hi(cur_p_pkt)
$L335:
	lw	$5,%lo(cur_p_pkt)($2)
	lui	$2,%hi(per_icmp_threshold)
	lw	$6,%lo(per_icmp_threshold)($2)
	lw	$2,16($5)
	addiu	$2,$2,1
	slt	$7,$6,$2
	beq	$7,$0,$L68
	sw	$2,16($5)

	andi	$2,$3,0x100
	beq	$2,$0,$L332
	move	$2,$0

	blez	$6,$L332
	lui	$6,%hi(LogFlag)

	lw	$7,12($9)
	lui	$2,%hi(attack_saddr+16)
	lw	$5,%lo(LogFlag)($6)
	sw	$7,%lo(attack_saddr+16)($2)
	li	$7,1			# 0x1
	move	$2,$0
	sw	$7,%lo(dosFilter_block)($4)
	ori	$4,$5,0x100
	sw	$4,%lo(LogFlag)($6)
	lui	$4,%hi(attack_daddr+16)
	lw	$7,16($9)
	or	$11,$11,$2
	b	$L64
	sw	$7,%lo(attack_daddr+16)($4)

$L300:
	lbu	$4,9($9)
	bne	$4,$2,$L333
	move	$2,$0

	lui	$4,%hi(dosFilter_block)
	li	$2,1			# 0x1
	lw	$5,%lo(dosFilter_block)($4)
	beq	$5,$2,$L310
	lui	$5,%hi(attack_saddr+12)

	lui	$2,%hi(cur_p_pkt)
$L336:
	lw	$5,%lo(cur_p_pkt)($2)
	lui	$2,%hi(per_udp_threshold)
	lw	$6,%lo(per_udp_threshold)($2)
	lw	$2,12($5)
	addiu	$2,$2,1
	slt	$7,$6,$2
	beq	$7,$0,$L63
	sw	$2,12($5)

	andi	$2,$3,0x80
	beq	$2,$0,$L333
	move	$2,$0

	blez	$6,$L333
	lui	$6,%hi(LogFlag)

	lw	$7,12($9)
	lui	$2,%hi(attack_saddr+12)
	lw	$5,%lo(LogFlag)($6)
	sw	$7,%lo(attack_saddr+12)($2)
	li	$7,1			# 0x1
	move	$2,$0
	sw	$7,%lo(dosFilter_block)($4)
	ori	$4,$5,0x80
	sw	$4,%lo(LogFlag)($6)
	lui	$4,%hi(attack_daddr+12)
	lw	$7,16($9)
	or	$11,$11,$2
	b	$L59
	sw	$7,%lo(attack_daddr+12)($4)

$L295:
	sh	$0,32($4)
	b	$L44
	sh	$0,34($4)

$L306:
	lhu	$5,36($4)
	beq	$5,$0,$L76
	lhu	$2,2($10)

	beq	$2,$5,$L270
	li	$6,17			# 0x11

	lw	$2,24($4)
	addiu	$2,$2,1
	sw	$2,24($4)
	lbu	$7,9($9)
	bne	$7,$6,$L78
	nop

$L79:
	lhu	$6,2($10)
	beq	$6,$5,$L78
	nop

	addiu	$2,$2,1
	b	$L78
	sw	$2,24($4)

$L305:
	lui	$4,%hi(dosFilter_block)
	li	$2,1			# 0x1
	lw	$6,%lo(dosFilter_block)($4)
	beq	$6,$2,$L311
	lui	$6,%hi(attack_saddr+4)

$L51:
	lui	$6,%hi(per_syn_threshold)
	lw	$2,4($5)
	lw	$6,%lo(per_syn_threshold)($6)
	addiu	$2,$2,1
	sw	$2,4($5)
	slt	$2,$6,$2
	beq	$2,$0,$L48
	andi	$2,$3,0x20

	beq	$2,$0,$L334
	andi	$2,$3,0x44

	blez	$6,$L334
	lui	$5,%hi(LogFlag)

	lw	$7,12($9)
	lui	$6,%hi(attack_saddr+4)
	lw	$2,%lo(LogFlag)($5)
	sw	$7,%lo(attack_saddr+4)($6)
	li	$6,1			# 0x1
	ori	$2,$2,0x20
	sw	$6,%lo(dosFilter_block)($4)
	sw	$2,%lo(LogFlag)($5)
	lui	$2,%hi(attack_daddr+4)
	lw	$4,16($9)
	b	$L48
	sw	$4,%lo(attack_daddr+4)($2)

$L76:
	sh	$2,36($4)
	b	$L75
	lbu	$2,9($9)

$L307:
	b	$L79
	lw	$2,24($4)

$L309:
	lw	$2,12($9)
	lw	$5,%lo(attack_saddr+16)($5)
	bne	$5,$2,$L335
	lui	$2,%hi(cur_p_pkt)

	li	$2,-1			# 0xffffffffffffffff
	b	$L64
	or	$11,$11,$2

$L310:
	lw	$2,12($9)
	lw	$5,%lo(attack_saddr+12)($5)
	bne	$5,$2,$L336
	lui	$2,%hi(cur_p_pkt)

	li	$2,-1			# 0xffffffffffffffff
	b	$L59
	or	$11,$11,$2

$L148:
	lui	$3,%hi(LogFlag)
	lw	$5,12($9)
	lui	$4,%hi(attack_saddr+40)
	li	$11,-1			# 0xffffffffffffffff
	lw	$2,%lo(LogFlag)($3)
	sw	$5,%lo(attack_saddr+40)($4)
	lw	$4,16($9)
	ori	$2,$2,0x2000
	sw	$2,%lo(LogFlag)($3)
	lui	$2,%hi(attack_daddr+40)
	b	$L44
	sw	$4,%lo(attack_daddr+40)($2)

$L308:
	lw	$2,12($9)
	lw	$5,%lo(attack_saddr+20)($5)
	bne	$5,$2,$L337
	lui	$2,%hi(cur_p_pkt)

	li	$2,-1			# 0xffffffffffffffff
	b	$L69
	or	$11,$11,$2

$L304:
	lw	$2,12($9)
	lw	$6,%lo(attack_saddr+8)($6)
	bne	$6,$2,$L338
	lui	$6,%hi(per_fin_threshold)

$L168:
	b	$L55
	li	$11,-1			# 0xffffffffffffffff

$L311:
	lw	$2,12($9)
	lw	$6,%lo(attack_saddr+4)($6)
	bne	$6,$2,$L51
	andi	$2,$3,0x44

	bne	$2,$0,$L168
	nop

	b	$L153
	li	$11,-1			# 0xffffffffffffffff

	.set	macro
	.set	reorder
	.end	dos_filter_ipv4__action
	.size	dos_filter_ipv4__action, .-dos_filter_ipv4__action
	.section	.rodata.str1.4
	.align	2
$LC6:
	.ascii	"DoS: Whole System SYN Flood Attack\012\000"
	.align	2
$LC7:
	.ascii	"DoS: Whole System FIN Flood Attack\012\000"
	.align	2
$LC8:
	.ascii	"DoS: Whole System UDP Flood Attack\012\000"
	.align	2
$LC9:
	.ascii	"DoS: Whole System ICMP Flood Attack\012\000"
	.align	2
$LC10:
	.ascii	"DoS: Per-source SYN Flood Attack source=%u.%u.%u.%u dest"
	.ascii	"ination=%u.%u.%u.%u\012\000"
	.align	2
$LC11:
	.ascii	"DoS: Per-source FIN Flood Attack source=%u.%u.%u.%u dest"
	.ascii	"ination=%u.%u.%u.%u\012\000"
	.align	2
$LC12:
	.ascii	"DoS: Per-source UDP Flood Attack source=%u.%u.%u.%u dest"
	.ascii	"ination=%u.%u.%u.%u\012\000"
	.align	2
$LC13:
	.ascii	"DoS: Per-source ICMP Flood Attack source=%u.%u.%u.%u des"
	.ascii	"tination=%u.%u.%u.%u\012\000"
	.align	2
$LC14:
	.ascii	"DoS: Port Scan Attack source=%u.%u.%u.%u destination=%u."
	.ascii	"%u.%u.%u\012\000"
	.align	2
$LC15:
	.ascii	"DoS: Tcp Scan Attack source=%u.%u.%u.%u destination=%u.%"
	.ascii	"u.%u.%u\012\000"
	.align	2
$LC16:
	.ascii	"DoS: Tcp SYN With Data Attack source=%u.%u.%u.%u destina"
	.ascii	"tion=%u.%u.%u.%u\012\000"
	.align	2
$LC17:
	.ascii	"DoS: IP Land Attack source=%u.%u.%u.%u destination=%u.%u"
	.ascii	".%u.%u\012\000"
	.align	2
$LC18:
	.ascii	"DoS: UdpEchoChargen Attack source=%u.%u.%u.%u destinatio"
	.ascii	"n=%u.%u.%u.%u\012\000"
	.align	2
$LC19:
	.ascii	"DoS: UdpBomb Attack source=%u.%u.%u.%u destination=%u.%u"
	.ascii	".%u.%u\012\000"
	.align	2
$LC20:
	.ascii	"DoS: PingOfDeath Attack source=%u.%u.%u.%u destination=%"
	.ascii	"u.%u.%u.%u\012\000"
	.align	2
$LC21:
	.ascii	"DoS: IcmpSmurf Attack source=%u.%u.%u.%u destination=%u."
	.ascii	"%u.%u.%u\012\000"
	.align	2
$LC22:
	.ascii	"DoS: IpSpoof Attack source=%u.%u.%u.%u destination=%u.%u"
	.ascii	".%u.%u\012\000"
	.align	2
$LC23:
	.ascii	"DoS: TearDrop Attack source=%u.%u.%u.%u destination=%u.%"
	.ascii	"u.%u.%u\012\000"
	.text
	.align	2
	.globl	dos_timer_ipv4_handler
	.set	nomips16
	.set	nomicromips
	.ent	dos_timer_ipv4_handler
	.type	dos_timer_ipv4_handler, @function
dos_timer_ipv4_handler:
	.frame	$sp,56,$31		# vars= 0, regs= 4/0, args= 40, gp= 0
	.mask	0x80070000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-56
	sw	$31,52($sp)
	sw	$18,48($sp)
	sw	$17,44($sp)
	sw	$16,40($sp)
	lui	$2,%hi(dos_item_global)
	lw	$11,%lo(dos_item_global)($2)
	andi	$2,$11,0x1e
	beq	$2,$0,$L445
	lui	$4,%hi(dos_pkt+40960)

	lui	$16,%hi(dos_pkt)
	move	$6,$0
	move	$7,$0
	move	$8,$0
	move	$9,$0
	addiu	$2,$16,%lo(dos_pkt)
	addiu	$4,$4,%lo(dos_pkt+40960)
	li	$5,1			# 0x1
	andi	$14,$11,0x2
	andi	$13,$11,0x4
	andi	$12,$11,0x8
	b	$L340
	andi	$10,$11,0x10

$L342:
	addiu	$2,$2,40
	beq	$2,$4,$L446
	nop

$L340:
	lb	$3,0($2)
	bne	$3,$5,$L342
	nop

	beq	$14,$0,$L343
	nop

	lw	$3,4($2)
	addu	$15,$9,$3
	slt	$3,$0,$3
	movn	$9,$15,$3
$L343:
	beq	$13,$0,$L344
	nop

	lw	$3,8($2)
	addu	$15,$8,$3
	slt	$3,$0,$3
	movn	$8,$15,$3
$L344:
	beq	$12,$0,$L345
	nop

	lw	$3,12($2)
	addu	$15,$7,$3
	slt	$3,$0,$3
	movn	$7,$15,$3
$L345:
	beq	$10,$0,$L342
	nop

	lw	$3,16($2)
	addiu	$2,$2,40
	addu	$15,$6,$3
	slt	$3,$0,$3
	bne	$2,$4,$L340
	movn	$6,$15,$3

$L446:
	lui	$2,%hi(whole_syn_threshold)
	lw	$2,%lo(whole_syn_threshold)($2)
	slt	$9,$2,$9
	bne	$9,$0,$L346
	lui	$18,%hi(LogFlag)

$L483:
	lw	$17,%lo(LogFlag)($18)
$L347:
	lui	$2,%hi(whole_fin_threshold)
	lw	$2,%lo(whole_fin_threshold)($2)
	slt	$8,$2,$8
	beq	$8,$0,$L349
	andi	$3,$11,0x4

	beq	$3,$0,$L349
	nop

	blez	$2,$L465
	lui	$2,%hi(whole_udp_threshold)

	ori	$17,$17,0x4
	sw	$17,%lo(LogFlag)($18)
$L349:
	lui	$2,%hi(whole_udp_threshold)
$L465:
	lw	$2,%lo(whole_udp_threshold)($2)
	slt	$7,$2,$7
	beq	$7,$0,$L350
	andi	$3,$11,0x8

	beq	$3,$0,$L350
	nop

	blez	$2,$L466
	lui	$2,%hi(whole_icmp_threshold)

	ori	$17,$17,0x8
	sw	$17,%lo(LogFlag)($18)
$L350:
	lui	$2,%hi(whole_icmp_threshold)
$L466:
	lw	$2,%lo(whole_icmp_threshold)($2)
	slt	$6,$2,$6
	beq	$6,$0,$L341
	andi	$11,$11,0x10

	beq	$11,$0,$L341
	nop

	blez	$2,$L467
	andi	$2,$17,0x2

	ori	$17,$17,0x10
	sw	$17,%lo(LogFlag)($18)
$L341:
	andi	$2,$17,0x2
$L467:
	bne	$2,$0,$L447
	nop

$L351:
	andi	$2,$17,0x4
	bne	$2,$0,$L448
	andi	$2,$17,0x8

$L484:
	bne	$2,$0,$L449
	andi	$2,$17,0x10

$L485:
	bne	$2,$0,$L450
	andi	$2,$17,0x20

$L486:
	bne	$2,$0,$L468
	lui	$2,%hi(attack_saddr)

	andi	$2,$17,0x40
$L487:
	bne	$2,$0,$L469
	lui	$2,%hi(attack_saddr)

	andi	$2,$17,0x80
$L488:
	bne	$2,$0,$L470
	lui	$2,%hi(attack_saddr)

	andi	$2,$17,0x100
$L489:
	bne	$2,$0,$L471
	lui	$2,%hi(attack_saddr)

	andi	$2,$17,0x200
$L490:
	bne	$2,$0,$L472
	lui	$2,%hi(attack_saddr)

	andi	$2,$17,0x8000
$L491:
	bne	$2,$0,$L473
	lui	$2,%hi(attack_saddr)

	ext	$2,$17,16,1
$L492:
	bne	$2,$0,$L474
	lui	$2,%hi(attack_saddr)

	andi	$2,$17,0x800
$L493:
	bne	$2,$0,$L475
	lui	$2,%hi(attack_saddr)

	ext	$2,$17,18,1
$L494:
	bne	$2,$0,$L476
	lui	$2,%hi(attack_saddr)

	ext	$2,$17,17,1
$L495:
	bne	$2,$0,$L477
	lui	$2,%hi(attack_saddr)

	andi	$2,$17,0x4000
$L496:
	bne	$2,$0,$L478
	lui	$2,%hi(attack_saddr)

	andi	$2,$17,0x400
$L497:
	bne	$2,$0,$L479
	lui	$2,%hi(attack_saddr)

	andi	$2,$17,0x1000
$L498:
	bne	$2,$0,$L480
	lui	$2,%hi(attack_saddr)

$L367:
	andi	$17,$17,0x2000
$L464:
	beq	$17,$0,$L481
	li	$6,40960			# 0xa000

	lui	$2,%hi(attack_saddr)
	lui	$3,%hi(attack_daddr)
	addiu	$2,$2,%lo(attack_saddr)
	addiu	$3,$3,%lo(attack_daddr)
	lui	$4,%hi($LC23)
	lbu	$7,42($2)
	lbu	$6,41($2)
	addiu	$4,$4,%lo($LC23)
	lbu	$5,40($2)
	lbu	$10,43($3)
	lbu	$9,42($3)
	lbu	$8,41($3)
	lbu	$2,43($2)
	lbu	$3,40($3)
	sw	$10,32($sp)
	sw	$9,28($sp)
	sw	$8,24($sp)
	sw	$3,20($sp)
	jal	scrlog_printk
	sw	$2,16($sp)

	li	$6,40960			# 0xa000
$L481:
	sw	$0,%lo(LogFlag)($18)
	move	$5,$0
	jal	memset
	addiu	$4,$16,%lo(dos_pkt)

	lui	$2,%hi(icmp_echo_request_time)
	lui	$3,%hi(jiffies)
	lw	$2,%lo(icmp_echo_request_time)($2)
	lw	$3,%lo(jiffies)($3)
	addiu	$2,$2,100
	subu	$2,$2,$3
	bgez	$2,$L482
	lw	$31,52($sp)

	lui	$2,%hi(icmp_echo_request)
	sw	$0,%lo(icmp_echo_request)($2)
	lw	$31,52($sp)
$L482:
	lw	$18,48($sp)
	lw	$17,44($sp)
	lw	$16,40($sp)
	jr	$31
	addiu	$sp,$sp,56

$L346:
	andi	$3,$11,0x2
	beq	$3,$0,$L483
	nop

	blez	$2,$L483
	lw	$17,%lo(LogFlag)($18)

	ori	$17,$17,0x2
	b	$L347
	sw	$17,%lo(LogFlag)($18)

$L445:
	lui	$18,%hi(LogFlag)
	lw	$17,%lo(LogFlag)($18)
	andi	$2,$17,0x2
	beq	$2,$0,$L351
	lui	$16,%hi(dos_pkt)

$L447:
	lui	$4,%hi($LC6)
	jal	scrlog_printk
	addiu	$4,$4,%lo($LC6)

	andi	$2,$17,0x4
	beq	$2,$0,$L484
	andi	$2,$17,0x8

$L448:
	lui	$4,%hi($LC7)
	jal	scrlog_printk
	addiu	$4,$4,%lo($LC7)

	andi	$2,$17,0x8
	beq	$2,$0,$L485
	andi	$2,$17,0x10

$L449:
	lui	$4,%hi($LC8)
	jal	scrlog_printk
	addiu	$4,$4,%lo($LC8)

	andi	$2,$17,0x10
	beq	$2,$0,$L486
	andi	$2,$17,0x20

$L450:
	lui	$4,%hi($LC9)
	jal	scrlog_printk
	addiu	$4,$4,%lo($LC9)

	andi	$2,$17,0x20
	beq	$2,$0,$L487
	andi	$2,$17,0x40

	lui	$2,%hi(attack_saddr)
$L468:
	lui	$3,%hi(attack_daddr)
	addiu	$2,$2,%lo(attack_saddr)
	addiu	$3,$3,%lo(attack_daddr)
	lui	$4,%hi($LC10)
	lbu	$7,6($2)
	lbu	$6,5($2)
	addiu	$4,$4,%lo($LC10)
	lbu	$5,4($2)
	lbu	$10,7($3)
	lbu	$9,6($3)
	lbu	$8,5($3)
	lbu	$2,7($2)
	lbu	$3,4($3)
	sw	$10,32($sp)
	sw	$9,28($sp)
	sw	$8,24($sp)
	sw	$3,20($sp)
	jal	scrlog_printk
	sw	$2,16($sp)

	andi	$2,$17,0x40
	beq	$2,$0,$L488
	andi	$2,$17,0x80

	lui	$2,%hi(attack_saddr)
$L469:
	lui	$3,%hi(attack_daddr)
	addiu	$2,$2,%lo(attack_saddr)
	addiu	$3,$3,%lo(attack_daddr)
	lui	$4,%hi($LC11)
	lbu	$7,10($2)
	lbu	$6,9($2)
	addiu	$4,$4,%lo($LC11)
	lbu	$5,8($2)
	lbu	$10,11($3)
	lbu	$9,10($3)
	lbu	$8,9($3)
	lbu	$2,11($2)
	lbu	$3,8($3)
	sw	$10,32($sp)
	sw	$9,28($sp)
	sw	$8,24($sp)
	sw	$3,20($sp)
	jal	scrlog_printk
	sw	$2,16($sp)

	andi	$2,$17,0x80
	beq	$2,$0,$L489
	andi	$2,$17,0x100

	lui	$2,%hi(attack_saddr)
$L470:
	lui	$3,%hi(attack_daddr)
	addiu	$2,$2,%lo(attack_saddr)
	addiu	$3,$3,%lo(attack_daddr)
	lui	$4,%hi($LC12)
	lbu	$7,14($2)
	lbu	$6,13($2)
	addiu	$4,$4,%lo($LC12)
	lbu	$5,12($2)
	lbu	$10,15($3)
	lbu	$9,14($3)
	lbu	$8,13($3)
	lbu	$2,15($2)
	lbu	$3,12($3)
	sw	$10,32($sp)
	sw	$9,28($sp)
	sw	$8,24($sp)
	sw	$3,20($sp)
	jal	scrlog_printk
	sw	$2,16($sp)

	andi	$2,$17,0x100
	beq	$2,$0,$L490
	andi	$2,$17,0x200

	lui	$2,%hi(attack_saddr)
$L471:
	lui	$3,%hi(attack_daddr)
	addiu	$2,$2,%lo(attack_saddr)
	addiu	$3,$3,%lo(attack_daddr)
	lui	$4,%hi($LC13)
	lbu	$7,18($2)
	lbu	$6,17($2)
	addiu	$4,$4,%lo($LC13)
	lbu	$5,16($2)
	lbu	$10,19($3)
	lbu	$9,18($3)
	lbu	$8,17($3)
	lbu	$2,19($2)
	lbu	$3,16($3)
	sw	$10,32($sp)
	sw	$9,28($sp)
	sw	$8,24($sp)
	sw	$3,20($sp)
	jal	scrlog_printk
	sw	$2,16($sp)

	andi	$2,$17,0x200
	beq	$2,$0,$L491
	andi	$2,$17,0x8000

	lui	$2,%hi(attack_saddr)
$L472:
	lui	$3,%hi(attack_daddr)
	addiu	$2,$2,%lo(attack_saddr)
	addiu	$3,$3,%lo(attack_daddr)
	lui	$4,%hi($LC14)
	lbu	$7,26($2)
	lbu	$6,25($2)
	addiu	$4,$4,%lo($LC14)
	lbu	$5,24($2)
	lbu	$10,27($3)
	lbu	$9,26($3)
	lbu	$8,25($3)
	lbu	$2,27($2)
	lbu	$3,24($3)
	sw	$10,32($sp)
	sw	$9,28($sp)
	sw	$8,24($sp)
	sw	$3,20($sp)
	jal	scrlog_printk
	sw	$2,16($sp)

	andi	$2,$17,0x8000
	beq	$2,$0,$L492
	ext	$2,$17,16,1

	lui	$2,%hi(attack_saddr)
$L473:
	lui	$3,%hi(attack_daddr)
	addiu	$2,$2,%lo(attack_saddr)
	addiu	$3,$3,%lo(attack_daddr)
	lui	$4,%hi($LC15)
	lbu	$7,50($2)
	lbu	$6,49($2)
	addiu	$4,$4,%lo($LC15)
	lbu	$5,48($2)
	lbu	$10,51($3)
	lbu	$9,50($3)
	lbu	$8,49($3)
	lbu	$2,51($2)
	lbu	$3,48($3)
	sw	$10,32($sp)
	sw	$9,28($sp)
	sw	$8,24($sp)
	sw	$3,20($sp)
	jal	scrlog_printk
	sw	$2,16($sp)

	ext	$2,$17,16,1
	beq	$2,$0,$L493
	andi	$2,$17,0x800

	lui	$2,%hi(attack_saddr)
$L474:
	lui	$3,%hi(attack_daddr)
	addiu	$2,$2,%lo(attack_saddr)
	addiu	$3,$3,%lo(attack_daddr)
	lui	$4,%hi($LC16)
	lbu	$7,54($2)
	lbu	$6,53($2)
	addiu	$4,$4,%lo($LC16)
	lbu	$5,52($2)
	lbu	$10,55($3)
	lbu	$9,54($3)
	lbu	$8,53($3)
	lbu	$2,55($2)
	lbu	$3,52($3)
	sw	$10,32($sp)
	sw	$9,28($sp)
	sw	$8,24($sp)
	sw	$3,20($sp)
	jal	scrlog_printk
	sw	$2,16($sp)

	andi	$2,$17,0x800
	beq	$2,$0,$L494
	ext	$2,$17,18,1

	lui	$2,%hi(attack_saddr)
$L475:
	lui	$3,%hi(attack_daddr)
	addiu	$2,$2,%lo(attack_saddr)
	addiu	$3,$3,%lo(attack_daddr)
	lui	$4,%hi($LC17)
	lbu	$7,34($2)
	lbu	$6,33($2)
	addiu	$4,$4,%lo($LC17)
	lbu	$5,32($2)
	lbu	$10,35($3)
	lbu	$9,34($3)
	lbu	$8,33($3)
	lbu	$2,35($2)
	lbu	$3,32($3)
	sw	$10,32($sp)
	sw	$9,28($sp)
	sw	$8,24($sp)
	sw	$3,20($sp)
	jal	scrlog_printk
	sw	$2,16($sp)

	ext	$2,$17,18,1
	beq	$2,$0,$L495
	ext	$2,$17,17,1

	lui	$2,%hi(attack_saddr)
$L476:
	lui	$3,%hi(attack_daddr)
	addiu	$2,$2,%lo(attack_saddr)
	addiu	$3,$3,%lo(attack_daddr)
	lui	$4,%hi($LC18)
	lbu	$7,62($2)
	lbu	$6,61($2)
	addiu	$4,$4,%lo($LC18)
	lbu	$5,60($2)
	lbu	$10,63($3)
	lbu	$9,62($3)
	lbu	$8,61($3)
	lbu	$2,63($2)
	lbu	$3,60($3)
	sw	$10,32($sp)
	sw	$9,28($sp)
	sw	$8,24($sp)
	sw	$3,20($sp)
	jal	scrlog_printk
	sw	$2,16($sp)

	ext	$2,$17,17,1
	beq	$2,$0,$L496
	andi	$2,$17,0x4000

	lui	$2,%hi(attack_saddr)
$L477:
	lui	$3,%hi(attack_daddr)
	addiu	$2,$2,%lo(attack_saddr)
	addiu	$3,$3,%lo(attack_daddr)
	lui	$4,%hi($LC19)
	lbu	$7,58($2)
	lbu	$6,57($2)
	addiu	$4,$4,%lo($LC19)
	lbu	$5,56($2)
	lbu	$10,59($3)
	lbu	$9,58($3)
	lbu	$8,57($3)
	lbu	$2,59($2)
	lbu	$3,56($3)
	sw	$10,32($sp)
	sw	$9,28($sp)
	sw	$8,24($sp)
	sw	$3,20($sp)
	jal	scrlog_printk
	sw	$2,16($sp)

	andi	$2,$17,0x4000
	beq	$2,$0,$L497
	andi	$2,$17,0x400

	lui	$2,%hi(attack_saddr)
$L478:
	lui	$3,%hi(attack_daddr)
	addiu	$2,$2,%lo(attack_saddr)
	addiu	$3,$3,%lo(attack_daddr)
	lui	$4,%hi($LC20)
	lbu	$7,46($2)
	lbu	$6,45($2)
	addiu	$4,$4,%lo($LC20)
	lbu	$5,44($2)
	lbu	$10,47($3)
	lbu	$9,46($3)
	lbu	$8,45($3)
	lbu	$2,47($2)
	lbu	$3,44($3)
	sw	$10,32($sp)
	sw	$9,28($sp)
	sw	$8,24($sp)
	sw	$3,20($sp)
	jal	scrlog_printk
	sw	$2,16($sp)

	andi	$2,$17,0x400
	beq	$2,$0,$L498
	andi	$2,$17,0x1000

	lui	$2,%hi(attack_saddr)
$L479:
	lui	$3,%hi(attack_daddr)
	addiu	$2,$2,%lo(attack_saddr)
	addiu	$3,$3,%lo(attack_daddr)
	lui	$4,%hi($LC21)
	lbu	$7,30($2)
	lbu	$6,29($2)
	addiu	$4,$4,%lo($LC21)
	lbu	$5,28($2)
	lbu	$10,31($3)
	lbu	$9,30($3)
	lbu	$8,29($3)
	lbu	$2,31($2)
	lbu	$3,28($3)
	sw	$10,32($sp)
	sw	$9,28($sp)
	sw	$8,24($sp)
	sw	$3,20($sp)
	jal	scrlog_printk
	sw	$2,16($sp)

	andi	$2,$17,0x1000
	beq	$2,$0,$L367
	lui	$2,%hi(attack_saddr)

$L480:
	lui	$3,%hi(attack_daddr)
	addiu	$2,$2,%lo(attack_saddr)
	addiu	$3,$3,%lo(attack_daddr)
	lui	$4,%hi($LC22)
	lbu	$7,38($2)
	lbu	$6,37($2)
	addiu	$4,$4,%lo($LC22)
	lbu	$5,36($2)
	lbu	$10,39($3)
	lbu	$9,38($3)
	lbu	$8,37($3)
	lbu	$2,39($2)
	lbu	$3,36($3)
	sw	$10,32($sp)
	sw	$9,28($sp)
	sw	$8,24($sp)
	sw	$3,20($sp)
	jal	scrlog_printk
	sw	$2,16($sp)

	b	$L464
	andi	$17,$17,0x2000

	.set	macro
	.set	reorder
	.end	dos_timer_ipv4_handler
	.size	dos_timer_ipv4_handler, .-dos_timer_ipv4_handler
	.align	2
	.globl	dos_ipv4_init
	.set	nomips16
	.set	nomicromips
	.ent	dos_ipv4_init
	.type	dos_ipv4_init, @function
dos_ipv4_init:
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$16,16($sp)
	lui	$16,%hi(_tcpDosScanBitmap)
	li	$6,64			# 0x40
	addiu	$4,$16,%lo(_tcpDosScanBitmap)
	jal	memset
	move	$5,$0

	li	$2,1			# 0x1
	addiu	$3,$16,%lo(_tcpDosScanBitmap)
	lw	$31,20($sp)
	sb	$2,%lo(_tcpDosScanBitmap)($16)
	lw	$16,16($sp)
	sb	$2,63($3)
	sb	$2,58($3)
	sb	$2,41($3)
	sb	$2,40($3)
	sb	$2,33($3)
	sb	$2,32($3)
	sb	$2,9($3)
	sb	$2,8($3)
	sb	$2,3($3)
	jr	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	dos_ipv4_init
	.size	dos_ipv4_init, .-dos_ipv4_init
	.local	icmp_echo_request_time
	.comm	icmp_echo_request_time,4,4
	.local	icmp_echo_request
	.comm	icmp_echo_request,4,4
	.local	ip_name1
	.comm	ip_name1,16,4
	.local	ip_name0
	.comm	ip_name0,16,4
	.local	attack_daddr
	.comm	attack_daddr,108,4
	.local	attack_saddr
	.comm	attack_saddr,108,4
	.local	cur_p_pkt
	.comm	cur_p_pkt,4,4
	.local	dos_pkt
	.comm	dos_pkt,40960,4
	.local	LogFlag
	.comm	LogFlag,4,4
	.local	_tcpDosScanBitmap
	.comm	_tcpDosScanBitmap,64,4
	.section	.rodata.str1.4
	.align	2
$LC24:
	.ascii	"NULL\000"
	.align	2
$LC25:
	.ascii	"PerSynFlood\000"
	.align	2
$LC26:
	.ascii	"PerFinFlood\000"
	.align	2
$LC27:
	.ascii	"PerUdpFlood\000"
	.align	2
$LC28:
	.ascii	"PerIcmpFlood\000"
	.align	2
$LC29:
	.ascii	"PerPingFlood\000"
	.align	2
$LC30:
	.ascii	"TcpUdpPortScan\000"
	.align	2
$LC31:
	.ascii	"IcmpSmurf\000"
	.align	2
$LC32:
	.ascii	"IpLand\000"
	.align	2
$LC33:
	.ascii	"IpSpoof\000"
	.align	2
$LC34:
	.ascii	"TearDrop\000"
	.align	2
$LC35:
	.ascii	"PingOfDeath\000"
	.align	2
$LC36:
	.ascii	"TcpScan\000"
	.align	2
$LC37:
	.ascii	"TcpSynWithData\000"
	.align	2
$LC38:
	.ascii	"UdpBomb\000"
	.align	2
$LC39:
	.ascii	"UdpEchoChargen\000"
	.align	2
$LC40:
	.ascii	"UdpFraggle\000"
	.align	2
$LC41:
	.ascii	"IcmpEchoStorm\000"
	.align	2
$LC42:
	.ascii	"UdpSnorkAttack\000"
	.align	2
$LC43:
	.ascii	"TcpInvalidFlags\000"
	.align	2
$LC44:
	.ascii	"TcpRstAttack\000"
	.align	2
$LC45:
	.ascii	"BcMcSrcIpAttack\000"
	.align	2
$LC46:
	.ascii	"ShortIpPkt\000"
	.align	2
$LC47:
	.ascii	"IcmpPingSweep\000"
	.align	2
$LC48:
	.ascii	"IcmpEchoWithoutReply\000"
	.align	2
$LC49:
	.ascii	"TcpInvalidOptions\000"
	.align	2
$LC50:
	.ascii	"LoopBackEchoChargen\000"
	.align	2
$LC51:
	.ascii	"MaxDosType\000"
	.rdata
	.align	2
	.type	rtl_dos_name, @object
	.size	rtl_dos_name, 112
rtl_dos_name:
	.word	$LC24
	.word	$LC25
	.word	$LC26
	.word	$LC27
	.word	$LC28
	.word	$LC29
	.word	$LC30
	.word	$LC31
	.word	$LC32
	.word	$LC33
	.word	$LC34
	.word	$LC35
	.word	$LC36
	.word	$LC37
	.word	$LC38
	.word	$LC39
	.word	$LC40
	.word	$LC41
	.word	$LC42
	.word	$LC43
	.word	$LC44
	.word	$LC45
	.word	$LC46
	.word	$LC47
	.word	$LC48
	.word	$LC49
	.word	$LC50
	.word	$LC51
	.ident	"GCC: (Realtek MSDK-6.4.1 Build 3055) 6.4.1 20180425"
