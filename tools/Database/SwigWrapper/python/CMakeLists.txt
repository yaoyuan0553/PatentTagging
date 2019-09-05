cmake_minimum_required(VERSION 3.14)
project(DatabaseQueryPython)

find_package(SWIG)
include(UseSWIG)

set(CMAKE_CXX_STANDARD 17)

set(SWIG_OUT_DIR ${CMAKE_SOURCE_DIR}/swig_out/python)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${SWIG_OUT_DIR})

set_property(SOURCE DatabaseQueryPython.i PROPERTY CPLUSPLUS ON)

set_property(SOURCE DatabaseQueryPython.i PROPERTY SWIG_MODULE_NAME DatabaseQueryPython)

swig_add_library(
        DatabaseQueryPython
        TYPE SHARED
        LANGUAGE python
        OUTPUT_DIR ${SWIG_OUT_DIR}
        OUTFILE_DIR ${SWIG_OUT_DIR}
        SOURCES DatabaseQueryPython.i ${SWIG_WRAPPER_SRC_DIR}/DatabaseQueryWrapper.cpp
            ${SWIG_CPP_SRC_DIR}/src/DataTypes.cpp ${SWIG_CPP_SRC_DIR}/src/IndexTableV2.cpp
            ${SWIG_CPP_SRC_DIR}/src/DataRecordFileV2.cpp
)

set_property(TARGET DatabaseQueryPython PROPERTY SWIG_COMPILE_OPTIONS -py3 -threads -doxygen -fastproxy -olddefs)

target_include_directories(
        DatabaseQueryPython PRIVATE
        /usr/include/python3.6
        ${CMAKE_SOURCE_DIR}/shared/include
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${SWIG_WRAPPER_SRC_DIR}
        ${SWIG_CPP_SRC_DIR}/include
)

target_link_libraries(DatabaseQueryPython XmlSharedTools)
