#include "test.h"
#ifdef TL_SINGLE_TEST_FILE
#define TL_LOGGING_IMPL
#endif // TL_SINGLE_TEST_FILE
#define TLDS_IMPLEMENTATION
#include "tinylib/data_struct.h"
#include <stdio.h>

int dyn_arr_test_cases(void)
{
    puts("- Dynamic Array Test Cases");
    int* int_arr = NULL;
    assert(arrlen(int_arr) == 0);
    assert(arrcap(int_arr) == 0);

    {
        int v = arrpush(int_arr, 11);
        TLDS_array_header* arr_header = tlds_header(int_arr);
        assert(v == 11);
        assert(arrlen(int_arr) == 1);
        assert(arrcap(int_arr) == 4);
    }

    {
        size_t _n = arrdel(int_arr, 0);
        assert(_n == 0); // new length
        assert(arrlen(int_arr) == 0);
        assert(arrcap(int_arr) == 4);
    }

    {
        arrpush_n(int_arr, 1, 2, 3, 4, 5, 6);
        assert(arrlen(int_arr) == 6);
        assert(arrcap(int_arr) == 8);
    }

    puts("int_arr data:");
    for (size_t i = 0; i < arrlenu(int_arr); ++i) {
        fprintf(stdout, "%d ", int_arr[i]);
    }
    puts("\n");

    int* int_arr2 = NULL;

    arrpush_n(int_arr2, 7, 8, 9, 10, 11, 12);

    {
        arrappend(int_arr, int_arr2);
        TLDS_array_header* arr_header2 = tlds_header(int_arr2);
        assert(arrlen(int_arr) == 12);
        assert(arrcap(int_arr) == 16);
    }

    puts("int_arr2 data:");
    for (size_t i = 0; i < arrlenu(int_arr2); ++i) {
        fprintf(stdout, "%d ", int_arr2[i]);
    }
    puts("\n");

    puts("int_arr after append:");
    for (size_t i = 0; i < arrlenu(int_arr); ++i) {
        fprintf(stdout, "%d ", int_arr[i]);
    }
    puts("\n");

    puts("\n");
    return 0;
}
