set(Catch2_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/include")
set(Catch2_LIBRARY_DIRS "")
set(Catch2_LIBRARIES "Catch2")
set(Catch2_FOUND TRUE)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/library")