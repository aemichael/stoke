  .text
  .globl _Z9transformjj
  .type _Z9transformjj, @function

#! file-offset 0x11e0
#! rip-offset  0x4011e0
#! capacity    16 bytes

# Text             #  Line  RIP       Bytes  Opcode            
._Z9transformjj:   
  movl %edi, %eax  
  subq $0x80000000, %rax
  subq $0x80000000, %rax
  movl %esi, %esi
  subq $0x80000000, %rsi
  andq %rsi, %rax
  movl %eax, %eax 
  retq             
  nop              
  nop              
  nop              
  nop              
  nop              
  nop              
  nop              
  nop              
  nop              
  nop              
  nop              
                                                               
.size _Z9transformjj, .-_Z9transformjj