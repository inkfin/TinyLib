cmake_minimum_required(VERSION 3.10)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 11)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

project(
  tinylib
  LANGUAGES C
  VERSION 0.0.1)

include_directories(include)

set(SOURCE_FILES
    test/main.c
    test/test_dyn_arr.c
    test/test_logging.c
    # test/test_memory_pool.c
)

add_executable(tinylib_test ${SOURCE_FILES})

# ===== Output Directories ==============

set_target_properties(tinylib_test PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/target/$<CONFIG>"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/target/$<CONFIG>"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/target/$<CONFIG>"
    ## Visual Studio specific
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:tinylib_test>"
    VS_DEBUGGER_COMMAND           "$<TARGET_FILE:tinylib_test>"
    VS_DEBUGGER_ENVIRONMENT       "PATH=%PATH%"
)
message(STATUS "Set ${BINARY_NAME} binary output directory to ${CMAKE_SOURCE_DIR}/target")

