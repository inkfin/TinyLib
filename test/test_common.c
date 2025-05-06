#include "test.h"
#include <stdio.h>
#include <tinylib/common.h>

void common_test_cases(void)
{
    puts("- Common Macros Test Cases");
    puts("  TL_FOREACH(puts, \"Hello\", \"World!\"): ");
    TL_FOREACH(puts, "Hello", "World!");
    puts("\n");
}
