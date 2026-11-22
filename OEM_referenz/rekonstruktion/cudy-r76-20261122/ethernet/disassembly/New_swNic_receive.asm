8052a368  a0ffbd27  addiu      $sp, $sp, -0x60
8052a36c  5c00bfaf  sw         $ra, 0x5c($sp)
8052a370  5800beaf  sw         $fp, 0x58($sp)
8052a374  5400b7af  sw         $s7, 0x54($sp)
8052a378  5000b6af  sw         $s6, 0x50($sp)
8052a37c  4c00b5af  sw         $s5, 0x4c($sp)
8052a380  4800b4af  sw         $s4, 0x48($sp)
8052a384  4400b3af  sw         $s3, 0x44($sp)
8052a388  4000b2af  sw         $s2, 0x40($sp)
8052a38c  3c00b1af  sw         $s1, 0x3c($sp)
8052a390  3800b0af  sw         $s0, 0x38($sp)
8052a394  25908000  move       $s2, $a0
8052a398  00606241  di         $v0
8052a39c  01004230  andi       $v0, $v0, 1
8052a3a0  c0000000  ehb        
8052a3a4  1800a2af  sw         $v0, 0x18($sp)
8052a3a8  6880023c  lui        $v0, 0x8068
8052a3ac  5580153c  lui        $s5, 0x8055
8052a3b0  1c00a2af  sw         $v0, 0x1c($sp)
8052a3b4  6880023c  lui        $v0, 0x8068
8052a3b8  6880163c  lui        $s6, 0x8068
8052a3bc  2000a2af  sw         $v0, 0x20($sp)
8052a3c0  6c80023c  lui        $v0, 0x806c
8052a3c4  6880113c  lui        $s1, 0x8068
8052a3c8  2400a2af  sw         $v0, 0x24($sp)
8052a3cc  40d9a226  addiu      $v0, $s5, -0x26c0
8052a3d0  1838c48e  lw         $a0, 0x3818($s6)
8052a3d4  6880103c  lui        $s0, 0x8068
8052a3d8  2800a2af  sw         $v0, 0x28($sp)
8052a3dc  01b8023c  lui        $v0, 0xb801
8052a3e0  8038378e  lw         $s7, 0x3880($s1)
8052a3e4  6780133c  lui        $s3, 0x8067
8052a3e8  6880143c  lui        $s4, 0x8068
8052a3ec  0400428c  lw         $v0, 4($v0)
8052a3f0  23104400  subu       $v0, $v0, $a0
8052a3f4  42110200  srl        $v0, $v0, 5
8052a3f8  2200e212  beq        $s7, $v0, 0x8052a484
8052a3fc  9838068e  lw         $a2, 0x3898($s0)
8052a400  9838078e  lw         $a3, 0x3898($s0)
8052a404  40111700  sll        $v0, $s7, 5
8052a408  20000524  addiu      $a1, $zero, 0x20
8052a40c  21a8e200  addu       $s5, $a3, $v0
8052a410  4426628e  lw         $v0, 0x2644($s3)
8052a414  25f0a002  move       $fp, $s5
8052a418  09f84000  jalr       $v0
8052a41c  2520c003  move       $a0, $fp
8052a420  00c0043c  lui        $a0, 0xc000
8052a424  1400a28e  lw         $v0, 0x14($s5)
8052a428  24104400  and        $v0, $v0, $a0
8052a42c  28004410  beq        $v0, $a0, 0x8052a4d0
8052a430  1c00a28f  lw         $v0, 0x1c($sp)
8052a434  0020023c  lui        $v0, 0x2000
8052a438  01000424  addiu      $a0, $zero, 1
8052a43c  2530c203  or         $a2, $fp, $v0
8052a440  0100e326  addiu      $v1, $s7, 1
8052a444  0000c28c  lw         $v0, ($a2)
8052a448  0400827c  ins        $v0, $a0, 0, 1
8052a44c  0000c2ac  sw         $v0, ($a2)
8052a450  b038828e  lw         $v0, 0x38b0($s4)
8052a454  1838c48e  lw         $a0, 0x3818($s6)
8052a458  26106200  xor        $v0, $v1, $v0
8052a45c  0a180200  movz       $v1, $zero, $v0
8052a460  01b8023c  lui        $v0, 0xb801
8052a464  803823ae  sw         $v1, 0x3880($s1)
8052a468  25b86000  move       $s7, $v1
8052a46c  0400428c  lw         $v0, 4($v0)
8052a470  23104400  subu       $v0, $v0, $a0
8052a474  42110200  srl        $v0, $v0, 5
8052a478  e2ffe216  bne        $s7, $v0, 0x8052a404
8052a47c  9838078e  lw         $a3, 0x3898($s0)
8052a480  9838068e  lw         $a2, 0x3898($s0)
8052a484  40111700  sll        $v0, $s7, 5
8052a488  2130c200  addu       $a2, $a2, $v0
8052a48c  0020023c  lui        $v0, 0x2000
8052a490  2510c200  or         $v0, $a2, $v0
8052a494  0000428c  lw         $v0, ($v0)
8052a498  01004230  andi       $v0, $v0, 1
8052a49c  74004014  bnez       $v0, 0x8052a670
8052a4a0  4426628e  lw         $v0, 0x2644($s3)
8052a4a4  25f0c000  move       $fp, $a2
8052a4a8  25a8c000  move       $s5, $a2
8052a4ac  20000524  addiu      $a1, $zero, 0x20
8052a4b0  09f84000  jalr       $v0
8052a4b4  2520c003  move       $a0, $fp
8052a4b8  00c0043c  lui        $a0, 0xc000
8052a4bc  1400a28e  lw         $v0, 0x14($s5)
8052a4c0  24104400  and        $v0, $v0, $a0
8052a4c4  dcff4414  bne        $v0, $a0, 0x8052a438
8052a4c8  0020023c  lui        $v0, 0x2000
8052a4cc  1c00a28f  lw         $v0, 0x1c($sp)
8052a4d0  80581700  sll        $t3, $s7, 2
8052a4d4  d838428c  lw         $v0, 0x38d8($v0)
8052a4d8  21104b00  addu       $v0, $v0, $t3
8052a4dc  00004a8c  lw         $t2, ($v0)
8052a4e0  d4ff4011  beqz       $t2, 0x8052a434
8052a4e4  2000a28f  lw         $v0, 0x20($sp)
8052a4e8  1000a427  addiu      $a0, $sp, 0x10
8052a4ec  3000aaaf  sw         $t2, 0x30($sp)
8052a4f0  2c00abaf  sw         $t3, 0x2c($sp)
8052a4f4  f7a1140c  jal        0x805287dc
8052a4f8  bc11458c  lw         $a1, 0x11bc($v0)
8052a4fc  74004010  beqz       $v0, 0x8052a6d0
8052a500  3000aa8f  lw         $t2, 0x30($sp)
8052a504  1000a48e  lw         $a0, 0x10($s5)
8052a508  08004c92  lbu        $t4, 8($s2)
8052a50c  005c847c  ext        $a0, $a0, 0x10, 0xc
8052a510  2400a38f  lw         $v1, 0x24($sp)
8052a514  2c00ab8f  lw         $t3, 0x2c($sp)
8052a518  000044a6  sh         $a0, ($s2)
8052a51c  1000a48e  lw         $a0, 0x10($s5)
8052a520  4013847c  ext        $a0, $a0, 0xd, 3
8052a524  020044a6  sh         $a0, 2($s2)
8052a528  0800a48e  lw         $a0, 8($s5)
8052a52c  0468658c  lw         $a1, 0x6804($v1)
8052a530  000e847c  ext        $a0, $a0, 0x18, 2
8052a534  040044a6  sh         $a0, 4($s2)
8052a538  1400a48e  lw         $a0, 0x14($s5)
8052a53c  0c004aae  sw         $t2, 0xc($s2)
8052a540  c014847c  ext        $a0, $a0, 0x13, 3
8052a544  c4288c7c  ins        $t4, $a0, 3, 3
8052a548  08004ca2  sb         $t4, 8($s2)
8052a54c  0800a48e  lw         $a0, 8($s5)
8052a550  0068847c  ext        $a0, $a0, 0, 0xe
8052a554  fcff8424  addiu      $a0, $a0, -4
8052a558  060044a6  sh         $a0, 6($s2)
8052a55c  1000ac8e  lw         $t4, 0x10($s5)
8052a560  4013847d  ext        $a0, $t4, 0xd, 3
8052a564  1300a010  beqz       $a1, 0x8052a5b4
8052a568  005c8d7d  ext        $t5, $t4, 0x10, 0xc
8052a56c  07000524  addiu      $a1, $zero, 7
8052a570  51008510  beq        $a0, $a1, 0x8052a6b8
8052a574  00000000  nop        
8052a578  0800a48e  lw         $a0, 8($s5)
8052a57c  001d857c  ext        $a1, $a0, 0x14, 4
8052a580  0800ae30  andi       $t6, $a1, 8
8052a584  0b00c015  bnez       $t6, 0x8052a5b4
8052a588  f0000e3c  lui        $t6, 0xf0
8052a58c  24208e00  and        $a0, $a0, $t6
8052a590  a8ff8010  beqz       $a0, 0x8052a434
8052a594  2800a38f  lw         $v1, 0x28($sp)
8052a598  05000424  addiu      $a0, $zero, 5
8052a59c  2128a300  addu       $a1, $a1, $v1
8052a5a0  0000a590  lbu        $a1, ($a1)
8052a5a4  a3ffa410  beq        $a1, $a0, 0x8052a434
8052a5a8  00078c7d  ext        $t4, $t4, 0x1c, 1
8052a5ac  30a50434  ori        $a0, $zero, 0xa530
8052a5b0  0b688c00  movn       $t5, $a0, $t4
8052a5b4  00060524  addiu      $a1, $zero, 0x600
8052a5b8  1c00a38f  lw         $v1, 0x1c($sp)
8052a5bc  00004da6  sh         $t5, ($s2)
8052a5c0  0020123c  lui        $s2, 0x2000
8052a5c4  0400a68e  lw         $a2, 4($s5)
8052a5c8  2590d203  or         $s2, $fp, $s2
8052a5cc  d838648c  lw         $a0, 0x38d8($v1)
8052a5d0  dc0046ad  sw         $a2, 0xdc($t2)
8052a5d4  21588b00  addu       $t3, $a0, $t3
8052a5d8  1000a48f  lw         $a0, 0x10($sp)
8052a5dc  000064ad  sw         $a0, ($t3)
8052a5e0  040042ae  sw         $v0, 4($s2)
8052a5e4  1000a48f  lw         $a0, 0x10($sp)
8052a5e8  4426628e  lw         $v0, 0x2644($s3)
8052a5ec  09f84000  jalr       $v0
8052a5f0  dc00848c  lw         $a0, 0xdc($a0)
8052a5f4  01000424  addiu      $a0, $zero, 1
8052a5f8  0000428e  lw         $v0, ($s2)
8052a5fc  0100e326  addiu      $v1, $s7, 1
8052a600  0400827c  ins        $v0, $a0, 0, 1
8052a604  000042ae  sw         $v0, ($s2)
8052a608  6880023c  lui        $v0, 0x8068
8052a60c  1800b08f  lw         $s0, 0x18($sp)
8052a610  b038428c  lw         $v0, 0x38b0($v0)
8052a614  26106200  xor        $v0, $v1, $v0
8052a618  0a180200  movz       $v1, $zero, $v0
8052a61c  01b8023c  lui        $v0, 0xb801
8052a620  803823ae  sw         $v1, 0x3880($s1)
8052a624  7e00033c  lui        $v1, 0x7e
8052a628  2c0043ac  sw         $v1, 0x2c($v0)
8052a62c  02000012  beqz       $s0, 0x8052a638
8052a630  00606041  di         
8052a634  20606041  ei         
8052a638  c0000000  ehb        
8052a63c  5c00bf8f  lw         $ra, 0x5c($sp)
8052a640  25100000  move       $v0, $zero
8052a644  5800be8f  lw         $fp, 0x58($sp)
8052a648  5400b78f  lw         $s7, 0x54($sp)
8052a64c  5000b68f  lw         $s6, 0x50($sp)
8052a650  4c00b58f  lw         $s5, 0x4c($sp)
8052a654  4800b48f  lw         $s4, 0x48($sp)
8052a658  4400b38f  lw         $s3, 0x44($sp)
8052a65c  4000b28f  lw         $s2, 0x40($sp)
8052a660  3c00b18f  lw         $s1, 0x3c($sp)
8052a664  3800b08f  lw         $s0, 0x38($sp)
8052a668  0800e003  jr         $ra
8052a66c  6000bd27  addiu      $sp, $sp, 0x60
8052a670  1800b08f  lw         $s0, 0x18($sp)
8052a674  02000012  beqz       $s0, 0x8052a680
8052a678  00606041  di         
8052a67c  20606041  ei         
8052a680  c0000000  ehb        
8052a684  ffff0224  addiu      $v0, $zero, -1
8052a688  5c00bf8f  lw         $ra, 0x5c($sp)
8052a68c  5800be8f  lw         $fp, 0x58($sp)
8052a690  5400b78f  lw         $s7, 0x54($sp)
8052a694  5000b68f  lw         $s6, 0x50($sp)
8052a698  4c00b58f  lw         $s5, 0x4c($sp)
8052a69c  4800b48f  lw         $s4, 0x48($sp)
8052a6a0  4400b38f  lw         $s3, 0x44($sp)
8052a6a4  4000b28f  lw         $s2, 0x40($sp)
8052a6a8  3c00b18f  lw         $s1, 0x3c($sp)
8052a6ac  3800b08f  lw         $s0, 0x38($sp)
8052a6b0  0800e003  jr         $ra
8052a6b4  6000bd27  addiu      $sp, $sp, 0x60
8052a6b8  0800a48e  lw         $a0, 8($s5)
8052a6bc  c005847c  ext        $a0, $a0, 0x17, 1
8052a6c0  5cff8010  beqz       $a0, 0x8052a434
8052a6c4  33a50d34  ori        $t5, $zero, 0xa533
8052a6c8  bbff0010  b          0x8052a5b8
8052a6cc  00060524  addiu      $a1, $zero, 0x600
8052a6d0  6880033c  lui        $v1, 0x8068
8052a6d4  1800b08f  lw         $s0, 0x18($sp)
8052a6d8  b811628c  lw         $v0, 0x11b8($v1)
8052a6dc  01004224  addiu      $v0, $v0, 1
8052a6e0  b81162ac  sw         $v0, 0x11b8($v1)
8052a6e4  01b8023c  lui        $v0, 0xb801
8052a6e8  7e00033c  lui        $v1, 0x7e
8052a6ec  2c0043ac  sw         $v1, 0x2c($v0)
8052a6f0  02000012  beqz       $s0, 0x8052a6fc
8052a6f4  00606041  di         
8052a6f8  20606041  ei         
8052a6fc  c0000000  ehb        
8052a700  e1ff0010  b          0x8052a688
8052a704  ffff0224  addiu      $v0, $zero, -1
