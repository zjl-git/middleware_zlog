#include "zlog_ports.h"
#include "zlog_async.h"
#include "zlog_config.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

static SemaphoreHandle_t g_zlog_lock_mutex;

#ifdef ZLOG_ASYNC_OUTPUT_ENABLE
static TaskHandle_t g_zlog_task_handle;
static SemaphoreHandle_t g_zlog_async_notice;
static uint8_t g_poll_get_log[ZLOG_LINE_BUF_SIZE - 4];

static void zlog_task_handle(void *arg)
{
    BaseType_t ret;
    uint32_t get_log_size = 0;
    
    while (1) {
        ret = xSemaphoreTake(g_zlog_async_notice, portMAX_DELAY);
        if (ret == pdFALSE) {
            continue;
        }

        
        while (1) {
            memset(g_poll_get_log, 0x00, sizeof(g_poll_get_log));
#ifdef ZLOG_ASYNC_LINE_OUTPUT
            get_log_size = zlog_async_get_line_log(g_poll_get_log, sizeof(g_poll_get_log));
#else 
            get_log_size = zlog_async_get_log(g_poll_get_log, sizeof(g_poll_get_log));
#endif

            if (get_log_size > 0) {
                zlog_port_output(g_poll_get_log, get_log_size);
            } else {
                break;
            }
        }
    }
}
#endif


void zlog_async_output_notice(void)
{
#ifdef ZLOG_ASYNC_OUTPUT_ENABLE
    xSemaphoreGive(g_zlog_async_notice);
#endif
}

void zlog_port_init(void)
{
    // g_zlog_lock_mutex = xSemaphoreCreateMutex();
    g_zlog_lock_mutex = xSemaphoreCreateBinary();
    xSemaphoreGive(g_zlog_lock_mutex);

#ifdef ZLOG_ASYNC_OUTPUT_ENABLE
    // g_zlog_async_notice = xSemaphoreCreateMutex();
    g_zlog_async_notice = xSemaphoreCreateBinary();
    xTaskCreate(zlog_task_handle, "zlog_task", 4096, NULL, 1, &g_zlog_task_handle);
#endif

}

void zlog_port_deinit(void)
{

}

#include "esp_log.h"
#define TAGS            "zlog"
void zlog_port_output(uint8_t *log, uint32_t size)
{
    ESP_LOGI(TAGS, "%s", log);
}

void zlog_port_lock(void)
{
    xSemaphoreTake(g_zlog_lock_mutex, portMAX_DELAY);
}

void zlog_port_unlock(void)
{
    xSemaphoreGive(g_zlog_lock_mutex);
}

char *zlog_port_get_time(void)
{
    static char cur_system_time[16] = "";
    snprintf(cur_system_time, 16, "%lu", xTaskGetTickCount());
    return cur_system_time;
}

char *zlog_port_get_p_info(void)
{
    return "p_info";
}

char *zlog_port_get_t_info(void)
{
    /*
    TaskStatus_t  status;
    vTaskGetInfo(NULL, &status, pdTRUE, eInvalid);
    return status.pcTaskName;
    */
   return "t_info";
}
