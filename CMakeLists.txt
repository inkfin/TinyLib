# cmake-lint: disable=C0301
cmake_minimum_required(VERSION 3.29)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 23)

project(
  tinylib
  LANGUAGES C
  VERSION 0.0.1)

include_directories(include)

set(TEST_SOURCE_FILES
    test/main.c
    test/test_common.c
    test/test_data_struct.c
    test/test_logging.c
    # test/test_memory_pool.c
)

add_executable(tinylib_test ${TEST_SOURCE_FILES})

# ===== Output Directories ==============

set_target_properties(tinylib_test PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/target/$<CONFIG>"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/target/$<CONFIG>"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/target/$<CONFIG>"
    ## Visual Studio specific
    # VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:tinylib_test>"
    VS_DEBUGGER_COMMAND           "$<TARGET_FILE:tinylib_test>"
)
message(STATUS "Set ${BINARY_NAME} binary output directory to ${CMAKE_SOURCE_DIR}/target")

##########################
## TINYLIB PREPROCESSOR ##
##########################

set(TLPP_RULES_FILE "${CMAKE_SOURCE_DIR}/config/rules.txt"
    CACHE STRING "Rules file for the preprocessor")
set(TLPP_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/config/gen"
    CACHE STRING "Output directory for the preprocessor")

set(TLPP_SOURCE_FILES
    src/preprocessor/tinyppexec.c
)

add_executable(tinyppexec ${TLPP_SOURCE_FILES})

target_include_directories(tinyppexec PRIVATE
    include/tinylib/preprocessor
)

target_compile_definitions(tinyppexec PRIVATE
    TLPP_DEBUG=1
    TLPP_VERBOSE=1 # For debugging purposes
    TLPP_RULES_FILE="${TLPP_RULES_FILE}"
    TLPP_OUTPUT_DIR="${TLPP_OUTPUT_DIR}"
)

set(TINYPP_TEST_SOURCE_FILES
    test/preprocessor/test_pp.c
)

add_executable(tinypp_test ${TINYPP_TEST_SOURCE_FILES})

target_include_directories(tinypp_test PRIVATE
    include/tinylib/preprocessor
)

target_compile_definitions(tinypp_test PRIVATE
    TLPP_DEBUG=1
    TLPP_VERBOSE=1 # For debugging purposes
    TLPP_RULES_FILE="${TLPP_RULES_FILE}"
    TLPP_OUTPUT_DIR="${TLPP_OUTPUT_DIR}"
)

