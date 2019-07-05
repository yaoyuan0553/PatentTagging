cmake_minimum_required(VERSION 3.10)
project(tools)

set(CMAKE_CXX_STANDARD 17)

add_executable(tools main.cpp Semaphore.cpp Semaphore.h ConcurrentQueue.h PatentTagCollector.cpp PatentTagCollector.h PatentTagWalker.cpp PatentTagWalker.h FileDistributor.cpp FileDistributor.h ThreadJob.cpp ThreadJob.h ThreadDispatcher.cpp ThreadDispatcher.h StatsThread.cpp StatsThread.h)

target_include_directories(tools PRIVATE cpptqdm/)
target_link_libraries(tools pugixml pthread)
