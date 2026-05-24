#ifndef MAPS_OS_CONSOLE_H
#define MAPS_OS_CONSOLE_H

#include "maps_os/types.h"

void console_init(void);
void console_putc(char ch);
void console_write(const char* data, u32 len);
void console_write_cstr(const char* text);

#endif // MAPS_OS_CONSOLE_H

