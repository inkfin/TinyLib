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

## Project Build

TinyLib's own Makefile is only a bootstrap layer. The first `make` invocation
compiles `build.c` to `target/build`, then that C build driver handles project
targets through `tinylib/compile.h`.

Common targets:

```sh
make
make snapshot
make c99-snapshot
make bundle-test
```

The build driver self-rebuilds with `TL_GO_REBUILD_URSELF`, so edits to
`build.c` take effect on the next target invocation. Use `MODE=dbg` for debug
flags and `SANITIZE=1` for the sanitizer-debug preset:

```sh
make snapshot MODE=dbg
make snapshot SANITIZE=1
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

## Logging

`TL_LogConfig cfg = {0}` is the default logging configuration. See the comments
on `TL_LogConfig` in `tinylib/logging.h` for the full startup example and field
defaults.

## Compile Helpers

`tinylib/compile.h` provides nob.h-style helpers for small C build programs. It
is not a vendored nob.h layer; it follows TinyLib's arrays, logging, naming, and
implementation-unit conventions. The initial implementation targets GCC/Clang
on POSIX and supports debug/release defaults, flag/source management, recursive
source discovery, timestamp rebuild checks, command execution, compact build
logs through `tinylib/logging.h`, and optional self-rebuild:

```c
#include "tinylib/compile.c"

static b32_t build_app(void)
{
    TL_CompileCmd cmd = {0};

    tl_compile_cmd_init(&cmd, NULL);
    tl_compile_set_compiler(&cmd, "clang");
    tl_compile_apply_preset(&cmd, &tl_compile_preset_debug);
    tl_compile_set_standard(&cmd, TL_C_STD_GNU11);
    tl_compile_set_output(&cmd, "target/app");
    tl_compile_includes(&cmd, "include");

    const char *exts[] = { ".c" };
    TL_SourceFindConfig sources = {
        .root = "src",
        .extensions = exts,
        .extensions_count = TL_COUNT_OF(exts),
    };
    tl_compile_add_sources_recursive(&cmd, &sources);

    return tl_build_target_finish("target/app", tl_compile_run(&cmd));
}

int main(int argc, char **argv)
{
    static const TL_BuildTarget targets[] = {
        { "app", build_app },
    };
    TL_BuildConfig build = {
        .project_name = "Example",
        .build_dir = "target",
        .compiler = "clang",
        .mode = "debug",
        .default_target = "app",
        .targets = targets,
        .targets_count = TL_COUNT_OF(targets),
    };

    TL_GO_REBUILD_URSELF(argc, argv);
    return tl_build_run(argc, argv, &build);
}
```

Recursive discovery skips `.git`, `target`, `build`, `cmake-build-*`, and
hidden directories by default. Use `TL_SourceFindConfig` to provide extensions,
custom ignore rules, or include hidden directories.

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
