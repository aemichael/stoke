  .text
  .globl _Z11transform32jj
  .type _Z11transform32jj, @function

#! file-offset 0x11e0
#! rip-offset  0x4011e0
#! capacity    16 bytes

# Text               #  Line  RIP       Bytes  Opcode              
._Z11transform32jj:  #        0x4011e0  0      OPC=<label>         
  movslq %edi, %rax  #  1     0x4011e0  3      OPC=movslq_r64_r32  
  cmpl %edi, %eax    #  2     0x4011e3  2      OPC=cmpl_r32_r32_1  
  sbbl %esi, %eax    #  3     0x4011e5  2      OPC=sbbl_r32_r32_1  
  retq               #  4     0x4011e7  1      OPC=retq            
                                                                   
.size _Z11transform32jj, .-_Z11transform32jj
