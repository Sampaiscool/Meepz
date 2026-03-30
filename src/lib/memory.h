#ifndef MEMORY_H
#define MEMORY_H

#include "../types.h"

void* malloc(int size);
void memory_copy(void *src, void *dest, int nbytes);
void memory_set(void *dest, uint8_t val, uint32_t len);

#endif
