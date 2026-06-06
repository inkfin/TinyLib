#define TL_DS_SHORT_NAMES
#include "../include/tinylib/data_struct.h"

#include <assert.h>
#include <stdio.h>

int dyn_arr_test_cases(void)
{
    puts("- Dynamic Array Test Cases");

    TL_ArrInt *int_arr = NULL;
    arr_init(int_arr, NULL);
    assert(int_arr != NULL);
    assert(arr_len(int_arr) == 0);
    assert(arr_cap(int_arr) == 0);
    assert(arr_addnidx(int_arr, 0) == 0);
    assert(arr_addnptr(int_arr, 0) == int_arr->data);

    assert(arr_push(int_arr, 11));
    assert(arr_len(int_arr) == 1);
    assert(arr_cap(int_arr) == 1);
    assert(*arr_front(int_arr) == 11);

    assert(arr_del(int_arr, 0));
    assert(arr_len(int_arr) == 0);
    assert(arr_cap(int_arr) == 1);

    assert(arr_push_n(int_arr, 1, 2, 3, 4, 5, 6));
    assert(arr_len(int_arr) == 6);
    assert(arr_cap(int_arr) == 6);

    puts("int_arr data:");
    for (size_t i = 0; i < arr_len(int_arr); ++i) {
        fprintf(stdout, "%d ", int_arr->data[i]);
    }
    puts("\n");

    TL_ArrInt *int_arr2 = NULL;
    assert(arr_push_n(int_arr2, 7, 8, 9, 10, 11, 12));
    assert(arr_append(int_arr, int_arr2));
    assert(arr_len(int_arr) == 12);
    assert(arr_cap(int_arr) == 12);

    puts("int_arr2 data:");
    for (size_t i = 0; i < arr_len(int_arr2); ++i) {
        fprintf(stdout, "%d ", int_arr2->data[i]);
    }
    puts("\n");

    puts("int_arr after append:");
    for (size_t i = 0; i < arr_len(int_arr); ++i) {
        fprintf(stdout, "%d ", int_arr->data[i]);
    }
    puts("\n");

    {
        int popped = 0;
        assert(arr_pop(int_arr, &popped));
        assert(popped == 12);
        assert(arr_len(int_arr) == 11);
    }

    {
        TL_ArrU8 *bytes = NULL;
        assert(arr_push_n(bytes, 1, 2, 3, 255));
        assert(arr_len(bytes) == 4);
        assert(*arr_front(bytes) == 1);
        assert(*arr_back(bytes) == 255);
        arr_free(bytes);
    }

    arr_free(int_arr);
    arr_free(int_arr2);

    puts("\n");
    return 0;
}
