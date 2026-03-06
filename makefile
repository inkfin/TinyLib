# Makefile for compiling a simple C project
# The output binary will be placed in the target/ directory
# The preprocessor output will be saved to a file
# Optimization flags will be enabled

# Define the build directory
BUILD_DIR := target/
ifndef BUILD_DIR
    $(error BUILD_DIR not defined)
endif

# Define the unity source file
UNITY_SRC := test/test.c

# Define the executable name
EXECUTABLE := combined_test

# Define the compiler and optimization level
CC := clang
CFLAGS := -O2 -Wall -Wextra -std=c23 -I ./include/

# Define the preprocessor output file
PREPROCESSOR_OUTPUT := $(BUILD_DIR)preprocessed_output.c

# Define the rules
all: $(BUILD_DIR)$(EXECUTABLE)

$(BUILD_DIR)$(EXECUTABLE): $(UNITY_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $(UNITY_SRC)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

preprocess: $(PREPROCESSOR_OUTPUT)

$(PREPROCESSOR_OUTPUT): $(UNITY_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -E -P -o $@ $^

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all preprocess clean

# Define the default target
.DEFAULT_GOAL := all
