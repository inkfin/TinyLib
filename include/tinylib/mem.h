#ifndef TINYLIB_MEM_H
#define TINYLIB_MEM_H

#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"
#include "c_ext.h"

#define TL_MEM_ALIGN _Alignof(max_align_t)

TL_ATTR_MAYBE_UNUSED
static inline
b32_t
tl_is_power_of_two(size_t n)
{
    return n != 0 && (n & (n - 1U)) == 0;
}

/*
 * Returns 0 on overflow or invalid alignment.
 * `align` must be a power of two.
 */
TL_ATTR_MAYBE_UNUSED
static inline
size_t
tl_align_up(size_t n, size_t align)
{
    size_t mask;

    if (!tl_is_power_of_two(align)) return 0;
    mask = align - 1U;
    if (n > SIZE_MAX - mask) return 0;
    return (n + mask) & ~mask;
}

TL_ATTR_MAYBE_UNUSED
static inline
uintptr_t
tl_align_up_ptr(uintptr_t n, size_t align)
{
    uintptr_t mask;

    if (!tl_is_power_of_two(align)) return 0;
    mask = (uintptr_t)align - 1U;
    if (n > UINTPTR_MAX - mask) return 0;
    return (n + mask) & ~mask;
}

TL_ATTR_MAYBE_UNUSED
static inline
size_t
tl_normalize_align(size_t align)
{
    if (align < sizeof(void *)) align = sizeof(void *);
    if (!tl_is_power_of_two(align)) return 0;
    return align;
}

typedef struct TL_AllocatorVTable {
    void *(*alloc)(void *ctx, size_t size, size_t align);
    void  (*free)(void *ctx, void *ptr, size_t size, size_t align);
    void *(*realloc)(void *ctx,
                     void *ptr,
                     size_t old_size,
                     size_t new_size,
                     size_t align);
} TL_AllocatorVTable;

typedef struct TL_Allocator {
    const TL_AllocatorVTable *vt;
    void *ctx;
} TL_Allocator;

TL_ATTR_MAYBE_UNUSED
static inline
void *
tl_allocator_alloc_aligned(TL_Allocator *allocator, size_t size, size_t align)
{
    return allocator->vt->alloc(allocator->ctx, size, align);
}

TL_ATTR_MAYBE_UNUSED
static inline
void *
tl_allocator_alloc(TL_Allocator *allocator, size_t size)
{
    return tl_allocator_alloc_aligned(allocator, size, TL_MEM_ALIGN);
}

TL_ATTR_MAYBE_UNUSED
static inline
void
tl_allocator_free_aligned(TL_Allocator *allocator,
                          void *ptr,
                          size_t size,
                          size_t align)
{
    allocator->vt->free(allocator->ctx, ptr, size, align);
}

TL_ATTR_MAYBE_UNUSED
static inline
void
tl_allocator_free(TL_Allocator *allocator, void *ptr, size_t size)
{
    tl_allocator_free_aligned(allocator, ptr, size, TL_MEM_ALIGN);
}

TL_ATTR_MAYBE_UNUSED
static inline
void *
tl_allocator_realloc_aligned(TL_Allocator *allocator,
                             void *ptr,
                             size_t old_size,
                             size_t new_size,
                             size_t align)
{
    return allocator->vt->realloc(allocator->ctx, ptr, old_size, new_size, align);
}

TL_ATTR_MAYBE_UNUSED
static inline
void *
tl_allocator_realloc(TL_Allocator *allocator,
                     void *ptr,
                     size_t old_size,
                     size_t new_size)
{
    return tl_allocator_realloc_aligned(allocator, ptr, old_size, new_size, TL_MEM_ALIGN);
}

/* stdlib allocator */

static inline
void *
tl_allocator_std_alloc(void *ctx, size_t size, size_t align)
{
    void *raw;
    uintptr_t base;
    uintptr_t aligned;

    (void)ctx;
    align = tl_normalize_align(align);
    if (align == 0 || size == 0) return NULL;

    if (align <= TL_MEM_ALIGN) {
        return malloc(size);
    }

    if (size > SIZE_MAX - align - sizeof(void *)) return NULL;
    raw = malloc(size + align - 1U + sizeof(void *));
    if (!raw) return NULL;

    base = (uintptr_t)raw + sizeof(void *);
    aligned = tl_align_up_ptr(base, align);
    if (aligned == 0) {
        free(raw);
        return NULL;
    }

    ((void **)aligned)[-1] = raw;
    return (void *)aligned;
}

