#include "../../include/tinylib/logging.c"
#include <stdio.h>

int main(void)
{
    TL_LogConfig cfg = TL_LOG_CONFIG_DEFAULT;

    cfg.level = TL_LOG_LEVEL_DEBUG;
    cfg.output = TL_LOG_OUTPUT_FILE;
    cfg.error_output = TL_LOG_OUTPUT_FILE;
    cfg.filename = "target/c99_logging_output.log";
    cfg.append = false;

    cfg.show_time = false;
    cfg.show_level = true;
    cfg.show_file = false;
    cfg.show_line = false;
    cfg.show_func = false;

    if (!tl_log_init(&cfg)) {
        fprintf(stderr, "failed to initialize logger\n");
        return 1;
    }

    TL_LOG_INFO("c99 info");
    TL_LOG_WARN("c99 warn");
    TL_LOG_ERROR("c99 error");
    tl_log_write_raw(TL_LOG_LEVEL_INFO, __FILE__, __LINE__, __func__, "c99 raw");

    tl_log_shutdown();

    puts("c99 logging test passed");
    return 0;
}
