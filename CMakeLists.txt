# Copyright © 2019-2025 Luka Arnecic.
# See the LICENSE file at the top-level directory of this distribution.

cmake_minimum_required(VERSION 3.8)

project("fr3nzy's library solution")

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
  if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    list(APPEND CMAKE_CXX_FLAGS_LIST
      -std=c++14
      -fPIE
      -fvisibility=hidden
      -pipe
      -Wpedantic
      -Wall
      -Wextra
      -Werror
      -Wshadow
      -Wconversion
      -Wno-unknown-warning-option
      -Wno-invalid-utf8
      -Wno-unknown-pragmas
    )
  elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND CMAKE_CXX_FLAGS_LIST
      -std=c++14
      -fPIE
      -fvisibility=hidden
      -pipe
      -Wpedantic
      -Wall
      -Wextra
      -Werror
      -Wshadow
      -Wconversion
      -Wno-unknown-pragmas
    )
  endif()

  string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_LIST}")

  set(CMAKE_EXE_LINKER_FLAGS "-Wl,-O1,-pie,--as-needed,--fatal-warnings,-rpath,.")
  
  message("***                CMAKE_CXX_FLAGS set to: ${CMAKE_CXX_FLAGS}")
  message("***         CMAKE_EXE_LINKER_FLAGS set to: ${CMAKE_EXE_LINKER_FLAGS}")
endif()

find_package(FLib REQUIRED HINTS "${CMAKE_SOURCE_DIR}")
message("***                          flib version: ${FLib_FIND_VERSION}")

find_package(catch2 REQUIRED HINTS "${CMAKE_SOURCE_DIR}/3rd-party/catch2")

include_directories("${FLib_INCLUDE_DIRS}"
                    "${Catch2_INCLUDE_DIRS}")

enable_testing()

add_subdirectory("${CMAKE_SOURCE_DIR}/examples")
add_subdirectory("${CMAKE_SOURCE_DIR}/test_dll")
add_subdirectory("${CMAKE_SOURCE_DIR}/tests")