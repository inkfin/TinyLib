#ifndef TINYLIB_COMMON_H
#define TINYLIB_COMMON_H

#ifdef _WIN32
#    define WIN32_LEAN_AND_MEAN
#    define _WINUSER_
#    define _WINGDI_
#    define _IMM_
#    define _WINCON_
#    include <windows.h>
#    include <direct.h>
#    include <shellapi.h>
#else
#    include <sys/types.h>
#    include <sys/wait.h>
#    include <sys/stat.h>
#    include <unistd.h>
#    include <fcntl.h>
#endif

#ifdef _WIN32
#    define TL_LINE_END "\r\n"
#else
#    define TL_LINE_END "\n"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/// support maximum argc=12
/// example:
///     TL_FOREACH(puts, "hello", "world!");

#define _TL_NUM_VA_ARGS_HELPER(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, n, ...) n
#define TL_NUM_VA_ARGS_(...) _TL_NUM_VA_ARGS_HELPER(__VA_ARGS__, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)


// one parameter

#define _TL_EXPAND_ARGS_HELPER_ONE_PARAM_1(F, _param1, _first, ...) \
    F(_param1, _first);
#define _TL_EXPAND_ARGS_HELPER_ONE_PARAM_2(F, _param1, _first, ...) \
    F(_param1, _first);                                   \
    _TL_EXPAND_ARGS_HELPER_ONE_PARAM_1(F, _param1, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_ONE_PARAM_3(F, _param1, _first, ...) \
    F(_param1, _first);                                   \
    _TL_EXPAND_ARGS_HELPER_ONE_PARAM_2(F, _param1, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_ONE_PARAM_4(F, _param1, _first, ...) \
    F(_param1, _first);                                   \
    _TL_EXPAND_ARGS_HELPER_ONE_PARAM_3(F, _param1, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_ONE_PARAM_5(F, _param1, _first, ...) \
    F(_param1, _first);                                   \
    _TL_EXPAND_ARGS_HELPER_ONE_PARAM_4(F, _param1, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_ONE_PARAM_6(F, _param1, _first, ...) \
    F(_param1, _first);                                   \
    _TL_EXPAND_ARGS_HELPER_ONE_PARAM_5(F, _param1, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_ONE_PARAM_7(F, _param1, _first, ...) \
    F(_param1, _first);                                   \
    _TL_EXPAND_ARGS_HELPER_ONE_PARAM_6(F, _param1, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_ONE_PARAM_8(F, _param1, _first, ...) \
    F(_param1, _first);                                   \
    _TL_EXPAND_ARGS_HELPER_ONE_PARAM_7(F, _param1, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_ONE_PARAM_9(F, _param1, _first, ...) \
    F(_param1, _first);                                   \
    _TL_EXPAND_ARGS_HELPER_ONE_PARAM_8(F, _param1, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_ONE_PARAM_10(F, _param1, _first, ...) \
    F(_param1, _first);                                    \
    _TL_EXPAND_ARGS_HELPER_ONE_PARAM_9(F, _param1, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_ONE_PARAM_11(F, _param1, _first, ...) \
    F(_param1, _first);                                    \
    _TL_EXPAND_ARGS_HELPER_ONE_PARAM_10(F, _param1, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_ONE_PARAM_12(F, _param1, _first, ...) \
    F(_param1, _first);                                    \
    _TL_EXPAND_ARGS_HELPER_ONE_PARAM_11(F, _param1, __VA_ARGS__)


// no parameter

#define _TL_EXPAND_ARGS_HELPER_1(F, _first, ...) \
    F(_first);
#define _TL_EXPAND_ARGS_HELPER_2(F, _first, ...) \
    F(_first);                                   \
    _TL_EXPAND_ARGS_HELPER_1(F, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_3(F, _first, ...) \
    F(_first);                                   \
    _TL_EXPAND_ARGS_HELPER_2(F, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_4(F, _first, ...) \
    F(_first);                                   \
    _TL_EXPAND_ARGS_HELPER_3(F, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_5(F, _first, ...) \
    F(_first);                                   \
    _TL_EXPAND_ARGS_HELPER_4(F, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_6(F, _first, ...) \
    F(_first);                                   \
    _TL_EXPAND_ARGS_HELPER_5(F, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_7(F, _first, ...) \
    F(_first);                                   \
    _TL_EXPAND_ARGS_HELPER_6(F, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_8(F, _first, ...) \
    F(_first);                                   \
    _TL_EXPAND_ARGS_HELPER_7(F, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_9(F, _first, ...) \
    F(_first);                                   \
    _TL_EXPAND_ARGS_HELPER_8(F, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_10(F, _first, ...) \
    F(_first);                                    \
    _TL_EXPAND_ARGS_HELPER_9(F, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_11(F, _first, ...) \
    F(_first);                                    \
    _TL_EXPAND_ARGS_HELPER_10(F, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_12(F, _first, ...) \
    F(_first);                                    \
    _TL_EXPAND_ARGS_HELPER_11(F, __VA_ARGS__)


#define TL_EMPTY()
#define TL_DEFER(m) m TL_EMPTY()
#define TL_EVAL(m) m
#define _TL_EXPAND_ARGS_HELPER_SELECTOR_ONE_PARAM(F, _param1, n, ...) \
    _TL_EXPAND_ARGS_HELPER_ONE_PARAM_##n(F, _param1, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_SELECTOR(F, n, ...) \
    _TL_EXPAND_ARGS_HELPER_##n(F, __VA_ARGS__)

#define TL_FOREACH_ONE_PARAM(F, _param1, ...) \
    TL_EVAL(TL_DEFER(_TL_EXPAND_ARGS_HELPER_SELECTOR_ONE_PARAM)(F, _param1, TL_NUM_VA_ARGS_(__VA_ARGS__), __VA_ARGS__))

#define TL_FOREACH(F, ...) \
    TL_EVAL(TL_DEFER(_TL_EXPAND_ARGS_HELPER_SELECTOR)(F, TL_NUM_VA_ARGS_(__VA_ARGS__), __VA_ARGS__))

#ifdef __cplusplus
}
#endif

#endif // TINYLIB_COMMON_H
