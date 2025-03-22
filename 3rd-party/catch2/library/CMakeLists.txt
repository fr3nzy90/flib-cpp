# Copyright © 2024-2025 Luka Arnecic.
# See the LICENSE file at the top-level directory of this distribution.

set(project_name "Catch2")

source_group("/Header Files" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/../include/catch2/*.hpp")
source_group("/Header Files (3rd-party)" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/../v3.8.0/catch2/catch_amalgamated.hpp")
source_group("/Source Files (3rd-party)" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/../v3.8.0/catch_amalgamated.cpp")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../v3.8.0")

file(GLOB src_files "${CMAKE_CURRENT_SOURCE_DIR}/../v3.8.0/catch_amalgamated.cpp")

# if debugging with visual studio, enable memory leak detection from catch2
if(MSVC AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_definitions(CATCH_CONFIG_WINDOWS_CRTDBG)
  message("Catch2 library: Visual Studio debug mode detected ... enabling memory leak detection")
endif()

add_library(${project_name} STATIC ${src_files})

 # disable cartain compiler warnings only for this project
if(MSVC)
  target_compile_options(${project_name} PRIVATE "/wd4003;")
else()
  target_compile_options(${project_name} PRIVATE -Wno-conversion)
endif()