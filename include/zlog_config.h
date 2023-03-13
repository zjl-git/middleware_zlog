#ifndef __ZLOG_CONFIG_H__
#define __ZLOG_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define ZLOG_LVL_ASSERT                      0
#define ZLOG_LVL_ERROR                       1
#define ZLOG_LVL_WARN                        2
#define ZLOG_LVL_INFO                        3
#define ZLOG_LVL_DEBUG                       4
#define ZLOG_LVL_VERBOSE                     5

#define ZLOG_OUTPUT_ENABLE

#define ZLOG_ASSERT_ENABLE

#define ZLOG_OUTPUT_LVL                         ZLOG_LVL_VERBOSE

#define ZLOG_LINE_BUF_SIZE                      1024

#define ZLOG_LINE_NUM_MAX_LEN                   5

#define ZLOG_FILTER_TAG_MAX_LEN                 20

#define ZLOG_FILTER_KEYWORD_MAX_LEN             16

#define ZLOG_FILTER_TAG_LEVEL_MAX_NUM           20

#define ZLOG_NEWLINE_SIGN                       "\n"

/*---------------------------------------------------------------------------*/

// #define ZLOG_ASYNC_OUTPUT_ENABLE

// #define ZLOG_ASYNC_LINE_OUTPUT

// #define ZLOG_ASYNC_OUTPUT_LVL                    ZLOG_LVL_ERROR

// #define ZLOG_ASYNC_OUTPUT_BUF_SIZE               (ZLOG_LINE_BUF_SIZE * 10)

/*---------------------------------------------------------------------------*/
// #define ZLOG_BUF_OUTPUT_ENABLE

// #define ZLOG_BUF_OUTPUT_BUF_SIZE                 (ZLOG_LINE_BUF_SIZE * 10)



#ifdef __cplusplus
}
#endif

#endif 
