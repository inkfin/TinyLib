/* vim: set ft=c : -*- mode: c -*-
 * compile.h
 *   TinyLib helpers for C build scripts.
 *
 *   This module is for small "build.c" programs: write the build logic in C,
 *   compile that program once, and let it compile the rest of your project.
 *   It is inspired by nob.h-style workflows, but it is not a vendored nob.h
 *   layer: the implementation uses TinyLib arrays, allocators, names, logging,
 *   and implementation-unit conventions.
 *
 *   Implementation notes:
 *     - Process execution and recursive source discovery target POSIX.
 *     - The default compiler command is "cc".
 *     - Compile flags are opt-in; use presets or add flags explicitly.
 *     - Built-in presets provide debug, sanitizer-debug, release, and warning
 *       flags.
 *     - Recursive source discovery is deterministic and sorted.
 *     - Recursive discovery skips .git, target, build, cmake-build-*,
 *       and hidden directories by default.
 *     - On failure, every public function prints an error to stderr with a
 *       description and a hint suggesting how to fix the issue.
 *
 *   How to start:
 *
 *     1. Create a build program, for example `build.c`.
 *
 *        #include "tinylib/compile.c"
 *
 *        static
 *        bool
 *        build_app(void)
 *        {
 *            TL_CompileCmd cmd = {0};
 *
 *            tl_compile_cmd_init(&cmd, NULL);
 *            tl_compile_set_compiler(&cmd, "clang");
 *            tl_compile_apply_preset(&cmd, &tl_compile_preset_debug);
 *            tl_compile_apply_preset(&cmd, &tl_compile_preset_warnings);
 *            tl_compile_set_standard(&cmd, TL_C_STD_GNU11);
 *            tl_compile_set_output(&cmd, "target/app");
 *            tl_compile_include(&cmd, "include");
 *            tl_compile_sources(&cmd, "src/main.c", "src/app.c");
 *
 *            tl_build_target_building("target/app", "compile");
 *            return tl_build_target_finish("target/app", tl_compile_run(&cmd));
 *        }
 *
 *        int main(int argc, char **argv)
 *        {
 *            static const TL_BuildTarget targets[] = {
 *                { "app", build_app },
 *            };
 *            TL_BuildConfig build = {
 *                .project_name = "Example",
 *                .build_dir = "target",
 *                .mode = "debug",
 *                .default_target = "app",
 *                .targets = targets,
 *                .targets_count = TL_COUNT_OF(targets),
 *            };
 *
 *            return tl_build_run_auto(argc, argv, &build);
 *        }
 *
 *     2. Bootstrap and run it:
 *
 *        cc -Iinclude -o build build.c
 *        ./build
 *
 *      `tl_build_run_auto()` first checks whether build.c is newer than the
 *      build executable. If it is, the build program recompiles itself and
 *      re-executes the current command before dispatching targets.
 *
 *   Recursive source discovery:
 *
 *        const char *exts[] = { ".c" };
 *        TL_SourceFindConfig sources = {
 *            .root = "src",
 *            .extensions = exts,
 *            .extensions_count = TL_COUNT_OF(exts),
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
 *        if (tl_needs_rebuild_array("target/app", inputs) > 0) {
 *            tl_compile_run(&cmd);
 *        }
 *
 *      `tl_needs_rebuild()` returns 1 when the output is missing or older than
 *      an input, 0 when it is up to date, and -1 on stat errors.
 *
 *   Convenience macros:
 *
 *     Every `tl_compile_add_*` function has a matching convenience macro that
 *     drops the `add_` prefix and accepts items directly:
 *
 *         tl_compile_source(cmd, "src/main.c")                    // one source
 *         tl_compile_sources(cmd, "src/main.c", "src/util.c")     // many sources
 *         tl_compile_sources_array(cmd, my_sources)               // from array
 *
 *         tl_compile_include(cmd, "include")                      // one dir
 *         tl_compile_includes(cmd, "include", "third_party")      // many dirs
 *         tl_compile_includes_array(cmd, my_dirs)                 // from array
 *
 *     The same pattern applies to defines, compile flags, link flags, and
 *     libraries through tl_compile_define / tl_compile_defines,
 *     tl_compile_flag / tl_compile_flags, tl_compile_link_flag / tl_compile_link_flags,
 *     and tl_compile_lib / tl_compile_libs, each with its _array counterpart.
 *
 *   Implementation model:
 *
 *     Include `tinylib/compile.h` for declarations. Compile
 *     `tinylib/compile.c`, or include it in exactly one build-program
 *     translation unit. `compile.c` includes the TinyLib logging implementation
 *     it needs, and implementation guards prevent accidental duplicate
 *     inclusion. It is intentionally not part of the default
 *     `tinylib/tinylib.c` umbrella implementation unit.
 */
