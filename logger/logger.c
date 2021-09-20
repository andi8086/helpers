#include "logger.h"
#include "../mutex/x_mutex.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>


const char *log_level_prefix[3] = {[LOG_LEVEL_ERROR - 1]   = "ERROR",
                                   [LOG_LEVEL_WARNING - 1] = "WARNING",
                                   [LOG_LEVEL_INFO - 1]    = "INFO"};

#define LOG_LEVEL_PREFIX_MAX_LEN 7

logger_t *logger_init(size_t num, get_time_hook_t gth)
{
        logger_t *logger = (logger_t *)malloc(sizeof(logger_t));
        if (!logger) {
                return NULL;
        }

        logger->log_mem = (log_entry_t *)malloc(sizeof(log_entry_t) * num);
        if (!logger->log_mem) {
                free(logger);
                return NULL;
        }
        logger->log_base     = logger->log_mem;
        logger->log_max      = logger->log_mem + num;
        logger->level        = LOG_LEVEL_NONE;
        logger->logs         = 0;
        logger->get_time     = gth;
        logger->log_prefixes = NULL;

        xmutex_init(&logger->lock);

        return logger;
}


logger_t *logger_init_static(logger_t *logger, log_entry_t *entries, size_t num,
                             get_time_hook_t gth)
{
        logger->log_mem      = entries;
        logger->log_base     = entries;
        logger->log_max      = entries + num;
        logger->level        = LOG_LEVEL_NONE;
        logger->logs         = 0;
        logger->get_time     = gth;
        logger->log_prefixes = NULL;

        xmutex_init(&logger->lock);

        return logger;
}


void logger_set_prefixes(logger_t *logger, const char **log_prefixes)
{
        logger->log_prefixes = log_prefixes;
}


void logger_free(logger_t *logger)
{
        free(logger->log_base);
        free(logger);
}


// two columns, two spaces, new line and null terminator subtracted
static char log_buffer[LOG_MSG_LENGTH - 6];


static void logger_log(logger_t *logger, uint8_t log_level, uint8_t log,
                       char *fmt, va_list ap)
{
        if (logger->level == LOG_LEVEL_NONE) {
                /* logs disabled */
                return;
        }

        if (logger->level < log_level || log_level > LOG_LEVEL_INFO) {
                return;
        }

        if (!(logger->logs & (1ULL << log))) {
                return;
        }

        if (logger->log_mem >= logger->log_max) {
                return;
        }

        xmutex_lock(&logger->lock);
        memset(log_buffer, 0, sizeof(log_buffer));
        /* subtract 5 from length for two cols, two spaces and one new-line */
        vsnprintf(log_buffer,
                  LOG_MSG_LENGTH - strlen(log_level_prefix[log_level - 1]) -
                      strlen(logger->log_prefixes[log]) - 5,
                  fmt, ap);
        /* difference between LOG_MSG_LENGTH - 1 and buffer size is exactly 5 as
         * well */
        snprintf(logger->log_mem->msg, LOG_MSG_LENGTH - 1, "%s: %s: %s\n",
                 log_level_prefix[log_level - 1], logger->log_prefixes[log],
                 log_buffer);
        logger->log_mem->time_stamp = logger->get_time();
        logger->log_mem++;
        xmutex_unlock(&logger->lock);
}


void logger_info(logger_t *logger, uint8_t log, char *fmt, ...)
{
        va_list ap;

        va_start(ap, fmt);
        logger_log(logger, LOG_LEVEL_INFO, log, fmt, ap);
        va_end(ap);
}


void logger_info_v(logger_t *logger, uint8_t log, char *fmt, va_list args)
{
        logger_log(logger, LOG_LEVEL_INFO, log, fmt, args);
}


void logger_warn(logger_t *logger, uint8_t log, char *fmt, ...)
{
        va_list ap;

        va_start(ap, fmt);
        logger_log(logger, LOG_LEVEL_WARNING, log, fmt, ap);
        va_end(ap);
}


void logger_warn_v(logger_t *logger, uint8_t log, char *fmt, va_list args)
{
        logger_log(logger, LOG_LEVEL_WARNING, log, fmt, args);
}


void logger_err(logger_t *logger, uint8_t log, char *fmt, ...)
{
        va_list ap;

        va_start(ap, fmt);
        logger_log(logger, LOG_LEVEL_ERROR, log, fmt, ap);
        va_end(ap);
}


void logger_err_v(logger_t *logger, uint8_t log, char *fmt, va_list args)
{
        logger_log(logger, LOG_LEVEL_ERROR, log, fmt, args);
}


void logger_enable_log(logger_t *logger, uint8_t log)
{
        xmutex_lock(&logger->lock);
        logger->logs |= (1ULL << log);
        xmutex_unlock(&logger->lock);
}


void logger_disable_log(logger_t *logger, uint8_t log)
{
        xmutex_lock(&logger->lock);
        logger->logs &= ~(1ULL << log);
        xmutex_unlock(&logger->lock);
}


void logger_enable_all(logger_t *logger)
{
        xmutex_lock(&logger->lock);
        logger->logs = (1ULL << (LOGS_MAX - 1)) - 1;
        xmutex_unlock(&logger->lock);
}


void logger_disable_all(logger_t *logger)
{
        xmutex_lock(&logger->lock);
        logger->logs = 0;
        xmutex_unlock(&logger->lock);
}


void logger_set_level(logger_t *logger, uint8_t log_level)
{
        xmutex_lock(&logger->lock);
        if (log_level > 3) {
                logger->level = 3;
        }
        logger->level = log_level;
        xmutex_unlock(&logger->lock);
}


uint8_t logger_get_level(logger_t *logger)
{
        return logger->level;
}
