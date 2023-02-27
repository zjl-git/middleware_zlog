#ifndef __ZLOG_UTILS_H__
#define __ZLOG_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "zlog_ports.h"

uint32_t zlog_strcpy(uint32_t cur_len, char *dst, const char *src);

uint32_t zlog_cpyln(uint8_t *line, uint8_t *log, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif 
