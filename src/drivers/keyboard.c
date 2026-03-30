#include "keyboard.h"
#include "../cpu/idt.h"
#include "../lib/screen.h"
#include "../shell/shell.h"
#include "../shell/editor.h"

extern int cursor;

void keyboard_handler() {
    static const unsigned char scancode_to_ascii[] = {
        0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
        '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
    };

    unsigned char scancode = port_read(0x60);
    port_write(0x20, 0x20);

    if (scancode & 0x80) return;

    if (scancode == 0x4B) {
        if (is_editing) {
            if (cursor > 480) cursor -= 2; 
        } else {
            if (buffer_idx > 0) { buffer_idx--; cursor -= 2; }
        }
        update_cursor(cursor);
        return;
    }
    if (scancode == 0x4D) {
        if (is_editing) {
            int idx = (cursor - 480) / 2;
            if (editing_node->content[idx] != 0) cursor += 2;
        } else {
            if (shell_buffer[buffer_idx] != 0) { buffer_idx++; cursor += 2; }
        }
        update_cursor(cursor);
        return;
    }
    if (scancode == 0x48) {
        if (is_editing) {
            if (cursor >= 160) cursor -= 160;
        } else {
            history_up();
        }
        update_cursor(cursor);
        return;
    }
    if (scancode == 0x50) {
        if (is_editing) {
            if (cursor < 3840) cursor += 160;
        } else {
            history_down();
        }
        update_cursor(cursor);
        return;
    }

    if (scancode == 1 && is_editing) {
        is_editing = 0;
        editing_node = 0;
        clear_screen();
        print("Ember: File saved.\n", 0x0A);
        extern void shell_execute();
        shell_execute();
        update_cursor(cursor);
        return;
    }

    char letter = scancode_to_ascii[scancode];
    if (letter == 0) return;

    if (is_editing) {
        handle_ember_input(letter);
    } else {
        handle_shell_input(letter);
    }

    update_cursor(cursor);
}
