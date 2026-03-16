#include "idt.h"

int cursor = 0;

void clear_screen() {
    char *video = (char*) 0xB8000;

    for (int i = 0; i < 4000; i += 2) {
        video [i] = ' ';
        video [i + 1] = 0x0F;
    }
    cursor = 0;
}

void print(char *string, char hex) {
    char *video = (char*) 0xB8000 + cursor;

    while (*string != 0) {

        if (*string == '\n') {
            int current_row = cursor / 160;
            int next_row = current_row + 1;
            int new_cursor = next_row * 160;

            cursor = new_cursor;
            video = (char*) 0xB8000 + cursor;

            string++;
            continue;
        }

        video[0] = *string;
        video[1] = hex;

        string++;
        video += 2;

        cursor += 2;
    }
}

void main() {
    pic_remap();
    idt_init();

    clear_screen();
    char font = 0x0F;

    print("TeaTeaTHuaba\n", font);
    print("MAKMAK", font);
}

void keyboard_handler() {
    unsigned char scancode = port_read(0x60);
    port_write(0x20, 0x20);
    print("Alloha", 0x0F);
}
