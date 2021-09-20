#include <string.h>

#include "regex.h"


void preg_match_end(preg_match_ctx_t *ctx)
{
        if (ctx->md) {
                pcre2_match_data_free(ctx->md);
        }
        if (ctx->re) {
                pcre2_code_free(ctx->re);
        }
}


char *preg_match(preg_match_ctx_t *ctx, char *subject, const char *pattern)
{
        PCRE2_SIZE erroroffset;
        int errorcode;

        if (!ctx->re || (ctx->re && pattern != ctx->pattern)) {
                ctx->pattern = pattern;
                if (ctx->re) {
                        pcre2_code_free(ctx->re);
                }
                ctx->re = pcre2_compile(pattern, PCRE2_ZERO_TERMINATED, 0,
                                        &errorcode, &erroroffset, NULL);
        }

        if (!ctx->md) {
                ctx->md = pcre2_match_data_create(1, NULL);
        }

        int rc = pcre2_match(ctx->re, subject, PCRE2_ZERO_TERMINATED, 0, 0,
                             ctx->md, NULL);


        if (rc <= 0) {
                return NULL;
        }

        PCRE2_SIZE *ovector;
        ovector = pcre2_get_ovector_pointer(ctx->md);

        PCRE2_SPTR ss_start = subject + ovector[0];
        PCRE2_SIZE ss_len   = ovector[1] - ovector[0];

        char *ret_str = malloc(ss_len + 1);

        if (!ret_str) {
                goto preg_match_enomem;
        }
        memcpy(ret_str, ss_start, ss_len);
        ret_str[ss_len] = '\0';
        return ret_str;
preg_match_enomem:
        pcre2_match_data_free(ctx->md);
        pcre2_code_free(ctx->re);

        return ret_str;
}
