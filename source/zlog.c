#include "zlog.h"
#include "zlog_buf.h"
#include "zlog_utils.h"
#include "zlog_async.h"

#define __is_print(ch)       ((unsigned int)((ch) - ' ') < 127u - ' ')

zlog_handler g_zlog_handler;

static char g_zlog_buf[ZLOG_LINE_BUF_SIZE];

static const char *g_level_output_info[] = {
        [ZLOG_LVL_ASSERT]  = "A/",
        [ZLOG_LVL_ERROR]   = "E/",
        [ZLOG_LVL_WARN]    = "W/",
        [ZLOG_LVL_INFO]    = "I/",
        [ZLOG_LVL_DEBUG]   = "D/",
        [ZLOG_LVL_VERBOSE] = "V/",
};

void (*zlog_assert_hook)(const char* expr, const char* func, uint32_t line);

void zlog_set_filter_level(uint8_t level) 
{
    ZLOG_ASSERT(level <= ZLOG_LVL_VERBOSE);
    g_zlog_handler.filter.level = level;
}

void zlog_set_filter_tag(const char *tag)
{
    strncpy(g_zlog_handler.filter.tag, tag, ZLOG_FILTER_TAG_MAX_LEN);
}

void zlog_clear_filter_tag(void)
{
    memset(g_zlog_handler.filter.tag, '\0', ZLOG_FILTER_TAG_MAX_LEN + 1);
}

void zlog_set_filter_keyword(const char *keyword)
{
    strncpy(g_zlog_handler.filter.keyword, keyword, ZLOG_FILTER_KEYWORD_MAX_LEN);
}

void zlog_clear_filter_keyword(void)
{
    memset(g_zlog_handler.filter.keyword, '\0', ZLOG_FILTER_TAG_MAX_LEN + 1);
}

void zlog_set_filter(uint8_t level, const char *tag, const char *keyword)
{
    zlog_set_filter_level(level);
    zlog_set_filter_tag(tag);
    zlog_set_filter_keyword(keyword);
}

/**********************************************************************/
void zlog_set_filter_tag_level_default(void)
{
    for (int i = 0; i < ZLOG_FILTER_TAG_LEVEL_MAX_NUM; i++) {
        memset(g_zlog_handler.filter.tag_level[i].tag, '\0', ZLOG_FILTER_TAG_MAX_LEN + 1);
        g_zlog_handler.filter.tag_level[i].level = ZLOG_FILTER_LVL_SILENT;
        g_zlog_handler.filter.tag_level[i].tag_use_flag = false;
    }
}

void zlog_set_filter_tag_level(const char *tag, uint8_t level)
{
    ZLOG_ASSERT(level <= ZLOG_LVL_VERBOSE);
    ZLOG_ASSERT(tag != ((void *)0));
    uint8_t i = 0;

    if (!g_zlog_handler.init_ok) {
        return ;
    }

    zlog_port_lock();

    for (i = 0; i < ZLOG_FILTER_TAG_LEVEL_MAX_NUM; i++) {
        if (g_zlog_handler.filter.tag_level[i].tag_use_flag == true &&
            !(strncmp(tag, g_zlog_handler.filter.tag_level[i].tag, ZLOG_FILTER_TAG_MAX_LEN))) {
            break;;
        }
    } 

    if (i < ZLOG_FILTER_TAG_LEVEL_MAX_NUM) {
        if (level == ZLOG_FILTER_LVL_ALL) {
            g_zlog_handler.filter.tag_level[i].tag_use_flag = false;
            g_zlog_handler.filter.tag_level[i].level = ZLOG_FILTER_LVL_SILENT;
            memset(g_zlog_handler.filter.tag_level[i].tag, '\0', ZLOG_FILTER_TAG_MAX_LEN + 1);
        } else {
            g_zlog_handler.filter.tag_level[i].level = level;
        }
    } else {
        if (level != ZLOG_FILTER_LVL_ALL) {
            for (i = 0; i < ZLOG_FILTER_TAG_LEVEL_MAX_NUM; i++) {
                if (g_zlog_handler.filter.tag_level[i].tag_use_flag == false) {
                    strncpy(g_zlog_handler.filter.tag_level[i].tag, tag, ZLOG_FILTER_TAG_MAX_LEN);
                    g_zlog_handler.filter.tag_level[i].level = level;
                    g_zlog_handler.filter.tag_level[i].tag_use_flag = true;
                }
            }
        }
    }

    zlog_port_unlock();
}

