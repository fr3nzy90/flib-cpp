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

set(FLib_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}")
set(FLib_LIBRARY_DIRS "")
set(FLib_LIBRARIES "")
file(READ "${FLib_INCLUDE_DIRS}/flib/version.hpp" FLib_VERSION_HEADER)
string(REGEX MATCH "#define FLIB_VERSION_MAJOR [0-9]+" FLib_VERSION_MAJOR_LINE "${FLib_VERSION_HEADER}")
string(REGEX MATCH "#define FLIB_VERSION_MINOR [0-9]+" FLib_VERSION_MINOR_LINE "${FLib_VERSION_HEADER}")
string(REGEX MATCH "#define FLIB_VERSION_PATCH [0-9]+" FLib_VERSION_PATCH_LINE "${FLib_VERSION_HEADER}")
string(REGEX MATCH "[0-9]+" FLib_VERSION_MAJOR "${FLib_VERSION_MAJOR_LINE}")
string(REGEX MATCH "[0-9]+" FLib_VERSION_MINOR "${FLib_VERSION_MINOR_LINE}")
string(REGEX MATCH "[0-9]+" FLib_VERSION_PATCH "${FLib_VERSION_PATCH_LINE}")
set(FLib_FIND_VERSION "${FLib_VERSION_MAJOR}.${FLib_VERSION_MINOR}.${FLib_VERSION_PATCH}")
set(FLib_FOUND TRUE)