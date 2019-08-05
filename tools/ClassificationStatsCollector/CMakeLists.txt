cmake_minimum_required(VERSION 3.10)
project(ClassificationStatsCollector)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(ClassificationStatsCollector src/main.cpp src/ClassificationStatsWriter.cpp)

target_include_directories(ClassificationStatsCollector PRIVATE
        ${CMAKE_SOURCE_DIR}/cpptqdm
        ${CMAKE_SOURCE_DIR}/shared/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ClassificationStatsCollector pugixml pthread stdc++fs XmlSharedTools)
