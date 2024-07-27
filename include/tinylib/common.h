#ifndef TINYLIB_COMMON_H
#define TINYLIB_COMMON_H

/* support maximum argc=12 */

#define _TL_NUM_VA_ARGS_HELPER(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, n, ...) n
#define TL_NUM_VA_ARGS_(...) _TL_NUM_VA_ARGS_HELPER(__VA_ARGS__, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

#define _TL_EXPAND_ARGS_HELPER_1(F, _name, _first, ...) \
    F(_name, _first);
#define _TL_EXPAND_ARGS_HELPER_2(F, _name, _first, ...) \
    F(_name, _first);                                   \
    _TL_EXPAND_ARGS_HELPER_1(F, _name, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_3(F, _name, _first, ...) \
    F(_name, _first);                                   \
    _TL_EXPAND_ARGS_HELPER_2(F, _name, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_4(F, _name, _first, ...) \
    F(_name, _first);                                   \
    _TL_EXPAND_ARGS_HELPER_3(F, _name, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_5(F, _name, _first, ...) \
    F(_name, _first);                                   \
    _TL_EXPAND_ARGS_HELPER_4(F, _name, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_6(F, _name, _first, ...) \
    F(_name, _first);                                   \
    _TL_EXPAND_ARGS_HELPER_5(F, _name, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_7(F, _name, _first, ...) \
    F(_name, _first);                                   \
    _TL_EXPAND_ARGS_HELPER_6(F, _name, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_8(F, _name, _first, ...) \
    F(_name, _first);                                   \
    _TL_EXPAND_ARGS_HELPER_7(F, _name, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_9(F, _name, _first, ...) \
    F(_name, _first);                                   \
    _TL_EXPAND_ARGS_HELPER_8(F, _name, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_10(F, _name, _first, ...) \
    F(_name, _first);                                    \
    _TL_EXPAND_ARGS_HELPER_9(F, _name, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_11(F, _name, _first, ...) \
    F(_name, _first);                                    \
    _TL_EXPAND_ARGS_HELPER_10(F, _name, __VA_ARGS__)
#define _TL_EXPAND_ARGS_HELPER_12(F, _name, _first, ...) \
    F(_name, _first);                                    \
    _TL_EXPAND_ARGS_HELPER_11(F, _name, __VA_ARGS__)

#define TL_EMPTY()
#define TL_DEFER(m) m TL_EMPTY()
#define TL_EVAL(m) m
#define _TL_EXPAND_ARGS_HELPER_SELECTOR(F, _name, n, ...) \
    _TL_EXPAND_ARGS_HELPER_##n(F, _name, __VA_ARGS__)
#define TL_FOREACH(F, _name, ...) \
    TL_EVAL(TL_DEFER(_TL_EXPAND_ARGS_HELPER_SELECTOR)(F, _name, TL_NUM_VA_ARGS_(__VA_ARGS__), __VA_ARGS__))

#endif // TINYLIB_COMMON_H
