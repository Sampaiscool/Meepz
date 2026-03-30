#ifndef ATA_H
#define ATA_H

#include "../types.h"

void ata_read_sector(uint32_t lba, uint8_t *buffer);
void ata_write_sector(uint32_t lba, uint8_t *buffer);

extern void port_write(unsigned short port, unsigned char data);
extern unsigned char port_read(unsigned short port);
extern unsigned short port_read_16(unsigned short port);

#endif
