#ifndef IDT_H
#define IDT_H

void port_write(unsigned short port, unsigned char data);
unsigned char port_read(unsigned short port);
void pic_remap();
void idt_init();
void keyboard_handler();
void keyboard_handler_asm();

#endif
