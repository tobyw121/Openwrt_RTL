8052a708  c8ffbd27  addiu      $sp, $sp, -0x38
8052a70c  3400bfaf  sw         $ra, 0x34($sp)
8052a710  3000b7af  sw         $s7, 0x30($sp)
8052a714  2c00b6af  sw         $s6, 0x2c($sp)
8052a718  2800b5af  sw         $s5, 0x28($sp)
8052a71c  2400b4af  sw         $s4, 0x24($sp)
8052a720  2000b3af  sw         $s3, 0x20($sp)
8052a724  1c00b2af  sw         $s2, 0x1c($sp)
8052a728  1800b1af  sw         $s1, 0x18($sp)
8052a72c  1400b0af  sw         $s0, 0x14($sp)
8052a730  6880133c  lui        $s3, 0x8068
8052a734  0c00e28c  lw         $v0, 0xc($a3)
8052a738  6880033c  lui        $v1, 0x8068
8052a73c  50387326  addiu      $s3, $s3, 0x3850
8052a740  70386324  addiu      $v1, $v1, 0x3870
8052a744  03004230  andi       $v0, $v0, 3
8052a748  80100200  sll        $v0, $v0, 2
8052a74c  21406202  addu       $t0, $s3, $v0
8052a750  21186200  addu       $v1, $v1, $v0
8052a754  0000178d  lw         $s7, ($t0)
8052a758  0000688c  lw         $t0, ($v1)
8052a75c  6880033c  lui        $v1, 0x8068
8052a760  40386324  addiu      $v1, $v1, 0x3840
8052a764  0100f426  addiu      $s4, $s7, 1
8052a768  21186200  addu       $v1, $v1, $v0
8052a76c  26408802  xor        $t0, $s4, $t0
8052a770  0aa00800  movz       $s4, $zero, $t0
8052a774  0000638c  lw         $v1, ($v1)
8052a778  a5008312  beq        $s4, $v1, 0x8052aa10
8052a77c  25a88000  move       $s5, $a0
8052a780  6880043c  lui        $a0, 0x8068
8052a784  40191700  sll        $v1, $s7, 5
8052a788  60388424  addiu      $a0, $a0, 0x3860
8052a78c  2590e000  move       $s2, $a3
8052a790  21108200  addu       $v0, $a0, $v0
8052a794  2588c000  move       $s1, $a2
8052a798  25b0a000  move       $s6, $a1
8052a79c  0000508c  lw         $s0, ($v0)
8052a7a0  20000624  addiu      $a2, $zero, 0x20
8052a7a4  25280000  move       $a1, $zero
8052a7a8  21800302  addu       $s0, $s0, $v1
8052a7ac  40b8060c  jal        0x801ae100
8052a7b0  25200002  move       $a0, $s0
8052a7b4  6880043c  lui        $a0, 0x8068
8052a7b8  0c00428e  lw         $v0, 0xc($s2)
8052a7bc  80181700  sll        $v1, $s7, 2
8052a7c0  c8388424  addiu      $a0, $a0, 0x38c8
8052a7c4  04002526  addiu      $a1, $s1, 4
8052a7c8  03004230  andi       $v0, $v0, 3
8052a7cc  80100200  sll        $v0, $v0, 2
8052a7d0  40000624  addiu      $a2, $zero, 0x40
8052a7d4  21104400  addu       $v0, $v0, $a0
8052a7d8  3c00312e  sltiu      $s1, $s1, 0x3c
8052a7dc  0a30b100  movz       $a2, $a1, $s1
8052a7e0  0000428c  lw         $v0, ($v0)
8052a7e4  2588c000  move       $s1, $a2
8052a7e8  21184300  addu       $v1, $v0, $v1
8052a7ec  0080317e  ext        $s1, $s1, 0, 0x11
8052a7f0  000075ac  sw         $s5, ($v1)
8052a7f4  0000028e  lw         $v0, ($s0)
8052a7f8  0800048e  lw         $a0, 8($s0)
8052a7fc  0c00058e  lw         $a1, 0xc($s0)
8052a800  0c004234  ori        $v0, $v0, 0xc
8052a804  040016ae  sw         $s6, 4($s0)
8052a808  c4fb247e  ins        $a0, $s1, 0xf, 0x11
8052a80c  1000038e  lw         $v1, 0x10($s0)
8052a810  84b1227e  ins        $v0, $s1, 6, 0x11
8052a814  080004ae  sw         $a0, 8($s0)
8052a818  000002ae  sw         $v0, ($s0)
8052a81c  00004496  lhu        $a0, ($s2)
8052a820  ff0f8430  andi       $a0, $a0, 0xfff
8052a824  0458857c  ins        $a1, $a0, 0, 0xc
8052a828  0c0005ae  sw         $a1, 0xc($s0)
8052a82c  02004596  lhu        $a1, 2($s2)
8052a830  04f6a37c  ins        $v1, $a1, 0x18, 7
8052a834  100003ae  sw         $v1, 0x10($s0)
8052a838  06004396  lhu        $v1, 6($s2)
8052a83c  20006330  andi       $v1, $v1, 0x20
8052a840  0d006010  beqz       $v1, 0x8052a878
8052a844  01000624  addiu      $a2, $zero, 1
8052a848  0421c27c  ins        $v0, $a2, 4, 1
8052a84c  000002ae  sw         $v0, ($s0)
8052a850  06004396  lhu        $v1, 6($s2)
8052a854  40006330  andi       $v1, $v1, 0x40
8052a858  56006010  beqz       $v1, 0x8052a9b4
8052a85c  00000000  nop        
8052a860  4429c27c  ins        $v0, $a2, 5, 1
8052a864  000002ae  sw         $v0, ($s0)
8052a868  1400028e  lw         $v0, 0x14($s0)
8052a86c  03000324  addiu      $v1, $zero, 3
8052a870  84ff627c  ins        $v0, $v1, 0x1e, 2
8052a874  140002ae  sw         $v0, 0x14($s0)
8052a878  0000028e  lw         $v0, ($s0)
8052a87c  81000324  addiu      $v1, $zero, 0x81
8052a880  04e7027c  ins        $v0, $zero, 0x1c, 1
8052a884  000002ae  sw         $v0, ($s0)
8052a888  dc00a58e  lw         $a1, 0xdc($s5)
8052a88c  0c00a594  lhu        $a1, 0xc($a1)
8052a890  3a00a310  beq        $a1, $v1, 0x8052a97c
8052a894  6480033c  lui        $v1, 0x8064
8052a898  cd4a090c  jal        0x80252b34
8052a89c  00000000  nop        
8052a8a0  0800038e  lw         $v1, 8($s0)
8052a8a4  3f004230  andi       $v0, $v0, 0x3f
8052a8a8  0c00048e  lw         $a0, 0xc($s0)
8052a8ac  0440437c  ins        $v1, $v0, 0, 9
8052a8b0  ff0f8430  andi       $a0, $a0, 0xfff
8052a8b4  080003ae  sw         $v1, 8($s0)
8052a8b8  e64a090c  jal        0x80252b98
8052a8bc  00000000  nop        
8052a8c0  0c00038e  lw         $v1, 0xc($s0)
8052a8c4  84c5437c  ins        $v1, $v0, 0x16, 3
8052a8c8  6c80023c  lui        $v0, 0x806c
8052a8cc  0c0003ae  sw         $v1, 0xc($s0)
8052a8d0  1468428c  lw         $v0, 0x6814($v0)
8052a8d4  08004010  beqz       $v0, 0x8052a8f8
8052a8d8  6780023c  lui        $v0, 0x8067
8052a8dc  1000028e  lw         $v0, 0x10($s0)
8052a8e0  007f053c  lui        $a1, 0x7f00
8052a8e4  0004043c  lui        $a0, 0x400
8052a8e8  24104500  and        $v0, $v0, $a1
8052a8ec  3f004410  beq        $v0, $a0, 0x8052a9ec
8052a8f0  00000000  nop        
8052a8f4  6780023c  lui        $v0, 0x8067
8052a8f8  20000524  addiu      $a1, $zero, 0x20
8052a8fc  4826428c  lw         $v0, 0x2648($v0)
8052a900  09f84000  jalr       $v0
8052a904  25200002  move       $a0, $s0
8052a908  0f000000  sync       
8052a90c  0020023c  lui        $v0, 0x2000
8052a910  01000524  addiu      $a1, $zero, 1
8052a914  25800202  or         $s0, $s0, $v0
8052a918  01b8043c  lui        $a0, 0xb801
8052a91c  0000038e  lw         $v1, ($s0)
8052a920  25100000  move       $v0, $zero
8052a924  0400a37c  ins        $v1, $a1, 0, 1
8052a928  8000053c  lui        $a1, 0x80
8052a92c  000003ae  sw         $v1, ($s0)
8052a930  0c00438e  lw         $v1, 0xc($s2)
8052a934  03006330  andi       $v1, $v1, 3
8052a938  80180300  sll        $v1, $v1, 2
8052a93c  21986302  addu       $s3, $s3, $v1
8052a940  000074ae  sw         $s4, ($s3)
8052a944  0000838c  lw         $v1, ($a0)
8052a948  25186500  or         $v1, $v1, $a1
8052a94c  000083ac  sw         $v1, ($a0)
8052a950  3400bf8f  lw         $ra, 0x34($sp)
8052a954  3000b78f  lw         $s7, 0x30($sp)
8052a958  2c00b68f  lw         $s6, 0x2c($sp)
8052a95c  2800b58f  lw         $s5, 0x28($sp)
8052a960  2400b48f  lw         $s4, 0x24($sp)
8052a964  2000b38f  lw         $s3, 0x20($sp)
8052a968  1c00b28f  lw         $s2, 0x1c($sp)
8052a96c  1800b18f  lw         $s1, 0x18($sp)
8052a970  1400b08f  lw         $s0, 0x14($sp)
8052a974  0800e003  jr         $ra
8052a978  3800bd27  addiu      $sp, $sp, 0x38
8052a97c  181d638c  lw         $v1, 0x1d18($v1)
8052a980  05006010  beqz       $v1, 0x8052a998
8052a984  00000000  nop        
8052a988  b800a392  lbu        $v1, 0xb8($s5)
8052a98c  02006330  andi       $v1, $v1, 2
8052a990  0b006014  bnez       $v1, 0x8052a9c0
8052a994  01000324  addiu      $v1, $zero, 1
8052a998  0800038e  lw         $v1, 8($s0)
8052a99c  0440037c  ins        $v1, $zero, 0, 9
8052a9a0  080003ae  sw         $v1, 8($s0)
8052a9a4  01000324  addiu      $v1, $zero, 1
8052a9a8  04e7627c  ins        $v0, $v1, 0x1c, 1
8052a9ac  c2ff0010  b          0x8052a8b8
8052a9b0  000002ae  sw         $v0, ($s0)
8052a9b4  4429027c  ins        $v0, $zero, 5, 1
8052a9b8  abff0010  b          0x8052a868
8052a9bc  000002ae  sw         $v0, ($s0)
8052a9c0  04e7627c  ins        $v0, $v1, 0x1c, 1
8052a9c4  cd4a090c  jal        0x80252b34
8052a9c8  000002ae  sw         $v0, ($s0)
8052a9cc  0800038e  lw         $v1, 8($s0)
8052a9d0  3f004530  andi       $a1, $v0, 0x3f
8052a9d4  0c00048e  lw         $a0, 0xc($s0)
8052a9d8  0000028e  lw         $v0, ($s0)
8052a9dc  0440a37c  ins        $v1, $a1, 0, 9
8052a9e0  ff0f8430  andi       $a0, $a0, 0xfff
8052a9e4  efff0010  b          0x8052a9a4
8052a9e8  080003ae  sw         $v1, 8($s0)
8052a9ec  0800028e  lw         $v0, 8($s0)
8052a9f0  04004230  andi       $v0, $v0, 4
8052a9f4  c0ff4010  beqz       $v0, 0x8052a8f8
8052a9f8  6780023c  lui        $v0, 0x8067
8052a9fc  ac00a28e  lw         $v0, 0xac($s5)
8052aa00  0017427c  ext        $v0, $v0, 0x1c, 3
8052aa04  84c5437c  ins        $v1, $v0, 0x16, 3
8052aa08  baff0010  b          0x8052a8f4
8052aa0c  0c0003ae  sw         $v1, 0xc($s0)
8052aa10  cfff0010  b          0x8052a950
8052aa14  ffff0224  addiu      $v0, $zero, -1
