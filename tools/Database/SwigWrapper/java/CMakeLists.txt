cmake_minimum_required(VERSION 3.14)
project(DatabaseQueryJava)

find_package(SWIG)
include(UseSWIG)

set(CMAKE_CXX_STANDARD 17)

set(SWIG_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/swig_out/java)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${SWIG_OUTPUT_DIR})

set_property(SOURCE DatabaseQueryJava.i PROPERTY CPLUSPLUS ON)

set_property(SOURCE DatabaseQueryJava.i PROPERTY SWIG_MODULE_NAME DatabaseQueryJava)

swig_add_library(
        DatabaseQueryJava
        TYPE SHARED
        LANGUAGE java
        OUTPUT_DIR ${SWIG_OUTPUT_DIR}
        OUTFILE_DIR ${SWIG_OUTPUT_DIR}
        SOURCES DatabaseQueryJava.i ${SWIG_WRAPPER_SRC_DIR}/DatabaseQueryWrapper.cpp ${SWIG_CPP_SRC_DIR}/src/DataTypes.cpp
)

set_property(TARGET DatabaseQueryJava PROPERTY SWIG_COMPILE_OPTIONS -package DatabaseQuery)

target_include_directories(
        DatabaseQueryJava PRIVATE
        /usr/share/jdk1.8.0_202/include
        /usr/share/jdk1.8.0_202/include/linux
        ${CMAKE_SOURCE_DIR}/shared/include
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${SWIG_WRAPPER_SRC_DIR}
        ${SWIG_CPP_SRC_DIR}/include
)

#set(JAVA_OUT_PATH ${CMAKE_SOURCE_DIR}/java_out_test)
#set(JAVA_OUT_PATH /home/yuan/IdeaProjects/SwigTest/src/main/java)

#if (${JAVA_OUT_PATH})
message(STATUS "copying output files to [${JAVA_OUT_PATH}]")
add_custom_command(TARGET DatabaseQueryJava POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${SWIG_OUTPUT_DIR}" "${JAVA_OUT_PATH}/DatabaseQuery")
#else()
#    message(STATUS "JAVA_OUT_PATH not set, output files not copied")
#endif()
