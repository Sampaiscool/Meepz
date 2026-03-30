#include "idt.h"

typedef struct {
    unsigned short base_low;
    unsigned short selector;
    unsigned char  zero;
    unsigned char  flags;
    unsigned short base_high;
} __attribute__((packed)) IDTEntry;

typedef struct {
    unsigned short limit;
    unsigned int   base;
} __attribute__((packed)) IDTDescriptor;

IDTEntry idt[256] = {0};
IDTDescriptor idt_desc = {0};

void keyboard_handler();

void port_write(unsigned short port, unsigned char data) {
    __asm__("out %%al, %%dx" : : "a"(data), "d"(port));
}

unsigned char port_read(unsigned short port) {
    unsigned char result;
    __asm__("in %%dx, %%al" : "=a"(result) : "d"(port));
    return result;
}

unsigned short port_read_16(unsigned short port) {
    unsigned short result;
    __asm__("inw %%dx, %%ax" : "=a"(result) : "d"(port));
    return result;
}

void dummy_handler() {
    port_write(0x20, 0x20);
}

void pic_remap() {
    port_write(0x20, 0x11);
    port_write(0xA0, 0x11);

    port_write(0x21, 0x20);
    port_write(0xA1, 0x28);

    port_write(0x21, 0x04);
    port_write(0xA1, 0x02);

    port_write(0x21, 0x01);
    port_write(0xA1, 0x01);

    port_write(0x21, 0xFD);
    port_write(0xA1, 0xFF);
}

void set_idt_entry(int n, unsigned int handler) {
    idt[n].base_low  = handler & 0xFFFF;
    idt[n].base_high = (handler >> 16) & 0xFFFF;
    idt[n].selector  = 0x08;
    idt[n].zero      = 0;
    idt[n].flags     = 0x8E;
}

void idt_init() {
    idt_desc.limit = (sizeof(IDTEntry) * 256) - 1;
    idt_desc.base  = (unsigned int) &idt;

    for (int i = 0; i < 256; i++) {
        set_idt_entry(i, (unsigned int)dummy_handler); 
    }

    set_idt_entry(33, (unsigned int)keyboard_handler_asm);

    __asm__ volatile("lidt %0" : : "m"(idt_desc));
    __asm__ volatile("sti");
}
