  .text
  .globl register_tm_clones
  .type register_tm_clones, @function

#! file-offset 0x570
#! rip-offset  0x400570
#! capacity    64 bytes

# Text                  #  Line  RIP       Bytes  Opcode              
.register_tm_clones:    #        0x400570  0      OPC=<label>         
  movl $0x601048, %esi  #  1     0x400570  5      OPC=movl_r32_imm32  
  pushq %rbp            #  2     0x400575  1      OPC=pushq_r64_1     
  subq $0x601048, %rsi  #  3     0x400576  7      OPC=subq_r64_imm32  
  sarq $0x3, %rsi       #  4     0x40057d  4      OPC=sarq_r64_imm8   
  movq %rsp, %rbp       #  5     0x400581  3      OPC=movq_r64_r64    
  movq %rsi, %rax       #  6     0x400584  3      OPC=movq_r64_r64    
  shrq $0x3f, %rax      #  7     0x400587  4      OPC=shrq_r64_imm8   
  addq %rax, %rsi       #  8     0x40058b  3      OPC=addq_r64_r64    
  sarq $0x1, %rsi       #  9     0x40058e  3      OPC=sarq_r64_one    
  je .L_4005a8          #  10    0x400591  2      OPC=je_label        
  movl $0x0, %eax       #  11    0x400593  5      OPC=movl_r32_imm32  
  testq %rax, %rax      #  12    0x400598  3      OPC=testq_r64_r64   
  je .L_4005a8          #  13    0x40059b  2      OPC=je_label        
  popq %rbp             #  14    0x40059d  1      OPC=popq_r64_1      
  movl $0x601048, %edi  #  15    0x40059e  5      OPC=movl_r32_imm32  
  jmpq %rax             #  16    0x4005a3  2      OPC=jmpq_r64        
  nop                   #  17    0x4005a5  1      OPC=nop             
  nop                   #  18    0x4005a6  1      OPC=nop             
  nop                   #  19    0x4005a7  1      OPC=nop             
.L_4005a8:              #        0x4005a8  0      OPC=<label>         
  popq %rbp             #  20    0x4005a8  1      OPC=popq_r64_1      
  retq                  #  21    0x4005a9  1      OPC=retq            
  nop                   #  22    0x4005aa  1      OPC=nop             
  nop                   #  23    0x4005ab  1      OPC=nop             
  nop                   #  24    0x4005ac  1      OPC=nop             
  nop                   #  25    0x4005ad  1      OPC=nop             
  nop                   #  26    0x4005ae  1      OPC=nop             
  nop                   #  27    0x4005af  1      OPC=nop             
                                                                      
.size register_tm_clones, .-register_tm_clones

