#include "vesa.h"

void put_pixel(uint32_t x, uint32_t y, uint32_t color, multiboot_info_t* info) {
    if (x >= info->framebuffer_width || y >= info->framebuffer_height) return;

    // Cast de 64-bit addr expliciet naar een 32-bit unsigned integer, dan naar een pointer
    uint32_t fb_ptr = (uint32_t)info->framebuffer_addr; 
    
    if (fb_ptr == 0) return; // Beveiliging tegen null-pointer

    uint32_t* pixel = (uint32_t*)(fb_ptr + (y * info->framebuffer_pitch) + (x * 4));
    *pixel = color;
}

void draw_rect(int x, int y, int w, int h, uint32_t color, multiboot_info_t* info) {
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            put_pixel(x + j, y + i, color, info);
        }
    }
}
