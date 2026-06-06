# TinyLib Dynamic Arrays

Dynamic arrays are plain typed pointers:

```c
TL_ArrInt *items = NULL;

tl_arr_push(items, 10);
tl_arr_push(items, 20);

int first = items[0];
size_t len = tl_arr_len(items);

tl_arr_free(items);
```

The metadata header is stored immediately before the returned data pointer.
User code treats the value as `T *`.

## Core API

- `tl_arr_len(arr)` returns the current element count.
- `tl_arr_cap(arr)` returns the current reserved capacity.
- `tl_arr_init(arr, allocator)` initializes an empty array with a chosen
  allocator.
- `tl_arr_free(arr)` releases the array and sets the pointer to `NULL`.
- `tl_arr_reserve(arr, n)` reserves capacity.
- `tl_arr_resize(arr, n)` changes length.
- `tl_arr_push(arr, value)` appends one value.
- `tl_arr_push_n(arr, ...)` appends literal values.
- `tl_arr_pop(arr, out_ptr)` removes the last value and copies it to `out_ptr`.
- `tl_arr_at(arr, idx)` returns a pointer to an element or `NULL`.

## Convenience API

- `tl_arr_append(dst, src)` appends another TinyLib array of the same element
  type.
- `tl_arr_addnptr(arr, n)` appends `n` uninitialized slots and returns the first
  new slot.
- `tl_arr_addnidx(arr, n)` appends `n` uninitialized slots and returns the first
  new index.
- `tl_arr_ins(arr, idx, value)` inserts one value.
- `tl_arr_insn(arr, idx, n)` inserts `n` uninitialized slots.
- `tl_arr_del(arr, idx)` deletes one element and preserves order.
- `tl_arr_deln(arr, idx, n)` deletes `n` elements and preserves order.
- `tl_arr_del_swap(arr, idx)` deletes one element by swapping with the last
  element.

## Type Aliases

Basic aliases such as `TL_ArrInt`, `TL_ArrU8`, and `TL_ArrFloat` are typedefs
for their element type. For example, `TL_ArrInt *` is an `int *`.

Use `TL_DECLARE_ARR_TYPE(Name, T)` when a project wants a named array element
alias for a custom type:

```c
typedef struct Token Token;
TL_DECLARE_ARR_TYPE(TL_ArrToken, Token);

TL_ArrToken *tokens = NULL;
```

No wrapper struct is created.

## Debug Checks

Define `TL_DS_DEBUG` before including the header to add a debug magic value to
the hidden header. This is only a diagnostic tool for catching obvious misuse
with assertions; it is not a release-mode safety mechanism.

## Tradeoffs

This design keeps `arr[i]` access and lets macros infer `sizeof(*arr)` and
`alignof(*arr)`. The cost is that mutating operations must receive an lvalue
array pointer because growth can move the allocation and update the caller's
pointer.
