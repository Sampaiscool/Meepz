#include "shell.h"
#include "../lib/screen.h"
#include "commands.h"
#include "../fs/fs.h"

int starts_with(char *str, char *prefix);

char shell_buffer[256];
int buffer_idx = 0;
int history_index = -1;

static char history[MAX_HISTORY][256];
static int history_count = 0;

void clear_input_line() {
    while (buffer_idx > 0) {
        print("\b \b", 0x0F);
        buffer_idx--;
    }
    for (int i = 0; i < 256; i++) shell_buffer[i] = 0;
}

void history_add(char *cmd) {
    if (cmd[0] == 0) return;

    int len = 0;
    while (cmd[len] != 0 && len < 255) len++;

    if (history_count > 0) {
        int last_idx = (history_count - 1);
        int last_len = 0;
        while (history[last_idx][last_len] != 0) last_len++;
        if (last_len == len) {
            int same = 1;
            for (int i = 0; i < len; i++) {
                if (history[last_idx][i] != cmd[i]) { same = 0; break; }
            }
            if (same) return;
        }
    }

    for (int i = 0; i < len && i < 255; i++) {
        history[history_count][i] = cmd[i];
    }
    history[history_count][len] = 0;

    if (history_count < MAX_HISTORY - 1) {
        history_count++;
    }
    history_index = -1;
}

void history_up() {
    if (history_count == 0) return;

    if (history_index == -1) {
        history_index = history_count - 1;
    } else if (history_index > 0) {
        history_index--;
    }

    clear_input_line();

    int i = 0;
    while (history[history_index][i] != 0) {
        shell_buffer[i] = history[history_index][i];
        char str[2] = {history[history_index][i], 0};
        print(str, 0x0F);
        i++;
        buffer_idx++;
    }
}

void history_down() {
    if (history_index == -1) return;

    if (history_index < history_count - 1) {
        history_index++;
        clear_input_line();

        int i = 0;
        while (history[history_index][i] != 0) {
            shell_buffer[i] = history[history_index][i];
            char str[2] = {history[history_index][i], 0};
            print(str, 0x0F);
            i++;
            buffer_idx++;
        }
    } else {
        history_index = -1;
        clear_input_line();
    }
}

void insert_char_at(char *buffer, int pos, char letter, int max_size) {
    int len = 0;
    while (buffer[len] != 0 && len < max_size) len++;

    for (int i = len; i > pos; i--) {
        if (i < max_size - 1) {
            buffer[i] = buffer[i - 1];
        }
    }
    buffer[pos] = letter;
    if (len < max_size - 1 && buffer[len+1] == 0) {
        buffer[len + 1] = 0;
    }
}

void refresh_line(char *buffer, int buf_idx);

void shell_execute() {
    print("\n", 0x0F);

    history_add(shell_buffer);

    if (starts_with(shell_buffer, "cd ")) {
        cmd_cd(&shell_buffer[3]);
    } else if (starts_with(shell_buffer, "mkdir ")) {
        cmd_mkdir(&shell_buffer[6]);
    } else if (starts_with(shell_buffer, "touch ")) {
        cmd_touch(&shell_buffer[6]);
    } else if (starts_with(shell_buffer, "cat ")) {
        cmd_cat(&shell_buffer[4]);
    } else if (starts_with(shell_buffer, "write ")) {
        cmd_write(&shell_buffer[6]);
    } else if (starts_with(shell_buffer, "ember ")) {
        cmd_ember(&shell_buffer[6]);
    } else if (starts_with(shell_buffer, "rm ")) {
        cmd_rm(&shell_buffer[3]);
    } else if (starts_with(shell_buffer, "hostsave")) {
        cmd_hostsave(0);
    } else if (str_step_compare(shell_buffer, "clear")) {
        cmd_clear(0);
    } else if (str_step_compare(shell_buffer, "ls")) {
        cmd_ls(0);
    } else if (str_step_compare(shell_buffer, "help")) {
        cmd_help(0);
    } else if (str_step_compare(shell_buffer, "hi")) {
        cmd_hi(0);
    } else if (str_step_compare(shell_buffer, "poweroff")) {
        cmd_poweroff(0);
    } else if (shell_buffer[0] != 0) {
        print("Unknown: ", 0x0C);
        print(shell_buffer, 0x0C);
    }

    for (int i = 0; i < 256; i++) shell_buffer[i] = 0;
    buffer_idx = 0;
    history_index = -1;
    
    if (is_editing != 1) {
        prompt_show();
    }
}

void prompt_show() {
    print("\nMeepz:", 0x0E);
    print(current_directory->name, 0x0E);
    print("> ", 0x0E);
}

void handle_shell_input(char letter) {
    if (letter == '\n') {
        while(shell_buffer[buffer_idx] != 0) {
            buffer_idx++;
            cursor += 2;
        }
        shell_execute();
    } else if (letter == '\b') {
        if (buffer_idx > 0) {
            for (int i = buffer_idx - 1; shell_buffer[i] != 0; i++) {
                shell_buffer[i] = shell_buffer[i + 1];
            }
            buffer_idx--;
            cursor -= 2;
            refresh_line(shell_buffer, buffer_idx);
        }
    } else if (buffer_idx < 255) {
        insert_char_at(shell_buffer, buffer_idx, letter, 256);

        char str[2] = {letter, 0};
        print(str, 0x0F);

        refresh_line(shell_buffer, buffer_idx + 1);

        buffer_idx++;
        update_cursor(cursor);
    }
}
