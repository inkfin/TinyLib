# TinyLib

A collection of small C utilities.

## Usage

Copy the `include/tinylib` folder into a project.

For declarations, include the headers you need:

```c
#include "tinylib/data_struct.h"
#include "tinylib/logging.h"
```

Modules with a `.c` file provide their implementation through an explicit
implementation unit. Compile the file normally, or include it in exactly one
project translation unit:

```c
#include "tinylib/logging.c"
```

The folder also provides umbrella entries:

```c
#include "tinylib/tinylib.h" /* declarations */
#include "tinylib/tinylib.c" /* implementation unit, once */
```

## Single-file bundle

Generate an amalgamated implementation file:

```sh
make bundle
```

This writes `target/tinylib.c`. Copy that one file and compile it, or include it
in exactly one project translation unit.

Validate the generated bundle with:

```sh
make bundle-test
```

## Memory

TinyLib exposes a small allocator vtable with explicit `size`, `old_size`, and
`align` parameters. Built-in allocators include the standard allocator, arena
allocator, and fixed-size pool allocator. See [docs/allocator.md](docs/allocator.md).

## Dynamic Arrays

Dynamic arrays are hidden-header typed pointers, so user code can keep direct
`arr[i]` access while TinyLib stores length/capacity metadata before the data
pointer. See [docs/dynamic-array.md](docs/dynamic-array.md).

## Hash Maps

`TL_Map` provides a generic open-addressing hash table with typed macro helpers
and string-key convenience wrappers. See [docs/hash-map.md](docs/hash-map.md).

## Threading

`tinylib/logging.c` serializes global logger configuration and log writes with
an internal platform lock when available. Define `TL_LOG_NO_THREADS` before
compiling the implementation to disable locking. On POSIX platforms, projects
that compile the logging implementation may need to link with pthread support.

## Compiler Extensions

`tinylib/c_ext.h` collects small portability wrappers for language and compiler
extensions.

Constructor registration is available through:

```c
#include "tinylib/c_ext.h"

static void app_init(void)
{
    /* small startup work */
}

TL_REGISTER_INIT(app_init)
```

And teardown registration through:

```c
static void app_fini(void)
{
    /* small shutdown work */
}

TL_REGISTER_FINI(app_fini)
```

Notes:

- Supported toolchains: GCC, Clang, MSVC
- Use these macros only at file scope
- The target function must be `void fn(void)`
- Do not add a trailing semicolon after `TL_REGISTER_INIT(...)` or `TL_REGISTER_FINI(...)`
- Do not rely on execution order across translation units
- Keep registered work small and safe during process startup/shutdown

If code needs to detect support ahead of time, check `TL_HAS_CONSTRUCTOR_REGISTRATION`.

## Naming

Public symbols use the `tl_` function prefix and `TL_` type/macro prefix.
Internal helper macros use a double-underscore module form such as `TL_DS__*`.
Short aliases are off by default and require an explicit module opt-in such as
`TL_DS_SHORT_NAMES`, `TL_STR_SHORT_NAMES`, `TL_MEM_SHORT_NAMES`, or
`TL_LOG_SHORT_NAMES`.

## References

<https://github.com/tsoding/data-mining-in-c/blob/main/src/nob.h>

<https://github.com/rxi/log.c>
