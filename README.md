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

## Threading

`tinylib/logging.c` serializes global logger configuration and log writes with
an internal platform lock when available. Define `TL_LOG_NO_THREADS` before
compiling the implementation to disable locking. On POSIX platforms, projects
that compile the logging implementation may need to link with pthread support.

## Naming

Public symbols use the `tl_` function prefix and `TL_` type/macro prefix.
Internal helper macros use a double-underscore module form such as `TL_DS__*`.
Short aliases are off by default and require an explicit module opt-in such as
`TL_DS_SHORT_NAMES`, `TL_STR_SHORT_NAMES`, `TL_MEM_SHORT_NAMES`, or
`TL_LOG_SHORT_NAMES`.

## References

<https://github.com/tsoding/data-mining-in-c/blob/main/src/nob.h>

<https://github.com/rxi/log.c>
