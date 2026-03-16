[bits 32]
extern main
global kernel_entry
global keyboard_handler_asm
extern keyboard_handler

kernel_entry:
    mov esp, 0x90000
    call main

    jmp $

keyboard_handler_asm:
    pusha               ; save all registers
    call keyboard_handler ; call our C function
    popa                ; restore all registers
    iret                ; interrupt return — NOT ret!
