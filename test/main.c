#include "../include/tinylib/logging.h"
#include <stdio.h>

extern int macros_test_cases(void);
extern int dyn_arr_test_cases(void);
extern int map_test_cases(void);
extern int logging_test_cases(void);
extern int mem_test_cases(void);
extern int strview_test_cases(void);
extern int compile_test_cases(void);

int main(void)
{
    tl_log_set_level(TL_LOG_LEVEL_INFO);

    puts("Start batched testing...");
    puts("=========================");

    // Test logging
    logging_test_cases();

    // Test common macros
    macros_test_cases();

    // Test dynamic array
    dyn_arr_test_cases();

    // Test hash map
    map_test_cases();

    // Test memory pool
    mem_test_cases();

    // Test string view
    strview_test_cases();

    // Test compile helpers
    compile_test_cases();

    tl_log_shutdown();
    return 0;
}
