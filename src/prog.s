    .globl _start
    .section .text
    
_start:
    addi x1, zero, 1
    j _hang
_hang: