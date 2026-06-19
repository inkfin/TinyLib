# Makefile for compiling a simple C project
# The output binary will be placed in the target/ directory
# The preprocessor output will be saved to a file
# Optimization flags will be enabled

# Define the build directory
BUILD_DIR := target/
ifndef BUILD_DIR
    $(error BUILD_DIR not defined)
endif

# Test source files (non-unity build)
TEST_SRCS := test/main.c \
			 test/test_macros.c \
			 test/test_data_struct.c \
			 test/test_map.c \
			 test/test_logging.c \
			 test/test_mem.c \
			 test/test_strview.c \
			 test/test_compile.c
PREPROCESS_SRC := test/main.c

# Define the executable name
EXECUTABLE := combined_test

# Define the compiler and build mode
CC := clang
MODE ?= release

COMMON_FLAGS := -Wall -Wextra -D_CRT_SECURE_NO_WARNINGS -I ./include/

ifeq ($(MODE),dbg)
OPT_FLAGS := -Og -g -DDEBUG
else
OPT_FLAGS := -O2
endif

# Optional sanitizers:
#   make <target> SANITIZE=1
SANITIZE ?= 0
SAN_FLAGS :=
ifeq ($(SANITIZE),1)
SAN_FLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer -g
endif

GNU11_CFLAGS := $(OPT_FLAGS) $(COMMON_FLAGS) -std=gnu11 $(SAN_FLAGS)
C99_CFLAGS := $(OPT_FLAGS) $(COMMON_FLAGS) -std=c99 $(SAN_FLAGS)

# Define the preprocessor output file
PREPROCESSOR_OUTPUT := $(BUILD_DIR)preprocessed_output.c
TEST_OUTPUT := $(BUILD_DIR)test_output.txt
EXPECTED_TEST_OUTPUT := outputs/gnu11/expected_output.txt
LOG_OUTPUT := $(BUILD_DIR)logging_test.log
EXPECTED_LOG_OUTPUT := outputs/gnu11/expected_logging_output.txt

C99_TEST_SRC := test/c99/logging_test.c
C99_TEST_EXECUTABLE := c99_logging_test
C99_STDOUT_OUTPUT := $(BUILD_DIR)c99_test_output.txt
C99_LOG_OUTPUT := $(BUILD_DIR)c99_logging_output.log
C99_EXPECTED_STDOUT := outputs/c99/expected_stdout.txt
C99_EXPECTED_LOG := outputs/c99/expected_log.txt

BUNDLE_OUTPUT := $(BUILD_DIR)tinylib.c
BUNDLE_TEST_SRC := test/test_bundle.c
BUNDLE_TEST_EXECUTABLE := bundle_test

# Define the rules
all: $(BUILD_DIR)$(EXECUTABLE)

$(BUILD_DIR)$(EXECUTABLE): $(TEST_SRCS) | $(BUILD_DIR)
	$(CC) $(GNU11_CFLAGS) -o $@ $(TEST_SRCS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

preprocess: $(PREPROCESSOR_OUTPUT)

$(PREPROCESSOR_OUTPUT): $(PREPROCESS_SRC) | $(BUILD_DIR)
	$(CC) $(GNU11_CFLAGS) -E -P -o $@ $^

run: $(BUILD_DIR)$(EXECUTABLE)
	./$(BUILD_DIR)$(EXECUTABLE)

c99-build: $(BUILD_DIR)$(C99_TEST_EXECUTABLE)

$(BUILD_DIR)$(C99_TEST_EXECUTABLE): $(C99_TEST_SRC) | $(BUILD_DIR)
	$(CC) $(C99_CFLAGS) -o $@ $<

c99-run: c99-build
	./$(BUILD_DIR)$(C99_TEST_EXECUTABLE) > $(C99_STDOUT_OUTPUT) 2>&1

c99-snapshot: c99-run
	diff -u $(C99_EXPECTED_STDOUT) $(C99_STDOUT_OUTPUT)
	diff -u $(C99_EXPECTED_LOG) $(C99_LOG_OUTPUT)

c99-snapshot-update: c99-run
	mkdir -p outputs/c99
	cp $(C99_STDOUT_OUTPUT) $(C99_EXPECTED_STDOUT)
	cp $(C99_LOG_OUTPUT) $(C99_EXPECTED_LOG)

snapshot: $(BUILD_DIR)$(EXECUTABLE) | $(BUILD_DIR)
	./$(BUILD_DIR)$(EXECUTABLE) > $(TEST_OUTPUT) 2>&1
	diff -u $(EXPECTED_TEST_OUTPUT) $(TEST_OUTPUT)
	diff -u $(EXPECTED_LOG_OUTPUT) $(LOG_OUTPUT)

snapshot-update: $(BUILD_DIR)$(EXECUTABLE) | $(BUILD_DIR)
	./$(BUILD_DIR)$(EXECUTABLE) > $(TEST_OUTPUT) 2>&1
	mkdir -p outputs/gnu11
	cp $(TEST_OUTPUT) $(EXPECTED_TEST_OUTPUT)
	cp $(LOG_OUTPUT) $(EXPECTED_LOG_OUTPUT)

bundle: $(BUNDLE_OUTPUT)

$(BUNDLE_OUTPUT): tools/bundle.py include/tinylib/*.h include/tinylib/*.c | $(BUILD_DIR)
	python3 tools/bundle.py -o $@

$(BUILD_DIR)$(BUNDLE_TEST_EXECUTABLE): $(BUNDLE_TEST_SRC) $(BUNDLE_OUTPUT) | $(BUILD_DIR)
	$(CC) $(GNU11_CFLAGS) -o $@ $(BUNDLE_TEST_SRC)

bundle-test: $(BUILD_DIR)$(BUNDLE_TEST_EXECUTABLE)
	./$(BUILD_DIR)$(BUNDLE_TEST_EXECUTABLE)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all preprocess run snapshot snapshot-update c99-build c99-run c99-snapshot c99-snapshot-update bundle bundle-test clean

# Define the default target
.DEFAULT_GOAL := all
