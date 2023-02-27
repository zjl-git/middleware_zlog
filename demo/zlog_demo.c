#include "zlog_demo.h"
#include "zlog.h"
#include "zlog_buf.h"
#include "zlog_ports.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#define LOG1_TAG        "test1"
#define LOG2_TAG        "test2"
#define LOG3_TAG        "test_dump"

static uint8_t g_test_buf[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
};

static TaskHandle_t g_task1_handle;
static TaskHandle_t g_task2_handle;

static void task1_handle(void *arg)
{
    uint32_t i = 0;

    while (1) {
        i += 1;

        zlog_assert(LOG1_TAG,"this task1 assert log, index:%d", i);
        zlog_error(LOG1_TAG,"this task1 error log, index:%d", i);
        zlog_warn(LOG1_TAG,"this task1 warn log, index:%d", i);
        zlog_info(LOG2_TAG,"this task1 info log, index:%d", i);
        zlog_debug(LOG2_TAG,"this task1 debug log, index:%d", i);
        zlog_verbose(LOG2_TAG,"this task1 verbose log, index:%d", i);
    
        if (i == 1) {
            zlog_set_filter_level(ZLOG_LVL_INFO);
        } else if (i == 2) {
            zlog_set_filter_level(ZLOG_LVL_VERBOSE);
        } else if (i == 3) {
            zlog_set_filter_tag(LOG2_TAG);
        } else if (i == 4) {
            zlog_clear_filter_tag();
        } else if (i == 5) {
            zlog_set_filter_keyword("info");
        } else if (i == 6) {
            zlog_clear_filter_keyword();
        } else {
            ZLOG_ASSERT(false);
        }


        // zlog_dump(LOG3_TAG, g_test_buf, sizeof(g_test_buf));
        printf("-------------------------------------------------------------------\r\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void task2_handle(void *arg)
{
    while(1) {
        zlog_flush();
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

static void zlog_demo_assert_hook(const char* expr, const char* func, uint32_t line)
{
    zlog_assert("ASSERT","(%s) has assert failed at %s:%ld.\n", expr, func, line);
}

void zlog_demo_init(void)
{
    zlog_init();

    zlog_set_format(ZLOG_LVL_ASSERT, ZLOG_FORMAT_ALL & ~(ZLOG_FORMAT_P_INFO));
    zlog_set_format(ZLOG_LVL_ERROR, ZLOG_FORMAT_ALL & ~(ZLOG_FORMAT_P_INFO|ZLOG_FORMAT_I_INFO));
    zlog_set_format(ZLOG_LVL_WARN, ZLOG_FORMAT_ALL & ~(ZLOG_FORMAT_P_INFO|ZLOG_FORMAT_I_INFO|ZLOG_FORMAT_DIR));
    zlog_set_format(ZLOG_LVL_INFO, ZLOG_FORMAT_ALL & ~(ZLOG_FORMAT_P_INFO|ZLOG_FORMAT_I_INFO|ZLOG_FORMAT_DIR|ZLOG_FORMAT_FUNC));
    zlog_set_format(ZLOG_LVL_DEBUG, ZLOG_FORMAT_ALL & ~(ZLOG_FORMAT_P_INFO|ZLOG_FORMAT_I_INFO|ZLOG_FORMAT_DIR|ZLOG_FORMAT_FUNC|ZLOG_FORMAT_LINE));
    zlog_set_format(ZLOG_LVL_VERBOSE, ZLOG_FORMAT_LEVEL|ZLOG_FORMAT_TAG|ZLOG_FORMAT_TIME);

    zlog_assert_set_hook(zlog_demo_assert_hook);

    zlog_start();

    // xTaskCreate(task1_handle, "zlog_test1_task", 4096, NULL, 3, &g_task1_handle);
    xTaskCreate(task2_handle, "zlog_test2_task", 4096, NULL, 1, &g_task2_handle);
}