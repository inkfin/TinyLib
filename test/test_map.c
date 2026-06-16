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
    map_init(int_map, int, int, NULL);
    assert(map_empty(int_map));
    assert(map_reserve(int_map, 64));

    for (int i = 0; i < 80; ++i) {
        assert(map_put(int_map, i, i * 10));
    }
    assert(map_len(int_map) == 80);

    for (int i = 0; i < 80; ++i) {
        int *value = map_get(int_map, i, int);
        assert(value != NULL);
        assert(*value == i * 10);
    }

    assert(map_put(int_map, 42, 9001));
    {
        int *value = map_get(int_map, 42, int);
        assert(value != NULL);
        assert(*value == 9001);
    }

    assert(map_remove(int_map, 42));
    assert(!map_contains(int_map, 42));
    assert(!map_remove(int_map, 42));
    assert(map_put(int_map, 42, 420));
    {
        int *value = map_get(int_map, 42, int);
        assert(value != NULL);
        assert(*value == 420);
    }

    map_free(int_map);
    assert(map_len(int_map) == 0);

    TL_Map names = {0};
    map_init_strview(names, int, NULL);
    assert(map_put_cstr(names, "alice", 11));
    assert(map_put_cstr(names, "bob", 22));
    assert(map_put_cstr(names, "alice", 33));

    {
        int *value = map_get_cstr(names, "alice", int);
        assert(value != NULL);
        assert(*value == 33);
    }
    {
        int *value = map_get_cstr(names, "bob", int);
        assert(value != NULL);
        assert(*value == 22);
    }
    assert(map_contains_cstr(names, "alice"));
    assert(map_remove_cstr(names, "alice"));
    assert(!map_contains_cstr(names, "alice"));
    assert(map_get_cstr(names, "missing", int) == NULL);
    map_free(names);

    TL_Map labels = {0};
    map_init_strview(labels, const char *, NULL);
    assert(map_put_cstr_as(labels, "lang", const char *, "c"));
    {
        const char **value = map_get_cstr(labels, "lang", const char *);
        assert(value != NULL);
        assert(strcmp(*value, "c") == 0);
    }
    map_free(labels);

    puts("\n");
    return 0;
}
