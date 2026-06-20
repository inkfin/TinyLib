/* vim: set ft=c : -*- mode: c -*-
 * build.c
 *   TinyLib project build script.
 */

#include "include/tinylib/compile.c"

#include <stdlib.h>
#include <string.h>

#define BUILD_DIR "target"
#define TEST_BIN BUILD_DIR "/combined_test"
#define C99_TEST_BIN BUILD_DIR "/c99_logging_test"
#define BUNDLE_TEST_BIN BUILD_DIR "/bundle_test"
#define BUNDLE_OUTPUT BUILD_DIR "/tinylib.c"
#define PREPROCESS_OUTPUT BUILD_DIR "/preprocessed_output.c"
#define TEST_OUTPUT BUILD_DIR "/test_output.txt"
#define LOG_OUTPUT BUILD_DIR "/logging_test.log"
#define C99_STDOUT_OUTPUT BUILD_DIR "/c99_test_output.txt"
#define C99_LOG_OUTPUT BUILD_DIR "/c99_logging_output.log"

static const char *tl_build_test_sources[] = {
    "test/main.c",
    "test/test_macros.c",
    "test/test_data_struct.c",
    "test/test_map.c",
    "test/test_logging.c",
    "test/test_mem.c",
    "test/test_strview.c",
    "test/test_compile.c",
    "test/test_impl.c",
};

static const char *tl_build_c99_sources[] = {
    "test/c99/logging_test.c",
};

static const char *tl_build_optimized_flags[] = {
    "-O2",
};

static const TL_CompilePreset tl_build_preset_optimized = {
    .name = "tinylib-test-optimized",
    .flags = tl_build_optimized_flags,
    .flags_count = TL_COUNT_OF(tl_build_optimized_flags),
};

static const char *tl_build_tinylib_exts[] = {
    ".h",
    ".c",
};

static const TL_SourceFindConfig tl_build_tinylib_sources[] = {
    {
        .root = "include/tinylib",
        .extensions = tl_build_tinylib_exts,
        .extensions_count = TL_COUNT_OF(tl_build_tinylib_exts),
        .recursive = true,
    },
};

static
bool
tl_build_ok(TL_CmdResult result)
{
    return result.ok;
}

static
bool
tl_build_prepare(void)
{
    return tl_mkdir_if_needed(BUILD_DIR);
}

static
bool
tl_build_needs_tinylib_rebuild(const char *output, const char **inputs, size_t inputs_count)
{
    return tl_needs_rebuild_with_sources(output,
                                         inputs,
                                         inputs_count,
                                         tl_build_tinylib_sources,
                                         TL_COUNT_OF(tl_build_tinylib_sources)) != 0;
}

static
bool
tl_build_apply_mode(TL_CompileCmd *cmd)
{
    const char *mode = getenv("MODE");
    const char *sanitize = getenv("SANITIZE");

    if ((sanitize && strcmp(sanitize, "1") == 0) ||
        (mode && (strcmp(mode, "sanitize") == 0 || strcmp(mode, "asan") == 0))) {
        return tl_compile_apply_preset(cmd, &tl_compile_preset_debug_sanitize);
    }
    if (mode && (strcmp(mode, "dbg") == 0 || strcmp(mode, "debug") == 0)) {
        return tl_compile_apply_preset(cmd, &tl_compile_preset_debug);
    }
    if (mode && strcmp(mode, "release-ndebug") == 0) {
        return tl_compile_apply_preset(cmd, &tl_compile_preset_release);
    }
    return tl_compile_apply_preset(cmd, &tl_build_preset_optimized);
}

static
const char *
tl_build_mode_name(void)
{
    const char *mode = getenv("MODE");
    const char *sanitize = getenv("SANITIZE");

    if ((sanitize && strcmp(sanitize, "1") == 0) ||
        (mode && (strcmp(mode, "sanitize") == 0 || strcmp(mode, "asan") == 0))) {
        return "debug-sanitize";
    }
    if (mode && (strcmp(mode, "dbg") == 0 || strcmp(mode, "debug") == 0)) {
        return "debug";
    }
    if (mode && strcmp(mode, "release-ndebug") == 0) {
        return "release-ndebug";
    }
    return "optimized";
}

static
const char *
tl_build_compiler_name(void)
{
    const char *cc = getenv("CC");
    return cc ? cc : "clang";
}

