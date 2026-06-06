# TinyLib Allocator Contract

TinyLib allocators use an explicit-size contract:

```c
void *alloc(void *ctx, size_t size, size_t align);
void  free(void *ctx, void *ptr, size_t size, size_t align);
void *realloc(void *ctx, void *ptr, size_t old_size, size_t new_size, size_t align);
```

The caller owns size metadata. Allocators should not need to discover an
allocation's old size from hidden allocator headers.

## Rules

- `align` must be a power of two. TinyLib normalizes alignments smaller than
  `sizeof(void *)` to `sizeof(void *)`.
- `size == 0` allocations return `NULL`.
- `free(NULL, size, align)` is valid and does nothing.
- `realloc(NULL, old_size, new_size, align)` behaves like `alloc(new_size, align)`.
- `realloc(ptr, old_size, 0, align)` frees when the allocator has meaningful
  free semantics, and returns `NULL`.
- `realloc` copies `min(old_size, new_size)` bytes when it has to move memory.
- Containers such as dynamic arrays pass exact allocation byte sizes to the
  allocator.

## Built-In Allocators

### Standard Allocator

`tl_default_allocator` wraps `malloc`, `free`, and `realloc`. Over-aligned
allocations use a small private base-pointer slot so they can be released
correctly.

### Arena

`TL_Arena` is a bump allocator with mark/restore support.

- `free` is a no-op.
- `realloc` allocates a new block and copies from the old pointer.
- Old arena allocations remain valid until `tl_arena_restore`,
  `tl_arena_reset`, or `tl_arena_destroy`.
- Arena allocations do not carry per-allocation size headers.

### Fixed Pool

`TL_FixedPool` allocates fixed-size blocks and keeps freed blocks in a free
list. It is intended for objects with a stable maximum size such as nodes,
tokens, small AST objects, or handles.

- Requests larger than `block_size` fail.
- Requests with alignment stronger than `block_align` fail.
- `realloc` can only grow within the fixed block size.
- The pool does not validate whether a freed pointer belongs to the pool.

## Debugging

Allocator implementations may keep debug-only metadata, but correctness must
not depend on recovering the old allocation size at free/realloc time. Prefer
assertions and diagnostics for misuse, and keep release behavior simple.
