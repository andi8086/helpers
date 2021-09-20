#ifndef COMPRESSION_H
#define COMPRESSION_H

#include <stddef.h>
#include <zlib.h>

int zlib_inflate(char *buffer, size_t size, char *decompressed);
int zlib_deflate(char *buffer, size_t size, char *compressed,
                 size_t *compressed_size);


#endif
