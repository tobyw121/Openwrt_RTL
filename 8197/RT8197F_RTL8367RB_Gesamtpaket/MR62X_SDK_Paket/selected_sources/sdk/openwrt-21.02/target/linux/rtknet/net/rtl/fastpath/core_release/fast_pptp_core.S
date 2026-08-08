	.file	1 "fast_pptp_core.c"
	.section .mdebug.abi32
	.previous
	.nan	legacy
	.module	softfloat
	.module	oddspreg
	.text
	.align	2
	.globl	sync_tx_pptp_gre_seqno
	.set	nomips16
	.set	nomicromips
	.ent	sync_tx_pptp_gre_seqno
	.type	sync_tx_pptp_gre_seqno, @function
sync_tx_pptp_gre_seqno:
	.frame	$sp,40,$31		# vars= 8, regs= 4/0, args= 16, gp= 0
	.mask	0x80070000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-40
	sw	$31,36($sp)
	sw	$18,32($sp)
	sw	$17,28($sp)
	sw	$16,24($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	move	$18,$4
	.set	macro
	.set	reorder

	move	$17,$2
	lui	$2,%hi(fast_pptp_fw)
	lw	$2,%lo(fast_pptp_fw)($2)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L33
	lw	$31,36($sp)
	.set	macro
	.set	reorder

 #APP
 # 40 "./arch/mips/include/asm/irqflags.h" 1
		.set	push						
	.set	reorder						
	.set	noat						
	di	$16					
	andi	$16, 1					
	sll $0, $0, 3			
	.set	pop						

 # 0 "" 2
 #NO_APP
	lbu	$3,23($17)
	li	$2,47			# 0x2f
	beq	$3,$2,$L31
$L4:
	move	$2,$16
$L34:
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
	lw	$31,36($sp)
$L33:
	lw	$18,32($sp)
	lw	$17,28($sp)
	lw	$16,24($sp)
	.set	noreorder
	.set	nomacro
	jr	$31
	addiu	$sp,$sp,40
	.set	macro
	.set	reorder

$L31:
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_len
	move	$4,$18
	.set	macro
	.set	reorder

	sltu	$2,$2,35
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L34
	move	$2,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	move	$4,$18
	.set	macro
	.set	reorder

	li	$4,1			# 0x1
	lw	$3,14($17)
	andi	$3,$3,0xf
	sll	$3,$3,2
	addiu	$3,$3,14
	addu	$2,$2,$3
	lbu	$3,1($2)
	lbu	$7,2($2)
	lbu	$6,3($2)
	andi	$5,$3,0x7
	sb	$7,20($sp)
	.set	noreorder
	.set	nomacro
	bne	$5,$4,$L4
	sb	$6,21($sp)
	.set	macro
	.set	reorder

	lhu	$4,20($sp)
	li	$5,34827			# 0x880b
	wsbh	$4,$4
	andi	$4,$4,0xffff
	bne	$4,$5,$L4
	lbu	$4,0($2)
	andi	$4,$4,0x10
	.set	noreorder
	.set	nomacro
	bne	$4,$0,$L26
	lui	$5,%hi(pptpAccInfo)
	.set	macro
	.set	reorder

	addiu	$5,$5,%lo(pptpAccInfo)
$L9:
	seb	$3,$3
	.set	noreorder
	.set	nomacro
	bgez	$3,$L15
	move	$7,$0
	.set	macro
	.set	reorder

	lhu	$4,6($2)
$L35:
	lhu	$3,18($5)
	bne	$4,$3,$L4
	lw	$4,32($5)
	lw	$3,30($17)
	.set	noreorder
	.set	nomacro
	bne	$4,$3,$L4
	addiu	$4,$2,12
	.set	macro
	.set	reorder

	lw	$3,40($5)
	addiu	$6,$sp,16
	addiu	$2,$2,16
	wsbh	$3,$3
	ror	$3,$3,16
	sw	$3,16($sp)
$L12:
	lbu	$3,0($6)
	addiu	$4,$4,1
	addiu	$6,$6,1
	.set	noreorder
	.set	nomacro
	bne	$2,$4,$L12
	sb	$3,-1($4)
	.set	macro
	.set	reorder

	bne	$7,$0,$L4
$L15:
	lhu	$2,44($5)
	addiu	$4,$17,30
	addiu	$2,$2,1
	andi	$2,$2,0xffff
	wsbh	$3,$2
	sh	$2,44($5)
	sh	$0,24($17)
	sh	$3,18($17)
	lw	$6,14($17)
	lw	$3,18($17)
	lw	$2,22($17)
	andi	$5,$6,0xf
	lw	$7,26($17)
	addu	$6,$6,$3
	sll	$5,$5,2
	addu	$8,$6,$2
	sltu	$3,$6,$3
	addu	$3,$3,$8
	addiu	$5,$5,14
	addu	$6,$3,$7
	sltu	$2,$3,$2
	addu	$2,$2,$6
	addu	$5,$17,$5
	sltu	$7,$2,$7
	addu	$2,$2,$7
$L13:
	lw	$3,0($4)
	addiu	$4,$4,4
	addu	$2,$3,$2
	sltu	$3,$2,$3
	.set	noreorder
	.set	nomacro
	bne	$5,$4,$L13
	addu	$2,$2,$3
	.set	macro
	.set	reorder

	sll	$3,$2,16
	addu	$3,$3,$2
	srl	$4,$3,16
	sltu	$2,$3,$2
	addu	$2,$2,$4
	nor	$2,$0,$2
	.set	noreorder
	.set	nomacro
	b	$L4
	sh	$2,24($17)
	.set	macro
	.set	reorder

$L26:
	addiu	$5,$5,%lo(pptpAccInfo)
	lhu	$6,18($17)
	lhu	$4,46($5)
	beq	$6,$4,$L10
	lhu	$6,6($2)
	lhu	$4,18($5)
	beq	$6,$4,$L32
$L10:
	seb	$3,$3
	.set	noreorder
	.set	nomacro
	bgez	$3,$L4
	li	$7,1			# 0x1
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L35
	lhu	$4,6($2)
	.set	macro
	.set	reorder

$L32:
	lw	$4,32($5)
	lw	$3,30($17)
	.set	noreorder
	.set	nomacro
	bne	$4,$3,$L4
	addiu	$8,$sp,16
	.set	macro
	.set	reorder

	lw	$3,8($2)
	lw	$4,36($5)
	addiu	$6,$2,8
	addiu	$11,$2,12
	wsbh	$3,$3
	wsbh	$7,$4
	ror	$3,$3,16
	addiu	$4,$4,1
	srl	$10,$3,8
	sb	$3,48($5)
	srl	$9,$3,16
	sw	$4,36($5)
	srl	$3,$3,24
	sb	$10,49($5)
	ror	$7,$7,16
	sb	$9,50($5)
	sb	$3,51($5)
	sw	$7,16($sp)
$L11:
	lbu	$3,0($8)
	addiu	$6,$6,1
	addiu	$8,$8,1
	.set	noreorder
	.set	nomacro
	bne	$11,$6,$L11
	sb	$3,-1($6)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L9
	lbu	$3,1($2)
	.set	macro
	.set	reorder

	.end	sync_tx_pptp_gre_seqno
	.size	sync_tx_pptp_gre_seqno, .-sync_tx_pptp_gre_seqno
	.section	.rodata.str1.4,"aMS",@progbits,1
	.align	2
$LC0:
	.ascii	"%d\012\000"
	.text
	.align	2
	.globl	ppfw_read_proc
	.set	nomips16
	.set	nomicromips
	.ent	ppfw_read_proc
	.type	ppfw_read_proc, @function
ppfw_read_proc:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	lui	$2,%hi(fast_pptp_fw)
	lui	$5,%hi($LC0)
	lw	$6,%lo(fast_pptp_fw)($2)
	jal	seq_printf
	addiu	$5,$5,%lo($LC0)

	move	$2,$0
	lw	$31,20($sp)
	jr	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	ppfw_read_proc
	.size	ppfw_read_proc, .-ppfw_read_proc
	.align	2
	.globl	pptpconn_read_proc
	.set	nomips16
	.set	nomicromips
	.ent	pptpconn_read_proc
	.type	pptpconn_read_proc, @function
pptpconn_read_proc:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	lui	$2,%hi(pptp_conn_check)
	lui	$5,%hi($LC0)
	lw	$6,%lo(pptp_conn_check)($2)
	jal	seq_printf
	addiu	$5,$5,%lo($LC0)

	move	$2,$0
	lw	$31,20($sp)
	jr	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	pptpconn_read_proc
	.size	pptpconn_read_proc, .-pptpconn_read_proc
	.align	2
	.globl	fastpath_pptp_single_open
	.set	nomips16
	.set	nomicromips
	.ent	fastpath_pptp_single_open
	.type	fastpath_pptp_single_open, @function
fastpath_pptp_single_open:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	move	$4,$5
	lui	$5,%hi(ppfw_read_proc)
	move	$6,$0
	addiu	$5,$5,%lo(ppfw_read_proc)
	j	single_open
	nop

	.set	macro
	.set	reorder
	.end	fastpath_pptp_single_open
	.size	fastpath_pptp_single_open, .-fastpath_pptp_single_open
	.align	2
	.globl	pptp_conn_single_open
	.set	nomips16
	.set	nomicromips
	.ent	pptp_conn_single_open
	.type	pptp_conn_single_open, @function
pptp_conn_single_open:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	move	$4,$5
	lui	$5,%hi(pptpconn_read_proc)
	move	$6,$0
	addiu	$5,$5,%lo(pptpconn_read_proc)
	j	single_open
	nop

	.set	macro
	.set	reorder
	.end	pptp_conn_single_open
	.size	pptp_conn_single_open, .-pptp_conn_single_open
	.section	.rodata.str1.4
	.align	2
$LC1:
	.ascii	"fast_pptp\000"
	.align	2
$LC2:
	.ascii	"pptp_conn_ck\000"
	.section	.init.text,"ax",@progbits
	.align	2
	.globl	fast_pptp_init
	.set	nomips16
	.set	nomicromips
	.ent	fast_pptp_init
	.type	fast_pptp_init, @function
fast_pptp_init:
	.frame	$sp,32,$31		# vars= 0, regs= 2/0, args= 24, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$16,24($sp)
	lui	$16,%hi(proc_root)
	lui	$7,%hi(fastpath_pptp_proc_fops)
	sw	$0,16($sp)
	lui	$4,%hi($LC1)
	addiu	$6,$16,%lo(proc_root)
	addiu	$7,$7,%lo(fastpath_pptp_proc_fops)
	move	$5,$0
	jal	proc_create_data
	addiu	$4,$4,%lo($LC1)

	lui	$7,%hi(pptp_conn_proc_fops)
	lui	$4,%hi($LC2)
	sw	$0,16($sp)
	lui	$3,%hi(res1)
	addiu	$7,$7,%lo(pptp_conn_proc_fops)
	addiu	$6,$16,%lo(proc_root)
	move	$5,$0
	sw	$2,%lo(res1)($3)
	jal	proc_create_data
	addiu	$4,$4,%lo($LC2)

	lui	$4,%hi(pptpAccInfo)
	lui	$3,%hi(res_check_pptp)
	li	$6,64			# 0x40
	move	$5,$0
	addiu	$4,$4,%lo(pptpAccInfo)
	jal	memset
	sw	$2,%lo(res_check_pptp)($3)

	move	$2,$0
	lw	$31,28($sp)
	lw	$16,24($sp)
	jr	$31
	addiu	$sp,$sp,32

	.set	macro
	.set	reorder
	.end	fast_pptp_init
	.size	fast_pptp_init, .-fast_pptp_init
	.section	.exit.text,"ax",@progbits
	.align	2
	.globl	fast_pptp_exit
	.set	nomips16
	.set	nomicromips
	.ent	fast_pptp_exit
	.type	fast_pptp_exit, @function
fast_pptp_exit:
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
	beq	$2,$0,$L45
	lui	$5,%hi(proc_root)

	lui	$4,%hi($LC1)
	addiu	$5,$5,%lo(proc_root)
	jal	remove_proc_entry
	addiu	$4,$4,%lo($LC1)

	sw	$0,%lo(res1)($16)
$L45:
	lui	$16,%hi(res_check_pptp)
	lw	$2,%lo(res_check_pptp)($16)
	beq	$2,$0,$L44
	lui	$5,%hi(proc_root)

	lui	$4,%hi($LC2)
	addiu	$5,$5,%lo(proc_root)
	jal	remove_proc_entry
	addiu	$4,$4,%lo($LC2)

	sw	$0,%lo(res_check_pptp)($16)
$L44:
	lw	$31,20($sp)
	lw	$16,16($sp)
	jr	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	fast_pptp_exit
	.size	fast_pptp_exit, .-fast_pptp_exit
	.text
	.align	2
	.globl	is_pptp_device
	.set	nomips16
	.set	nomicromips
	.ent	is_pptp_device
	.type	is_pptp_device, @function
is_pptp_device:
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	lui	$2,%hi(pptpAccInfo+56)
	lw	$2,%lo(pptpAccInfo+56)($2)
	beq	$2,$0,$L56
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

$L56:
	.set	noreorder
	.set	nomacro
	jr	$31
	move	$2,$0
	.set	macro
	.set	reorder

	.end	is_pptp_device
	.size	is_pptp_device, .-is_pptp_device
	.align	2
	.globl	set_pptp_device
	.set	nomips16
	.set	nomicromips
	.ent	set_pptp_device
	.type	set_pptp_device, @function
set_pptp_device:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	lui	$2,%hi(fast_pptp_fw)
	lw	$2,%lo(fast_pptp_fw)($2)
	bne	$2,$0,$L71
	nop

	jr	$31
	nop

$L71:
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	jal	rtl_get_dev_by_name
	nop

	lui	$3,%hi(pptpAccInfo+56)
	lw	$31,20($sp)
	sw	$2,%lo(pptpAccInfo+56)($3)
	jr	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	set_pptp_device
	.size	set_pptp_device, .-set_pptp_device
	.align	2
	.globl	Check_GRE_rx_net_device
	.set	nomips16
	.set	nomicromips
	.ent	Check_GRE_rx_net_device
	.type	Check_GRE_rx_net_device, @function
Check_GRE_rx_net_device:
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
	lui	$16,%hi(pptpAccInfo)
	jal	rtl_ip_hdr
	move	$18,$4

	addiu	$16,$16,%lo(pptpAccInfo)
	move	$4,$18
	jal	rtl_get_skb_data
	move	$19,$2

	lw	$17,8($16)
	beq	$17,$0,$L73
	move	$20,$2

	lw	$2,0($19)
	move	$4,$18
	jal	rtl_get_skb_dev
	andi	$18,$2,0xf

	beq	$17,$2,$L80
	sll	$2,$18,2

$L73:
	move	$2,$0
	lw	$31,36($sp)
$L81:
	lw	$20,32($sp)
	lw	$19,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	jr	$31
	addiu	$sp,$sp,40

$L80:
	lhu	$3,30($16)
	addu	$2,$20,$2
	lhu	$2,6($2)
	bne	$2,$3,$L73
	li	$2,1			# 0x1

	b	$L81
	lw	$31,36($sp)

	.set	macro
	.set	reorder
	.end	Check_GRE_rx_net_device
	.size	Check_GRE_rx_net_device, .-Check_GRE_rx_net_device
	.align	2
	.globl	fast_pptp_filter
	.set	nomips16
	.set	nomicromips
	.ent	fast_pptp_filter
	.type	fast_pptp_filter, @function
fast_pptp_filter:
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
	jal	rtl_ip_hdr
	move	$17,$4

	move	$4,$17
	jal	rtl_get_skb_len
	move	$16,$2

	sltu	$2,$2,56
	bne	$2,$0,$L82
	li	$2,6			# 0x6

	lbu	$3,9($16)
	beq	$3,$2,$L108
	li	$3,47878			# 0xbb06

$L82:
	lw	$31,36($sp)
$L110:
	lw	$19,32($sp)
$L111:
	lw	$18,28($sp)
	lw	$17,24($sp)
	lw	$16,20($sp)
	jr	$31
	addiu	$sp,$sp,40

$L108:
	lw	$2,0($16)
	andi	$2,$2,0xf
	sll	$2,$2,2
	addu	$2,$16,$2
	lhu	$4,0($2)
	bne	$4,$3,$L110
	lw	$31,36($sp)

	lw	$3,12($2)
	li	$4,1			# 0x1
	ext	$3,$3,4,4
	sll	$3,$3,2
	addu	$2,$2,$3
	lhu	$3,2($2)
	wsbh	$3,$3
	andi	$3,$3,0xffff
	bne	$3,$4,$L111
	lw	$19,32($sp)

	lhu	$4,8($2)
	li	$6,8			# 0x8
	wsbh	$5,$4
	andi	$5,$5,0xffff
	beq	$5,$6,$L109
	nop

	wsbh	$3,$4
$L112:
	li	$4,13			# 0xd
	andi	$3,$3,0xffff
	bne	$3,$4,$L82
	lui	$18,%hi(pptpAccInfo)

	addiu	$18,$18,%lo(pptpAccInfo)
	lw	$3,36($18)
	beq	$3,$0,$L86
	nop

	lw	$3,20($18)
	beq	$3,$0,$L86
	nop

	lw	$3,32($18)
	beq	$3,$0,$L86
	nop

	lhu	$3,18($18)
	lhu	$2,12($2)
	bne	$3,$2,$L110
	lw	$31,36($sp)

$L86:
	lui	$2,%hi(pptp_tcp_finished)
	sw	$0,4($18)
	b	$L82
	sw	$0,%lo(pptp_tcp_finished)($2)

$L109:
	lbu	$5,16($2)
	bne	$5,$3,$L112
	wsbh	$3,$4

	lui	$18,%hi(pptpAccInfo)
	addiu	$18,$18,%lo(pptpAccInfo)
	lw	$3,36($18)
	beq	$3,$0,$L107
	nop

	lw	$3,20($18)
	beq	$3,$0,$L107
	nop

	lw	$3,32($18)
	beq	$3,$0,$L107
	nop

	lhu	$4,18($18)
	lhu	$3,12($2)
	bne	$4,$3,$L110
	lw	$31,36($sp)

	lhu	$5,30($18)
	lhu	$4,14($2)
	bne	$5,$4,$L111
	lw	$19,32($sp)

$L85:
	sh	$3,18($18)
	lui	$3,%hi(pptp_tcp_finished)
	lhu	$2,14($2)
	li	$19,1			# 0x1
	move	$4,$17
	sw	$19,%lo(pptp_tcp_finished)($3)
	sh	$2,30($18)
	lw	$2,16($16)
	sw	$2,20($18)
	lw	$2,12($16)
	sw	$19,36($18)
	jal	rtl_get_skb_dev
	sw	$2,32($18)

	move	$4,$17
	jal	rtl_get_skb_data
	sw	$2,8($18)

	lui	$4,%hi(pptpAccInfo+24)
	li	$6,6			# 0x6
	addiu	$5,$2,-8
	jal	memcpy
	addiu	$4,$4,%lo(pptpAccInfo+24)

	jal	rtl_get_skb_data
	move	$4,$17

	lui	$4,%hi(pptpAccInfo+12)
	li	$6,6			# 0x6
	addiu	$5,$2,-14
	jal	memcpy
	addiu	$4,$4,%lo(pptpAccInfo+12)

	b	$L82
	sw	$19,4($18)

$L107:
	b	$L85
	lhu	$3,12($2)

	.set	macro
	.set	reorder
	.end	fast_pptp_filter
	.size	fast_pptp_filter, .-fast_pptp_filter
	.align	2
	.globl	fast_pptp_to_lan
	.set	nomips16
	.set	nomicromips
	.ent	fast_pptp_to_lan
	.type	fast_pptp_to_lan, @function
fast_pptp_to_lan:
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
	lui	$16,%hi(pptpAccInfo)
	lw	$17,0($4)
	move	$18,$4
	addiu	$16,$16,%lo(pptpAccInfo)
	jal	rtl_get_skb_dev
	move	$4,$17

	lw	$3,8($16)
	beq	$3,$2,$L114
	nop

$L116:
	move	$2,$0
$L113:
	lw	$31,44($sp)
$L157:
	lw	$21,40($sp)
	lw	$20,36($sp)
	lw	$19,32($sp)
	lw	$18,28($sp)
	lw	$17,24($sp)
	lw	$16,20($sp)
	jr	$31
	addiu	$sp,$sp,48

$L114:
	jal	rtl_ip_hdr
	move	$4,$17

	li	$3,47			# 0x2f
	lbu	$2,9($2)
	bne	$2,$3,$L113
	move	$2,$0

	jal	rtl_get_skb_len
	move	$4,$17

	sltu	$2,$2,20
	bne	$2,$0,$L113
	move	$2,$0

	jal	rtl_ip_hdr
	move	$4,$17

	move	$4,$17
	jal	rtl_get_skb_data
	move	$19,$2

	li	$4,1			# 0x1
	lw	$3,0($19)
	andi	$3,$3,0xf
	sll	$3,$3,2
	addu	$2,$2,$3
	lbu	$3,1($2)
	andi	$3,$3,0x7
	bne	$3,$4,$L116
	nop

	lhu	$3,2($2)
	li	$4,34827			# 0x880b
	wsbh	$3,$3
	andi	$3,$3,0xffff
	bne	$3,$4,$L116
	nop

	lhu	$4,6($2)
	lhu	$3,30($16)
	bne	$4,$3,$L116
	nop

	lbu	$3,0($2)
	andi	$3,$3,0x10
	beq	$3,$0,$L131
	nop

	lw	$3,8($2)
	li	$21,12			# 0xc
	wsbh	$3,$3
	ror	$3,$3,16
	sw	$3,40($16)
$L117:
	lb	$3,1($2)
	addiu	$5,$21,4
	lhu	$4,4($2)
	slt	$3,$3,0
	beq	$4,$0,$L116
	movn	$21,$5,$3

	addu	$2,$2,$21
	lbu	$3,0($2)
	bne	$3,$0,$L119
	li	$4,255			# 0xff

	addiu	$2,$2,1
	li	$20,1			# 0x1
$L120:
	lbu	$2,0($2)
$L158:
	li	$3,33			# 0x21
	beq	$2,$3,$L121
	li	$3,253			# 0xfd

	bne	$2,$3,$L113
	move	$2,$0

	lw	$4,56($16)
	beq	$4,$0,$L157
	lw	$31,44($sp)

	lw	$19,0($19)
	jal	rtl_get_ppp_dev_priv
	andi	$19,$19,0xf

	beq	$2,$0,$L116
	sll	$5,$19,2

	move	$4,$17
	addu	$5,$5,$21
	addu	$5,$20,$5
	jal	skb_pull
	addiu	$5,$5,-1

	jal	rtl_get_skb_data
	move	$4,$17

	li	$3,-3			# 0xfffffffffffffffd
	sb	$0,0($2)
	sb	$3,1($2)
	jal	rtl_get_ppp_dev_priv
	lw	$4,56($16)

	move	$4,$2
	jal	rtl_ppp_recv_lock
	move	$19,$2

	li	$6,1			# 0x1
	move	$5,$17
	jal	rtl_ppp_receive_nonmp_frame
	move	$4,$19

	move	$4,$19
	jal	rtl_ppp_recv_unlock
	move	$17,$2

	beq	$17,$0,$L134
	li	$19,1			# 0x1

	sw	$17,0($18)
$L130:
	jal	rtl_get_skb_data
	move	$4,$17

	andi	$18,$2,0x3
	bne	$18,$0,$L153
	nop

$L124:
	jal	rtl_skb_reset_network_header
	move	$4,$17

	jal	rtl_skb_reset_transport_header
	move	$4,$17

	lw	$4,56($16)
	beq	$4,$0,$L113
	move	$2,$0

	beq	$19,$0,$L154
	nop

$L127:
	lw	$5,56($16)
$L156:
	jal	rtl_set_skb_dev
	move	$4,$17

	b	$L113
	li	$2,1			# 0x1

$L131:
	b	$L117
	li	$21,8			# 0x8

$L119:
	beq	$3,$4,$L155
	move	$20,$0

	b	$L158
	lbu	$2,0($2)

$L154:
	jal	rtl_get_ppp_dev_priv
	nop

	beq	$2,$0,$L127
	nop

	jal	rtl_get_ppp_dev_priv
	lw	$4,56($16)

	move	$4,$17
	jal	rtl_get_skb_len
	move	$18,$2

	addiu	$6,$2,-2
	move	$5,$0
	jal	rtl_inc_ppp_stats
	move	$4,$18

	b	$L156
	lw	$5,56($16)

$L153:
	jal	rtl_skb_headroom
	move	$4,$17

	sltu	$2,$2,$18
	bne	$2,$0,$L124
	nop

	jal	rtl_get_skb_data
	move	$4,$17

	move	$4,$17
	jal	rtl_get_skb_data
	subu	$20,$2,$18

	move	$4,$17
	jal	rtl_get_skb_len
	move	$21,$2

	move	$6,$2
	move	$5,$21
	jal	memmove
	move	$4,$20

	li	$6,1			# 0x1
	move	$5,$18
	jal	rtl_set_skb_data
	move	$4,$17

	li	$6,1			# 0x1
	move	$5,$18
	jal	rtl_set_skb_tail
	move	$4,$17

	b	$L124
	nop

$L121:
	lw	$5,0($19)
	addiu	$20,$20,1
	move	$4,$17
	move	$19,$0
	andi	$5,$5,0xf
	sll	$5,$5,2
	addu	$5,$5,$21
	jal	skb_pull
	addu	$5,$20,$5

	b	$L130
	nop

$L134:
	b	$L113
	li	$2,-2			# 0xfffffffffffffffe

$L155:
	lbu	$4,1($2)
	li	$3,3			# 0x3
	bne	$4,$3,$L116
	nop

	lbu	$4,2($2)
	bne	$4,$0,$L133
	addiu	$3,$2,2

	addiu	$2,$2,3
	b	$L120
	li	$20,3			# 0x3

$L133:
	li	$20,2			# 0x2
	b	$L120
	move	$2,$3

	.set	macro
	.set	reorder
	.end	fast_pptp_to_lan
	.size	fast_pptp_to_lan, .-fast_pptp_to_lan
	.align	2
	.globl	get_fastpptp_lastxmit
	.set	nomips16
	.set	nomicromips
	.ent	get_fastpptp_lastxmit
	.type	get_fastpptp_lastxmit, @function
get_fastpptp_lastxmit:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	lui	$2,%hi(fast_pptp_fw)
	lw	$2,%lo(fast_pptp_fw)($2)
	beq	$2,$0,$L162
	nop

	lui	$2,%hi(pptpAccInfo)
	li	$3,1			# 0x1
	addiu	$4,$2,%lo(pptpAccInfo)
	lw	$4,4($4)
	beq	$4,$3,$L163
	nop

$L162:
	jr	$31
	move	$2,$0

$L163:
	jr	$31
	lw	$2,%lo(pptpAccInfo)($2)

	.set	macro
	.set	reorder
	.end	get_fastpptp_lastxmit
	.size	get_fastpptp_lastxmit, .-get_fastpptp_lastxmit
	.section	.rodata.str1.4
	.align	2
$LC3:
	.ascii	"ppp0\000"
	.align	2
$LC4:
	.ascii	"pptpInfo.ppp0_dev == NULL\012\000"
	.align	2
$LC5:
	.ascii	"RTL\000"
	.align	2
$LC6:
	.ascii	"pptpInfo.ppp0_dev->priv == NULL\012\000"
	.text
	.align	2
	.globl	fast_pptp_to_wan
	.set	nomips16
	.set	nomicromips
	.ent	fast_pptp_to_wan
	.type	fast_pptp_to_wan, @function
fast_pptp_to_wan:
	.frame	$sp,88,$31		# vars= 40, regs= 7/0, args= 16, gp= 0
	.mask	0x803f0000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-88
	sw	$31,84($sp)
	sw	$21,80($sp)
	sw	$20,76($sp)
	sw	$19,72($sp)
	sw	$18,68($sp)
	sw	$17,64($sp)
	sw	$16,60($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_ip_hdr
	move	$18,$4
	.set	macro
	.set	reorder

	move	$4,$18
	.set	noreorder
	.set	nomacro
	jal	rtl_ip_hdr
	lbu	$16,12($2)
	.set	macro
	.set	reorder

	move	$4,$18
	.set	noreorder
	.set	nomacro
	jal	rtl_ip_hdr
	lw	$17,12($2)
	.set	macro
	.set	reorder

	li	$3,10			# 0xa
	.set	noreorder
	.set	nomacro
	beq	$16,$3,$L198
	li	$3,16384			# 0x4000
	.set	macro
	.set	reorder

	lui	$19,%hi(pptpAccInfo)
$L204:
	addiu	$17,$19,%lo(pptpAccInfo)
$L205:
	lw	$4,56($17)
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L164
	move	$16,$0
	.set	macro
	.set	reorder

	jal	rtl_get_ppp_dev_name
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L203
	move	$2,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_ppp_dev_name
	lw	$4,56($17)
	.set	macro
	.set	reorder

	lui	$3,%hi($LC3)
	addiu	$3,$3,%lo($LC3)
 #APP
 # 77 "./arch/mips/include/asm/string.h" 1
	.set	noreorder
	.set	noat
	lbu	$16,($2)
1:	lbu	$1,($3)
	addiu	$2,1
	bne	$1,$16,2f
	addiu	$3,1
	bnez	$16,1b
	lbu	$16,($2)
	move	$16,$1
2:	subu	$16,$1
3:	.set	at
	.set	reorder
 # 0 "" 2
 #NO_APP
	bne	$16,$0,$L167
	lw	$2,4($17)
	beq	$2,$0,$L168
	lw	$4,56($17)
	beq	$4,$0,$L178
	jal	rtl_get_ppp_dev_priv
	beq	$2,$0,$L168
	.set	noreorder
	.set	nomacro
	jal	rtl_get_ppp_dev_priv
	lw	$4,56($17)
	.set	macro
	.set	reorder

	move	$4,$2
	.set	noreorder
	.set	nomacro
	jal	rtl_ppp_vj_check
	move	$20,$2
	.set	macro
	.set	reorder

	li	$3,1			# 0x1
	beq	$2,$3,$L167
	.set	noreorder
	.set	nomacro
	jal	rtl_ip_hdr
	move	$4,$18
	.set	macro
	.set	reorder

	move	$4,$20
	.set	noreorder
	.set	nomacro
	jal	rtl_ppp_xmit_lock
	lbu	$21,1($2)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_ppp_xmit_pending
	move	$4,$20
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L170
	lui	$5,%hi($LC5)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_ppp_xmit_unlock
	move	$4,$20
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L203
	move	$2,$16
	.set	macro
	.set	reorder

$L168:
	lw	$4,56($17)
	beq	$4,$0,$L178
	jal	rtl_get_ppp_dev_priv
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L199
	lui	$4,%hi($LC6)
	.set	macro
	.set	reorder

$L167:
	move	$16,$0
$L164:
	move	$2,$16
$L203:
	lw	$31,84($sp)
	lw	$21,80($sp)
	lw	$20,76($sp)
	lw	$19,72($sp)
	lw	$18,68($sp)
	lw	$17,64($sp)
	lw	$16,60($sp)
	.set	noreorder
	.set	nomacro
	jr	$31
	addiu	$sp,$sp,88
	.set	macro
	.set	reorder

$L198:
	andi	$17,$17,0xff00
	.set	noreorder
	.set	nomacro
	bne	$17,$3,$L204
	lui	$19,%hi(pptpAccInfo)
	.set	macro
	.set	reorder

	lw	$2,12($2)
	li	$4,16711680			# 0xff0000
	li	$3,4194304			# 0x400000
	and	$2,$2,$4
	.set	noreorder
	.set	nomacro
	bne	$2,$3,$L205
	addiu	$17,$19,%lo(pptpAccInfo)
	.set	macro
	.set	reorder

	lui	$3,%hi(pptp_conn_check)
	li	$2,3			# 0x3
	lw	$3,%lo(pptp_conn_check)($3)
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L205
	move	$16,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L203
	move	$2,$16
	.set	macro
	.set	reorder

$L178:
	lui	$4,%hi($LC4)
	.set	noreorder
	.set	nomacro
	jal	printk
	addiu	$4,$4,%lo($LC4)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L203
	move	$2,$16
	.set	macro
	.set	reorder

$L170:
	li	$6,3			# 0x3
	addiu	$5,$5,%lo($LC5)
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_cb
	move	$4,$18
	.set	macro
	.set	reorder

	move	$4,$18
	.set	noreorder
	.set	nomacro
	jal	rtl_ppp_start_xmit
	lw	$5,56($17)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_ppp_xmit_pending
	move	$4,$20
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L200
	move	$4,$20
	.set	macro
	.set	reorder

	jal	rtl_get_ppp_xmit_pending
	move	$5,$0
	move	$4,$20
	.set	noreorder
	.set	nomacro
	jal	rtl_set_ppp_xmit_pending
	move	$18,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_ppp_xmit_unlock
	move	$4,$20
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_skb_headroom
	move	$4,$18
	.set	macro
	.set	reorder

	sltu	$2,$2,50
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L206
	li	$5,50			# 0x32
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_skb_cloned
	move	$4,$18
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L201
	li	$5,50			# 0x32
	.set	macro
	.set	reorder

$L206:
	.set	noreorder
	.set	nomacro
	jal	skb_realloc_headroom
	move	$4,$18
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L202
	move	$16,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	consume_skb
	move	$4,$18
	.set	macro
	.set	reorder

	li	$5,50			# 0x32
$L207:
	.set	noreorder
	.set	nomacro
	jal	skb_push
	move	$4,$16
	.set	macro
	.set	reorder

	lui	$5,%hi(pptpAccInfo+24)
	li	$6,6			# 0x6
	addiu	$5,$5,%lo(pptpAccInfo+24)
	move	$4,$2
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$18,$2
	.set	macro
	.set	reorder

	lui	$5,%hi(pptpAccInfo+12)
	li	$6,6			# 0x6
	addiu	$5,$5,%lo(pptpAccInfo+12)
	.set	noreorder
	.set	nomacro
	jal	memcpy
	addiu	$4,$18,6
	.set	macro
	.set	reorder

	li	$3,8			# 0x8
	lbu	$2,16($sp)
	li	$4,4			# 0x4
	sb	$3,12($18)
	li	$3,64			# 0x40
	sb	$0,13($18)
	move	$5,$0
	sh	$3,22($sp)
	ins	$2,$4,4,4
	sb	$3,24($sp)
	li	$3,47			# 0x2f
	sb	$21,17($sp)
	move	$4,$16
	sb	$3,25($sp)
	li	$3,5			# 0x5
	lw	$6,32($17)
	ins	$2,$3,0,4
	lw	$3,20($17)
	sw	$6,32($sp)
	sw	$3,28($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_ip_summed
	sb	$2,16($sp)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_len
	move	$4,$16
	.set	macro
	.set	reorder

	addiu	$4,$sp,32
	lhu	$3,44($17)
	addiu	$2,$2,-14
	sh	$0,26($sp)
	wsbh	$5,$2
	lw	$2,24($sp)
	addiu	$3,$3,1
	lw	$7,28($sp)
	sh	$5,18($sp)
	andi	$3,$3,0xffff
	lw	$6,16($sp)
	wsbh	$9,$3
	sh	$3,44($17)
	addiu	$3,$sp,16
	sh	$9,20($sp)
	andi	$5,$6,0xf
	lw	$8,20($sp)
	sll	$5,$5,2
	addu	$5,$3,$5
	addu	$3,$6,$8
	addu	$6,$3,$2
	sltu	$3,$3,$8
	addu	$3,$6,$3
	addu	$6,$3,$7
	sltu	$2,$3,$2
	addu	$2,$2,$6
	sltu	$7,$2,$7
	addu	$2,$2,$7
$L177:
	lw	$3,0($4)
	addiu	$4,$4,4
	addu	$2,$3,$2
	sltu	$3,$2,$3
	.set	noreorder
	.set	nomacro
	bne	$5,$4,$L177
	addu	$2,$2,$3
	.set	macro
	.set	reorder

	sll	$3,$2,16
	sh	$9,46($17)
	move	$4,$16
	addu	$3,$3,$2
	srl	$5,$3,16
	sltu	$2,$3,$2
	addu	$2,$2,$5
	nor	$2,$0,$2
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	sh	$2,26($sp)
	.set	macro
	.set	reorder

	li	$6,20			# 0x14
	addiu	$5,$sp,16
	.set	noreorder
	.set	nomacro
	jal	memcpy
	addiu	$4,$2,14
	.set	macro
	.set	reorder

	li	$2,48			# 0x30
	move	$4,$16
	sb	$2,36($sp)
	li	$2,-127			# 0xffffffffffffff81
	sb	$2,37($sp)
	li	$2,2952			# 0xb88
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_len
	sh	$2,38($sp)
	.set	macro
	.set	reorder

	move	$4,$16
	lw	$3,36($17)
	addiu	$6,$2,-50
	lw	$2,40($17)
	lhu	$7,18($17)
	wsbh	$6,$6
	wsbh	$5,$3
	wsbh	$2,$2
	sh	$6,40($sp)
	addiu	$3,$3,1
	sh	$7,42($sp)
	ror	$5,$5,16
	ror	$2,$2,16
	sw	$3,36($17)
	sw	$5,44($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	sw	$2,48($sp)
	.set	macro
	.set	reorder

	li	$6,16			# 0x10
	addiu	$5,$sp,36
	.set	noreorder
	.set	nomacro
	jal	memcpy
	addiu	$4,$2,34
	.set	macro
	.set	reorder

	lui	$2,%hi(jiffies)
	lw	$5,8($17)
	move	$4,$16
	lw	$2,%lo(jiffies)($2)
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_dev
	sw	$2,%lo(pptpAccInfo)($19)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	dev_queue_xmit
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L164
	li	$16,1			# 0x1
	.set	macro
	.set	reorder

$L199:
	.set	noreorder
	.set	nomacro
	jal	printk
	addiu	$4,$4,%lo($LC6)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L203
	move	$2,$16
	.set	macro
	.set	reorder

$L201:
	.set	noreorder
	.set	nomacro
	jal	rtl_skb_shared
	move	$4,$18
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L206
	li	$5,50			# 0x32
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L207
	move	$16,$18
	.set	macro
	.set	reorder

$L200:
	.set	noreorder
	.set	nomacro
	jal	rtl_ppp_xmit_unlock
	li	$16,1			# 0x1
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L203
	move	$2,$16
	.set	macro
	.set	reorder

$L202:
	.set	noreorder
	.set	nomacro
	jal	consume_skb
	move	$4,$18
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L164
	li	$16,1			# 0x1
	.set	macro
	.set	reorder

	.end	fast_pptp_to_wan
	.size	fast_pptp_to_wan, .-fast_pptp_to_wan
	.align	2
	.globl	fast_pptp_sync_rx_seq
	.set	nomips16
	.set	nomicromips
	.ent	fast_pptp_sync_rx_seq
	.type	fast_pptp_sync_rx_seq, @function
fast_pptp_sync_rx_seq:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	jal	rtl_ip_hdr
	nop

	li	$4,1			# 0x1
	lbu	$3,21($2)
	andi	$5,$3,0x7
	beq	$5,$4,$L217
	li	$5,34827			# 0x880b

$L208:
	lw	$31,20($sp)
$L218:
	jr	$31
	addiu	$sp,$sp,24

$L217:
	lhu	$4,22($2)
	wsbh	$4,$4
	andi	$4,$4,0xffff
	bne	$4,$5,$L218
	lw	$31,20($sp)

	lbu	$4,20($2)
	andi	$4,$4,0x10
	beq	$4,$0,$L219
	seb	$3,$3

	lui	$3,%hi(pptpAccInfo)
	addiu	$3,$3,%lo(pptpAccInfo)
	lw	$4,52($3)
	addiu	$5,$4,1
	wsbh	$4,$4
	sw	$5,52($3)
	ror	$4,$4,16
	lbu	$3,21($2)
	sw	$4,28($2)
	seb	$3,$3
$L219:
	bgez	$3,$L218
	lw	$31,20($sp)

	lui	$3,%hi(pptpAccInfo+48)
	lw	$3,%lo(pptpAccInfo+48)($3)
	wsbh	$3,$3
	ror	$3,$3,16
	b	$L208
	sw	$3,32($2)

	.set	macro
	.set	reorder
	.end	fast_pptp_sync_rx_seq
	.size	fast_pptp_sync_rx_seq, .-fast_pptp_sync_rx_seq
	.section	.rodata.str1.4
	.align	2
$LC7:
	.ascii	"net/rtl/fastpath/core/fast_pptp_core.c\000"
	.text
	.align	2
	.globl	pptpconn_write_proc
	.set	nomips16
	.set	nomicromips
	.ent	pptpconn_write_proc
	.type	pptpconn_write_proc, @function
pptpconn_write_proc:
	.frame	$sp,232,$31		# vars= 200, regs= 4/0, args= 16, gp= 0
	.mask	0x80070000,-4
	.fmask	0x00000000,0
	sltu	$2,$6,2
	bne	$2,$0,$L234
	beq	$5,$0,$L234
	addiu	$sp,$sp,-232
	sw	$31,228($sp)
	sw	$18,224($sp)
	sw	$17,220($sp)
	sw	$16,216($sp)
	addu	$3,$6,$5
	lw	$16,24($28)
	or	$2,$6,$5
	or	$2,$2,$3
	move	$17,$6
	and	$16,$2,$16
	.set	noreorder
	.set	nomacro
	bne	$16,$0,$L224
	move	$18,$5
	.set	macro
	.set	reorder

	lui	$4,%hi($LC7)
	li	$5,928			# 0x3a0
	.set	noreorder
	.set	nomacro
	jal	__might_fault
	addiu	$4,$4,%lo($LC7)
	.set	macro
	.set	reorder

	move	$5,$18
	addiu	$4,$sp,16
	move	$6,$17
 #APP
 # 938 "net/rtl/fastpath/core/fast_pptp_core.c" 1
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
	bne	$6,$0,$L237
	lw	$31,228($sp)
	.set	macro
	.set	reorder

	lb	$5,16($sp)
	addiu	$2,$5,-48
	andi	$2,$2,0x00ff
	sltu	$2,$2,10
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L226
	addiu	$4,$sp,16
	.set	macro
	.set	reorder

$L230:
	sll	$2,$16,2
	addiu	$4,$4,1
	addu	$2,$2,$16
	sll	$2,$2,1
	addu	$2,$2,$5
	lb	$5,0($4)
	addiu	$3,$5,-48
	andi	$3,$3,0x00ff
	sltu	$3,$3,10
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L230
	addiu	$16,$2,-48
	.set	macro
	.set	reorder

	move	$2,$16
$L226:
	lui	$3,%hi(pptp_conn_check)
	sw	$2,%lo(pptp_conn_check)($3)
	lw	$31,228($sp)
$L237:
	li	$2,-14			# 0xfffffffffffffff2
	lw	$18,224($sp)
	lw	$17,220($sp)
	lw	$16,216($sp)
	.set	noreorder
	.set	nomacro
	jr	$31
	addiu	$sp,$sp,232
	.set	macro
	.set	reorder

$L234:
	.set	noreorder
	.set	nomacro
	jr	$31
	li	$2,-14			# 0xfffffffffffffff2
	.set	macro
	.set	reorder

$L224:
	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	memset
	addiu	$4,$sp,16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L237
	lw	$31,228($sp)
	.set	macro
	.set	reorder

	.end	pptpconn_write_proc
	.size	pptpconn_write_proc, .-pptpconn_write_proc
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	pptp_conn_single_write
	.type	pptp_conn_single_write, @function
pptp_conn_single_write:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	j	pptpconn_write_proc
	nop

	.set	macro
	.set	reorder
	.end	pptp_conn_single_write
	.size	pptp_conn_single_write, .-pptp_conn_single_write
	.align	2
	.globl	ppfw_write_proc
	.set	nomips16
	.set	nomicromips
	.ent	ppfw_write_proc
	.type	ppfw_write_proc, @function
ppfw_write_proc:
	.frame	$sp,40,$31		# vars= 0, regs= 5/0, args= 16, gp= 0
	.mask	0x800f0000,-4
	.fmask	0x00000000,0
	sltu	$2,$6,2
	bne	$2,$0,$L263
	addiu	$sp,$sp,-40
	sw	$31,36($sp)
	sw	$19,32($sp)
	sw	$18,28($sp)
	sw	$17,24($sp)
	sw	$16,20($sp)
	move	$19,$5
	li	$5,34078720			# 0x2080000
	addiu	$4,$6,32
	addiu	$5,$5,32
	.set	noreorder
	.set	nomacro
	jal	__kmalloc
	move	$17,$6
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L262
	move	$18,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$19,$0,$L247
	addu	$3,$19,$17
	.set	macro
	.set	reorder

	lw	$16,24($28)
	or	$2,$19,$17
	or	$3,$3,$2
	and	$16,$3,$16
	.set	noreorder
	.set	nomacro
	bne	$16,$0,$L244
	li	$5,954			# 0x3ba
	.set	macro
	.set	reorder

	lui	$4,%hi($LC7)
	.set	noreorder
	.set	nomacro
	jal	__might_fault
	addiu	$4,$4,%lo($LC7)
	.set	macro
	.set	reorder

	move	$4,$18
	move	$5,$19
	move	$6,$17
 #APP
 # 964 "net/rtl/fastpath/core/fast_pptp_core.c" 1
	.set	noreorder
	jal	__copy_user
	.set	noat
	addu	$1, $5, $6
	.set	at
	.set	reorder
 # 0 "" 2
 #NO_APP
	bne	$6,$0,$L247
	lb	$2,0($18)
	addiu	$3,$2,-48
	andi	$3,$3,0x00ff
	sltu	$3,$3,10
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L246
	move	$5,$18
	.set	macro
	.set	reorder

$L255:
	sll	$3,$16,2
	addiu	$5,$5,1
	addu	$3,$3,$16
	sll	$3,$3,1
	addu	$3,$3,$2
	lb	$2,0($5)
	addiu	$4,$2,-48
	andi	$4,$4,0x00ff
	sltu	$4,$4,10
	.set	noreorder
	.set	nomacro
	bne	$4,$0,$L255
	addiu	$16,$3,-48
	.set	macro
	.set	reorder

	move	$3,$16
$L246:
	lui	$4,%hi(pptpAccInfo)
	lui	$16,%hi(fast_pptp_fw)
	li	$6,64			# 0x40
	move	$5,$0
	addiu	$4,$4,%lo(pptpAccInfo)
	.set	noreorder
	.set	nomacro
	jal	memset
	sw	$3,%lo(fast_pptp_fw)($16)
	.set	macro
	.set	reorder

	lw	$2,%lo(fast_pptp_fw)($16)
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L264
	lui	$2,%hi(sync_tx_pptp_gre_seqno)
	.set	macro
	.set	reorder

	lui	$2,%hi(sync_tx_pptp_gre_seqno_hook)
	sw	$0,%lo(sync_tx_pptp_gre_seqno_hook)($2)
$L250:
	.set	noreorder
	.set	nomacro
	jal	kfree
	move	$4,$18
	.set	macro
	.set	reorder

	move	$2,$17
$L239:
	lw	$31,36($sp)
	lw	$19,32($sp)
	lw	$18,28($sp)
	lw	$17,24($sp)
	lw	$16,20($sp)
	.set	noreorder
	.set	nomacro
	jr	$31
	addiu	$sp,$sp,40
	.set	macro
	.set	reorder

$L264:
	lui	$3,%hi(sync_tx_pptp_gre_seqno_hook)
	addiu	$2,$2,%lo(sync_tx_pptp_gre_seqno)
	.set	noreorder
	.set	nomacro
	b	$L250
	sw	$2,%lo(sync_tx_pptp_gre_seqno_hook)($3)
	.set	macro
	.set	reorder

$L244:
	move	$5,$0
	move	$6,$17
	.set	noreorder
	.set	nomacro
	jal	memset
	move	$4,$18
	.set	macro
	.set	reorder

$L247:
	.set	noreorder
	.set	nomacro
	jal	kfree
	move	$4,$18
	.set	macro
	.set	reorder

$L262:
	.set	noreorder
	.set	nomacro
	b	$L239
	li	$2,-14			# 0xfffffffffffffff2
	.set	macro
	.set	reorder

$L263:
	.set	noreorder
	.set	nomacro
	jr	$31
	li	$2,-14			# 0xfffffffffffffff2
	.set	macro
	.set	reorder

	.end	ppfw_write_proc
	.size	ppfw_write_proc, .-ppfw_write_proc
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	fastpath_pptp_single_write
	.type	fastpath_pptp_single_write, @function
fastpath_pptp_single_write:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	j	ppfw_write_proc
	nop

	.set	macro
	.set	reorder
	.end	fastpath_pptp_single_write
	.size	fastpath_pptp_single_write, .-fastpath_pptp_single_write
	.local	res_check_pptp
	.comm	res_check_pptp,4,4
	.local	res1
	.comm	res1,4,4
	.globl	pptp_conn_proc_fops
	.data
	.align	2
	.type	pptp_conn_proc_fops, @object
	.size	pptp_conn_proc_fops, 108
pptp_conn_proc_fops:
	.space	4
	.word	seq_lseek
	.word	seq_read
	.word	pptp_conn_single_write
	.space	28
	.word	pptp_conn_single_open
	.space	4
	.word	single_release
	.space	52
	.globl	fastpath_pptp_proc_fops
	.align	2
	.type	fastpath_pptp_proc_fops, @object
	.size	fastpath_pptp_proc_fops, 108
fastpath_pptp_proc_fops:
	.space	4
	.word	seq_lseek
	.word	seq_read
	.word	fastpath_pptp_single_write
	.space	28
	.word	fastpath_pptp_single_open
	.space	4
	.word	single_release
	.space	52
	.globl	fast_pptp_fw
	.section	.bss,"aw",@nobits
	.align	2
	.type	fast_pptp_fw, @object
	.size	fast_pptp_fw, 4
fast_pptp_fw:
	.space	4
	.local	pptp_conn_check
	.comm	pptp_conn_check,4,4
	.globl	pptp_tcp_finished
	.align	2
	.type	pptp_tcp_finished, @object
	.size	pptp_tcp_finished, 4
pptp_tcp_finished:
	.space	4
	.local	pptpAccInfo
	.comm	pptpAccInfo,64,4
	.ident	"GCC: (Realtek MSDK-6.4.1 Build 3055) 6.4.1 20180425"
