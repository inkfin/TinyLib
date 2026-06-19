BUILD_DIR := target
BUILD := $(BUILD_DIR)/build
CC ?= cc
BUILD_DEPS := build.c $(wildcard include/tinylib/*.h include/tinylib/*.c)

BUILD_TARGETS := \
	all \
	run \
	preprocess \
	snapshot \
	snapshot-update \
	c99-build \
	c99-run \
	c99-snapshot \
	c99-snapshot-update \
	bundle \
	bundle-test

all: $(BUILD)
	./$(BUILD) all

$(BUILD): $(BUILD_DEPS) | $(BUILD_DIR)
	$(CC) -Iinclude -o $@ build.c

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(filter-out all,$(BUILD_TARGETS)): $(BUILD)
	./$(BUILD) $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: $(BUILD_TARGETS) clean

.DEFAULT_GOAL := all
