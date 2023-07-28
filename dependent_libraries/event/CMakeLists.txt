include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dependent_libraries/include)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src EVENT_SRC)

add_library (event ${EVENT_SRC})