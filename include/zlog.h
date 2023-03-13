#ifndef __ZLOG_H__
#define __ZLOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "zlog_ports.h"
#include "zlog_config.h"

#define ZLOG_LVL_TOTAL_NUM                   6

#define ZLOG_FILTER_LVL_SILENT               ZLOG_LVL_ASSERT
#define ZLOG_FILTER_LVL_ALL                  ZLOG_LVL_VERBOSE

#define ZLOG_DUMP_WIDTH_NUM                  10

#ifdef ZLOG_ASSERT_ENABLE
    #define ZLOG_ASSERT(EXPR)                                                 \
    if (!(EXPR))                                                              \
    {                                                                         \
        if (zlog_assert_hook == NULL) {                                       \
            zlog_error("elog", "(%s) has assert failed at %s:%ld.", #EXPR, __FUNCTION__, __LINE__); \
            while (1);                                                        \
        } else {                                                              \
            zlog_assert_hook(#EXPR, __FUNCTION__, __LINE__);                  \
        }                                                                     \
    }
#else
    #define ZLOG_ASSERT(EXPR)                    ((void)0);
#endif

typedef enum {
    ZLOG_NO_ERR,
} zlog_status;

typedef enum {
    ZLOG_FORMAT_LEVEL           = 1 << 0,       /*level*/
    ZLOG_FORMAT_TAG             = 1 << 1,       /*tag*/
    ZLOG_FORMAT_TIME            = 1 << 2,       /*current time*/
    ZLOG_FORMAT_P_INFO          = 1 << 3,       /*process info*/
    ZLOG_FORMAT_I_INFO          = 1 << 4,       /*thread info*/
    ZLOG_FORMAT_DIR             = 1 << 5,       /*file directory and name*/
    ZLOG_FORMAT_FUNC            = 1 << 6,       /*function name*/
    ZLOG_FORMAT_LINE            = 1 << 7,       /*line number*/
} zlog_format_index;

#define ZLOG_FORMAT_ALL (ZLOG_FORMAT_LEVEL|ZLOG_FORMAT_TAG|ZLOG_FORMAT_TIME|ZLOG_FORMAT_P_INFO|ZLOG_FORMAT_I_INFO| \
                         ZLOG_FORMAT_DIR|ZLOG_FORMAT_FUNC|ZLOG_FORMAT_LINE)

#define ZLOG_A(tag, ...)        zlog_assert(tag, __VA_ARGS__)
#define ZLOG_E(tag, ...)        zlog_error(tag, __VA_ARGS__)
#define ZLOG_W(tag, ...)        zlog_warn(tag, __VA_ARGS__)
#define ZLOG_I(tag, ...)        zlog_info(tag, __VA_ARGS__)
#define ZLOG_D(tag, ...)        zlog_debug(tag, __VA_ARGS__)
#define ZLOG_V(tag, ...)        zlog_verbose(tag, __VA_ARGS__)

#ifndef ZLOG_OUTPUT_ENABLE
    #define zlog_assert(tag, ...)
    #define zlog_error(tag, ...)
    #define zlog_warn(tag, ...)
    #define zlog_info(tag, ...)
    #define zlog_debug(tag, ...)
    #define zlog_verbose(tag, ...)
    #define zlog_dump(tag, buf, size)
#else
#if ZLOG_OUTPUT_LVL >= ZLOG_LVL_ASSERT
    #define zlog_assert(tag, ...)   \
            zlog_output(ZLOG_LVL_ASSERT, tag, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
    #define zlog_assert(tag, ...)
#endif

#if ZLOG_OUTPUT_LVL >= ZLOG_LVL_ERROR
    #define zlog_error(tag, ...)   \
            zlog_output(ZLOG_LVL_ERROR, tag, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
    #define zlog_error(tag, ...)
#endif

#if ZLOG_OUTPUT_LVL >= ZLOG_LVL_WARN
    #define zlog_warn(tag, ...)   \
            zlog_output(ZLOG_LVL_WARN, tag, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
    #define zlog_warn(tag, ...)
#endif

#if ZLOG_OUTPUT_LVL >= ZLOG_LVL_INFO
    #define zlog_info(tag, ...)   \
            zlog_output(ZLOG_LVL_INFO, tag, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
    #define zlog_info(tag, ...)
#endif

#if ZLOG_OUTPUT_LVL >= ZLOG_LVL_DEBUG
    #define zlog_debug(tag, ...)   \
            zlog_output(ZLOG_LVL_DEBUG, tag, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
    #define zlog_debug(tag, ...)
#endif

#if ZLOG_OUTPUT_LVL >= ZLOG_LVL_VERBOSE
    #define zlog_verbose(tag, ...)   \
            zlog_output(ZLOG_LVL_VERBOSE, tag, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
    #define zlog_verbose(tag, ...)
#endif

#if ZLOG_OUTPUT_LVL >= ZLOG_LVL_VERBOSE
    #define zlog_dump(tag, buf, size)   \
            zlog_hexdump(tag, ZLOG_DUMP_WIDTH_NUM, buf, size)
#else
    #define zlog_dump(tag, buf, size)
#endif

#endif

/*log's tag filter*/
typedef struct _zlog_tag_level_filter {
    uint8_t level;
    char tag[ZLOG_FILTER_TAG_MAX_LEN + 1];
    bool tag_use_flag;                                /*false: tag is no used, true: tag is used*/
} zlog_tag_level_filter, *zlog_tag_level_filter_t;

/*log's filter*/
typedef struct _zlog_filter {
    uint8_t level;
    char tag[ZLOG_FILTER_TAG_MAX_LEN + 1];
    char keyword[ZLOG_FILTER_KEYWORD_MAX_LEN + 1];
    zlog_tag_level_filter tag_level[ZLOG_FILTER_TAG_LEVEL_MAX_NUM];
} zlog_filter, *zlog_filter_t;

typedef struct _zlog_handler {
    zlog_filter filter;
    uint32_t enabled_format_set[ZLOG_LVL_TOTAL_NUM];
    bool init_ok;
    bool output_enable;
} zlog_handler, *zlog_handler_t;

extern void (*zlog_assert_hook)(const char* expr, const char* func, uint32_t line);

void zlog_assert_set_hook(void (*hook)(const char* expr, const char* func, uint32_t line));

zlog_status zlog_init(void);

void zlog_deinit(void);

void zlog_start(void);

void zlog_output(uint8_t level, const char *tag, const char *file, const char *func,
                 const long line, const char *format, ...);

void zlog_raw(const char *format, ...);

void zlog_hexdump(const char *name, uint8_t width, const void *buf, uint32_t size);

void zlog_set_format(uint8_t level, uint32_t set);

bool zlog_get_format_enabled(uint8_t level, uint32_t set);

void zlog_set_output_enabled(bool enabled);

void zlog_set_filter_tag_level_default(void);

void zlog_set_filter_tag_level(const char *tag, uint8_t level);

uint8_t zlog_get_filter_tag_level(const char *tag);

void zlog_set_filter_level(uint8_t level);

void zlog_set_filter_tag(const char *tag);

void zlog_clear_filter_tag(void);

void zlog_set_filter_keyword(const char *keyword);

void zlog_clear_filter_keyword(void);

void zlog_set_filter(uint8_t level, const char *tag, const char *keyword);

#ifdef __cplusplus
}
#endif

#endif 
