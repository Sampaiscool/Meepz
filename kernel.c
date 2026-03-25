#include "idt.h"
#include "gdt.h"

#define MAX_NAME 64
#define MAX_CHILDREN 16

typedef enum {
    FILE,
    DIRECTORY
} NodeType;

typedef struct FSNode {
    char name[MAX_NAME];
    NodeType type;
    struct FSNode *parent;
    struct FSNode *children[MAX_CHILDREN];
    int child_count;
    char content[256];
} FSNode;

FSNode *current_directory;
FSNode *root_node;

int cursor = 0;

char shell_buffer[256];
int buffer_idx = 0;

// we make our own string
int str_step_compare(char *str1, char *str2) {
    int i = 0;
    while (str1[i] != 0 && str2[i] != 0) {
        if (str1[i] != str2[i]) return 0;
        i++;
    }
    return (str1[i] == str2[i]);
}

void strncpy(char *dest, const char *src, int n) {
    for (int i = 0; i < n && src[i] != 0; i++) {
        dest[i] = src[i];
    }
    dest[n] = 0;
}

// let the heap start before kernel
unsigned char* heap_pointer = (unsigned char*)0x200000; 

void* malloc(int size) {
    void* res = (void*)heap_pointer;
    heap_pointer += size;
    
    // set memory to 0
    unsigned char* p = (unsigned char*)res;
    for(int i = 0; i < size; i++) p[i] = 0;
    
    return res;
}

FSNode* create_node(char *name, NodeType type) {
    FSNode* node = (FSNode*)malloc(sizeof(FSNode));
    strncpy(node->name, name, MAX_NAME - 1);
    node->type = type;
    node->child_count = 0;
    return node;
}

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
    int max_cursor = 25 * 160; // 25 rows * 160 bytes per row

    if (cursor >= max_cursor) {
        // shift every row up by one
        for (int i = 0; i < 24 * 160; i++) {
            video[i] = video[i + 160];
        }
        // clear the last row
        for (int i = 24 * 160; i < 25 * 160; i += 2) {
            video[i]     = ' ';
            video[i + 1] = 0x0F;
        }
        // put cursor at start of last row
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

void main() {
    gdt_install();
    pic_remap();
    idt_init();

    // init the root
    root_node = create_node("/", DIRECTORY);
    root_node->parent = 0; // root has no parrent
    current_directory = root_node;

    FSNode* home = create_node("home", DIRECTORY);
    home->parent = root_node;
    root_node->children[root_node->child_count++] = home;

    FSNode* meep = create_node("meep.txt", FILE);
    meep->parent = home;
    home->children[home->child_count++] = meep;

    FSNode* dev = create_node("dev", DIRECTORY);
    dev->parent = root_node;
    root_node->children[root_node->child_count++] = dev;

     clear_screen();
    print("Meepz OS Loaded ! Shadow Wizard Money Gang\n", 0x0B);
    print("Meepz:", 0x0E);
    print(current_directory->name, 0x0E);
    print("> ", 0x0E);
}

void list_files() {
    print("Listing directory: ", 0x0F);
    print(current_directory->name, 0x0F);
    print("\n", 0x0F);

    for (int i = 0; i < current_directory->child_count; i++) {
        if (current_directory->children[i]->type == DIRECTORY) {
            print("[DIR] ", 0x0B); // blue
        } else {
            print("[FIL] ", 0x0A); // green
        }
        print(current_directory->children[i]->name, 0x0F);
        print("\n", 0x0F);
    }
}

void change_directory(char *target_name) {
    if (str_step_compare(target_name, "..")) {
        if (current_directory->parent != 0) {
            current_directory = current_directory->parent;
        }
        return;
    }

    // search in current directory
    for (int i = 0; i < current_directory->child_count; i++) {
        if (str_step_compare(current_directory->children[i]->name, target_name)) {
            if (current_directory->children[i]->type == DIRECTORY) {
                current_directory = current_directory->children[i];
                return;
            } else {
                print("Error: Not a directory\n", 0x0C);
                return;
            }
        }
    }
    print("Directory not found\n", 0x0C);
}

void make_directory(char *name) {
    if (current_directory->child_count < 16) {
        FSNode* new_dir = create_node(name, DIRECTORY);
        new_dir->parent = current_directory;
        current_directory->children[current_directory->child_count++] = new_dir;
    } else {
        print("Error: Directory full!\n", 0x0C);
    }
}

void make_file(char *name) {
    if (current_directory->child_count < 16) {
        FSNode* new_file = create_node(name, FILE);
        new_file->parent = current_directory;
        current_directory->children[current_directory->child_count++] = new_file;
    } else {
        print("Error: Directory full!\n", 0x0C);
    }
}

int starts_with(char *str, char *prefix) {
    int i = 0;
    while (prefix[i] != 0) {
        if (str[i] != prefix[i]) return 0;
        i++;
    }
    return 1;
}

void execute_command() {
    print("\n", 0x0F);

    if (str_step_compare(shell_buffer, "clear")) {
        clear_screen();
    } else if (str_step_compare(shell_buffer, "ls")) {
        list_files();
    } else if (starts_with(shell_buffer, "cd ")) {
        change_directory(&shell_buffer[3]); // "cd " is 3 chars
    } else if (starts_with(shell_buffer, "mkdir ")) {
        make_directory(&shell_buffer[6]);   // "mkdir " is 6 chars
    } else if (starts_with(shell_buffer, "touch ")) {
        make_file(&shell_buffer[6]);        // "touch " is 6 chars
    } else if (str_step_compare(shell_buffer, "help")) {
        print("Commands: ls, cd, mkdir, touch, clear, hi, poweroff", 0x0A);
    } else if (str_step_compare(shell_buffer, "hi")) {
        print("Alloha from the kernel!", 0x0B);
    } else if (str_step_compare(shell_buffer, "poweroff")) {
        print("Goodbye!\n", 0x0A);
        __asm__("hlt");
    } else if (shell_buffer[0] != 0) { // Alleen printen als er echt iets getypt is
        print("Unknown: ", 0x0C);
        print(shell_buffer, 0x0C);
    }

    // Reset buffer
    for (int i = 0; i < 256; i++) shell_buffer[i] = 0;
    buffer_idx = 0;

    print("\nMeepz:", 0x0E);
    print(current_directory->name, 0x0E);
    print("> ", 0x0E);
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
