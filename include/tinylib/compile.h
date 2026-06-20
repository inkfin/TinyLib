/* vim: set ft=c : -*- mode: c -*-
 * compile.h
 *   TinyLib helpers for C build scripts.
 *
 *   This module is for small "build.c" programs: write the build logic in C,
 *   compile that program once, and let it compile the rest of your project.
 *   It is inspired by nob-style workflows, but uses TinyLib arrays,
 *   allocators, names, and implementation-unit conventions.
 *
 *   Current implementation notes:
 *     - Process execution and recursive source discovery target POSIX.
 *     - The default compiler command is "cc".
 *     - Compile flags are opt-in; use presets or add flags explicitly.
 *     - Built-in presets provide debug, sanitizer-debug, release, and warning
 *       flags.
 *     - Recursive source discovery is deterministic and sorted.
 *     - Recursive discovery skips .git, target, build, cmake-build-*,
 *       and hidden directories by default.
 *
 *   How to start:
 *
 *     1. Create a build program, for example `build.c`.
 *
 *        #include "tinylib/logging.c"
 *        #include "tinylib/compile.c"
 *
 *        int main(int argc, char **argv)
 *        {
 *            TL_GO_REBUILD_URSELF(argc, argv);
 *
 *            TL_CompileCmd cmd = {0};
 *            tl_compile_cmd_init(&cmd, NULL);
 *            tl_compile_set_compiler(&cmd, "clang");
 *            tl_compile_apply_preset(&cmd, &tl_compile_preset_debug);
 *            tl_compile_apply_preset(&cmd, &tl_compile_preset_warnings);
 *            tl_compile_set_standard(&cmd, TL_C_STD_GNU11);
 *            tl_compile_set_output(&cmd, "target/app");
 *            tl_compile_includes(&cmd, "include");
 *            tl_compile_sources(&cmd, "src/main.c", "src/app.c");
 *
 *            TL_CmdResult result = tl_compile_run(&cmd);
 *            tl_compile_cmd_free(&cmd);
 *            return result.ok ? 0 : 1;
 *        }
 *
 *     2. Bootstrap and run it:
 *
 *        cc -Iinclude -o build build.c
 *        ./build
 *
 *      `TL_GO_REBUILD_URSELF(argc, argv)` checks whether build.c is newer than
 *      the build executable. If it is, the build program recompiles itself and
 *      re-executes the current command.
 *
 *   Recursive source discovery:
 *
 *        const char *exts[] = { ".c" };
 *        TL_SourceFindConfig sources = {
 *            .root = "src",
 *            .extensions = exts,
 *            .extensions_count = 1,
 *        };
 *        tl_compile_add_sources_recursive(&cmd, &sources);
 *
 *      The extension list is caller-configurable, so projects can include
 *      `.c`, generated `.inc`, platform-specific files, or other source-like
 *      inputs as needed. Matching is exact suffix matching and case-sensitive.
 *
 *   Manual rebuild checks:
 *
 *        const char *inputs[] = { "src/main.c", "src/app.c" };
 *        if (tl_needs_rebuild("target/app", inputs, 2) > 0) {
 *            tl_compile_run(&cmd);
 *        }
 *
 *      `tl_needs_rebuild()` returns 1 when the output is missing or older than
 *      an input, 0 when it is up to date, and -1 on stat errors.
 *
 *   Implementation model:
 *
 *     Include `tinylib/compile.h` for declarations. Compile
 *     `tinylib/logging.c` and `tinylib/compile.c`, or include both in exactly
 *     one build-program translation unit. It is intentionally not part of the
 *     default `tinylib/tinylib.c` umbrella implementation unit.
 */
#ifndef TINYLIB_COMPILE_H
#define TINYLIB_COMPILE_H

#include "defs.h"
#include "data_struct.h"
#include "logging.h"
#include "mem.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Built-in compiler executable presets.
 *
 * These values are convenience names for common C compiler commands. Calling
 * tl_compile_set_compiler_kind() updates both `compiler_kind` and the
 * executable string stored in TL_CompileCmd.compiler. Use
 * tl_compile_set_compiler() when you need an exact executable path or command
 * name such as "zig cc", "/usr/bin/clang", or a wrapper script.
 */
typedef enum TL_CompilerKind {
    TL_COMPILER_CC = 0,
    TL_COMPILER_CLANG,
    TL_COMPILER_GCC,
} TL_CompilerKind;

/* C language standard flag.
 *
 * TL_C_STD_DEFAULT emits no explicit standard flag. Other values emit the
 * corresponding GCC/Clang-style `-std=...` argument.
 */
