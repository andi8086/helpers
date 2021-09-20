#ifndef HELPERS_LOGGER_H
#define HELPERS_LOGGER_H

#define LOG_L_ERR  (1ULL << 0)
#define LOG_L_WARN (1ULL << 1)
#define LOG_L_INFO (1ULL << 2)
#define LOG_L_ALL  (LOG_LEVEL_INFO | LOG_LEVEL_WARN | LOG_LEVEL_ERR)

#define LOGS_MAX 64

#define LOG_LEVEL_NONE    0
#define LOG_LEVEL_ERROR   1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_INFO    3

#define LOG_MSG_LENGTH 128


#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "../mutex/xmutex.h"


typedef struct {
        uint64_t time_stamp;
        char msg[128];
} log_entry_t;


typedef uint64_t (*get_time_hook_t)(void);


typedef struct {
        xmutex_t lock;
        log_entry_t *log_base;
        log_entry_t *log_mem;
        log_entry_t *log_max;
        get_time_hook_t get_time;
        uint8_t level;
        uint64_t logs;
        const char **log_prefixes;
} logger_t;


logger_t *logger_init(size_t num, get_time_hook_t gth);
logger_t *logger_init_static(logger_t *logger, log_entry_t *entries, size_t num,
                             get_time_hook_t gth);
void logger_set_prefixes(logger_t *logger, const char **log_prefixes);
void logger_free(logger_t *logger);
void logger_info(logger_t *logger, uint8_t log, char *fmt, ...);
void logger_warn(logger_t *logger, uint8_t log, char *fmt, ...);
void logger_err(logger_t *logger, uint8_t log, char *fmt, ...);
void logger_info_v(logger_t *logger, uint8_t log, char *fmt, va_list args);
void logger_warn_v(logger_t *logger, uint8_t log, char *fmt, va_list args);
void logger_err_v(logger_t *logger, uint8_t log, char *fmt, va_list args);

void logger_enable_log(logger_t *logger, uint8_t log);
void logger_disable_log(logger_t *logger, uint8_t log);
void logger_enable_all(logger_t *logger);
void logger_disable_all(logger_t *logger);
void logger_set_level(logger_t *logger, uint8_t log_level);
uint8_t logger_get_level(logger_t *logger);


#endif
