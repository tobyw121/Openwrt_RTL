8052aa58  d8ffbd27  addiu      $sp, $sp, -0x28
8052aa5c  2400bfaf  sw         $ra, 0x24($sp)
8052aa60  2000b4af  sw         $s4, 0x20($sp)
8052aa64  1c00b3af  sw         $s3, 0x1c($sp)
8052aa68  1800b2af  sw         $s2, 0x18($sp)
8052aa6c  1400b1af  sw         $s1, 0x14($sp)
8052aa70  1000b0af  sw         $s0, 0x10($sp)
8052aa74  00606541  di         $a1
8052aa78  0100a530  andi       $a1, $a1, 1
8052aa7c  c0000000  ehb        
8052aa80  5580023c  lui        $v0, 0x8055
8052aa84  80900400  sll        $s2, $a0, 2
8052aa88  30d94224  addiu      $v0, $v0, -0x26d0
8052aa8c  6880143c  lui        $s4, 0x8068
8052aa90  21105200  addu       $v0, $v0, $s2
8052aa94  40389426  addiu      $s4, $s4, 0x3840
8052aa98  6880133c  lui        $s3, 0x8068
8052aa9c  0000448c  lw         $a0, ($v0)
8052aaa0  6880023c  lui        $v0, 0x8068
8052aaa4  c8387326  addiu      $s3, $s3, 0x38c8
8052aaa8  30384224  addiu      $v0, $v0, 0x3830
8052aaac  6880113c  lui        $s1, 0x8068
8052aab0  21105200  addu       $v0, $v0, $s2
8052aab4  0000908c  lw         $s0, ($a0)
8052aab8  21987202  addu       $s3, $s3, $s2
8052aabc  70383126  addiu      $s1, $s1, 0x3870
8052aac0  0000438c  lw         $v1, ($v0)
8052aac4  21109202  addu       $v0, $s4, $s2
8052aac8  23800302  subu       $s0, $s0, $v1
8052aacc  0000428c  lw         $v0, ($v0)
8052aad0  42811000  srl        $s0, $s0, 5
8052aad4  21000212  beq        $s0, $v0, 0x8052ab5c
8052aad8  2530a000  move       $a2, $a1
8052aadc  0000638e  lw         $v1, ($s3)
8052aae0  80380200  sll        $a3, $v0, 2
8052aae4  21186700  addu       $v1, $v1, $a3
8052aae8  0000648c  lw         $a0, ($v1)
8052aaec  12008010  beqz       $a0, 0x8052ab38
8052aaf0  21183202  addu       $v1, $s1, $s2
8052aaf4  0200c010  beqz       $a2, 0x8052ab00
8052aaf8  00606041  di         
8052aafc  20606041  ei         
8052ab00  c0000000  ehb        
8052ab04  afa20c0c  jal        0x80328abc
8052ab08  01000524  addiu      $a1, $zero, 1
8052ab0c  00606541  di         $a1
8052ab10  0100a530  andi       $a1, $a1, 1
8052ab14  c0000000  ehb        
8052ab18  21309202  addu       $a2, $s4, $s2
8052ab1c  0000628e  lw         $v0, ($s3)
8052ab20  0000c38c  lw         $v1, ($a2)
8052ab24  80180300  sll        $v1, $v1, 2
8052ab28  21104300  addu       $v0, $v0, $v1
8052ab2c  000040ac  sw         $zero, ($v0)
8052ab30  0000c28c  lw         $v0, ($a2)
8052ab34  21183202  addu       $v1, $s1, $s2
8052ab38  21309202  addu       $a2, $s4, $s2
8052ab3c  01004224  addiu      $v0, $v0, 1
8052ab40  0000638c  lw         $v1, ($v1)
8052ab44  e3ff4314  bne        $v0, $v1, 0x8052aad4
8052ab48  0000c2ac  sw         $v0, ($a2)
8052ab4c  25100000  move       $v0, $zero
8052ab50  0000c0ac  sw         $zero, ($a2)
8052ab54  e1ff0216  bne        $s0, $v0, 0x8052aadc
8052ab58  2530a000  move       $a2, $a1
8052ab5c  0200a010  beqz       $a1, 0x8052ab68
8052ab60  00606041  di         
8052ab64  20606041  ei         
8052ab68  c0000000  ehb        
8052ab6c  2400bf8f  lw         $ra, 0x24($sp)
8052ab70  25100000  move       $v0, $zero
8052ab74  2000b48f  lw         $s4, 0x20($sp)
8052ab78  1c00b38f  lw         $s3, 0x1c($sp)
8052ab7c  1800b28f  lw         $s2, 0x18($sp)
8052ab80  1400b18f  lw         $s1, 0x14($sp)
8052ab84  1000b08f  lw         $s0, 0x10($sp)
8052ab88  0800e003  jr         $ra
8052ab8c  2800bd27  addiu      $sp, $sp, 0x28