typedef enum TL_CStandard {
    TL_C_STD_DEFAULT = 0,
    TL_C_STD_C99,
    TL_C_STD_C11,
    TL_C_STD_GNU11,
    TL_C_STD_C17,
    TL_C_STD_GNU17,
    TL_C_STD_C23,
} TL_CStandard;

/* Reusable compile configuration fragment.
 *
 * Presets make build defaults explicit instead of hardcoding them into command
 * rendering. A project can apply one or more presets to a command, for example
 * one preset for a target and one preset for a compilation mode.
 *
 * name:
 *   Optional human-readable label for diagnostics or user code.
 *
 * standard:
 *   Optional language standard to set when the preset is applied. Use
 *   TL_C_STD_DEFAULT to leave the command standard unchanged.
 *
 * defines/flags/link_flags/libs:
 *   Arrays of strings to append to the command. Defines should omit the `-D`
 *   prefix and libraries should omit the `-l` prefix, matching the lower-level
 *   tl_compile_add_define() and tl_compile_add_lib() APIs.
 */
typedef struct TL_CompilePreset {
    const char *name;
    TL_CStandard standard;
    const char **defines;
    size_t defines_count;
    const char **flags;
    size_t flags_count;
    const char **link_flags;
    size_t link_flags_count;
    const char **libs;
    size_t libs_count;
} TL_CompilePreset;

/* Built-in presets.
 *
 * tl_compile_preset_debug:
 *   Adds `-Og -g` and define `DEBUG`.
 *
 * tl_compile_preset_debug_sanitize:
 *   Adds the user's common sanitizer-debug setup: `-O0 -g3 -ggdb`,
 *   `-fsanitize=address,undefined`, `-fno-omit-frame-pointer`,
 *   `-fstack-protector-strong`, `-fno-common`, define `DEBUG`, and the
 *   matching sanitizer link flag.
 *
 * tl_compile_preset_release:
 *   Adds `-O2` and define `NDEBUG`.
 *
 * tl_compile_preset_warnings:
 *   Adds `-Wall -Wextra`.
 */
extern const TL_CompilePreset tl_compile_preset_debug;
extern const TL_CompilePreset tl_compile_preset_debug_sanitize;
extern const TL_CompilePreset tl_compile_preset_release;
extern const TL_CompilePreset tl_compile_preset_warnings;

/* Recursive source discovery configuration.
 *
 * root:
 *   Directory to scan. Required.
 *
 * extensions/extensions_count:
 *   Exact, case-sensitive suffixes to keep, such as ".c" or ".h". If
 *   extensions is NULL, discovery defaults to one extension: ".c".
 *
 * ignore_dirs/ignore_dirs_count:
 *   Directory names to skip. If ignore_dirs is NULL, discovery skips ".git",
 *   "target", and "build". Directories whose names begin with
 *   "cmake-build-" are always skipped.
 *
 * include_hidden:
 *   When zero, hidden directories are skipped. Files are still filtered only by
 *   extension. Set non-zero to allow recursion into hidden directories.
 *
 * recursive:
 *   When non-zero, tl_source_find() recurses into child directories. The
 *   tl_compile_add_sources_recursive() helper forces recursive behavior even
 *   when this field is zero.
 */
typedef struct TL_SourceFindConfig {
    const char *root;
    const char **extensions;
    size_t extensions_count;
    const char **ignore_dirs;
    size_t ignore_dirs_count;
    int include_hidden;
    int recursive;
} TL_SourceFindConfig;

/* Mutable compile command state.
 *
 * Initialize with tl_compile_cmd_init() before use and release with
 * tl_compile_cmd_free(). The string arrays are TinyLib dynamic arrays of owned
 * duplicated strings; callers may inspect them but should mutate through the
 * tl_compile_add_* helpers unless they intentionally manage the invariants.
 *
 * allocator:
 *   Allocator used for command-owned strings and arrays. NULL at init resolves
 *   to tl_default_allocator.
 *
 * compiler_kind/compiler:
 *   Preset kind and executable string. `compiler` is the command argv[0].
 *
 * standard:
 *   C standard used by tl_compile_render_argv().
 *
 * echo:
 *   When non-zero, tl_compile_run() prints the rendered command to stderr
 *   before executing it.
 *
 * output:
 *   Optional output path emitted as `-o <output>`.
 *
 * sources/include_dirs/defines/flags/link_flags/libs:
 *   Ordered command components. Render order is compiler, standard, include
 *   dirs, defines, flags, output, sources, link flags, libraries.
 */