uint8_t zlog_get_filter_tag_level(const char *tag)
{
    ZLOG_ASSERT(tag != ((void *)0));
    uint8_t level = ZLOG_FILTER_LVL_ALL;

    if (!g_zlog_handler.init_ok) {
        return level;
    }

    zlog_port_lock();
    for (int i = 0; i < ZLOG_FILTER_TAG_LEVEL_MAX_NUM; i++) {
        if (g_zlog_handler.filter.tag_level[i].tag_use_flag == true &&
            !(strncmp(tag, g_zlog_handler.filter.tag_level[i].tag, ZLOG_FILTER_TAG_MAX_LEN))) {
            level = g_zlog_handler.filter.tag_level[i].level;
        }
    }

    zlog_port_unlock();
    return level;
}
/**********************************************************************/

void zlog_set_format(uint8_t level, uint32_t set)
{
    ZLOG_ASSERT(level <= ZLOG_LVL_VERBOSE);
    g_zlog_handler.enabled_format_set[level] = set;
}

bool zlog_get_format_enabled(uint8_t level, uint32_t set)
{
    ZLOG_ASSERT(level <= ZLOG_LVL_VERBOSE);
    if (g_zlog_handler.enabled_format_set[level] & set) {
        return true;
    }
    return false;
}

void zlog_set_output_enabled(bool enabled)
{
    g_zlog_handler.output_enable = enabled;
}

zlog_status zlog_init(void)
{
    zlog_status res = ZLOG_NO_ERR;

    if (g_zlog_handler.init_ok == true) {
        return res;
    }

    zlog_port_init();

#ifdef ZLOG_ASYNC_OUTPUT_ENABLE
    zlog_async_init();
#endif

    zlog_set_filter_level(ZLOG_LVL_VERBOSE);

    zlog_set_filter_tag_level_default();

    g_zlog_handler.init_ok = true;
    return res;
}

void zlog_deinit(void)
{
#ifdef ZLOG_ASYNC_OUTPUT_ENABLE
    zlog_async_deinit();
#endif
    zlog_port_deinit();

    g_zlog_handler.init_ok = false;
}

void zlog_start(void)
{
    if (!g_zlog_handler.init_ok) {
        return;
    }

    zlog_set_output_enabled(true);

#ifdef ZLOG_ASYNC_OUTPUT_ENABLE
    zlog_async_enabled(true);
#elif defined ZLOG_BUF_OUTPUT_ENABLE
    zlog_buf_enabled(true);
#endif
}



