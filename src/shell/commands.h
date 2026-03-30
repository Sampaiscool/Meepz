#ifndef COMMANDS_H
#define COMMANDS_H

#define MAX_COMMANDS 32

typedef void (*command_func_t)(char *arg);

typedef struct {
    char *name;
    command_func_t func;
} command_t;

void register_command(char *name, command_func_t func);
void execute_command(char *cmd, char *arg);
command_func_t find_command(char *name);

void cmd_ls(char *arg);
void cmd_cd(char *arg);
void cmd_mkdir(char *arg);
void cmd_touch(char *arg);
void cmd_cat(char *arg);
void cmd_write(char *arg);
void cmd_rm(char *arg);
void cmd_hostsave(char *arg);
void cmd_clear(char *arg);
void cmd_help(char *arg);
void cmd_hi(char *arg);
void cmd_poweroff(char *arg);
void cmd_ember(char *arg);

void commands_init();

#endif
