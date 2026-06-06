/* vim: set ft=c : -*- mode: c -*-
 * data_struct.h
 *   Hidden-header dynamic array utilities for GNU11.
 */
#ifndef TINYLIB_DATA_STRUCT_H
#define TINYLIB_DATA_STRUCT_H

#include "defs.h"
#include "c_ext.h"
#include "mem.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef TL_DS_DEBUG_PRINT
#include <stdio.h>
#define TL_DS__PRINT(fmt, ...) fprintf(stderr, "[TL_DS] %s:%d %s " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#define TL_DS__PRINT(fmt, ...)
#endif

#ifndef TL_DS_ASSERT
#define TL_DS_ASSERT(expr, msg, ...)                                                   \
    do {                                                                              \
        if (!(expr)) {                                                                \
            TL_DS__PRINT("[ERROR] Assertion failed (" #expr "): " msg, ##__VA_ARGS__); \
            TL_BREAKPOINT();                                                          \
            abort();                                                                  \
        }                                                                             \
    } while (0)
#endif

#if !TL_HAS_STATEMENT_EXPR
#error "data_struct.h currently requires GNU statement expressions."
#endif

#define TL_DS__EXPR(...) ({ __VA_ARGS__ })

#ifdef TL_DS_DEBUG
#define TL_DS__ARR_MAGIC UINT32_C(0xDA77A77A)
#endif

typedef struct TL__ArrHdr {
    size_t len;
    size_t cap;
    size_t elem_size;
    size_t align;
    TL_Allocator *alloc;
#ifdef TL_DS_DEBUG
    uint32_t magic;
#endif
} TL__ArrHdr;

typedef struct TL__ArrResult {
    void *data;
    b32_t ok;
} TL__ArrResult;

typedef struct TL__ArrPtrResult {
    void *data;
    byte_t *ptr;
    b32_t ok;
} TL__ArrPtrResult;

/*
 * Dynamic arrays are plain `T *` values. The metadata header lives immediately
 * before the returned data pointer, padded to the element alignment.
 */
#define TL_DECLARE_ARR_TYPE(Name, T) typedef T Name
#define TL_DEFINE_ARR_TYPE(Name, T) TL_DECLARE_ARR_TYPE(Name, T)
#define TL_DS__DECLARE_ARR_TYPE(Name, T) TL_DECLARE_ARR_TYPE(Name, T);

#define TL_DS_BASIC_ARR_TYPES(X)          \
    X(TL_ArrBool, bool)                  \
    X(TL_ArrChar, char)                  \
    X(TL_ArrSChar, signed char)          \
    X(TL_ArrUChar, unsigned char)        \
    X(TL_ArrShort, short)                \
    X(TL_ArrUShort, unsigned short)      \
    X(TL_ArrInt, int)                    \
    X(TL_ArrUInt, unsigned int)          \
    X(TL_ArrLong, long)                  \
    X(TL_ArrULong, unsigned long)        \
    X(TL_ArrLLong, long long)            \
    X(TL_ArrULLong, unsigned long long)  \
    X(TL_ArrSize, size_t)                \
    X(TL_ArrPtrdiff, ptrdiff_t)          \
    X(TL_ArrFloat, float)                \
    X(TL_ArrDouble, double)              \
    X(TL_ArrLDouble, long double)        \
    X(TL_ArrByte, byte_t)                \
    X(TL_ArrB8, b8_t)                    \
    X(TL_ArrB32, b32_t)                  \
    X(TL_ArrI8, i8_t)                    \
    X(TL_ArrI16, i16_t)                  \
    X(TL_ArrI32, i32_t)                  \
    X(TL_ArrI64, i64_t)                  \
    X(TL_ArrU8, u8_t)                    \
    X(TL_ArrU16, u16_t)                  \
    X(TL_ArrU32, u32_t)                  \
    X(TL_ArrU64, u64_t)

#ifndef TL_DS_NO_BASIC_TYPES
TL_DS_BASIC_ARR_TYPES(TL_DS__DECLARE_ARR_TYPE)
#endif

#define TL_DS__ALIGNOF_VALUE(value) ((size_t)__alignof__(value))
#define TL_DS__HEADER_SIZE(align) tl_align_up(sizeof(TL__ArrHdr), (align))
#define TL_DS__HDR_WITH_ALIGN(arr, align) \
    ((TL__ArrHdr *)((byte_t *)(arr) - TL_DS__HEADER_SIZE((align))))
#define TL_DS__HDR(arr) TL_DS__HDR_WITH_ALIGN((arr), TL_DS__ALIGNOF_VALUE(*(arr)))

#ifdef TL_DS_DEBUG
TL_ATTR_MAYBE_UNUSED
static inline
void
tl__arr_check(const TL__ArrHdr *hdr)
{
    TL_DS_ASSERT(hdr != NULL, "array header must not be NULL");
    TL_DS_ASSERT(hdr->magic == TL_DS__ARR_MAGIC, "invalid dynamic array pointer");
}
#else
TL_ATTR_MAYBE_UNUSED
static inline
void
tl__arr_check(const TL__ArrHdr *hdr)
{
    (void)hdr;
}
#endif

TL_ATTR_MAYBE_UNUSED
static inline
TL_Allocator *
tl__arr_allocator(const TL__ArrHdr *hdr)
{
    return (hdr && hdr->alloc) ? hdr->alloc : (TL_Allocator *)&tl_default_allocator;
}

TL_ATTR_MAYBE_UNUSED
static inline
size_t
tl__arr_total_size(size_t elem_size, size_t align, size_t cap)
{
    size_t header_size;
    size_t data_size;

    header_size = TL_DS__HEADER_SIZE(align);
    if (header_size == 0) return 0;
    if (cap > SIZE_MAX / elem_size) return 0;
    data_size = cap * elem_size;
    if (data_size > SIZE_MAX - header_size) return 0;
    return header_size + data_size;
}

TL_ATTR_MAYBE_UNUSED
static inline
void *
tl__arr_data_from_hdr(TL__ArrHdr *hdr)
{
    return (byte_t *)hdr + TL_DS__HEADER_SIZE(hdr->align);
}

TL_ATTR_MAYBE_UNUSED
static inline
TL__ArrResult
tl__arr_init_impl(TL_Allocator *alloc, size_t elem_size, size_t align)
{
    TL__ArrResult result = {0};
    TL_Allocator *resolved = alloc ? alloc : (TL_Allocator *)&tl_default_allocator;
    size_t total_size = tl__arr_total_size(elem_size, align, 0);
    TL__ArrHdr *hdr;

    TL_DS_ASSERT(elem_size > 0, "element size must be greater than 0");
    TL_DS_ASSERT(tl_is_power_of_two(align), "array alignment must be a power of two");
    if (total_size == 0) return result;

    hdr = (TL__ArrHdr *)tl_allocator_alloc_aligned(resolved, total_size, align);
    if (!hdr) return result;

    hdr->len = 0;
    hdr->cap = 0;
    hdr->elem_size = elem_size;
    hdr->align = align;
    hdr->alloc = resolved;
#ifdef TL_DS_DEBUG
    hdr->magic = TL_DS__ARR_MAGIC;
#endif

    result.data = tl__arr_data_from_hdr(hdr);
    result.ok = 1;
    return result;
}

TL_ATTR_MAYBE_UNUSED
static inline
size_t
tl__arr_next_cap(size_t old_cap, size_t need_cap)
{
    if (need_cap <= old_cap) return old_cap;
    if (old_cap == 0) return need_cap;
    if (old_cap > SIZE_MAX / 2U) return need_cap;
    return TL_MAX(need_cap, old_cap * 2U);
}

TL_ATTR_MAYBE_UNUSED
static inline
TL__ArrResult
tl__arr_reserve_impl(void *arr,
                     TL_Allocator *init_alloc,
                     size_t elem_size,
                     size_t align,
                     size_t need_cap)
{
    TL__ArrResult result = {0};
    TL__ArrHdr *hdr;
    TL_Allocator *alloc;
    size_t old_total;
    size_t new_total;
    size_t new_cap;

    if (!arr) {
        result = tl__arr_init_impl(init_alloc, elem_size, align);
        if (!result.ok) return result;
        arr = result.data;
    }

    hdr = TL_DS__HDR_WITH_ALIGN(arr, align);
    tl__arr_check(hdr);
    TL_DS_ASSERT(hdr->elem_size == elem_size, "array element size mismatch");
    TL_DS_ASSERT(hdr->align == align, "array alignment mismatch");

    if (need_cap <= hdr->cap) {
        result.data = arr;
        result.ok = 1;
        return result;
    }

    new_cap = tl__arr_next_cap(hdr->cap, need_cap);
    old_total = tl__arr_total_size(elem_size, align, hdr->cap);
    new_total = tl__arr_total_size(elem_size, align, new_cap);
    if (old_total == 0 || new_total == 0) return result;

    alloc = tl__arr_allocator(hdr);
    hdr = (TL__ArrHdr *)tl_allocator_realloc_aligned(alloc, hdr, old_total, new_total, align);
    if (!hdr) return result;

    hdr->cap = new_cap;
    hdr->elem_size = elem_size;
    hdr->align = align;
    hdr->alloc = alloc;
#ifdef TL_DS_DEBUG
    hdr->magic = TL_DS__ARR_MAGIC;
#endif

    result.data = tl__arr_data_from_hdr(hdr);
    result.ok = 1;
    return result;
}

TL_ATTR_MAYBE_UNUSED
static inline
TL__ArrResult
tl__arr_resize_impl(void *arr, size_t elem_size, size_t align, size_t new_len)
{
    TL__ArrResult result = tl__arr_reserve_impl(arr, NULL, elem_size, align, new_len);
    TL__ArrHdr *hdr;

    if (!result.ok) return result;
    hdr = TL_DS__HDR_WITH_ALIGN(result.data, align);
    hdr->len = new_len;
    return result;
}

TL_ATTR_MAYBE_UNUSED
static inline
TL__ArrResult
tl__arr_append_impl(void *arr, const void *src, size_t count, size_t elem_size, size_t align)
{
    TL__ArrResult result = {0};
    TL__ArrHdr *hdr;
    size_t old_len;

    TL_DS_ASSERT(src != NULL || count == 0, "append source must not be NULL when count > 0");

    old_len = arr ? TL_DS__HDR_WITH_ALIGN(arr, align)->len : 0U;
    if (old_len > SIZE_MAX - count) return result;

    result = tl__arr_reserve_impl(arr, NULL, elem_size, align, old_len + count);
    if (!result.ok) return result;

    hdr = TL_DS__HDR_WITH_ALIGN(result.data, align);
    if (count > 0) {
        memcpy((byte_t *)result.data + old_len * elem_size, src, count * elem_size);
        hdr->len = old_len + count;
    }
    return result;
}

TL_ATTR_MAYBE_UNUSED
static inline
TL__ArrPtrResult
tl__arr_addnptr_impl(void *arr, size_t elem_size, size_t align, size_t count)
{
    TL__ArrPtrResult result = {0};
    TL__ArrResult reserve;
    TL__ArrHdr *hdr;
    size_t old_len;

    old_len = arr ? TL_DS__HDR_WITH_ALIGN(arr, align)->len : 0U;
    if (old_len > SIZE_MAX - count) return result;

    reserve = tl__arr_reserve_impl(arr, NULL, elem_size, align, old_len + count);
    if (!reserve.ok) return result;

    hdr = TL_DS__HDR_WITH_ALIGN(reserve.data, align);
    hdr->len = old_len + count;

    result.data = reserve.data;
    result.ptr = (byte_t *)reserve.data + old_len * elem_size;
    result.ok = 1;
    return result;
}

TL_ATTR_MAYBE_UNUSED
static inline
TL__ArrPtrResult
tl__arr_insn_impl(void *arr, size_t elem_size, size_t align, size_t idx, size_t count)
{
    TL__ArrPtrResult result = {0};
    TL__ArrHdr *hdr;
    size_t old_len;

    old_len = arr ? TL_DS__HDR_WITH_ALIGN(arr, align)->len : 0U;
    TL_DS_ASSERT(idx <= old_len, "insert index out of bounds");

    result = tl__arr_addnptr_impl(arr, elem_size, align, count);
    if (!result.ok) return result;

    hdr = TL_DS__HDR_WITH_ALIGN(result.data, align);
    memmove((byte_t *)result.data + (idx + count) * elem_size,
            (byte_t *)result.data + idx * elem_size,
            (old_len - idx) * elem_size);
    result.ptr = (byte_t *)result.data + idx * elem_size;
    (void)hdr;
    return result;
}

TL_ATTR_MAYBE_UNUSED
static inline
b32_t
tl__arr_pop_impl(void *arr, size_t elem_size, size_t align, void *out)
{
    TL__ArrHdr *hdr;

    TL_DS_ASSERT(arr != NULL, "array must not be NULL");
    TL_DS_ASSERT(out != NULL, "pop destination must not be NULL");

    hdr = TL_DS__HDR_WITH_ALIGN(arr, align);
    tl__arr_check(hdr);
    TL_DS_ASSERT(hdr->len > 0, "array is empty");

    hdr->len--;
    memcpy(out, (byte_t *)arr + hdr->len * elem_size, elem_size);
    return 1;
}

TL_ATTR_MAYBE_UNUSED
static inline
b32_t
tl__arr_deln_impl(void *arr, size_t elem_size, size_t align, size_t idx, size_t count)
{
    TL__ArrHdr *hdr;

    TL_DS_ASSERT(arr != NULL, "array must not be NULL");
    hdr = TL_DS__HDR_WITH_ALIGN(arr, align);
    tl__arr_check(hdr);
    TL_DS_ASSERT(idx <= hdr->len, "delete index out of bounds");
    TL_DS_ASSERT(count <= hdr->len - idx, "delete count out of bounds");

    if (count == 0) return 1;
    memmove((byte_t *)arr + idx * elem_size,
            (byte_t *)arr + (idx + count) * elem_size,
            (hdr->len - idx - count) * elem_size);
    hdr->len -= count;
    return 1;
}

TL_ATTR_MAYBE_UNUSED
static inline
b32_t
tl__arr_del_swap_impl(void *arr, size_t elem_size, size_t align, size_t idx)
{
    TL__ArrHdr *hdr;

    TL_DS_ASSERT(arr != NULL, "array must not be NULL");
    hdr = TL_DS__HDR_WITH_ALIGN(arr, align);
    tl__arr_check(hdr);
    TL_DS_ASSERT(idx < hdr->len, "delete index out of bounds");

    if (idx != hdr->len - 1U) {
        memcpy((byte_t *)arr + idx * elem_size,
               (byte_t *)arr + (hdr->len - 1U) * elem_size,
               elem_size);
    }
    hdr->len--;
    return 1;
}

TL_ATTR_MAYBE_UNUSED
static inline
void
tl__arr_clear_impl(void *arr, size_t align)
{
    TL__ArrHdr *hdr;
    if (!arr) return;
    hdr = TL_DS__HDR_WITH_ALIGN(arr, align);
    tl__arr_check(hdr);
    hdr->len = 0;
}

TL_ATTR_MAYBE_UNUSED
static inline
void
tl__arr_free_impl(void *arr, size_t elem_size, size_t align)
{
    TL__ArrHdr *hdr;
    TL_Allocator *alloc;
    size_t total_size;

    if (!arr) return;
    hdr = TL_DS__HDR_WITH_ALIGN(arr, align);
    tl__arr_check(hdr);
    alloc = tl__arr_allocator(hdr);
    total_size = tl__arr_total_size(elem_size, align, hdr->cap);
    if (total_size == 0) return;
    tl_allocator_free_aligned(alloc, hdr, total_size, align);
}

/* Core metadata and element access. */
#define tl_arr_len(arr)   ((arr) ? TL_DS__HDR(arr)->len : 0U)
#define tl_arr_cap(arr)   ((arr) ? TL_DS__HDR(arr)->cap : 0U)
#define tl_arr_empty(arr) (tl_arr_len(arr) == 0U)
#define tl_arr_data(arr) ((const TL_TYPEOF(*(arr)) *)(arr))
#define tl_arr_data_mut(arr) (arr)

#define tl_arr_at(arr, idx) \
    (((arr) != NULL && (size_t)(idx) < tl_arr_len(arr)) ? &(arr)[(idx)] : NULL)
#define tl_arr_at_mut(arr, idx) tl_arr_at((arr), (idx))
#define tl_arr_front(arr) tl_arr_at((arr), 0)
#define tl_arr_front_mut(arr) tl_arr_at_mut((arr), 0)
#define tl_arr_back(arr) \
    (((arr) != NULL && tl_arr_len(arr) > 0U) ? &(arr)[tl_arr_len(arr) - 1U] : NULL)
#define tl_arr_back_mut(arr) tl_arr_back((arr))

/* Core lifetime and capacity operations. */
#define tl_arr_init(arr, allocator) \
    do { \
        TL_REQUIRE_LVALUE(arr); \
        TL__ArrResult tl__r = tl__arr_init_impl((allocator), sizeof(*(arr)), TL_DS__ALIGNOF_VALUE(*(arr))); \
        (arr) = (TL_TYPEOF(arr))tl__r.data; \
    } while (0)

#define tl_arr_clear(arr) \
    do { \
        tl__arr_clear_impl((arr), TL_DS__ALIGNOF_VALUE(*(arr))); \
    } while (0)

#define tl_arr_free(arr) \
    do { \
        TL_REQUIRE_LVALUE(arr); \
        tl__arr_free_impl((arr), sizeof(*(arr)), TL_DS__ALIGNOF_VALUE(*(arr))); \
        (arr) = NULL; \
    } while (0)

#define tl_arr_reserve(arr, n) \
    TL_DS__EXPR( \
        TL_REQUIRE_LVALUE(arr); \
        TL__ArrResult tl__r = tl__arr_reserve_impl((arr), NULL, sizeof(*(arr)), TL_DS__ALIGNOF_VALUE(*(arr)), (size_t)(n)); \
        if (tl__r.ok) (arr) = (TL_TYPEOF(arr))tl__r.data; \
        tl__r.ok; \
    )

#define tl_arr_resize(arr, n) \
    TL_DS__EXPR( \
        TL_REQUIRE_LVALUE(arr); \
        TL__ArrResult tl__r = tl__arr_resize_impl((arr), sizeof(*(arr)), TL_DS__ALIGNOF_VALUE(*(arr)), (size_t)(n)); \
        if (tl__r.ok) (arr) = (TL_TYPEOF(arr))tl__r.data; \
        tl__r.ok; \
    )

/* Append and insertion helpers. */
#define tl_arr_push(arr, value) \
    TL_DS__EXPR( \
        TL_REQUIRE_LVALUE(arr); \
        TL_TYPEOF(*(arr)) tl__value = (value); \
        TL__ArrResult tl__r = tl__arr_append_impl((arr), &tl__value, 1U, sizeof(tl__value), TL_DS__ALIGNOF_VALUE(tl__value)); \
        if (tl__r.ok) (arr) = (TL_TYPEOF(arr))tl__r.data; \
        tl__r.ok; \
    )

#define tl_arr_push_n(arr, ...) \
    TL_DS__EXPR( \
        TL_REQUIRE_LVALUE(arr); \
        TL_TYPEOF(*(arr)) tl__items[] = { __VA_ARGS__ }; \
        TL__ArrResult tl__r = tl__arr_append_impl((arr), tl__items, TL_COUNT_OF(tl__items), sizeof(tl__items[0]), TL_DS__ALIGNOF_VALUE(tl__items[0])); \
        if (tl__r.ok) (arr) = (TL_TYPEOF(arr))tl__r.data; \
        tl__r.ok; \
    )

#define tl_arr_append(dst, src) \
    TL_DS__EXPR( \
        TL_REQUIRE_LVALUE(dst); \
        const TL_TYPEOF(*(dst)) *tl__src_data = (src); \
        (void)tl__src_data; \
        TL__ArrResult tl__r = tl__arr_append_impl((dst), (src), tl_arr_len(src), sizeof(*(dst)), TL_DS__ALIGNOF_VALUE(*(dst))); \
        if (tl__r.ok) (dst) = (TL_TYPEOF(dst))tl__r.data; \
        tl__r.ok; \
    )

#define tl_arr_addnptr(arr, n) \
    TL_DS__EXPR( \
        TL_REQUIRE_LVALUE(arr); \
        TL__ArrPtrResult tl__r = tl__arr_addnptr_impl((arr), sizeof(*(arr)), TL_DS__ALIGNOF_VALUE(*(arr)), (size_t)(n)); \
        if (tl__r.ok) (arr) = (TL_TYPEOF(arr))tl__r.data; \
        (TL_TYPEOF(arr))tl__r.ptr; \
    )

#define tl_arr_addnidx(arr, n) \
    TL_DS__EXPR( \
        TL_REQUIRE_LVALUE(arr); \
        size_t tl__old_len = tl_arr_len(arr); \
        TL__ArrPtrResult tl__r = tl__arr_addnptr_impl((arr), sizeof(*(arr)), TL_DS__ALIGNOF_VALUE(*(arr)), (size_t)(n)); \
        if (tl__r.ok) (arr) = (TL_TYPEOF(arr))tl__r.data; \
        tl__r.ok ? tl__old_len : (size_t)-1; \
    )

#define tl_arr_insn(arr, idx, n) \
    TL_DS__EXPR( \
        TL_REQUIRE_LVALUE(arr); \
        TL__ArrPtrResult tl__r = tl__arr_insn_impl((arr), sizeof(*(arr)), TL_DS__ALIGNOF_VALUE(*(arr)), (size_t)(idx), (size_t)(n)); \
        if (tl__r.ok) (arr) = (TL_TYPEOF(arr))tl__r.data; \
        (TL_TYPEOF(arr))tl__r.ptr; \
    )

#define tl_arr_ins(arr, idx, value) \
    TL_DS__EXPR( \
        TL_REQUIRE_LVALUE(arr); \
        TL_TYPEOF(*(arr)) tl__value = (value); \
        TL_TYPEOF(arr) tl__slot = tl_arr_insn((arr), (idx), 1U); \
        if (tl__slot != NULL) *tl__slot = tl__value; \
        tl__slot != NULL; \
    )

#define tl_arr_pushp(arr) tl_arr_addnptr((arr), 1U)

/* Removal helpers. */
#define tl_arr_pop(arr, out_ptr) \
    TL_DS__EXPR( \
        TL_TYPEOF(arr) tl__out = (out_ptr); \
        tl__arr_pop_impl((arr), sizeof(*(arr)), TL_DS__ALIGNOF_VALUE(*(arr)), tl__out); \
    )

#define tl_arr_del(arr, idx) \
    TL_DS__EXPR( \
        tl__arr_deln_impl((arr), sizeof(*(arr)), TL_DS__ALIGNOF_VALUE(*(arr)), (size_t)(idx), 1U); \
    )

#define tl_arr_deln(arr, idx, n) \
    TL_DS__EXPR( \
        tl__arr_deln_impl((arr), sizeof(*(arr)), TL_DS__ALIGNOF_VALUE(*(arr)), (size_t)(idx), (size_t)(n)); \
    )

#define tl_arr_del_swap(arr, idx) \
    TL_DS__EXPR( \
        tl__arr_del_swap_impl((arr), sizeof(*(arr)), TL_DS__ALIGNOF_VALUE(*(arr)), (size_t)(idx)); \
    )

#ifdef TL_DS_SHORT_NAMES
/* Optional short aliases. */
#define ArrBool        TL_ArrBool
#define ArrChar        TL_ArrChar
#define ArrSChar       TL_ArrSChar
#define ArrUChar       TL_ArrUChar
#define ArrShort       TL_ArrShort
#define ArrUShort      TL_ArrUShort
#define ArrInt         TL_ArrInt
#define ArrUInt        TL_ArrUInt
#define ArrLong        TL_ArrLong
#define ArrULong       TL_ArrULong
#define ArrLLong       TL_ArrLLong
#define ArrULLong      TL_ArrULLong
#define ArrSize        TL_ArrSize
#define ArrPtrdiff     TL_ArrPtrdiff
#define ArrFloat       TL_ArrFloat
#define ArrDouble      TL_ArrDouble
#define ArrLDouble     TL_ArrLDouble
#define ArrByte        TL_ArrByte
#define ArrB8          TL_ArrB8
#define ArrB32         TL_ArrB32
#define ArrI8          TL_ArrI8
#define ArrI16         TL_ArrI16
#define ArrI32         TL_ArrI32
#define ArrI64         TL_ArrI64
#define ArrU8          TL_ArrU8
#define ArrU16         TL_ArrU16
#define ArrU32         TL_ArrU32
#define ArrU64         TL_ArrU64
#define arr_len        tl_arr_len
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

#endif /* TINYLIB_DATA_STRUCT_H */