void zlog_output(uint8_t level, const char *tag, const char *file, const char *func,
                 const long line, const char *format, ...)
{
    uint32_t tag_len = strlen(tag), zlog_len = 0, newline_len = strlen(ZLOG_NEWLINE_SIGN), check_keyword_index = 0;;
    char line_num[ZLOG_LINE_NUM_MAX_LEN + 1] = { 0 };
    char tag_sapce[ZLOG_FILTER_TAG_MAX_LEN / 2 + 1] = { 0 };
    int32_t formate_result;
    va_list args;

    ZLOG_ASSERT(level <= ZLOG_LVL_VERBOSE);

    if (g_zlog_handler.output_enable == false) {
        return ;
    }

    if (level > g_zlog_handler.filter.level || level > zlog_get_filter_tag_level(tag)) {
        return ;
    }

    if (g_zlog_handler.filter.tag[0] != '\0') {
        if (!strstr(tag, g_zlog_handler.filter.tag)) {
            return;
        }
    }
    memset(g_zlog_buf, 0x00, sizeof(g_zlog_buf));

    va_start(args, format);

    zlog_port_lock();

    if (zlog_get_format_enabled(level, ZLOG_FORMAT_LEVEL)) {
        zlog_len += zlog_strcpy(zlog_len, g_zlog_buf + zlog_len, g_level_output_info[level]);
    }

    if (zlog_get_format_enabled(level, ZLOG_FORMAT_TAG)) {
        zlog_len += zlog_strcpy(zlog_len, g_zlog_buf + zlog_len, tag);
        if (tag_len <= ZLOG_FILTER_TAG_MAX_LEN / 2) {
            memset(tag_sapce, ' ', ZLOG_FILTER_TAG_MAX_LEN / 2 - tag_len);
            zlog_len += zlog_strcpy(zlog_len, g_zlog_buf + zlog_len, tag_sapce);
        }
        zlog_len += zlog_strcpy(zlog_len, g_zlog_buf + zlog_len, " ");
    }

    if (zlog_get_format_enabled(level, ZLOG_FORMAT_TIME|ZLOG_FORMAT_P_INFO|ZLOG_FORMAT_I_INFO)) {
        zlog_len += zlog_strcpy(zlog_len, g_zlog_buf + zlog_len, "[");

        if (zlog_get_format_enabled(level, ZLOG_FORMAT_TIME)) {
            zlog_len += zlog_strcpy(zlog_len, g_zlog_buf + zlog_len, zlog_port_get_time());
            if (zlog_get_format_enabled(level, ZLOG_FORMAT_P_INFO|ZLOG_FORMAT_I_INFO)) {
                zlog_len += zlog_strcpy(zlog_len, g_zlog_buf + zlog_len, " ");
            }
        }

        if (zlog_get_format_enabled(level, ZLOG_FORMAT_P_INFO)) {
            zlog_len += zlog_strcpy(zlog_len, g_zlog_buf + zlog_len, zlog_port_get_p_info());
            if (zlog_get_format_enabled(level, ZLOG_FORMAT_I_INFO)) {
                zlog_len += zlog_strcpy(zlog_len, g_zlog_buf + zlog_len, " ");
            }
        }

        if (zlog_get_format_enabled(level, ZLOG_FORMAT_I_INFO)) {
            zlog_len += zlog_strcpy(zlog_len, g_zlog_buf + zlog_len, zlog_port_get_t_info());
        }
        zlog_len += zlog_strcpy(zlog_len, g_zlog_buf + zlog_len, "] ");
    }

    if (zlog_get_format_enabled(level, ZLOG_FORMAT_DIR|ZLOG_FORMAT_FUNC|ZLOG_FORMAT_LINE)) {
        zlog_len += zlog_strcpy(zlog_len, g_zlog_buf + zlog_len, "(");

        if (zlog_get_format_enabled(level, ZLOG_FORMAT_DIR)) {
            zlog_len += zlog_strcpy(zlog_len, g_zlog_buf + zlog_len, file);
            if (zlog_get_format_enabled(level, ZLOG_FORMAT_FUNC|ZLOG_FORMAT_LINE)) {
                zlog_len += zlog_strcpy(zlog_len, g_zlog_buf + zlog_len, ":");
            }
        }

        if (zlog_get_format_enabled(level, ZLOG_FORMAT_FUNC)) {
            zlog_len += zlog_strcpy(zlog_len, g_zlog_buf + zlog_len, func);
            if (zlog_get_format_enabled(level, ZLOG_FORMAT_LINE)) {
                zlog_len += zlog_strcpy(zlog_len, g_zlog_buf + zlog_len, " ");
            }
        }

        if (zlog_get_format_enabled(level, ZLOG_FORMAT_LINE)) {
            snprintf(line_num, ZLOG_LINE_NUM_MAX_LEN, "%ld", line);
            zlog_len += zlog_strcpy(zlog_len, g_zlog_buf + zlog_len, line_num);
        }
        zlog_len += zlog_strcpy(zlog_len, g_zlog_buf + zlog_len, ") ");
    }

    formate_result = vsnprintf(g_zlog_buf + zlog_len, ZLOG_LINE_BUF_SIZE - zlog_len, format, args);
    check_keyword_index = zlog_len;

    va_end(args);

    if ((zlog_len + formate_result <= ZLOG_LINE_BUF_SIZE) && (formate_result >= -1)) {
        zlog_len += formate_result;
    } else {
        zlog_len = ZLOG_LINE_BUF_SIZE;
    }

    if (zlog_len + newline_len > ZLOG_LINE_BUF_SIZE) {
        zlog_len = ZLOG_LINE_BUF_SIZE - newline_len;
    }

    if (g_zlog_handler.filter.keyword[0] != '\0') {
        g_zlog_buf[zlog_len] = '\0';
        if (!strstr(g_zlog_buf + check_keyword_index, g_zlog_handler.filter.keyword)) {
            zlog_port_unlock();
            return;
        }
    }

    zlog_len += zlog_strcpy(zlog_len, g_zlog_buf + zlog_len, ZLOG_NEWLINE_SIGN);

#ifdef ZLOG_ASYNC_OUTPUT_ENABLE
    zlog_async_output(level, g_zlog_buf, zlog_len);
#elif defined ZLOG_BUF_OUTPUT_ENABLE
    zlog_buf_output(g_zlog_buf, zlog_len);
#else
    zlog_port_output((uint8_t *)g_zlog_buf, zlog_len);
#endif
    zlog_port_unlock();
}

