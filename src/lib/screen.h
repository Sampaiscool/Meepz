#ifndef SCREEN_H
#define SCREEN_H

void port_write(unsigned short port, unsigned char data);
unsigned char port_read(unsigned short port);

void clear_screen();
void print(char *string, char hex);
void scroll_if_needed();
void update_cursor(int pos);
void refresh_line(char *buffer, int buf_idx);

extern int cursor;

#endif
