  .text
  .globl main
  .type main, @function

#! file-offset 0x490
#! rip-offset  0x400490
#! capacity    115 bytes

# Text                          #  Line  RIP       Bytes  Opcode              
.main:                          #        0x400490  0      OPC=<label>         
  pushq %r14                    #  1     0x400490  2      OPC=pushq_r64_1     
  pushq %r13                    #  2     0x400492  2      OPC=pushq_r64_1     
  pushq %r12                    #  3     0x400494  2      OPC=pushq_r64_1     
  pushq %rbp                    #  4     0x400496  1      OPC=pushq_r64_1     
  pushq %rbx                    #  5     0x400497  1      OPC=pushq_r64_1     
  movq 0x8(%rsi), %rdi          #  6     0x400498  4      OPC=movq_r64_m64    
  callq .atoi_plt               #  7     0x40049c  5      OPC=callq_label     
  testl %eax, %eax              #  8     0x4004a1  2      OPC=testl_r32_r32   
  movl %eax, %r12d              #  9     0x4004a3  3      OPC=movl_r32_r32    
  jle .L_4004ff                 #  10    0x4004a6  2      OPC=jle_label       
  xorl %ebx, %ebx               #  11    0x4004a8  2      OPC=xorl_r32_r32    
  movl $0x1, %r14d              #  12    0x4004aa  6      OPC=movl_r32_imm32  
  xorl %ebp, %ebp               #  13    0x4004b0  2      OPC=xorl_r32_r32    
  movl $0x66666667, %r13d       #  14    0x4004b2  6      OPC=movl_r32_imm32  
  nop                           #  15    0x4004b8  1      OPC=nop             
  nop                           #  16    0x4004b9  1      OPC=nop             
  nop                           #  17    0x4004ba  1      OPC=nop             
  nop                           #  18    0x4004bb  1      OPC=nop             
  nop                           #  19    0x4004bc  1      OPC=nop             
  nop                           #  20    0x4004bd  1      OPC=nop             
  nop                           #  21    0x4004be  1      OPC=nop             
  nop                           #  22    0x4004bf  1      OPC=nop             
.L_4004c0:                      #        0x4004c0  0      OPC=<label>         
  callq .rand_plt               #  23    0x4004c0  5      OPC=callq_label     
  movl %eax, %ecx               #  24    0x4004c5  2      OPC=movl_r32_r32    
  leal 0x7(%r14,%r14,2), %r14d  #  25    0x4004c7  5      OPC=leal_r32_m16    
  movl %ebx, %esi               #  26    0x4004cc  2      OPC=movl_r32_r32    
  imull %r13d                   #  27    0x4004ce  3      OPC=imull_r32       
  movl %ecx, %eax               #  28    0x4004d1  2      OPC=movl_r32_r32    
  addl $0x1, %ebx               #  29    0x4004d3  3      OPC=addl_r32_imm8   
  sarl $0x1f, %eax              #  30    0x4004d6  3      OPC=sarl_r32_imm8   
  sarl $0x1, %edx               #  31    0x4004d9  2      OPC=sarl_r32_one    
  subl %eax, %edx               #  32    0x4004db  2      OPC=subl_r32_r32    
  leal (%rdx,%rdx,4), %eax      #  33    0x4004dd  3      OPC=leal_r32_m16    
  subl %eax, %ecx               #  34    0x4004e0  2      OPC=subl_r32_r32    
  addl %ecx, %r14d              #  35    0x4004e2  3      OPC=addl_r32_r32    
  movl %r14d, %edi              #  36    0x4004e5  3      OPC=movl_r32_r32    
  callq ._Z11transform32jj      #  37    0x4004e8  5      OPC=callq_label     
  addl %eax, %ebp               #  38    0x4004ed  2      OPC=addl_r32_r32    
  cmpl %r12d, %ebx              #  39    0x4004ef  3      OPC=cmpl_r32_r32    
  jne .L_4004c0                 #  40    0x4004f2  2      OPC=jne_label       
.L_4004f4:                      #        0x4004f4  0      OPC=<label>         
  popq %rbx                     #  41    0x4004f4  1      OPC=popq_r64_1      
  movl %ebp, %eax               #  42    0x4004f5  2      OPC=movl_r32_r32    
  popq %rbp                     #  43    0x4004f7  1      OPC=popq_r64_1      
  popq %r12                     #  44    0x4004f8  2      OPC=popq_r64_1      
  popq %r13                     #  45    0x4004fa  2      OPC=popq_r64_1      
  popq %r14                     #  46    0x4004fc  2      OPC=popq_r64_1      
  retq                          #  47    0x4004fe  1      OPC=retq            
.L_4004ff:                      #        0x4004ff  0      OPC=<label>         
  xorl %ebp, %ebp               #  48    0x4004ff  2      OPC=xorl_r32_r32    
  jmpq .L_4004f4                #  49    0x400501  2      OPC=jmpq_label      
                                                                              
.size main, .-main

