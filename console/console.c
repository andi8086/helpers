#include "console.h"

#include <stdio.h>
#include <stdint.h>

#ifdef _WIN32
#        include <windows.h>
#endif

/* Windows has no line buffered console, thus, it is either fully buffered or
 * not buffered at all. However, if it is fully buffered (default), we see no
 * output in MSYS2/MINGW64 using printf("...\n"), until the the buffer is
 * full. This function just disables the buffering, because we don't want to
 * flush after every single printed line. */
void console_set_unbuffered(void)
{
        setvbuf(stdout, NULL, _IONBF, 0);
        setvbuf(stderr, NULL, _IONBF, 0);
}


void console_win32_enable_ansi(void)
{
#ifdef _WIN32
        /* enable VT100 processing */
        DWORD l_mode;
        HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

        GetConsoleMode(hStdout, &l_mode);
        SetConsoleMode(hStdout, l_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING |
                                    DISABLE_NEWLINE_AUTO_RETURN);
#endif
}


void console_clear()
{
        printf("\033[2J\033[;H");
}


void console_color(uint8_t r, uint8_t g, uint8_t b)
{
        printf("\033[38;2;%d;%d;%dm", r, g, b);
}


void console_bgcolor(uint8_t r, uint8_t g, uint8_t b)
{
        printf("\033[48;2;%d;%d;%dm", r, g, b);
}


void console_goto(uint8_t row, uint8_t col)
{
        printf("\033[%d;%dH", row, col);
}
