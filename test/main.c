#include "test.h"
#include "tinylib/logging.h"
#include <stdio.h>

int main(void)
{
#ifdef _DEBUG
    log_set_level(TL_INFO);
#else
    log_set_level(TL_WARNING);
#endif

    puts("Start batched testing...");
    puts("=========================");

    // Test logging
    logging_test_cases();

    // Test common macros
    common_test_cases();

    // Test dynamic array
    dyn_arr_test_cases();

    // Test memory pool
    // memory_pool_test_cases();

    // Test logging
    logging_test_cases();
}
