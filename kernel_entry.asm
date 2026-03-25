[bits 32]

; --- DE MULTIBOOT HEADER (Honderd procent nodig voor GRUB) ---
section .multiboot
    align 4
    dd 0x1BADB002              ; Magic number
    dd 0x03                    ; Flags (ALIGNS + MEMINFO)
    dd -(0x1BADB002 + 0x03)    ; Checksum (Magic + Flags + Checksum moet 0 zijn)

section .text
extern main
global kernel_entry
global keyboard_handler_asm
extern keyboard_handler

kernel_entry:
    ; GRUB laat ons achter in een werkende Protected Mode.
    ; We stellen alleen de stack in en gaan direct naar C.
    mov esp, stack_top
    
    ; Push de multiboot magic en info (optioneel, maar netjes)
    push ebx
    push eax

    call main
    jmp $

keyboard_handler_asm:
    pushad
    call keyboard_handler
    popad
    iretd

section .bss
align 16
stack_bottom:
    resb 16384                 ; 16KB stack
stack_top:
