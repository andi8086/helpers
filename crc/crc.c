#include "crc.h"


uint32_t crc32_edb88320(const char *s, size_t n)
{
        uint32_t crc = UINT32_MAX;
        /* reverse poly, LSB first (normal = 0x04C11DB7) */
        const uint32_t poly = 0xEDB88320;

        for (size_t i = 0; i < n; i++) {
                char ch = s[i];
                for (size_t j = 0; j < 8; j++) {
                        uint32_t b = (ch ^ crc) & 1;
                        crc >>= 1;
                        if (b) {
                                crc = crc ^ poly;
                        }
                        ch >>= 1;
                }
        }
        return ~crc;
}