typedef struct TL_CompileCmd {
    TL_Allocator *allocator;
    TL_CompilerKind compiler_kind;
    TL_CStandard standard;
    int echo;

    char *compiler;
    char *output;
    char **sources;
    char **include_dirs;
    char **defines;
    char **flags;
    char **link_flags;
    char **libs;
} TL_CompileCmd;

/* Result from tl_compile_run().
 *
 * ok is non-zero only when the child process exits normally with status 0.
 * exit_code is the child exit status when available, or -1 when process start
 * or wait failed.
 */
typedef struct TL_CmdResult {
    int exit_code;
    b32_t ok;
} TL_CmdResult;

/* Generic command execution options.
 *
 * echo:
 *   When non-zero, print the command before running it.
 *
 * stdout_path:
 *   Optional path to receive stdout. When redirect_stderr is non-zero, stderr
 *   is redirected to the same file.
 *
 * redirect_stderr:
 *   Redirect stderr to stdout_path. Ignored when stdout_path is NULL.
 */
typedef struct TL_CmdOptions {
    int echo;
    const char *stdout_path;
    int redirect_stderr;
} TL_CmdOptions;

/* Build-log configuration.
 *
 * The compile module uses tinylib/logging.h for build-script output. Call
 * tl_build_log_init() near the start of main() to get compact build-style
 * messages instead of the default source-location logger prefix.
 *
 * project_name/build_dir:
 *   Optional values printed during configuration.
 *
 * verbose:
 *   When non-zero, command argv lines are printed. When zero, callers can still
 *   emit higher-level target messages.
 */
typedef struct TL_BuildLogConfig {
    const char *project_name;
    const char *build_dir;
    int verbose;
} TL_BuildLogConfig;

/* Build target dispatch entry.
 *
 * Use with tl_build_dispatch() to keep build.c target routing table-driven.
 */
typedef struct TL_BuildTarget {
    const char *name;
    b32_t (*run)(void);
} TL_BuildTarget;

/* Initialize a compile command.
 *
 * Sets default compiler "cc", default C standard, echo enabled, and empty
 * owned arrays. No optimization, warning, or debug/release flags are added
 * automatically; apply presets or append flags explicitly. `allocator` may be
 * NULL for tl_default_allocator.
 *
 * Returns non-zero on success. On failure, do not use the partially initialized
 * command except to pass it to tl_compile_cmd_free().
 */
b32_t
tl_compile_cmd_init(TL_CompileCmd *cmd, TL_Allocator *allocator);

/* Release all memory owned by a compile command.
 *
 * Safe to call with NULL. After release, the command is zeroed and may be
 * initialized again.
 */
void
tl_compile_cmd_free(TL_CompileCmd *cmd);

/* Set the compiler executable string.
 *
 * Replaces TL_CompileCmd.compiler with an owned copy of `compiler`.
 * Returns non-zero on success.
 */
b32_t
tl_compile_set_compiler(TL_CompileCmd *cmd, const char *compiler);

/* Set the compiler by built-in preset.
 *
 * Updates both compiler_kind and compiler executable string.
 * Returns non-zero on success.
 */
b32_t
tl_compile_set_compiler_kind(TL_CompileCmd *cmd, TL_CompilerKind kind);

/* Set the C language standard used during argv rendering. */
b32_t
tl_compile_set_standard(TL_CompileCmd *cmd, TL_CStandard standard);

/* Apply a reusable compile preset.
 *
 * Appends preset defines, flags, linker flags, and libraries to `cmd` in the
 * order stored in the preset. If preset->standard is not TL_C_STD_DEFAULT, the
 * command standard is set to that value.
 *
 * Returns zero if any append fails. Earlier successful appends are kept.
 */
b32_t
tl_compile_apply_preset(TL_CompileCmd *cmd, const TL_CompilePreset *preset);

/* Set the output path emitted as `-o <path>`.
 *
 * Replaces any previous output path with an owned copy.
 */
b32_t
tl_compile_set_output(TL_CompileCmd *cmd, const char *path);

/* Append one source path.
 *
 * The path is duplicated and emitted after `-o <output>` during argv rendering.
 */
b32_t
tl_compile_add_source(TL_CompileCmd *cmd, const char *path);

/* Append one include directory.
 *
 * The path is duplicated and rendered as `-I<path>`.
 */
b32_t
tl_compile_add_include(TL_CompileCmd *cmd, const char *path);

