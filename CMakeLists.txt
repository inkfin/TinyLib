cmake_minimum_required(VERSION 3.10)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 23)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

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

option(TLPP_RULES_FILE "Rules file for the preprocessor" "${CMAKE_SOURCE_DIR}/config/rules.txt")
option(TLPP_OUTPUT_DIR "Output directory for the preprocessor" "${CMAKE_SOURCE_DIR}/config/gen")

set(TLPP_SOURCE_FILES
    src/preprocessor/preprocessor.c
)

add_executable(tinylib_preprocessor ${TLPP_SOURCE_FILES})

target_include_directories(tinylib_preprocessor PRIVATE
    include/tinylib/preprocessor
)

target_compile_definitions(tinylib_preprocessor PRIVATE
    TINYLIB_PREPROCESSOR_DEBUG=1
    TINYLIB_PREPROCESSOR_VERBOSE=1 # For debugging purposes
    TINYLIB_PREPROCESSOR_RULES_FILE="${TLPP_RULES_FILE}"
    TINYLIB_PREPROCESSOR_OUTPUT_DIR="${TLPP_OUTPUT_DIR}"
)
