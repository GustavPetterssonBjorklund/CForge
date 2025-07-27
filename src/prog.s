    .section .data
    .byte 0x01, 0x02, 0x03, 0x04, 0x05
    .word 0x12345678

    .globl _start
    .section .text
_start:
    addi x1, zero, 1
    j _hang
_hang:
    j _hang  # Infinite loop to hang the program