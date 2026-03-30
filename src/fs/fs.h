#ifndef FS_H
#define FS_H

#include "../types.h"

#define MAX_NODES 64

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

typedef struct {
    uint32_t magic;
    uint32_t total_inodes;
    uint32_t first_data_sec;
} __attribute__((packed)) Superblock;

typedef struct {
    char name[32];
    uint32_t size;
    uint32_t type;
    uint32_t start_sector; 
    int32_t parent_index;
    uint8_t present;       
} __attribute__((packed)) DiskInode;

#define MEEP_MAGIC 0x1337BEEF

extern FSNode *current_directory;
extern FSNode *root_node;

FSNode* create_node(char *name, NodeType type);
int str_step_compare(char *str1, char *str2);
void strncpy_custom(char *dest, const char *src, int n);

void list_files();
void change_directory(char *target_name);
void make_directory(char *name);
void make_file(char *name);
void remove_node(char *name);
void cat_file(char *name);
void write_to_file(char *input);
int has_valid_extension(char *name);
int is_name_blocked(char *name);

void fs_init();
void fs_save_to_disk();
void fs_load_from_disk();

#endif
