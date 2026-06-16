# TinyLib Hash Maps

`TL_Map` is a generic open-addressing hash table. It stores keys, values, and
slot states in separate arrays and uses linear probing.

## Basic Usage

```c
TL_Map map = {0};

tl_map_init_bytewise(map, int, int, NULL);
tl_map_put(map, 7, 70);

const int *value = tl_map_get_const(map, 7, int);
if (value) {
    int copy = 0;
    tl_map_try_get(map, 7, &copy);
}

tl_map_free(map);
```

`tl_map_init_bytewise(map, KeyType, ValueType, allocator)` records key/value
size and alignment. Later typed macros assert that the passed key/value sizes
and alignments match the map.

Bytewise maps hash and compare the object representation of each key. This is a
good default for scalar keys and fully initialized POD-like keys. Do not use it
for struct keys with padding-sensitive equality semantics; provide a custom
`hash` / `eq` pair with `tl_map_init_ex(...)` instead.

## String Keys

The default map hashes key bytes. For pointer keys, that means pointer identity.
Use `tl_map_init_cstr` and the `*_cstr` operations when the key is a
null-terminated string and equality should use string contents:

```c
TL_Map names = {0};

tl_map_init_cstr(names, int, NULL);
tl_map_put_cstr(names, "alice", 11);

const int *score = tl_map_get_const_cstr(names, "alice", int);

tl_map_free(names);
```

`tl_map_init_cstr(...)` is a convenience alias for `tl_map_init_strview(...)`.
TinyLib stores a `TL_StrView` key and copies the `const char *` pointer, not the
pointed-to string bytes. The caller must ensure string storage outlives the map
entry.

Pointers returned by `tl_map_get_const*` and `tl_map_get_mut*` refer to internal
map storage. They may be invalidated by later operations that rehash, insert, or
remove entries. Use `tl_map_try_get*` when a stable copy-out read is preferred.

## API

- `tl_map_len(map)` returns the number of live entries.
- `tl_map_cap(map)` returns the slot capacity.
- `tl_map_empty(map)` returns whether length is zero.
- `tl_map_init_bytewise(map, KeyType, ValueType, allocator)` initializes a
  bytewise key map.
- `tl_map_init_ex(map, KeyType, ValueType, allocator, hash, eq)` initializes
  with custom key hash/equality callbacks.
- `tl_map_init_strview(map, ValueType, allocator)` initializes a `TL_StrView`
  key map.
- `tl_map_init_cstr(map, ValueType, allocator)` initializes a string-key map.
- `tl_map_free(map)` releases all map storage.
- `tl_map_reserve(map, n)` reserves enough slots for at least `n` live entries
  without another growth rehash.
- `tl_map_put(map, key, value)` inserts or replaces one entry.
- `tl_map_put_as(map, key, ValueType, value)` inserts or replaces one entry
  while explicitly naming the value type. Use this when C's expression type is
  not the stored type, such as storing a string literal as `const char *`.
- `tl_map_get_const(map, key, ValueType)` returns a read-only pointer to the
  stored value or `NULL`.
- `tl_map_get_mut(map, key, ValueType)` returns a mutable pointer to the stored
  value or `NULL`.
- `tl_map_try_get(map, key, out_ptr)` copies the stored value into `*out_ptr`
  and returns whether the key was found.
- `tl_map_contains(map, key)` checks for a key.
- `tl_map_remove(map, key)` removes a key.
- `tl_map_get_const_cstr`, `tl_map_get_mut_cstr`, and `tl_map_try_get_cstr`
  are string-key read variants that take a C string key.
- `tl_map_get_const_strview`, `tl_map_get_mut_strview`, and
  `tl_map_try_get_strview` are string-key read variants that take a
  `TL_StrView` key.
- `tl_map_put_cstr`, `tl_map_contains_cstr`, and `tl_map_remove_cstr` are
  string-key convenience operations.
- `tl_map_put_cstr_as(map, key, ValueType, value)` is the string-key version of
  `tl_map_put_as`.

## Current Limits

- Keys and values are copied by bytes. Destructors are not called.
- Iteration is not exposed yet.
- The map is not thread-safe by itself.
- Removed slots leave tombstones. Growth/rehash cleans them up automatically.
