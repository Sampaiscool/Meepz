#include "idt.h"

typedef struct {
    unsigned short base_low;   // lower 16 bits of handler address
    unsigned short selector;   // code segment selector (0x08 from our GDT)
    unsigned char  zero;       // always 0
    unsigned char  flags;      // type and attributes
    unsigned short base_high;  // upper 16 bits of handler address
} __attribute__((packed)) IDTEntry;

typedef struct {
    unsigned short limit;  // size of IDT - 1
    unsigned int   base;   // address of IDT
} __attribute__((packed)) IDTDescriptor;

IDTEntry idt[256] = {0};
IDTDescriptor idt_desc = {0};

void keyboard_handler();

// port I/O helpers — inline ASM to read/write hardware ports)
void port_write(unsigned short port, unsigned char data) {
    __asm__("out %%al, %%dx" : : "a"(data), "d"(port));
}

unsigned char port_read(unsigned short port) {
    unsigned char result;
    __asm__("in %%dx, %%al" : "=a"(result) : "d"(port));
    return result;
}

void dummy_handler() {
    port_write(0x20, 0x20); // Send EOI so PIC knows we heard it
}

void pic_remap() {
    // tell both PICs we're initializing (0x11 = init command)
    port_write(0x20, 0x11);   // master PIC command port
    port_write(0xA0, 0x11);   // slave PIC command port

    // remap master to start at interrupt 32
    port_write(0x21, 0x20);   // master offset = 32
    port_write(0xA1, 0x28);   // slave offset = 40

    // tell PICs how they're connected to each other
    port_write(0x21, 0x04);   // master: slave is on IRQ2
    port_write(0xA1, 0x02);   // slave: connected to master IRQ2

    // set 8086 mode
    port_write(0x21, 0x01);
    port_write(0xA1, 0x01);

    // 0xFD = 11111101 in binary — only bit 1 (IRQ1/keyboard) is unmasked
    port_write(0x21, 0xFD);  // only unmask keyboard on master PIC
    port_write(0xA1, 0xFF);  // mask everything on slave PIC
}

void set_idt_entry(int n, unsigned int handler) {
    idt[n].base_low  = handler & 0xFFFF;
    idt[n].base_high = (handler >> 16) & 0xFFFF;
    idt[n].selector  = 0x08;   // our GDT code segment
    idt[n].zero      = 0;
    idt[n].flags     = 0x8E;   // present, ring 0, interrupt gate
}

void idt_init() {
    idt_desc.limit = (sizeof(IDTEntry) * 256) - 1;
    idt_desc.base  = (unsigned int) &idt;

    // 1. Zet eerst alles op een dummy_handler (of 0)
    for (int i = 0; i < 256; i++) {
        set_idt_entry(i, (unsigned int)dummy_handler); 
    }

    // 2. Alleen interrupt 33 is voor het keyboard (IRQ 1 + 32 offset)
    set_idt_entry(33, (unsigned int)keyboard_handler_asm);

    __asm__ volatile("lidt %0" : : "m"(idt_desc));
    __asm__ volatile("sti"); // Zet interrupts aan
}
