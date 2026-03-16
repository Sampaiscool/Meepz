[bits 32]
extern main
global kernel_entry
global keyboard_handler_asm
extern keyboard_handler

kernel_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, 0x90000
    call main

    jmp $

keyboard_handler_asm:
    pushad               ; save all registers
    call keyboard_handler ; call our C function
    popad                ; restore all registers
    iretd                ; interrupt return — NOT ret!
