/// dyn_arr.c
///
/// Tiny C Lib dynamic array implementation that uses C23 modern features.
///
/// references:
/// - [stb]: <https://github.com/nothings/stb>

#ifndef TINYLIB_DATA_STRUCT_H
#define TINYLIB_DATA_STRUCT_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

///
/// Configurations
/// ================
///

#if !defined(TLDS_REALLOC) && !defined(TLDS_FREE)
#include <stdlib.h>
#define TLDS_REALLOC(c, p, s) realloc(p, s)
#define TLDS_FREE(c, p) free(p)
#endif

/// Short name such as arr_push is used by default,
#ifndef TLDS_NO_SHORT_NAME
#define arrlen tlds_arrlen
#define arrlenu tlds_arrlenu
#define arrcap tlds_arrcap
#define arrsetlen tlds_arrsetlen
#define arrsetcap tlds_arrsetcap
#define arrpop tlds_arrpop
#define arrpush tlds_arrpush
#define arrpush_n tlds_arrpush_n
#define arrappend tlds_arrappend
#define arrins tlds_arrins
#define arrinsn tlds_arrinsn
#define arraddnptr tlds_arraddnptr
#define arraddnidx tlds_arraddnidx
#define arrdel tlds_arrdel
#define arrdeln tlds_arrdeln
#define arrdelswap tlds_arrdelswap
#endif

///
/// Dynamic Array Interface
/// =========================
///

typedef struct {
    size_t len;
    size_t cap;
} TLDS_array_header;

///
/// notes:
/// - We use static inline functions to implement the dynamic array interface.
///   No extra implementation file is needed.
/// - stb library uses a header technique to store the array length and capacity
///   'before' the array data. Which is perfect for C since it uses pointers to
///   access arrays. The drawback is that we need to use macros to access the
///   metadata every time, which can be a bit annoying.
///
/// members:
/// - len: current length of the array
/// - cap: allocated length of the array
/// - [...]: allocated data of the array
///
/// usage:
///
///  ptrdiff_t arrlen(_T* arr);
///              Returns the number of elements in the array. (tlds_arrlen)
///
///     size_t arrlenu(T* arr);
///              Returns the number of elements in the array as an unsigned
///              type. (tlds_arrlenu)
///
///     size_t arrcap(T* arr);
///              Returns the number of total elements the array can contain
///              without needing to be reallocated. (tlds_arrcap)
///
///     size_t arrsetlen(T* arr, int n);
///              Changes the length of the array to n. Allocates uninitialized
///              slots at the end if necessary. (tlds_arrsetlen)
///
///      void* arrsetcap(T* arr, int n);
///              Sets the length of allocated storage to at least n. It will not
///              change the length of the array. If n is less than the current
///              length, it will not do anything.
///              Returns the new array pointer. (tlds_arrsetcap)
///
///          T arrpop(T* arr);
///              Removes the final element of the array and returns it.
///              (tlds_arrpop)
///
///          T arrpush(T* arr, T v);
///              Appends the item v to the end of array arr. Returns v.
///              (tlds_arrpush)
///
///       void arrpush_n(T* arr, T v1, ...);
///              Append multiple items to the end of the array. (tlds_arrpush_n)
///
///     size_t arrappend(T* a, T* b);
///              Appends array b to the end of array a. Returns the new length.
///              (tlds_arrappend)
///
///          T arrins(T* arr, int i, T v);
///              Inserts the item v into the middle of array arr, into arr[i],
///              moving the rest of the array over. Returns v. (tlds_arrins)
///
///      void* arrinsn(T* arr, int i, int n);
///              Inserts n uninitialized items into array arr starting at
///              arr[i], moving the rest of the array over. Returns a pointer to
///              the first uninitialized item added. (tlds_arrinsn)
///
///         T* arraddnptr(T* arr, int n);
///              Appends n uninitialized items onto array at the end.
///              Returns a pointer to the first uninitialized item added.
///              (tlds_arraddnptr)
///
///     size_t arraddnidx(T* arr, int n);
///              Appends n uninitialized items onto array at the end.
///              Returns the index of the first uninitialized item added.
///              (tlds_arraddnidx)
///
///     size_t arrdel(T* arr, int i);
///              Deletes the element at arr[i], moving the rest of the array
///              over. Returns the new length of the array. (tlds_arrdel)
///
///     size_t arrdeln(T* arr, int i, int n);
///              Deletes n elements starting at arr[i], moving the rest of the
///              array over. Returns the new length of the array. (tlds_arrdeln)
///
///       void arrdelswap(T* arr, int i);
///              Deletes the element at arr[i], replacing it with the element
///              from the end of the array. O(1) performance. (tlds_arrdelswap)

///
/// Implementation: Macro interface
/// - ripped off from the stb library
///

// simple utility macros to access the array metadata
// clang-format off

// getters
#define tlds_header(arr)  ((TLDS_array_header*)(arr) - 1)
#define tlds_arrcap(arr)  ((arr) ? tlds_header(arr)->cap : 0)
#define tlds_arrlen(arr)  ((arr) ? (ptrdiff_t) tlds_header(arr)->len : 0)
#define tlds_arrlenu(arr) ((arr) ?             tlds_header(arr)->len : 0)
#define tlds_arrlast(arr) ((arr)[tlds_header(arr)->len-1])

