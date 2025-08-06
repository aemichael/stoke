  .text
  .globl deregister_tm_clones
  .type deregister_tm_clones, @function

#! file-offset 0x530
#! rip-offset  0x400530
#! capacity    64 bytes

# Text                  #  Line  RIP       Bytes  Opcode              
.deregister_tm_clones:  #        0x400530  0      OPC=<label>         
  movl $0x60104f, %eax  #  1     0x400530  5      OPC=movl_r32_imm32  
  pushq %rbp            #  2     0x400535  1      OPC=pushq_r64_1     
  subq $0x601048, %rax  #  3     0x400536  6      OPC=subq_rax_imm32  
  cmpq $0xe, %rax       #  4     0x40053c  4      OPC=cmpq_r64_imm8   
  movq %rsp, %rbp       #  5     0x400540  3      OPC=movq_r64_r64    
  jbe .L_400560         #  6     0x400543  2      OPC=jbe_label       
  movl $0x0, %eax       #  7     0x400545  5      OPC=movl_r32_imm32  
  testq %rax, %rax      #  8     0x40054a  3      OPC=testq_r64_r64   
  je .L_400560          #  9     0x40054d  2      OPC=je_label        
  popq %rbp             #  10    0x40054f  1      OPC=popq_r64_1      
  movl $0x601048, %edi  #  11    0x400550  5      OPC=movl_r32_imm32  
  jmpq %rax             #  12    0x400555  2      OPC=jmpq_r64        
  nop                   #  13    0x400557  1      OPC=nop             
  nop                   #  14    0x400558  1      OPC=nop             
  nop                   #  15    0x400559  1      OPC=nop             
  nop                   #  16    0x40055a  1      OPC=nop             
  nop                   #  17    0x40055b  1      OPC=nop             
  nop                   #  18    0x40055c  1      OPC=nop             
  nop                   #  19    0x40055d  1      OPC=nop             
  nop                   #  20    0x40055e  1      OPC=nop             
  nop                   #  21    0x40055f  1      OPC=nop             
.L_400560:              #        0x400560  0      OPC=<label>         
  popq %rbp             #  22    0x400560  1      OPC=popq_r64_1      
  retq                  #  23    0x400561  1      OPC=retq            
  nop                   #  24    0x400562  1      OPC=nop             
  nop                   #  25    0x400563  1      OPC=nop             
  nop                   #  26    0x400564  1      OPC=nop             
  nop                   #  27    0x400565  1      OPC=nop             
  nop                   #  28    0x400566  1      OPC=nop             
  nop                   #  29    0x400567  1      OPC=nop             
  nop                   #  30    0x400568  1      OPC=nop             
  nop                   #  31    0x400569  1      OPC=nop             
  nop                   #  32    0x40056a  1      OPC=nop             
  nop                   #  33    0x40056b  1      OPC=nop             
  nop                   #  34    0x40056c  1      OPC=nop             
  nop                   #  35    0x40056d  1      OPC=nop             
  nop                   #  36    0x40056e  1      OPC=nop             
  nop                   #  37    0x40056f  1      OPC=nop             
                                                                      
.size deregister_tm_clones, .-deregister_tm_clones

