/* vim: set ft=c : -*- mode: c -*-
 * build.c
 *   TinyLib project build driver.
 */

#include "include/tinylib/compile.c"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

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
};

static const char *tl_build_c99_sources[] = {
    "test/c99/logging_test.c",
};

static const char *tl_build_bundle_test_sources[] = {
    "test/test_bundle.c",
};

static const char *tl_build_optimized_flags[] = {
    "-O2",
};

static const TL_CompilePreset tl_build_preset_optimized = {
    .name = "tinylib-test-optimized",
    .flags = tl_build_optimized_flags,
    .flags_count = TL_COUNT_OF(tl_build_optimized_flags),
};

static
int
tl_build_mkdir(const char *path)
{
    if (mkdir(path, 0777) == 0) return 1;
    return errno == EEXIST;
}

static
int
tl_build_ensure_target(void)
{
    if (tl_build_mkdir(BUILD_DIR)) return 1;
    fprintf(stderr, "build: failed to create %s: %s\n", BUILD_DIR, strerror(errno));
    return 0;
}

static
void
tl_build_print_argv(char *const argv[])
{
    size_t i;

    fprintf(stderr, "[build]");
    for (i = 0; argv[i]; ++i) {
        fprintf(stderr, " %s", argv[i]);
    }
    fputc('\n', stderr);
}

static
int
tl_build_wait(pid_t pid)
{
    int status = 0;

    if (waitpid(pid, &status, 0) < 0) {
        fprintf(stderr, "build: waitpid failed: %s\n", strerror(errno));
        return 1;
    }
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    return 1;
}

static
int
tl_build_run(char *const argv[])
{
    pid_t pid;

    tl_build_print_argv(argv);
    pid = fork();
    if (pid == 0) {
        execvp(argv[0], argv);
        fprintf(stderr, "build: failed to execute %s: %s\n", argv[0], strerror(errno));
        _exit(127);
    }
    if (pid < 0) {
        fprintf(stderr, "build: fork failed: %s\n", strerror(errno));
        return 1;
    }
    return tl_build_wait(pid);
}

static
int
tl_build_run_redirect(char *const argv[], const char *output_path)
{
    pid_t pid;

    tl_build_print_argv(argv);
    pid = fork();
    if (pid == 0) {
        int fd = open(output_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd < 0) {
            fprintf(stderr, "build: failed to open %s: %s\n", output_path, strerror(errno));
            _exit(1);
        }
        if (dup2(fd, STDOUT_FILENO) < 0 || dup2(fd, STDERR_FILENO) < 0) {
            fprintf(stderr, "build: failed to redirect output: %s\n", strerror(errno));
            close(fd);
            _exit(1);
        }
        close(fd);
        execvp(argv[0], argv);
        fprintf(stderr, "build: failed to execute %s: %s\n", argv[0], strerror(errno));
        _exit(127);
    }
    if (pid < 0) {
        fprintf(stderr, "build: fork failed: %s\n", strerror(errno));
        return 1;
    }
    return tl_build_wait(pid);
}

