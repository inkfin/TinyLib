# cmake-lint: disable=C0301
cmake_minimum_required(VERSION 3.29)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 23)

project(
  tinylib
  LANGUAGES C
  VERSION 0.0.1)

function(tl_dedup_cached_flags var_name)
  if(DEFINED ${var_name})
    separate_arguments(_tl_flags NATIVE_COMMAND "${${var_name}}")
    set(_tl_deduped)
    foreach(_tl_flag IN LISTS _tl_flags)
      if(NOT _tl_flag IN_LIST _tl_deduped)
        list(APPEND _tl_deduped "${_tl_flag}")
      endif()
    endforeach()
    string(REPLACE ";" " " _tl_joined "${_tl_deduped}")
    set(${var_name} "${_tl_joined}" CACHE STRING "" FORCE)
  endif()
endfunction()

tl_dedup_cached_flags(CMAKE_EXE_LINKER_FLAGS)
tl_dedup_cached_flags(CMAKE_MODULE_LINKER_FLAGS)
tl_dedup_cached_flags(CMAKE_SHARED_LINKER_FLAGS)

include_directories(include)

set(TEST_SOURCE_FILES
    test/main.c
    test/test_macros.c
    test/test_data_struct.c
    test/test_map.c
    test/test_logging.c
    test/test_mem.c
    test/test_strview.c
    test/test_compile.c
)

add_executable(tinylib_test ${TEST_SOURCE_FILES})

find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(TINYLIB_BUNDLE_FILE "${CMAKE_SOURCE_DIR}/target/tinylib.c")
set(TINYLIB_BUNDLE_INPUTS
    tools/bundle.py
    include/tinylib/compile.c
    include/tinylib/compile.h
    include/tinylib/c_ext.h
    include/tinylib/data_struct.h
    include/tinylib/defs.h
    include/tinylib/logging.c
    include/tinylib/logging.h
    include/tinylib/macros.h
    include/tinylib/mem.h
    include/tinylib/strview.h
    include/tinylib/tinylib.c
    include/tinylib/tinylib.h
)

add_custom_command(
    OUTPUT "${TINYLIB_BUNDLE_FILE}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_SOURCE_DIR}/target"
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/bundle.py" -o "${TINYLIB_BUNDLE_FILE}"
    DEPENDS ${TINYLIB_BUNDLE_INPUTS}
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    VERBATIM
)

add_custom_target(tinylib_bundle DEPENDS "${TINYLIB_BUNDLE_FILE}")

add_executable(tinylib_bundle_test test/test_bundle.c)
add_dependencies(tinylib_bundle_test tinylib_bundle)

# ===== Output Directories ==============

set_target_properties(tinylib_test PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/target/$<CONFIG>"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/target/$<CONFIG>"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/target/$<CONFIG>"
    ## Visual Studio specific
    # VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:tinylib_test>"
    VS_DEBUGGER_COMMAND           "$<TARGET_FILE:tinylib_test>"
)
message(STATUS "Set tinylib_test binary output directory to ${CMAKE_SOURCE_DIR}/target")

##########################
## TINYLIB PREPROCESSOR ##
##########################

set(TLPP_RULES_FILE "${CMAKE_SOURCE_DIR}/config/rules.txt"
    CACHE STRING "Rules file for the preprocessor")
set(TLPP_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/config/gen"
    CACHE STRING "Output directory for the preprocessor")

set(TLPP_SOURCE_FILES
    preprocessor/lexer.c
    preprocessor/preprocessor.c
    preprocessor/main.c
)

add_executable(tlppc ${TLPP_SOURCE_FILES})

target_include_directories(tlppc PRIVATE
    include/tinylib/preprocessor
)

target_compile_definitions(tlppc PRIVATE
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
    preprocessor
)

target_compile_definitions(tinypp_test PRIVATE
    TLPP_DEBUG=1
    TLPP_VERBOSE=1 # For debugging purposes
    TLPP_RULES_FILE="${TLPP_RULES_FILE}"
    TLPP_OUTPUT_DIR="${TLPP_OUTPUT_DIR}"
)
