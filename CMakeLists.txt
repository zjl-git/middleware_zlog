idf_component_register(SRCS "source/zlog.c"
                            "source/zlog_utils.c"
                            "source/zlog_ports.c"
                            "source/zlog_buf.c"
                            "source/zlog_async.c"
                            "demo/zlog_demo.c"
                    INCLUDE_DIRS "./include"
                                 "./demo"
                    )
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
