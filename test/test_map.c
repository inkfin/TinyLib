#define TL_DS_SHORT_NAMES
#include "../include/tinylib/data_struct.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int
map_test_cases(void)
{
    puts("- Hash Map Test Cases");

    TL_Map int_map = {0};
    map_init_bytewise(int_map, int, int, NULL);
    assert(map_empty(int_map));
    assert(map_reserve(int_map, 64));

    for (int i = 0; i < 80; ++i) {
        assert(map_put(int_map, i, i * 10));
    }
    assert(map_len(int_map) == 80);

    for (int i = 0; i < 80; ++i) {
        const int *value = map_get_const(int_map, i, int);
        assert(value != NULL);
        assert(*value == i * 10);
    }

    {
        size_t cap_before_update = map_cap(int_map);
        int *value = map_get_mut(int_map, 42, int);
        assert(value != NULL);
        *value = 9001;
        assert(map_cap(int_map) == cap_before_update);
    }

    assert(map_put(int_map, 42, 9001));
    {
        int value = 0;
        assert(map_try_get(int_map, 42, &value));
        assert(value == 9001);
    }

    {
        int value = -1;
        assert(!map_try_get(int_map, -1, &value));
        assert(value == -1);
    }

    for (int i = 0; i < 60; ++i) {
        if (i == 42) continue;
        assert(map_remove(int_map, i));
    }

    {
        size_t cap_before_reserve = map_cap(int_map);
        assert(map_reserve(int_map, 80));
        assert(map_cap(int_map) == cap_before_reserve);

        for (int i = 1000; i < 1060; ++i) {
            assert(map_put(int_map, i, i * 10));
            assert(map_cap(int_map) == cap_before_reserve);
        }
        assert(map_len(int_map) == 81);
    }

    {
        const int *value = map_get_const(int_map, 42, int);
        assert(value != NULL);
        assert(*value == 9001);
    }

    assert(map_remove(int_map, 42));
    assert(!map_contains(int_map, 42));
    assert(!map_remove(int_map, 42));
    assert(map_put(int_map, 42, 420));
    {
        const int *value = map_get_const(int_map, 42, int);
        assert(value != NULL);
        assert(*value == 420);
    }

    map_free(int_map);
    assert(map_len(int_map) == 0);

    TL_Map names = {0};
    map_init_cstr(names, int, NULL);
    assert(map_put_cstr(names, "alice", 11));
    assert(map_put_cstr(names, "bob", 22));
    assert(map_put_cstr(names, "alice", 33));

    {
        const int *value = map_get_const_cstr(names, "alice", int);
        assert(value != NULL);
        assert(*value == 33);
    }
    {
        int value = 0;
        assert(map_try_get_cstr(names, "bob", &value));
        assert(value == 22);
    }
    {
        TL_StrView alice = { .data = "alice", .beg = 0, .end = 5 };
        const int *value = map_get_const_strview(names, alice, int);
        assert(value != NULL);
        assert(*value == 33);
    }
    assert(map_contains_cstr(names, "alice"));
    assert(map_remove_cstr(names, "alice"));
    assert(!map_contains_cstr(names, "alice"));
    assert(map_get_const_cstr(names, "missing", int) == NULL);
    map_free(names);

    TL_Map labels = {0};
    map_init_strview(labels, const char *, NULL);
    assert(map_put_cstr_as(labels, "lang", const char *, "c"));
    {
        const char *value = NULL;
        assert(map_try_get_cstr(labels, "lang", &value));
        assert(strcmp(value, "c") == 0);
    }
    {
        TL_StrView lang = { .data = "lang", .beg = 0, .end = 4 };
        const char *const *value = map_get_const_strview(labels, lang, const char *);
        assert(value != NULL);
        assert(strcmp(*value, "c") == 0);
    }
    map_free(labels);

    puts("\n");
    return 0;
}
