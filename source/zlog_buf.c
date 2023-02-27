#include "zlog.h"
#include "zlog_buf.h"

#ifdef ZLOG_BUF_OUTPUT_ENABLE

static bool g_is_enabled = false;
static uint32_t g_buf_write_size = 0;
static uint8_t g_zlog_buf[ZLOG_BUF_OUTPUT_BUF_SIZE] = {0};

void zlog_buf_output(const char *log, uint32_t size)
{
    uint32_t write_size = 0, write_index = 0;

    if (g_is_enabled != true) {
        return ;
    }

    while (true) {
        if (g_buf_write_size + size > ZLOG_BUF_OUTPUT_BUF_SIZE) {
            write_size = ZLOG_BUF_OUTPUT_BUF_SIZE - g_buf_write_size;
            memcpy(g_zlog_buf + g_buf_write_size, log + write_index, write_size);
            write_index += write_size;
            size -= write_size;

            zlog_port_output(g_zlog_buf, ZLOG_BUF_OUTPUT_BUF_SIZE);
            g_buf_write_size = 0;
        } else {
            memcpy(g_zlog_buf + g_buf_write_size, log + write_index, size);
            g_buf_write_size += size;
            break;
        }
    }
}

void zlog_flush(void)
{
    if (g_buf_write_size == 0) {
        return ;
    }

    zlog_port_lock();

    zlog_port_output(g_zlog_buf, g_buf_write_size);
    g_buf_write_size = 0;

    zlog_port_unlock();
}

void zlog_buf_enabled(bool enabled)
{
    g_is_enabled = enabled;
}
#else 
void zlog_flush(void)
{

}
#endif