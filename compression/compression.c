#include "compression.h"

#include <zlib.h>
#include <stdlib.h>


#define CHUNK 262144


int zlib_deflate(char *buffer, size_t size, char *compressed,
                 size_t *compressed_size)
{
        size_t where = 0, dest = 0;

        int ret, flush;
        unsigned have;
        z_stream strm;
        unsigned char in[CHUNK];
        unsigned char out[CHUNK];

        strm.zalloc = Z_NULL;
        strm.zfree  = Z_NULL;
        strm.opaque = Z_NULL;
        ret         = deflateInit(&strm, Z_DEFAULT_COMPRESSION);

        if (ret != Z_OK) {
                return ret;
        }

        do {
                if (where + CHUNK < size) {
                        memcpy(in, buffer + where, CHUNK);
                        strm.avail_in = CHUNK;
                        where += CHUNK;
                } else if (where == size - 1) {
                        strm.avail_in = 0;
                } else {
                        memcpy(in, buffer + where, size - where);
                        strm.avail_in = size - where;
                        where         = size - 1;
                }
                flush        = where == size - 1 ? Z_FINISH : Z_NO_FLUSH;
                strm.next_in = in;

                do {
                        strm.avail_out = CHUNK;
                        strm.next_out  = out;

                        ret = deflate(&strm, flush);
                        /* assert(ret != Z_STREAM_ERROR); */

                        have = CHUNK - strm.avail_out;
                        memcpy(compressed + dest, out, have);
                        dest += have;
                } while (strm.avail_out == 0);
        } while (flush != Z_FINISH);

        (void)deflateEnd(&strm);
        *compressed_size = dest;
        return Z_OK;
}


int zlib_inflate(char *buffer, size_t size, char *decompressed)
{
        int ret;
        unsigned int have;
        z_stream strm;
        unsigned char in[CHUNK];
        unsigned char out[CHUNK];
        size_t where  = 0;
        size_t dest   = 0;
        strm.zalloc   = Z_NULL;
        strm.zfree    = Z_NULL;
        strm.opaque   = Z_NULL;
        strm.avail_in = 0;
        strm.next_in  = Z_NULL;
        ret           = inflateInit(&strm);

        if (ret != Z_OK) {
                return ret;
        }

        do {
                if (where + CHUNK < size) {
                        memcpy(in, buffer + where, CHUNK);
                        strm.avail_in = CHUNK;
                        where += CHUNK;
                } else if (where == size - 1) {
                        strm.avail_in = 0;
                        break;
                } else {
                        memcpy(in, buffer + where, size - where);
                        strm.avail_in = size - where;
                        where         = size - 1;
                }
                strm.next_in = in;

                do {
                        strm.avail_out = CHUNK;
                        strm.next_out  = out;

                        ret = inflate(&strm, Z_NO_FLUSH);
                        /* assert(ret != Z_STREAM_ERROR) */
                        switch (ret) {
                        case Z_NEED_DICT: ret = Z_DATA_ERROR;
                        case Z_DATA_ERROR:
                        case Z_MEM_ERROR: (void)inflateEnd(&strm); return ret;
                        }
                        have = CHUNK - strm.avail_out;
                        memcpy(decompressed + dest, out, have);
                        dest += have;
                } while (strm.avail_out == 0);
        } while (ret != Z_STREAM_END);

        (void)inflateEnd(&strm);
        return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}
