#include "screen.h"

int cursor = 0;

void clear_screen() {
    char *video = (char*) 0xB8000;

    for (int i = 0; i < 4000; i += 2) {
        video [i] = ' ';
        video [i + 1] = 0x0F;
    }
    cursor = 0;
}

void scroll_if_needed() {
    char *video = (char*)0xB8000;
    int max_cursor = 25 * 160;

    if (cursor >= max_cursor) {
        for (int i = 0; i < 24 * 160; i++) {
            video[i] = video[i + 160];
        }
        for (int i = 24 * 160; i < 25 * 160; i += 2) {
            video[i]     = ' ';
            video[i + 1] = 0x0F;
        }
        cursor = 24 * 160;
    }
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

        scroll_if_needed();
        video = (char*) 0xB8000 + cursor;
        video[0] = *string;
        video[1] = hex;

        string++;
        video += 2;

        cursor += 2;
    }
}

void update_cursor(int pos) {
    unsigned short position = pos / 2;

    port_write(0x3D4, 0x0F);
    port_write(0x3D5, (unsigned char)(position & 0xFF));
    port_write(0x3D4, 0x0E);
    port_write(0x3D5, (unsigned char)((position >> 8) & 0xFF));
}

void refresh_line(char *buffer, int buf_idx) {
    int temp_cursor = cursor;
    int i = buf_idx;

    while (buffer[i] != 0) {
        char str[2] = {buffer[i], 0};
        print(str, 0x0F);
        i++;
    }
    
    print(" ", 0x0F);

    cursor = temp_cursor;
    update_cursor(cursor);
}
