cmake_minimum_required(VERSION 3.10)
project(Database)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(DatabaseGenerator src/main.cpp src/DataTypes.cpp include/DataTypes.h include/XpathQueryCollection.cpp include/XpathQueryCollection.h src/DataAndIndexGeneratorThread.cpp include/DataAndIndexGeneratorThread.h src/DatabaseFileWriterThread.cpp include/DatabaseFileWriterThread.h include/Helper.h)

target_include_directories(DatabaseGenerator PRIVATE
        ${CMAKE_SOURCE_DIR}/cpptqdm
        ${CMAKE_SOURCE_DIR}/shared/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(DatabaseGenerator XmlSharedTools)
