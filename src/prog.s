    .globl _start
    .section .text
_start:
    addi x1, zero, 0x1  # Set x1 to 1
    addi x2, zero, 0x2  # Set x2 to 2
    add x3, x1, x2      # Add x1 and x2

    # Load a byte from memory
    la x4, bytes        # Load address of bytes
    lb x5, 0(x4)        # Load byte at offset 0 into

    j _hang

_hang:
    j _hang

    .section .data
bytes:
    .byte 0x00, 0x01, 0x02
words:
    .word 0x12345678, 0x9ABCDEF0

    .section .bss
buffer:
    .space 64  # Reserve 64 bytes of uninitialized space
