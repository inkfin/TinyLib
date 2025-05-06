#include "test.h"
#include <stdio.h>
#include <tinylib/common.h>

void common_test_cases(void)
{
    TL_FOREACH(puts, "Hello", "World!");
}
