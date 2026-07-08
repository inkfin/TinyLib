/* vim: set ft=c : -*- mode: c -*-
 * build.c
 *   TinyLib project build script.
 */

#include "include/tinylib/compile.c"

#include <stdlib.h>

#define BUILD_DIR "target"
#define TEST_BIN BUILD_DIR "/debug/combined_test"
#define C99_TEST_BIN BUILD_DIR "/debug/c99_logging_test"
#define BUNDLE_TEST_BIN BUILD_DIR "/debug/bundle_test"
#define BUNDLE_OUTPUT BUILD_DIR "/tinylib.c"
#define PREPROCESS_OUTPUT BUILD_DIR "/preprocessed_output.c"
#define TEST_OUTPUT BUILD_DIR "/test_output.txt"
#define LOG_OUTPUT BUILD_DIR "/logging_test.log"
#define C99_STDOUT_OUTPUT BUILD_DIR "/c99_test_output.txt"
#define C99_LOG_OUTPUT BUILD_DIR "/c99_logging_output.log"

static
TL_SourceFindConfig
tl_build_tinylib_source_set(void)
{
    TL_SourceFindConfig cfg = {0};

    cfg.root = "include/tinylib";
    cfg.recursive = true;
    tl_source_find_extensions(&cfg, ".h", ".c");
    return cfg;
}

static
const char *
tl_build_compiler_name(void)
{
    const char *cc = getenv("CC");
    return cc ? cc : "clang";
}

static
TL_BuildTarget
tl_build_test_target(const char *name, const TL_CompilePreset *preset)
{
    TL_BuildTarget target = {0};

    target.name = name;
    target.kind = TL_BUILD_TARGET_COMPILE;
    target.compile.output_name = "combined_test";
    target.compile.preset = preset;
    target.compile.always = true;
    target.compile.runnable = true;
    tl_build_compile_sources(&target.compile,
                             "test/main.c",
                             "test/test_macros.c",
                             "test/test_data_struct.c",
                             "test/test_map.c",
                             "test/test_logging.c",
                             "test/test_mem.c",
                             "test/test_strview.c",
                             "test/test_compile.c",
                             "test/test_impl.c");
    tl_build_compile_deps(&target.compile, "build.c");
    tl_build_compile_add_dep_source_set(&target.compile, tl_build_tinylib_source_set());
    return target;
}

static
TL_BuildTarget
tl_build_preprocess_target(void)
{
    TL_BuildTarget target = {0};

    target.name = "preprocess";
    target.kind = TL_BUILD_TARGET_COMPILE;
    target.compile.output_name = "preprocessed_output.c";
    target.compile.preset = &tl_compile_preset_debug;
    tl_build_compile_sources(&target.compile, "test/main.c");
    tl_build_compile_deps(&target.compile, "build.c", "test/main.c");
    tl_build_compile_flags(&target.compile, "-E", "-P");
    tl_build_compile_add_dep_source_set(&target.compile, tl_build_tinylib_source_set());
    return target;
}

static
TL_BuildTarget
tl_build_c99_target(void)
{
    TL_BuildTarget target = {0};

    target.name = "c99-build";
    target.kind = TL_BUILD_TARGET_COMPILE;
    target.compile.output_name = "c99_logging_test";
    target.compile.preset = &tl_compile_preset_debug;
    target.compile.standard = TL_C_STD_C99;
    target.compile.runnable = true;
    tl_build_compile_sources(&target.compile, "test/c99/logging_test.c");
    tl_build_compile_deps(&target.compile, "build.c");
    tl_build_compile_add_dep_source_set(&target.compile, tl_build_tinylib_source_set());
    return target;
}

static
TL_BuildTarget
tl_build_bundle_target(void)
{
    TL_BuildTarget target = {0};

    target.name = "bundle";
    target.kind = TL_BUILD_TARGET_CMD;
    tl_build_cmd_args(&target.cmd, "python3", "tools/bundle.py", "-o", BUNDLE_OUTPUT);
    tl_build_cmd_outputs(&target.cmd, BUNDLE_OUTPUT);
    tl_build_cmd_deps(&target.cmd, "tools/bundle.py");
    tl_build_cmd_add_source_set(&target.cmd, tl_build_tinylib_source_set());
    return target;
}

static
TL_BuildTarget
tl_build_bundle_test_build_target(void)
{
    TL_BuildTarget target = {0};

    target.name = "bundle-test-build";
    target.kind = TL_BUILD_TARGET_COMPILE;
    tl_build_target_deps(&target, "bundle");
    target.compile.output_name = "bundle_test";
    target.compile.preset = &tl_compile_preset_debug;
    target.compile.runnable = true;
    tl_build_compile_sources(&target.compile, "test/test_bundle.c");
    tl_build_compile_deps(&target.compile, "build.c", "test/test_bundle.c", BUNDLE_OUTPUT);
    tl_build_compile_add_dep_source_set(&target.compile, tl_build_tinylib_source_set());
    return target;
}

