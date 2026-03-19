[bits 16]
[org 0x7C00]

; ============================================
; STAGE 1 — 16-bit real mode
; This is where BIOS drops us, at address 0x7C00
; CPU thinks its a 1980s 8086 right now
; ============================================

    ; print our message using BIOS so we know we booted
    mov si, message
.loop:
    lodsb               ; load byte at [si] into al, increment si
    cmp al, 0           ; check for null terminator
    je .done            ; if zero, string is finished
    mov ah, 0x0E        ; BIOS teletype function
    int 0x10            ; call BIOS to print char in al
    jmp .loop
.done:

    ; print a hex value to prove our debug tool works
    mov ax, 0x1F3A
    call printhex

    ; read kernel from disk into memory at 0x1000
    ; must happen before cli — int 0x13 is a BIOS interrupt, 16-bit only
    mov ah, 0x02        ; BIOS function: read sectors
    mov al, 50          ; load 50 sectors to be safe (25KB)
    mov ch, 0           ; cylinder 0
    mov cl, 2           ; start at sector 2 (sector 1 is our bootloader)
    mov dh, 0           ; head 0
    mov bx, 0x1000      ; load into memory address 0x1000
    int 0x13            ; call BIOS disk interrupt

    jmp start_pm

printhex:
    mov cx, 4           ; 4 hex digits in a 16-bit value
.hexloop:
    rol ax, 4           ; rotate left 4 bits, brings next nibble to bottom
    mov bx, 0           ; zero bx so upper byte doesnt corrupt our index
    mov bl, al          ; copy low byte of ax into bl
    and bx, 0xF         ; mask off everything except bottom 4 bits (one nibble)
    mov bl, [hex_chars + bx] ; look up ascii character in our table
    push ax             ; save ax — BIOS int 0x10 will clobber it
    push cx             ; save cx — BIOS int 0x10 might clobber it
    mov al, bl          ; move character into al for BIOS
    mov ah, 0x0E        ; BIOS teletype function
    int 0x10            ; print character
    pop cx              ; restore cx
    pop ax              ; restore ax
    loop .hexloop       ; decrement cx, jump back if not zero
    ret

message   db "I AM A BOOTLOADER", 0
hex_chars db "0123456789ABCDEF"


; ============================================
; SWITCH TO 32-bit PROTECTED MODE
; Steps:
;   1. Disable interrupts (BIOS interrupts wont work in protected mode)
;   2. Load the GDT (tell CPU about our memory segments)
;   3. Flip bit 0 of cr0 (the "enable protected mode" bit)
;   4. Far jump to flush CPU pipeline and land in 32-bit code
; ============================================
start_pm:
    cli                 ; disable interrupts — BIOS interrupts (int 0x10 etc)
                        ; are 16-bit and will crash us in protected mode

    lgdt [gdt_descriptor] ; load the GDT — tell the CPU where our segment
                          ; table lives in memory

    mov eax, cr0        ; cr0 is a special control register
                        ; we cant mov directly into it, so we go through eax
    or eax, 1           ; set bit 0 — this is the protected mode enable bit
    mov cr0, eax        ; write it back — CPU is NOW in protected mode

    ; far jump — this does two things:
    ;   1. flushes the CPU pipeline (it was pre-fetching 16-bit instructions)
    ;   2. loads cs (code segment) with our GDT code segment selector (0x08)
    ; 0x08 = second GDT entry (null is first at 0x00, code is second at 0x08)
    jmp 0x08:protected_mode

; ============================================
; from this point on we are in 32-bit protected mode
; we tell NASM to assemble the following as 32-bit
; ============================================
[bits 32]
protected_mode:
    ; set up all data segment registers to point to our GDT data segment
    ; 0x10 = third GDT entry (null=0x00, code=0x08, data=0x10)
    mov ax, 0x10        ; data segment selector
    mov ds, ax          ; data segment
    mov ss, ax          ; stack segment
    mov es, ax          ; extra segment
    mov fs, ax          ; extra segment 2
    mov gs, ax          ; extra segment 3

    ; set up a stack — we pick an arbitrary address below where we loaded
    ; our bootloader (0x7C00) so it doesnt overwrite our code
    mov ebp, 0x7C00
    mov esp, ebp

    jmp 0x1000        ;jump to kernel

; ============================================
; GDT — Global Descriptor Table
; tells the CPU about memory segments
; each entry is 8 bytes
; rings: 0=kernel(full access), 3=userspace(restricted)
; for now everything is ring 0 — we are the kernel
; ============================================

gdt_start:
    ; entry 0 — null descriptor
    ; CPU requires the first entry to be all zeros
    dd 0x0
    dd 0x0

    ; entry 1 — code segment (0x08)
    ; covers full 4GB, ring 0, executable, readable
    dw 0xFFFF           ; limit bits 0-15 (0xFFFF = 4GB with granularity bit)
    dw 0x0000           ; base bits 0-15 (segment starts at address 0)
    db 0x00             ; base bits 16-23
    db 10011010b        ; access byte:
                        ;   bit 7: present (1 = valid segment)
                        ;   bit 6-5: ring level (00 = ring 0, kernel)
                        ;   bit 4: descriptor type (1 = code/data)
                        ;   bit 3: executable (1 = code segment)
                        ;   bit 1: readable (1 = can read code segment)
    db 11001111b        ; flags + limit bits 16-19:
                        ;   bit 7: granularity (1 = limit in 4KB pages)
                        ;   bit 6: size (1 = 32-bit, 0 = 16-bit)
    db 0x00             ; base bits 24-31

    ; entry 2 — data segment (0x10)
    ; covers full 4GB, ring 0, writable
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b        ; access byte:
                        ;   same as code but bit 3 = 0 (not executable)
                        ;   bit 1 = 1 (writable data segment)
    db 11001111b
    db 0x00

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1   ; size of GDT minus 1 (CPU requirement)
    dd gdt_start                  ; physical address of GDT

; pad file to exactly 510 bytes, then write boot signature
; BIOS checks last 2 bytes for 0x55 0xAA to confirm this is bootable
times 510 - ($ - $$) db 0
dw 0xAA55
