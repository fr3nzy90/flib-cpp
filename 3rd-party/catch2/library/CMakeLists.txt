# MIT License
#
# Copyright (c) 2024 Luka Arnecic
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

set(project_name "Catch2")

source_group("/Header Files" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/../include/catch2/*.hpp")
source_group("/Header Files (3rd-party)" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/../v3.7.1/catch2/catch_amalgamated.hpp")
source_group("/Source Files (3rd-party)" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/../v3.7.1/catch_amalgamated.cpp")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../v3.7.1")

file(GLOB src_files "${CMAKE_CURRENT_SOURCE_DIR}/../v3.7.1/catch_amalgamated.cpp")

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