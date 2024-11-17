# Copyright © 2020-2024 Luka Arnecic.
# See the LICENSE file at the top-level directory of this distribution.

set(project_name "test_dll")
project(${project_name} DESCRIPTION "fr3nzy's test dll")

source_group("/Header Files" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
source_group("/Source Files" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

file(GLOB src_files
  "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(${project_name} SHARED ${src_files})

set_target_properties(${project_name} PROPERTIES PREFIX "")