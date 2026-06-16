#include "../include/tinylib/strview.h"
#include <stdio.h>

static const char *test_str =
"    \n"
"\thello world!"
"    ";

int strview_test_cases() {
    size_t test_len = strlen(test_str);
    char *buf = malloc(test_len + 1);
    memcpy(buf, test_str, test_len + 1);
    TL_Str8 str = {
        .data = buf,
        .len = test_len,
    };
    TL_StrView sv = tl_sv_from_str8(&str);
    printf("before trim: [" TL_STR_FMT "]\n", TL_STR_ARG(sv));
    sv = tl_sv_trim(&sv);
    printf("after trim: [" TL_STR_FMT "]\n", TL_STR_ARG(sv));
    return 0;
}
