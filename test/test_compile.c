#define TL_COMPILE_EXPOSE_INTERNALS
#include "../include/tinylib/compile.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <utime.h>

static void
test_write_file(const char *path, const char *text)
{
    FILE *f = fopen(path, "w");
    assert(f != NULL);
    fputs(text, f);
    fclose(f);
}

static void
test_mkdir(const char *path)
{
    if (mkdir(path, 0777) != 0) {
        /* Existing directories are fine for repeat test runs. */
    }
}

static void
test_set_mtime(const char *path, time_t t)
{
    struct utimbuf times;
    times.actime = t;
    times.modtime = t;
    assert(utime(path, &times) == 0);
}

static int
test_argv_contains(char **argv, const char *needle)
{
    size_t i;
    for (i = 0; argv[i]; ++i) {
        if (strcmp(argv[i], needle) == 0) return 1;
    }
    return 0;
}

static size_t
test_argv_index(char **argv, const char *needle)
{
    size_t i;
    for (i = 0; argv[i]; ++i) {
        if (strcmp(argv[i], needle) == 0) return i;
    }
    return (size_t)-1;
}

static void
compile_render_test(void)
{
    TL_CompileCmd cmd = {0};
    TL_CompileCmd sanitize_cmd = {0};
    char **argv = NULL;
    size_t output_idx;

    assert(tl_compile_cmd_init(&cmd, NULL));
    assert(tl_compile_set_compiler(&cmd, "clang"));
    assert(tl_compile_set_standard(&cmd, TL_C_STD_GNU11));
    assert(tl_compile_set_output(&cmd, "target/app"));
    assert(tl_compile_include(&cmd, "include"));
    assert(tl_compile_include(&cmd, "third_party"));
    assert(tl_compile_define(&cmd, "APP=1"));
    assert(tl_compile_flag(&cmd, "-Werror"));
    assert(tl_compile_sources(&cmd, "src/main.c", "src/util.c"));
    assert(tl_compile_link_flag(&cmd, "-pthread"));
    assert(tl_compile_lib(&cmd, "m"));

    assert(tl_compile_render_argv(&cmd, &argv));
    assert(strcmp(argv[0], "clang") == 0);
    assert(!test_argv_contains(argv, "-O2"));
    assert(!test_argv_contains(argv, "-DNDEBUG"));
    assert(!test_argv_contains(argv, "-Wall"));
    assert(!test_argv_contains(argv, "-Wextra"));
    tl_compile_argv_free(&cmd, argv);
    argv = NULL;

    assert(tl_compile_apply_preset(&cmd, &tl_compile_preset_release));
    assert(tl_compile_render_argv(&cmd, &argv));
    assert(test_argv_contains(argv, "-O2"));
    assert(test_argv_contains(argv, "-DNDEBUG"));
    assert(test_argv_contains(argv, "-Wall"));
    assert(test_argv_contains(argv, "-Wextra"));
    assert(test_argv_contains(argv, "-std=gnu11"));
    assert(test_argv_contains(argv, "-Iinclude"));
    assert(test_argv_contains(argv, "-Ithird_party"));
    assert(test_argv_contains(argv, "-DAPP=1"));
    assert(test_argv_contains(argv, "-Werror"));
    assert(test_argv_contains(argv, "src/main.c"));
    assert(test_argv_contains(argv, "src/util.c"));
    assert(test_argv_contains(argv, "-pthread"));
    assert(test_argv_contains(argv, "-lm"));

    output_idx = test_argv_index(argv, "-o");
    assert(output_idx != (size_t)-1);
    assert(strcmp(argv[output_idx + 1U], "target/app") == 0);

    tl_compile_argv_free(&cmd, argv);
    argv = NULL;
    tl_compile_cmd_free(&cmd);

    assert(tl_compile_cmd_init(&sanitize_cmd, NULL));
    assert(tl_compile_apply_preset(&sanitize_cmd, &tl_compile_preset_debug_sanitize));
    assert(tl_compile_render_argv(&sanitize_cmd, &argv));
    assert(test_argv_contains(argv, "-O0"));
    assert(test_argv_contains(argv, "-g3"));
    assert(test_argv_contains(argv, "-ggdb"));
    assert(test_argv_contains(argv, "-fsanitize=address,undefined"));
    assert(test_argv_contains(argv, "-fno-omit-frame-pointer"));
    assert(test_argv_contains(argv, "-fstack-protector-strong"));
    assert(test_argv_contains(argv, "-fno-common"));
    assert(test_argv_contains(argv, "-DDEBUG"));

    tl_compile_argv_free(&sanitize_cmd, argv);
    tl_compile_cmd_free(&sanitize_cmd);
}

static void
compile_source_find_test(void)
{
    TL_SourceFindConfig cfg = {0};
    char **sources = NULL;
    TL_CompileCmd cmd = {0};

    test_mkdir("target");
    test_mkdir("target/tl_compile_find");
    test_mkdir("target/tl_compile_find/src");
    test_mkdir("target/tl_compile_find/src/nested");
    test_mkdir("target/tl_compile_find/target");
    test_mkdir("target/tl_compile_find/.hidden");
    test_mkdir("target/tl_compile_find/cmake-build-debug");

    test_write_file("target/tl_compile_find/src/a.c", "int a(void) { return 1; }\n");
    test_write_file("target/tl_compile_find/src/b.h", "int b(void);\n");
    test_write_file("target/tl_compile_find/src/nested/c.c", "int c(void) { return 3; }\n");
    test_write_file("target/tl_compile_find/target/ignored.c", "int ignored(void) { return 0; }\n");
    test_write_file("target/tl_compile_find/.hidden/hidden.c", "int hidden(void) { return 0; }\n");
    test_write_file("target/tl_compile_find/cmake-build-debug/generated.c", "int generated(void) { return 0; }\n");

    cfg.root = "target/tl_compile_find";
    cfg.recursive = true;
    assert(tl_source_find_extensions(&cfg, ".c", ".h"));

    assert(tl_source_find(&cfg, &sources));
    assert(tl_arr_len(sources) == 3);
    assert(strcmp(sources[0], "target/tl_compile_find/src/a.c") == 0);
    assert(strcmp(sources[1], "target/tl_compile_find/src/b.h") == 0);
    assert(strcmp(sources[2], "target/tl_compile_find/src/nested/c.c") == 0);
    tl_source_find_free(sources);

    assert(tl_compile_cmd_init(&cmd, NULL));
    assert(tl_compile_add_sources_recursive(&cmd, &cfg));
    assert(tl_arr_len(cmd.sources) == 3);
    tl_compile_cmd_free(&cmd);
}

