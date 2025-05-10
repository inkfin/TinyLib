/// dyn_arr.c
///
/// Tiny C Lib dynamic array implementation
///

#ifndef TINYLIB_DYN_ARR_H
#define TINYLIB_DYN_ARR_H

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "logging.h"

/* Configurations */
#define TL_DYN_ARRAY_IMPL_NOMACRO 0
#define TL_DYN_ARRAY_IMPL_DEFAULT 1
#define TL_DYN_ARRAY_IMPL_INPLACE_MACRO 2

#ifndef TL_DYN_ARR_IMPL
#define TL_DYN_ARR_IMPL TL_DYN_ARRAY_IMPL_INPLACE_MACRO
#endif // TL_DYN_ARR_IMPL

/* Declearations */
#ifdef __cplusplus
extern "C" {
#endif

/// Dynamic Array Interface
/// =========================
///
/// members:
/// - len: current length of the array
/// - cap: allocated length of the array
/// - data: pointer to the array data
///
/// Example:
/// typedef struct {
///     size_t len;
///     size_t cap;
///     _T* data;
/// } dyn_arr;
///
/// errno_t arr_push   (dyn_arr* arr, _T* item);
/// errno_t arr_push_n (dyn_arr* arr, _T* item1, _T* item2, ...);
/// errno_t arr_append (dyn_arr* arr, _T* item);
/// errno_t arr_free   (dyn_arr* arr);
///
/// Inplace expansion macros:
/// void    tl_arr_push_in (_arr, ...)
/// void    tl_arr_free_n  (_arr, ...)

#if TL_DYN_ARR_IMPL >= TL_DYN_ARRAY_IMPL_DEFAULT

#define TL_DECLEAR_ARRAY(_name, _type) \
    typedef struct _name##_t {         \
        size_t len;                    \
        size_t cap;                    \
        _type* data;                   \
    } _name##_t;

#if TL_DYN_ARR_IMPL >= TL_DYN_ARRAY_IMPL_INPLACE_MACRO

#define tl_arr_push_in(_arr, ...)                            \
    do {                                                     \
        TL_FOREACH_TWO_PARAM(tl_arr_push, _arr, __VA_ARGS__) \
    } while (0)

#define tl_arr_free_n(_arr, ...)                          \
    do {                                                  \
        TL_FOREACH_ONE_PARAM(arr_free, _arr, __VA_ARGS__) \
    } while (0)

#endif // TL_DYN_ARR_IMPL >= TL_DYN_ARRAY_IMPL_INPLACE_MACRO

#define tl_arr_push(arr, pnew) \
    tl__arr_push_impl(         \
        (void**)&(arr).data,   \
        &(arr).len,            \
        &(arr).cap,            \
        sizeof((arr).data[0]), \
        (const void*)pnew)

#define tl_arr_push_n(arr, ...)       \
    tl__arr_push_n_impl(              \
        (void**)&(arr).data,          \
        &(arr).len,                   \
        &(arr).cap,                   \
        sizeof(arr).data[0],          \
        TL_NUM_VA_ARGS_(__VA_ARGS__), \
        __VA_ARGS__)

#define tl_arr_append(arr, arr_other)          \
    tl__arr_append_impl(                       \
        (void**)&(arr).data,                   \
        &(arr).len,                            \
        &(arr).cap,                            \
        sizeof((arr).data[0]),                 \
        (const void* const*)&(arr_other).data, \
        &(arr_other).len,                      \
        &(arr_other).cap,                      \
        sizeof((arr_other).data[0]))

#define tl_arr_free(arr)     \
    tl__arr_free_impl(       \
        (void**)&(arr).data, \
        &(arr).len,          \
        &(arr).cap)

#endif // TL_DYN_ARR_IMPL >= TL_DYN_ARRAY_IMPL_DEFAULT

static inline errno_t tl__arr_push_impl(
    void**      pdata,
    size_t*     plen,
    size_t*     pcap,
    size_t      item_size,
    const void* item)
{
    if (*plen + 1 > *pcap) {
        size_t new_cap = *pcap ? *pcap * 2 : 1;
        void*  new_data = realloc(*pdata, new_cap * item_size);
        if (!new_data) {
            TL_LOG(TL_ERROR, "Array realloc failed when push");
            return ENOMEM;
        }
        *pdata = new_data;
        *pcap = new_cap;
    }

    memcpy((char*)*pdata + (*plen * item_size), item, item_size);
    (*plen)++;

    return 0;
}

static inline errno_t tl__arr_push_n_impl(
    void**  pdata,
    size_t* plen,
    size_t* pcap,
    size_t  item_size,
    size_t  n_items,
    ...)
{
    size_t needed = *plen + n_items;
    if (needed > *pcap) {
        size_t new_cap = *pcap ? *pcap * 2 : 1;
        while (new_cap < needed) {
            new_cap = new_cap * 2;
        }
        void* new_data = realloc(*pdata, new_cap * item_size);
        if (!new_data) {
            TL_LOG(TL_ERROR, "Array realloc failed when push n");
            return ENOMEM;
        }
        *pdata = new_data;
        *pcap = new_cap;
    }

    va_list ap;
    va_start(ap, n_items);
    for (size_t i = 0; i < n_items; i++) {
        const void* elem_ptr = va_arg(ap, const void*);
        memcpy(
            (char*)*pdata + ((*plen + i) * item_size),
            elem_ptr,
            item_size);
    }
    va_end(ap);

    *plen = needed;
    return 0;
}

static inline errno_t tl__arr_append_impl(
    void**             pdata,
    size_t*            plen,
    size_t*            pcap,
    size_t             item_size,
    const void* const* pdata_other,
    const size_t*      plen_other,
    const size_t*      pcap_other,
    const size_t       item_size_other)
{
    assert(item_size == item_size_other && "array append item size doesn't match");

    if (*plen + *plen_other > *pcap) {
        size_t new_cap = *pcap ? *pcap * 2 : 1;
        while (new_cap < *plen + *plen_other) {
            new_cap = new_cap * 2;
        }
        void* new_data = realloc(*pdata, new_cap * item_size);
        if (!new_data) {
            TL_LOG(TL_ERROR, "Array realloc failed when append");
            return ENOMEM;
        }
        *pdata = new_data;
        *pcap = new_cap;
    }

    memcpy(
        (char*)*pdata + (*plen * item_size),
        *pdata_other,
        *plen_other * item_size);

    *plen += *plen_other;
    return 0;
}

static inline errno_t tl__arr_free_impl(
    void**  pdata,
    size_t* plen,
    size_t* pcap)
{
    free(*pdata);
    *pdata = NULL;
    *plen = 0;
    *pcap = 0;
    return 0;
}

#ifdef __cplusplus
}
#endif

#endif // TINYLIB_DYN_ARR_H