static inline
void
tl_allocator_std_free(void *ctx, void *ptr, size_t size, size_t align)
{
    (void)ctx;
    (void)size;
    if (!ptr) return;

    align = tl_normalize_align(align);
    if (align == 0) return;

    if (align <= TL_MEM_ALIGN) {
        free(ptr);
    } else {
        free(((void **)ptr)[-1]);
    }
}

static inline
void *
tl_allocator_std_realloc(void *ctx,
                         void *ptr,
                         size_t old_size,
                         size_t new_size,
                         size_t align)
{
    void *next;

    (void)ctx;
    align = tl_normalize_align(align);
    if (align == 0) return NULL;

    if (!ptr) return tl_allocator_std_alloc(ctx, new_size, align);
    if (new_size == 0) {
        tl_allocator_std_free(ctx, ptr, old_size, align);
        return NULL;
    }

    if (align <= TL_MEM_ALIGN) {
        return realloc(ptr, new_size);
    }

    next = tl_allocator_std_alloc(ctx, new_size, align);
    if (!next) return NULL;
    memcpy(next, ptr, old_size < new_size ? old_size : new_size);
    tl_allocator_std_free(ctx, ptr, old_size, align);
    return next;
}

TL_ATTR_MAYBE_UNUSED
static const TL_AllocatorVTable tl_allocatorvt_std = {
    .alloc = tl_allocator_std_alloc,
    .free = tl_allocator_std_free,
    .realloc = tl_allocator_std_realloc,
};

/* arena allocator */

#define TL_ARENA_INITIAL_CAP 4096

typedef struct TL_ArenaChunk {
    byte_t *data;
    size_t used;
    size_t cap;
    struct TL_ArenaChunk *next;
} TL_ArenaChunk;

typedef struct TL_Arena {
    TL_ArenaChunk *chunks;
} TL_Arena;

typedef struct TL_ArenaMark {
    TL_ArenaChunk *chunk;
    size_t used;
} TL_ArenaMark;

static inline
TL_ArenaChunk *
tl_arena__add_chunk(size_t cap)
{
    TL_ArenaChunk *next = (TL_ArenaChunk *)malloc(sizeof(TL_ArenaChunk));
    if (!next) return NULL;

    next->data = (byte_t *)malloc(cap);
    if (!next->data) {
        free(next);
        return NULL;
    }

    next->used = 0;
    next->cap = cap;
    next->next = NULL;
    return next;
}

static inline
TL_ArenaChunk *
tl_arena__last_chunk(TL_Arena *arena)
{
    TL_ArenaChunk *chunk = arena->chunks;
    if (!chunk) return NULL;
    while (chunk->next) chunk = chunk->next;
    return chunk;
}

TL_ATTR_MAYBE_UNUSED
static inline
void *
tl_arena_alloc_aligned(TL_Arena *arena, size_t size, size_t align)
{
    TL_ArenaChunk *chunk;
    uintptr_t current;
    uintptr_t aligned;
    size_t end_used;
    size_t needed_cap;

    assert(arena != NULL);
    align = tl_normalize_align(align);
    if (align == 0 || size == 0) return NULL;

    chunk = tl_arena__last_chunk(arena);
    if (chunk) {
        current = (uintptr_t)chunk->data + chunk->used;
        aligned = tl_align_up_ptr(current, align);
        if (aligned != 0 && aligned >= (uintptr_t)chunk->data) {
            end_used = (size_t)(aligned - (uintptr_t)chunk->data);
            if (end_used <= chunk->cap && size <= chunk->cap - end_used) {
                chunk->used = end_used + size;
                return (void *)aligned;
            }
        }
    }

    if (size > SIZE_MAX - (align - 1U)) return NULL;
    needed_cap = size + align - 1U;
    if (needed_cap < TL_ARENA_INITIAL_CAP) needed_cap = TL_ARENA_INITIAL_CAP;

    chunk = tl_arena__add_chunk(needed_cap);
    if (!chunk) return NULL;

    if (!arena->chunks) {
        arena->chunks = chunk;
    } else {
        tl_arena__last_chunk(arena)->next = chunk;
    }

    current = (uintptr_t)chunk->data;
    aligned = tl_align_up_ptr(current, align);
    if (aligned == 0 || aligned < (uintptr_t)chunk->data) return NULL;

    end_used = (size_t)(aligned - (uintptr_t)chunk->data);
    if (end_used > chunk->cap || size > chunk->cap - end_used) return NULL;

    chunk->used = end_used + size;
    return (void *)aligned;
}

