/* vim: set ft=c : -*- mode: c -*-
 * data_struct.h
 *   Typed dynamic array utilities for GNU11.
 *
 *   Model:
 *     - arrays are explicit heap objects with a visible flexible header:
 *         `tl_arr_of(T) *arr`
 *     - metadata lives in the object itself (`len`, `cap`, `alloc`)
 *     - there is no hidden stb-style prefix metadata before the returned pointer
 *     - array identity is stable, but the allocation may still move on growth
 *
 *   Initialization:
 *     - arrays are expected to be explicitly initialized with `tl_arr_init`
 *     - an initialized empty array is a valid header-only allocation with:
 *         `len == 0`, `cap == 0`
 *     - a never-initialized `NULL` pointer is treated as a different state
 *       from an initialized empty array by design
 *
 *   API shape:
 *     - read-only accessors return const-qualified views where appropriate
 *     - mutating operations require an lvalue array pointer so reallocation can
 *       update the caller-visible handle
 *     - abbreviated snake_case names are available only under `TLDS_ABBR`
 *
 *   Requirements:
 *     - this header currently relies on GNU statement expressions internally
 *     - generic language/attribute helpers live in `c_ext.h`
 */
#ifndef TINYLIB_DATA_STRUCT_H
#define TINYLIB_DATA_STRUCT_H

#include "defs.h"
#include "c_ext.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifdef TLDS_DEBUG_PRINT
#include <stdio.h>
#define TLDS__PRINT(fmt, ...) fprintf(stderr, "%s: " fmt "\n", __func__, ##__VA_ARGS__)
#else
#define TLDS__PRINT(fmt, ...)
#endif

