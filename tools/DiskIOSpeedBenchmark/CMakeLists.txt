cmake_minimum_required(VERSION 3.10)
project(DiskIOSpeedBenchmark)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(DiskIOSpeedBenchmark main.cpp)

target_include_directories(DiskIOSpeedBenchmark PRIVATE
        ${CMAKE_SOURCE_DIR}/cpptqdm
        ${CMAKE_SOURCE_DIR}/shared/include
)

target_link_libraries(DiskIOSpeedBenchmark XmlSharedTools)