TL_ATTR_MAYBE_UNUSED
static inline
void *
tl_arena_alloc(TL_Arena *arena, size_t size)
{
    return tl_arena_alloc_aligned(arena, size, TL_MEM_ALIGN);
}

TL_ATTR_MAYBE_UNUSED
static inline
TL_ArenaMark
tl_arena_mark(TL_Arena *arena)
{
    TL_ArenaChunk *chunk = tl_arena__last_chunk(arena);
    return (TL_ArenaMark){
        .chunk = chunk,
        .used = chunk ? chunk->used : 0,
    };
}

TL_ATTR_MAYBE_UNUSED
static inline
void
tl_arena_restore(TL_Arena *arena, TL_ArenaMark mark)
{
    TL_ArenaChunk *chunk;
    TL_ArenaChunk *next;

    assert(arena != NULL);
    if (!mark.chunk) {
        chunk = arena->chunks;
        arena->chunks = NULL;
    } else {
        mark.chunk->used = mark.used;
        chunk = mark.chunk->next;
        mark.chunk->next = NULL;
    }

    while (chunk) {
        next = chunk->next;
        free(chunk->data);
        free(chunk);
        chunk = next;
    }
}

TL_ATTR_MAYBE_UNUSED
static inline
void
tl_arena_reset(TL_Arena *arena)
{
    TL_ArenaChunk *chunk;
    assert(arena != NULL);
    for (chunk = arena->chunks; chunk; chunk = chunk->next) {
        chunk->used = 0;
    }
}

TL_ATTR_MAYBE_UNUSED
static inline
void
tl_arena_destroy(TL_Arena *arena)
{
    TL_ArenaChunk *chunk;
    TL_ArenaChunk *next;

    if (!arena) return;
    chunk = arena->chunks;
    while (chunk) {
        next = chunk->next;
        free(chunk->data);
        free(chunk);
        chunk = next;
    }
    arena->chunks = NULL;
}

static inline
void *
tl_allocator_arena_alloc(void *ctx, size_t size, size_t align)
{
    assert(ctx != NULL);
    return tl_arena_alloc_aligned((TL_Arena *)ctx, size, align);
}

static inline
void
tl_allocator_arena_free(void *ctx, void *ptr, size_t size, size_t align)
{
    (void)ctx;
    (void)ptr;
    (void)size;
    (void)align;
}

static inline
void *
tl_allocator_arena_realloc(void *ctx,
                           void *ptr,
                           size_t old_size,
                           size_t new_size,
                           size_t align)
{
    void *next;

    assert(ctx != NULL);
    if (!ptr) return tl_allocator_arena_alloc(ctx, new_size, align);
    if (new_size == 0) return NULL;

    next = tl_allocator_arena_alloc(ctx, new_size, align);
    if (!next) return NULL;
    memcpy(next, ptr, old_size < new_size ? old_size : new_size);
    return next;
}

TL_ATTR_MAYBE_UNUSED
static const TL_AllocatorVTable tl_allocatorvt_arena = {
    .alloc = tl_allocator_arena_alloc,
    .free = tl_allocator_arena_free,
    .realloc = tl_allocator_arena_realloc,
};

TL_ATTR_MAYBE_UNUSED
static inline
TL_Allocator
tl_get_allocator_arena(TL_Arena *arena)
{
    return (TL_Allocator){
        .vt = &tl_allocatorvt_arena,
        .ctx = arena,
    };
}

TL_ATTR_MAYBE_UNUSED
static inline
void
tl_destroy_arena(TL_Arena *arena)
{
    tl_arena_destroy(arena);
}

TL_ATTR_MAYBE_UNUSED
static const TL_Allocator tl_default_allocator = (TL_Allocator){
    .vt = &tl_allocatorvt_std,
    .ctx = NULL,
};

#define TL_TEMP_SCOPE(arena_name) \
    for (TL_Arena arena_name = {0}; \
         (arena_name).chunks != (TL_ArenaChunk *)(uintptr_t)1; \
         tl_arena_destroy(&(arena_name)), (arena_name).chunks = (TL_ArenaChunk *)(uintptr_t)1)

#ifdef TL_MEM_SHORT_NAMES
#define TEMP_SCOPE TL_TEMP_SCOPE

typedef TL_Allocator       Allocator;
typedef TL_AllocatorVTable AllocatorVTable;
typedef TL_ArenaChunk      ArenaChunk;
typedef TL_Arena           Arena;
typedef TL_ArenaMark       ArenaMark;
#endif

#endif /* TINYLIB_MEM_H */