#ifndef TINYLIB_COMPILE_H
#define TINYLIB_COMPILE_H

#if !defined(_MSC_VER) && (!defined(__STDC_VERSION__) || (__STDC_VERSION__ < 199901L))
#error "tinylib/compile.h requires C99 or later."
#endif

#include "defs.h"
#include "data_struct.h"
#include "logging.h"
#include "mem.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

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
 *   When false, hidden directories are skipped. Files are still filtered only by
 *   extension. Set true to allow recursion into hidden directories.
 *
 * recursive:
 *   When true, tl_source_find() recurses into child directories. The
 *   tl_compile_add_sources_recursive() helper forces recursive behavior even
 *   when this field is false.
 */
typedef struct TL_SourceFindConfig {
    const char *root;
    const char **extensions;
    size_t extensions_count;
    const char **ignore_dirs;
    size_t ignore_dirs_count;
    bool include_hidden;
    bool recursive;
} TL_SourceFindConfig;

/* Mutable compile command state.
 *
 * Initialize with tl_compile_cmd_init() before use and release with
 * tl_compile_cmd_free(). The string arrays are TinyLib dynamic arrays of owned
 * duplicated strings; callers may inspect them but should mutate through the
 * tl_compile_add_* helpers unless they intentionally manage the invariants.
 *
 * allocator:
 *   Allocator used for command-owned strings and arrays. Pass NULL to use an
 *   automatic internal arena — tl_compile_run() will clean it up for you.
 *
 * compiler:
 *   Executable string, used as argv[0] during rendering.
 *
 * standard:
 *   C standard used by tl_compile_render_argv().
 *
 * echo:
 *   When true, tl_compile_run() prints the rendered command to stderr
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
    TL_CStandard standard;
    bool echo;

    char *compiler;
    char *output;
    char **sources;
    char **include_dirs;
    char **defines;
    char **flags;
    char **link_flags;
    char **libs;

    TL_Arena internal_arena;
    TL_Allocator internal_allocator;
} TL_CompileCmd;

/* Result from tl_compile_run().
 *
 * ok is non-zero only when the child process exits normally with status 0.
 * exit_code is the child exit status when available, or -1 when process start
 * or wait failed.
 */
typedef struct TL_CmdResult {
    int exit_code;
    bool ok;
} TL_CmdResult;

/* Generic command execution options.
 *
 * echo:
 *   When true, print the command before running it.
 *
 * stdout_path:
 *   Optional path to receive stdout. When redirect_stderr is non-zero, stderr
 *   is redirected to the same file.
 *
 * redirect_stderr:
 *   Redirect stderr to stdout_path. Ignored when stdout_path is NULL.
 */
typedef struct TL_CmdOptions {
    bool echo;
    const char *stdout_path;
    bool redirect_stderr;
} TL_CmdOptions;

/* Build target dispatch entry used by TL_BuildConfig. */
typedef struct TL_BuildTarget {
    const char *name;
    bool (*run)(void);
} TL_BuildTarget;

