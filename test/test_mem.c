#include "../include/tinylib/mem.h"
#include <stdio.h>

#define CHECK_ARENA_PTR(ptr, expected_size) \
    do { \
        assert((ptr) != NULL); \
        assert(TL_ARENA_HEADER(ptr)->size == (expected_size)); \
        assert(TL_ARENA_HEADER(ptr)->magic == TL_ARENA_HEADER_MAGIC); \
    } while (0)

#define CHECK_ARENA_CHUNK(arena, expected_used, expected_cap) \
    do { \
        assert((arena).chunks != NULL); \
        assert((arena).chunks->used == (expected_used)); \
        assert((arena).chunks->cap == (expected_cap)); \
    } while (0)

int mem_test_cases() {
    size_t expected_size = 0;

    TL_Arena arena = {0};
    TL_Allocator arena_allocator = tl_get_allocator_arena(&arena);

    byte_t *p1 = tl_allocator_alloc(&arena_allocator, 10);
    CHECK_ARENA_PTR(p1, 10);
    expected_size = tl_align_up(10, TL_MEM_ALIGN) + TL_ARENA_HEADER_SIZE;
    CHECK_ARENA_CHUNK(arena, expected_size, TL_ARENA_INITIAL_CAP);

    byte_t *p2 = tl_allocator_alloc(&arena_allocator, 20);
    CHECK_ARENA_PTR(p2, 20);
    expected_size += tl_align_up(20, TL_MEM_ALIGN) + TL_ARENA_HEADER_SIZE;
    CHECK_ARENA_CHUNK(arena, expected_size, TL_ARENA_INITIAL_CAP);

    byte_t *p3 = tl_allocator_alloc(&arena_allocator, 30);
    CHECK_ARENA_PTR(p3, 30);
    expected_size += tl_align_up(30, TL_MEM_ALIGN) + TL_ARENA_HEADER_SIZE;
    CHECK_ARENA_CHUNK(arena, expected_size, TL_ARENA_INITIAL_CAP);

    tl_destroy_arena(&arena);
    return 0;
}
