/*****************************************************************************
 * CMenu - Simple Menu System for console applications
 * v1.0
 * Author: Andreas J. Reichel <andreas@reichel.bayern>
 * MIT License
 *
 * Copyright (c) 2020 Andreas J. Reichel
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cmenu.h"
#include "../console/console.h"


static cmenu_entry_t *current_menu = 0;
static void *current_ctx           = 0;
static cmenu_color_t token_color   = {255, 255, 255};
static cmenu_color_t text_color    = {192, 192, 192};


void cmenu_set_token_color(cmenu_color_t c)
{
        token_color.r = c.r;
        token_color.g = c.g;
        token_color.b = c.b;
}


void cmenu_set_text_color(cmenu_color_t c)
{
        text_color.r = c.r;
        text_color.g = c.g;
        text_color.b = c.b;
}


cmenu_entry_t *cmenu_get(void)
{
        return current_menu;
}


void cmenu_set(cmenu_entry_t *menu, void *ctx)
{
        current_menu = menu;
        current_ctx  = ctx;
}


void cmenu_display()
{
        if (!current_menu) {
                return;
        }

        cmenu_entry_t *p = current_menu;
        char *d;
        int col = 0;

        for (int i = -15; i < 16; i++) {
                console_color(0, 0, 255 - (i * i));
                printf("==");
        }
        printf("\n");

        do {
                d = p->description;
                if (!d || !p->token) {
                        break;
                };
                if (*d == '-') {
                        do {
                                printf("\n");
                                col = 0;
                        } while (*(++d) == '-');
                } else {
                        console_color(token_color.r, token_color.g,
                                      token_color.b);
                        printf("[%s]\t", p->token);
                        console_color(text_color.r, text_color.g, text_color.b);
                        printf("%s\t\t", p->description);
                        col++;
                }
                if (col % 2 == 0 && col != 0) {
                        printf("\n");
                }
        } while ((++p)->description);

        printf("\n");
        for (int i = -15; i < 16; i++) {
                console_color(0, 0, 255 - (i * i));
                printf("==");
        }
        printf("\n");
        console_color(text_color.r, text_color.g, text_color.b);
        printf(">");
}


void cmenu_run(void)
{
        char entry[10];
        cmenu_entry_t *p = current_menu;

        if (!p || !p->token) {
                return;
        }

        cmenu_display();
        scanf("%9s", entry);

        do {
                if (strlen(p->token) == 0) {
                        continue;
                }

                if (strncmp(p->token, entry, strlen(p->token) + 1) == 0) {
                        if (p->function) {
                                p->function(current_ctx);
                        }
                }
        } while ((++p)->token);
}
