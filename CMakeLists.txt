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

cmake_minimum_required(VERSION 3.8)

project("fr3nzy's library solution")

if(DEFINED OUTPUT_DIRECTORY)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}")

  message("*** CMAKE_ARCHIVE_OUTPUT_DIRECTORY set to: ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
  message("*** CMAKE_LIBRARY_OUTPUT_DIRECTORY set to: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
  message("*** CMAKE_RUNTIME_OUTPUT_DIRECTORY set to: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
endif()

if(MSVC)
  list(APPEND CMAKE_CXX_FLAGS_LIST
    /DWIN32
    /D_WINDOWS
    /GR
    /EHsc
    /W4
    /WX
    /std:c++14
  )
  string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_LIST}")
  
  message("***                CMAKE_CXX_FLAGS set to: ${CMAKE_CXX_FLAGS}")
else()
  list(APPEND CMAKE_CXX_FLAGS_LIST
    -std=c++11
    -fPIE
    -fvisibility=hidden
    -pipe
    -Wpedantic
    -Wall
    -Wextra
    -Werror
    -Wshadow
    -Wconversion
  )
  string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_LIST}")

  set(CMAKE_EXE_LINKER_FLAGS "-Wl,-O1,-pie,--as-needed,--fatal-warnings,-rpath,.")
  
  message("***                CMAKE_CXX_FLAGS set to: ${CMAKE_CXX_FLAGS}")
  message("***         CMAKE_EXE_LINKER_FLAGS set to: ${CMAKE_EXE_LINKER_FLAGS}")
endif()

find_package(catch REQUIRED HINTS "${CMAKE_SOURCE_DIR}/3rd-party/catch")
find_package(FLib REQUIRED HINTS "${CMAKE_SOURCE_DIR}/flib")

include_directories("${FLib_INCLUDE_DIRS}"
                    "${Catch_INCLUDE_DIRS}")

add_subdirectory("${CMAKE_SOURCE_DIR}/tests")