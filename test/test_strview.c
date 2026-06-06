#include "../include/tinylib/strview.h"
#include <stdio.h>

static const char *test_str =
"    \n"
"\thello world!"
"    ";

int strview_test_cases() {
    TL_Str8 str = {
        .data = strdup(test_str),
        .len = strlen(test_str),
    };
    TL_StrView sv = tl_sv_from_str8(&str);
    printf("before trim: [" TL_STR_FMT "]\n", TL_STR_ARG(sv));
    sv = tl_sv_trim(&sv);
    printf("after trim: [" TL_STR_FMT "]\n", TL_STR_ARG(sv));
    return 0;
}
