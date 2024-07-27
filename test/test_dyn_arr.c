#include "tinylib/dyn_arr.h"
#include <stdio.h>

decltype_arr(int, int_arr_t);

int main(void)
{
    int_arr_t int_arr = { 0 };
    init_arr(int_arr);

    int item = 1;

    append_arr_items(int_arr, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);

    for (int i = 0; i < int_arr.len; ++i) {
        printf("%d ", int_arr.data[i]);
    }
    puts("\n");

    return 0;
}
