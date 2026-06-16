#include <stdio.h>
#include <assert.h>
#include "../include/tinylib/c_ext.h"
#include "../include/tinylib/logging.h"
#include "../include/tinylib/macros.h"

#define TL_TEST_ID(x) (x)
#define TL_TEST_ADD(base, x) ((base) + (x))
#define TL_TEST_AFFINE(a, b, x) (((a) * (x)) + (b))

#define TL_TEST_STORE(x)            \
    do {                            \
        values[idx++] = (x);        \
    } while (0)

#define TL_TEST_STORE_ONE_PARAM(scale, x) \
    do {                                  \
        values_one[idx_one++] = (scale) * (x); \
    } while (0)

#define TL_TEST_STORE_TWO_PARAM(scale, bias, x) \
    do {                                        \
        values_two[idx_two++] = ((scale) * (x)) + (bias); \
    } while (0)

static int tl_test_registered_init_ran = 0;

static void tl_test_registered_init(void)
{
    tl_test_registered_init_ran = 1;
}

TL_REGISTER_INIT(tl_test_registered_init)

TL_STATIC_ASSERT(TL_HAS_CONSTRUCTOR_REGISTRATION == 1,
                 "test build expects constructor registration support");

int macros_test_cases(void)
{
    TL_LOG_INFO("- Common Macros Test Cases");

    assert(tl_test_registered_init_ran == 1);

    {
        int TL_CONCAT2(my, Var) = 42;
        int TL_CONCAT3(num, _, 1) = 7;
        int TL_CONCAT4(t, a, g, 4) = 11;
        int TL_CONCAT5(v, a, l, _, 5) = 15;
        int TL_CONCAT6(v, e, r, s, _, 6) = 16;

        assert(myVar == 42);
        assert(num_1 == 7);
        assert(tag4 == 11);
        assert(val_5 == 15);
        assert(vers_6 == 16);
    }

    {
        int sum = TL_FOREACH(TL_TEST_ID, +, 1, 2, 3, 4);
        int sum_with_base = TL_FOREACH_ONE_PARAM(TL_TEST_ADD, +, 10, 1, 2, 3);
        int affine_sum = TL_FOREACH_TWO_PARAM(TL_TEST_AFFINE, +, 2, 1, 1, 2, 3);

        assert(sum == 10);
        assert(sum_with_base == 36);
        assert(affine_sum == 15);
    }

    {
        int values[4] = {0};
        int values_one[4] = {0};
        int values_two[4] = {0};
        size_t idx = 0;
        size_t idx_one = 0;
        size_t idx_two = 0;

        TL_FOREACH_F(TL_TEST_STORE, 4, 5, 6);
        TL_FOREACH_F_ONE_PARAM(TL_TEST_STORE_ONE_PARAM, 3, 1, 2, 3);
        TL_FOREACH_F_TWO_PARAM(TL_TEST_STORE_TWO_PARAM, 2, 1, 1, 2, 3);

        assert(idx == 3);
        assert(values[0] == 4 && values[1] == 5 && values[2] == 6);

        assert(idx_one == 3);
        assert(values_one[0] == 3 && values_one[1] == 6 && values_one[2] == 9);

        assert(idx_two == 3);
        assert(values_two[0] == 3 && values_two[1] == 5 && values_two[2] == 7);
    }

    TL_LOG_INFO("  macro assertions passed.");
    TL_LOG_INFO("");

    return 0;
}
