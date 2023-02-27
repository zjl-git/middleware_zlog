#ifndef __ZLOG_PORTS_H__
#define __ZLOG_PORTS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>


void zlog_port_init(void);

void zlog_port_deinit(void);

void zlog_port_output(uint8_t *log, uint32_t size);

void zlog_port_lock(void);

void zlog_port_unlock(void);

char *zlog_port_get_time(void);

char *zlog_port_get_p_info(void);

char *zlog_port_get_t_info(void);

void zlog_async_output_notice(void);


#ifdef __cplusplus
}
#endif

#endif 
