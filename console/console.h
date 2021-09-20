#ifndef HELPERS_CONSOLE
#define HELPERS_CONSOLE


#include <stdint.h>


void console_set_unbuffered(void);
void console_win32_enable_ansi(void);
void console_clear();
void console_color(uint8_t r, uint8_t g, uint8_t b);
void console_bgcolor(uint8_t r, uint8_t g, uint8_t b);
void console_goto(uint8_t row, uint8_t col);

#endif
