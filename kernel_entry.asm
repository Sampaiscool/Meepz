[bits 32]

; --- DE MULTIBOOT HEADER (Honderd procent nodig voor GRUB) ---
section .multiboot
    align 4
    dd 0x1BADB002               ; Magic
    dd 0x00010007               ; Flags: ALIGN (bit 0) + MEMINFO (bit 1) + VIDEO (bit 2) + ADDR (bit 16)
                                ; We gebruiken 0x10007 (of 0x00010003 als je ADDR niet wilt)
    dd -(0x1BADB002 + 0x00010007) ; Checksum moet weer kloppen!

    ; Als bit 16 in flags aan staat (0x10000), verwacht GRUB deze 5 velden:
    ; Omdat we in linker.ld hebben gezegd dat we op 1MB starten (. = 1M):
    extern _code_start, _bss_end
    dd 0x00100000               ; header_addr (fysiek adres van de multiboot header)
    dd 0x00100000               ; load_addr (start van de .text sectie)
    dd 0                        ; load_end_addr (0 = laad het hele bestand)
    dd 0                        ; bss_end_addr (0 = geen bss-check of vul later in)
    dd kernel_entry             ; entry_addr (waar moet GRUB naartoe springen)

    ; Video requests
    dd 0                        ; Mode type
    dd 800                      ; Width
    dd 600                      ; Height
    dd 32                       ; BPP

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
