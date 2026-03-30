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

void main() {
    gdt_install();
    print("GDT done\n", 0x0A);

    pic_remap();
    print("PIC done\n", 0x0A);

    idt_init();
    print("IDT done\n", 0x0A);

    fs_init();
    print("FS done\n", 0x0A);

    commands_init();
    print("Commands done\n", 0x0A);

    uint8_t disk_test[512];
    ata_read_sector(0, disk_test);

    print("ATA Driver geladen. Sector 0 gelezen.\n", 0x0A);

    clear_screen();

    print("Meepz OS Loaded ! Shadow Wizard Money Gang\n", 0x0B);
    print("Meepz:", 0x0E);
    print(current_directory->name, 0x0E);
    print("> ", 0x0E);
}
