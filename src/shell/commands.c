#include "commands.h"
#include "../lib/screen.h"
#include "../fs/fs.h"
#include "editor.h"

static command_t command_table[MAX_COMMANDS];
static int command_count = 0;

void register_command(char *name, command_func_t func) {
    if (command_count < MAX_COMMANDS) {
        command_table[command_count].name = name;
        command_table[command_count].func = func;
        command_count++;
    }
}

command_func_t find_command(char *name) {
    for (int i = 0; i < command_count; i++) {
        if (str_step_compare(command_table[i].name, name)) {
            return command_table[i].func;
        }
    }
    return 0;
}

int starts_with(char *str, char *prefix) {
    int i = 0;
    while (prefix[i] != 0) {
        if (str[i] != prefix[i]) return 0;
        i++;
    }
    return 1;
}

void execute_command(char *cmd, char *arg) {
    command_func_t cmd_func = find_command(cmd);
    if (cmd_func) {
        cmd_func(arg);
    } else {
        print("Unknown: ", 0x0C);
        print(cmd, 0x0C);
    }
}

void cmd_ls(char *arg) {
    (void)arg;
    list_files();
}

void cmd_cd(char *arg) {
    change_directory(arg);
}

void cmd_mkdir(char *arg) {
    make_directory(arg);
}

void cmd_touch(char *arg) {
    make_file(arg);
}

void cmd_cat(char *arg) {
    cat_file(arg);
}

void cmd_write(char *arg) {
    write_to_file(arg);
}

void cmd_rm(char *arg) {
    remove_node(arg);
}

void cmd_hostsave(char *arg) {
    fs_save_to_disk();
}

void cmd_clear(char *arg) {
    (void)arg;
    clear_screen();
}

void cmd_help(char *arg) {
    (void)arg;
    print("Commands: ls, cd, mkdir, touch, clear, rm, cat, write, ember, help, hi, poweroff", 0x0A);
}

void cmd_hi(char *arg) {
    (void)arg;
    print("Alloha from the kernel!", 0x0B);
}

void cmd_poweroff(char *arg) {
    (void)arg;
    print("Goodbye!\n", 0x0A);
    __asm__("hlt");
}

void cmd_ember(char *arg) {
    start_ember_editor(arg);
}

void commands_init() {
    register_command("ls", cmd_ls);
    register_command("cd", cmd_cd);
    register_command("mkdir", cmd_mkdir);
    register_command("touch", cmd_touch);
    register_command("cat", cmd_cat);
    register_command("write", cmd_write);
    register_command("rm", cmd_rm);
    register_command("hostsave", cmd_hostsave);
    register_command("clear", cmd_clear);
    register_command("help", cmd_help);
    register_command("hi", cmd_hi);
    register_command("poweroff", cmd_poweroff);
    register_command("ember", cmd_ember);
}