static
void
tl_build_compile_common(TL_CompileCmd *cmd, const char *output)
{
    const char *cc = getenv("CC");

    if (!cc) cc = "clang";
    tl_compile_cmd_init(cmd, NULL);
    cmd->echo = tl_build_is_verbose();
    tl_compile_set_compiler(cmd, cc);
    tl_compile_set_standard(cmd, TL_C_STD_GNU11);
    tl_compile_apply_preset(cmd, &tl_compile_preset_warnings);
    tl_build_apply_mode(cmd);
    tl_compile_define(cmd, "_CRT_SECURE_NO_WARNINGS");
    tl_compile_include(cmd, "include");
    tl_compile_set_output(cmd, output);
}

static
bool
tl_build_compile_sources(const char *output,
                         TL_CStandard standard,
                         const char **sources,
                         size_t sources_count)
{
    const char *deps[] = { "build.c" };
    TL_CompileCmd cmd = {0};

    if (!tl_build_prepare()) return false;
    if (!tl_build_needs_tinylib_rebuild(output, deps, TL_COUNT_OF(deps)) &&
        tl_needs_rebuild(output, sources, sources_count) == 0) {
        return tl_build_target_skipped(output, "up to date");
    }
    tl_build_target_building(output, "compile");
    tl_build_compile_common(&cmd, output);
    tl_compile_set_standard(&cmd, standard);
    tl_compile_add_sources(&cmd, sources, sources_count);
    return tl_build_target_finish(output, tl_compile_run(&cmd));
}

static
bool
tl_build_compile_test(void)
{
    return tl_build_compile_sources(TEST_BIN,
                                    TL_C_STD_GNU11,
                                    tl_build_test_sources,
                                    TL_COUNT_OF(tl_build_test_sources));
}

static
bool
tl_build_c99(void)
{
    return tl_build_compile_sources(C99_TEST_BIN,
                                    TL_C_STD_C99,
                                    tl_build_c99_sources,
                                    TL_COUNT_OF(tl_build_c99_sources));
}

static
bool
tl_build_clean(void)
{
    tl_build_target_building(BUILD_DIR, "remove");
    if (!tl_remove_dir(BUILD_DIR)) {
        fprintf(stderr, "failed to remove build directory: %s\n", BUILD_DIR);
        return tl_build_target_failed(BUILD_DIR);
    }
    return tl_build_target_built(BUILD_DIR);
}

static
bool
tl_build_preprocess(void)
{
    const char *deps[] = { "build.c", "test/main.c" };
    TL_CompileCmd cmd = {0};

    if (!tl_build_prepare()) return false;
    if (!tl_build_needs_tinylib_rebuild(PREPROCESS_OUTPUT, deps, TL_COUNT_OF(deps))) {
        return tl_build_target_skipped(PREPROCESS_OUTPUT, "up to date");
    }
    tl_build_target_building(PREPROCESS_OUTPUT, "preprocess");
    tl_build_compile_common(&cmd, PREPROCESS_OUTPUT);
    tl_compile_flag(&cmd, "-E");
    tl_compile_flag(&cmd, "-P");
    tl_compile_source(&cmd, "test/main.c");
    return tl_build_target_finish(PREPROCESS_OUTPUT, tl_compile_run(&cmd));
}

static
bool
tl_build_bundle(void)
{
    const char *deps[] = { "tools/bundle.py" };

    if (!tl_build_prepare()) return false;
    if (tl_needs_rebuild_with_sources_array(BUNDLE_OUTPUT,
                                            deps,
                                            tl_build_tinylib_sources) == 0) {
        return tl_build_target_skipped(BUNDLE_OUTPUT, "up to date");
    }
    tl_build_target_building(BUNDLE_OUTPUT, "bundle");
    return tl_build_target_finish(BUNDLE_OUTPUT, tl_cmd("python3", "tools/bundle.py", "-o", BUNDLE_OUTPUT));
}

static
bool
tl_build_bundle_test(void)
{
    const char *sources[] = { "test/test_bundle.c" };
    const char *deps[] = { "build.c", "test/test_bundle.c", BUNDLE_OUTPUT };
    TL_CompileCmd cmd = {0};

    if (!tl_build_bundle()) return false;
    if (!tl_build_needs_tinylib_rebuild(BUNDLE_TEST_BIN, deps, TL_COUNT_OF(deps))) {
        return tl_build_target_skipped(BUNDLE_TEST_BIN, "up to date");
    }
    tl_build_target_building(BUNDLE_TEST_BIN, "compile");
    tl_build_compile_common(&cmd, BUNDLE_TEST_BIN);
    tl_compile_sources_array(&cmd, sources);
    return tl_build_target_finish(BUNDLE_TEST_BIN, tl_compile_run(&cmd));
}

