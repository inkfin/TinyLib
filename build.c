/* vim: set ft=c : -*- mode: c -*-
 * build.c
 *   TinyLib project build script.
 */

#include "include/tinylib/compile.c"

#include <stdlib.h>

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

static const char *tl_build_common_includes[] = {
    "include",
};

static const char *tl_build_common_defines[] = {
    "_CRT_SECURE_NO_WARNINGS",
};

static const char *tl_build_deps[] = {
    "build.c",
};

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

static const char *tl_build_bundle_test_sources[] = {
    "test/test_bundle.c",
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

static const char *tl_build_preprocess_deps[] = {
    "build.c",
    "test/main.c",
};

static const char *tl_build_preprocess_sources[] = {
    "test/main.c",
};

static const char *tl_build_preprocess_flags[] = {
    "-E",
    "-P",
};

static const char *tl_build_bundle_deps[] = {
    "tools/bundle.py",
};

static const char *tl_build_bundle_outputs[] = {
    BUNDLE_OUTPUT,
};

static const char *tl_build_bundle_cmd[] = {
    "python3",
    "tools/bundle.py",
    "-o",
    BUNDLE_OUTPUT,
    NULL,
};

static const char *tl_build_bundle_test_deps[] = {
    "build.c",
    "test/test_bundle.c",
    BUNDLE_OUTPUT,
};

static const char *tl_build_test_cmd[] = {
    TEST_BIN,
    NULL,
};

static const char *tl_build_c99_cmd[] = {
    C99_TEST_BIN,
    NULL,
};

static const char *tl_build_bundle_test_cmd[] = {
    BUNDLE_TEST_BIN,
    NULL,
};

static const char *tl_build_dep_all[] = { "all" };
static const char *tl_build_dep_c99[] = { "c99-build" };
static const char *tl_build_dep_c99_run[] = { "c99-run" };
static const char *tl_build_dep_bundle[] = { "bundle" };
static const char *tl_build_dep_bundle_test_build[] = { "bundle-test-build" };

static
const char *
tl_build_compiler_name(void)
{
    const char *cc = getenv("CC");
    return cc ? cc : "clang";
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

int
main(int argc, char **argv)
{
    TL_BuildConfig build = {0};
    static const TL_BuildTarget targets[] = {
        {
            .name = "all",
            .kind = TL_BUILD_TARGET_COMPILE,
            .compile = {
                .output_name = "combined_test",
                .preset = &tl_compile_preset_debug,
                tl_build_compile_sources_array(tl_build_test_sources),
                tl_build_compile_deps_array(tl_build_deps),
                tl_build_compile_dep_source_sets_array(tl_build_tinylib_sources),
            },
        },
        {
            .name = "debug",
            .kind = TL_BUILD_TARGET_COMPILE,
            .compile = {
                .output_name = "combined_test",
                .preset = &tl_compile_preset_debug,
                .always = true,
                tl_build_compile_sources_array(tl_build_test_sources),
                tl_build_compile_deps_array(tl_build_deps),
                tl_build_compile_dep_source_sets_array(tl_build_tinylib_sources),
            },
        },
        {
            .name = "debug-sanitize",
            .kind = TL_BUILD_TARGET_COMPILE,
            .compile = {
                .output_name = "combined_test",
                .preset = &tl_compile_preset_debug_sanitize,
                .always = true,
                tl_build_compile_sources_array(tl_build_test_sources),
                tl_build_compile_deps_array(tl_build_deps),
                tl_build_compile_dep_source_sets_array(tl_build_tinylib_sources),
            },
        },
        {
            .name = "release",
            .kind = TL_BUILD_TARGET_COMPILE,
            .compile = {
                .output_name = "combined_test",
                .preset = &tl_compile_preset_release,
                .always = true,
                tl_build_compile_sources_array(tl_build_test_sources),
                tl_build_compile_deps_array(tl_build_deps),
                tl_build_compile_dep_source_sets_array(tl_build_tinylib_sources),
            },
        },
        {
            .name = "relwithdebinfo",
            .kind = TL_BUILD_TARGET_COMPILE,
            .compile = {
                .output_name = "combined_test",
                .preset = &tl_compile_preset_relwithdebinfo,
                .always = true,
                tl_build_compile_sources_array(tl_build_test_sources),
                tl_build_compile_deps_array(tl_build_deps),
                tl_build_compile_dep_source_sets_array(tl_build_tinylib_sources),
            },
        },
        {
            .name = "clean",
            .kind = TL_BUILD_TARGET_CALLBACK,
            .run = tl_build_clean,
        },
        {
            .name = "run",
            .kind = TL_BUILD_TARGET_CMD,
            tl_build_target_deps_array(tl_build_dep_all),
            .cmd = {
                .argv = tl_build_test_cmd,
                .always = true,
            },
        },
        {
            .name = "preprocess",
            .kind = TL_BUILD_TARGET_COMPILE,
            .compile = {
                .output_name = "preprocessed_output.c",
                .preset = &tl_compile_preset_debug,
                tl_build_compile_sources_array(tl_build_preprocess_sources),
                tl_build_compile_deps_array(tl_build_preprocess_deps),
                tl_build_compile_flags_array(tl_build_preprocess_flags),
                tl_build_compile_dep_source_sets_array(tl_build_tinylib_sources),
            },
        },
        {
            .name = "snapshot",
            .kind = TL_BUILD_TARGET_CALLBACK,
            tl_build_target_deps_array(tl_build_dep_all),
            .run = tl_build_snapshot,
        },
        {
            .name = "snapshot-update",
            .kind = TL_BUILD_TARGET_CALLBACK,
            tl_build_target_deps_array(tl_build_dep_all),
            .run = tl_build_snapshot_update,
        },
        {
            .name = "c99-build",
            .kind = TL_BUILD_TARGET_COMPILE,
            .compile = {
                .output_name = "c99_logging_test",
                .preset = &tl_compile_preset_debug,
                .standard = TL_C_STD_C99,
                tl_build_compile_sources_array(tl_build_c99_sources),
                tl_build_compile_deps_array(tl_build_deps),
                tl_build_compile_dep_source_sets_array(tl_build_tinylib_sources),
            },
        },
        {
            .name = "c99-run",
            .kind = TL_BUILD_TARGET_CMD,
            tl_build_target_deps_array(tl_build_dep_c99),
            .cmd = {
                .argv = tl_build_c99_cmd,
                .stdout_path = C99_STDOUT_OUTPUT,
                .redirect_stderr = true,
                .always = true,
            },
        },
        {
            .name = "c99-snapshot",
            .kind = TL_BUILD_TARGET_CALLBACK,
            tl_build_target_deps_array(tl_build_dep_c99_run),
            .run = tl_build_c99_snapshot,
        },
        {
            .name = "c99-snapshot-update",
            .kind = TL_BUILD_TARGET_CALLBACK,
            tl_build_target_deps_array(tl_build_dep_c99_run),
            .run = tl_build_c99_snapshot_update,
        },
        {
            .name = "bundle",
            .kind = TL_BUILD_TARGET_CMD,
            .cmd = {
                .argv = tl_build_bundle_cmd,
                tl_build_cmd_outputs_array(tl_build_bundle_outputs),
                tl_build_cmd_deps_array(tl_build_bundle_deps),
                tl_build_cmd_source_sets_array(tl_build_tinylib_sources),
            },
        },
        {
            .name = "bundle-test-build",
            .kind = TL_BUILD_TARGET_COMPILE,
            tl_build_target_deps_array(tl_build_dep_bundle),
            .compile = {
                .output_name = "bundle_test",
                .preset = &tl_compile_preset_debug,
                tl_build_compile_sources_array(tl_build_bundle_test_sources),
                tl_build_compile_deps_array(tl_build_bundle_test_deps),
                tl_build_compile_dep_source_sets_array(tl_build_tinylib_sources),
            },
        },
        {
            .name = "bundle-test",
            .kind = TL_BUILD_TARGET_CMD,
            tl_build_target_deps_array(tl_build_dep_bundle_test_build),
            .cmd = {
                .argv = tl_build_bundle_test_cmd,
                .always = true,
            },
        },
    };

    build.project_name = "TinyLib";
    build.build_dir = BUILD_DIR;
    build.compiler = tl_build_compiler_name();
    build.standard = TL_C_STD_GNU11;
    build.include_dirs = tl_build_common_includes;
    build.include_dirs_count = TL_COUNT_OF(tl_build_common_includes);
    build.defines = tl_build_common_defines;
    build.defines_count = TL_COUNT_OF(tl_build_common_defines);
    build.default_target = "all";
    build.targets = targets;
    build.targets_count = TL_COUNT_OF(targets);
    build.verbose = getenv("VERBOSE") != NULL;
    return tl_build_run_auto(argc, argv, &build);
}
