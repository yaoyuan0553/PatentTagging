cmake_minimum_required(VERSION 3.14)
project(DatabaseQueryWrapper)

# to pass down include directories to the sub-directories
include_directories(${SWIG_CPP_SRC_DIR})
include_directories(${SWIG_CPP_SRC_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/shared/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SWIG_WRAPPER_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(python)
add_subdirectory(java)
