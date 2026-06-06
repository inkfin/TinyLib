#include "../include/tinylib/mem.h"
#include <stdio.h>

#define CHECK_ARENA_CHUNK(arena, expected_used, expected_cap) \
    do { \
        assert((arena).chunks != NULL); \
        assert((arena).chunks->used == (expected_used)); \
        assert((arena).chunks->cap == (expected_cap)); \
    } while (0)

int mem_test_cases() {
    size_t expected_size = 0;
    TL_Allocator std_allocator = tl_default_allocator;
    TL_FixedPool fixed_pool = {0};

    TL_Arena arena = {0};
    TL_Allocator arena_allocator = tl_get_allocator_arena(&arena);

    byte_t *p1 = tl_allocator_alloc(&arena_allocator, 10);
    assert(p1 != NULL);
    expected_size = 10;
    CHECK_ARENA_CHUNK(arena, expected_size, TL_ARENA_INITIAL_CAP);

    byte_t *p2 = tl_allocator_alloc(&arena_allocator, 20);
    assert(p2 != NULL);
    expected_size = tl_align_up(expected_size, TL_MEM_ALIGN) + 20;
    CHECK_ARENA_CHUNK(arena, expected_size, TL_ARENA_INITIAL_CAP);

    byte_t *p3 = tl_allocator_alloc(&arena_allocator, 30);
    assert(p3 != NULL);
    expected_size = tl_align_up(expected_size, TL_MEM_ALIGN) + 30;
    CHECK_ARENA_CHUNK(arena, expected_size, TL_ARENA_INITIAL_CAP);

    byte_t *p4 = tl_allocator_alloc_aligned(&arena_allocator, 16, 64);
    assert(p4 != NULL);
    assert(((uintptr_t)p4 % 64U) == 0);

    byte_t *std_p = tl_allocator_alloc_aligned(&std_allocator, 33, 64);
    assert(std_p != NULL);
    assert(((uintptr_t)std_p % 64U) == 0);

    std_p = tl_allocator_realloc_aligned(&std_allocator, std_p, 33, 66, 64);
    assert(std_p != NULL);
    assert(((uintptr_t)std_p % 64U) == 0);
    tl_allocator_free_aligned(&std_allocator, std_p, 66, 64);

    assert(tl_fixed_pool_init(&fixed_pool, 32, 32, 2));
    TL_Allocator fixed_allocator = tl_get_allocator_fixed_pool(&fixed_pool);

    byte_t *pool_a = tl_allocator_alloc_aligned(&fixed_allocator, 24, 32);
    byte_t *pool_b = tl_allocator_alloc_aligned(&fixed_allocator, 24, 32);
    assert(pool_a != NULL);
    assert(pool_b != NULL);
    assert(((uintptr_t)pool_a % 32U) == 0);
    assert(((uintptr_t)pool_b % 32U) == 0);
    assert(pool_a != pool_b);

    tl_allocator_free_aligned(&fixed_allocator, pool_a, 24, 32);
    byte_t *pool_c = tl_allocator_alloc_aligned(&fixed_allocator, 24, 32);
    assert(pool_c == pool_a);

    for (size_t i = 0; i < 24; ++i) {
        pool_c[i] = (byte_t)i;
    }
    pool_c = tl_allocator_realloc_aligned(&fixed_allocator, pool_c, 24, 32, 32);
    assert(pool_c != NULL);
    for (size_t i = 0; i < 24; ++i) {
        assert(pool_c[i] == (byte_t)i);
    }
    assert(tl_allocator_alloc_aligned(&fixed_allocator, 64, 32) == NULL);

    tl_allocator_free_aligned(&fixed_allocator, pool_b, 24, 32);
    tl_allocator_free_aligned(&fixed_allocator, pool_c, 32, 32);
    tl_fixed_pool_destroy(&fixed_pool);

    tl_destroy_arena(&arena);
    return 0;
}