static void
compile_rebuild_test(void)
{
    const char *input = "target/tl_compile_rebuild_input.c";
    const char *output = "target/tl_compile_rebuild_output";

    test_mkdir("target");
    remove(input);
    remove(output);
    test_write_file(input, "int main(void) { return 0; }\n");
    assert(tl_needs_rebuild1(output, input) == 1);

    test_write_file(output, "binary\n");
    test_set_mtime(input, 100);
    test_set_mtime(output, 200);
    assert(tl_needs_rebuild1(output, input) == 0);

    test_set_mtime(input, 300);
    assert(tl_needs_rebuild1(output, input) == 1);
}

static void
compile_smoke_compile_test(void)
{
    const char *cc = getenv("CC");
    TL_CompileCmd cmd = {0};
    TL_CmdResult result;

    test_mkdir("target");
    test_write_file("target/tl_compile_smoke.c", "int main(void) { return 0; }\n");

    if (!cc) cc = "cc";
    assert(tl_compile_cmd_init(&cmd, NULL));
    cmd.echo = false;
    assert(tl_compile_set_compiler(&cmd, cc));
    assert(tl_compile_apply_preset(&cmd, &tl_compile_preset_debug));
    assert(tl_compile_set_standard(&cmd, TL_C_STD_C99));
    assert(tl_compile_set_output(&cmd, "target/tl_compile_smoke"));
    assert(tl_compile_source(&cmd, "target/tl_compile_smoke.c"));

    result = tl_compile_run(&cmd);
    assert(result.ok);

    tl_compile_cmd_free(&cmd);
}

static void
compile_build_helpers_test(void)
{
    const char *inputs[] = { "target/tl_compile_helpers/input.txt" };
    TL_SourceFindConfig source_set = {0};
    TL_CmdOptions options = {0};
    TL_CmdResult result;

    assert(tl_mkdir_if_needed("target"));
    assert(tl_mkdir_if_needed("target/tl_compile_helpers"));
    test_write_file("target/tl_compile_helpers/input.txt", "copy me\n");
    assert(tl_copy_file("target/tl_compile_helpers/input.txt", "target/tl_compile_helpers/copy.txt"));

    options.echo = false;
    options.stdout_path = "target/tl_compile_helpers/cc_version.txt";
    options.redirect_stderr = true;
    result = tl_cmd_ex(&options, "cc", "--version");
    assert(result.ok);

    test_write_file("target/tl_compile_helpers/source.c", "int helper(void) { return 1; }\n");
    source_set.root = "target/tl_compile_helpers";
    source_set.recursive = true;
    assert(tl_source_find_extension(&source_set, ".c"));
    {
        TL_SourceFindConfig source_sets[] = { source_set };
        assert(tl_needs_rebuild_with_sources_array("target/tl_compile_helpers/missing",
                                                   inputs,
                                                   source_sets) == 1);
    }
}

static void
compile_build_config_target_test(void)
{
    const char *cc = getenv("CC");
    TL_BuildTarget target = {0};
    TL_BuildConfig build = {0};
    char *argv[] = { "build-config-test" };
    struct stat st;

    test_mkdir("target");
    test_write_file("target/tl_build_config_input.c",
                    "#ifndef TL_BUILD_CONFIG_DEFINE\n"
                    "#error missing build config define\n"
                    "#endif\n"
                    "int main(void) { return 0; }\n");

    build.project_name = "BuildConfigTest";
    build.build_dir = "target/tl_build_config";
    build.compiler = cc ? cc : "cc";
    build.standard = TL_C_STD_C99;
    build.default_target = "app";
    assert(tl_build_config_define(&build, "TL_BUILD_CONFIG_DEFINE=1"));

    target.name = "app";
    target.kind = TL_BUILD_TARGET_COMPILE;
    target.compile.output_name = "app";
    target.compile.preset = &tl_compile_preset_release;
    target.compile.runnable = true;
    assert(tl_build_compile_source(&target.compile, "target/tl_build_config_input.c"));
    assert(tl_build_config_add_target(&build, target));

    assert(tl_build_run(1, argv, &build) == EXIT_SUCCESS);
    assert(stat("target/tl_build_config", &st) == 0 && S_ISDIR(st.st_mode));
    assert(stat("target/tl_build_config/release", &st) == 0 && S_ISDIR(st.st_mode));
    assert(stat("target/tl_build_config/release/app", &st) == 0);
    {
        char *run_argv[] = { "build-config-test", "run" };
        assert(tl_build_run(2, run_argv, &build) == EXIT_SUCCESS);
    }
}

int
compile_test_cases(void)
{
    puts("- Compile Helper Test Cases");

    compile_render_test();
    compile_source_find_test();
    compile_rebuild_test();
    compile_smoke_compile_test();
    compile_build_helpers_test();
    compile_build_config_target_test();

    return 0;
}
