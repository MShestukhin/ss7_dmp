project(dmp_sca)

SET(CMAKE_CXX_FLAGS "-lrt")
cmake_minimum_required(VERSION 2.8)
aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST})
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)

find_package(Boost COMPONENTS system log thread filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

add_library(parser STATIC  ./parser.cpp)

target_link_libraries(dmp_sca ${Boost_LIBRARIES})

add_library(libpq STATIC IMPORTED)
set_target_properties(libpq PROPERTIES IMPORTED_LOCATION /usr/local/pgsql/lib/libpq.so)

target_link_libraries(dmp_sca libpq)

add_library(libconfig STATIC IMPORTED)
set_target_properties(libconfig PROPERTIES IMPORTED_LOCATION /usr/local/lib/libconfig++.a)
target_link_libraries(dmp_sca libconfig)
