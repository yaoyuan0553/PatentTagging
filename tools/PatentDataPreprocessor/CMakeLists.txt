cmake_minimum_required(VERSION 3.10)
project(PatentDataPreprocessor)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)


add_executable(PatentDataPreprocessor src/main.cpp)

target_include_directories(PatentDataPreprocessor PRIVATE
        ${CMAKE_SOURCE_DIR}/cpptqdm
        ${CMAKE_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(PatentDataPreprocessor pugixml pthread)
