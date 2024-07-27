/// dyn_arr.c
///
/// Tiny C Lib dynamic array implementation
///

#ifndef TINYLIB_DYN_ARR_H
#define TINYLIB_DYN_ARR_H

#include "common.h"
#include <stdlib.h>

/* Declearations */
#ifdef __cplusplus
extern "C" {
#endif

#define decltype_arr(_T, _tname) \
    typedef struct {             \
        size_t len;              \
        size_t alloc_len;        \
        _T* data;                \
    } _tname

#define init_arr(_name)                             \
    do {                                            \
        _name.data = malloc(sizeof(_name.data[0])); \
    } while (0)

#define append_arr_item(_name, _item)                                                                   \
    do {                                                                                                \
        size_t new_alloc_len = _name.len + 1 > _name.alloc_len ? _name.alloc_len * 2 : _name.alloc_len; \
        if (new_alloc_len != _name.alloc_len) {                                                         \
            _name.alloc_len = new_alloc_len;                                                            \
            _name.data = realloc(&_name.data, sizeof(_name.data[0]) * _name.alloc_len);                 \
        }                                                                                               \
        _name.data[_name.len] = _item;                                                                  \
        ++_name.len;                                                                                    \
    } while (0)

#define append_arr(_name, arr)                \
    for (int _i = 0; _i < arr.len; ++_i) {    \
        append_arr_item(_name, arr.data[_i]); \
    }

#define append_arr_items(_name, ...)                    \
    do {                                                \
        TL_FOREACH(append_arr_item, _name, __VA_ARGS__) \
    } while (0)

#ifdef __cplusplus
}
#endif

/* Implementation */
#ifdef TINYLIB_DYN_ARR_IMPL

#endif

#endif // TINYLIB_DYN_ARR_H
