#include "ata.h"

#define STATUS_BSY 0x80
#define STATUS_DRQ 0x08

static void ata_wait_ready() {
    while (port_read(0x1F7) & STATUS_BSY);
    while (!(port_read(0x1F7) & STATUS_DRQ));
}

static void ata_wait_bsy() {
    while (port_read(0x1F7) & STATUS_BSY);
}

static void ata_wait_drq() {
    while (!(port_read(0x1F7) & STATUS_DRQ));
}

void ata_write_sector(uint32_t lba, uint8_t *buffer) {
    ata_wait_bsy();

    port_write(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    port_write(0x1F2, 1);
    port_write(0x1F3, (uint8_t)lba);
    port_write(0x1F4, (uint8_t)(lba >> 8));
    port_write(0x1F5, (uint8_t)(lba >> 16));
    port_write(0x1F7, 0x30);

    ata_wait_drq();

    unsigned short *ptr = (unsigned short *)buffer;
    for (int i = 0; i < 256; i++) {
        __asm__("outw %%ax, %%dx" : : "a"(ptr[i]), "d"(0x1F0));
        for(int j = 0; j < 10; j++) __asm__("nop"); 
    }

    for(int i = 0; i < 4; i++) port_read(0x1F7);
}

void ata_read_sector(uint32_t lba, uint8_t *buffer) {
    ata_wait_bsy();

    port_write(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    port_write(0x1F2, 1);
    port_write(0x1F3, (uint8_t)lba);
    port_write(0x1F4, (uint8_t)(lba >> 8));
    port_write(0x1F5, (uint8_t)(lba >> 16));
    port_write(0x1F7, 0x20);

    ata_wait_drq();

    unsigned short *ptr = (unsigned short *)buffer;
    for (int i = 0; i < 256; i++) {
        ptr[i] = port_read_16(0x1F0);
    }
}
