cmake_minimum_required(VERSION 3.0)
project(meteos-backend)

option(BUILD_TEST "" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER /usr/bin/g++-8)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

# build project binaries

file(GLOB_RECURSE project_SRC
    "../src/*.h"
    "../src/*.cpp"
)

add_executable(meteos-backend ${project_SRC})

set(cpprestsdk_DIR /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/cmake/)

find_package(SQLite3 REQUIRED)
find_package(Boost REQUIRED system)
find_package(cpprestsdk REQUIRED)

target_link_libraries(meteos-backend PRIVATE stdc++fs sqlite3 cpprestsdk::cpprest ${Boost_SYSTEM_LIBRARY})

# build test binaries

if (BUILD_TEST) 

file(GLOB_RECURSE project_test_SRC
    "../test/*.cpp"
)

add_executable(meteos-backend-test ${project_test_SRC})

find_package(Catch2 REQUIRED)
target_link_libraries(meteos-backend-test PRIVATE Catch2::Catch2 cpprestsdk::cpprest ${Boost_SYSTEM_LIBRARY})

endif (BUILD_TEST)