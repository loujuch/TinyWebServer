cmake_minimum_required(VERSION 2.8)

project(TinyWebServer)

set(CMAKE_CXX_STANDARD 11)

# set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE Debug)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)


aux_source_directory(${PROJECT_SOURCE_DIR}/src SRCS)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/dependent_libraries/log/include)
include_directories(${PROJECT_SOURCE_DIR}/dependent_libraries/event/include)

add_subdirectory(${PROJECT_SOURCE_DIR}/dependent_libraries/log)
add_subdirectory(${PROJECT_SOURCE_DIR}/dependent_libraries/event)

add_executable(TinyWebServer ${SRCS})

target_link_libraries(TinyWebServer log event pthread )
