#include "idt.h"

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
unsigned char* heap_pointer = (unsigned char*)0x100000; 

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

    // 1. Initialiseer de Root
    root_node = create_node("/", DIRECTORY);
    root_node->parent = 0; // De root heeft geen bovenliggende map
    current_directory = root_node;

    // 2. Maak een 'home' map
    FSNode* home = create_node("home", DIRECTORY);
    home->parent = root_node;
    root_node->children[root_node->child_count++] = home;

    // 3. Maak een bestand IN de 'home' map
    FSNode* meep = create_node("meep.txt", FILE);
    meep->parent = home;
    home->children[home->child_count++] = meep;

    // 4. Maak een 'dev' map in de root
    FSNode* dev = create_node("dev", DIRECTORY);
    dev->parent = root_node;
    root_node->children[root_node->child_count++] = dev;

    clear_screen();
    print("Meepz OS Loaded ! Shadow Wizard Money Gang\n", 0x0B);
    
    // Toon de prompt met de huidige locatie
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
            print("[DIR] ", 0x0B); // Blauw voor mappen
        } else {
            print("[FIL] ", 0x0A); // Groen voor files
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

    // Zoek in de huidige map naar de target
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

void execute_command() {
    print("\n", 0x0F);

    if (str_step_compare(shell_buffer, "clear")) {
        clear_screen();
    } else if (shell_buffer[0] == 'c' && shell_buffer[1] == 'd' && shell_buffer[2] == ' ') {
        change_directory(&shell_buffer[3]);
    } else if (str_step_compare(shell_buffer, "help")) {
        print("Meepz Commands: help, clear, hi", 0x0A); // green
    } else if (str_step_compare(shell_buffer, "hi")) {
        print("Alloha from the kernel!", 0x0B); // lightblue
    } else if (str_step_compare(shell_buffer, "ls")) {
        list_files();
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
