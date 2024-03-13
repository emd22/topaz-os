cmake_minimum_required(VERSION 3.25)

set(HAVE_FLAG_SEARCH_PATHS_FIRST 0)

set(BUILD_NAME Boot32)
project(${BUILD_NAME} ASM C)


unset(_CMAKE_APPLE_ARCHS_DEFAULT)
set(CMAKE_CROSSCOMPILING 1)

set(CMAKE_SYSTEM_NAME Generic)

add_subdirectory(boot/Boot16)

set(CMAKE_EXE_LINKER_FLAGS "")
set(CMAKE_EXE_LINKER_FLAGS_INIT "")

set(CMAKE_CXX_COMPILER /opt/homebrew/bin/i686-elf-gcc)
set(CMAKE_C_COMPILER /opt/homebrew/bin/i686-elf-gcc)
set(CMAKE_ASM_COMPILER /opt/homebrew/bin/i686-elf-as)
set(CMAKE_LINKER i686-elf-ld)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(C_WARNING_FLAGS "")
set(CMAKE_C_FLAGS "-Os -m32 -fno-pie -ffreestanding -nostdlib -nostdinc -fno-stack-protector -nostartfiles ${C_WARNING_FLAGS}")
set(CMAKE_ASM_FLAGS "-nostdlib -nostartfiles")
set(LINKER_FLAGS "-Wl,--oformat=binary,-T${CMAKE_CURRENT_LIST_DIR}/boot/linker.ld,-melf_i386")

file(GLOB_RECURSE ASM_SOURCES "${CMAKE_CURRENT_LIST_DIR}/*.S" "${CMAKE_CURRENT_LIST_DIR}/boot/*.S")
file(GLOB_RECURSE SOURCES  "${CMAKE_CURRENT_LIST_DIR}/*.c" "${CMAKE_CURRENT_LIST_DIR}/boot/*.c" "${CMAKE_CURRENT_LIST_DIR}/CLib/*.c")
file(GLOB_RECURSE HEADERS "./**/*.h")

set(CMAKE_C_LINK_FLAGS "")
set(CMAKE_CXX_LINK_FLAGS "")

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/CLib
)

add_executable(${BUILD_NAME} ${ASM_SOURCES} ${SOURCES} ${HEADERS})
set_target_properties(${BUILD_NAME} PROPERTIES LINK_FLAGS ${LINKER_FLAGS})

set(DISK_IMAGE_PATH ${CMAKE_BINARY_DIR}/../topaz.img)
