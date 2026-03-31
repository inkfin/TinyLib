#ifndef TL_MEMORY_H
#define TL_MEMORY_H

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "c_ext.h"


/**
 * NOTE: returns 0 when overflow happen
 */
static inline
size_t
tl_align_up(size_t n, size_t align) {
    size_t mask = align - 1;
    if (n > SIZE_MAX - mask) { return 0; }
    return (n + mask) & ~mask;
}

#define TL_MEM_ALIGN _Alignof(max_align_t)


/**
 * A vtable stores the behavior of an allocator. The allocator itself is just a pointer to the vtable and a context pointer.
 */
typedef struct TL_AllocatorVTable {
    void* (*alloc)(void* ctx, size_t size);
    void  (*free)(void* ctx, void* ptr);
    void* (*realloc)(void* ctx, void* ptr, size_t new_size);
} TL_AllocatorVTable;


/**
 * A TL_Allocator is an fat pointer to a mempool
 */
typedef struct TL_Allocator {
    const TL_AllocatorVTable *vt;
    void *ctx;
} TL_Allocator;

/** public interface **/

TL_ATTR_MAYBE_UNUSED
static inline
void *
tl_allocator_alloc(TL_Allocator *allocator, size_t size)
{
    return allocator->vt->alloc(allocator->ctx, size);
}

TL_ATTR_MAYBE_UNUSED
static inline
void
tl_allocator_free(TL_Allocator *allocator, void *ptr)
{
    allocator->vt->free(allocator->ctx, ptr);
}

TL_ATTR_MAYBE_UNUSED
static inline
void *
tl_allocator_realloc(TL_Allocator *allocator, void *ptr, size_t new_size)
{
    return allocator->vt->realloc(allocator->ctx, ptr, new_size);
}

/** stdlib **/

static inline
void *
tl_allocator_std_alloc(void *ctx, size_t size)
{
    (void)ctx;
    return malloc(size);
}

static inline
void
tl_allocator_std_free(void *ctx, void *ptr)
{
    (void)ctx;
    free(ptr);
}

static inline
void *
tl_allocator_std_realloc(void *ctx, void *ptr, size_t new_size)
{
    (void)ctx;
    return realloc(ptr, new_size);
}

TL_ATTR_MAYBE_UNUSED
static const TL_AllocatorVTable tl_allocatorvt_std = {
    .alloc = tl_allocator_std_alloc,
    .free = tl_allocator_std_free,
    .realloc = tl_allocator_std_realloc,
};


/** arena **/

#define TL_ARENA_INITIAL_CAP 4096

typedef struct TL_ArenaHeader {
    size_t size;
} TL_ArenaHeader;

#define TL_ARENA_HEADER_SIZE tl_align_up(sizeof(TL_ArenaHeader), TL_MEM_ALIGN)
#define TL_ARENA_HEADER(p)   ((TL_ArenaHeader *)((byte_t *)(p) - tl_align_up(sizeof(TL_ArenaHeader), TL_MEM_ALIGN)))

typedef struct TL_ArenaChunk {
    byte_t *data;
    size_t used;
    size_t cap;
    struct TL_ArenaChunk *next;
} TL_ArenaChunk;

typedef struct TL_Arena {
    TL_ArenaChunk *chunks;
} TL_Arena;

/* Arena Helpers */

static inline
TL_ArenaChunk *
tl_allocator_arena__add_chunk(size_t cap) {
    TL_ArenaChunk *next = malloc(sizeof(TL_ArenaChunk));
    if (next == NULL) { return NULL; }
    next->data = malloc(cap);
    if (next->data == NULL) { free(next); return NULL; }
    next->used = 0;
    next->cap = cap;
    next->next = NULL;
    return next;
}

static inline
void *
tl_allocator_arena__grow_size(TL_Arena *arena, size_t size) {
    assert(arena != NULL);
    if (size > SIZE_MAX - TL_ARENA_HEADER_SIZE) { return NULL; }
    byte_t *base = NULL;
    size_t start = 0;
    size_t required = TL_ARENA_HEADER_SIZE + size;
    size_t required_aligned = tl_align_up(required, TL_MEM_ALIGN);
    if (required > required_aligned) { return NULL; } // overflow
    if (arena->chunks == NULL) {
        // Init arena
        arena->chunks = tl_allocator_arena__add_chunk(
                required_aligned > TL_ARENA_INITIAL_CAP ? required_aligned : TL_ARENA_INITIAL_CAP);
        if (arena->chunks == NULL) { return NULL; }
        arena->chunks->used = required_aligned;
        base = (byte_t *)arena->chunks->data;
        start = 0;
    } else {
        // Find last chunk
        TL_ArenaChunk *p = arena->chunks;
        while (p->next) { p = p->next; }
        if (required_aligned > (p->cap - p->used)) {
            size_t new_cap = (p->cap < SIZE_MAX / 2) ? (p->cap * 2) : SIZE_MAX;
            new_cap = required_aligned > new_cap ? required_aligned : new_cap;
            p->next = tl_allocator_arena__add_chunk(new_cap);
            if (p->next == NULL) { return NULL; }
            p->next->used = required_aligned;
            base = (byte_t *)p->next->data;
            start = 0;
        } else {
            base = (byte_t *)p->data;
            start = p->used;
            p->used += required_aligned;
        }
    }

    TL_ARENA_HEADER(base + start)->size = size;
    return base + start + TL_ARENA_HEADER_SIZE;
}

static inline
void *
tl_allocator_arena_alloc(void *ctx, size_t size) {
    assert(ctx != NULL);
    return tl_allocator_arena__grow_size((TL_Arena *)ctx, size);
}

static inline
void
tl_allocator_arena_free(void *ctx, void *ptr) {
    (void)ctx;
    (void)ptr;
    // do nothing here
}

static inline
void *
tl_allocator_arena_realloc(void *ctx, void *ptr, size_t new_size) {
    // just allocate new memory
    assert(ctx != NULL);
    void *retp = tl_allocator_arena__grow_size((TL_Arena *)ctx, new_size);
    if (retp == NULL) { return NULL; }
    size_t old_size = TL_ARENA_HEADER(ptr)->size;
    memcpy(retp, (byte_t *)ptr, old_size < new_size ? old_size : new_size);
    TL_ARENA_HEADER(retp)->size = new_size;
    return retp;
}

TL_ATTR_MAYBE_UNUSED
static const TL_AllocatorVTable tl_allocatorvt_arena = {
    .alloc = tl_allocator_arena_alloc,
    .free = tl_allocator_arena_free,
    .realloc = tl_allocator_arena_realloc,
};

#define TL_TEMP_SCOPE(name)

/*
 * Default allocator is only a wrapper of std malloc/free, no context needed.
 */
TL_ATTR_MAYBE_UNUSED
static const TL_Allocator tl_default_allocator = (TL_Allocator){
    .vt = &tl_allocatorvt_std,
    .ctx = NULL,
};


#ifndef TLM_NO_ABBR
#define TEMP_SCOPE      TL_TEMP_SCOPE
#define Allocator       TL_Allocator
#define AllocatorVTable TL_AllocatorVTable
#define ArenaChunk      TL_ArenaChunk
#define Arena           TL_Arena
#endif

#endif  // TL_MEMORY_H