/* Build driver configuration.
 *
 * A build.c normally declares a target table and one TL_BuildConfig, then calls
 * tl_build_run() from main(). The compile module owns help output, compact
 * logging setup, configuration display, target dispatch, and final summary.
 *
 * log_path:
 *   When set, every tl_compile_run() call inside the build redirects the
 *   compiler's stdout and stderr into this file (truncated per run).
 */
typedef struct TL_BuildConfig {
    const char *project_name;
    const char *build_dir;
    const char *mode;
    const char *default_target;
    const char *log_path;
    const TL_BuildTarget *targets;
    size_t targets_count;
    bool verbose;
} TL_BuildConfig;

/* Initialize a compile command.
 *
 * Sets default compiler "cc", default C standard, echo enabled, and empty
 * owned arrays. No optimization, warning, or debug/release flags are added
 * automatically; apply presets or append flags explicitly.
 *
 * Pass NULL for `allocator` to get an automatic internal arena — all memory is
 * pooled and cleaned up automatically by tl_compile_run(). Pass a custom
 * TL_Allocator for manual memory control; in that case, call
 * tl_compile_cmd_free() when done.
 *
 * Returns non-zero on success, zero for NULL cmd. */
bool
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
bool
tl_compile_set_compiler(TL_CompileCmd *cmd, const char *compiler);

/* Set the C language standard used during argv rendering. */
bool
tl_compile_set_standard(TL_CompileCmd *cmd, TL_CStandard standard);

/* Apply a reusable compile preset.
 *
 * Appends preset defines, flags, linker flags, and libraries to `cmd` in the
 * order stored in the preset. If preset->standard is not TL_C_STD_DEFAULT, the
 * command standard is set to that value.
 *
 * Returns zero if any append fails. Earlier successful appends are kept.
 */
bool
tl_compile_apply_preset(TL_CompileCmd *cmd, const TL_CompilePreset *preset);

/* Set the output path emitted as `-o <path>`.
 *
 * Replaces any previous output path with an owned copy.
 */
bool
tl_compile_set_output(TL_CompileCmd *cmd, const char *path);

/* Append one source path.
 *
 * The path is duplicated and emitted after `-o <output>` during argv rendering.
 */
bool
tl_compile_add_source(TL_CompileCmd *cmd, const char *path);

/* Append one include directory.
 *
 * The path is duplicated and rendered as `-I<path>`.
 */
bool
tl_compile_add_include(TL_CompileCmd *cmd, const char *path);

/* Append one preprocessor define.
 *
 * The define is duplicated and rendered as `-D<define>`. Pass only the define
 * body, for example "DEBUG" or "APP_VERSION=1".
 */
bool
tl_compile_add_define(TL_CompileCmd *cmd, const char *define);

/* Append one raw compile flag.
 *
 * Raw flags are emitted after default mode/warning/standard/include/define
 * flags and before output/source arguments.
 */
bool
tl_compile_add_flag(TL_CompileCmd *cmd, const char *flag);

/* Append one raw linker flag.
 *
 * Link flags are emitted after sources and before libraries.
 */
bool
tl_compile_add_link_flag(TL_CompileCmd *cmd, const char *flag);

/* Append one library name.
 *
 * The library is rendered as `-l<lib>`. Pass "m", not "-lm".
 */
bool
tl_compile_add_lib(TL_CompileCmd *cmd, const char *lib);

/* Append multiple source paths.
 *
 * Appends in the order provided. Returns zero if any append fails; earlier
 * successful appends are kept.
 */
bool
tl_compile_add_sources(TL_CompileCmd *cmd, const char **paths, size_t paths_count);

/* Append multiple include directories in the order provided. */
bool
tl_compile_add_includes(TL_CompileCmd *cmd, const char **paths, size_t paths_count);

/* Append multiple raw compile flags in the order provided. */
bool
tl_compile_add_flags(TL_CompileCmd *cmd, const char **flags, size_t flags_count);

/* Append multiple preprocessor defines in the order provided. */
bool
tl_compile_add_defines(TL_CompileCmd *cmd, const char **defines, size_t defines_count);

