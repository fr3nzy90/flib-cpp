# Copyright © 2024 Luka Arnecic.
# See the LICENSE file at the top-level directory of this distribution.

set(project_name "mld_example")
project(${project_name} DESCRIPTION "Memory leak detection example")

source_group("/Header Files" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/headers/*.hpp")
source_group("/Source Files" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

file(GLOB src_files "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(${project_name} ${src_files})

add_dependencies(${project_name} test_dll)