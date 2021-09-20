// Implement arc4random_buf with Windows Crypto Functions
#include <windows.h>
#include "arc4random.h"

void arc4random_buf(char *dst, size_t size)
{
        HCRYPTPROV h;

        BOOL res = CryptAcquireContext(&h, NULL, NULL, PROV_RSA_FULL, 0);
        if (!res) {
                if (GetLastError() == NTE_BAD_KEYSET) {
                        res = CryptAcquireContext(&h, NULL, NULL, PROV_RSA_FULL,
                                                  CRYPT_NEWKEYSET);
                        if (!res) {
                                /* what now? */
                                h = NULL;
                        }
                }
        }

        res = CryptGenRandom(h, size, dst);

        CryptReleaseContext(h, 0);
}