/* Append multiple raw linker flags in the order provided. */
bool
tl_compile_add_link_flags(TL_CompileCmd *cmd, const char **flags, size_t flags_count);

/* Append multiple library names in the order provided. */
bool
tl_compile_add_libs(TL_CompileCmd *cmd, const char **libs, size_t libs_count);

/* Find source-like files under cfg->root.
 *
 * On success, stores a TinyLib dynamic array of owned strings in *out_sources.
 * Results are sorted lexicographically for deterministic command generation.
 * The caller must free the result with tl_source_find_free().
 *
 * Returns non-zero on success. Returns zero for invalid arguments, allocation
 * failure, or inability to open the root directory.
 */
bool
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
bool
tl_compile_add_sources_recursive(TL_CompileCmd *cmd, const TL_SourceFindConfig *cfg);

/* Render a compile command as a NULL-terminated argv array.
 *
 * The returned array and every string in it are owned by the caller and must be
 * released with tl_compile_argv_free(cmd, argv). The final NULL sentinel is
 * included in the TinyLib array length but is ignored by tl_compile_argv_free().
 */
bool
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
bool
tl_mkdir_if_needed(const char *path);

/* Recursively remove a directory and all its contents.
 *
 * POSIX only; returns 0 on Windows. Symlinks and special files are not followed.
 * Returns non-zero on success.
 */
bool
tl_remove_dir(const char *path);

/* Copy one file, replacing the destination. */
bool
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
 * execv() and never returns. On rebuild failure it exits the process with a
 * non-zero status.
 *
 * Returns non-zero when no rebuild was needed (source is up to date), zero on
 * stat errors or invalid arguments.
 */
bool
tl_go_rebuild_urself(int argc, char **argv, const char *source_path);

/* Run a complete target-table build.
 *
 * Handles help requests, compact logging setup, configuration output, target
 * dispatch, and final summary. Returns a process status code.
 */
int
tl_build_run(int argc, char **argv, const TL_BuildConfig *config);

/* Return whether verbose command output is enabled for the active build. */
bool
tl_build_is_verbose(void);

/* Target lifecycle helpers.
 *
 * These emit compact log lines and maintain the active build summary counts.
 */
void
tl_build_target_building(const char *target, const char *detail);

bool
tl_build_target_skipped(const char *target, const char *reason);

bool
tl_build_target_built(const char *target);

bool
tl_build_target_failed(const char *target);

/* Convenience wrapper: call tl_build_target_built() or tl_build_target_failed()
 * based on the command result. Use this instead of writing the ternary yourself. */
bool
tl_build_target_finish(const char *target, TL_CmdResult result);

void
tl_build_target_running(const char *target, const char *detail);

void
tl_build_target_checking(const char *target, const char *detail);

#define TL__COMPILE_COUNT_ARGS(...) \
    (sizeof((const char *[]){ __VA_ARGS__ }) / sizeof(const char *))

/* Variadic convenience wrapper for tl_cmd_run(). */
#define tl_cmd(...) \
    tl_cmd_run((const char *const[]){ __VA_ARGS__, NULL })

/* Variadic convenience wrapper for tl_cmd_run_ex(). */
#define tl_cmd_ex(options, ...) \
    tl_cmd_run_ex((const char *const[]){ __VA_ARGS__, NULL }, (options))

/* --- source -------------------------------------------------------------- */

#define tl_compile_source(cmd, path) \
    tl_compile_add_source((cmd), (path))

#define tl_compile_sources(cmd, ...) \
    tl_compile_add_sources((cmd), (const char *[]){ __VA_ARGS__ }, TL__COMPILE_COUNT_ARGS(__VA_ARGS__))

#define tl_compile_sources_array(cmd, paths) \
    tl_compile_add_sources((cmd), (paths), TL_COUNT_OF(paths))

/* --- include dir --------------------------------------------------------- */

#define tl_compile_include(cmd, dir) \
    tl_compile_add_include((cmd), (dir))

