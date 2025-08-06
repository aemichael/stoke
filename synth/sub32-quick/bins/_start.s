  .text
  .globl _start
  .type _start, @function

#! file-offset 0x503
#! rip-offset  0x400503
#! capacity    45 bytes

# Text                          #  Line  RIP       Bytes  Opcode              
._start:                        #        0x400503  0      OPC=<label>         
  xorl %ebp, %ebp               #  1     0x400503  2      OPC=xorl_r32_r32    
  movq %rdx, %r9                #  2     0x400505  3      OPC=movq_r64_r64    
  popq %rsi                     #  3     0x400508  1      OPC=popq_r64_1      
  movq %rsp, %rdx               #  4     0x400509  3      OPC=movq_r64_r64    
  andq $0xfffffff0, %rsp        #  5     0x40050c  4      OPC=andq_r64_imm8   
  pushq %rax                    #  6     0x400510  1      OPC=pushq_r64_1     
  pushq %rsp                    #  7     0x400511  1      OPC=pushq_r64_1     
  movq $0x400680, %r8           #  8     0x400512  7      OPC=movq_r64_imm32  
  movq $0x400610, %rcx          #  9     0x400519  7      OPC=movq_r64_imm32  
  movq $0x400490, %rdi          #  10    0x400520  7      OPC=movq_r64_imm32  
  callq .__libc_start_main_plt  #  11    0x400527  5      OPC=callq_label     
  retq                          #  12    0x40052c  1      OPC=retq            
  nop                           #  13    0x40052d  1      OPC=nop             
  nop                           #  14    0x40052e  1      OPC=nop             
  nop                           #  15    0x40052f  1      OPC=nop             
                                                                              
.size _start, .-_start

