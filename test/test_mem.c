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

    tl_destroy_arena(&arena);
    return 0;
}
