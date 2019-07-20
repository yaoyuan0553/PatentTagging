cmake_minimum_required(VERSION 3.10)
project(PatentTagCollector)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_executable(PatentTagCollector src/main.cpp src/Semaphore.cpp include/Semaphore.h src/PatentTagCollector.cpp include/PatentTagCollector.h src/PatentTagWalker.cpp include/PatentTagWalker.h src/FileDistributor.cpp include/FileDistributor.h src/ThreadDispatcher.cpp include/ThreadDispatcher.h src/StatsThread.cpp include/StatsThread.h)

target_include_directories(PatentTagCollector PRIVATE
        ${CMAKE_SOURCE_DIR}/cpptqdm
        ${CMAKE_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(PatentTagCollector pugixml pthread)
