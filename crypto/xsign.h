#ifndef XSIGN_H
#define XSIGN_H

#include <stdint.h>


void xed25519_sign(uint8_t signature[64],
                   const uint8_t secret_key[32],
                   const uint8_t random[64],
                   const uint8_t *message, size_t message_size);

int xed25519_verify(const uint8_t signature[64],
                    const uint8_t public_key[32],
                    const uint8_t *message, size_t message_size);

#endif