#define tl_compile_includes(cmd, ...) \
    tl_compile_add_includes((cmd), (const char *[]){ __VA_ARGS__ }, TL__COMPILE_COUNT_ARGS(__VA_ARGS__))

#define tl_compile_includes_array(cmd, paths) \
    tl_compile_add_includes((cmd), (paths), TL_COUNT_OF(paths))

/* --- define -------------------------------------------------------------- */

#define tl_compile_define(cmd, def) \
    tl_compile_add_define((cmd), (def))

#define tl_compile_defines(cmd, ...) \
    tl_compile_add_defines((cmd), (const char *[]){ __VA_ARGS__ }, TL__COMPILE_COUNT_ARGS(__VA_ARGS__))

#define tl_compile_defines_array(cmd, defines) \
    tl_compile_add_defines((cmd), (defines), TL_COUNT_OF(defines))

/* --- compile flag -------------------------------------------------------- */

#define tl_compile_flag(cmd, flag) \
    tl_compile_add_flag((cmd), (flag))

#define tl_compile_flags(cmd, ...) \
    tl_compile_add_flags((cmd), (const char *[]){ __VA_ARGS__ }, TL__COMPILE_COUNT_ARGS(__VA_ARGS__))

#define tl_compile_flags_array(cmd, flags) \
    tl_compile_add_flags((cmd), (flags), TL_COUNT_OF(flags))

/* --- link flag ----------------------------------------------------------- */

#define tl_compile_link_flag(cmd, flag) \
    tl_compile_add_link_flag((cmd), (flag))

#define tl_compile_link_flags(cmd, ...) \
    tl_compile_add_link_flags((cmd), (const char *[]){ __VA_ARGS__ }, TL__COMPILE_COUNT_ARGS(__VA_ARGS__))

#define tl_compile_link_flags_array(cmd, flags) \
    tl_compile_add_link_flags((cmd), (flags), TL_COUNT_OF(flags))

/* --- library ------------------------------------------------------------- */

#define tl_compile_lib(cmd, lib) \
    tl_compile_add_lib((cmd), (lib))

#define tl_compile_libs(cmd, ...) \
    tl_compile_add_libs((cmd), (const char *[]){ __VA_ARGS__ }, TL__COMPILE_COUNT_ARGS(__VA_ARGS__))

#define tl_compile_libs_array(cmd, libs) \
    tl_compile_add_libs((cmd), (libs), TL_COUNT_OF(libs))

/* Rebuild check for a real C array of explicit inputs.
 *
 * `input_paths` must be an array visible at the callsite, not a pointer
 * parameter.
 */
#define tl_needs_rebuild_array(output_path, input_paths) \
    tl_needs_rebuild((output_path), (input_paths), TL_COUNT_OF(input_paths))

/* Rebuild check for real C arrays of explicit inputs and source sets.
 *
 * Both `input_paths` and `source_sets` must be arrays visible at the callsite,
 * not pointer parameters.
 */
#define tl_needs_rebuild_with_sources_array(output_path, input_paths, source_sets) \
    tl_needs_rebuild_with_sources((output_path), \
                                  (input_paths), \
                                  TL_COUNT_OF(input_paths), \
                                  (source_sets), \
                                  TL_COUNT_OF(source_sets))

/* Self-rebuild convenience macro that passes __FILE__ as the build source. */
#define TL_GO_REBUILD_URSELF(argc, argv) tl_go_rebuild_urself((argc), (argv), __FILE__)

/* All-in-one convenience wrapper that self-rebuilds before dispatching targets.
 *
 * Checks whether the build source (__FILE__ at the callsite) is newer than the
 * running binary, recompiles if needed, and re-execs; otherwise falls through
 * to tl_build_run().
 */
#define tl_build_run_auto(argc, argv, config) \
    (tl_go_rebuild_urself((argc), (argv), __FILE__) \
        ? tl_build_run((argc), (argv), (config)) \
        : EXIT_FAILURE)

#ifdef __cplusplus
}
#endif

#endif /* TINYLIB_COMPILE_H */
