    .globl _start
    .section .text
_start:
    addi x1, zero, 0x1  # Set x1 to 1
    addi x2, zero, 0x2  # Set x2 to 2
    add x3, x1, x2      # Add x1 and x2

    j _hang

_hang:
    j _hang

    .section .data
data:
    .byte 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07

