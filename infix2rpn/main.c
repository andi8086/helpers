#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "infix2rpn.h"


char buffer[1024];

int main(int argc, char **argv)
{
        if (argc != 2) {
                printf("Syntax: %s infix-expression\n", argv[0]);
                return EXIT_FAILURE;
        }
        int res = infix_to_rpn(argv[1], buffer, sizeof(buffer));
        printf("%s\n", buffer);
        printf("Return value: %d\n", res);
        return EXIT_SUCCESS;
}
