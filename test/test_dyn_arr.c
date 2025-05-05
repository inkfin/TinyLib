#include "test.h"
#include "tinylib/dyn_arr.h"
#include <stdio.h>

typedef struct {
    size_t len;
    size_t cap;
    int* data;
} int_arr_t;

void dyn_arr_test_cases(void)
{
    puts("- Dynamic Array Test Cases");
    int_arr_t int_arr = { 0 };
    arr_init(int_arr, 4);

    arr_append_items(int_arr, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);

    for (size_t i = 0; i < int_arr.len; ++i) {
        printf("%d ", int_arr.data[i]);
    }
    puts("\n");
}
