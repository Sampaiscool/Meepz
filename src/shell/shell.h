#ifndef SHELL_H
#define SHELL_H

#define MAX_HISTORY 32

extern char shell_buffer[256];
extern int buffer_idx;
extern int is_editing;
extern int history_index;

void handle_shell_input(char letter);
void shell_execute();
void prompt_show();
void insert_char_at(char *buffer, int pos, char letter, int max_size);
void history_add(char *cmd);
void history_up();
void history_down();
void clear_input_line();

#endif
