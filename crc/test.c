#include "crc.h"

#include <string.h>
#include <stdio.h>

int error_msg(int err)
{
        if (err) {
                printf("crc32 test failed!\n");
        } else {
                printf("crc32 test succeeded!\n");
        }
        return err;
}


int main(void)
{
        return error_msg(crc32_edb88320("123456789", strlen("123456789")) ==
                                 0xCBF43926
                             ? 0
                             : -1);
}
