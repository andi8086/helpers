#ifndef NET_CRYPTO_H
#define NET_CRYPTO_H

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "net_api.h"

#if defined(HAVE_BSD_STDLIB)
#        include <bsd/stdlib.h>
#else
#        include "arc4random.h"
#endif
#include <monocypher.h>


#define CRYPT_KEY_SIZE 32

/* The following should be allocable on stack and must
 * be small enough to be transmitted by TCP/IP in one go! */
#define CRYPT_DATA_BLOCK_SIZE 4096


#define NET_CRYPTO_INCOMPLETE_PACKET -75536
#define NET_CRYPTO_UNLOCK_FAILED     -75537
#define NET_CRYPTO_MLOCK_FAILED      -75539

typedef struct {
        uint8_t ssec[CRYPT_KEY_SIZE];
        uint8_t send_recv_key[CRYPT_KEY_SIZE];
        uint8_t sign_key[CRYPT_KEY_SIZE];
} net_crypt_ctx_t;

int net_diffie_hellman_x25519(net_api_t *n, net_crypt_ctx_t *ctx);
ssize_t net_send_single_crypto(net_api_t *n, net_crypt_ctx_t *ctx,
                               const char *src, size_t num_bytes);
ssize_t net_recv_single_crypto(net_api_t *n, net_crypt_ctx_t *ctx,
                               const char *dst, size_t num_bytes);

#endif
