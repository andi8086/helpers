#ifndef REGEX_H
#define REGEX_H


#include "../external/pcre2/src/pcre2.h"


typedef struct {
        const char *pattern;
        pcre2_match_data *md;
        pcre2_code *re;
} preg_match_ctx_t;


void preg_match_end(preg_match_ctx_t *ctx);
char *preg_match(preg_match_ctx_t *ctx, char *subject, const char *pattern);

#endif
