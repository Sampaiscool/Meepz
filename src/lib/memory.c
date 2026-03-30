#include "memory.h"

unsigned char* heap_pointer = (unsigned char*)0x200000;

void* malloc(int size) {
    void* res = (void*)heap_pointer;
    heap_pointer += size;

    unsigned char* p = (unsigned char*)res;
    for(int i = 0; i < size; i++) p[i] = 0;

    return res;
}

void memory_copy(void *src, void *dest, int nbytes) {
    char *s = (char *)src;
    char *d = (char *)dest;
    for (int i = 0; i < nbytes; i++) {
        d[i] = s[i];
    }
}

void memory_set(void *dest, uint8_t val, uint32_t len) {
    uint8_t *temp = (uint8_t *)dest;
    for ( ; len != 0; len--) *temp++ = val;
}
