/*
 * Base64 encoding/decoding (RFC1341)
 * Copyright (c) 2005-2011, Jouni Malinen <j@w1.fi>
 * Copyright (c) 2023, Andreas J. Reichel <andreas@reichel.bayern>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */
#ifndef H_BASE64_H
#define H_BASE64_H

#include <stddef.h>


typedef void *(*base64_malloc_t)(size_t);
typedef void (*base64_free_t)(void *);

void base64_set_malloc_free(base64_malloc_t mall, base64_free_t mfre);

unsigned char *base64_encode(const unsigned char *src, size_t len,
                             size_t *out_len);
unsigned char *base64_decode(const unsigned char *src, size_t len,
                             size_t *out_len);

#endif
