#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "../base64.h"


#define BUFF_SIZE 4096

char input_buffer[BUFF_SIZE];


int main(void)
{
        for (size_t i = 0; i < BUFF_SIZE; i++) {
                input_buffer[i] = rand() % 256;
        }

        /* encode */

        size_t out_len;
        char *output_buffer = base64_encode(input_buffer, BUFF_SIZE, &out_len);

        if (!output_buffer) {
                fprintf(stderr, "Out of memory in base64_encode\n");
                return EXIT_FAILURE;
        }

        /* decode */

        char *decoded = base64_decode(output_buffer, out_len, &out_len);

        if (!decoded) {
                fprintf(stderr, "Out of memory in base64_decode\n");
                return EXIT_FAILURE;
        }

        if (out_len != BUFF_SIZE) {
                fprintf(stderr, "Decoded data has wrong size\n");
                return EXIT_FAILURE;
        }

        free(output_buffer);

        if (memcmp(decoded, input_buffer, BUFF_SIZE) != 0) {
                free(decoded);
                fprintf(stderr, "Decoded data is wrong\n");
                return EXIT_FAILURE;
        }

        free(decoded);
        fprintf(stdout,
                "base64-test: %u bytes successfully encoded and redecoded.\n",
                BUFF_SIZE);

        return EXIT_SUCCESS;
}
