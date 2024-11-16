# MIT License
#
# Copyright (c) 2019 Luka Arnecic
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
# documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
# OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

set(project_name "tests")
project(${project_name} DESCRIPTION "fr3nzy's library tests")

source_group("/Header Files" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/headers/*.hpp")
source_group("/Source Files/Common" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/common/*.cpp")
source_group("/Source Files/Suites" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/suites/*.cpp")

file(GLOB src_files
  "${CMAKE_CURRENT_SOURCE_DIR}/src/common/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/suites/*.cpp")

if(UNIX)
  add_definitions(-pthread)
endif()

add_executable(${project_name} ${src_files})

add_dependencies(${project_name} ${Catch2_LIBRARIES} test_dll)

target_link_libraries(${project_name} ${Catch2_LIBRARIES})

if(UNIX)
  target_link_libraries(${project_name} pthread dl)
endif()