#ifndef TINYLIB_COMMON_H
#define TINYLIB_COMMON_H

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WINUSER_
#define _WINGDI_
#define _IMM_
#define _WINCON_
#include <direct.h>
#include <windows.h>
#include <shellapi.h>
#else
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#define TL_LINE_END "\r\n"
#else
#define TL_LINE_END "\n"
#endif

/// support maximum argc=12
/// example:
///     TL_FOREACH(puts, "hello", "world!");
///     TL_FOREACH_ONE_PARAM(printf, "Hello, %s!\n", "Bob", "Tom");

#define _TL_NUM_VA_ARGS_HELPER(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, n, ...) n
#define TL_NUM_VA_ARGS_(...) _TL_NUM_VA_ARGS_HELPER(__VA_ARGS__, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

// two parameters

#define _TL_EXPAND_ARGS_HELPER_TWO_PARAM_1(F, EOL, _param1, _param2, _first, ...) \
    F(_param1, _param2, _first)
#define _TL_EXPAND_ARGS_HELPER_TWO_PARAM_2(F, EOL, _param1, _param2, _first, ...) \
    F(_param1, _param2, _first)EOL                                                \
    _TL_EXPAND_ARGS_HELPER_TWO_PARAM_1(F, EOL, _param1, _param2, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_TWO_PARAM_3(F, EOL, _param1, _param2, _first, ...) \
    F(_param1, _param2, _first)EOL                                                \
    _TL_EXPAND_ARGS_HELPER_TWO_PARAM_2(F, EOL, _param1, _param2, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_TWO_PARAM_4(F, EOL, _param1, _param2, _first, ...) \
    F(_param1, _param2, _first)EOL                                                \
    _TL_EXPAND_ARGS_HELPER_TWO_PARAM_3(F, EOL, _param1, _param2, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_TWO_PARAM_5(F, EOL, _param1, _param2, _first, ...) \
    F(_param1, _param2, _first)EOL                                                \
    _TL_EXPAND_ARGS_HELPER_TWO_PARAM_4(F, EOL, _param1, _param2, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_TWO_PARAM_6(F, EOL, _param1, _param2, _first, ...) \
    F(_param1, _param2, _first)EOL                                                \
    _TL_EXPAND_ARGS_HELPER_TWO_PARAM_5(F, EOL, _param1, _param2, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_TWO_PARAM_7(F, EOL, _param1, _param2, _first, ...) \
    F(_param1, _param2, _first)EOL                                                \
    _TL_EXPAND_ARGS_HELPER_TWO_PARAM_6(F, EOL, _param1, _param2, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_TWO_PARAM_8(F, EOL, _param1, _param2, _first, ...) \
    F(_param1, _param2, _first)EOL                                                \
    _TL_EXPAND_ARGS_HELPER_TWO_PARAM_7(F, EOL, _param1, _param2, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_TWO_PARAM_9(F, EOL, _param1, _param2, _first, ...) \
    F(_param1, _param2, _first)EOL                                                \
    _TL_EXPAND_ARGS_HELPER_TWO_PARAM_8(F, EOL, _param1, _param2, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_TWO_PARAM_10(F, EOL, _param1, _param2, _first, ...) \
    F(_param1, _param2, _first)EOL                                                 \
    _TL_EXPAND_ARGS_HELPER_TWO_PARAM_9(F, EOL, _param1, _param2, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_TWO_PARAM_11(F, EOL, _param1, _param2, _first, ...) \
    F(_param1, _param2, _first)EOL                                                 \
    _TL_EXPAND_ARGS_HELPER_TWO_PARAM_10(F, EOL, _param1, _param2, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_TWO_PARAM_12(F, EOL, _param1, _param2, _first, ...) \
    F(_param1, _param2, _first)EOL                                                 \
    _TL_EXPAND_ARGS_HELPER_TWO_PARAM_11(F, EOL, _param1, _param2, __VA_ARGS__)

// one parameter

#define _TL_EXPAND_ARGS_HELPER_ONE_PARAM_1(F, EOL, _param1, _first, ...) \
    F(_param1, _first)
#define _TL_EXPAND_ARGS_HELPER_ONE_PARAM_2(F, EOL, _param1, _first, ...) \
    F(_param1, _first)EOL                                                \
    _TL_EXPAND_ARGS_HELPER_ONE_PARAM_1(F, EOL, _param1, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_ONE_PARAM_3(F, EOL, _param1, _first, ...) \
    F(_param1, _first)EOL                                                \
    _TL_EXPAND_ARGS_HELPER_ONE_PARAM_2(F, EOL, _param1, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_ONE_PARAM_4(F, EOL, _param1, _first, ...) \
    F(_param1, _first)EOL                                                \
    _TL_EXPAND_ARGS_HELPER_ONE_PARAM_3(F, EOL, _param1, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_ONE_PARAM_5(F, EOL, _param1, _first, ...) \
    F(_param1, _first)EOL                                                \
    _TL_EXPAND_ARGS_HELPER_ONE_PARAM_4(F, EOL, _param1, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_ONE_PARAM_6(F, EOL, _param1, _first, ...) \
    F(_param1, _first)EOL                                                \
    _TL_EXPAND_ARGS_HELPER_ONE_PARAM_5(F, EOL, _param1, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_ONE_PARAM_7(F, EOL, _param1, _first, ...) \
    F(_param1, _first)EOL                                                \
    _TL_EXPAND_ARGS_HELPER_ONE_PARAM_6(F, EOL, _param1, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_ONE_PARAM_8(F, EOL, _param1, _first, ...) \
    F(_param1, _first)EOL                                                \
    _TL_EXPAND_ARGS_HELPER_ONE_PARAM_7(F, EOL, _param1, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_ONE_PARAM_9(F, EOL, _param1, _first, ...) \
    F(_param1, _first)EOL                                                \
    _TL_EXPAND_ARGS_HELPER_ONE_PARAM_8(F, EOL, _param1, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_ONE_PARAM_10(F, EOL, _param1, _first, ...) \
    F(_param1, _first)EOL                                                 \
    _TL_EXPAND_ARGS_HELPER_ONE_PARAM_9(F, EOL, _param1, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_ONE_PARAM_11(F, EOL, _param1, _first, ...) \
    F(_param1, _first)EOL                                                 \
    _TL_EXPAND_ARGS_HELPER_ONE_PARAM_10(F, EOL, _param1, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_ONE_PARAM_12(F, EOL, _param1, _first, ...) \
    F(_param1, _first)EOL                                                 \
    _TL_EXPAND_ARGS_HELPER_ONE_PARAM_11(F, EOL, _param1, __VA_ARGS__)

// no parameter

#define _TL_EXPAND_ARGS_HELPER_1(F, EOL, _first, ...) \
    F(_first)
#define _TL_EXPAND_ARGS_HELPER_2(F, EOL, _first, ...) \
    F(_first)EOL                                      \
    _TL_EXPAND_ARGS_HELPER_1(F, EOL, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_3(F, EOL, _first, ...) \
    F(_first)EOL                                      \
    _TL_EXPAND_ARGS_HELPER_2(F, EOL, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_4(F, EOL, _first, ...) \
    F(_first)EOL                                      \
    _TL_EXPAND_ARGS_HELPER_3(F, EOL, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_5(F, EOL, _first, ...) \
    F(_first)EOL                                      \
    _TL_EXPAND_ARGS_HELPER_4(F, EOL, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_6(F, EOL, _first, ...) \
    F(_first)EOL                                      \
    _TL_EXPAND_ARGS_HELPER_5(F, EOL, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_7(F, EOL, _first, ...) \
    F(_first)EOL                                      \
    _TL_EXPAND_ARGS_HELPER_6(F, EOL, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_8(F, EOL, _first, ...) \
    F(_first)EOL                                      \
    _TL_EXPAND_ARGS_HELPER_7(F, EOL, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_9(F, EOL, _first, ...) \
    F(_first)EOL                                      \
    _TL_EXPAND_ARGS_HELPER_8(F, EOL, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_10(F, EOL, _first, ...) \
    F(_first)EOL                                       \
    _TL_EXPAND_ARGS_HELPER_9(F, EOL, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_11(F, EOL, _first, ...) \
    F(_first)EOL                                       \
    _TL_EXPAND_ARGS_HELPER_10(F, EOL, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_12(F, EOL, _first, ...) \
    F(_first)EOL                                       \
    _TL_EXPAND_ARGS_HELPER_11(F, EOL, __VA_ARGS__)

#define TL_EMPTY()
#define TL_DEFER(m) m TL_EMPTY()
#define TL_EVAL(m) m

#define _TL_EXPAND_ARGS_HELPER_SELECTOR_TWO_PARAM(F, EOL, _param1, _param2, n, ...) \
    _TL_EXPAND_ARGS_HELPER_TWO_PARAM_##n(F, EOL, _param1, _param2, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_SELECTOR_ONE_PARAM(F, EOL, _param1, n, ...) \
    _TL_EXPAND_ARGS_HELPER_ONE_PARAM_##n(F, EOL, _param1, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_SELECTOR(F, EOL, n, ...) \
    _TL_EXPAND_ARGS_HELPER_##n(F, EOL, __VA_ARGS__)

#define TL_FOREACH_TWO_PARAM(F, EOL, _param1, _param2, ...) \
    TL_EVAL(TL_DEFER(_TL_EXPAND_ARGS_HELPER_SELECTOR_TWO_PARAM)(F, EOL, _param1, _param2, TL_NUM_VA_ARGS_(__VA_ARGS__), __VA_ARGS__))

#define TL_FOREACH_ONE_PARAM(F, EOL, _param1, ...) \
    TL_EVAL(TL_DEFER(_TL_EXPAND_ARGS_HELPER_SELECTOR_ONE_PARAM)(F, EOL, _param1, TL_NUM_VA_ARGS_(__VA_ARGS__), __VA_ARGS__))

#define TL_FOREACH(F, EOL, ...) \
    TL_EVAL(TL_DEFER(_TL_EXPAND_ARGS_HELPER_SELECTOR)(F, EOL, TL_NUM_VA_ARGS_(__VA_ARGS__), __VA_ARGS__))

#define TL_FOREACH_F(F, ...) TL_FOREACH(F,;,__VA_ARGS__)
#define TL_FOREACH_F_ONE_PARAM(F, ...) TL_FOREACH_ONE_PARAM(F,;,__VA_ARGS__)
#define TL_FOREACH_F_TWO_PARAM(F, ...) TL_FOREACH_TWO_PARAM(F,;,__VA_ARGS__)

#endif // TINYLIB_COMMON_H
