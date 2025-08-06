  .text
  .globl frame_dummy
  .type frame_dummy, @function

#! file-offset 0x5d0
#! rip-offset  0x4005d0
#! capacity    48 bytes

# Text                      #  Line  RIP       Bytes  Opcode              
.frame_dummy:               #        0x4005d0  0      OPC=<label>         
  movl $0x600e20, %edi      #  1     0x4005d0  5      OPC=movl_r32_imm32  
  cmpq $0x0, (%rdi)         #  2     0x4005d5  4      OPC=cmpq_m64_imm8   
  jne .L_4005e0             #  3     0x4005d9  2      OPC=jne_label       
.L_4005db:                  #        0x4005db  0      OPC=<label>         
  jmpq .register_tm_clones  #  4     0x4005db  2      OPC=jmpq_label      
  nop                       #  5     0x4005dd  1      OPC=nop             
  nop                       #  6     0x4005de  1      OPC=nop             
  nop                       #  7     0x4005df  1      OPC=nop             
.L_4005e0:                  #        0x4005e0  0      OPC=<label>         
  movl $0x0, %eax           #  8     0x4005e0  5      OPC=movl_r32_imm32  
  testq %rax, %rax          #  9     0x4005e5  3      OPC=testq_r64_r64   
  je .L_4005db              #  10    0x4005e8  2      OPC=je_label        
  pushq %rbp                #  11    0x4005ea  1      OPC=pushq_r64_1     
  movq %rsp, %rbp           #  12    0x4005eb  3      OPC=movq_r64_r64    
  callq %rax                #  13    0x4005ee  2      OPC=callq_r64       
  popq %rbp                 #  14    0x4005f0  1      OPC=popq_r64_1      
  jmpq .register_tm_clones  #  15    0x4005f1  5      OPC=jmpq_label_1    
  nop                       #  16    0x4005f6  1      OPC=nop             
  nop                       #  17    0x4005f7  1      OPC=nop             
  nop                       #  18    0x4005f8  1      OPC=nop             
  nop                       #  19    0x4005f9  1      OPC=nop             
  nop                       #  20    0x4005fa  1      OPC=nop             
  nop                       #  21    0x4005fb  1      OPC=nop             
  nop                       #  22    0x4005fc  1      OPC=nop             
  nop                       #  23    0x4005fd  1      OPC=nop             
  nop                       #  24    0x4005fe  1      OPC=nop             
  nop                       #  25    0x4005ff  1      OPC=nop             
                                                                          
.size frame_dummy, .-frame_dummy