/* Append one preprocessor define.
 *
 * The define is duplicated and rendered as `-D<define>`. Pass only the define
 * body, for example "DEBUG" or "APP_VERSION=1".
 */
b32_t
tl_compile_add_define(TL_CompileCmd *cmd, const char *define);

/* Append one raw compile flag.
 *
 * Raw flags are emitted after default mode/warning/standard/include/define
 * flags and before output/source arguments.
 */
b32_t
tl_compile_add_flag(TL_CompileCmd *cmd, const char *flag);

/* Append one raw linker flag.
 *
 * Link flags are emitted after sources and before libraries.
 */
b32_t
tl_compile_add_link_flag(TL_CompileCmd *cmd, const char *flag);

/* Append one library name.
 *
 * The library is rendered as `-l<lib>`. Pass "m", not "-lm".
 */
b32_t
tl_compile_add_lib(TL_CompileCmd *cmd, const char *lib);

/* Append multiple source paths.
 *
 * Appends in the order provided. Returns zero if any append fails; earlier
 * successful appends are kept.
 */
b32_t
tl_compile_add_sources(TL_CompileCmd *cmd, size_t count, const char **paths);

/* Append multiple include directories in the order provided. */
b32_t
tl_compile_add_includes(TL_CompileCmd *cmd, size_t count, const char **paths);

/* Append multiple raw compile flags in the order provided. */
b32_t
tl_compile_add_flags(TL_CompileCmd *cmd, size_t count, const char **flags);

/* Find source-like files under cfg->root.
 *
 * On success, stores a TinyLib dynamic array of owned strings in *out_sources.
 * Results are sorted lexicographically for deterministic command generation.
 * The caller must free the result with tl_source_find_free().
 *
 * Returns non-zero on success. Returns zero for invalid arguments, allocation
 * failure, or inability to open the root directory.
 */
b32_t
tl_source_find(const TL_SourceFindConfig *cfg, char ***out_sources);

/* Free a source array returned by tl_source_find(). */
void
tl_source_find_free(char **sources);

/* Discover sources recursively and append them to a compile command.
 *
 * This is a convenience wrapper around tl_source_find() and
 * tl_compile_add_source(). It forces recursive discovery regardless of
 * cfg->recursive. Discovered paths are duplicated into `cmd`, so the temporary
 * discovery array is released before this function returns.
 */
b32_t
tl_compile_add_sources_recursive(TL_CompileCmd *cmd, const TL_SourceFindConfig *cfg);

/* Render a compile command as a NULL-terminated argv array.
 *
 * The returned array and every string in it are owned by the caller and must be
 * released with tl_compile_argv_free(cmd, argv). The final NULL sentinel is
 * included in the TinyLib array length but is ignored by tl_compile_argv_free().
 */
b32_t
tl_compile_render_argv(const TL_CompileCmd *cmd, char ***argv_out);

/* Free an argv array returned by tl_compile_render_argv(). */
void
tl_compile_argv_free(const TL_CompileCmd *cmd, char **argv);

/* Render and execute a compile command.
 *
 * POSIX builds use fork/execvp/waitpid and do not invoke a shell. If cmd->echo
 * is non-zero, the rendered command is printed to stderr before execution.
 */
TL_CmdResult
tl_compile_run(TL_CompileCmd *cmd);

/* Initialize compact build logging through tinylib/logging.h. */
b32_t
tl_build_log_init(const TL_BuildLogConfig *cfg);

/* Print one configuration setting, such as compiler or mode. */
void
tl_build_log_setting(const char *name, const char *value);

/* Print one build event line. */
void
tl_build_log_event(const char *kind, const char *name, const char *detail);

/* Print a build summary from explicit counters. */
void
tl_build_log_summary(size_t built, size_t skipped, size_t failed);

/* Run a generic command argv.
 *
 * argv must be NULL-terminated. This does not invoke a shell.
 */
TL_CmdResult
tl_cmd_run(const char *const argv[]);

/* Run a generic command argv with options such as echo and capture.
 *
 * argv must be NULL-terminated. When options is NULL, echo is enabled and no
 * output redirection is used.
 */
TL_CmdResult
tl_cmd_run_ex(const char *const argv[], const TL_CmdOptions *options);

/* Create one directory if it does not already exist. */
b32_t
tl_mkdir_if_needed(const char *path);

/* Copy one file, replacing the destination. */
b32_t
tl_copy_file(const char *src_path, const char *dst_path);

/* Run `diff -u expected actual`.
 *
 * Returns the command exit code, where 0 means no difference.
 */
