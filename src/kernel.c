#include "multiboot.h"
#include "cpu/idt.h"
#include "cpu/gdt.h"
#include "lib/screen.h"
#include "lib/memory.h"
#include "fs/fs.h"
#include "shell/shell.h"
#include "shell/commands.h"
#include "shell/editor.h"
#include "drivers/keyboard.h"
#include "drivers/ata.h"
#include "drivers/vesa.h"

void main(uint32_t magic, multiboot_info_t* info) {
    //gdt_install();

    //pic_remap();

    //idt_init();

    //fs_init();

    //commands_init();

    //uint8_t disk_test[512];
    //ata_read_sector(0, disk_test);

    // Check of we echt in grafische modus zitten
    if (info->flags & (1 << 12)) {
        // Teken een blauw scherm als test (0x0000FF)
        draw_rect(0, 0, 800, 600, 0x0000FF, info);
        // Teken een rood vierkant in het midden
        draw_rect(350, 250, 100, 100, 0xFF0000, info);
    }

    while(1) { __asm__ volatile ("hlt"); }
}
