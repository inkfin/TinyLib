#include "test.h"
#include "tinylib/dyn_arr.h"
#include <stdio.h>

typedef struct {
    size_t len;
    size_t cap;
    int*   data;
} int_arr_t;

void dyn_arr_test_cases(void)
{
    puts("- Dynamic Array Test Cases");
    int_arr_t int_arr = { 0 };

    int err = tl_arr_push(int_arr, &(int) { 0 });

    if (err) {
        printf("Error: %d\n", err);
    }

    tl_arr_push_n(int_arr, &(int) { 1 }, &(int) { 2 }, &(int) { 3 }, &(int) { 4 }, &(int) { 5 }, &(int) { 6 });

    puts("int_arr data:");
    for (size_t i = 0; i < int_arr.len; ++i) {
        printf("%d ", int_arr.data[i]);
    }
    puts("\n");

    int_arr_t int_arr2 = { 0 };

    tl_arr_push_n(int_arr2, &(int) { 7 }, &(int) { 8 }, &(int) { 9 }, &(int) { 10 });

    puts("int_arr2 data:");
    for (size_t i = 0; i < int_arr2.len; ++i) {
        printf("%d ", int_arr2.data[i]);
    }
    puts("\n");

    tl_arr_append(int_arr, int_arr2);
    puts("int_arr after append:");
    for (size_t i = 0; i < int_arr.len; ++i) {
        printf("%d ", int_arr.data[i]);
    }
    puts("\n");

    puts("\n");
}
