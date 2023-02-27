#include "zlog.h"
#include "zlog_utils.h"
#include "zlog_async.h"

#ifdef ZLOG_ASYNC_OUTPUT_ENABLE

static bool g_init_ok = false;
static bool g_is_enabled = false;

static uint8_t g_zlog_buf[ZLOG_ASYNC_OUTPUT_BUF_SIZE];
static uint32_t g_write_index = 0;
static uint32_t g_read_index = 0;

static bool g_buf_is_full = false;
static bool g_buf_is_empty = true;

static uint32_t zlog_async_get_buf_used(void)
{
    if (g_write_index > g_read_index) {
        return g_write_index - g_read_index;
    }

    if (!g_buf_is_full && !g_buf_is_empty) {
        return ZLOG_ASYNC_OUTPUT_BUF_SIZE - (g_read_index - g_write_index);
    } else if (g_buf_is_full) {
        return ZLOG_ASYNC_OUTPUT_BUF_SIZE;
    } else {
        return 0;
    }
}

static uint32_t zlog_async_get_buf_space(void)
{
    return ZLOG_ASYNC_OUTPUT_BUF_SIZE - zlog_async_get_buf_used();
}

static uint32_t zlog_async_put(const char *log, uint32_t size)
{
    uint32_t space = zlog_async_get_buf_space();
    if (space == 0) {
        size = 0;
        return size;
    }

    if (space <= size) {
        size = space;
        g_buf_is_full = true;
    }

    if (g_write_index + size < ZLOG_ASYNC_OUTPUT_BUF_SIZE) {
        memcpy(g_zlog_buf + g_write_index, log, size);
        g_write_index += size;
    } else {
        memcpy(g_zlog_buf + g_write_index, log, ZLOG_ASYNC_OUTPUT_BUF_SIZE - g_write_index);
        memcpy(g_zlog_buf, log + ZLOG_ASYNC_OUTPUT_BUF_SIZE - g_write_index, size - (ZLOG_ASYNC_OUTPUT_BUF_SIZE - g_write_index));
        g_write_index += size - ZLOG_ASYNC_OUTPUT_BUF_SIZE;
    }

    g_buf_is_empty = false;
    return size;
}

zlog_status zlog_async_init(void)
{
    zlog_status res = ZLOG_NO_ERR;

    if (g_init_ok == true) {
        return res;
    }

    g_init_ok = true;
    return res;
}

void zlog_async_deinit(void)
{
    if (g_init_ok != true) {
        return ;
    }

    g_init_ok = false;
}

void zlog_async_enabled(bool enabled)
{
    g_is_enabled = true;
}

void zlog_async_output(uint8_t level, const char *log, uint32_t size)
{
    uint32_t put_size = 0;;

    if (!g_is_enabled) {
        zlog_port_output((uint8_t *)log, size);
        return;
    }

    if (level >= ZLOG_ASYNC_OUTPUT_LVL) {
        put_size = zlog_async_put(log, size);
        if (put_size > 0) {
            zlog_async_output_notice();
        }
    } else {
        zlog_port_output((uint8_t *)log, size);
    }
}

uint32_t zlog_async_get_line_log(uint8_t *log, uint32_t size)
{
    uint32_t used = 0, cpy_log_size = 0;

    zlog_port_lock();

    used = zlog_async_get_buf_used();
    if (used == 0 || size == 0) {
        zlog_port_unlock();
        return cpy_log_size;
    }

    if (used <= size) {
        size = used;
    }

    if (g_read_index + size < ZLOG_ASYNC_OUTPUT_BUF_SIZE) {
        cpy_log_size = zlog_cpyln(log, g_zlog_buf + g_read_index, size);
        g_read_index += cpy_log_size;
    } else {
        cpy_log_size = zlog_cpyln(log, g_zlog_buf + g_read_index, ZLOG_ASYNC_OUTPUT_BUF_SIZE - g_read_index);
        if (cpy_log_size == ZLOG_ASYNC_OUTPUT_BUF_SIZE - g_read_index) {
            cpy_log_size += zlog_cpyln(log + cpy_log_size, g_zlog_buf, size - cpy_log_size);
            g_read_index += cpy_log_size - ZLOG_ASYNC_OUTPUT_BUF_SIZE;
        } else {
            g_read_index += cpy_log_size;
        }
    }

    if (used == cpy_log_size) {
        g_buf_is_empty = true;
    }

    if (cpy_log_size) {
        g_buf_is_full = false;
    }

    zlog_port_unlock();
    return cpy_log_size;
}

uint32_t zlog_async_get_log(uint8_t *log, uint32_t size)
{
    uint32_t used = 0;

    zlog_port_lock();

    used = zlog_async_get_buf_used();
    if (used == 0 || size == 0) {
        zlog_port_unlock();
        return 0;
    }

    if (used <= size) {
        size = used;
        g_buf_is_empty = true;
    }

    if (g_read_index + size < ZLOG_ASYNC_OUTPUT_BUF_SIZE) {
        memcpy(log, g_zlog_buf + g_read_index, size);
        g_read_index += size;
    } else {
        memcpy(log, g_zlog_buf + g_read_index, ZLOG_ASYNC_OUTPUT_BUF_SIZE - g_read_index);
        memcpy(log + ZLOG_ASYNC_OUTPUT_BUF_SIZE - g_read_index, g_zlog_buf, size - (ZLOG_ASYNC_OUTPUT_BUF_SIZE - g_read_index));
        g_read_index += size - ZLOG_ASYNC_OUTPUT_BUF_SIZE;
    }

    g_buf_is_full = false;

    zlog_port_unlock();
    return size;
}

#endif

