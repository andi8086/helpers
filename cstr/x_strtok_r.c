#include "x_strtok_r.h"

#include <string.h>

#define STRTOK_R_CHECK(s, b)   \
        if (*s == '\0') {      \
                *save_ptr = s; \
                return b;      \
        }


char *x_strtok_r(char *s, const char *delim, char **save_ptr)
{
        char *end;

        s = s ? s : *save_ptr;
        STRTOK_R_CHECK(s, NULL)

        s += strspn(s, delim);
        STRTOK_R_CHECK(s, NULL)

        end = s + strcspn(s, delim);
        STRTOK_R_CHECK(end, s);

        *end      = '\0';
        *save_ptr = end + 1;
        return s;
}


#undef STRTOK_R_CHECK
