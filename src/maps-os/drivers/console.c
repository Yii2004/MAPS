#include "maps_os/console.h"

#define CONSOLE_BASE 0x10000100u
#define CONSOLE_DATA 0x00u
#define CONSOLE_STATUS 0x04u

#define CONSOLE_REG(offset) (*(volatile u32*)(uintptr_t)(CONSOLE_BASE + (offset)))

void console_init(void) {}

void console_putc(char ch) {
    CONSOLE_REG(CONSOLE_DATA) = (u32)(u8)ch;
}

void console_write(const char* data, u32 len) {
    if (data == 0 || len == 0) {
        return;
    }
    for (u32 i = 0; i < len; ++i) {
        console_putc(data[i]);
    }
}

void console_write_cstr(const char* text) {
    u32 len = 0;
    if (text == 0) {
        return;
    }
    while (text[len] != '\0') {
        ++len;
    }
    console_write(text, len);
}
