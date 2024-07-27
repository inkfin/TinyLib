cmake_minimum_required(VERSION 3.6)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 99)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

project(
  tinylib
  LANGUAGES C
  VERSION 0.0.1)

include_directories(include)
set(SOURCE_FILES test/test_dyn_arr.c)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# ===== Output Directories ==============

set_target_properties(
  ${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE
                             ${CMAKE_SOURCE_DIR}/target/Release)
set_target_properties(
  ${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG
                             ${CMAKE_SOURCE_DIR}/target/Debug)
if(WIN32)
    set_target_properties(
    ${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY
                               ${CMAKE_SOURCE_DIR}/target/Debug)
elseif(UNIX)

endif()
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/target/Release)
else() # Debug
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/target/Debug)
endif()
message(
  STATUS "Set binary output directory to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

# ===== Copy Commands ===================

# cmake configure time copy
if(EXISTS "${CMAKE_BINARY_DIR}/compile_commands.json")
    file(COPY "${CMAKE_BINARY_DIR}/compile_commands.json"
       DESTINATION ${CMAKE_SOURCE_DIR})
endif()
