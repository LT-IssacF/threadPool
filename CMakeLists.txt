cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

project(main LANGUAGES CXX)

set(SOURCES example.cpp include/threadpool.cpp)
add_executable(main ${SOURCES})

find_package(Threads REQUIRED)
target_link_libraries(main PUBLIC Threads::Threads)
target_include_directories(main PUBLIC ${CMAKE_SOURCE_DIR}/include)