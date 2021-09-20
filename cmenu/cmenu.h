#pragma once

/******************************************************************************
 *
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

#include <stdint.h>
#include "cmenu.h"

typedef struct {
        char *description;
        char *token;
        void (*function)(void *);
} cmenu_entry_t;


typedef struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
} cmenu_color_t;


void cmenu_set_token_color(cmenu_color_t c);
void cmenu_set_text_color(cmenu_color_t c);
cmenu_entry_t *cmenu_get(void);
void cmenu_set(cmenu_entry_t *menu, void *ctx);
void cmenu_display(void);
void cmenu_run(void);
