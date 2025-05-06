/// dyn_arr.c
///
/// Tiny C Lib dynamic array implementation
///

#ifndef TINYLIB_DYN_ARR_H
#define TINYLIB_DYN_ARR_H

#include "common.h"
#include <stdlib.h>

/* Configurations */
// #define TINYLIB_DYN_ARR_FUNC_IMPL    1
// #define TINYLIB_DYN_ARR_MACRO_IMPL   0

// TODO: finish FUNC implementation
#define TINYLIB_DYN_ARR_MACRO_IMPL 1

#ifndef TINYLIB_DYN_ARR_MACRO_IMPL
#define TINYLIB_DYN_ARR_MACRO_IMPL 0
#endif

#ifndef TINYLIB_DYN_ARR_FUNC_IMPL
#if TINYLIB_DYN_ARR_MACRO_IMPL
#define TINYLIB_DYN_ARR_FUNC_IMPL 0
#else
#define TINYLIB_DYN_ARR_FUNC_IMPL 1
#endif
#endif

/* Declearations */
#ifdef __cplusplus
extern "C" {
#endif

/// Dynamic array declaration
/// =========================
///
/// members:
/// - len: current length of the array
/// - cap: allocated length of the array
/// - data: pointer to the array data
///
/// function void arr_init(dyn_arr* arr, size_t initial_cap, size_t item_size);
/// function void arr_append_item(dyn_arr* arr, void* item);
/// function void arr_append(dyn_arr* arr, void* item);
/// function void arr_append_items(dyn_arr* arr, void* item1, ...);
/// function void arr_free(dyn_arr* arr);

#if TINYLIB_DYN_ARR_FUNC_IMPL
#undef TINYLIB_DYN_ARR_FUNC_IMPL
#include <cstring>

typedef struct {
    size_t len;
    size_t cap;
    size_t item_size;
    void* data;
} dyn_arr;

inline bool arr_init(dyn_arr* arr, size_t initial_cap, size_t item_size)
{
    arr->len = 0;
    arr->cap = 1;
    arr->item_size = item_size;
    arr->data = malloc(item_size * arr->cap);
    return arr->data != NULL;
}

inline bool arr_append_item(dyn_arr* arr, void* item)
{
    if (arr->len >= arr->cap) {
        arr->cap *= 2;
        void* new_data = realloc(arr->data, arr->item_size * arr->cap);
        if (new_data == NULL) {
            return false; // realloc failed
        }
        arr->data = new_data;
    }
    memcpy((char*)arr->data + arr->len * arr->item_size, item, arr->item_size);
    ++arr->len;
    return true;
}

inline bool array_append(dyn_arr* arr, void* item)
{
    return arr_append_item(arr, item, arr->item_size);
}

#endif // TINYLIB_DYN_ARR_FUNC_IMPL

/// Dynamic array declaration [MACRO VERSION]
/// =========================================
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
/// function void arr_init(dyn_arr* arr, size_t initial_cap);
/// function void arr_append_item(dyn_arr* arr, _T* item);
/// function void arr_append(dyn_arr* arr, _T* item);
/// function void arr_append_items(dyn_arr* arr, int item1, int item2, ...);
/// function void arr_free(dyn_arr* arr);

#if TINYLIB_DYN_ARR_MACRO_IMPL
#undef TINYLIB_DYN_ARR_MACRO_IMPL

#define arr_init(_name, _initial_cap)                              \
    do {                                                           \
        _name.len = 0;                                             \
        _name.cap = _initial_cap;                                  \
        _name.data = malloc(_initial_cap * sizeof(_name.data[0])); \
    } while (0)

#define arr_append_item(_name, _item)                                            \
    do {                                                                         \
        size_t new_cap = _name.len + 1 > _name.cap ? _name.cap * 2 : _name.cap;  \
        if (new_cap != _name.cap) {                                              \
            _name.cap = new_cap;                                                 \
            _name.data = realloc(_name.data, sizeof(_name.data[0]) * _name.cap); \
        }                                                                        \
        _name.data[_name.len] = _item;                                           \
        ++_name.len;                                                             \
    } while (0)

#define arr_append(_name, arr)                \
    for (int _i = 0; _i < arr.len; ++_i) {    \
        arr_append_item(_name, arr.data[_i]); \
    }

#define arr_append_items(_name, ...)                              \
    do {                                                          \
        TL_FOREACH_ONE_PARAM(arr_append_item, _name, __VA_ARGS__) \
    } while (0)

#define arr_free(_name)    \
    do {                   \
        free(_name.data);  \
        _name.data = NULL; \
        _name.len = 0;     \
        _name.cap = 0;     \
    } while (0)

#define arr_free_items(_name, ...)                         \
    do {                                                   \
        TL_FOREACH_ONE_PARAM(arr_free, _name, __VA_ARGS__) \
    } while (0)

#endif // TINYLIB_DYN_ARR_MACRO_IMPL

#ifdef __cplusplus
}
#endif

#endif // TINYLIB_DYN_ARR_H