int
tl_diff_files(const char *expected_path, const char *actual_path);

/* Return whether an output should be rebuilt from a set of inputs.
 *
 * Returns:
 *   1  output is missing or older than at least one input
 *   0  output exists and is up to date
 *  -1  stat error or invalid arguments
 */
int
tl_needs_rebuild(const char *output_path, const char **input_paths, size_t input_paths_count);

/* Single-input convenience wrapper around tl_needs_rebuild(). */
int
tl_needs_rebuild1(const char *output_path, const char *input_path);

/* Rebuild check over explicit inputs plus discovered source sets.
 *
 * This is useful for build scripts that want a target to depend on a small
 * explicit list, such as build.c or a generator script, plus recursive source
 * discovery results. Returns the same values as tl_needs_rebuild().
 */
int
tl_needs_rebuild_with_sources(const char *output_path,
                              const char **input_paths,
                              size_t input_paths_count,
                              const TL_SourceFindConfig *source_sets,
                              size_t source_sets_count);

/* Rebuild and re-execute the current build program when its source is newer.
 *
 * `argc` and `argv` should come from main(). `source_path` is usually __FILE__,
 * and TL_GO_REBUILD_URSELF supplies that automatically. The compiler used for
 * self-rebuild is getenv("CC") when set, otherwise "cc".
 *
 * On successful self-rebuild this function replaces the current process with
 * execv(). On rebuild failure it exits the process with a non-zero status.
 */
void
tl_go_rebuild_urself(int argc, char **argv, const char *source_path);

/* Dispatch argv[1] through a target table.
 *
 * default_target is used when no argv[1] is supplied. Target callbacks use the
 * TinyLib boolean convention: non-zero means success. The dispatcher converts
 * that result to a process status code: 0 for success, 1 for failure. On
 * unknown targets, this prints a compact usage message and returns 1.
 */
int
tl_build_dispatch(int argc,
                  char **argv,
                  const TL_BuildTarget *targets,
                  size_t targets_count,
                  const char *default_target);

#define TL__COMPILE_COUNT_ARGS(...) \
    (sizeof((const char *[]){ __VA_ARGS__ }) / sizeof(const char *))

/* Variadic convenience wrapper for tl_cmd_run(). */
#define tl_cmd(...) \
    tl_cmd_run((const char *const[]){ __VA_ARGS__, NULL })

/* Variadic convenience wrapper for tl_cmd_run_ex(). */
#define tl_cmd_ex(options, ...) \
    tl_cmd_run_ex((const char *const[]){ __VA_ARGS__, NULL }, (options))

/* Variadic convenience wrapper for tl_compile_add_sources(). */
#define tl_compile_sources(cmd, ...) \
    tl_compile_add_sources((cmd), TL__COMPILE_COUNT_ARGS(__VA_ARGS__), (const char *[]){ __VA_ARGS__ })

/* Variadic convenience wrapper for tl_compile_add_includes(). */
#define tl_compile_includes(cmd, ...) \
    tl_compile_add_includes((cmd), TL__COMPILE_COUNT_ARGS(__VA_ARGS__), (const char *[]){ __VA_ARGS__ })

/* Variadic convenience wrapper for tl_compile_add_flags(). */
#define tl_compile_flags(cmd, ...) \
    tl_compile_add_flags((cmd), TL__COMPILE_COUNT_ARGS(__VA_ARGS__), (const char *[]){ __VA_ARGS__ })

/* Self-rebuild convenience macro that passes __FILE__ as the build source. */
#define TL_GO_REBUILD_URSELF(argc, argv) tl_go_rebuild_urself((argc), (argv), __FILE__)

#if defined(TL_COMPILE_SHORT_NAMES) || defined(TL_SHORT_NAMES)
typedef TL_CompilerKind CompilerKind;
typedef TL_CStandard CStandard;
typedef TL_CompilePreset CompilePreset;
typedef TL_SourceFindConfig SourceFindConfig;
typedef TL_CompileCmd CompileCmd;
typedef TL_CmdResult CmdResult;
typedef TL_CmdOptions CmdOptions;
typedef TL_BuildLogConfig BuildLogConfig;
typedef TL_BuildTarget BuildTarget;
#define cmd tl_cmd
#define cmd_ex tl_cmd_ex
#define GO_REBUILD_URSELF TL_GO_REBUILD_URSELF
#endif

#ifdef __cplusplus
}
#endif

#endif /* TINYLIB_COMPILE_H */
