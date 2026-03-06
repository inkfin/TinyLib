#include "test.h"
#define TL_LOG_IMPLEMENTATION
#include "../include/tinylib/logging.h"

int logging_test_cases()
{
    TL_LogConfig cfg = TL_LOG_CONFIG_DEFAULT;
    cfg.level = TL_LOG_LEVEL_DEBUG;
    cfg.output = TL_LOG_OUTPUT_FILE;
    cfg.error_output = TL_LOG_OUTPUT_FILE;
    cfg.filename = "target/logging_test.log";
    cfg.append = false;
    cfg.show_time = false;
    cfg.show_level = false;
    cfg.show_file = false;
    cfg.show_line = false;
    cfg.show_func = false;

    if (!tl_log_init(&cfg))
        return -1;

    puts("- Logging Test Cases");
    {
        const char *name = "Bob";
        TL_LOG_INFO("Hello! xxx");
        TL_LOG_INFO("Hello! %s", name);
        TL_LOG_WARN("Hello! %s", name);
        TL_LOG_ERROR("Hello! %s", name);
    }

    tl_log_write_raw(
        TL_LOG_LEVEL_INFO, __FILE__, __LINE__, __func__, "raw logging path");
    tl_log_flush();

    puts("\n");

    return 0;
}
