cmake_minimum_required (VERSION 3.0)
cmake_policy(VERSION 3.0)
project(onhttp)

aux_source_directory(. SOURCES)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} onion)
