cmake_minimum_required(VERSION 3.10)
project(Database)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_SOURCE_DIR}/lib)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")


add_executable(DatabaseGenerator src/main.cpp src/DataTypes.cpp include/DataTypes.h include/XpathQueryCollection.cpp include/XpathQueryCollection.h src/DataAndIndexGeneratorThread.cpp include/DataAndIndexGeneratorThread.h src/DatabaseFileWriterThread.cpp include/DatabaseFileWriterThread.h include/Helper.h src/DatabaseQueryManager.cpp include/DatabaseQueryManager.h include/DataBasicTypes.h)
add_library(DatabaseQuerySelector SHARED src/DatabaseQuerySelector.cpp src/DataTypes.cpp src/DatabaseQueryManager.cpp)
add_executable(DatabaseQuerySelectorExe src/DatabaseQuerySelector.cpp src/DatabaseQueryManager.cpp src/DataTypes.cpp)

target_include_directories(DatabaseGenerator PRIVATE
        ${CMAKE_SOURCE_DIR}/cpptqdm
        ${CMAKE_SOURCE_DIR}/shared/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(DatabaseQuerySelector PRIVATE
        ${CMAKE_SOURCE_DIR}/cpptqdm
        ${CMAKE_SOURCE_DIR}/shared/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(DatabaseQuerySelectorExe PRIVATE
        ${CMAKE_SOURCE_DIR}/cpptqdm
        ${CMAKE_SOURCE_DIR}/shared/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

target_link_libraries(DatabaseGenerator XmlSharedTools)
target_link_libraries(DatabaseQuerySelector XmlSharedTools)
target_link_libraries(DatabaseQuerySelectorExe XmlSharedTools)


# custom variable for swig wrappers
set(SWIG_CPP_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(SwigWrapper)