static
bool
tl_build_run_test(void)
{
    if (!tl_build_compile_test()) return false;
    tl_build_target_running(TEST_BIN, NULL);
    return tl_build_ok(tl_cmd(TEST_BIN));
}

static
bool
tl_build_run_c99(void)
{
    TL_CmdOptions options = {0};

    if (!tl_build_c99()) return false;
    tl_build_target_running(C99_TEST_BIN, NULL);
    options.echo = tl_build_is_verbose();
    options.stdout_path = C99_STDOUT_OUTPUT;
    options.redirect_stderr = true;
    return tl_build_ok(tl_cmd_ex(&options, C99_TEST_BIN));
}

static
bool
tl_build_run_bundle_test(void)
{
    if (!tl_build_bundle_test()) return false;
    tl_build_target_running(BUNDLE_TEST_BIN, NULL);
    return tl_build_ok(tl_cmd(BUNDLE_TEST_BIN));
}

static
bool
tl_build_snapshot(void)
{
    TL_CmdOptions options = {0};

    if (!tl_build_compile_test()) return false;
    tl_build_target_running(TEST_BIN, "snapshot");
    options.echo = tl_build_is_verbose();
    options.stdout_path = TEST_OUTPUT;
    options.redirect_stderr = true;
    if (!tl_cmd_ex(&options, TEST_BIN).ok) return false;
    tl_build_target_checking(TEST_OUTPUT, "snapshot");
    if (tl_diff_files("outputs/gnu11/expected_output.txt", TEST_OUTPUT) != 0) return false;
    if (tl_diff_files("outputs/gnu11/expected_logging_output.txt", LOG_OUTPUT) != 0) return false;
    return true;
}

static
bool
tl_build_snapshot_update(void)
{
    TL_CmdOptions options = {0};

    if (!tl_build_compile_test()) return false;
    tl_build_target_running(TEST_BIN, "snapshot update");
    options.echo = tl_build_is_verbose();
    options.stdout_path = TEST_OUTPUT;
    options.redirect_stderr = true;
    if (!tl_cmd_ex(&options, TEST_BIN).ok) return false;
    return tl_copy_file(TEST_OUTPUT, "outputs/gnu11/expected_output.txt") &&
           tl_copy_file(LOG_OUTPUT, "outputs/gnu11/expected_logging_output.txt");
}

static
bool
tl_build_c99_snapshot(void)
{
    if (!tl_build_run_c99()) return false;
    tl_build_target_checking(C99_STDOUT_OUTPUT, "snapshot");
    if (tl_diff_files("outputs/c99/expected_stdout.txt", C99_STDOUT_OUTPUT) != 0) return false;
    if (tl_diff_files("outputs/c99/expected_log.txt", C99_LOG_OUTPUT) != 0) return false;
    return true;
}

static
bool
tl_build_c99_snapshot_update(void)
{
    if (!tl_build_run_c99()) return false;
    return tl_copy_file(C99_STDOUT_OUTPUT, "outputs/c99/expected_stdout.txt") &&
           tl_copy_file(C99_LOG_OUTPUT, "outputs/c99/expected_log.txt");
}

int
main(int argc, char **argv)
{
    TL_BuildConfig build = {0};
    static const TL_BuildTarget targets[] = {
        { "clean", tl_build_clean },
        { "all", tl_build_compile_test },
        { "run", tl_build_run_test },
        { "preprocess", tl_build_preprocess },
        { "snapshot", tl_build_snapshot },
        { "snapshot-update", tl_build_snapshot_update },
        { "c99-build", tl_build_c99 },
        { "c99-run", tl_build_run_c99 },
        { "c99-snapshot", tl_build_c99_snapshot },
        { "c99-snapshot-update", tl_build_c99_snapshot_update },
        { "bundle", tl_build_bundle },
        { "bundle-test", tl_build_run_bundle_test },
    };

    build.project_name = "TinyLib";
    build.build_dir = BUILD_DIR;
    build.compiler = tl_build_compiler_name();
    build.mode = tl_build_mode_name();
    build.default_target = "all";
    build.targets = targets;
    build.targets_count = TL_COUNT_OF(targets);
    build.verbose = getenv("VERBOSE") != NULL;
    return tl_build_run_auto(argc, argv, &build);
}
