#define TL_IMPLEMENTATION
#include "tinylib/logging.h"

int main()
{
    char* name = "Bob";
    TL_LOG(TL_INFO, "Hello! %s", name);
    TL_LOG(TL_WARNING, "Hello! %s", name);
    TL_LOG(TL_ERROR, "Hello! %s", name);
    return 0;
}
