#ifndef EDITOR_H
#define EDITOR_H

#include "../fs/fs.h"

extern int is_editing;
extern FSNode *editing_node;

void start_ember_editor(char *name);
void handle_ember_input(char letter);

#endif