void zlog_raw(const char *format, ...)
{
    va_list args;
    uint32_t zlog_len = 0;
    int format_result;

    if (!g_zlog_handler.output_enable) {
        return ;
    }
    memset(g_zlog_buf, 0x00, sizeof(g_zlog_buf));

    va_start(args, format);

    zlog_port_lock();

    format_result = vsnprintf(g_zlog_buf, ZLOG_LINE_BUF_SIZE, format, args);
    if ((format_result > -1) && (format_result <= ZLOG_LINE_BUF_SIZE)) {
        zlog_len = format_result;
    } else {
        zlog_len = ZLOG_LINE_BUF_SIZE;
    }

#ifdef ZLOG_ASYNC_OUTPUT_ENABLE
    zlog_async_output(ZLOG_LVL_ASSERT, g_zlog_buf, zlog_len);
#elif defined ZLOG_BUF_OUTPUT_ENABLE
    zlog_buf_output(g_zlog_buf, zlog_len);
#else
    zlog_port_output((uint8_t *)g_zlog_buf, zlog_len);
#endif

    zlog_port_unlock();
    va_end(args);
}

void zlog_hexdump(const char *name, uint8_t width, const void *buf, uint32_t size)
{
    uint32_t i, j;
    uint16_t zlog_len = 0;
    const uint8_t *buf_p = buf;
    char dump_string[8] = {0};
    int format_result;

    if (!g_zlog_handler.output_enable) {
        return;
    }

    if (g_zlog_handler.filter.level < ZLOG_LVL_DEBUG) {
        return ;
    }

    if (!strstr(name, g_zlog_handler.filter.tag)) {
        return ;
    }
    memset(g_zlog_buf, 0x00, sizeof(g_zlog_buf));

    zlog_port_lock();

    for (i = 0; i < size; i += width) {
        /*header*/
        format_result = snprintf(g_zlog_buf, ZLOG_LINE_BUF_SIZE, "D/HEX %s: %04X-%04X: ", name, i, i + width - 1);
        if ((format_result > -1) && (format_result <= ZLOG_LINE_BUF_SIZE)) {
            zlog_len = format_result;
        } else {
            zlog_len = ZLOG_LINE_BUF_SIZE;
        }

        /*dump hex*/
        for (j = 0; j < width; j++) {
            if (i + j < size) {
                snprintf(dump_string, sizeof(dump_string), "%02X ", buf_p[i + j]);
            } else {
                strncpy(dump_string, "   ", sizeof(dump_string));
            }
            zlog_len += zlog_strcpy(zlog_len, g_zlog_buf + zlog_len, dump_string);

            if ((j + 1) % 8 == 0) {
                zlog_len += zlog_strcpy(zlog_len, g_zlog_buf + zlog_len, " ");
            }
        }

        /*dump char for hex*/
        for (j = 0; j < width; j++) {
            if (i + j < size) {
                snprintf(dump_string, sizeof(dump_string), "%c", __is_print(buf_p[i + j]) ? buf_p[i + j] : '.');
                zlog_len += zlog_strcpy(zlog_len, g_zlog_buf + zlog_len, dump_string);
            }
        }

        /*overflow check and reserve some space for newline sign*/
        if (zlog_len + strlen(ZLOG_NEWLINE_SIGN) > ZLOG_LINE_BUF_SIZE) {
            zlog_len = ZLOG_LINE_BUF_SIZE - strlen(ZLOG_NEWLINE_SIGN);
        }

        /* package newline sign */
        zlog_len += zlog_strcpy(zlog_len, g_zlog_buf + zlog_len, ZLOG_NEWLINE_SIGN);

#ifdef ZLOG_ASYNC_OUTPUT_ENABLE
        zlog_async_output(ZLOG_LVL_DEBUG, g_zlog_buf, zlog_len);
#elif defined ZLOG_BUF_OUTPUT_ENABLE
        zlog_buf_output(g_zlog_buf, zlog_len);
#else
        zlog_port_output((uint8_t *)g_zlog_buf, zlog_len);
#endif 

    }
    zlog_port_unlock();
}

void zlog_assert_set_hook(void (*hook)(const char* expr, const char* func, uint32_t line))
{
    zlog_assert_hook = hook;
}

