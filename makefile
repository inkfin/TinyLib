# Makefile for compiling a simple C project
# The output binary will be placed in the target/ directory
# The preprocessor output will be saved to a file
# Optimization flags will be enabled

# Define the build directory
BUILD_DIR := target/
ifndef BUILD_DIR
    $(error BUILD_DIR not defined)
endif

# Define the source files
SRCS := $(wildcard test/*.c)

# Define the object files
OBJS := $(SRCS:.c=.o)

# Define the executable name
EXECUTABLE := myprogram

# Define the compiler and optimization level
CC := clang
CFLAGS := -O2 -Wall -Wextra -std=c99 -I ./include/

# Define the preprocessor output file
PREPROCESSOR_OUTPUT := preprocessed_output.c

# Define the rules
all: $(BUILD_DIR)$(EXECUTABLE)

$(BUILD_DIR)$(EXECUTABLE): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

preprocess: $(PREPROCESSOR_OUTPUT)

$(PREPROCESSOR_OUTPUT): $(SRCS)
	$(CC) $(CFLAGS) -E -P -o $@ $^

clean:
	rm -rf $(BUILD_DIR) $(PREPROCESSOR_OUTPUT) $(OBJS)

.PHONY: all preprocess clean

# Define the default target
.DEFAULT_GOAL := all
