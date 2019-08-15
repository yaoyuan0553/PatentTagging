cmake_minimum_required(VERSION 3.10)
project(Database)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(Database src/main.cpp src/DataTypes.cpp include/DataTypes.h src/NodeFilters.cpp include/NodeFilters.h src/OutputFormatters.cpp include/OutputFormatters.h include/XpathQueryCollection.cpp include/XpathQueryCollection.h)

target_include_directories(Database PRIVATE
        ${CMAKE_SOURCE_DIR}/cpptqdm
        ${CMAKE_SOURCE_DIR}/shared/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(Database XmlSharedTools)
