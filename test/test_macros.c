#include "test.h"
#include <stdio.h>
#include "../include/tinylib/macros.h"

int common_test_cases(void)
{
    puts("- Common Macros Test Cases");
    puts("  TL_FOREACH(puts, \"Hello\", \"World!\"): ");
    TL_FOREACH_F(puts, "Hello", "World!");
    puts("  TL_FOREACH_ONE_PARAM(printf, \"Hello, %s!\\n\", \"Bob\", \"Tom\"): ");
    TL_FOREACH_F_ONE_PARAM(printf, "Hello, %s!\n", "Bob", "Tom");
    puts("\n");

    return 0;
}
