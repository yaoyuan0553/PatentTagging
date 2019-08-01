cmake_minimum_required(VERSION 3.10)
project(PatentDataPreprocessor)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(PatentDataPreprocessor src/main.cpp include/XmlFileReader.h src/XmlFileReader.cpp include/PatentInfoCollector.h src/PatentTagAbstractWalker.cpp include/PatentTagAbstractWalker.h src/PatentInfoCollector.cpp src/PatentInfoWriter.cpp include/PatentInfoWriter.h src/test.cpp include/Utility.h include/PatentInfoPC.h src/PatentInfoPC.cpp src/XmlTagTextWalker.cpp include/XmlTagTextWalker.h src/PatentTagTextCollector.cpp include/PatentTagTextCollector.h include/FormatFunctors.h include/FunctorDict.h src/FormatFunctors.cpp include/TagConstants.h include/TagNodeFilterFunctors.h src/TagNodeFilterFunctors.cpp)

target_include_directories(PatentDataPreprocessor PRIVATE
        ${CMAKE_SOURCE_DIR}/cpptqdm
        ${CMAKE_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(PatentDataPreprocessor pugixml pthread stdc++fs)
