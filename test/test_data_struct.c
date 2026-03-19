#include "test.h"
#define TLDS_ABBR
#include "tinylib/data_struct.h"

#include <assert.h>
#include <stdio.h>

int dyn_arr_test_cases(void)
{
    puts("- Dynamic Array Test Cases");

    arr_of(int) *int_arr = NULL;
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
    assert(arr_cap(int_arr) == 8);

    puts("int_arr data:");
    for (size_t i = 0; i < arr_lenu(int_arr); ++i) {
        fprintf(stdout, "%d ", int_arr->data[i]);
    }
    puts("\n");

    arr_of(int) *int_arr2 = NULL;
    assert(arr_push_n(int_arr2, 7, 8, 9, 10, 11, 12));
    assert(arr_append(int_arr, int_arr2));
    assert(arr_len(int_arr) == 12);
    assert(arr_cap(int_arr) == 16);

    puts("int_arr2 data:");
    for (size_t i = 0; i < arr_lenu(int_arr2); ++i) {
        fprintf(stdout, "%d ", int_arr2->data[i]);
    }
    puts("\n");

    puts("int_arr after append:");
    for (size_t i = 0; i < arr_lenu(int_arr); ++i) {
        fprintf(stdout, "%d ", int_arr->data[i]);
    }
    puts("\n");

    {
        int popped = 0;
        assert(arr_pop(int_arr, &popped));
        assert(popped == 12);
        assert(arr_len(int_arr) == 11);
    }

    arr_free(int_arr);
    arr_free(int_arr2);

    puts("\n");
    return 0;
}