static
TL_BuildTarget
tl_build_cmd_target(const char *name, const char *dep, const char *argv0)
{
    TL_BuildTarget target = {0};

    target.name = name;
    target.kind = TL_BUILD_TARGET_CMD;
    if (dep) tl_build_target_deps(&target, dep);
    tl_build_cmd_args(&target.cmd, argv0);
    target.cmd.always = true;
    return target;
}

static
TL_BuildTarget
tl_build_c99_run_target(void)
{
    TL_BuildTarget target = tl_build_cmd_target("c99-run", "c99-build", C99_TEST_BIN);

    target.cmd.stdout_path = C99_STDOUT_OUTPUT;
    target.cmd.redirect_stderr = true;
    return target;
}

static
bool
tl_build_clean(void)
{
    if (!tl_remove_dir(BUILD_DIR)) {
        fprintf(stderr, "failed to remove build directory: %s\n", BUILD_DIR);
        return tl_build_target_finish(BUILD_DIR, (TL_CmdResult){0});
    }
    return tl_build_target_finish(BUILD_DIR, (TL_CmdResult){.ok = true});
}

static
bool
tl_build_snapshot(void)
{
    TL_CmdOptions options = {0};

    options.stdout_path = TEST_OUTPUT;
    options.redirect_stderr = true;
    if (!tl_cmd_ex(&options, TEST_BIN).ok) return false;
    if (tl_diff_files("outputs/gnu11/expected_output.txt", TEST_OUTPUT) != 0) return false;
    if (tl_diff_files("outputs/gnu11/expected_logging_output.txt", LOG_OUTPUT) != 0) return false;
    return true;
}

static
bool
tl_build_snapshot_update(void)
{
    TL_CmdOptions options = {0};

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
    if (tl_diff_files("outputs/c99/expected_stdout.txt", C99_STDOUT_OUTPUT) != 0) return false;
    if (tl_diff_files("outputs/c99/expected_log.txt", C99_LOG_OUTPUT) != 0) return false;
    return true;
}

static
bool
tl_build_c99_snapshot_update(void)
{
    return tl_copy_file(C99_STDOUT_OUTPUT, "outputs/c99/expected_stdout.txt") &&
           tl_copy_file(C99_LOG_OUTPUT, "outputs/c99/expected_log.txt");
}

static
TL_BuildTarget
tl_build_callback_target(const char *name, const char *dep, bool (*run)(void))
{
    TL_BuildTarget target = {0};

    target.name = name;
    target.kind = TL_BUILD_TARGET_CALLBACK;
    target.run = run;
    if (dep) tl_build_target_deps(&target, dep);
    return target;
}

static
bool
tl_build_add_targets(TL_BuildConfig *build)
{
    return tl_build_config_add_target(build, tl_build_test_target("all", &tl_compile_preset_debug)) &&
           tl_build_config_add_target(build, tl_build_test_target("debug", &tl_compile_preset_debug)) &&
           tl_build_config_add_target(build, tl_build_test_target("debug-sanitize", &tl_compile_preset_debug_sanitize)) &&
           tl_build_config_add_target(build, tl_build_test_target("release", &tl_compile_preset_release)) &&
           tl_build_config_add_target(build, tl_build_test_target("relwithdebinfo", &tl_compile_preset_relwithdebinfo)) &&
           tl_build_config_add_target(build, tl_build_callback_target("clean", NULL, tl_build_clean)) &&
           tl_build_config_add_target(build, tl_build_preprocess_target()) &&
           tl_build_config_add_target(build, tl_build_callback_target("snapshot", "all", tl_build_snapshot)) &&
           tl_build_config_add_target(build, tl_build_callback_target("snapshot-update", "all", tl_build_snapshot_update)) &&
           tl_build_config_add_target(build, tl_build_c99_target()) &&
           tl_build_config_add_target(build, tl_build_c99_run_target()) &&
           tl_build_config_add_target(build, tl_build_callback_target("c99-snapshot", "c99-run", tl_build_c99_snapshot)) &&
           tl_build_config_add_target(build, tl_build_callback_target("c99-snapshot-update", "c99-run", tl_build_c99_snapshot_update)) &&
           tl_build_config_add_target(build, tl_build_bundle_target()) &&
           tl_build_config_add_target(build, tl_build_bundle_test_build_target()) &&
           tl_build_config_add_target(build, tl_build_cmd_target("bundle-test", "bundle-test-build", BUNDLE_TEST_BIN));
}

int
main(int argc, char **argv)
{
    TL_BuildConfig build = {0};

    build.project_name = "TinyLib";
    build.build_dir = BUILD_DIR;
    build.compiler = tl_build_compiler_name();
    build.standard = TL_C_STD_GNU11;
    build.default_target = "all";
    build.verbose = getenv("VERBOSE") != NULL;
    tl_build_config_includes(&build, "include");
    tl_build_config_defines(&build, "_CRT_SECURE_NO_WARNINGS");
    if (!tl_build_add_targets(&build)) return EXIT_FAILURE;
    return tl_build_run_auto(argc, argv, &build);
}
