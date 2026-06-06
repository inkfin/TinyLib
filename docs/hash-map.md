# TinyLib Hash Maps

`TL_Map` is a generic open-addressing hash table. It stores keys, values, and
slot states in separate arrays and uses linear probing.

## Basic Usage

```c
TL_Map map = {0};

tl_map_init(map, int, int, NULL);
tl_map_put(map, 7, 70);

int *value = tl_map_get(map, 7, int);
if (value) {
    *value += 1;
}

tl_map_free(map);
```

`tl_map_init(map, KeyType, ValueType, allocator)` records key/value size and
alignment. Later typed macros assert that the passed key/value sizes and
alignments match the map.

## String Keys

The default map hashes key bytes. For pointer keys, that means pointer identity.
Use `tl_map_init_cstr` and the `*_cstr` operations when the key is a
null-terminated string and equality should use string contents:

```c
TL_Map names = {0};

tl_map_init_cstr(names, int, NULL);
tl_map_put_cstr(names, "alice", 11);

int *score = tl_map_get_cstr(names, "alice", int);

tl_map_free(names);
```

TinyLib copies the `const char *` pointer, not the pointed-to string. The caller
must ensure string storage outlives the map entry.

## API

- `tl_map_len(map)` returns the number of live entries.
- `tl_map_cap(map)` returns the slot capacity.
- `tl_map_empty(map)` returns whether length is zero.
- `tl_map_init(map, KeyType, ValueType, allocator)` initializes a byte-key map.
- `tl_map_init_ex(map, KeyType, ValueType, allocator, hash, eq)` initializes
  with custom key hash/equality callbacks.
- `tl_map_init_cstr(map, ValueType, allocator)` initializes a string-key map.
- `tl_map_free(map)` releases all map storage.
- `tl_map_reserve(map, n)` reserves enough slots for at least `n` live entries.
- `tl_map_put(map, key, value)` inserts or replaces one entry.
- `tl_map_put_as(map, key, ValueType, value)` inserts or replaces one entry
  while explicitly naming the value type. Use this when C's expression type is
  not the stored type, such as storing a string literal as `const char *`.
- `tl_map_get(map, key, ValueType)` returns a pointer to the value or `NULL`.
- `tl_map_contains(map, key)` checks for a key.
- `tl_map_remove(map, key)` removes a key.
- `tl_map_put_cstr`, `tl_map_get_cstr`, `tl_map_contains_cstr`, and
  `tl_map_remove_cstr` are string-key variants.
- `tl_map_put_cstr_as(map, key, ValueType, value)` is the string-key version of
  `tl_map_put_as`.

## Current Limits

- Keys and values are copied by bytes. Destructors are not called.
- Iteration is not exposed yet.
- The map is not thread-safe by itself.
- Removed slots leave tombstones. Growth/rehash cleans them up automatically.
