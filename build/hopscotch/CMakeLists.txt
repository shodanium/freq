cmake_minimum_required(VERSION 3.2)

project(freq)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O3")

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})

add_subdirectory(hopscotch-map)

add_executable(freq src/main.cpp)
target_link_libraries(freq PRIVATE tsl::hopscotch_map)
