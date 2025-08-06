  .text
  .globl __libc_csu_init
  .type __libc_csu_init, @function

#! file-offset 0x610
#! rip-offset  0x400610
#! capacity    112 bytes

# Text                       #  Line  RIP       Bytes  Opcode             
.__libc_csu_init:            #        0x400610  0      OPC=<label>        
  pushq %r15                 #  1     0x400610  2      OPC=pushq_r64_1    
  movl %edi, %r15d           #  2     0x400612  3      OPC=movl_r32_r32   
  pushq %r14                 #  3     0x400615  2      OPC=pushq_r64_1    
  movq %rsi, %r14            #  4     0x400617  3      OPC=movq_r64_r64   
  pushq %r13                 #  5     0x40061a  2      OPC=pushq_r64_1    
  movq %rdx, %r13            #  6     0x40061c  3      OPC=movq_r64_r64   
  pushq %r12                 #  7     0x40061f  2      OPC=pushq_r64_1    
  leaq 0x2007e8(%rip), %r12  #  8     0x400621  7      OPC=leaq_r64_m16   
  pushq %rbp                 #  9     0x400628  1      OPC=pushq_r64_1    
  leaq 0x2007e8(%rip), %rbp  #  10    0x400629  7      OPC=leaq_r64_m16   
  pushq %rbx                 #  11    0x400630  1      OPC=pushq_r64_1    
  subq %r12, %rbp            #  12    0x400631  3      OPC=subq_r64_r64   
  xorl %ebx, %ebx            #  13    0x400634  2      OPC=xorl_r32_r32   
  sarq $0x3, %rbp            #  14    0x400636  4      OPC=sarq_r64_imm8  
  subq $0x8, %rsp            #  15    0x40063a  4      OPC=subq_r64_imm8  
  callq ._init               #  16    0x40063e  5      OPC=callq_label    
  testq %rbp, %rbp           #  17    0x400643  3      OPC=testq_r64_r64  
  je .L_400666               #  18    0x400646  2      OPC=je_label       
  nop                        #  19    0x400648  1      OPC=nop            
  nop                        #  20    0x400649  1      OPC=nop            
  nop                        #  21    0x40064a  1      OPC=nop            
  nop                        #  22    0x40064b  1      OPC=nop            
  nop                        #  23    0x40064c  1      OPC=nop            
  nop                        #  24    0x40064d  1      OPC=nop            
  nop                        #  25    0x40064e  1      OPC=nop            
  nop                        #  26    0x40064f  1      OPC=nop            
.L_400650:                   #        0x400650  0      OPC=<label>        
  movq %r13, %rdx            #  27    0x400650  3      OPC=movq_r64_r64   
  movq %r14, %rsi            #  28    0x400653  3      OPC=movq_r64_r64   
  movl %r15d, %edi           #  29    0x400656  3      OPC=movl_r32_r32   
  callq (%r12,%rbx,8)        #  30    0x400659  4      OPC=callq_m64      
  addq $0x1, %rbx            #  31    0x40065d  4      OPC=addq_r64_imm8  
  cmpq %rbp, %rbx            #  32    0x400661  3      OPC=cmpq_r64_r64   
  jne .L_400650              #  33    0x400664  2      OPC=jne_label      
.L_400666:                   #        0x400666  0      OPC=<label>        
  addq $0x8, %rsp            #  34    0x400666  4      OPC=addq_r64_imm8  
  popq %rbx                  #  35    0x40066a  1      OPC=popq_r64_1     
  popq %rbp                  #  36    0x40066b  1      OPC=popq_r64_1     
  popq %r12                  #  37    0x40066c  2      OPC=popq_r64_1     
  popq %r13                  #  38    0x40066e  2      OPC=popq_r64_1     
  popq %r14                  #  39    0x400670  2      OPC=popq_r64_1     
  popq %r15                  #  40    0x400672  2      OPC=popq_r64_1     
  retq                       #  41    0x400674  1      OPC=retq           
  nop                        #  42    0x400675  1      OPC=nop            
  nop                        #  43    0x400676  1      OPC=nop            
  nop                        #  44    0x400677  1      OPC=nop            
  nop                        #  45    0x400678  1      OPC=nop            
  nop                        #  46    0x400679  1      OPC=nop            
  nop                        #  47    0x40067a  1      OPC=nop            
  nop                        #  48    0x40067b  1      OPC=nop            
  nop                        #  49    0x40067c  1      OPC=nop            
  nop                        #  50    0x40067d  1      OPC=nop            
  nop                        #  51    0x40067e  1      OPC=nop            
  nop                        #  52    0x40067f  1      OPC=nop            
                                                                          
.size __libc_csu_init, .-__libc_csu_init

