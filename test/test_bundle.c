#define TL_DS_SHORT_NAMES
#include "../target/tinylib.c"

#include <assert.h>
#include <stdint.h>

int
main(void)
{
    TL_ArrInt *arr = NULL;
    assert(arr_push_n(arr, 1, 2, 3));
    assert(arr_len(arr) == 3);
    assert(arr[0] == 1);
    assert(arr[2] == 3);
    arr_free(arr);

    TL_Map map = {0};
    map_init_strview(map, int, NULL);
    assert(map_put_cstr(map, "answer", 42));
    {
        const int *value = map_get_const_cstr(map, "answer", int);
        assert(value != NULL);
        assert(*value == 42);
    }
    map_free(map);

    TL_Arena arena = {0};
    TL_Allocator arena_allocator = tl_get_allocator_arena(&arena);
    void *arena_ptr = tl_allocator_alloc_aligned(&arena_allocator, 48, 32);
    assert(arena_ptr != NULL);
    assert(((uintptr_t)arena_ptr % 32U) == 0);
    tl_arena_destroy(&arena);

    TL_FixedPool pool = {0};
    assert(tl_fixed_pool_init(&pool, 32, 32, 4));
    TL_Allocator pool_allocator = tl_get_allocator_fixed_pool(&pool);
    void *pool_ptr = tl_allocator_alloc_aligned(&pool_allocator, 24, 32);
    assert(pool_ptr != NULL);
    assert(((uintptr_t)pool_ptr % 32U) == 0);
    tl_allocator_free_aligned(&pool_allocator, pool_ptr, 24, 32);
    tl_fixed_pool_destroy(&pool);

    assert(tl_log_init(NULL));
    tl_log_set_level(TL_LOG_LEVEL_ERROR);
    assert(tl_log_get_level() == TL_LOG_LEVEL_ERROR);
    tl_log_shutdown();

    return 0;
}
