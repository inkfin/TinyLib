#include "test.h"
#define TL_LOGGING_IMPL
#include "tinylib/logging.h"

void logging_test_cases()
{
    puts("- Logging Test Cases");
    char* name = "Bob";
    TL_LOG(TL_INFO, "Hello! xxx");
    TL_LOG(TL_INFO, "Hello! %s", name);
    TL_LOG(TL_WARNING, "Hello! %s", name);
    TL_LOG(TL_ERROR, "Hello! %s", name);
    puts("\n");
}
