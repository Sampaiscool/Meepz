#include "fs.h"
#include "../lib/screen.h"
#include "../lib/memory.h"
#include "../drivers/ata.h"

FSNode *current_directory;
FSNode *root_node;

char *blocked_list[] = {"..", ".", "/", " ", 0};
char *allowed_extensions[] = {".txt", ".asm", ".meep", 0};

int str_step_compare(char *str1, char *str2) {
    int i = 0;
    while (str1[i] != 0 && str2[i] != 0) {
        if (str1[i] != str2[i]) return 0;
        i++;
    }
    return (str1[i] == str2[i]);
}

void strncpy_custom(char *dest, const char *src, int n) {
    for (int i = 0; i < n && src[i] != 0; i++) {
        dest[i] = src[i];
    }
    dest[n] = 0;
}

FSNode* create_node(char *name, NodeType type) {
    FSNode* node = (FSNode*)malloc(sizeof(FSNode));
    strncpy_custom(node->name, name, MAX_NAME - 1);
    node->type = type;
    node->child_count = 0;
    node->content[0] = 0;
    return node;
}

int has_valid_extension(char *name) {
    int name_len = 0;
    while (name[name_len] != 0) name_len++;

    int dot_index = -1;
    for (int i = name_len - 1; i >= 0; i--) {
        if (name[i] == '.') {
            dot_index = i;
            break;
        }
    }

    if (dot_index == -1) return 0;

    for (int i = 0; allowed_extensions[i] != 0; i++) {
        if (str_step_compare(&name[dot_index], allowed_extensions[i])) {
            return 1;
        }
    }

    return 0;
}

int is_name_blocked(char *name) {
    for (int i = 0; blocked_list[i] != 0; i++) {
        if (str_step_compare(name, blocked_list[i])) {
            return 1;
        }
    }
    return 0;
}

void list_files() {
    print("Listing: ", 0x0F);
    print(current_directory->name, 0x0F);
    print("\n", 0x0F);

    for (int i = 0; i < current_directory->child_count; i++) {
        if (current_directory->children[i]->type == DIRECTORY) {
            print("[DIR] ", 0x0B);
        } else {
            print("[FIL] ", 0x0A);
        }
        print(current_directory->children[i]->name, 0x0F);
        print("\n", 0x0F);
    }
}

