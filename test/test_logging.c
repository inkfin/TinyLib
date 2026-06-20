#include "../include/tinylib/logging.h"
#include <assert.h>

int logging_test_cases()
{
    TL_LogConfig cfg = {0};
    TL_LogConfig default_cfg;

    assert(tl_log_init(NULL));
    default_cfg = tl_log_get_config();
    assert(default_cfg.level == TL_LOG_LEVEL_INFO);
    assert(default_cfg.output == TL_LOG_OUTPUT_DEFAULT);
    assert(default_cfg.error_output == TL_LOG_OUTPUT_DEFAULT);
    assert(default_cfg.disable_auto_flush == 0);
    assert(default_cfg.disable_time == 0);
    assert(default_cfg.disable_level == 0);
    assert(default_cfg.disable_file == 0);
    assert(default_cfg.disable_line == 0);
    assert(default_cfg.disable_func == 0);

    cfg.level = TL_LOG_LEVEL_DEBUG;
    cfg.output = TL_LOG_OUTPUT_FILE;
    cfg.error_output = TL_LOG_OUTPUT_FILE;
    cfg.filename = "target/logging_test.log";
    cfg.disable_time = 1;
    cfg.disable_level = 1;
    cfg.disable_file = 1;
    cfg.disable_line = 1;
    cfg.disable_func = 1;

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
