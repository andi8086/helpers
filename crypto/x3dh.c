#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <monocypher.h>
#include <optional/monocypher-ed25519.h>

#include "xsign.h"

#define KEY_SIZE 32

uint8_t PIKA[KEY_SIZE];
uint8_t SIKA[KEY_SIZE];
uint8_t PEKA[KEY_SIZE];
uint8_t SEKA[KEY_SIZE];

uint8_t PIKB[KEY_SIZE];
uint8_t SIKB[KEY_SIZE];
uint8_t PSPKB[KEY_SIZE];
uint8_t SigXPSPKB[KEY_SIZE*2];
uint8_t SSPKB[KEY_SIZE];
uint8_t POPKB[KEY_SIZE];
uint8_t SOPKB[KEY_SIZE];


void hkdf(uint8_t *out, uint8_t *in, size_t insize)
{
        uint8_t domain_sep[32];
        uint8_t salt[32];

        memset(domain_sep, 0xFF, 32);
        memset(salt, 0, 32);

        crypto_sha512_hkdf(out, 32,
                           in, insize,
                           salt, 32,
                           domain_sep, 32);
}


static void hexdump(uint8_t *data, size_t len)
{
        for (int i = 0; i < len; i++) {
                printf("%02X ", (uint8_t)data[i]);
        }
}


int test_main(void)
{
        char buffer[64];
        /* B generates identity key and prekeys and publishes them */

        /* B generates identity key IKB */
        arc4random_buf(SIKB, KEY_SIZE);
        crypto_x25519_public_key(PIKB, SIKB);

        /* B generates signed prekey SPKB */
        arc4random_buf(SSPKB, KEY_SIZE);
        crypto_x25519_public_key(PSPKB, SSPKB);

        /* TODO: don't use PSPKB as the message to sign directly here */
        arc4random_buf(buffer, sizeof(buffer));
        xed25519_sign(SigXPSPKB, SSPKB, buffer, PSPKB, sizeof(PSPKB));

        /* B generates one-time prekey */
        arc4random_buf(SOPKB, KEY_SIZE);
        crypto_x25519_public_key(POPKB, SOPKB);

/*********************************/
        /* A generates her identity key */
        arc4random_buf(SIKA, KEY_SIZE);
        crypto_x25519_public_key(PIKA, SIKA);

        /* A generates her ephemeral key */
        arc4random_buf(SEKA, KEY_SIZE);
        crypto_x25519_public_key(PEKA, SEKA);

        /*********************************/

        /* Alice checks signature of Bobs PSPKB */
        if (xed25519_verify(SigXPSPKB, PSPKB, PSPKB, sizeof(PSPKB))) {
                printf("Bobs identify could not be verified!\n");
        }

        /* Alice calculates DH1...DH4 */
        uint8_t DH1[KEY_SIZE];
        uint8_t DH2[KEY_SIZE];
        uint8_t DH3[KEY_SIZE];
        uint8_t DH4[KEY_SIZE];

        crypto_x25519(DH1, SIKA, PSPKB);
        crypto_x25519(DH2, SEKA, PIKB);
        crypto_x25519(DH3, SEKA, PSPKB);
        crypto_x25519(DH4, SEKA, POPKB);

        uint8_t comb[KEY_SIZE*4];
        memcpy(comb, DH1, KEY_SIZE);
        memcpy(comb+KEY_SIZE, DH2, KEY_SIZE);
        memcpy(comb+2*KEY_SIZE, DH3, KEY_SIZE);
        memcpy(comb+3*KEY_SIZE, DH4, KEY_SIZE);

        uint8_t SK[32];
        hkdf(SK, comb, sizeof(comb));

        crypto_wipe(SEKA, sizeof(SEKA));
        crypto_wipe(DH1, sizeof(DH1));
        crypto_wipe(DH2, sizeof(DH2));
        crypto_wipe(DH3, sizeof(DH3));
        crypto_wipe(DH4, sizeof(DH4));

        printf("\nAlice got SK: \n");
        hexdump(SK, sizeof(SK));
        uint8_t AD[KEY_SIZE];
        memcpy(comb, PIKA, sizeof(PIKA));
        memcpy(comb+KEY_SIZE, PIKB, sizeof(PIKB));
        hkdf(AD, comb, KEY_SIZE*2);
        printf("\nAlice got AD: \n");
        hexdump(AD, sizeof(AD));

        char first_msg[16];
        snprintf(first_msg, 16, "Hallo!\n");
        uint8_t mac[16];
        uint8_t nonce[24];
        uint8_t cipher[16];
        arc4random_buf(nonce, 24);
        crypto_aead_lock(cipher, mac, SK, nonce,
                         AD, sizeof(AD),
                         first_msg, sizeof(first_msg));

        /* Bob receives... */
        crypto_x25519(DH1, SSPKB, PIKA);
        crypto_x25519(DH2, SIKB, PEKA);
        crypto_x25519(DH3, SSPKB, PEKA);
        crypto_x25519(DH4, SOPKB, PEKA);

        memcpy(comb, DH1, KEY_SIZE);
        memcpy(comb+KEY_SIZE, DH2, KEY_SIZE);
        memcpy(comb+2*KEY_SIZE, DH3, KEY_SIZE);
        memcpy(comb+3*KEY_SIZE, DH4, KEY_SIZE);

        crypto_wipe(SK, sizeof(SK));
        hkdf(SK, comb, sizeof(comb));
        printf("\nBob got SK: \n");
        hexdump(SK, sizeof(SK));

        memcpy(comb, PIKA, sizeof(PIKA));
        memcpy(comb+KEY_SIZE, PIKB, sizeof(PIKB));
        hkdf(AD, comb, KEY_SIZE*2);
        printf("\nBob got AD: \n");
        hexdump(AD, sizeof(AD));

        uint8_t decrypted[16];
        crypto_aead_unlock(decrypted, mac, SK, nonce,
                           AD, sizeof(AD),
                           cipher, sizeof(cipher));

        printf("%s\n", decrypted);

        return 0;
}
