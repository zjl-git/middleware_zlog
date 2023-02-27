#include "zlog_utils.h"
#include "zlog_config.h"

uint32_t zlog_strcpy(uint32_t cur_len, char *dst, const char *src)
{
    const char *src_old = src;

    while (*src != 0) {
        if (cur_len++ < ZLOG_LINE_BUF_SIZE) {
            *dst++ = *src++;
        } else {
            break;
        }
    }
    return src - src_old;
}

uint32_t zlog_cpyln(uint8_t *line, uint8_t *log, uint32_t len)
{
    uint32_t newline_len = strlen(ZLOG_NEWLINE_SIGN), copy_size = 0;

    while (len--) {
        *line++ = *log++;
        copy_size++;
        if (copy_size >= newline_len && !strncmp((char *)(log - newline_len), ZLOG_NEWLINE_SIGN, newline_len)) {
            break;
        }
    }
    return copy_size;
}