#ifndef __ZLOG_ASYNC_H__
#define __ZLOG_ASYNC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "zlog.h"

zlog_status zlog_async_init(void);

void zlog_async_deinit(void);

void zlog_async_enabled(bool enabled);

void zlog_async_output(uint8_t level, const char *log, uint32_t size);

uint32_t zlog_async_get_line_log(uint8_t *log, uint32_t size);

uint32_t zlog_async_get_log(uint8_t *log, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif 