#ifndef TLDS_ASSERT
#define TLDS_ASSERT(expr, msg, ...)                                                   \
    do {                                                                              \
        if (!(expr)) {                                                                \
            TLDS__PRINT("[ERROR] Assertion failed (" #expr "): " msg, ##__VA_ARGS__); \
            assert(expr);                                                             \
        }                                                                             \
    } while (0)
#endif

#if !TL_HAS_STATEMENT_EXPR
#error "data_struct.h currently requires GNU statement expressions."
#endif

#define TLDS__EXPR(...) ({ __VA_ARGS__ })

typedef struct tl_allocator tl_allocator;
struct tl_allocator {
    void *(*realloc)(const tl_allocator *self, void *ptr, size_t old_size, size_t new_size);
    void (*free)(const tl_allocator *self, void *ptr, size_t size);
};

typedef struct tl__arr_hdr {
    size_t len;
    size_t cap;
    const tl_allocator *alloc;
    byte_t data[];
} tl__arr_hdr;

#define tl_arr_of(T) \
    struct {         \
        size_t len;  \
        size_t cap;  \
        const tl_allocator *alloc; \
        T data[];    \
    }

#define tl_arr_ref(arr)  (&(arr))
#define tl_arr_cref(arr) ((const TL_TYPEOF(arr) *)&(arr))

#define tl_arr_len(arr)   ((arr) ? (arr)->len : 0U)
#define tl_arr_lenu(arr)  ((arr) ? (arr)->len : 0U)
#define tl_arr_cap(arr)   ((arr) ? (arr)->cap : 0U)
#define tl_arr_empty(arr) (tl_arr_len(arr) == 0U)

#define tl_arr_data(arr) \
    ((arr) ? (const TL_TYPEOF((arr)->data[0]) *)(arr)->data : NULL)
#define tl_arr_data_mut(arr) \
    ((arr) ? (arr)->data : NULL)

#define tl_arr_at(arr, idx) \
    (((arr) != NULL && (size_t)(idx) < (arr)->len) ? &tl_arr_data(arr)[(idx)] : NULL)
#define tl_arr_at_mut(arr, idx) \
    (((arr) != NULL && (size_t)(idx) < (arr)->len) ? &tl_arr_data_mut(arr)[(idx)] : NULL)

#define tl_arr_front(arr) tl_arr_at((arr), 0)
#define tl_arr_front_mut(arr) tl_arr_at_mut((arr), 0)
#define tl_arr_back(arr) \
    (((arr) != NULL && (arr)->len > 0U) ? &tl_arr_data(arr)[(arr)->len - 1U] : NULL)
#define tl_arr_back_mut(arr) \
    (((arr) != NULL && (arr)->len > 0U) ? &tl_arr_data_mut(arr)[(arr)->len - 1U] : NULL)

#ifdef TLDS_ABBR
#define arr_of         tl_arr_of
#define arr_ref        tl_arr_ref
#define arr_cref       tl_arr_cref
#define arr_len        tl_arr_len
#define arr_lenu       tl_arr_lenu
#define arr_cap        tl_arr_cap
#define arr_empty      tl_arr_empty
#define arr_data       tl_arr_data
#define arr_data_mut   tl_arr_data_mut
#define arr_at         tl_arr_at
#define arr_at_mut     tl_arr_at_mut
#define arr_front      tl_arr_front
#define arr_front_mut  tl_arr_front_mut
#define arr_back       tl_arr_back
#define arr_back_mut   tl_arr_back_mut
#define arr_init       tl_arr_init
#define arr_free       tl_arr_free
#define arr_clear      tl_arr_clear
#define arr_reserve    tl_arr_reserve
#define arr_resize     tl_arr_resize
#define arr_set_cap    tl_arr_reserve
#define arr_set_len    tl_arr_resize
#define arr_push       tl_arr_push
#define arr_push_n     tl_arr_push_n
#define arr_pushp      tl_arr_pushp
#define arr_append     tl_arr_append
#define arr_addnptr    tl_arr_addnptr
#define arr_addnidx    tl_arr_addnidx
#define arr_insn       tl_arr_insn
#define arr_ins        tl_arr_ins
#define arr_pop        tl_arr_pop
#define arr_del        tl_arr_del
#define arr_deln       tl_arr_deln
#define arr_del_swap   tl_arr_del_swap
#endif

static inline void *
allocator_stdmalloc(const tl_allocator *self, void *ptr, size_t old_size, size_t new_size) {
    (void)self;
    (void)old_size;
    return realloc(ptr, new_size);
}

static inline void
allocator_stdfree(const tl_allocator *self, void *ptr, size_t size) {
    (void)self;
    (void)size;
    free(ptr);
}

static const tl_allocator tl_allocator_default = {
    .realloc = allocator_stdmalloc,
    .free = allocator_stdfree,
};

static inline const tl_allocator *
tl__arr_allocator(const tl__arr_hdr *arr) {
    return (arr && arr->alloc) ? arr->alloc : &tl_allocator_default;
}

static inline size_t
tl__arr_hdr_size(void) {
    return offsetof(tl__arr_hdr, data);
}

static inline tl__arr_hdr *
tl__arr_init_impl(const tl_allocator *alloc) {
    tl__arr_hdr *arr;
    const tl_allocator *resolved = alloc ? alloc : &tl_allocator_default;
    size_t total_size = tl__arr_hdr_size();

    arr = (tl__arr_hdr *)resolved->realloc(resolved, NULL, 0U, total_size);
    if (arr == NULL) {
        TLDS__PRINT("[ERROR] failed to allocate empty array header");
        return NULL;
    }

    arr->len = 0;
    arr->cap = 0;
    arr->alloc = resolved;
    return arr;
}

static inline size_t
tl__arr_total_bytes(size_t elem_size, size_t cap) {
    size_t data_bytes;

    if (elem_size == 0) return 0;
    if (cap > SIZE_MAX / elem_size) return 0;
    data_bytes = elem_size * cap;
    if (data_bytes > SIZE_MAX - tl__arr_hdr_size()) return 0;
    return tl__arr_hdr_size() + data_bytes;
}

static inline size_t
tl__arr_next_cap(size_t old_cap, size_t need_cap) {
    size_t new_cap;

    if (need_cap <= old_cap) return old_cap;
    if (old_cap == 0) return need_cap;

    new_cap = old_cap;
    while (new_cap < need_cap) {
        if (new_cap > SIZE_MAX / 2) return need_cap;
        new_cap *= 2;
    }
    return new_cap;
}

static inline b32_t
tl__arr_reserve_impl(void **arrp, size_t elem_size, size_t need_cap) {
    tl__arr_hdr *arr;
    tl__arr_hdr *next;
    const tl_allocator *alloc;
    size_t old_total;
    size_t new_total;
    size_t new_cap;

    TLDS_ASSERT(arrp != NULL, "array pointer must not be NULL");
    TLDS_ASSERT(elem_size > 0, "element size must be greater than 0");

    arr = (tl__arr_hdr *)*arrp;
    if (arr == NULL) {
        arr = tl__arr_init_impl(NULL);
        if (arr == NULL) return 0;
        *arrp = arr;
    }
    if (need_cap <= arr->cap) return 1;

    new_cap = tl__arr_next_cap(arr->cap, need_cap);
    new_total = tl__arr_total_bytes(elem_size, new_cap);
    if (new_total == 0 && new_cap != 0) {
        TLDS_ASSERT(0, "array allocation size overflow");
        return 0;
    }

    alloc = tl__arr_allocator(arr);
    old_total = tl__arr_total_bytes(elem_size, arr->cap);
    next = (tl__arr_hdr *)alloc->realloc(alloc, arr, old_total, new_total);
    if (next == NULL && new_total != 0U) {
        TLDS__PRINT("[ERROR] failed to grow array to %zu elements", new_cap);
        return 0;
    }

    if (next != NULL) {
        next->cap = new_cap;
        next->alloc = alloc;
    }

    *arrp = next;
    return 1;
}

static inline b32_t
tl__arr_resize_impl(void **arrp, size_t elem_size, size_t new_len) {
    tl__arr_hdr *arr;

    if (!tl__arr_reserve_impl(arrp, elem_size, new_len)) return 0;
    arr = (tl__arr_hdr *)*arrp;
    if (arr != NULL) arr->len = new_len;
    return 1;
}

static inline b32_t
tl__arr_append_impl(void **arrp, const void *src, size_t count, size_t elem_size) {
    tl__arr_hdr *arr;
    size_t old_len;

    TLDS_ASSERT(arrp != NULL, "array pointer must not be NULL");
    TLDS_ASSERT(src != NULL || count == 0, "append source must not be NULL when count > 0");

    if (count == 0) return 1;

    arr = (tl__arr_hdr *)*arrp;
    old_len = arr ? arr->len : 0U;
    if (old_len > SIZE_MAX - count) {
        TLDS_ASSERT(0, "array length overflow");
        return 0;
    }
    if (!tl__arr_reserve_impl(arrp, elem_size, old_len + count)) return 0;

    arr = (tl__arr_hdr *)*arrp;
    memcpy(arr->data + old_len * elem_size, src, count * elem_size);
    arr->len = old_len + count;
    return 1;
}

static inline byte_t *
tl__arr_addnptr_impl(void **arrp, size_t elem_size, size_t count) {
    tl__arr_hdr *arr;
    size_t old_len;

    TLDS_ASSERT(arrp != NULL, "array pointer must not be NULL");

    arr = (tl__arr_hdr *)*arrp;
    if (arr == NULL) {
        arr = tl__arr_init_impl(NULL);
        if (arr == NULL) return NULL;
        *arrp = arr;
    }
    old_len = arr->len;
    if (count == 0) {
        return arr->data + old_len * elem_size;
    }
    if (old_len > SIZE_MAX - count) {
        TLDS_ASSERT(0, "array length overflow");
        return NULL;
    }
    if (!tl__arr_reserve_impl(arrp, elem_size, old_len + count)) return NULL;

    arr = (tl__arr_hdr *)*arrp;
    arr->len = old_len + count;
    return arr->data + old_len * elem_size;
}

static inline b32_t
tl__arr_insn_impl(void **arrp, size_t elem_size, size_t idx, size_t count, byte_t **out) {
    tl__arr_hdr *arr;
    byte_t *slot;

    TLDS_ASSERT(arrp != NULL, "array pointer must not be NULL");
    if (out) *out = NULL;

    arr = (tl__arr_hdr *)*arrp;
    if (arr == NULL) {
        arr = tl__arr_init_impl(NULL);
        if (arr == NULL) return 0;
        *arrp = arr;
    }
    TLDS_ASSERT(idx <= arr->len, "insert index out of bounds");

    if (count == 0) {
        if (out) *out = arr->data + idx * elem_size;
        return 1;
    }

    slot = tl__arr_addnptr_impl(arrp, elem_size, count);
    if (slot == NULL) return 0;

    arr = (tl__arr_hdr *)*arrp;
    memmove(arr->data + (idx + count) * elem_size,
            arr->data + idx * elem_size,
            (arr->len - idx - count) * elem_size);
    if (out) *out = arr->data + idx * elem_size;
    return 1;
}

static inline b32_t
tl__arr_pop_impl(void *arr, size_t elem_size, void *out) {
    tl__arr_hdr *hdr = (tl__arr_hdr *)arr;

    TLDS_ASSERT(hdr != NULL, "array must not be NULL");
    TLDS_ASSERT(out != NULL, "pop destination must not be NULL");
    TLDS_ASSERT(hdr->len > 0, "array is empty");

    hdr->len--;
    memcpy(out, hdr->data + hdr->len * elem_size, elem_size);
    return 1;
}

static inline b32_t
tl__arr_deln_impl(void *arr, size_t elem_size, size_t idx, size_t count) {
    tl__arr_hdr *hdr = (tl__arr_hdr *)arr;

    TLDS_ASSERT(hdr != NULL, "array must not be NULL");
    TLDS_ASSERT(idx <= hdr->len, "delete index out of bounds");
    TLDS_ASSERT(count <= hdr->len - idx, "delete count out of bounds");

    if (count == 0) return 1;

    memmove(hdr->data + idx * elem_size,
            hdr->data + (idx + count) * elem_size,
            (hdr->len - idx - count) * elem_size);
    hdr->len -= count;
    return 1;
}

static inline b32_t
tl__arr_del_swap_impl(void *arr, size_t elem_size, size_t idx) {
    tl__arr_hdr *hdr = (tl__arr_hdr *)arr;

    TLDS_ASSERT(hdr != NULL, "array must not be NULL");
    TLDS_ASSERT(idx < hdr->len, "delete index out of bounds");

    if (idx != hdr->len - 1U) {
        memcpy(hdr->data + idx * elem_size,
               hdr->data + (hdr->len - 1U) * elem_size,
               elem_size);
    }
    hdr->len--;
    return 1;
}

static inline void
tl__arr_clear_impl(void *arr) {
    tl__arr_hdr *hdr = (tl__arr_hdr *)arr;
    if (hdr != NULL) hdr->len = 0;
}

static inline void
tl__arr_free_impl(void **arrp, size_t elem_size) {
    tl__arr_hdr *arr;
    const tl_allocator *alloc;
    size_t total_size;

    TLDS_ASSERT(arrp != NULL, "array pointer must not be NULL");

    arr = (tl__arr_hdr *)*arrp;
    if (arr == NULL) return;

    alloc = tl__arr_allocator(arr);
    total_size = tl__arr_total_bytes(elem_size, arr->cap);
    alloc->free(alloc, arr, total_size);
    *arrp = NULL;
}

#define tl_arr_init(arr, allocator) \
    do {                            \
        TL_REQUIRE_LVALUE(arr);     \
        (arr) = (TL_TYPEOF(arr))tl__arr_init_impl((allocator)); \
    } while (0)

#define tl_arr_clear(arr) \
    do {                  \
        tl__arr_clear_impl((arr)); \
    } while (0)

#define tl_arr_free(arr) \
    do { \
        TL_REQUIRE_LVALUE(arr); \
        tl__arr_free_impl((void **)&(arr), sizeof((arr)->data[0])); \
    } while (0)

#define tl_arr_reserve(arr, n) \
    TLDS__EXPR( \
        TL_REQUIRE_LVALUE(arr); \
        tl__arr_reserve_impl((void **)&(arr), sizeof((arr)->data[0]), (size_t)(n)); \
    )

#define tl_arr_resize(arr, n) \
    TLDS__EXPR( \
        TL_REQUIRE_LVALUE(arr); \
        tl__arr_resize_impl((void **)&(arr), sizeof((arr)->data[0]), (size_t)(n)); \
    )

#define tl_arr_push(arr, value) \
    TLDS__EXPR( \
        TL_REQUIRE_LVALUE(arr); \
        TL_TYPEOF((arr)->data[0]) tl__value = (value); \
        tl__arr_append_impl((void **)&(arr), &tl__value, 1U, sizeof(tl__value)); \
    )

#define tl_arr_push_n(arr, ...) \
    TLDS__EXPR( \
        TL_REQUIRE_LVALUE(arr); \
        TL_TYPEOF((arr)->data[0]) tl__items[] = { __VA_ARGS__ }; \
        tl__arr_append_impl((void **)&(arr), tl__items, TL_COUNT_OF(tl__items), sizeof(tl__items[0])); \
    )

#define tl_arr_append(dst, src) \
    TLDS__EXPR( \
        TL_REQUIRE_LVALUE(dst); \
        const TL_TYPEOF((dst)->data[0]) *tl__src_data = tl_arr_data(src); \
        (void)tl__src_data; \
        tl__arr_append_impl((void **)&(dst), tl_arr_data(src), tl_arr_len(src), sizeof((dst)->data[0])); \
    )

#define tl_arr_addnptr(arr, n) \
    TLDS__EXPR( \
        TL_REQUIRE_LVALUE(arr); \
        (TL_TYPEOF(&(arr)->data[0]))tl__arr_addnptr_impl((void **)&(arr), sizeof((arr)->data[0]), (size_t)(n)); \
    )

#define tl_arr_addnidx(arr, n) \
    TLDS__EXPR( \
        TL_REQUIRE_LVALUE(arr); \
        size_t tl__old_len = tl_arr_len(arr); \
        tl__arr_addnptr_impl((void **)&(arr), sizeof((arr)->data[0]), (size_t)(n)) != NULL ? tl__old_len : (size_t)-1; \
    )

#define tl_arr_insn(arr, idx, n) \
    TLDS__EXPR( \
        TL_REQUIRE_LVALUE(arr); \
        byte_t *tl__slot = NULL; \
        tl__arr_insn_impl((void **)&(arr), sizeof((arr)->data[0]), (size_t)(idx), (size_t)(n), &tl__slot) \
            ? (TL_TYPEOF(&(arr)->data[0]))tl__slot \
            : NULL; \
    )

#define tl_arr_ins(arr, idx, value) \
    TLDS__EXPR( \
        TL_REQUIRE_LVALUE(arr); \
        TL_TYPEOF((arr)->data[0]) tl__value = (value); \
        TL_TYPEOF(&(arr)->data[0]) tl__slot = tl_arr_insn((arr), (idx), 1U); \
        if (tl__slot != NULL) *tl__slot = tl__value; \
        tl__slot != NULL; \
    )

#define tl_arr_pushp(arr) \
    tl_arr_addnptr((arr), 1U)

#define tl_arr_pop(arr, out_ptr) \
    TLDS__EXPR( \
        TL_TYPEOF(&(arr)->data[0]) tl__out = (out_ptr); \
        tl__arr_pop_impl((arr), sizeof((arr)->data[0]), tl__out); \
    )

#define tl_arr_del(arr, idx) \
    TLDS__EXPR( \
        tl__arr_deln_impl((arr), sizeof((arr)->data[0]), (size_t)(idx), 1U); \
    )

#define tl_arr_deln(arr, idx, n) \
    TLDS__EXPR( \
        tl__arr_deln_impl((arr), sizeof((arr)->data[0]), (size_t)(idx), (size_t)(n)); \
    )

#define tl_arr_del_swap(arr, idx) \
    TLDS__EXPR( \
        tl__arr_del_swap_impl((arr), sizeof((arr)->data[0]), (size_t)(idx)); \
    )

#endif /* TINYLIB_DATA_STRUCT_H */
