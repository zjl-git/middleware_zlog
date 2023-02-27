#ifndef __ZLOG_BUF_H__
#define __ZLOG_BUF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "zlog_ports.h"

void zlog_buf_output(const char *log, uint32_t size);

void zlog_flush(void);

void zlog_buf_enabled(bool enabled);

#ifdef __cplusplus
}
#endif

#endif 
