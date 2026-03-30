#include "editor.h"
#include "../lib/screen.h"
#include "shell.h"
#include "../fs/fs.h"

int is_editing = 0;
FSNode *editing_node = 0;

void start_ember_editor(char *name) {
    for (int i = 0; i < current_directory->child_count; i++) {
        if (str_step_compare(current_directory->children[i]->name, name)) {
            if (current_directory->children[i]->type == FILE) {
                editing_node = current_directory->children[i];
                is_editing = 1;
                clear_screen();
                print("Ember Editor (Press ESC to Save & Exit)\n", 0x0B);
                print("Editing: ", 0x0F);
                print(editing_node->name, 0x0E);
                print("\n\n", 0x0F);

                print(editing_node->content, 0x0F);
                return;
            }
        }
    }
    print("File not found!\n", 0x0C);
}

void handle_ember_input(char letter) {
    int start_of_text = 480; 
    int idx = (cursor - start_of_text) / 2;

    if (letter == '\b') {
        if (idx > 0) {
            for (int i = idx - 1; editing_node->content[i] != 0; i++) {
                editing_node->content[i] = editing_node->content[i + 1];
            }
            cursor -= 2;
            refresh_line(editing_node->content, idx - 1);
        }
    } else if (idx < 254) {
        insert_char_at(editing_node->content, idx, letter, 255);

        int old_cursor = cursor;
        refresh_line(editing_node->content, idx); 

        cursor = old_cursor + 2;
        update_cursor(cursor);
    }
}
