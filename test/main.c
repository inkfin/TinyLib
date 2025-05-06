#include <stdio.h>
#include "test.h"

int main(void)
{
    puts("Start testing...");
    puts("=========================");

    // Test common macros
    common_test_cases();

    // Test dynamic array
    dyn_arr_test_cases();

    // Test memory pool
    // memory_pool_test_cases();

    // Test logging
    logging_test_cases();
}
