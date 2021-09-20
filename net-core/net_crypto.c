#include "net_crypto.h"
#include "net_msg.h"

#include <monocypher.h>
#include <string.h>

#define PLATFORM_LINUX 1
#define PLATFORM_WIN32 2

#if BUILD_PLATFORM == PLATFORM_LINUX
#        include <sys/mman.h>
#elif BUILD_PLATFORM == PLATFORM_WIN32
#        include "mlock_win32.h"
#endif


static void hexdump(uint8_t *data, size_t len)
{
        for (int i = 0; i < len; i++) {
                printf("%02X ", (uint8_t)data[i]);
        }
}


typedef struct {
        uint8_t crypt_data[CRYPT_DATA_BLOCK_SIZE];
        size_t size;
        uint8_t mac[16];
        uint8_t nonce[24];
} enc_data_t;


static void net_encrypt_x25519(net_api_t *n, net_crypt_ctx_t *ctx,
                               enc_data_t *data)
{
        /* generate a new nonce for every block */
        arc4random_buf(data->nonce, sizeof(data->nonce));
        crypto_aead_lock(data->crypt_data, data->mac, ctx->send_recv_key,
                         data->nonce, NULL, 0, data->crypt_data, data->size);
}


static int net_decrypt_x25519(net_api_t *n, net_crypt_ctx_t *ctx,
                              enc_data_t *data)
{
        int res;
        res = crypto_aead_unlock(data->crypt_data, data->mac,
                                 ctx->send_recv_key, data->nonce, NULL, 0,
                                 data->crypt_data, data->size);
        return res;
}


ssize_t net_send_single_crypto(net_api_t *n, net_crypt_ctx_t *ctx,
                               const char *src, size_t num_bytes)
{
        enc_data_t data;
        memcpy(data.crypt_data, src, num_bytes);
        data.size = num_bytes;
        net_encrypt_x25519(n, ctx, &data);
        ssize_t bytes = net_send_packet(n, (char *)&data, sizeof(data), 0);
        return bytes;
}


ssize_t net_recv_single_crypto(net_api_t *n, net_crypt_ctx_t *ctx,
                               const char *dst, size_t num_bytes)
{
        enc_data_t data;
        int res;
        ssize_t bytes;
        size_t rsize;
        rsize = sizeof(data);

        bytes = net_recv_packet(n, (char *)&data, &rsize, NULL);

        if (bytes <= 0) {
                return NET_CRYPTO_INCOMPLETE_PACKET;
        }
        res = net_decrypt_x25519(n, ctx, &data);
        if (res) {
                return NET_CRYPTO_UNLOCK_FAILED;
        }
        memcpy((void *)dst, data.crypt_data, data.size);
        return data.size;
}


#define DHCMD_ACK 63

int net_diffie_hellman_x25519(net_api_t *n, net_crypt_ctx_t *ctx)
{
        const size_t key_size = sizeof(ctx->send_recv_key);
        struct {
                uint8_t remote_pk[key_size]; /* remote public key */
                uint8_t local_sk[key_size];  /* local secret key */
                uint8_t local_pk[key_size];  /* local public key */
        } tmp_keys;
        uint8_t shared_keys[key_size * 2];
        int res;

        if (mlock(&tmp_keys, sizeof(tmp_keys))) {
                printf("mlock failed\n");
                fflush(stdout);
                return NET_CRYPTO_MLOCK_FAILED;
        }

        if (n->obj_type == NET_SERVER) {
                arc4random_buf(tmp_keys.local_sk, key_size);
                crypto_x25519_public_key(tmp_keys.local_pk, tmp_keys.local_sk);

                /* send local public key */
                res = net_send_stream(n, tmp_keys.local_pk, key_size);
                if (res != key_size) {
                        goto error_munlock_exit;
                }

                uint8_t cmd;
                res = net_recv_stream(n, NULL, &cmd, 1);
                if (cmd != DHCMD_ACK) {
                        goto error_munlock_exit;
                }

                /* retrieve remote pk */
                res = net_recv_stream(n, NULL, (char *)tmp_keys.remote_pk,
                                      key_size);
                if (res != sizeof(tmp_keys.local_pk)) {
                        goto error_munlock_exit;
                }

                crypto_x25519(ctx->ssec, tmp_keys.local_sk, tmp_keys.remote_pk);

                crypto_wipe(tmp_keys.local_sk, key_size);

                mlock(shared_keys, sizeof(shared_keys));

                crypto_blake2b_ctx bctx;
                crypto_blake2b_init(&bctx, key_size * 2);
                crypto_blake2b_update(&bctx, ctx->ssec, key_size);
                crypto_blake2b_update(&bctx, tmp_keys.local_pk, key_size);
                crypto_blake2b_update(&bctx, tmp_keys.remote_pk, key_size);
                crypto_blake2b_final(&bctx, shared_keys);

                memcpy(ctx->send_recv_key, shared_keys, key_size);
                memcpy(ctx->sign_key, shared_keys + key_size, key_size);

                crypto_wipe(shared_keys, key_size * 2);
                munlock(shared_keys, sizeof(shared_keys));
                crypto_wipe(&tmp_keys, sizeof(tmp_keys));
                munlock(&tmp_keys, sizeof(tmp_keys));

                return 0;
        }

        /* Client implementation */
        arc4random_buf(tmp_keys.local_sk, key_size);
        crypto_x25519_public_key(tmp_keys.local_pk, tmp_keys.local_sk);

        /* retrieve remote public key */
        res = net_recv_stream(n, NULL, (char *)tmp_keys.remote_pk, key_size);
        if (res != sizeof(tmp_keys.remote_pk)) {
                goto error_munlock_exit;
        }

        uint8_t cmd = DHCMD_ACK;
        net_send_stream(n, (char *)&cmd, 1);

        /* send local pk */
        res = net_send_stream(n, (char *)tmp_keys.local_pk, key_size);
        if (res != sizeof(tmp_keys.local_pk)) {
                goto error_munlock_exit;
        }

        crypto_x25519(ctx->ssec, tmp_keys.local_sk, tmp_keys.remote_pk);
        crypto_wipe(tmp_keys.local_sk, key_size);

        mlock(shared_keys, sizeof(shared_keys));

        crypto_blake2b_ctx bctx;
        crypto_blake2b_init(&bctx, key_size * 2);
        crypto_blake2b_update(&bctx, ctx->ssec, key_size);
        crypto_blake2b_update(&bctx, tmp_keys.remote_pk, key_size);
        crypto_blake2b_update(&bctx, tmp_keys.local_pk, key_size);
        crypto_blake2b_final(&bctx, shared_keys);

        memcpy(ctx->send_recv_key, shared_keys, key_size);
        memcpy(ctx->sign_key, shared_keys + key_size, key_size);

        crypto_wipe(shared_keys, sizeof(shared_keys));
        munlock(shared_keys, sizeof(shared_keys));
        crypto_wipe(&tmp_keys, sizeof(tmp_keys));
        munlock(&tmp_keys, sizeof(tmp_keys));

        return 0;

error_munlock_exit:
        crypto_wipe(&tmp_keys, sizeof(tmp_keys));
        munlock(&tmp_keys, sizeof(tmp_keys));
        return -1;
}
