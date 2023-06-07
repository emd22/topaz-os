cmake_minimum_required(VERSION 3.5)

set(BUILD_NAME DiskBuilder)
project(${BUILD_NAME} C)

file(GLOB_RECURSE SOURCES src/*.c)
file(GLOB_RECURSE HEADERS src/*.h)

add_executable(${BUILD_NAME} ${SOURCES} ${HEADERS})
include_directories("../src/")

set_property(TARGET ${BUILD_NAME} PROPERTY C_STANDARD 11)