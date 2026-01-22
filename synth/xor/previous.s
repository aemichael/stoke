  .text
  .globl _start
  .type _start, @function

# Auto-generated x86_64 assembly file
# Generated from synthesized instruction sequence

._start:
  movq $0x8000000000000000, %rax
  movw %si, %ax
  movq %rdi, %rbx
  xorq %rax, %rdi
  movw %di, %bx
  movq %rbx, %rax
  retq

.size _start, .-_start