void change_directory(char *name) {
    if (str_step_compare(name, "..")) {
        if (current_directory->parent != 0) {
            current_directory = current_directory->parent;
        }
        return;
    }

    for (int i = 0; i < current_directory->child_count; i++) {
        if (str_step_compare(current_directory->children[i]->name, name)) {
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
    if (is_name_blocked(name)) {
        print("Error: Name is blocked!\n", 0x0C);
        return;
    }

    if (current_directory->child_count < 16) {
        FSNode* new_dir = create_node(name, DIRECTORY);
        new_dir->parent = current_directory;
        current_directory->children[current_directory->child_count++] = new_dir;
    } else {
        print("Error: Directory full!\n", 0x0C);
    }
}

void make_file(char *name) {
    if (is_name_blocked(name)) {
        print("Error: Name is blocked!\n", 0x0C);
        return;
    }

    if (!has_valid_extension(name)) {
        print("Error: Invalid or missing extension!\n", 0x0C);
        print("Supported: .txt, .asm, .meep\n", 0x0E);
        return;
    }

    if (current_directory->child_count < 16) {
        FSNode* new_file = create_node(name, FILE);
        new_file->parent = current_directory;
        current_directory->children[current_directory->child_count++] = new_file;
    } else {
        print("Error: Directory full!\n", 0x0C);
    }
}

void remove_node(char *name) {
    int found = -1;
    for (int i = 0; i < current_directory->child_count; i++) {
        if (str_step_compare(current_directory->children[i]->name, name)) {
            found = i;
            break;
        }
    }

    if (found != -1) {
        for (int i = found; i < current_directory->child_count - 1; i++) {
            current_directory->children[i] = current_directory->children[i + 1];
        }
        current_directory->child_count--;
        print("Removed.\n", 0x0A);
    } else {
        print("File not found.\n", 0x0C);
    }
}

void cat_file(char *name) {
    for (int i = 0; i < current_directory->child_count; i++) {
        if (str_step_compare(current_directory->children[i]->name, name)) {
            if (current_directory->children[i]->type == FILE) {
                print(current_directory->children[i]->content, 0x0F);
                print("\n", 0x0F);
                return;
            } else {
                print("Error: Is a directory\n", 0x0C);
                return;
            }
        }
    }
    print("File not found\n", 0x0C);
}

void write_to_file(char *input) {
    char name[MAX_NAME];
    int i = 0;
    while (input[i] != ' ' && input[i] != 0) {
        name[i] = input[i];
        i++;
    }
    name[i] = 0;

    if (input[i] == ' ') i++;
    
    char data[256];
    int j = 0;
    while (input[i] != 0 && j < 255) {
        data[j++] = input[i++];
    }
    data[j] = 0;

    for (int j = 0; j < current_directory->child_count; j++) {
        if (str_step_compare(current_directory->children[j]->name, name)) {
            strncpy_custom(current_directory->children[j]->content, data, 255);
            print("Written to file.\n", 0x0A);
            return;
        }
    }
    print("File not found.\n", 0x0C);
}

void fs_init() {
    uint8_t buffer[512];
    ata_read_sector(0, buffer);
    
    Superblock* sb = (Superblock*)buffer;

    if (sb->magic == MEEP_MAGIC) {
        print("FS: MeepFS gedetecteerd! Laden...\n", 0x0A);
        fs_load_from_disk();
    } else {
        print("FS: Geen MeepFS gevonden. Formatteren...\n", 0x0E);
        
        Superblock new_sb;
        new_sb.magic = MEEP_MAGIC;
        new_sb.total_inodes = 0;
        new_sb.first_data_sec = 10;
        
        uint8_t write_buf[512] = {0};
        memory_copy(&new_sb, write_buf, sizeof(Superblock));
        ata_write_sector(0, write_buf);
        
        root_node = create_node("/", DIRECTORY);
        root_node->parent = 0;
        current_directory = root_node;
        
        print("FS: Schijf geformatteerd.\n", 0x0A);
    }
}

void fs_wait() {
    for(int i = 0; i < 5000; i++) { __asm__("nop"); }
}

int flatten_fs(FSNode* node, FSNode** list, int count) {
    if (!node || count >= MAX_NODES) return count;

    if (node != root_node) {
        list[count++] = node;
    }

    for (int i = 0; i < node->child_count; i++) {
        if (count < MAX_NODES) {
            count = flatten_fs(node->children[i], list, count);
        }
    }
    return count;
}

void fs_save_to_disk() {
    print("FS: Bezig met opslaan...\n", 0x0E);
    
    FSNode* all_nodes[MAX_NODES];
    memory_set(all_nodes, 0, sizeof(all_nodes));
    
    int total = flatten_fs(root_node, all_nodes, 0);

    for (int i = 0; i < total; i++) {
        FSNode* n = all_nodes[i];
        
        DiskInode di;
        memory_set(&di, 0, sizeof(DiskInode));
        strncpy_custom(di.name, n->name, 31);
        di.type = (uint32_t)n->type;
        di.present = 1;
        
        // Data begint na alle metadata sectoren
        // Sector 0 = Superblock
        // Sectoren 1 t/m MAX_NODES = Metadata
        // Sectoren daarna = Data
        di.start_sector = (MAX_NODES + 1) + i; 

        di.parent_index = -1;
        for (int j = 0; j < total; j++) {
            if (all_nodes[j] == n->parent) {
                di.parent_index = j;
                break;
            }
        }

        uint8_t meta_buf[512] = {0};
        memory_copy(&di, meta_buf, sizeof(DiskInode));
        ata_write_sector(1 + i, meta_buf);
        fs_wait();

        if (n->type == FILE) {
            uint8_t data_buf[512] = {0};
            memory_copy(n->content, data_buf, 256);
            ata_write_sector(di.start_sector, data_buf);
            fs_wait();
        }
    }

    // Wis ongebruikte slots zodat oude bestanden niet terugkomen
    for (int i = total; i < MAX_NODES; i++) {
        uint8_t empty[512] = {0};
        ata_write_sector(1 + i, empty);
    }

    print("FS: Alles recursief opgeslagen!\n", 0x0A);
}

void fs_load_from_disk() {
    uint8_t buffer[512];
    FSNode* loaded_nodes[MAX_NODES];
    memory_set(loaded_nodes, 0, sizeof(loaded_nodes));

    root_node = create_node("/", DIRECTORY);
    root_node->parent = 0;
    current_directory = root_node;

    // PASS 1: Nodes aanmaken
    for (int i = 0; i < MAX_NODES; i++) {
        ata_read_sector(1 + i, buffer);
        DiskInode* di = (DiskInode*)buffer;
        
        if (di->present == 1 && di->name[0] != 0) {
            loaded_nodes[i] = create_node(di->name, (NodeType)di->type);
            
            if (di->type == FILE) {
                uint8_t data_buf[512];
                ata_read_sector(di->start_sector, data_buf);
                memory_copy(data_buf, loaded_nodes[i]->content, 255);
            }
        }
    }

    // PASS 2: Hiërarchie herstellen
    for (int i = 0; i < MAX_NODES; i++) {
        if (loaded_nodes[i] == 0) continue;
        
        ata_read_sector(1 + i, buffer);
        DiskInode* di = (DiskInode*)buffer;

        FSNode* parent = root_node;
        if (di->parent_index != -1 && di->parent_index < MAX_NODES) {
            if (loaded_nodes[di->parent_index] != 0) {
                parent = loaded_nodes[di->parent_index];
            }
        }
        
        loaded_nodes[i]->parent = parent;
        if (parent->child_count < MAX_CHILDREN) {
            parent->children[parent->child_count++] = loaded_nodes[i];
        }
    }
    print("FS: Systeem hersteld!\n", 0x0A);
}