// setters
#define tlds_arrsetcap(arr, n)  (tlds_arrgrow(arr, 0, n))
#define tlds_arrsetlen(arr, n)  ((tlds_arrcap(arr) < (size_t)(n)             \
                                   ? tlds_arrsetcap((arr), (size_t)(n)), 0 \
                                   : 0),                                   \
                                 (arr) ? tlds_header(arr)->len = (size_t)(n) : 0)

// manipulations
#define tlds_arrpop(arr)        (tlds_header(arr)->len--, (arr)[tlds_header(arr)->len])
#define tlds_arrpush(arr, v)    (tlds_arrmaybegrow((arr), 1), \
                                 (arr)[tlds_header(arr)->len++] = (v))
#define tlds_arrappend(a, b)    (tlds_arrmaybegrow((a), tlds_arrlen(b)), \
                                 memcpy(&(a)[tlds_header(a)->len], (b),  \
                                     sizeof((b)[0]) * tlds_arrlen(b)),   \
                                 tlds_header(a)->len += tlds_arrlen(b))
#define tlds_arraddnptr(arr, n) (tlds_arrmaybegrow(arr,n),                                               \
                                 (n) ? (tlds_header(arr)->len += (n), &(arr)[tlds_header(arr)->len-(n)]) \
                                 : (arr))
#define tlds_arraddnidx(arr, n) (tlds_arrmaybegrow((arr),n),                                     \
                                 (n) ? (tlds_header(arr)->len += (n), tlds_header(arr)->len-(n)) \
                                 : tlds_arrlen(arr))
#define tlds_arrins(arr, i, v)  (tlds_arrinsn((arr),(i),1), (arr)[i]=(v))
#define tlds_arrinsn(arr, i, n) (tlds_arraddnidx((arr),(n)),         \
                                 memmove(&(arr)[(i)+(n)], &(arr)[i], \
                                   sizeof((arr)[0]) * (tlds_header(arr)->len-(n)-(i))))

#define tlds_arrfree(arr)       ((void) ((arr) ? TLDS_FREE(NULL,tlds_header(arr)) : (void)0), (arr)=NULL)
#define tlds_arrdel(arr, i)     tlds_arrdeln(arr,i,1)
#define tlds_arrdeln(arr, i, n) (memmove(&(arr)[i], &(arr)[(i)+(n)], \
                                   sizeof((arr)[0]) * (tlds_header(arr)->len-(n)-(i))), \
                                 tlds_header(arr)->len -= (n))
#define tlds_arrdelswap(arr, i) ((arr)[i] = tlds_arrlast(arr), tlds_header(arr)->len -= 1)

// memory allocation macros
#define tlds_arrgrow(arr, addlen, mincap) ((arr) = tlds__arrgrow_impl((arr), sizeof((arr)[0]), (addlen), (mincap)))
#define tlds_arrmaybegrow(arr, n)         ((!(arr) || tlds_header(arr)->len + (n) > tlds_header(arr)->cap) \
                                           ? (tlds_arrgrow(arr, n, 0),0) : 0)

// funtion with variable arguments
#if __STDC_VERSION__ >= 202311L // C23
#define tlds_arrpush_n(arr, ...) do {             \
    typeof(arr[0]) _tmp[] = {__VA_ARGS__};        \
    size_t _len = sizeof(_tmp) / sizeof(_tmp[0]); \
    for(size_t i = 0; i < _len; ++i) {            \
        tlds_arrpush(arr, _tmp[i]);               \
    }                                             \
} while(0)
#else
#include <tinylib/macrohelper.h>
#define tlds_arrpush_n(arr, ...) do {             \
    TL_FOREACH_F_ONE_PARAM(tlds_arrpush, arr, __VA_ARGS__); \
} while(0)
#endif
// balck macro magic to expand functions with variable arguments
#define tlds_arrpush_n_black(arr, ...) do {          \
    TL_FOREACH_ONE_PARAM(tlds_arrpush, __VA_ARGS__); \
} while (0)

// end of utility macros
// clang-format on

#ifdef __cplusplus
extern "C" {
#endif
///
/// Implementation: Function interface
///

extern void* tlds__arrgrow_impl(
    void* arr, size_t elemsize, size_t addlen, size_t mincap);
extern void* tlds__arrfree_impl(void* arr);

#ifdef __cplusplus
}
#endif

#endif // TINYLIB_DATA_STRUCT_H


#ifdef TLDS_IMPLEMENTATION

#ifdef __cplusplus
extern "C" {
#endif

void* tlds__arrgrow_impl(
    void* arr, size_t elemsize, size_t addlen, size_t mincap)
{
    void*  arr_new;
    size_t min_len = tlds_arrlen(arr) + addlen;

    // compute the minimum capacity needed
    if (min_len > mincap)
        mincap = min_len;

    if (mincap <= tlds_arrcap(arr))
        return arr;

    // increase needed capacity to guarantee O(1) amortized
    if (mincap < 2 * tlds_arrcap(arr))
        mincap = 2 * tlds_arrcap(arr);
    else if (mincap < 4)
        mincap = 4;

    arr_new = TLDS_REALLOC(NULL, (arr) ? tlds_header(arr) : 0,
        elemsize * mincap + sizeof(TLDS_array_header));
    // move b to the start of the array data
    arr_new = (char*)arr_new + sizeof(TLDS_array_header);
    if (!arr) {
        tlds_header(arr_new)->len = 0;
    }
    tlds_header(arr_new)->cap = mincap;

    return arr_new;
}

#ifdef __cplusplus
}
#endif

#endif // TLDS_IMPLEMENTATION