static
int
tl_build_copy_file(const char *src_path, const char *dst_path)
{
    FILE *src;
    FILE *dst;
    char buffer[8192];
    size_t n;
    int ok = 1;

    src = fopen(src_path, "rb");
    if (!src) {
        fprintf(stderr, "build: failed to open %s: %s\n", src_path, strerror(errno));
        return 0;
    }
    dst = fopen(dst_path, "wb");
    if (!dst) {
        fprintf(stderr, "build: failed to open %s: %s\n", dst_path, strerror(errno));
        fclose(src);
        return 0;
    }
    while ((n = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        if (fwrite(buffer, 1, n, dst) != n) {
            fprintf(stderr, "build: failed to write %s: %s\n", dst_path, strerror(errno));
            ok = 0;
            break;
        }
    }
    if (ferror(src)) {
        fprintf(stderr, "build: failed to read %s: %s\n", src_path, strerror(errno));
        ok = 0;
    }
    fclose(dst);
    fclose(src);
    return ok;
}

static
int
tl_build_find_tinylib_inputs(char ***out_sources)
{
    const char *exts[] = { ".h", ".c" };
    TL_SourceFindConfig cfg = {0};

    cfg.root = "include/tinylib";
    cfg.extensions = exts;
    cfg.extensions_count = TL_COUNT_OF(exts);
    cfg.recursive = 1;
    return tl_source_find(&cfg, out_sources);
}

static
int
tl_build_push_deps(const char ***deps, const char **paths, size_t count)
{
    size_t i;

    for (i = 0; i < count; ++i) {
        if (!tl_arr_push(*deps, paths[i])) return 0;
    }
    return 1;
}

static
int
tl_build_push_source_deps(const char ***deps, char **sources)
{
    size_t i;

    for (i = 0; i < tl_arr_len(sources); ++i) {
        if (!tl_arr_push(*deps, sources[i])) return 0;
    }
    return 1;
}

static
int
tl_build_needs_rebuild(const char *output, const char **fixed, size_t fixed_count)
{
    const char **deps = NULL;
    char **tinylib_inputs = NULL;
    int needs;

    tl_arr_init(deps, NULL);
    if (!tl_arr_push(deps, "build.c") ||
        !tl_build_push_deps(&deps, fixed, fixed_count) ||
        !tl_build_find_tinylib_inputs(&tinylib_inputs) ||
        !tl_build_push_source_deps(&deps, tinylib_inputs)) {
        tl_arr_free(deps);
        tl_source_find_free(tinylib_inputs);
        return 1;
    }

    needs = tl_needs_rebuild(output, deps, tl_arr_len(deps));
    tl_arr_free(deps);
    tl_source_find_free(tinylib_inputs);
    return needs != 0;
}

static
int
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
int
tl_build_compile_common(TL_CompileCmd *cmd, const char *output)
{
    const char *cc = getenv("CC");

    if (!cc) cc = "clang";
    return tl_compile_cmd_init(cmd, NULL) &&
           tl_compile_set_compiler(cmd, cc) &&
           tl_compile_set_standard(cmd, TL_C_STD_GNU11) &&
           tl_compile_apply_preset(cmd, &tl_compile_preset_warnings) &&
           tl_build_apply_mode(cmd) &&
           tl_compile_add_define(cmd, "_CRT_SECURE_NO_WARNINGS") &&
           tl_compile_add_include(cmd, "include") &&
           tl_compile_set_output(cmd, output);
}

static
int
tl_build_compile_test(void)
{
    TL_CompileCmd cmd = {0};
    TL_CmdResult result;

    if (!tl_build_ensure_target()) return 1;
    if (!tl_build_needs_rebuild(TEST_BIN, tl_build_test_sources, TL_COUNT_OF(tl_build_test_sources))) {
        return 0;
    }
    if (!tl_build_compile_common(&cmd, TEST_BIN) ||
        !tl_compile_add_sources(&cmd, TL_COUNT_OF(tl_build_test_sources), tl_build_test_sources)) {
        tl_compile_cmd_free(&cmd);
        return 1;
    }
    result = tl_compile_run(&cmd);
    tl_compile_cmd_free(&cmd);
    return result.ok ? 0 : 1;
}

static
int
tl_build_preprocess(void)
{
    TL_CompileCmd cmd = {0};
    TL_CmdResult result;
    const char *deps[] = { "test/main.c" };

    if (!tl_build_ensure_target()) return 1;
    if (!tl_build_needs_rebuild(PREPROCESS_OUTPUT, deps, TL_COUNT_OF(deps))) return 0;
    if (!tl_build_compile_common(&cmd, PREPROCESS_OUTPUT) ||
        !tl_compile_add_flag(&cmd, "-E") ||
        !tl_compile_add_flag(&cmd, "-P") ||
        !tl_compile_add_source(&cmd, "test/main.c")) {
        tl_compile_cmd_free(&cmd);
        return 1;
    }
    result = tl_compile_run(&cmd);
    tl_compile_cmd_free(&cmd);
    return result.ok ? 0 : 1;
}

static
int
tl_build_c99(void)
{
    TL_CompileCmd cmd = {0};
    TL_CmdResult result;

    if (!tl_build_ensure_target()) return 1;
    if (!tl_build_needs_rebuild(C99_TEST_BIN, tl_build_c99_sources, TL_COUNT_OF(tl_build_c99_sources))) {
        return 0;
    }
    if (!tl_build_compile_common(&cmd, C99_TEST_BIN) ||
        !tl_compile_set_standard(&cmd, TL_C_STD_C99) ||
        !tl_compile_add_sources(&cmd, TL_COUNT_OF(tl_build_c99_sources), tl_build_c99_sources)) {
        tl_compile_cmd_free(&cmd);
        return 1;
    }
    result = tl_compile_run(&cmd);
    tl_compile_cmd_free(&cmd);
    return result.ok ? 0 : 1;
}

static
int
tl_build_bundle(void)
{
    const char **deps = NULL;
    char **tinylib_inputs = NULL;
    int needs;
    char *const argv[] = {
        "python3",
        "tools/bundle.py",
        "-o",
        BUNDLE_OUTPUT,
        NULL,
    };

    if (!tl_build_ensure_target()) return 1;
    tl_arr_init(deps, NULL);
    if (!tl_arr_push(deps, "tools/bundle.py") ||
        !tl_build_find_tinylib_inputs(&tinylib_inputs) ||
        !tl_build_push_source_deps(&deps, tinylib_inputs)) {
        tl_arr_free(deps);
        tl_source_find_free(tinylib_inputs);
        return 1;
    }
    needs = tl_needs_rebuild(BUNDLE_OUTPUT, deps, tl_arr_len(deps));
    tl_arr_free(deps);
    tl_source_find_free(tinylib_inputs);
    if (needs == 0) return 0;
    return tl_build_run(argv);
}

static
int
tl_build_bundle_test(void)
{
    TL_CompileCmd cmd = {0};
    TL_CmdResult result;
    const char *deps[] = { "test/test_bundle.c", BUNDLE_OUTPUT };

    if (tl_build_bundle() != 0) return 1;
    if (!tl_build_needs_rebuild(BUNDLE_TEST_BIN, deps, TL_COUNT_OF(deps))) return 0;
    if (!tl_build_compile_common(&cmd, BUNDLE_TEST_BIN) ||
        !tl_compile_add_source(&cmd, "test/test_bundle.c")) {
        tl_compile_cmd_free(&cmd);
        return 1;
    }
    result = tl_compile_run(&cmd);
    tl_compile_cmd_free(&cmd);
    return result.ok ? 0 : 1;
}

static
int
tl_build_run_test(void)
{
    char *const argv[] = { TEST_BIN, NULL };

    if (tl_build_compile_test() != 0) return 1;
    return tl_build_run(argv);
}

static
int
tl_build_run_c99(void)
{
    char *const argv[] = { C99_TEST_BIN, NULL };

    if (tl_build_c99() != 0) return 1;
    return tl_build_run_redirect(argv, C99_STDOUT_OUTPUT);
}

static
int
tl_build_run_bundle_test(void)
{
    char *const argv[] = { BUNDLE_TEST_BIN, NULL };

    if (tl_build_bundle_test() != 0) return 1;
    return tl_build_run(argv);
}

static
int
tl_build_diff(const char *expected, const char *actual)
{
    char *const argv[] = { "diff", "-u", (char *)expected, (char *)actual, NULL };
    return tl_build_run(argv);
}

static
int
tl_build_snapshot(void)
{
    char *const argv[] = { TEST_BIN, NULL };

    if (tl_build_compile_test() != 0) return 1;
    if (tl_build_run_redirect(argv, TEST_OUTPUT) != 0) return 1;
    if (tl_build_diff("outputs/gnu11/expected_output.txt", TEST_OUTPUT) != 0) return 1;
    if (tl_build_diff("outputs/gnu11/expected_logging_output.txt", LOG_OUTPUT) != 0) return 1;
    return 0;
}

static
int
tl_build_snapshot_update(void)
{
    char *const argv[] = { TEST_BIN, NULL };

    if (tl_build_compile_test() != 0) return 1;
    if (tl_build_run_redirect(argv, TEST_OUTPUT) != 0) return 1;
    if (!tl_build_copy_file(TEST_OUTPUT, "outputs/gnu11/expected_output.txt")) return 1;
    if (!tl_build_copy_file(LOG_OUTPUT, "outputs/gnu11/expected_logging_output.txt")) return 1;
    return 0;
}

static
int
tl_build_c99_snapshot(void)
{
    if (tl_build_run_c99() != 0) return 1;
    if (tl_build_diff("outputs/c99/expected_stdout.txt", C99_STDOUT_OUTPUT) != 0) return 1;
    if (tl_build_diff("outputs/c99/expected_log.txt", C99_LOG_OUTPUT) != 0) return 1;
    return 0;
}

static
int
tl_build_c99_snapshot_update(void)
{
    if (tl_build_run_c99() != 0) return 1;
    if (!tl_build_copy_file(C99_STDOUT_OUTPUT, "outputs/c99/expected_stdout.txt")) return 1;
    if (!tl_build_copy_file(C99_LOG_OUTPUT, "outputs/c99/expected_log.txt")) return 1;
    return 0;
}

static
void
tl_build_usage(const char *program)
{
    fprintf(stderr, "usage: %s [target]\n", program);
    fprintf(stderr, "targets: all run preprocess snapshot snapshot-update c99-build c99-run\n");
    fprintf(stderr, "         c99-snapshot c99-snapshot-update bundle bundle-test\n");
}

int
main(int argc, char **argv)
{
    const char *target;

    TL_GO_REBUILD_URSELF(argc, argv);

    target = argc > 1 ? argv[1] : "all";
    if (strcmp(target, "all") == 0) return tl_build_compile_test();
    if (strcmp(target, "run") == 0) return tl_build_run_test();
    if (strcmp(target, "preprocess") == 0) return tl_build_preprocess();
    if (strcmp(target, "snapshot") == 0) return tl_build_snapshot();
    if (strcmp(target, "snapshot-update") == 0) return tl_build_snapshot_update();
    if (strcmp(target, "c99-build") == 0) return tl_build_c99();
    if (strcmp(target, "c99-run") == 0) return tl_build_run_c99();
    if (strcmp(target, "c99-snapshot") == 0) return tl_build_c99_snapshot();
    if (strcmp(target, "c99-snapshot-update") == 0) return tl_build_c99_snapshot_update();
    if (strcmp(target, "bundle") == 0) return tl_build_bundle();
    if (strcmp(target, "bundle-test") == 0) return tl_build_run_bundle_test();

    tl_build_usage(argv[0]);
    return 1;
}
