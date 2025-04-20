# Copyright © 2019-2025 Luka Arnecic.
# See the LICENSE file at the top-level directory of this distribution.

set(project_name "tests")
set(test_dll "test_dll")
project(${project_name} DESCRIPTION "fr3nzy's library tests")

source_group("/Header Files/Common" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/common/*.hpp")
source_group("/Source Files/Common" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/common/*.cpp")
source_group("/Header Files/Suites" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/suites/*.hpp")
source_group("/Source Files/Suites" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/suites/*.cpp")

file(GLOB src_files
  "${CMAKE_CURRENT_SOURCE_DIR}/src/common/*.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/common/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/suites/*.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/suites/*.cpp")

if(UNIX)
  add_definitions(-pthread)
endif()

add_executable(${project_name} ${src_files})

add_dependencies(${project_name} ${Catch2_LIBRARIES} ${test_dll})

target_link_libraries(${project_name} ${Catch2_LIBRARIES})

if(UNIX)
  target_link_libraries(${project_name} pthread dl)
endif()

add_custom_command(TARGET ${project_name} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:${test_dll}>" "$<TARGET_FILE_DIR:${project_name}>")