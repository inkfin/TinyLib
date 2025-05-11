#!/bin/sh

TEST_NAME=$1

BUILD_DIR=${2:-target}/

CC=${CC:-gcc}

${CC} -o ${BUILD_DIR}${TEST_NAME} test/test_${TEST_NAME}.c -Wall -Wextra -I./include -I./test -DTL_SINGLE_TEST_FILE -D_Debug