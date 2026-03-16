#include "idt.h"

int cursor = 0;

char shell_buffer[256];
int buffer_idx = 0;

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

    while (port_read(0x64) & 0x01) {
        port_read(0x60);
    }

    clear_screen();
    char font = 0x0F;

    print("Yup", font);
}

// we make our own string
int str_step_compare(char *str1, char *str2) {
    int i = 0;
    while (str1[i] != 0 && str2[i] != 0) {
        if (str1[i] != str2[i]) return 0;
        i++;
    }
    return (str1[i] == str2[i]);
}

void execute_command() {
    print("\n", 0x0F);

    if (str_step_compare(shell_buffer, "clear")) {
        clear_screen();
    } else if (str_step_compare(shell_buffer, "help")) {
        print("Meepz Commands: help, clear, hi", 0x0A); // green
    } else if (str_step_compare(shell_buffer, "hi")) {
        print("Alloha from the kernel!", 0x0B); // lightblue
    } else {
        print("Unknown: ", 0x0C); // red
        print(shell_buffer, 0x0C);
    }

    // reset buffer for next one
    for(int i = 0; i < 256; i++) shell_buffer[i] = 0;
    buffer_idx = 0;
    print("\nMeepz> ", 0x0E); // yellow
}

void keyboard_handler() {
    static const unsigned char scancode_to_ascii[] = {
        0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
        '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
    };

    unsigned char scancode = port_read(0x60);
    port_write(0x20, 0x20); // EOI

    if (scancode & 0x80) return; // we do not care bc we are the shadow wizard money gang

    char letter = scancode_to_ascii[scancode];

    if (letter == '\n') {
        execute_command();
    } else if (letter == '\b') {
        if (buffer_idx > 0) {
            buffer_idx--;
            shell_buffer[buffer_idx] = 0;
            cursor -= 2;
            print(" ", 0x0F);
            cursor -= 2;
        }
    } else if (letter != 0) {
        if (buffer_idx < 255) {
            shell_buffer[buffer_idx++] = letter;
            char str[2] = {letter, 0};
            print(str, 0x0F);
        }
    }
}
