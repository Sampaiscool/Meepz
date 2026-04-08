#ifndef VESA_H
#define VESA_H

#include "../multiboot.h"

void put_pixel(uint32_t x, uint32_t y, uint32_t color, multiboot_info_t* info);
void draw_rect(int x, int y, int w, int h, uint32_t color, multiboot_info_t* info);

#endif
