# Copyright � 2024-2025 Luka Arnecic.
# See the LICENSE file at the top-level directory of this distribution.

set(project_name "worker_example")
project(${project_name} DESCRIPTION "Worker usage example")

source_group("/Header Files" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp")
source_group("/Source Files" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

file(GLOB src_files
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

if(UNIX)
  add_definitions(-pthread)
endif()

add_executable(${project_name} ${src_files})

if(UNIX)
  target_link_libraries(${project_name} pthread)
endif()