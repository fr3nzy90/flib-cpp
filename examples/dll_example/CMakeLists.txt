# Copyright © 2024 Luka Arnecic.
# See the LICENSE file at the top-level directory of this distribution.

set(project_name "dll_example")
project(${project_name} DESCRIPTION "DLL usage example")

source_group("/Header Files" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp")
source_group("/Source Files" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

file(GLOB src_files
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(${project_name} ${src_files})

add_dependencies(${project_name} test_dll)