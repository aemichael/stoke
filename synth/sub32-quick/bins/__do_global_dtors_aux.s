  .text
  .globl __do_global_dtors_aux
  .type __do_global_dtors_aux, @function

#! file-offset 0x5b0
#! rip-offset  0x4005b0
#! capacity    32 bytes

# Text                         #  Line  RIP       Bytes  Opcode            
.__do_global_dtors_aux:        #        0x4005b0  0      OPC=<label>       
  cmpb $0x0, 0x200a91(%rip)    #  1     0x4005b0  7      OPC=cmpb_m8_imm8  
  jne .L_4005ca                #  2     0x4005b7  2      OPC=jne_label     
  pushq %rbp                   #  3     0x4005b9  1      OPC=pushq_r64_1   
  movq %rsp, %rbp              #  4     0x4005ba  3      OPC=movq_r64_r64  
  callq .deregister_tm_clones  #  5     0x4005bd  5      OPC=callq_label   
  popq %rbp                    #  6     0x4005c2  1      OPC=popq_r64_1    
  movb $0x1, 0x200a7e(%rip)    #  7     0x4005c3  7      OPC=movb_m8_imm8  
.L_4005ca:                     #        0x4005ca  0      OPC=<label>       
  nop                          #  8     0x4005ca  1      OPC=nop           
  retq                         #  9     0x4005cb  1      OPC=retq          
  nop                          #  10    0x4005cc  1      OPC=nop           
  nop                          #  11    0x4005cd  1      OPC=nop           
  nop                          #  12    0x4005ce  1      OPC=nop           
  nop                          #  13    0x4005cf  1      OPC=nop           
                                                                           
.size __do_global_dtors_aux, .-__do_global_dtors_aux

